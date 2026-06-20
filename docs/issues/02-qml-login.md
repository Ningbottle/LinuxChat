# Issue 2: 登录界面（Login）

**PRD**: [qml-migration-prd.md](../specs/qml-migration-prd.md)
**Step**: 2 of 5
**Dependencies**: Issue 1
**Estimated effort**: Medium

## Objective

用 QML 实现登录界面，替代 `LoginDialog`（QDialog），建立 `StackView` 导航框架。创建 `main.qml` 的 `StackView` 框架和 `LoginDialog.qml` 登录界面，复用 `LoginController` 管理连接/登录状态。UI 布局参照当前 `login_dialog.cpp` 的 `setup_ui()`（104-208 行）。登录成功后 `StackView` push `ChatWindow.qml`（占位），断开连接时 pop 回登录界面。

## Files to Create

| 文件 | 类型 | 说明 |
|------|------|------|
| `client/qml/main.qml` | QML（替换现有占位） | `ApplicationWindow` + `StackView`，初始项为 `LoginDialog.qml`，监听 `LoginController.state` 变化执行 push/pop |
| `client/qml/views/LoginDialog.qml` | QML | 登录界面，参照 `login_dialog.cpp:setup_ui()` 布局，使用 `Theme.colors` 着色 |
| `client/qml/views/ChatWindow.qml` | QML（占位） | 空壳 `Page`，仅显示 "Chat - Coming in Step 3"，用于验证 StackView push |
| `client/qml/components/LoginBackground.qml` | QML | 报纸纹理 + 地球仪 SVG 水印背景组件，替代 `login_dialog.cpp:drawNewspaperBackground` + `drawGlobe` |

## Files to Modify

| 文件 | 修改内容 |
|------|---------|
| `client/main.cpp` | `#ifdef HAS_QML` 分支：不再创建 `LoginDialog` / `MainWindow`，改为 `engine.load("qrc:/qml/main.qml")`。保留 `--test-chat` 分支但改为加载 QML 测试模式。 |
| `client/CMakeLists.txt` | `qt_add_qml_module` 的 `QML_FILES` 列表添加新文件 |

## Acceptance Criteria

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

## Implementation Notes

- `main.qml` 是整个 QML UI 的入口，使用 `ApplicationWindow` + `StackView` 管理页面导航
- `LoginDialog.qml` 的布局应参照 `login_dialog.cpp:setup_ui()` 104-208 行：标题 "LinuxChat"、服务器地址+端口+连接按钮、用户名+密码输入、登录+注册按钮、状态标签
- `LoginController` 的状态变化驱动 UI 更新：按钮启用/禁用、状态文本、加载指示器
- `LoginBackground.qml` 使用 QML Canvas 或 Image 实现报纸纹理和地球仪水印
- `ChatWindow.qml` 此步仅作占位，Step 3 会替换为完整实现
- 所有颜色通过 `Theme.colors.xxx` 绑定，确保主题切换时登录界面也能响应
- StackView 的 push/pop 时机由 `LoginController.state` 变化驱动：`authenticated` 时 push，`disconnected`（从已认证状态回落）时 pop
