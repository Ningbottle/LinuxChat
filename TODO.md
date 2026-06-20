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
| 客户端 UI | ✅ 完成/迁移中 | Qt6 Widgets + 水墨国风 QSS + --test-chat；QML 脚手架进行中 |
| 协议层 | ✅ 完成 | JSON-over-TCP, 4B BE prefix, EAGAIN 重试 |
| 测试 | ✅ 完成 | 108 个测试用例，全部通过 |
| 文档 | ✅ 完成 | CDD 工作流，PRD + Blueprint + Journal + INDEX 已初始化 |

---

## Step 07 — 连接竞态条件修复 ✅ DONE (2026-06-19)

**问题**：客户端连接服务端时，有时成功，有时失败（recv n=0）

**根因**（深度分析）：
1. **Windows socket descriptor 复用竞争**：`abort()` 调用 `closesocket()` 发送 FIN（异步），`connectToHost()` 立即创建新 socket 复用相同 fd，内核的残留 FIN 污染新连接
2. **双重信号发射**：`on_socket_error()` 和 `on_socket_disconnected()` 对同一事件发射两个不同信号
3. **服务端 FIN 数据丢失**：`recv_msgs()` 在 FIN 时直接返回 nullopt，丢弃已缓冲数据

**修复方案**（三轮迭代）：
- **第一轮**：每次连接创建新 QTcpSocket（`setup_socket()`），避免 fd 复用竞争
- **第二轮**：SO_LINGER {1,0} 强制 RST 替代 FIN
- **第三轮**：`deleteLater()` → 同步 `delete`；`disconnect_from_server()` 改用 `abort()` 替代 `disconnectFromHost()`

**根因**：`MainWindow::closeEvent()` 中 `disconnectFromHost()` 发送 FIN → Windows 端口复用 → 残留 FIN 污染新连接

**状态**：✅ 已修复，5 个智能体审查通过（2 个服务不可用）

**深度分析文档**：
- `docs/archive/connection-bug-analysis-2026-06-19.md` — 第一轮分析
- `docs/archive/post-fix-review-2026-06-19.md` — 修复后审查
- `docs/archive/final-fix-report-2026-06-19.md` — 最终修复报告
- `docs/archive/third-round-analysis-2026-06-19.md` — 第三轮分析（根因定位）

---

## Step 07b — 架构修复全量执行 ✅ DONE (2026-06-19)

**目标**：执行架构审计报告中的全部修复（P0 + HIGH + MEDIUM + LOW）

**修复清单**（5个P0 + 3个CRITICAL + 4个HIGH + 4个MEDIUM + 4个LOW = 20项）：

| # | 修复项 | 状态 |
|---|--------|------|
| P0-1 | 帧大小统一 256KB（客户端从16MB改为256KB） | ✅ |
| P0-2 | write_all 添加 5秒超时（防止线程池饥饿） | ✅ |
| P0-3 | now_stamp() 提取到 log_utils.h + localtime_r | ✅ |
| P0-4 | send_to_fd 释放锁后再发送（防死锁） | ✅ |
| P0-5 | generation counter 在 remove_client 中递增 | ✅ |
| C-1 | generation counter 检查是空操作（紧急修复） | ✅ |
| C-2 | running_ 改为 std::atomic<bool> | ✅ |
| C-3 | 客户端超大帧用 disconnect_from_server() 替代 disconnectFromHost() | ✅ |
| H-1 | send_to_fd TOCTOU 竞态 — 改用 shared_ptr + shutdown() | ✅ |
| H-2 | broadcast() TOCTOU 竞态 — 改用 shared_ptr | ✅ |
| H-3 | write_all 部分写入 — 超时时 shutdown(fd, SHUT_WR) | ✅ |
| H-4 | session.username 数据竞争 — 添加 username_mutex_ | ✅ |
| M-1 | 客户端添加 body_len==0 检查 | ✅ |
| M-2 | signal() → sigaction()（POSIX保证） | ✅ |
| M-3 | 服务端添加出站帧大小检查（防御深度） | ✅ |
| M-4 | now_stamp 优化为 snprintf 栈分配 | ✅ |
| L-1 | 清理热路径 debug cout | ✅ |
| L-2 | epoll_ctl 返回值检查 | ✅ |
| L-3 | 客户端 body_len 截断风险修复 | ✅ |

