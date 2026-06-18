# TODO

> Active execution plan (steps). Gate completion on Automated checks + UAT.

Reference: `docs/specs/prd.md` and `docs/specs/blueprint.md`.
Archive: move completed steps to `docs/archive/TODO_YYYY-MM-DD.md` when noisy.
Scaling note: if active implementation work branches into `TODO-<area>.md`, keep root `TODO.md` as the index and keep each step in exactly one TODO file.

---

## Step 00 — project setup ✅ DONE (2026-06-17)

Goal:
- Establish the project contract (PRD + Blueprint) and a project-specific README.

Outcome:
- CDD 合约已物化:`AGENTS.md` / `docs/specs/prd.md` / `docs/specs/blueprint.md` / `docs/JOURNAL.md` / `docs/prompts/PROMPT-INDEX.md` / `docs/INDEX.md`(stub)
- 原 `CONTRACT.md`/`ARCHITECTURE.md`/`TODO.md`/`README.md` 已备份至 `docs/legacy/`
- CDD Phase 1 只读审计完成,发现 3 个连接层缺陷(见下方 Step 01-03 及 `docs/specs/prd.md` Known Issues)
- 原 superpowers Phase 1/2/3(server/client/integration)开发均已完成,产物保留

---

## Step 01 — 修复连接竞态 #1:epoll 水平触发 + 异步 worker 数据错位 ✅ DONE (2026-06-17) [Critical]

Goal:
- 消除"有时连得上有时连不上 / 登录后立即断开"的最高频根因。

Outcome:
- `recv_msgs` 改为 drain loop (循环读取直到 EAGAIN)
- oversized 帧判定优化：数据错位时优先清缓冲区继续，避免误杀
- 水平触发下"未读完会再次就绪"是安全的
- 验证：反复连接 20 次，无断开

UAT:
- [x] 代码修复完成（drain + generation + EVP + 日志 + 超时）
- [x] 服务端日志无 oversized 误判
- [x] 多帧响应(LOGIN_OK+HISTORY+USER_LIST+NOTIFY)客户端全部正确接收

---

## Step 02 — 修复连接竞态 #2:fd 复用导致 worker 串话 ✅ DONE (2026-06-17) [Critical]

Goal:
- 消除反复重连时偶发的"消息发错用户 / 数据写错 fd"。

Outcome:
- `ClientSession` 增加 `uint64_t generation` 字段，每次新建 session 递增
- worker lambda 按值捕获 generation token，校验不匹配则直接 return
- `sessions_` 改为 `shared_ptr<ClientSession>`，消除 use-after-free
- broadcast 优化：复制-释放-发送策略，避免长时持锁

UAT:
- [x] 快速反复连接/断开 50 次，无消息串话
- [x] 服务端无 use-after-close 相关的崩溃/异常

---

## Step 03 — 升级 SHA-256 到 OpenSSL 3.x 兼容 API ✅ DONE (2026-06-17) [High]

Goal:
- 消除服务端在新 Linux 发行版(Ubuntu 22.04+/Debian 12)上的废弃 API 告警/潜在编译问题。

Outcome:
- `sha256_hex` 改用 `EVP_Digest` / `EVP_MD_CTX` API
- 提取到 `MessageRouter` 类中作为静态方法
- 无 `-Wdeprecated-declarations` 告警

UAT:
- [x] 注册→登录密码校验正常
- [x] 编译无废弃 API 告警

---

## Step 04 — MessageRouter 架构重构 + 连接改进 ✅ DONE (2026-06-17) [Medium]

Goal:
- 从 main.cpp 提取消息路由逻辑到独立 MessageRouter 类
- 改进客户端连接超时语义
- 修复 broadcast 锁优化和 write_all EAGAIN 重试

Outcome:
- **MessageRouter 提取**: 430 行 main.cpp → 113 行 MessageRouter + 精简 main.cpp
- **shared_ptr 消除 use-after-free**: `sessions_` 从裸指针改为 `shared_ptr<ClientSession>`
- **broadcast 锁优化**: 复制-释放-发送策略，避免在持锁期间发送
- **write_all EAGAIN 重试**: `protocol.cpp` 中 `send_all` 循环处理 EAGAIN/EINTR
- **客户端超时**: 区分 TCP 连接超时与应用层登录超时(8s QTimer)
- **108 个单元测试**: test_protocol(19) + test_database(27) + test_message_handler(15) + test_message_router(27) + test_crypto(9) + test_thread_pool(11)

UAT:
- [x] MessageRouter 路由正确：REGISTER/LOGIN/BROADCAST/PRIVATE/HISTORY/LOGOUT
- [x] 服务端不响应时，8s 后客户端提示"登录超时"
- [x] 所有 108 个测试通过

---

## Step 05 — Glassmorphism 暗色主题 ✅ DONE (2026-06-17)

Goal:
- 实现现代 Glassmorphism 暗色主题，替换旧报纸复古风格
- 支持 `--test-chat` 直连模式用于快速 QSS 迭代

Outcome:
- **Glassmorphism Dark Theme**: 深色背景 + 半透明毛玻璃卡片 + 微妙边框发光
- **配色**: Dark slate (#0f172a) + Indigo accent (#6366f1) + 毛玻璃效果
- **Bypass 模式**: `--test-chat` 跳过登录，直接进入聊天界面测试 QSS
- **组件全覆盖**: LoginDialog, MainWindow, ChatView, Sidebar, InputArea
- **设计文档**: `docs/designs/2026-06-17-qss-chat-redesign.md`

UAT:
- [x] 直接 -t 启动聊天主界面（无登录对话、无服务器连接）
- [x] 改 QSS 后快速 rebuild+rerun 看到视觉变化
- [x] live send 在 -t 下产生正确 self bubble
- [x] 正常路径（无 flag）仍走登录

---

## Step 06 — 测试补全 ✅ DONE (2026-06-17)

Goal:
- 补全单元测试，覆盖所有核心模块

Outcome:
- **108 个测试用例**，覆盖 6 个模块：
  - `test_protocol.cpp`: 19 tests (帧编解码、边界情况)
  - `test_database.cpp`: 27 tests (CRUD、历史查询、并发)
  - `test_message_handler.cpp`: 15 tests (处理器业务逻辑)
  - `test_message_router.cpp`: 27 tests (路由分发、认证、在线管理)
  - `test_crypto.cpp`: 9 tests (SHA-256 EVP API)
  - `test_thread_pool.cpp`: 11 tests (线程池任务分发)
- 所有测试通过 `ctest --output-on-failure` 验证

---

## 当前状态总结

| 组件 | 状态 | 说明 |
|------|------|------|
| 服务端核心 | ✅ 完成 | epoll + ThreadPool + MessageRouter + SQLite |
| 客户端 UI | ✅ 完成 | Qt6 + Glassmorphism Dark + --test-chat |
| 协议层 | ✅ 完成 | JSON-over-TCP, 4B BE prefix, EAGAIN 重试 |
| 测试 | ✅ 完成 | 108 个测试用例，全部通过 |
| 文档 | ✅ 完成 | CDD 工作流，PRD + Blueprint + Journal |

---

## Future Steps (Deferred)

- Friend system, blacklist, offline messages, file transfer
- HTTPS/WSS 升级
- 消息加密 (E2E)
- 移动端客户端
