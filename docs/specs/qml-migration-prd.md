# PRD — LinuxChat 客户端 QML 迁移

Status: Draft | Version: 0.1 | 创建日期: 2026-06-19

---

## 1. 概述

### 1.1 背景

LinuxChat 是一个 C/S 架构的网络即时通讯系统，服务端为 Linux epoll (C++)，客户端为 Windows Qt6。当前客户端 UI 层使用 **Qt Widgets + QSS**（`MainWindow` / `ChatView` / `LoginDialog` / `style.qss` 1036 行），存在以下问题：

- QSS 对动态状态（hover/focus/selected）的表达能力有限，难以实现现代聊天 UI 的交互动效
- 每个消息气泡是一个 `QWidget`（`chat_view.cpp:create_bubble` 构造 QLabel + QHBoxLayout），大量消息时内存和渲染开销大
- 主题切换需要替换整个 QSS 文件，无法做到运行时热切换
- 自定义绘制（`paintEvent` 中的 `drawNewspaperBackground` / `drawGlobe`）与样式逻辑耦合

项目已有 QML 迁移脚手架：`CMakeLists.txt` 中的 `HAS_QML` 宏守卫、`qml/main.qml` 占位文件、`Backend` / `MessageModel` / `UserModel` C++ 类。本次迁移将在此基础上完成从 Qt Widgets 到 QML/Qt Quick 的完整切换。

### 1.2 迁移目标

将客户端 UI 层从 Qt Widgets + QSS **增量迁移**到 QML/Qt Quick，同时满足：

- 迁移过程中 Widgets 和 QML 可通过 `HAS_QML` 宏共存，每步可独立编译运行
- `ChatClient`（TCP 协议层）**不做任何修改**
- 服务端**不做任何修改**
- `--test-chat` CLI 参数在迁移完成后仍然可用

### 1.3 已做出的关键决策

| # | 决策 | 理由 |
|---|------|------|
| 1 | 增量迁移，使用现有 `HAS_QML` 宏实现共存 | 每步可独立验证，降低回归风险 |
| 2 | `qmlRegisterSingletonType` 暴露 `ChatClient` 给 QML | QML 中直接调用协议层，无需额外桥接 |
| 3 | `ChatBackend` 门面 + 嵌套模型（`MessageListModel` / `SessionModel` / `UserListModel`）| 单一入口点，QML 绑定简洁 |
| 4 | QML `StackView` 管理 login → chat 导航 | 替代 `main.cpp` 中的 `while(true)` 循环 |
| 5 | 目录结构：`views/` + `components/` + `styles/` | 职责清晰，可扩展 |
| 6 | `Theme.qml` 单例 + 4 套皮肤预设（Minimal / Dense / Motion / iMessage）| 运行时热切换，用户可选 |
| 7 | 保留 `--test-chat` CLI 参数 | 开发迭代效率 |
| 8 | 迁移顺序：基础设施 → 登录 → 聊天 → 动效 → 清理 | 依赖链最短，每步可交付 |
| 9 | 服务端完全不动 | 范围控制 |

---

## 2. 目标与非目标

### 2.1 目标（Goals）

| ID | 目标 | 可衡量标准 |
|----|------|-----------|
| G1 | 完成 UI 层从 Qt Widgets 到 QML 的迁移 | 所有 Widgets UI 文件删除，QML 界面功能等价 |
| G2 | 支持 4 套主题皮肤运行时切换 | `Theme.qml` 中定义 Minimal / Dense / Motion / iMessage，QML 中一行代码切换 |
| G3 | 消息气泡使用 QML Delegate 而非独立 QWidget | `ListView` + `MessageBubble.qml` delegate，内存占用降低 |
| G4 | 保留 `--test-chat` 模式 | `./linuxchat_client --test-chat` 直接进入测试聊天界面 |
| G5 | 迁移过程中每步可独立编译运行 | 每个 Step 完成后 `cmake --build` 通过，功能可用 |
| G6 | 移除 `Qt6::Widgets` 最终依赖 | `CMakeLists.txt` 中不再 `find_package(Qt6 REQUIRED COMPONENTS Widgets)` |

### 2.2 非目标（Non-goals）

| ID | 非目标 | 说明 |
|----|--------|------|
| NG1 | 修改服务端 | 服务端代码、协议、数据库均不变 |
| NG2 | 修改 `ChatClient` 类 | `chat_client.h/cpp` 保持原样，仅通过 `Backend` 门面暴露给 QML |
| NG3 | 实现好友系统/黑名单 | 协议预留字段不实现 |
| NG4 | 实现文件/图片传输 | 本期纯文本消息 |
| NG5 | 多语言国际化 | 仅中文 UI，`qsTr()` 包裹但不实现翻译文件 |
| NG6 | 移动端适配 | 仅桌面端（960x640 固定窗口） |

---

## 3. 架构图

### 3.1 迁移前（当前架构）

