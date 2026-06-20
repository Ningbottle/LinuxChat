# LinuxChat 连接 Bug 深度分析报告

**日期**: 2026-06-19
**分析方法**: 7 个智能体并行分析
**问题**: 客户端第一次连接时，服务器立即收到 recv n=0（客户端 FIN）

---

## 七个智能体综合分析报告

---

### Agent A: Qt 信号/槽分析 (opus)

**发现**: `blockSignals(true)` 会永久丢弃信号，不会排队重放。如果连接在 blockSignals 期间完成，`connected()` 信号会丢失。

**结论**: blockSignals 设计正确，但可能导致信号丢失。

---

### Agent B: TCP 行为分析 (opus)

**发现**: 1ms 内 FIN 是异常的。`abort()` 在内核层面发送 RST，但新 SYN 可能与旧 RST 竞争。

**结论**: 根因是 `abort()` + `connectToHost()` 的时序竞争。

---

### Agent C: 竞态条件分析 (opus)

**发现 7 个竞态条件**:

| 编号 | 问题 | 严重程度 |
|------|------|---------|
| 1 | 双 `is_connecting_` 标志不同步 | 中 |
| 2 | errorOccurred + disconnected 双信号导致 UI 消息覆盖 | **高** |
| 3 | connect_timer_ 使用 disconnectFromHost 而非 abort | 中 |
| 4 | connect_timer_ 与 connected 信号顺序不确定 | 中 |
| 5 | on_socket_error() 无条件发射 connection_error | 中 |
| 6 | blockSignals 窗口移除导致 abort() 重入 | 高（已缓解） |
| 7 | 超时不清理 ChatClient::is_connecting_ | 低 |

---

### Agent D: 协议实现分析 (opus)

**发现确认的 Bug**:

```cpp
// protocol.cpp drain 循环
if (n == 0) {
    return std::nullopt;  // BUG: 丢弃了 recv_buf 中的数据
}
```

当客户端先发数据再关闭连接时，服务器会丢弃已接收的数据。

---

### Agent E: 代码质量审查 (sonnet)

**发现**:
- **Critical**: 密码明文传输、硬编码 IP
- **High**: send_json 返回值未检查、端口解析无验证、错误信号重复
- **Medium**: 性能问题（flush 频繁、内存拷贝）

---

### Agent F: Qt 最佳实践审查 (sonnet)

**关键发现**:

`on_socket_error` 和 `on_socket_disconnected` 双重信号发射是 BUG：

```
1. errorOccurred → on_socket_error() → is_connecting_=false → emit connection_error
2. disconnected → on_socket_disconnected() → is_connecting_=false → emit disconnected()
```

**结果**: 用户先看到 "连接失败: xxx"，立即被覆盖为 "服务器已断开连接"。

---

### Agent G: 网络编程质量审查 (sonnet)

**服务端发现**:

| 等级 | 问题 |
|------|------|
| **高** | `write_all()` busy-wait 阻塞，无超时/重试上限 |
| **高** | `send_to_fd()` 在锁内调用可能阻塞的 I/O |
| **中** | `running_` 非 atomic，数据竞争 |

---

### Agent H: 详细时序分析 (opus)

**根因确认**:

**在 Windows 上，`abort()` 发送的是 FIN 而非 RST**，因为 Qt 未设置 `SO_LINGER {1,0}`。

完整时序:
```
1. socket_->abort() → closesocket(old_fd) → FIN 内核排队中...
2. socket_->connectToHost() → new_fd = ::socket() → (new_fd == old_fd 复用)
3. ::connect(new_fd) → SYN 发出 → 服务器 accept() → 连接建立
4. 内核处理 old_fd 的 FIN → FIN 发到服务器 → recv() 返回 0
```

---

## 最终结论

### 根因

