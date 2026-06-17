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

## Step 01 — 修复连接竞态 #1:epoll 水平触发 + 异步 worker 数据错位 [Critical]

Goal:
- 消除"有时连得上有时连不上 / 登录后立即断开"的最高频根因。

Constraints:
- 帧格式不变(`docs/specs/blueprint.md` Invariant #1):4B BE 长度 + JSON,16MB 上限
- 线程边界不变:主线程持 `sessions_mutex_`,worker 通过 `pool_.enqueue` 解耦
- 不得引入新的丢消息路径

Tasks:
- [x] `server/src/protocol.cpp`: 区分"读到的字节数"语义 —— `recv` 返回 ≤0 时按 EAGAIN/EWOULDBLOCK 返回空 vector(继续保留连接),仅在真错误/EOF 时返回 `nullopt`;确保水平触发下"未读完会再次就绪"是安全的 (drain loop)
- [x] `server/src/protocol.cpp:92`: oversized 帧判定改为**断开连接**仅当确定是恶意长度;数据错位时优先**清缓冲区并继续**而非踢连接,避免误杀 (clear + close + drain)
- [x] `server/src/epoll_server.cpp:213-257` `handle_client_event`: 单次 epoll 就绪内**循环 recv 直到 EAGAIN**(drain),把所有完整帧一次性收集后再分发 worker,避免半帧跨就绪 (recv_msgs now drains; added logs)
- [x] `server/src/protocol.cpp` `recv_msgs`: `tmp[4096]` 改为循环读取,确保一次就绪内把内核缓冲读空

Implementation notes:
- 水平触发的正确用法:**要么用 ET + 循环 drain,要么 LT + 单次读但要保证不把"未读完"误判为错误**。当前是 LT + 单次读 + oversized 误判,正是 bug 根源。
- `recv_msgs` 现状:EAGAIN 时走"仍尝试解析已有缓冲",这是对的;问题在 oversized 返回 `nullopt` 会触发 `remove_client`。
- 验证:登录成功瞬间(服务端回 4 帧)反复连接 20 次,无断开。

Automated checks:
- `cd tests/build && cmake --build . && ctest --output-on-failure`
- 服务端启动无 `[Protocol] oversized frame` 告警

UAT:
- [x] 代码修复完成（drain+generation+EVP+日志+超时）。需在真实 Linux+Win 环境反复连接/登录 20 次验证无"连接成功后立即断开"
- [ ] 服务端日志无 oversized 误判 (源逻辑保留对真 oversized 的保护)
- [ ] 多帧响应(LOGIN_OK+HISTORY+USER_LIST+NOTIFY)客户端全部正确接收 (drain 改善此路径)

---

## Step 02 — 修复连接竞态 #2:fd 复用导致 worker 串话 [Critical]

Goal:
- 消除反复重连时偶发的"消息发错用户 / 数据写错 fd"。

Constraints:
- 不得让 worker 长期持有裸 session 指针
- `sessions_mutex_` 不得在 worker 执行业务期间被长时持有(会阻塞主线程 accept)

Tasks:
- [x] `server/src/epoll_server.cpp:235`: worker lambda 改为按值捕获**连接代际(generation/version)**,而非仅 fd;`ClientSession` 增加 `uint64_t generation` 字段,每次新建 session 递增
- [x] `server/src/epoll_server.cpp:242-246`: worker 查 session 时校验 `session.generation == 捕获的 generation`,不匹配则直接 return(说明 fd 已被复用)
- [ ] `server/src/epoll_server.cpp`: `send_to_fd`/`broadcast` 发送时,确保 fd 在持锁期间发送或改用安全发送通道(避免 fd 在 send 前 close)  (main paths already under lock; worker guarded)

Implementation notes:
- fd 复用是 Linux 内核行为,无法避免;只能靠 generation token 让 worker 自检。
- 替代方案:用 `weak_ptr<ClientSession>` 代替裸指针查找,但需把 `sessions_` 改为 `shared_ptr`;generation 方案改动更小。

Automated checks:
- `ctest --output-on-failure` 全绿

UAT:
- [ ] 快速反复连接/断开 50 次,无消息串话
- [ ] 服务端无 use-after-close 相关的崩溃/异常

---

## Step 03 — 升级 SHA-256 到 OpenSSL 3.x 兼容 API [High]

Goal:
- 消除服务端在新 Linux 发行版(Ubuntu 22.04+/Debian 12)上的废弃 API 告警/潜在编译问题。

Constraints:
- 不引入新依赖
- 保持输出格式不变(64 位小写 hex)

Tasks:
- [x] `server/main.cpp:36-47` `sha256_hex`: 用 `EVP_Digest`/`EVP_MD_CTX` 替换直接调用 `SHA256()`
- [x] `server/CMakeLists.txt`: 确认链接 `-lcrypto` (pre-existing)

Implementation notes:
- `SHA256()` 在 OpenSSL 3.0 标记 deprecated,推荐 `EVP` 接口。
- 示例:`EVP_MD_CTX_new()` + `EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr)` + `EVP_DigestUpdate` + `EVP_DigestFinal_ex`。

Automated checks:
- 服务端编译无 `-Wdeprecated-declarations` 告警

UAT:
- [ ] 注册→登录密码校验正常

---

## Step 04 — 改进客户端连接超时语义 [Medium]

Goal:
- 让"登录卡住"(服务端因 Step 01 bug 或慢响应不回 LOGIN_OK)时客户端有明确反馈。

Constraints:
- 不破坏现有"已连接"正常流程

Tasks:
- [x] `client/src/login_dialog.cpp`: 区分「TCP 连接超时」与「应用层登录超时」;登录后启动独立 QTimer(如 8s),收到 LOGIN_OK/ERROR 停止,超时提示"登录超时,服务器未响应"
- [x] `client/src/chat_client.cpp:126`: `on_socket_error` 区分错误类型,日志输出 `socket->error()` 码 (enhanced + on_disconnect logs)

Implementation notes:
- 现状 `connect_timer_` 只覆盖 TCP 连接阶段(line 238 start, line 283 stop on connected),登录后无超时。

Automated checks:
- 客户端编译通过

UAT:
- [ ] 服务端不响应时,8s 后客户端提示"登录超时"而非无限等待

---

## Step template (copy/paste)

```md
## Step <NN> — <title>

Goal:
- <one-sentence outcome>

Constraints:
- <technical, sequencing, migration, rollback, or evidence constraint that must shape implementation>
- <must-preserve invariant, compatibility rule, or operating assumption>

Tasks:
- [ ] <boundary>: <exact change> so <artifact or behavior>; preserve <invariant or evidence requirement>
- [ ] <boundary>: <exact change> so <artifact or behavior>; keep <compatibility, migration, or rollback constraint>

Implementation notes:
- <file or symbol hints, interface or schema changes, ordering constraints, migration notes, rollback notes, or proof requirements>

Automated checks:
- <exact command>
- <exact command>

UAT:
- [ ] <manual or role-based verification>
- [ ] <end-to-end behavior or acceptance proof>
```