**关键架构改进**：
- `ClientSession` 析构函数关闭 fd（而非 `remove_client`），确保 in-flight I/O 完成后再释放
- `remove_client` 调用 `shutdown(fd, SHUT_WR)` 使 in-flight send 立即失败
- `send_to_fd`/`broadcast` 复制 `shared_ptr` 而非裸 fd，防止 fd 复用投递错误
- `username` 通过 mutex 保护的 getter/setter 访问，消除多线程数据竞争

**审查报告**：`docs/archive/p0-review-2026-06-19.md`（4智能体）

**TDD 测试**（tests/test_protocol.cpp 新增 8 个测试）：
- ZeroLengthBody_ReturnsNullopt — body_len==0 断开连接
- ThreeConsecutiveParseFailures_ReturnsNullopt — 3次解析失败断开
- TwoParseFailuresThenSuccess_Continues — 2次失败后成功继续
- PeerClosedWithBufferedData_ReturnsMessages — FIN后返回已缓冲数据
- PeerClosedNoData_ReturnsNullopt — 无数据+FIN返回nullopt
- DestructorClosesFd — 析构函数关闭fd
- UsernameThreadSafety — username mutex 保护
- GenerationCounter — generation 计数器

**编译验证**：✅ 客户端 Windows 编译通过（服务端为 Linux-only，无法在 Windows 编译）

---

## Step 08 — QML 迁移（进行中）

**目标**：从 Qt Widgets + QSS 迁移到 QML，获得更好的 UI 表现力

**架构规划**：
- QML 层：main.qml, LoginScreen.qml, ChatScreen.qml, components/
- C++ 层：Backend (Q_PROPERTY), MessageModel, UserModel
- 数据流：C++ Backend → QML ListView (model binding)

**实施阶段**（PRD: docs/specs/qml-migration-prd.md）：
1. ✅ 基础设施（Infrastructure）— ChatBackend, SessionModel, LoginController, Theme.qml
2. ✅ 登录界面（Login）— LoginDialog.qml, StackView, LoginBackground.qml
3. ✅ 聊天界面（Chat）— ChatWindow.qml, Sidebar, MessageBubble, ChatHeader, InputArea, TimeSeparator, SettingsPopup
4. ✅ 动效与皮肤（Animations & Skins）— 4 skins token 对齐 HTML, 动画系统, QSettings 持久化
5. ✅ 清理（Cleanup）— 删除 Widgets 文件, 移除 Qt6::Widgets, QML-only 架构

**状态**：Step 1-5 全部完成 ✅ QML 迁移完成

**Step 1 完成内容**（Infrastructure）：
- ✅ ChatBackend 门面：Q_PROPERTY + Q_INVOKABLE 包装 ChatClient
- ✅ MessageModel：QAbstractListModel（sender, content, timestamp, isSelf, messageType）
- ✅ UserModel：QAbstractListModel（username）
- ✅ SessionModel：QAbstractListModel（targetName, isRoom, unreadCount, lastMessage, lastTimestamp）
- ✅ LoginController：状态机（disconnected→connecting→authenticating→authenticated）
- ✅ Theme.qml 单例：4 套皮肤（Minimal/Dense/Motion/iMessage），15 colors + fonts + spacing + radius + bubble + avatar
- ✅ main.cpp：#ifdef HAS_QML 双路径

**Step 2 完成内容**（Login）：
- ✅ main.qml：ApplicationWindow + StackView 导航
- ✅ LoginDialog.qml：登录/注册界面（glassmorphism card）
- ✅ LoginBackground.qml：报纸纹理 + 地球仪水印
- ✅ StackView push/pop 由 LoginController.state 驱动

**Step 3 完成内容**（Chat）：
- ✅ ChatWindow.qml：主聊天窗口（TopBar + Sidebar + ChatArea）
- ✅ Sidebar.qml：当前用户 + 会话列表 + 在线用户列表
- ✅ MessageBubble.qml：3 种变体（self/other/system），入场动画
- ✅ ChatHeader.qml：会话名 + 连接状态 + 设置按钮
- ✅ InputArea.qml：TextArea + 发送按钮，Ctrl+Enter 发送
- ✅ TimeSeparator.qml：时间分隔线
- ✅ SettingsPopup.qml：退出登录 + 皮肤选择器
- ✅ SessionTabItem.qml：会话列表项（未读徽章）
- ✅ UserListItem.qml：用户列表项（头像 + 在线点）

