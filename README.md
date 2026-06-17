# LinuxChat — 瓶子交流器 (Bottle Messenger)

一个基于 **C/S 架构** 的即时通讯应用:Linux epoll 服务端 + Windows Qt6 客户端,自定义 JSON-over-TCP 协议。报纸复古 + 紫藤萝瀑布视觉风格。本作品是《Linux 操作系统与程序设计》课程设计。

> **架构:C/S**。客户端为需安装运行的桌面程序(Qt6),服务端为 epoll TCP 服务器,二者通过自定义二进制帧协议通信。非 B/S(无浏览器/HTTP/Web 服务器)。

## 项目结构

```text
LinuxChat/
├── client/                 # Qt6 客户端 (Windows)
│   ├── include/  src/  resources/  CMakeLists.txt
├── server/                 # C++ epoll 服务端 (Linux)
│   ├── include/  src/  third_party/  CMakeLists.txt
├── tests/                  # Google Test
├── docs/                   # 文档系统 (CDD)
└── AGENTS.md / TODO.md     # CDD 契约 + 执行计划
```

## 设计风格

### 登录界面
- **报纸复古风格**:米色背景 + 细微噪点纹理
- **地球仪 SVG**:低透明度线条风格地球仪背景
- **字体**:LXGW WenKai(霞鹜文楷)正文 + Newsreader 标题

### 主聊天界面
- **紫藤萝瀑布背景**:宗璞《紫藤萝瀑布》风格 — 棕色藤蔓 + 淡紫色花穗 + 绿色叶子,SVG 平铺 opacity 0.35
- **配色**:石墨灰系(`#44403c` / `#78776c` / `#faf9f7`)

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
| 服务端 | C++17 / epoll(level-triggered) / ThreadPool / SQLite3(WAL) / OpenSSL(SHA-256) / nlohmann::json |
| 客户端 | C++17 / Qt6 Widgets / QSS / QPainter+SVG / QTcpSocket |
| 协议 | JSON-over-TCP,4 字节大端长度前缀,16MB 上限 |
| 测试 | Google Test(FetchContent) |

## 特性

- ✅ 报纸复古登录界面 + 紫藤萝瀑布聊天背景
- ✅ 自定义字体(LXGW WenKai、Newsreader)
- ✅ 消息气泡(发送/接收)+ 在线用户列表 + 当前会话高亮
- ✅ 公聊广播 + 一对一私聊 + 历史消息加载
- ✅ 跨平台(Windows 客户端 + Linux 服务端)

## 文档

- [产品需求 PRD](docs/specs/prd.md)
- [架构 Blueprint](docs/specs/blueprint.md)
- [通信协议](docs/protocol.md)
- [执行计划 TODO](TODO.md)
- [实施日志](docs/JOURNAL.md)
- [UI 设计规范](docs/superpowers/specs/2026-06-16-bottle-messenger-ui-design.md)

## 当前状态(2026-06-17 审计)

CDD Phase 1 审计发现 **3 个连接层缺陷**导致"有时连得上有时连不上 / 连接成功后立即断开"。修复计划见 [TODO.md](TODO.md) Step 01-03,审计详情见 [docs/JOURNAL.md](docs/JOURNAL.md)。

## 许可证

MIT License — 作者 Ningbottle

---

[![CDD Project](https://img.shields.io/badge/CDD-Project-ecc569?style=flat-square&labelColor=0d1a26)](https://github.com/ruphware/cdd-boilerplate)
[![CDD Skills](https://img.shields.io/badge/CDD-Skills-ecc569?style=flat-square&labelColor=0d1a26)](https://github.com/ruphware/cdd-skills)

<sup>This repo follows the [`CDD Project`](https://github.com/ruphware/cdd-boilerplate) + [`CDD Skills`](https://github.com/ruphware/cdd-skills) workflow with the local [`AGENTS.md`](./AGENTS.md) contract.</sup>
<sup>Start with `$cdd-boot`. Use `$cdd-audit` for implementation or codebase audits, `$cdd-plan` + `$cdd-implement` for feature work, and `$cdd-maintain` for doc drift, codebase cleanup, index refresh, refactor architecture audit, and upkeep.</sup>
