# Issue 5: 清理（Cleanup）

**PRD**: [qml-migration-prd.md](../specs/qml-migration-prd.md)
**Step**: 5 of 5
**Dependencies**: Issue 4
**Estimated effort**: Small

## Objective

删除所有 Qt Widgets UI 文件，移除 `Qt6::Widgets` 依赖。在所有功能通过 QML 实现后，删除旧的 Widgets UI 文件和 QSS 样式表。更新 `CMakeLists.txt` 移除 `Qt6::Widgets` 依赖（保留 `Qt6::Network` 和 `Qt6::Svg`）。清理 `main.cpp` 中的 `#ifdef HAS_QML` 条件编译，只保留 QML 路径。

## Files to Create

无。

## Files to Modify

| 文件 | 修改内容 |
|------|---------|
| `client/CMakeLists.txt` | `find_package(Qt6 REQUIRED COMPONENTS Widgets Network Svg)` -> `find_package(Qt6 REQUIRED COMPONENTS Network Svg)`。移除 `Qt6::Widgets` 链接。移除 `HAS_QML` 条件编译，QML 模块变为必须。 |
| `client/main.cpp` | 移除所有 `#ifdef HAS_QML` / `#else` 条件编译。移除 `#include <QApplication>` 改为 `#include <QGuiApplication>`。移除 QSS 加载代码（36-44 行）。移除 Widgets 路径的 LoginDialog/MainWindow 创建代码。 |
| `client/resources/resources.qrc` | 移除 `style.qss` 资源引用 |

## Files to Delete

| 文件 | 说明 |
|------|------|
| `client/include/main_window.h` | MainWindow 头文件 |
| `client/src/main_window.cpp` | MainWindow 实现（601 行） |
| `client/include/chat_view.h` | ChatView 头文件 |
| `client/src/chat_view.cpp` | ChatView 实现（289 行） |
| `client/include/login_dialog.h` | LoginDialog 头文件 |
| `client/src/login_dialog.cpp` | LoginDialog 实现（381 行） |
| `client/resources/style.qss` | QSS 样式表（1036 行） |

## Acceptance Criteria

- [x] 所有 7 个 Widgets 文件已删除
- [x] `cmake --build` 通过，无 `Qt6::Widgets` 依赖
- [ ] `linuxchat_client.exe` 正常启动，显示 QML 登录界面（需安装 Qt6 Qml 模块后验证）
- [ ] 完整流程可用：登录 -> 聊天 -> 私聊 -> 断开 -> 重新登录（需安装 Qt6 Qml 模块后验证）
- [ ] `--test-chat` 模式正常工作（需安装 Qt6 Qml 模块后验证）
- [ ] 无编译警告（需安装 Qt6 Qml 模块后验证）
- [x] `style.qss` 从 `resources.qrc` 中移除，资源文件不包含已删除文件

## Implementation Notes

- 本步骤是迁移的最后一步，前提是 Issue 1-4 的所有功能已通过 QML 实现并验证
- 删除文件前确认没有任何其他代码引用这些文件（grep 检查 include 和使用）
- `main.cpp` 中移除 `#ifdef HAS_QML` 条件编译后，只保留 QML 路径：`QGuiApplication` + `QQmlApplicationEngine`
- `CMakeLists.txt` 中移除 `HAS_QML` 相关的 `if/else/endif` 块，QML 模块变为必须依赖
- `resources.qrc` 中移除 `style.qss` 的 `<file>` 条目
- 预期代码行数变化：净减少约 2300 行 Widgets 代码（main_window 600 + chat_view 290 + login_dialog 380 + style.qss 1036）
- 移除 `Qt6::Widgets` 后，`find_package` 中只保留 `Network` 和 `Svg`
- 确保 `--test-chat` CLI 参数在清理后仍然可用，走 QML 测试模式路径
