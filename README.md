# LinuxChat — 瓶子交流器 (Bottle Messenger)

一个基于 **C/S 架构** 的即时通讯应用:Linux epoll 服务端 + Windows Qt6 客户端,自定义 JSON-over-TCP 协议。当前客户端为 Dark Slate + Indigo 专业深色 QSS 主题,并保留 QML 迁移脚手架。本作品是《Linux 操作系统与程序设计》课程设计。

> **架构:C/S**。客户端为需安装运行的桌面程序(Qt6),服务端为 epoll TCP 服务器,二者通过自定义二进制帧协议通信。非 B/S(无浏览器/HTTP/Web 服务器)。

## 项目结构

```text
LinuxChat/
├── client/                 # Qt6 客户端 (Windows)
│   ├── include/  src/  qml/  resources/  CMakeLists.txt
├── server/                 # C++ epoll 服务端 (Linux)
│   ├── include/  src/  third_party/  CMakeLists.txt
├── tests/                  # Google Test (108 tests)
├── docs/                   # 文档系统 (CDD)
└── AGENTS.md / TODO.md     # CDD 契约 + 执行计划
```

## 架构概览

### 服务端架构

```
main.cpp
  ├── Database        (SQLite3 WAL, 持久化存储)
  ├── EpollServer     (epoll LT 事件循环 + ThreadPool)
  └── MessageRouter   (消息路由: 类型分发 + 在线用户管理)
```

**MessageRouter** 是核心路由组件，从 main.cpp 提取（430行 → 113行）：
- `route()`: 根据 type 字段分发消息到对应 handler
- `handle_register/login()`: SHA-256 (OpenSSL EVP) + Database 认证
- `handle_broadcast/private()`: 消息存储 + 广播/私聊发送
- `handle_logout()`: 在线用户管理 + 通知广播
- `make_user_list_msg()`: 构建在线用户列表

### 客户端架构

```
MainWindow (主窗口)
  ├── ChatClient (TCP 协议封装)
  │    └── QTcpSocket → JSON-over-TCP
  ├── LoginDialog (登录/注册 UI)
  ├── ChatView (消息显示 + 输入)
  └── Sidebar (用户列表 + 会话切换)
```

## 设计风格 — Dark Slate + Indigo 主题

### Design Tokens
| Token | Hex | 用途 |
|-------|-----|------|
| bg | `#0f172a` | 主背景 (Canvas) |
| surface | `#1e293b` | 侧栏/卡片/对话框 |
| elevated | `#334155` | 悬浮/选中态 |
| card | `#475569` | 边框/分割线 |
| text | `#f1f5f9` | 主文字 |
| muted | `#94a3b8` | 次要文字 |
| accent | `#6366f1` | Indigo 强调 (按钮/在线/Tab) |
| accent_h | `#4f46e5` | Indigo hover |
| danger | `#ef4444` | 错误/危险 |
| success | `#22c55e` | 成功/在线 |
| radius | 4/8/12px | 圆角梯度 |

### 视觉特点
- **深色专业画布**: #0f172a 暗蓝背景 + #1e293b 表面层次
- **Indigo 强调系统**: #6366f1 用于按钮、在线状态、标签页下划线、未读徽章
- **完整交互状态**: 所有控件覆盖 hover/pressed/disabled/focus 状态
- **错误状态驱动**: 通过 `[error="true"]` 动态属性实现 QSS 错误样式
- **Fusion 兼容**: QComboBox/QCheckBox/QRadioButton/QProgressBar/QGroupBox 全覆盖
- **中文字体栈**: Segoe UI → Inter → YaHei → LXGW WenKai