```
main.cpp
  ├── QApplication + style.qss 加载
  ├── FontManager::instance().loadFonts()
  ├── --test-chat 分支 → MainWindow(testMode=true) + populateTestData()
  └── while(true) 循环:
       ├── LoginDialog.exec()          ← QDialog, 阻塞式
       │    ├── host/port 输入
       │    ├── connect/login/register 按钮
       │    └── ChatClient 信号 → slot
       └── MainWindow(client, username) ← QMainWindow
            ├── TopBar (logo + status + settings)
            ├── Sidebar (QFrame, 240px)
            │    ├── currentUser label
            │    └── userList (QListWidget)
            └── ChatTabs (QTabWidget)
                 ├── Tab 0: ChatView (公共聊天室)
                 └── Tab N: ChatView (私聊, 按需创建)
                      ├── QScrollArea + QVBoxLayout (气泡 widgets)
                      │    ├── bubbleSelf (QWidget#bubbleSelf)
                      │    ├── bubbleOther (QWidget#bubbleOther)
                      │    └── systemNotify (QLabel#systemNotify)
                      └── InputArea (QTextEdit + QPushButton#sendBtn)

ChatClient (QObject, 纯信号驱动, 不修改)
  ├── QTcpSocket → JSON-over-TCP (4字节大端长度前缀)
  └── signals: connected/disconnected/connection_error
               login_ok/error_received
               broadcast_received/private_received
               user_list_updated/history_received/notify_received
```

### 3.2 迁移后（目标架构）

```
main.cpp
  ├── QGuiApplication (非 QApplication)
  ├── FontManager::instance().loadFonts()
  ├── qmlRegisterSingletonType<ChatClient>("LinuxChat", 1, 0, "ChatClient", ...)
  ├── qmlRegisterSingletonType<ChatBackend>("LinuxChat", 1, 0, "ChatBackend", ...)
  ├── qmlRegisterSingletonType<Theme>("LinuxChat", 1, 0, "Theme", ...)
  └── QQmlApplicationEngine → main.qml

main.qml (ApplicationWindow + StackView)
  ├── StackView { id: stack }
  │    ├── initialItem: LoginDialog.qml
  │    │    ├── LoginController (QObject 状态机)
  │    │    │    States: disconnected → connecting → authenticating → authenticated
  │    │    ├── host/port 输入
  │    │    ├── connect/login/register 按钮
  │    │    └── 状态反馈 (status label)
  │    └── ChatWindow.qml (login 成功后 push)
  │         ├── TopBar (logo + status + settings)
  │         ├── Sidebar.qml (search + SessionModel delegate + user card)
  │         └── ChatArea
  │              ├── ChatHeader.qml (会话名 + 在线状态)
  │              ├── ListView + MessageBubble.qml delegate
  │              │    ├── bubbleSelf variant
  │              │    ├── bubbleOther variant
  │              │    └── systemNotify variant
  │              └── InputArea.qml (TextArea + send button)

ChatBackend (QObject 门面, qmlRegisterSingletonType)
  ├── 持有 ChatClient* (不拥有, main.cpp 传入)
  ├── Q_PROPERTY: connectionStatus, currentUser
  ├── Q_INVOKABLE: connectToServer, login, registerUser, sendMessage, sendPrivate, logout
  ├── MessageListModel* roomMessages     (QAbstractListModel)
  ├── QMap<QString, MessageListModel*> privateMessages
  ├── UserModel* onlineUsers             (QAbstractListModel)
  └── SessionModel* sessions             (QAbstractListModel)

Theme.qml (pragma Singleton)
  ├── Q_PROPERTY: currentSkin (enum: Minimal/Dense/Motion/iMessage)
  ├── Q_PROPERTY: colors (QVariantMap, 当前皮肤的颜色表)
  ├── Q_PROPERTY: fonts (QVariantMap, 当前皮肤的字体配置)
  ├── Q_PROPERTY: spacing (QVariantMap, 当前皮肤的间距配置)
  └── Q_INVOKABLE: setSkin(skinName)

ChatClient (QObject, 完全不修改)
  └── 同迁移前

LoginController (QObject, 新增)
  ├── Q_PROPERTY: state (disconnected/connecting/authenticating/authenticated)
  ├── Q_PROPERTY: statusText, isError, isLoading
  ├── Q_INVOKABLE: connectToServer, login, registerUser
  └── 内部: connectTimer, loginTimer (复用 LoginDialog 的超时逻辑)
```

### 3.3 模块依赖关系

```
QML 层:
  main.qml ──imports──→ LoginDialog.qml, ChatWindow.qml, Theme.qml
  ChatWindow.qml ──imports──→ Sidebar.qml, ChatHeader.qml, InputArea.qml, MessageBubble.qml
  所有 QML ──uses──→ ChatBackend (singleton), ChatClient (singleton), Theme (singleton)

C++ 层:
  main.cpp ──creates──→ ChatClient, ChatBackend, LoginController, Theme
  ChatBackend ──wraps──→ ChatClient (指针, 不拥有)
  ChatBackend ──owns──→ MessageListModel, UserModel, SessionModel
  LoginController ──uses──→ ChatClient (指针, 不拥有)
  MessageModel/UserModel ──extends──→ QAbstractListModel
```

---

## 4. 分步实施计划

### Step 1: 基础设施（Infrastructure）

