# Issue 4: 动效与皮肤（Animations & Skins）

**PRD**: [qml-migration-prd.md](../specs/qml-migration-prd.md)
**Step**: 4 of 5
**Dependencies**: Issue 3
**Estimated effort**: Medium

## Objective

实现 4 套主题皮肤的完整视觉效果和交互动效。`Theme.qml` 中的 4 套皮肤需要完整的颜色/字体/间距/圆角/阴影 token，并在所有组件中通过 `Theme.colors.xxx` 绑定。动效包括登录界面入场动画（弹簧物理）、消息发送气泡弹入动画（bounce）、会话切换过渡动画、侧边栏折叠/展开、在线状态脉冲动画。

## Files to Create

| 文件 | 类型 | 说明 |
|------|------|------|
| `client/qml/styles/Minimal.qml` | QML | Minimal 皮肤完整 token 定义（颜色/字体/间距/圆角/阴影） |
| `client/qml/styles/Dense.qml` | QML | Dense 皮肤定义 |
| `client/qml/styles/Motion.qml` | QML | Motion 皮肤定义 |
| `client/qml/styles/iMessage.qml` | QML | iMessage 皮肤定义 |
| `client/qml/animations/BounceAnimation.qml` | QML | 消息气泡弹入动画组件 |
| `client/qml/animations/SpringEntrance.qml` | QML | 登录界面弹簧入场动画组件 |
| `client/qml/animations/PulseAnimation.qml` | QML | 在线状态脉冲动画组件 |

## Files to Modify

| 文件 | 修改内容 |
|------|---------|
| `client/qml/styles/Theme.qml` | 完善 4 套皮肤的完整 token 映射，添加 `Q_INVOKABLE void setSkin(string name)` |
| `client/qml/views/LoginDialog.qml` | 添加 `SpringEntrance` 入场动画 |
| `client/qml/components/MessageBubble.qml` | 添加 `BounceAnimation`（新消息弹入） |
| `client/qml/views/ChatWindow.qml` | 添加会话切换过渡动画，侧边栏折叠 `PropertyAnimation` |
| `client/qml/components/Sidebar.qml` | 添加在线状态 `PulseAnimation` |
| `client/qml/components/SettingsPopup.qml` | 添加皮肤选择器（4 个选项），绑定 `Theme.setSkin()` |

## Acceptance Criteria

- [ ] 4 套皮肤均可通过设置弹窗切换，切换后所有组件颜色/字体/间距即时更新
- [ ] Minimal 皮肤视觉效果与当前 style.qss 一致（颜色 token 对齐）
- [ ] Dense 皮肤：字号 12px，间距紧凑，消息密度更高
- [ ] Motion 皮肤：暗黑背景，所有过渡动画开启，弹簧物理曲线
- [ ] iMessage 皮肤：白色背景，绿色气泡（self），灰色气泡（other）
- [ ] 登录界面入场：元素从下方滑入 + 弹簧回弹（SpringAnimation, damping: 0.5）
- [ ] 消息发送气泡：从底部弹入（scale 0.8 -> 1.0 + opacity 0 -> 1, 200ms）
- [ ] 会话切换：StackView push/pop 有滑入/滑出过渡（300ms, EaseInOut）
- [ ] 侧边栏折叠：宽度 240 -> 0，200ms InOutCubic（与当前 `main_window.cpp:437-453` 一致）
- [ ] 在线状态脉冲：绿点 opacity 1.0 <-> 0.5，1.5s 循环
- [ ] 皮肤选择持久化：`QSettings` 存储用户选择，下次启动自动应用

## Implementation Notes

**4 套皮肤定义**:

| 皮肤 | 色彩基调 | 字体 | 间距 | 特点 |
|------|---------|------|------|------|
| Minimal | 灰白 + 蓝色强调（当前 style.qss 的配色） | LXGW WenKai 14px | 标准 8/12/16px | 干净、专业、当前默认 |
| Dense | 深灰 + 橙色强调 | 系统字体 12px | 紧凑 4/8/12px | 信息密度高，适合多消息场景 |
| Motion | 暗黑 + 紫色强调 | LXGW WenKai 15px | 宽松 12/16/24px | 全部动效开启，过渡流畅 |
| iMessage | 白色 + 绿色气泡 | SF Pro (fallback LXGW) 15px | Apple 标准 8/16px | 模拟 iOS iMessage 风格 |

**Token 清单**（所有皮肤必须实现）:
- colors: `canvas` / `surface` / `text` / `muted` / `subtle` / `border` / `accent` / `accentHover` / `success` / `warning` / `danger` / `bubbleSelf` / `bubbleOther` / `bubbleSelfText` / `bubbleOtherText`
- fonts: `body` / `title` / `mono` / `caption`（每组含 `family` 和 `size`）
- spacing: `xs`(4px) / `sm`(8px) / `md`(12px) / `lg`(16px) / `xl`(24px)
- radius: `sm`(4px) / `md`(8px) / `lg`(12px) / `full`(9999px)
- bubble: `maxWidth`(400px) / `padding`({h:12, v:14})
- avatar: `size`(28px)

**动效细节**:
- `SpringEntrance.qml`：使用 `SpringAnimation`，damping 0.5，元素从下方滑入
- `BounceAnimation.qml`：scale 0.8 -> 1.0 + opacity 0 -> 1，duration 200ms
- `PulseAnimation.qml`：opacity 1.0 <-> 0.5，duration 1.5s，infinite loop
- 侧边栏折叠使用 `PropertyAnimation on width`，200ms InOutCubic
- 会话切换使用 StackView 的 push/pop transition，300ms EaseInOut

**持久化**:
- 使用 `QSettings` 存储用户选择的皮肤名称
- 应用启动时读取 `QSettings` 并调用 `Theme.setSkin()` 自动应用