**依赖**：需要安装 Qt6 Qml/Quick/QuickControls2 模块才能启用 QML UI
- 当前 Qt 安装：`C:\Qt\6.8.3\msvc2022_64`（缺少 Qml 模块）
- 安装方式：Qt Maintenance Tool → 添加 Qt6 Qml 组件

**Step 4 完成内容**（Animations & Skins）：
- ✅ Theme.qml token 对齐 4 个 HTML 设计稿（15 colors + fonts + spacing + radius + bubble + avatar）
- ✅ Minimal 皮肤：design-a-minimal.html（#FFFFFF bg, #8E8E93 accent, 4px radius）
- ✅ Dense 皮肤：design-b-dense.html（#F0F0F5 bg, #5865F2 accent, 10px radius）
- ✅ Motion 皮肤：design-c-motion.html（#E8E8EC bg, #4A6CF7 accent, glassmorphism）
- ✅ iMessage 皮肤：design-d-imessage.html（#FFFFFF bg, #007AFF accent, 18px radius）
- ✅ 消息气泡弹入动画（scale 0.8→1.0 + opacity 0→1, 200ms, OutBack easing）
- ✅ 登录界面弹簧入场（translateY 60→0, scale 0.9→1.0, 600ms, overshoot 1.5）
- ✅ 在线状态脉冲动画（opacity 1.0↔0.5 + scale 1.2↔0.8, 1.5s 循环）
- ✅ 会话切换过渡（opacity fade 200ms OutCubic）
- ✅ StackView push/pop 滑入/滑出过渡（300ms, slide + fade）
- ✅ QSettings 皮肤选择持久化（启动时自动恢复）
- ✅ SettingsPopup 皮肤选择器（4 个选项，实时切换）
- ✅ 动画组件：BounceAnimation.qml, SpringEntrance.qml, PulseAnimation.qml
- ✅ CMakeLists.txt 注册 20 个 QML 文件（含 3 个动画组件）

**Step 5 完成内容**（Cleanup）：
- ✅ 删除 7 个 Widgets 文件（~2300 行）
  - `include/main_window.h` + `src/main_window.cpp` (601 行)
  - `include/chat_view.h` + `src/chat_view.cpp` (289 行)
  - `include/login_dialog.h` + `src/login_dialog.cpp` (381 行)
  - `resources/style.qss` (1036 行)
- ✅ main.cpp 重写为 QML-only（移除 QApplication/QSS/Widgets 路径）
- ✅ CMakeLists.txt 移除 `Qt6::Widgets`，QML 模块改为 required
- ✅ resources.qrc 移除 style.qss 引用
- ✅ 移除所有 `#ifdef HAS_QML` 条件编译
- ✅ 验证：无残留代码引用已删除文件

**最终文件结构**：
```
client/
├── main.cpp                          # QML-only 入口
├── CMakeLists.txt                    # Qt6::Network + Qt6::Svg + Qt6::Qml + Qt6::Quick
├── include/
│   ├── chat_client.h                 # TCP 协议层
│   ├── chat_backend.h                # QML 门面
│   ├── message_model.h               # 消息 Model
│   ├── user_model.h                  # 用户 Model
│   ├── session_model.h               # 会话 Model
│   ├── login_controller.h            # 登录状态机
│   ├── font_manager.h                # 字体管理
│   └── backend.h                     # 旧门面（待清理）
├── src/ (8 个 .cpp)
├── qml/
│   ├── main.qml                      # StackView + QSettings
│   ├── views/ (2 个)
│   ├── components/ (10 个)
│   ├── animations/ (3 个)
│   └── styles/ (5 个)
└── resources/ (字体 + 图片)
```

---

## Step 09 — 前端现代化 Phase 1 ✅ DONE (2026-06-19)

**目标**：消除所有非 QSS 样式来源，为 QSS 全面重写扫清障碍