### 组件样式
- **登录框**: Elevated surface (#1e293b) + 错误状态红边框 + 状态文字动态变色
- **消息气泡**: 自己=Indigo (#6366f1) / 他人=Slate (#1e293b) + 微边框
- **侧边栏**: Surface 背景 + hover/selected 层次渐变
- **滚动条**: 6px 细滚动条 + hover/pressed 状态
- **标签页**: 底部 Indigo 下划线 + hover 背景 + disabled 状态
- **输入区**: 深色输入框 + Indigo focus 边框 + error 红边框

### 测试模式
```bash
# 快速测试 QSS 样式（跳过登录，直接进入聊天界面）
./linuxchat_client.exe --test-chat
```

## 快速开始

### Windows 客户端

**前置要求**:Qt 6.8.0+、Visual Studio 2022、CMake 3.16+、Qt MSVC 2022 64-bit 组件

```powershell
cd client
mkdir build; cd build
cmake .. -G "Visual Studio 18 2026" -DCMAKE_PREFIX_PATH="D:/Qt/6.8.0/msvc2022_64"
cmake --build . --config Release
cd Release
./linuxchat_client.exe
```

### Linux 服务端

**前置要求**:
```bash
# Ubuntu/Debian
sudo apt install build-essential cmake libsqlite3-dev libssl-dev
# CentOS/RHEL
sudo yum install gcc-c++ cmake sqlite-devel openssl-devel
```

**编译运行**:
```bash
cd server
mkdir build; cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

./linuxchat_server                 # 默认端口 8080
./linuxchat_server --port 9090     # 自定义端口
./linuxchat_server --help          # 帮助
```

### 运行测试

```bash
# 编译测试
cd tests
mkdir build; cd build
cmake ..
cmake --build .

# 运行所有测试 (108 个)
ctest --output-on-failure

# 运行特定模块测试
./test_protocol        # 协议测试 (19 tests)
./test_database        # 数据库测试 (27 tests)
./test_message_router  # 路由测试 (27 tests)
./test_message_handler # 处理器测试 (15 tests)
./test_crypto          # 加密测试 (9 tests)
./test_thread_pool     # 线程池测试 (11 tests)
```

## 连接服务器

在客户端登录界面:
- **服务器地址**:Linux 机器的 IP 地址(默认填 `120.55.63.32`,可改)
- **端口**:`8080`(或自定义)
- 先点「连接」建立 TCP,再点「登录」/「注册」

> ⚠️ 服务端启动后,务必确认 **8080 端口**在本机防火墙(`ufw`/`firewalld`)和云厂商**安全组**(阿里云等)均已放行入方向 TCP,否则外网连不上。
> 验证命令(在客户端机器): `nc -vz <server-ip> 8080` 或 `telnet <server-ip> 8080`。TCP connect 成功但登录后立即断开 → 看服务端日志是否有 oversized 或 handler 异常。

## 技术栈

| 层 | 技术 |
|---|---|
| 服务端 | C++17 / epoll(level-triggered) / ThreadPool / SQLite3(WAL) / OpenSSL(SHA-256 EVP) / nlohmann::json |
| 客户端 | C++17 / Qt6 Widgets / Dark Slate + Indigo QSS / QTcpSocket / 可选 QML 脚手架 |
| 协议 | JSON-over-TCP, 4 字节大端长度前缀, 16MB 上限, EAGAIN 重试 |
| 测试 | Google Test (FetchContent), 108 个测试用例 |

## 特性

- ✅ Dark Slate + Indigo 专业深色主题（Design Tokens + 完整交互状态）
- ✅ 自定义字体栈 (Segoe UI → Inter → YaHei → LXGW WenKai)
- ✅ 消息气泡(发送/接收) + 在线用户列表 + 当前会话高亮
- ✅ 公聊广播 + 一对一私聊 + 历史消息加载
- ✅ 跨平台(Windows 客户端 + Linux 服务端)
- ✅ MessageRouter 架构（430 行 → 113 行）
- ✅ 108 个单元测试全覆盖
- ✅ --test-chat 模式快速 UI 迭代
- ✅ Fusion 样式兼容（QComboBox/QCheckBox/QRadioButton/QProgressBar/QGroupBox）
- ✅ 登录对话框 error 动态属性驱动 QSS 错误状态
- 🚧 QML 迁移脚手架（Backend/MessageModel/UserModel + `qml/main.qml`; 运行入口仍为 Widgets）

## 文档

- [架构说明](ARCHITECTURE.md)
- [上下文索引](docs/INDEX.md)
- [项目合约](CONTRACT.md)
- [产品需求 PRD](docs/specs/prd.md)
- [架构 Blueprint](docs/specs/blueprint.md)
- [通信协议](docs/protocol.md)
- [执行计划 TODO](TODO.md)
- [实施日志](docs/JOURNAL.md)
- [课程设计展示](PRESENTATION.md)

## 许可证

MIT License — 作者 Ningbottle

---

[![CDD Project](https://img.shields.io/badge/CDD-Project-ecc569?style=flat-square&labelColor=0d1a26)](https://github.com/ruphware/cdd-boilerplate)
[![CDD Skills](https://img.shields.io/badge/CDD-Skills-ecc569?style=flat-square&labelColor=0d1a26)](https://github.com/ruphware/cdd-skills)

<sup>This repo follows the [`CDD Project`](https://github.com/ruphware/cdd-boilerplate) + [`CDD Skills`](https://github.com/ruphware/cdd-skills) workflow with the local [`AGENTS.md`](./AGENTS.md) contract.</sup>
<sup>Start with `$cdd-boot`. Use `$cdd-audit` for implementation or codebase audits, `$cdd-plan` + `$cdd-implement` for feature work, and `$cdd-maintain` for doc drift, codebase cleanup, index refresh, refactor architecture audit, and upkeep.</sup>