**Windows socket descriptor 复用竞争**:
- `abort()` 调用 `closesocket()` 发送 FIN（异步）
- `connectToHost()` 立即创建新 socket，复用相同 fd
- 内核的残留 FIN 操作污染了新连接

### 修复方案

**推荐方案: 每次连接创建新 QTcpSocket**

```cpp
void ChatClient::connect_to_server(const QString& host, quint16 port) {
    if (is_connecting_) return;
    is_connecting_ = true;

    // 销毁旧 socket，创建全新的
    if (socket_) {
        socket_->blockSignals(true);
        socket_->abort();
        socket_->deleteLater();
    }

    socket_ = new QTcpSocket(this);
    connect(socket_, &QTcpSocket::connected, this, &ChatClient::on_socket_connected);
    connect(socket_, &QTcpSocket::disconnected, this, &ChatClient::on_socket_disconnected);
    connect(socket_, &QTcpSocket::readyRead, this, &ChatClient::on_ready_read);
    connect(socket_, &QAbstractSocket::errorOccurred, this, &ChatClient::on_socket_error);

    recv_buf_.clear();
    socket_->connectToHost(host, port);
}
```

**同时修复**:
1. `on_socket_error()` 只记录日志，不发射信号
2. `on_socket_disconnected()` 统一处理状态转换
3. 服务器 `recv_msgs()` 在 FIN 时先处理 recv_buf 再返回

---

## 附录：服务器日志分析

```
2026-06-19 01:02:50.447 [Server] New connection from 36.161.73.100:37980 (fd=11)
2026-06-19 01:02:50.448 [Protocol] fd=11 recv n=0 (peer closed from client side)
2026-06-19 01:02:50.448 [Server] fd=11 recv_msgs returned nullopt -> removing client
2026-06-19 01:02:50.448 [Server] Client disconnected: (unauth) (fd=11, gen=1)

2026-06-19 01:02:59.379 [Server] New connection from 36.161.73.100:37981 (fd=11)
2026-06-19 01:04:49.666 [Protocol] fd=11 recv n=0 (peer closed from client side)
2026-06-19 01:04:49.666 [Server] fd=11 recv_msgs returned nullopt -> removing client
2026-06-19 01:04:49.666 [Server] Client disconnected: (unauth) (fd=11, gen=2)

2026-06-19 01:05:14.609 [Server] New connection from 36.161.73.100:37988 (fd=11)
2026-06-19 01:05:16.510 [Protocol] fd=11 header body_len=67
2026-06-19 01:05:16.510 [Server] Auth frame received fd=11 type=REGISTER
2026-06-19 01:05:17.511 [Protocol] fd=11 recv n=0 (peer closed from client side)
2026-06-19 01:05:17.511 [Server] fd=11 recv_msgs returned nullopt -> removing client
2026-06-19 01:05:17.511 [Server] Client disconnected: (unauth) (fd=11, gen=3)
```

**规律**:
- 第一次连接: 立即失败 (recv n=0)
- 第二次连接: 成功，空闲 110 秒后超时断开
- 第三次连接: 发送 REGISTER 后 1 秒断开

---

## 附录：七个智能体使用的分析方法

| Agent | 分析角度 | 模型 | 主要发现 |
|-------|---------|------|---------|
| A | Qt 信号/槽机制 | opus | blockSignals 永久丢弃信号 |
| B | TCP 协议行为 | opus | 1ms FIN 异常，abort/SYN 竞争 |
| C | 竞态条件 | opus | 7 个竞态条件 |
| D | 协议实现 | opus | recv_msgs FIN 时丢弃数据 |
| E | 代码质量 | sonnet | 密码明文、硬编码 IP、输入验证 |
| F | Qt 最佳实践 | sonnet | 双重信号发射 BUG |
| G | 网络编程质量 | sonnet | write_all busy-wait、锁内 I/O |
| H | 详细时序分析 | opus | Windows socket fd 复用竞争 |

---

**文档完成时间**: 2026-06-19 01:30