**目标**: 建立 QML 运行所需的所有 C++ 后端类和主题系统，但不替换任何 UI。

**描述**:

本步骤创建 QML 层的"地基"。`ChatBackend` 门面类封装 `ChatClient` 的信号/槽为 QML 可绑定的属性和方法。三个 Model 类（`MessageListModel`、`UserListModel`、`SessionModel`）将数据暴露为 `QAbstractListModel`，供 QML `ListView` 委托使用。`Theme.qml` 单例定义 4 套皮肤的颜色/字体/间距 token。`main.cpp` 注册所有单例类型。

**需要创建的文件**:

| 文件 | 类型 | 说明 |
|------|------|------|
| `client/include/chat_backend.h` | C++ Header | `ChatBackend` QObject 门面，持有 `ChatClient*`，拥有 `MessageListModel` / `UserListModel` / `SessionModel` |
| `client/src/chat_backend.cpp` | C++ Source | 实现：转发 `ChatClient` 信号到 Model，提供 `Q_INVOKABLE` 方法 |
| `client/include/session_model.h` | C++ Header | `SessionModel : QAbstractListModel`，角色：`targetName` / `isRoom` / `unreadCount` / `lastMessage` / `lastTimestamp` |
| `client/src/session_model.cpp` | C++ Source | 实现 Model 逻辑 |
| `client/include/login_controller.h` | C++ Header | `LoginController` QObject，状态机：`disconnected` → `connecting` → `authenticating` → `authenticated`，包含超时定时器 |
| `client/src/login_controller.cpp` | C++ Source | 复用 `login_dialog.cpp` 中的连接/登录超时逻辑（262-365 行） |
| `client/qml/styles/Theme.qml` | QML Singleton | `pragma Singleton`，4 套皮肤预设（Minimal / Dense / Motion / iMessage），`Q_PROPERTY` 暴露 `colors` / `fonts` / `spacing` |
| `client/qml/styles/Minimal.qml` | QML | Minimal 皮肤的颜色/字体/间距定义 |
| `client/qml/styles/Dense.qml` | QML | Dense 皮肤定义 |
| `client/qml/styles/Motion.qml` | QML | Motion 皮肤定义 |
| `client/qml/styles/iMessage.qml` | QML | iMessage 皮肤定义 |

**需要修改的文件**:

| 文件 | 修改内容 |
|------|---------|
| `client/main.cpp` | 在 `#ifdef HAS_QML` 分支中：创建 `ChatBackend` 和 `LoginController` 实例，调用 `qmlRegisterSingletonType` 注册 `ChatClient` / `ChatBackend` / `Theme`，创建 `QQmlApplicationEngine` 加载 `main.qml`。保留 Widgets 路径作为 else 分支。 |
| `client/CMakeLists.txt` | 在 `if(HAS_QML)` 块中：`qt_add_qml_module` 添加新 QML 文件（`qml/main.qml` + `qml/styles/Theme.qml` + 4 个皮肤文件）。添加新 C++ 源文件到 `SOURCES`。 |
| `client/include/message_model.h` | 扩展角色：添加 `MessageTypeRole`（`normal` / `system` / `timeSeparator`），添加 `loadFromJsonArray(QJsonArray, myUsername)` 方法 |
| `client/src/message_model.cpp` | 实现新角色和 `loadFromJsonArray` |
| `client/include/user_model.h` | 无需修改（已满足需求） |
| `client/include/backend.h` | **重命名为** `chat_backend.h`，扩展：添加 `sendPrivate()` / `sendHistoryReq()` / `disconnectFromServer()` Q_INVOKABLE，添加嵌套 Model 指针属性 |
| `client/src/backend.cpp` | **重命名为** `chat_backend.cpp`，实现扩展逻辑 |

**验收标准**:

- [ ] `cmake --build` 通过，HAS_QML 路径编译成功
- [ ] `main.cpp` 中 `#ifdef HAS_QML` 分支正确注册 3 个单例
- [ ] `ChatBackend` 可在 QML 中 `import LinuxChat 1.0` 后访问
- [ ] `MessageListModel` 的 `roleNames()` 返回 `sender` / `content` / `timestamp` / `isSelf` / `messageType`
- [ ] `UserListModel` 的 `roleNames()` 返回 `username`
- [ ] `SessionModel` 的 `roleNames()` 返回 `targetName` / `isRoom` / `unreadCount` / `lastMessage` / `lastTimestamp`
- [ ] `Theme.qml` 可在 QML 中访问 `Theme.colors.accent` 等属性
- [ ] 4 套皮肤的颜色 token 覆盖：`canvas` / `surface` / `text` / `muted` / `accent` / `success` / `danger` / `border` / `radius`
- [ ] `LoginController` 状态机：`disconnected` → 点击连接 → `connecting` → TCP 成功 → `authenticating` → LOGIN_OK → `authenticated`
- [ ] `LoginController` 超时：连接 10s / 登录 8s，超时后回到 `disconnected` 并设置错误文本
- [ ] 现有 Widgets 路径（`#else` 分支）不受影响，`--test-chat` 仍走 Widgets

