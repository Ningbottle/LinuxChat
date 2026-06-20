# Issue 1: 基础设施（Infrastructure）

**PRD**: [qml-migration-prd.md](../specs/qml-migration-prd.md)
**Step**: 1 of 5
**Dependencies**: None
**Estimated effort**: Large

## Objective

建立 QML 运行所需的所有 C++ 后端类和主题系统，但不替换任何 UI。创建 `ChatBackend` 门面类封装 `ChatClient` 的信号/槽为 QML 可绑定的属性和方法；实现三个 Model 类（`MessageListModel`、`UserListModel`、`SessionModel`）将数据暴露为 `QAbstractListModel`；创建 `Theme.qml` 单例定义 4 套皮肤的颜色/字体/间距 token；在 `main.cpp` 中注册所有单例类型。

## Files to Create

| 文件 | 类型 | 说明 |
|------|------|------|
| `client/include/chat_backend.h` | C++ Header | `ChatBackend` QObject 门面，持有 `ChatClient*`，拥有 `MessageListModel` / `UserListModel` / `SessionModel` |
| `client/src/chat_backend.cpp` | C++ Source | 实现：转发 `ChatClient` 信号到 Model，提供 `Q_INVOKABLE` 方法 |
| `client/include/session_model.h` | C++ Header | `SessionModel : QAbstractListModel`，角色：`targetName` / `isRoom` / `unreadCount` / `lastMessage` / `lastTimestamp` |
| `client/src/session_model.cpp` | C++ Source | 实现 Model 逻辑 |
| `client/include/login_controller.h` | C++ Header | `LoginController` QObject，状态机：`disconnected` -> `connecting` -> `authenticating` -> `authenticated`，包含超时定时器 |
| `client/src/login_controller.cpp` | C++ Source | 复用 `login_dialog.cpp` 中的连接/登录超时逻辑（262-365 行） |
| `client/qml/styles/Theme.qml` | QML Singleton | `pragma Singleton`，4 套皮肤预设（Minimal / Dense / Motion / iMessage），`Q_PROPERTY` 暴露 `colors` / `fonts` / `spacing` |
| `client/qml/styles/Minimal.qml` | QML | Minimal 皮肤的颜色/字体/间距定义 |
| `client/qml/styles/Dense.qml` | QML | Dense 皮肤定义 |
| `client/qml/styles/Motion.qml` | QML | Motion 皮肤定义 |
| `client/qml/styles/iMessage.qml` | QML | iMessage 皮肤定义 |

## Files to Modify

| 文件 | 修改内容 |
|------|---------|
| `client/main.cpp` | 在 `#ifdef HAS_QML` 分支中：创建 `ChatBackend` 和 `LoginController` 实例，调用 `qmlRegisterSingletonType` 注册 `ChatClient` / `ChatBackend` / `Theme`，创建 `QQmlApplicationEngine` 加载 `main.qml`。保留 Widgets 路径作为 else 分支。 |
| `client/CMakeLists.txt` | 在 `if(HAS_QML)` 块中：`qt_add_qml_module` 添加新 QML 文件（`qml/main.qml` + `qml/styles/Theme.qml` + 4 个皮肤文件）。添加新 C++ 源文件到 `SOURCES`。 |
| `client/include/message_model.h` | 扩展角色：添加 `MessageTypeRole`（`normal` / `system` / `timeSeparator`），添加 `loadFromJsonArray(QJsonArray, myUsername)` 方法 |
| `client/src/message_model.cpp` | 实现新角色和 `loadFromJsonArray` |
| `client/include/backend.h` | **重命名为** `chat_backend.h`，扩展：添加 `sendPrivate()` / `sendHistoryReq()` / `disconnectFromServer()` Q_INVOKABLE，添加嵌套 Model 指针属性 |
| `client/src/backend.cpp` | **重命名为** `chat_backend.cpp`，实现扩展逻辑 |

## Acceptance Criteria

- [ ] `cmake --build` 通过，HAS_QML 路径编译成功
- [ ] `main.cpp` 中 `#ifdef HAS_QML` 分支正确注册 3 个单例
- [ ] `ChatBackend` 可在 QML 中 `import LinuxChat 1.0` 后访问
- [ ] `MessageListModel` 的 `roleNames()` 返回 `sender` / `content` / `timestamp` / `isSelf` / `messageType`
- [ ] `UserListModel` 的 `roleNames()` 返回 `username`
- [ ] `SessionModel` 的 `roleNames()` 返回 `targetName` / `isRoom` / `unreadCount` / `lastMessage` / `lastTimestamp`
- [ ] `Theme.qml` 可在 QML 中访问 `Theme.colors.accent` 等属性
- [ ] 4 套皮肤的颜色 token 覆盖：`canvas` / `surface` / `text` / `muted` / `accent` / `success` / `danger` / `border` / `radius`
- [ ] `LoginController` 状态机：`disconnected` -> 点击连接 -> `connecting` -> TCP 成功 -> `authenticating` -> LOGIN_OK -> `authenticated`
- [ ] `LoginController` 超时：连接 10s / 登录 8s，超时后回到 `disconnected` 并设置错误文本
- [ ] 现有 Widgets 路径（`#else` 分支）不受影响，`--test-chat` 仍走 Widgets

## Implementation Notes

- `ChatBackend` 是 QML 与 `ChatClient` 之间的唯一入口点（门面模式），不拥有 `ChatClient` 生命周期
- `LoginController` 的超时逻辑应从 `login_dialog.cpp` 262-365 行提取复用
- `Theme.qml` 使用 `pragma Singleton`，通过 `qmlRegisterSingletonType` 注册
- 4 套皮肤的 token 清单见 PRD 8.3 节，包括 colors（14 个）、fonts（4 组）、spacing（5 级）、radius（4 级）、bubble（2 个）、avatar（1 个）
- `backend.h` 需要重命名为 `chat_backend.h`，注意更新所有 include 引用
- `MessageListModel` 的 `MessageTypeRole` 用于区分普通消息、系统通知和时间分隔线
- 迁移过程中 Widgets 和 QML 通过 `HAS_QML` 宏共存，每步可独立编译运行