**设计方案**（多智能体头脑风暴）：
- Option A: 保留水墨国风 QSS 增量优化（得分 8.10）
- **Option B: Modern Slate + Indigo 全面重设计（得分 7.50，推荐）**
- Option C: QML 全量迁移（得分 5.50，成本过高）
- Option D: Widgets + QML 混合渲染（得分 5.65）

**推荐方案**：Option B — Dark Slate + Indigo 主题
- 设计令牌：bg=#0f172a, surface=#1e293b, accent=#6366f1
- 圆角：4/8/12px 三级体系
- 字体：Segoe UI → Inter → YaHei → LXGW (回退)

**Phase 1 完成内容**：
- ✅ MainWindow paintEvent testMode门控 — 跳过 wisteria SVG 绘制
- ✅ MainWindow globePanel testMode门控 — 跳过 globe 面板创建
- ✅ LoginDialog testMode参数 — 新增 testMode 构造函数参数
- ✅ LoginDialog paintEvent testMode门控 — 跳过 newspaper/globe 绘制
- ✅ main_window.cpp 内联 setStyleSheet 修复 — 迁移到 QSS objectName 选择器
- ✅ style.qss 新增 settingsTitle 规则 — 16px/600/#1a1a1a

**验证方式**：`--test-chat` 模式下无自定义绘制叠加，背景纯色由 QSS 控制

---

## Step 10 — Critical 架构修复 + QSS 全面重写 ✅ DONE (2026-06-19)

**目标**：修复 comprehensive-review 发现的 Critical 问题 + 完成前端 Phase 2

### Critical 架构修复（comprehensive-review 发现）

| # | 修复项 | 状态 |
|---|--------|------|
| C-1 | 并发 fd 写入帧交错 — ClientSession 添加 send_mutex_ | ✅ |
| C-2 | 登录 TOCTOU 竞态 — login_in_progress_ 预约机制 | ✅ |
| C-3 | 无盐 SHA-256 — hash_password/verify_password 加盐 | ✅ |

**修复详情**：
- **C-1**: `ClientSession` 新增 `send_mutex_`，`broadcast()` 和 `send_to_fd()` 在发送前锁定，防止并发帧交错
- **C-2**: `MessageRouter` 新增 `login_mutex_` + `login_in_progress_` 集合，在整个登录流程中预约用户名
- **C-3**: `hash_password()` 生成 16 字节随机盐，存储格式 `salt_hex:hash_hex`；`verify_password()` 支持旧格式向后兼容

### QSS 全面重写（Phase 2）

- ✅ style.qss 全面重写 — Dark Slate + Indigo 主题（~680行）
- ✅ 设计令牌：bg=#0f172a, surface=#1e293b, accent=#6366f1
- ✅ 圆角统一 8px（按钮、输入框、气泡、标签页）
- ✅ 字体栈：Segoe UI → Inter → YaHei → LXGW
- ✅ 完整交互状态（hover/focus/pressed/disabled/selected）
- ✅ 所有旧水墨国风颜色已替换

**验证方式**：`--test-chat` 模式下视觉检查深色主题效果

---

## Step 11 — 前端 Phase 3-5 完成 ✅ DONE (2026-06-19)

**目标**：完成前端现代化剩余阶段

### Phase 3: 交互状态完善 + 细节打磨

- ✅ style.qss 从 703 行扩展到 1037 行
- ✅ 登录对话框 error property — `set_status_error(bool)` 方法 + QSS `[error="true"]` 选择器
- ✅ 按钮完整交互状态 — hover/pressed/disabled/focus 覆盖所有按钮
- ✅ 滚动条 :pressed 状态
- ✅ 标签页 disabled/scroll 按钮样式
- ✅ Fusion 兼容性块 — QComboBox/QCheckBox/QRadioButton/QProgressBar/QGroupBox/QSlider/QSpinBox

### Phase 4: 文档更新 + 合约演进

- ✅ CONTRACT.md v1.2 — 设计令牌约束、交互状态规则、Fusion 兼容性
- ✅ README.md — Dark Slate + Indigo 设计令牌表
- ✅ ARCHITECTURE.md — 系统概览图、设计决策表

### Phase 5: 组件深化

- ✅ QCompleter popup QSS 已就绪
- ✅ 侧边栏折叠 — QPropertyAnimation 动画 + toggle 按钮
- ✅ 消息气泡动画 — QGraphicsOpacityEffect fade-in 动画