---

### Step 2: 登录界面（Login）

**目标**: 用 QML 实现登录界面，替代 `LoginDialog`（QDialog），建立 `StackView` 导航框架。

**描述**:

本步骤创建 `main.qml` 的 `StackView` 框架和 `LoginDialog.qml` 登录界面。`LoginDialog.qml` 复用 `LoginController` 管理连接/登录状态，UI 布局参照当前 `login_dialog.cpp` 的 `setup_ui()`（104-208 行）：标题 "LinuxChat"、服务器地址+端口+连接按钮、用户名+密码输入、登录+注册按钮、状态标签。背景保留报纸纹理风格（通过 QML Canvas 或 Image 实现）。

登录成功后，`StackView` push `ChatWindow.qml`（此时为占位，Step 3 实现）。断开连接时 pop 回登录界面。

**需要创建的文件**:

| 文件 | 类型 | 说明 |
|------|------|------|
| `client/qml/main.qml` | QML（替换现有占位） | `ApplicationWindow` + `StackView`，初始项为 `LoginDialog.qml`，监听 `LoginController.state` 变化执行 push/pop |
| `client/qml/views/LoginDialog.qml` | QML | 登录界面，参照 `login_dialog.cpp:setup_ui()` 布局，使用 `Theme.colors` 着色 |
| `client/qml/views/ChatWindow.qml` | QML（占位） | 空壳 `Page`，仅显示 "Chat - Coming in Step 3"，用于验证 StackView push |
| `client/qml/components/LoginBackground.qml` | QML | 报纸纹理 + 地球仪 SVG 水印背景组件，替代 `login_dialog.cpp:drawNewspaperBackground` + `drawGlobe` |

**需要修改的文件**:

| 文件 | 修改内容 |
|------|---------|
| `client/main.cpp` | `#ifdef HAS_QML` 分支：不再创建 `LoginDialog` / `MainWindow`，改为 `engine.load("qrc:/qml/main.qml")`。保留 `--test-chat` 分支但改为加载 QML 测试模式。 |
| `client/CMakeLists.txt` | `qt_add_qml_module` 的 `QML_FILES` 列表添加新文件 |

**验收标准**:

- [ ] `--test-chat` 不走此路径（仍走 Widgets 或 QML 测试模式），正常启动
- [ ] 正常模式：启动后显示 QML 登录界面，布局与当前 Widgets 版本视觉一致
- [ ] 输入服务器地址和端口，点击"连接"：`LoginController.state` 从 `disconnected` 变为 `connecting`，按钮禁用，状态文本显示"正在连接..."
- [ ] TCP 连接成功：`state` 变为 `authenticating`，登录/注册按钮启用
- [ ] 输入用户名/密码，点击"登录"：发送 `LOGIN` 协议消息
- [ ] 收到 `LOGIN_OK`：`state` 变为 `authenticated`，StackView push `ChatWindow.qml`
- [ ] 连接失败/超时：`state` 回到 `disconnected`，显示错误信息
- [ ] 点击"注册"：发送 `REGISTER` 协议消息，成功后等同登录
- [ ] 报纸纹理背景 + 地球仪水印正确渲染
- [ ] Enter 键触发登录（密码框回车）

---

### Step 3: 聊天界面（Chat）

**目标**: 用 QML 实现完整的聊天界面，替代 `MainWindow` + `ChatView`。

**描述**:

本步骤是迁移的核心工作量。创建 `ChatWindow.qml`（主聊天窗口）、`Sidebar.qml`（侧边栏）、`MessageBubble.qml`（消息气泡）、`ChatHeader.qml`（聊天头部）、`InputArea.qml`（输入区域）。

`ChatWindow.qml` 参照 `main_window.cpp:setup_ui()`（52-202 行）的布局：顶部栏（标题 + 连接状态 + 设置按钮）、左侧边栏（当前用户 + 在线用户列表）、右侧聊天区（Tab 或会话列表 + 消息区域 + 输入框）。

消息显示使用 `ListView` + `MessageBubble.qml` delegate，替代当前 `ChatView` 中每个消息创建独立 `QWidget` 的方式。`ListView` 的 model 绑定到 `ChatBackend.roomMessages`（公共聊天室）或 `ChatBackend.privateMessages[target]`（私聊）。

侧边栏的在线用户列表绑定到 `ChatBackend.onlineUsers`（`UserListModel`）。双击用户打开私聊会话。

**需要创建的文件**:

| 文件 | 类型 | 说明 |
|------|------|------|
| `client/qml/views/ChatWindow.qml` | QML（替换占位） | 主聊天窗口：TopBar + Sidebar + ChatArea，参照 `main_window.cpp` 布局 |
| `client/qml/components/Sidebar.qml` | QML | 侧边栏：当前用户信息 + 在线用户列表（`ListView` + `UserListModel`），参照 `main_window.cpp:130-163` |
| `client/qml/components/MessageBubble.qml` | QML | 消息气泡组件，3 种变体（self / other / system），参照 `chat_view.cpp:create_bubble` / `create_system_bubble` |
| `client/qml/components/ChatHeader.qml` | QML | 聊天头部：会话名称 + 在线状态指示器，参照 `main_window.cpp` topBar 中的 chat 部分 |
| `client/qml/components/InputArea.qml` | QML | 输入区域：`TextArea` + 发送按钮，参照 `chat_view.cpp:43-75`，支持 Ctrl+Enter 发送 |
| `client/qml/components/TimeSeparator.qml` | QML | 时间分隔线组件，参照 `chat_view.cpp:create_time_separator` |
| `client/qml/components/SettingsPopup.qml` | QML | 设置弹窗：退出登录 + 主题切换，参照 `main_window.cpp:on_settings_clicked` |

