# LinuxChat 架构全面审计报告

**日期**: 2026-06-19
**智能体**: 6个（架构总览、服务端深度、客户端深度、安全审计、测试覆盖、QML迁移）

---

## 一、架构总览评估

**代码规模**: ~3,958 LOC 应用 / ~1,401 LOC 测试
**整体评级**: 课程项目中优秀，但有明确的工程债务

```
Client (Windows Qt6)           Server (Linux)
  LoginDialog                    EpollServer (主线程: epoll_wait LT)
  MainWindow + ChatView    -->   ThreadPool (4 workers)
  ChatClient (QTcpSocket)  -->   MessageRouter (业务逻辑)
  QML scaffold (未启用)          Database (SQLite WAL)
       |                              |
       +--- JSON-over-TCP (4B BE) ---+
```

---

## 二、问题汇总（按严重度排序）

### CRITICAL（3个）

| ID | 问题 | 位置 | 修复难度 |
|----|------|------|---------|
| C1 | 帧大小不一致：服务端256KB vs 客户端16MB | protocol.cpp:135 vs chat_client.cpp:264 | 低 |
| C2 | body_len==0 丢弃整个recv_buf | protocol.cpp:126-131 | 低 |
| C3 | write_all EAGAIN无限循环，阻塞线程池 | protocol.cpp:34-51 | 中 |

### HIGH（8个）

| ID | 问题 | 位置 | 修复难度 |
|----|------|------|---------|
| H1 | now_stamp() 重复4份 | 4个.cpp文件 | 低 |
| H2 | session.username 数据竞争（worker写/主线程读） | epoll_server.cpp:293, message_router.cpp:118 | 中 |
| H3 | send_to_fd 持锁执行阻塞I/O | epoll_server.cpp:382-387 | 中 |
| H4 | generation counter 检查无效（永远为true） | epoll_server.cpp:305-308 | 低 |
| H5 | find_fd 线性扫描持锁 | epoll_server.cpp:389-397 | 中 |
| H6 | 直接 Protocol::send_msg(session.fd) 绕过session map | message_router.cpp:126,159,199,219,229 | 低 |
| H7 | running_ 非 atomic | epoll_server.h:71 | 低 |
| H8 | SHA-256 无盐、timing attack | message_router.cpp:48, database.cpp:128 | 中 |

### MEDIUM（7个）

| ID | 问题 | 位置 |
|----|------|------|
| M1 | QML scaffold 是死代码 | backend.cpp, message_model.cpp |
| M2 | main.cpp while(true) + 多次 app.exec() | main.cpp:85-116 |
| M3 | Database 无 prepared statement 缓存 | database.cpp |
| M4 | 密码明文传输 | chat_client.cpp:144-158 |
| M5 | get_history 返回最旧而非最新 | database.cpp:175 |
| M6 | broadcast 持锁期间 shared_ptr 拷贝开销 | epoll_server.cpp:270-277 |
| M7 | processEvents() 在 closeEvent 中 | main_window.cpp:533 |

### 安全专项（3个 CRITICAL）

| ID | 问题 | CVSS |
|----|------|------|
| V-01 | 密码明文传输 | 9.1 |
| V-02 | SHA-256无盐存储 | 8.1 |
| V-03 | 无TLS加密 | 9.1 |

---

## 三、测试覆盖

| 组件 | LOC | 测试数 | 覆盖率 | 风险 |
|------|-----|--------|--------|------|
| Protocol (服务端) | 175 | 18 | ~85% | 低 |
| Database | ~200 | 21 | ~90% | 低 |
| MessageRouter | 340 | 26 | ~60% | 中 |
| EpollServer | 398 | **0** | **0%** | **高** |
| ThreadPool | ~100 | 9 | ~85% | 低 |
| ChatClient (Qt) | 349 | **0** | **0%** | **高**（3个未测试的bug修复） |
| QML Models | ~110 | 0 | 0% | 中 |

---

## 四、优先级行动计划

### P0 — 立即修复（本周）

| # | 任务 | 预估 | 涉及文件 |
|---|------|------|---------|
| 1 | 统一帧大小常量 256KB | 30min | protocol.cpp, chat_client.cpp |
| 2 | write_all 添加超时（5秒） | 2h | protocol.cpp |
| 3 | 提取 now_stamp() 到 log_utils.h + localtime_r | 1h | 4个.cpp + 新头文件 |
| 4 | send_to_fd 释放锁后再发送 | 1h | epoll_server.cpp |
| 5 | generation counter 在 remove_client 中失效 | 30min | epoll_server.cpp |

### P1 — 短期改进（两周内）

| # | 任务 | 预估 |
|---|------|------|
| 6 | session.username 加 atomic flag 或 mutex | 2h |
| 7 | 所有 handler 改用 server_->send_to_fd | 2h |
| 8 | running_ 改为 std::atomic<bool> | 15min |
| 9 | 提取 SO_LINGER helper 消除重复代码 | 1h |
| 10 | Database 缓存 prepared statements | 1h |
| 11 | body_len==0 跳过帧而非断开连接 | 30min |
| 12 | get_history 改为返回最新N条 | 30min |

### P2 — 中期架构升级（一个月内）

| # | 任务 | 预估 |
|---|------|------|
| 13 | find_fd 改用 unordered_map<string,int> 索引 | 2h |
| 14 | 添加 per-IP 连接限制（5个/IP） | 3h |
| 15 | 添加登录速率限制 | 3h |
| 16 | main.cpp 重构：单次 app.exec + 页面切换 | 4h |
| 17 | ChatClient 自动重连机制 | 4h |
| 18 | 客户端帧大小对齐 256KB | 30min |

### P3 — 长期目标

| # | 任务 | 预估 |
|---|------|------|
| 19 | TLS 1.3 加密（服务端OpenSSL + 客户端QSslSocket） | 20h |
| 20 | Argon2id 替代 SHA-256 | 8h |
| 21 | QML 迁移（增量，--qml 标志门控） | 200-280h |
| 22 | 客户端测试框架（ChatClient mock） | 16h |
| 23 | EpollServer 集成测试 | 16h |

---

## 五、QML 迁移建议

**结论**: GO — 但增量并行，不搞大爆炸重写

1. 保留 Widgets 为默认路径
2. `--qml` CLI 标志门控 QML 路径
3. 先构建 QML 登录屏（验证全栈）
4. 添加 ConversationManager 管理多会话 Model
5. 样式逐步迁移，不复制 QSS
6. Widgets 保持为 shipping product 直到 QML 达到功能对等

**Backend 缺失**: sendPrivate, historyReceived, privateReceived, notifyReceived, loadHistory
