# 第三轮分析报告 — recv n=0 连接Bug

**日期**: 2026-06-19 15:24
**服务器日志**:
```
[Server] New connection from 36.161.71.101:35716 (fd=11)
[Protocol] fd=11 recv n=0 (peer closed from client side)
[Server] fd=11 recv_msgs returned nullopt -> removing client
```

## 使用的Skills

| 智能体 | 技能 | 发现 |
|--------|------|------|
| Agent 1 | Explore (socket lifecycle) | `deleteLater()` 延迟销毁与新socket竞争 |
| Agent 2 | Explore (server-side) | 服务器端无bug，generation check有缺陷 |
| Agent 3 | Explore (Windows SO_LINGER) | SO_LINGER正确，RST/SYN时序竞争 |
| Agent 4 | `/comprehensive-review` (connection flow) | **根因：closeEvent发送FIN在先** |
| Agent 5 | `/comprehensive-review` (Qt signals) | `deleteLater()`应改为同步`delete` |
| Agent 6 | `/comprehensive-review` (protocol) | epoll level-triggered正确，无服务端bug |
| Agent 7 | `/comprehensive-review` (Windows networking) | 端口复用是核心机制 |

## 根因分析（5个智能体一致）

### 问题链条

```
MainWindow::closeEvent()
  └─ disconnect_from_server()
       └─ socket_->disconnectFromHost()   ← 发送FIN到网络（不可撤回）
  
[用户重新点击"连接"]

ChatClient::connect_to_server()
  └─ setup_socket()
       ├─ socket_->abort()                ← 发送RST（但FIN已在路上）
       ├─ socket_->deleteLater()          ← 延迟销毁，事件循环中可能干扰新socket
       └─ new QTcpSocket()
  └─ socket_->connectToHost()             ← 发送SYN，可能复用同一本地端口
```

### 时序（同一毫秒内）

| 时间 | 客户端 | 服务器 |
|------|--------|--------|
| T+0 | `disconnectFromHost()` 发送FIN | 收到FIN → CLOSE_WAIT |
| T+1 | `abort()` 发送RST | - |
| T+2 | `connectToHost()` 发送SYN | - |
| T+3 | - | accept() 新连接 (fd=11) |
| T+4 | - | recv() 返回0（FIN已在缓冲区） |

**关键**: Windows端口复用导致新旧连接使用相同的4元组 `(36.161.71.101:35716 → 120.55.63.32:8080)`。旧连接的FIN被内核缓冲到新连接的接收缓冲区。

## 修复方案

### Fix 1: `deleteLater()` → 同步 `delete`
`setup_socket()`中，`deleteLater()`延迟销毁旧socket，在事件循环中可能干扰新socket的连接。改为同步销毁。

### Fix 2: `closeEvent()` 使用 `abort()` 而非 `disconnectFromHost()`
`MainWindow::closeEvent()` 调用 `disconnect_from_server()` 发送FIN。改为使用 `abort()` + SO_LINGER 直接发送RST，避免FIN残留。

### Fix 3: `disconnect_from_server()` 内部使用 `abort()` 替代 `disconnectFromHost()`
所有断开路径统一使用RST而非FIN，彻底消除FIN残留问题。

## 与前两轮的区别

| 轮次 | 修复 | 为什么不够 |
|------|------|-----------|
| 第一轮 | 创建新socket (setup_socket) | 只解决了fd复用，没解决端口复用 |
| 第二轮 | SO_LINGER {1,0} | 只解决了abort()的FIN，没解决closeEvent的FIN |
| 第三轮 | **abort()替代disconnectFromHost()** | **消除所有FIN路径** |