**需要修改的文件**:

| 文件 | 修改内容 |
|------|---------|
| `client/include/chat_backend.h` | 添加 `Q_INVOKABLE void sendPrivate(const QString& to, const QString& content)` / `Q_INVOKABLE void requestHistory(const QString& target)` / `Q_INVOKABLE void disconnectFromServer()` |
| `client/src/chat_backend.cpp` | 实现新方法，连接 `ChatClient::private_received` 到 `privateMessages` Model，连接 `ChatClient::history_received` 到对应 Model 的 `loadFromJsonArray` |
| `client/main.cpp` | `--test-chat` 分支：改为加载 QML 测试模式，调用 `ChatBackend::populateTestData()` |
| `client/include/chat_backend.h` | 添加 `Q_INVOKABLE void populateTestData()` 用于测试模式 |

**验收标准**:

- [ ] 登录成功后显示完整的聊天界面，布局与当前 Widgets 版本功能等价
- [ ] 公共聊天室消息显示：`ListView` + `MessageBubble` delegate，气泡样式区分 self/other/system
- [ ] 发送消息：输入文本 → 点击发送 → `ChatBackend.sendMessage()` → `ChatClient.send_broadcast()` → 收到广播 → 气泡显示
- [ ] 私聊：双击侧边栏用户 → 切换到私聊会话 → `ChatBackend.sendPrivate()` → 收到私信 → 气泡显示
- [ ] 历史消息：登录后自动请求 `__room__` 历史，消息加载到 Model
- [ ] 在线用户列表：`ChatClient::user_list_updated` → `UserListModel` 更新 → `ListView` 刷新
- [ ] 系统通知：`ChatClient::notify_received` → 系统气泡显示
- [ ] 未读消息标记：非当前会话收到消息时显示未读计数
- [ ] 侧边栏折叠/展开动画（200ms，参照 `main_window.cpp:on_sidebar_toggle`）
- [ ] 设置弹窗：显示"退出登录"按钮，点击后 `StackView` pop 回登录界面
- [ ] 断开连接：`ChatClient::disconnected` → 显示系统消息 → 返回登录界面
- [ ] `--test-chat` 模式：直接进入聊天界面，填充测试数据，发送消息本地回显
- [ ] 窗口标题显示 "LinuxChat — {username}"
- [ ] 消息气泡最大宽度 400px，头像 28px 圆形

---

### Step 4: 动效与皮肤（Animations & Skins）

**目标**: 实现 4 套主题皮肤的完整视觉效果和交互动效。

**描述**:

本步骤在 Step 3 的基础上添加视觉打磨。`Theme.qml` 中的 4 套皮肤需要完整的颜色/字体/间距/圆角/阴影 token，并在所有组件中通过 `Theme.colors.xxx` 绑定。

动效包括：
- 登录界面入场动画（弹簧物理，参照 SpringAnimation）
- 消息发送气泡弹入动画（bounce，参照 `main_window.cpp:animate_message_appear` 的 fade-in 改为 bounce）
- 会话切换过渡动画（StackView 的 push/pop transition）
- 侧边栏折叠/展开（PropertyAnimation on width）
- 在线状态脉冲动画（OpacityAnimator on onlineDot）

**4 套皮肤定义**:

| 皮肤 | 色彩基调 | 字体 | 间距 | 特点 |
|------|---------|------|------|------|
| Minimal | 灰白 + 蓝色强调（当前 style.qss 的配色） | LXGW WenKai 14px | 标准 8/12/16px | 干净、专业、当前默认 |
| Dense | 深灰 + 橙色强调 | 系统字体 12px | 紧凑 4/8/12px | 信息密度高，适合多消息场景 |
| Motion | 暗黑 + 紫色强调 | LXGW WenKai 15px | 宽松 12/16/24px | 全部动效开启，过渡流畅 |
| iMessage | 白色 + 绿色气泡 | SF Pro (fallback LXGW) 15px | Apple 标准 8/16px | 模拟 iOS iMessage 风格 |

**需要创建的文件**:

| 文件 | 类型 | 说明 |
|------|------|------|
| `client/qml/styles/Minimal.qml` | QML | Minimal 皮肤完整 token 定义（颜色/字体/间距/圆角/阴影） |
| `client/qml/styles/Dense.qml` | QML | Dense 皮肤定义 |
| `client/qml/styles/Motion.qml` | QML | Motion 皮肤定义 |
| `client/qml/styles/iMessage.qml` | QML | iMessage 皮肤定义 |
| `client/qml/animations/BounceAnimation.qml` | QML | 消息气泡弹入动画组件 |
| `client/qml/animations/SpringEntrance.qml` | QML | 登录界面弹簧入场动画组件 |
| `client/qml/animations/PulseAnimation.qml` | QML | 在线状态脉冲动画组件 |

