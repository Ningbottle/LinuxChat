# Issue 3: 聊天界面（Chat）

**PRD**: [qml-migration-prd.md](../specs/qml-migration-prd.md)
**Step**: 3 of 5
**Dependencies**: Issue 2
**Estimated effort**: Large

## Objective

用 QML 实现完整的聊天界面，替代 `MainWindow` + `ChatView`。这是迁移的核心工作量。创建 `ChatWindow.qml`（主聊天窗口）、`Sidebar.qml`（侧边栏）、`MessageBubble.qml`（消息气泡）、`ChatHeader.qml`（聊天头部）、`InputArea.qml`（输入区域）等组件。消息显示使用 `ListView` + `MessageBubble.qml` delegate，替代当前每个消息创建独立 `QWidget` 的方式。

## Files to Create

| 文件 | 类型 | 说明 |
|------|------|------|
| `client/qml/views/ChatWindow.qml` | QML（替换占位） | 主聊天窗口：TopBar + Sidebar + ChatArea，参照 `main_window.cpp` 布局 |
| `client/qml/components/Sidebar.qml` | QML | 侧边栏：当前用户信息 + 在线用户列表（`ListView` + `UserListModel`），参照 `main_window.cpp:130-163` |
| `client/qml/components/MessageBubble.qml` | QML | 消息气泡组件，3 种变体（self / other / system），参照 `chat_view.cpp:create_bubble` / `create_system_bubble` |
| `client/qml/components/ChatHeader.qml` | QML | 聊天头部：会话名称 + 在线状态指示器，参照 `main_window.cpp` topBar 中的 chat 部分 |
| `client/qml/components/InputArea.qml` | QML | 输入区域：`TextArea` + 发送按钮，参照 `chat_view.cpp:43-75`，支持 Ctrl+Enter 发送 |
| `client/qml/components/TimeSeparator.qml` | QML | 时间分隔线组件，参照 `chat_view.cpp:create_time_separator` |
| `client/qml/components/SettingsPopup.qml` | QML | 设置弹窗：退出登录 + 主题切换，参照 `main_window.cpp:on_settings_clicked` |

## Files to Modify

| 文件 | 修改内容 |
|------|---------|
| `client/include/chat_backend.h` | 添加 `Q_INVOKABLE void sendPrivate(const QString& to, const QString& content)` / `Q_INVOKABLE void requestHistory(const QString& target)` / `Q_INVOKABLE void disconnectFromServer()` |
| `client/src/chat_backend.cpp` | 实现新方法，连接 `ChatClient::private_received` 到 `privateMessages` Model，连接 `ChatClient::history_received` 到对应 Model 的 `loadFromJsonArray` |
| `client/main.cpp` | `--test-chat` 分支：改为加载 QML 测试模式，调用 `ChatBackend::populateTestData()` |
| `client/include/chat_backend.h` | 添加 `Q_INVOKABLE void populateTestData()` 用于测试模式 |

## Acceptance Criteria

- [ ] 登录成功后显示完整的聊天界面，布局与当前 Widgets 版本功能等价
- [ ] 公共聊天室消息显示：`ListView` + `MessageBubble` delegate，气泡样式区分 self/other/system
- [ ] 发送消息：输入文本 -> 点击发送 -> `ChatBackend.sendMessage()` -> `ChatClient.send_broadcast()` -> 收到广播 -> 气泡显示
- [ ] 私聊：双击侧边栏用户 -> 切换到私聊会话 -> `ChatBackend.sendPrivate()` -> 收到私信 -> 气泡显示
- [ ] 历史消息：登录后自动请求 `__room__` 历史，消息加载到 Model
- [ ] 在线用户列表：`ChatClient::user_list_updated` -> `UserListModel` 更新 -> `ListView` 刷新
- [ ] 系统通知：`ChatClient::notify_received` -> 系统气泡显示
- [ ] 未读消息标记：非当前会话收到消息时显示未读计数
- [ ] 侧边栏折叠/展开动画（200ms，参照 `main_window.cpp:on_sidebar_toggle`）
- [ ] 设置弹窗：显示"退出登录"按钮，点击后 `StackView` pop 回登录界面
- [ ] 断开连接：`ChatClient::disconnected` -> 显示系统消息 -> 返回登录界面
- [ ] `--test-chat` 模式：直接进入聊天界面，填充测试数据，发送消息本地回显
- [ ] 窗口标题显示 "LinuxChat -- {username}"
- [ ] 消息气泡最大宽度 400px，头像 28px 圆形

## Implementation Notes

- `ChatWindow.qml` 参照 `main_window.cpp:setup_ui()`（52-202 行）的布局：顶部栏（标题 + 连接状态 + 设置按钮）、左侧边栏（当前用户 + 在线用户列表）、右侧聊天区
- `ListView` 的 model 绑定到 `ChatBackend.roomMessages`（公共聊天室）或 `ChatBackend.privateMessages[target]`（私聊）
- `MessageBubble.qml` 需要支持 3 种变体：self（自己发送的气泡）、other（他人发送的气泡）、system（系统通知）
- 侧边栏在线用户列表绑定到 `ChatBackend.onlineUsers`（`UserListModel`），双击用户打开私聊会话
- `InputArea.qml` 支持 Ctrl+Enter 发送消息，Enter 换行（或根据用户习惯切换）
- `SettingsPopup.qml` 提供退出登录功能，后续 Step 4 会添加主题切换
- `ChatBackend` 需要新增 `sendPrivate()`、`requestHistory()`、`disconnectFromServer()`、`populateTestData()` 方法
- `--test-chat` 模式通过 `ChatBackend::populateTestData()` 直接操作 Model，QML 端无需特殊处理
- 消息气泡最大宽度 400px，头像 28px 圆形，参照当前 Widgets 版本的视觉规格