---

## Step 12 — 安全加固 + P0 修复 ✅ DONE (2026-06-19)

**目标**：修复第二次 comprehensive-review 发现的 P0 + HIGH 安全问题

### P0 架构修复

| # | 修复项 | 状态 |
|---|--------|------|
| P0-1 | 登录预约泄漏 — 断开连接时清理 login_in_progress_ | ✅ |
| P0-2 | find_fd 裸 fd 竞态 — 改用 find_session 返回 shared_ptr | ✅ |

**修复详情**：
- **P0-1**: `ClientSession` 新增 `pending_login_` 跟踪登录中的用户名；`MessageRouter` 新增 `cleanup_login_reservation()` 方法；断开连接处理器在 `handle_logout` 前清理预约
- **P0-2**: `EpollServer::find_fd()` 改为 `find_session()` 返回 `shared_ptr<ClientSession>`，防止 fd 复用竞态

### HIGH 安全修复

| # | 修复项 | 状态 |
|---|--------|------|
| H-1 | 密码比较时序攻击 — 使用 CRYPTO_memcmp | ✅ |
| H-2 | 输入长度验证 — 用户名32/密码128/消息4096 | ✅ |
| H-3 | 登录频率限制 — 5次失败后封禁5分钟 | ✅ |

**修复详情**：
- **H-1**: `verify_password()` 使用 `CRYPTO_memcmp()` 替代 `==` 运算符，防止时序攻击
- **H-2**: 新增 `MAX_USERNAME_LEN`/`MAX_PASSWORD_LEN`/`MAX_MESSAGE_LEN` 常量，所有处理器添加长度检查
- **H-3**: `MessageRouter` 新增 `LoginAttempt` 结构和 `login_attempts_` 映射；`is_rate_limited()`/`record_failed_login()`/`clear_login_attempts()` 方法；`handle_login` 集成频率限制

### P1 修复

| # | 修复项 | 状态 |
|---|--------|------|
| P1-1 | RAND_bytes 失败应为致命错误 | ✅ |
| P1-2 | recv_buf 无界增长 — 添加 1MB 限制 | ✅ |

**修复详情**：
- **P1-1**: `generate_salt()` 中 RAND_bytes 失败时抛出 `std::runtime_error`，不再回退到时间-based 盐
- **P1-2**: `Protocol::recv_msgs()` 添加 `MAX_RECV_BUF_SIZE` (1MB) 检查，超限时断开连接

### 新增功能

- `ClientSession` 新增 `ip_address` 字段，用于频率限制
- `EpollServer::handle_new_connection()` 存储客户端 IP 地址
- `MessageRouter` 新增输入验证常量和频率限制方法

---

## Step 13 — Open Design 浅色主题 + DESIGN.md ✅ DONE (2026-06-19)

**目标**：按 Open Design 规范创建 DESIGN.md，重写 QSS 为 Modern Gray 浅色主题

### 设计系统

- ✅ 创建 `DESIGN.md` — Open Design 9 段式设计规范
- ✅ 设计令牌：canvas=#F5F5F5, surface=#FFFFFF, accent=#3B82F6, text=#1F2937
- ✅ 字体：LXGW WenKai（霞鹜文楷）文艺优雅风格
- ✅ 完整组件规范：按钮、输入框、消息气泡、侧边栏、标签页等

### QSS 全面重写（浅色主题）

- ✅ style.qss 全面重写 — Modern Gray 浅色主题（~850行）
- ✅ 从 Dark Slate + Indigo 深色主题完全切换为浅色
- ✅ 所有组件样式更新：登录对话框、消息气泡、滚动条、右键菜单等
- ✅ Fusion 兼容性块更新

### 字体优化

- ✅ 主字体改为 LXGW WenKai（霞鹜文楷）
- ✅ 字体文件已嵌入 `resources/fonts/`
- ✅ 等宽字体保持 Cascadia Code/Consolas

**验证方式**：`--test-chat` 模式下视觉检查浅色主题效果

---

## Future Steps (Deferred)

- Friend system, blacklist, offline messages, file transfer
- HTTPS/WSS 升级
- 消息加密 (E2E)
- 移动端客户端