**需要修改的文件**:

| 文件 | 修改内容 |
|------|---------|
| `client/qml/styles/Theme.qml` | 完善 4 套皮肤的完整 token 映射，添加 `Q_INVOKABLE void setSkin(string name)` |
| `client/qml/views/LoginDialog.qml` | 添加 `SpringEntrance` 入场动画 |
| `client/qml/components/MessageBubble.qml` | 添加 `BounceAnimation`（新消息弹入） |
| `client/qml/views/ChatWindow.qml` | 添加会话切换过渡动画，侧边栏折叠 `PropertyAnimation` |
| `client/qml/components/Sidebar.qml` | 添加在线状态 `PulseAnimation` |
| `client/qml/components/SettingsPopup.qml` | 添加皮肤选择器（4 个选项），绑定 `Theme.setSkin()` |

**验收标准**:

- [ ] 4 套皮肤均可通过设置弹窗切换，切换后所有组件颜色/字体/间距即时更新
- [ ] Minimal 皮肤视觉效果与当前 style.qss 一致（颜色 token 对齐）
- [ ] Dense 皮肤：字号 12px，间距紧凑，消息密度更高
- [ ] Motion 皮肤：暗黑背景，所有过渡动画开启，弹簧物理曲线
- [ ] iMessage 皮肤：白色背景，绿色气泡（self），灰色气泡（other）
- [ ] 登录界面入场：元素从下方滑入 + 弹簧回弹（SpringAnimation, damping: 0.5）
- [ ] 消息发送气泡：从底部弹入（scale 0.8 → 1.0 + opacity 0 → 1, 200ms）
- [ ] 会话切换：StackView push/pop 有滑入/滑出过渡（300ms, EaseInOut）
- [ ] 侧边栏折叠：宽度 240 → 0，200ms InOutCubic（与当前 `main_window.cpp:437-453` 一致）
- [ ] 在线状态脉冲：绿点 opacity 1.0 ↔ 0.5，1.5s 循环
- [ ] 皮肤选择持久化：`QSettings` 存储用户选择，下次启动自动应用

---

### Step 5: 清理（Cleanup）

**目标**: 删除所有 Qt Widgets UI 文件，移除 `Qt6::Widgets` 依赖。

**描述**:

本步骤在所有功能通过 QML 实现后，删除旧的 Widgets UI 文件和 QSS 样式表。更新 `CMakeLists.txt` 移除 `Qt6::Widgets` 依赖（保留 `Qt6::Network` 和 `Qt6::Svg`）。清理 `main.cpp` 中的 `#ifdef HAS_QML` 条件编译，只保留 QML 路径。

**需要删除的文件**:

| 文件 | 说明 |
|------|------|
| `client/include/main_window.h` | MainWindow 头文件 |
| `client/src/main_window.cpp` | MainWindow 实现（601 行） |
| `client/include/chat_view.h` | ChatView 头文件 |
| `client/src/chat_view.cpp` | ChatView 实现（289 行） |
| `client/include/login_dialog.h` | LoginDialog 头文件 |
| `client/src/login_dialog.cpp` | LoginDialog 实现（381 行） |
| `client/resources/style.qss` | QSS 样式表（1036 行） |

**需要修改的文件**:

| 文件 | 修改内容 |
|------|---------|
| `client/CMakeLists.txt` | `find_package(Qt6 REQUIRED COMPONENTS Widgets Network Svg)` → `find_package(Qt6 REQUIRED COMPONENTS Network Svg)`。移除 `Qt6::Widgets` 链接。移除 `HAS_QML` 条件编译，QML 模块变为必须。 |
| `client/main.cpp` | 移除所有 `#ifdef HAS_QML` / `#else` 条件编译。移除 `#include <QApplication>` 改为 `#include <QGuiApplication>`。移除 QSS 加载代码（36-44 行）。移除 Widgets 路径的 LoginDialog/MainWindow 创建代码。 |
| `client/resources/resources.qrc` | 移除 `style.qss` 资源引用 |

**验收标准**:

- [ ] 所有 7 个 Widgets 文件已删除
- [ ] `cmake --build` 通过，无 `Qt6::Widgets` 依赖
- [ ] `linuxchat_client.exe` 正常启动，显示 QML 登录界面
- [ ] 完整流程可用：登录 → 聊天 → 私聊 → 断开 → 重新登录
- [ ] `--test-chat` 模式正常工作
- [ ] 无编译警告（特别是 `QT_DEPRECATED_WARNINGS` 相关）
- [ ] `style.qss` 从 `resources.qrc` 中移除，资源文件不包含已删除文件

---

## 5. 风险与缓解

