# LinuxChat 最终修复报告

**日期**: 2026-06-19
**分析方法**: 7 个智能体并行分析

---

## 问题总结

### 1. recv n=0 仍然发生（第一次连接失败）

**根因**：`abort()` 发送 FIN 而非 RST，Windows `closesocket()` 异步，端口复用导致旧连接的 FIN 污染新连接。

**时序**：
```
T0: Socket A 连接到 server:8080，本地端口 37903
T1: setup_socket() 调用 abort() → closesocket() → FIN 异步排队
T2: 新 socket B 创建，复用相同本地端口 37903
T3: SYN 发出 → 服务器 accept() → fd=11
T4: Socket A 的 FIN 到达服务器 → 匹配到新连接的四元组 → recv(fd=11) 返回 0
```

**修复**：在 `abort()` 前设置 `SO_LINGER {1,0}`，强制发送 RST 而非 FIN。

### 2. JSON 解析错误（body 被破坏）

**根因**：与 recv n=0 相同 — 旧连接的残留数据污染了新连接的接收缓冲区。

**修复**：同上（SO_LINGER 修复）。

### 3. 攻击检测（66.132.195.107）

**模式**：Slowloris 式攻击，发送 900KB 假 body_len，不发送 body 数据。

**修复**：
- 降低 body_len 上限：16MB → 256KB
- JSON 解析失败累计 3 次后断开连接

---

## 实施的修复

### 客户端（chat_client.cpp）

```cpp
// 在 abort() 前设置 SO_LINGER {1,0}，强制 RST
#ifdef Q_OS_WIN
if (socket_->state() != QAbstractSocket::UnconnectedState) {
    qintptr fd = socket_->socketDescriptor();
    if (fd != -1) {
        struct linger lg;
        lg.l_onoff  = 1;
        lg.l_linger = 0;
        ::setsockopt(static_cast<SOCKET>(fd), SOL_SOCKET, SO_LINGER,
                     reinterpret_cast<const char*>(&lg), sizeof(lg));
    }
}
#endif
socket_->abort();  // 现在发送 RST，不是 FIN
```

### 服务端（protocol.cpp）

1. **降低 body_len 上限**：16MB → 256KB
2. **添加 body_len=0 检查**
3. **JSON 解析失败累计 3 次后断开连接**

```cpp
constexpr uint32_t MAX_BODY_LEN = 256 * 1024;
if (body_len > MAX_BODY_LEN) {
    // 断开连接
}

if (body_len == 0) {
    // 断开连接
}

// JSON 解析失败累计
if (++session.consecutive_parse_failures >= 3) {
    return std::nullopt;  // 断开连接
}
```

---

## 七个智能体分析结果

| Agent | 分析角度 | 主要发现 |
|-------|---------|---------|
| A | JSON 解析错误 | body 第一字节非法，可能是旧连接残留数据 |
| B | recv n=0 根因 | **abort() 发送 FIN 而非 RST，端口复用导致 FIN 污染** |
| C | 攻击模式 | Slowloris 式攻击，900KB 假 body_len |
| D | Windows fd 复用 | FIN 由四元组管理，问题是端口复用 |
| E | 帧验证 | body_len 上限过高，解析失败无断开机制 |
| F | 客户端发送时序 | 无竞态条件，问题在服务端缓冲区 |
| G | 错误处理 | recv_buf 无上限、write_all busy-wait |

---

## 仍需处理的问题

### 高优先级
1. **密码明文传输** → 需要 TLS 加密
2. **密码存储无盐** → 需要 bcrypt/argon2

### 中优先级
3. **每 IP 连接数限制** → 防止攻击
4. **Body 接收超时** → 防止 Slowloris 攻击
5. **write_all busy-wait** → 添加重试上限

### 低优先级
6. **send_json 返回值未检查**
7. **未知消息类型静默丢弃**
8. **peer_closed 未传递给服务端调用方**

---

**报告完成时间**: 2026-06-19 02:30