| # | 风险 | 影响 | 概率 | 缓解措施 |
|---|------|------|------|---------|
| R1 | QML `ListView` 大量消息时性能不如 Widgets `QScrollArea` | 消息多时卡顿 | 中 | 使用 `ListView` 的 `cacheBuffer` 和 `displayMarginBeginning/End`；`MessageModel` 实现增量插入而非全量重置；必要时限制显示消息数量（最近 500 条） |
| R2 | `qmlRegisterSingletonType` 的 `ChatClient` 单例生命周期管理 | QML 引擎销毁时 `ChatClient` 可能被提前释放 | 低 | `ChatClient` 由 `main.cpp` 栈分配，生命周期覆盖整个应用；单例注册时传入 `qmlEngine` 的 parent |
| R3 | 主题切换时 QML 组件未正确响应 `Theme.colors` 变化 | 切换皮肤后部分组件颜色未更新 | 中 | 所有组件使用 `Binding` 而非一次性赋值；`Theme.colors` 返回 `QVariantMap`，属性变化时发射 `changed` 信号 |
| R4 | `--test-chat` 模式在 QML 中的实现比 Widgets 复杂 | 测试模式功能缺失或行为不一致 | 低 | `ChatBackend::populateTestData()` 直接操作 Model，QML 端无需特殊处理；`main.cpp` 中通过设置 `ChatBackend.testMode = true` 标志 |
| R5 | `FontManager` 是 C++ 单例，QML 中需要访问字体族名称 | QML 中无法直接使用 `FontManager::instance()` | 低 | 将 `FontManager` 也注册为 QML 单例，或在 `Theme.qml` 的字体配置中硬编码字体族名称（`LXGW WenKai` / `Microsoft YaHei UI`） |
| R6 | 迁移过程中 Widgets 和 QML 路径分叉导致回归 | 某步修改破坏了另一条路径 | 中 | 每步完成后运行完整的手动测试清单；CI 中同时构建 HAS_QML=ON 和 HAS_QML=OFF 两个配置 |
| R7 | `ChatBackend` 门面类过度封装，信号转发引入延迟 | 消息显示延迟 | 低 | `ChatBackend` 仅做信号转发和 Model 更新，不引入额外队列；直接 `connect` 而非通过 lambda 二次分发 |

---

## 6. 成功指标

| # | 指标 | 目标值 | 测量方式 |
|---|------|--------|---------|
| M1 | 功能等价性 | 100% — 所有 Widgets 版本的功能在 QML 版本中可用 | 手动测试清单（登录/注册/公聊/私聊/历史/用户列表/断开/重连/test-chat） |
| M2 | 代码行数变化 | 净减少 — 删除 ~2300 行 Widgets 代码（main_window 600 + chat_view 290 + login_dialog 380 + style.qss 1036），新增 QML 代码预估 ~1500 行 | `git diff --stat` |
| M3 | 编译时间 | 不显著增加（< 10%） | `cmake --build` 计时对比 |
| M4 | 启动时间 | 不显著增加（< 200ms） | 从启动到登录界面显示的耗时 |
| M5 | 消息渲染性能 | 500 条消息滚动流畅（60fps） | `ListView` 滚动时的帧率监控 |
| M6 | 主题切换延迟 | < 100ms | 从点击皮肤选项到界面完全更新的耗时 |
| M7 | 二进制大小 | 不显著增加（< 2MB） | `linuxchat_client.exe` 文件大小对比 |

---

## 7. 目录结构（迁移完成后）

```
client/
├── main.cpp                          # 入口, QML 引擎初始化
├── CMakeLists.txt                    # Qt6::Network + Qt6::Qml + Qt6::Quick + Qt6::QuickControls2
├── include/
│   ├── chat_client.h                 # [不变] TCP 协议层
│   ├── chat_backend.h                # [新增] QML 门面
│   ├── message_model.h               # [修改] 消息 Model
│   ├── user_model.h                  # [不变] 用户 Model
│   ├── session_model.h               # [新增] 会话 Model
│   ├── login_controller.h            # [新增] 登录状态机
│   └── font_manager.h                # [不变] 字体管理
├── src/
│   ├── chat_client.cpp               # [不变]
│   ├── chat_backend.cpp              # [新增]
│   ├── message_model.cpp             # [修改]
│   ├── user_model.cpp                # [不变]
│   ├── session_model.cpp             # [新增]
│   ├── login_controller.cpp          # [新增]
│   └── font_manager.cpp              # [不变]
├── qml/
│   ├── main.qml                      # StackView 导航框架
│   ├── views/
│   │   ├── LoginDialog.qml           # 登录界面
│   │   └── ChatWindow.qml            # 聊天主窗口
│   ├── components/
│   │   ├── Sidebar.qml               # 侧边栏
│   │   ├── MessageBubble.qml         # 消息气泡 (3 变体)
│   │   ├── ChatHeader.qml            # 聊天头部
│   │   ├── InputArea.qml             # 输入区域
│   │   ├── TimeSeparator.qml         # 时间分隔线
│   │   ├── LoginBackground.qml       # 登录背景
│   │   └── SettingsPopup.qml         # 设置弹窗
│   ├── styles/
│   │   ├── Theme.qml                 # 主题单例 (pragma Singleton)
│   │   ├── Minimal.qml               # Minimal 皮肤
│   │   ├── Dense.qml                 # Dense 皮肤
│   │   ├── Motion.qml                # Motion 皮肤
│   │   └── iMessage.qml              # iMessage 皮肤
│   └── animations/
│       ├── BounceAnimation.qml       # 气泡弹入
│       ├── SpringEntrance.qml        # 弹簧入场
│       └── PulseAnimation.qml        # 脉冲动画
└── resources/
    ├── fonts/                         # [不变]
    ├── images/                        # [不变]
    └── resources.qrc                  # [修改] 移除 style.qss
```

---

## 8. 附录

### 8.1 ChatClient 信号到 ChatBackend 的映射

| ChatClient Signal | ChatBackend 处理 | Model 更新 |
|---|---|---|
| `connected()` | `connectionStatus = "Connected"` | — |
| `disconnected()` | `connectionStatus = "Disconnected"` | — |
| `connection_error(QString)` | `connectionStatus = "Error: ..."` | — |
| `login_ok(QString)` | `currentUser = username` | — |
| `error_received(QString, QString)` | 转发到 QML | — |
| `broadcast_received(from, content, ts)` | 格式化时间戳 | `roomMessages->addMessage(...)` |
| `private_received(from, to, content, ts)` | 确定会话 target | `privateMessages[target]->addMessage(...)` |
| `user_list_updated(QStringList)` | — | `onlineUsers->setUsers(users)` |
| `history_received(target, QJsonArray)` | — | `roomMessages->loadFromJsonArray(...)` 或 `privateMessages[target]->loadFromJsonArray(...)` |
| `notify_received(QString)` | — | `roomMessages->addMessage("", content, ts, false, "system")` |

### 8.2 LoginController 状态转换表

| 当前状态 | 事件 | 下一状态 | 副作用 |
|---------|------|---------|--------|
| `disconnected` | `connectToServer()` | `connecting` | 启动 10s 连接定时器 |
| `connecting` | `ChatClient::connected` | `authenticating` | 停止连接定时器，启用登录按钮 |
| `connecting` | `ChatClient::connection_error` | `disconnected` | 显示错误信息 |
| `connecting` | 连接定时器超时 | `disconnected` | 强制断开，显示超时错误 |
| `authenticating` | `login()` / `registerUser()` | `authenticating` | 发送协议消息，启动 8s 登录定时器 |
| `authenticating` | `ChatClient::login_ok` | `authenticated` | 停止定时器 |
| `authenticating` | `ChatClient::error_received` | `authenticating` | 显示错误，停止定时器，允许重试 |
| `authenticating` | 登录定时器超时 | `authenticating` | 显示超时错误，允许重试 |
| `authenticated` | — | — | StackView push ChatWindow |
| 任意 | `ChatClient::disconnected` | `disconnected` | 显示断开信息，StackView pop 回登录 |

### 8.3 Theme.qml Token 清单

```qml
// 所有皮肤必须实现以下 token:
Theme.colors.canvas        // 主背景色
Theme.colors.surface       // 卡片/面板背景
Theme.colors.text          // 主文字色
Theme.colors.muted         // 次要文字色
Theme.colors.subtle        // 更淡文字色
Theme.colors.border        // 边框色
Theme.colors.accent        // 强调色
Theme.colors.accentHover   // 强调色 hover
Theme.colors.success       // 成功/在线色
Theme.colors.warning       // 警告色
Theme.colors.danger        // 危险/错误色
Theme.colors.bubbleSelf    // 自己气泡背景
Theme.colors.bubbleOther   // 他人气泡背景
Theme.colors.bubbleSelfText    // 自己气泡文字
Theme.colors.bubbleOtherText   // 他人气泡文字

Theme.fonts.body           // 正文字体 { family, size }
Theme.fonts.title          // 标题字体
Theme.fonts.mono           // 等宽字体
Theme.fonts.caption        // 说明文字字体

Theme.spacing.xs           // 4px
Theme.spacing.sm           // 8px
Theme.spacing.md           // 12px
Theme.spacing.lg           // 16px
Theme.spacing.xl           // 24px

Theme.radius.sm            // 4px
Theme.radius.md            // 8px
Theme.radius.lg            // 12px
Theme.radius.full          // 9999px (圆形)

Theme.bubble.maxWidth      // 400px
Theme.bubble.padding       // { h: 12, v: 14 }
Theme.avatar.size          // 28px
```

### 8.4 与现有脚手架的关系

现有文件的处理方式：

| 现有文件 | 处理 |
|---------|------|
| `client/include/backend.h` | **重命名**为 `chat_backend.h`，扩展为完整门面 |
| `client/src/backend.cpp` | **重命名**为 `chat_backend.cpp`，扩展实现 |
| `client/include/message_model.h` | **修改**，添加 `MessageTypeRole` 和 `loadFromJsonArray` |
| `client/src/message_model.cpp` | **修改**，实现新方法 |
| `client/include/user_model.h` | **不变** |
| `client/src/user_model.cpp` | **不变** |
| `client/qml/main.qml` | **替换**，从占位变为 StackView 框架 |
