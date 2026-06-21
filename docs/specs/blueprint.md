# Blueprint — LinuxChat (瓶子交流器 / Bottle Messenger)

Status: Active | Version: 1.0 | Last Updated: 2026-06-19

## Purpose

C/S 架构即时通讯系统:Linux epoll 服务端 ↔ Windows Qt6 客户端,JSON-over-TCP 自定义协议。客户端已全面迁移至 Qt6 QML 架构，采用现代化 Ethereal 主题设计。本文整合原 `ARCHITECTURE.md`(系统结构)与 `CONTRACT.md`(模块契约),作为 CDD 架构契约的唯一来源。

## Architecture (high-level)

```text
┌─────────────────────────────────────────────────────────────────┐
│  Windows Client (Qt6 Widgets runtime + QML scaffold)             │
│  ┌────────────┐  ┌──────────────┐  ┌────────────────────────┐   │
│  │ LoginDialog │  │ ChatClient   │  │ MainWindow             │   │
│  │ (login/reg) │  │ (TCP+JSON)   │  │ (sidebar+chatTabs)     │   │
│  └─────┬──────┘  └──────┬───────┘  └───────────┬────────────┘   │
│        └────────────────┼───────────────────────┘               │
│                         │ QTcpSocket                            │
└─────────────────────────┼───────────────────────────────────────┘
                          │ JSON-over-TCP (4B BE prefix)
┌─────────────────────────┼───────────────────────────────────────┐
│  Linux Server (epoll)   │                                       │
│  ┌──────────────────────┴───────────────────────────────────┐   │
│  │ EpollServer (main thread: epoll_wait loop, level-trig)   │   │
│  │  ├── accept4() → ClientSession                            │   │
│  │  ├── recv_msgs() → parse frames                           │   │
│  │  └── pool_.enqueue() → dispatch to workers                │   │
│  └─────────────────────┬─────────────────────────────────────┘   │
│                        │                                        │
│  ┌─────────────────────┴─────────────────────────────────────┐   │
│  │ ThreadPool (N workers)                                     │   │
│  │  └── handle_message(session, json)                         │   │
│  │       ├── REGISTER/LOGIN → Database                       │   │
│  │       ├── BROADCAST → Database + broadcast()              │   │
│  │       ├── PRIVATE → Database + send_to_fd()               │   │
│  │       └── HISTORY_REQ → Database::get_history()           │   │
│  └─────────────────────┬─────────────────────────────────────┘   │
│  ┌─────────────────────┴─────────────────────────────────────┐   │
│  │ Database (SQLite3, WAL, mutex-protected)                  │   │
│  │  ├── users(username PK, password_hash, created_at)        │   │
│  │  └── messages(id, from_user, to_user, content, timestamp) │   │
│  └───────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
```

- **Components**: EpollServer(主线程 IO) / ThreadPool(N worker 业务) / MessageRouter(业务路由) / Database(SQLite 持久化) / ClientSession(连接态) / ChatClient+Backend+MessageModel+UserModel(C++ 核心业务) / QML UI(现代化客户端界面)
- **Data flow**: 客户端 QTcpSocket → 服务端 epoll 就绪 → recv_msgs 组帧 → worker 处理 → Database 读写 → send_msg 回客户端
- **Boundaries**: TCP 帧边界 = 4 字节大端长度 + JSON;session 边界 = fd;线程边界 = 主线程持有 sessions_mutex_,worker 通过 enqueue 解耦

## Key Invariants (MUST NOT BREAK)

1) **帧格式不变**:每条消息 = `[4 字节大端 uint32 长度][JSON UTF-8 正文]`,长度上限 256KB。客户端与服务端编解码必须严格对齐(见 `docs/protocol.md`)。
2) **认证前白名单**:未认证 session(`session.username` 为空)只允许 `REGISTER`/`LOGIN`,其余返回 `NOT_AUTHENTICATED`。
3) **`to="__room__"` 语义**:表示公共聊天室广播历史;`to=用户名` 表示私聊历史。
4) **广播排除**:登录成功的用户自己也会收到 USER_LIST;离开通知用 `broadcast(msg, exclude_username)` 排除自己。
5) **线程安全**:服务端使用 `std::shared_mutex` 控制并发。读操作（如获取列表）持共享锁（读锁），写操作（如登录、登出修改）持独占锁（写锁）。所有对 SQLite 的访问受到同样的并发控制机制保护，配合 WAL 模式实现高效读写。

## Interfaces / Contracts

### Database API (`server/include/database.h`)
```cpp
class Database {
    explicit Database(const std::string& db_path);
    bool register_user(const std::string& username, const std::string& password_hash);
    bool verify_user(const std::string& username, const std::string& password_hash);
    void store_message(const std::string& from, const std::string& to,
                       const std::string& content, int64_t timestamp);
    std::vector<nlohmann::json> get_history(const std::string& to, int limit = 20);
};
```

### Server routing (`server/main.cpp` + `server/include/message_router.h`)
```cpp
server.set_message_handler(
    [&router](ClientSession& session, const nlohmann::json& msg) {
        router.route(session, msg);
    });
server.set_disconnect_handler(
    [&router](ClientSession& session) {
        router.handle_logout(session);
    });

class MessageRouter {
    void route(ClientSession& session, const nlohmann::json& msg);
    void handle_register(ClientSession& session, const nlohmann::json& msg);
    void handle_login(ClientSession& session, const nlohmann::json& msg);
    void handle_logout(ClientSession& session);
    void handle_broadcast(ClientSession& session, const nlohmann::json& msg);
    void handle_private(ClientSession& session, const nlohmann::json& msg);
    void handle_history_req(ClientSession& session, const nlohmann::json& msg);
};
```

### ChatClient 信号 (`client/include/chat_client.h`)
```
connected(), disconnected(), connection_error(QString)
login_ok(QString), error_received(QString code, QString content)
broadcast_received(from, content, timestamp)
private_received(from, to, content, timestamp)
user_list_updated(QStringList), history_received(target, QJsonArray)
notify_received(QString)
```

### 协议消息类型(完整表见 `docs/protocol.md`)
- C→S: REGISTER, LOGIN, LOGOUT, BROADCAST, PRIVATE, FRIEND_REQ(未实现), FRIEND_ACK, BLACKLIST_*, HISTORY_REQ
- S→C: LOGIN_OK, BROADCAST, PRIVATE, USER_LIST, NOTIFY, HISTORY_RESP, ERROR

## Directory Overview

```text
LinuxChat/
├── client/                 # Qt6 客户端 (Windows)
│   ├── include/           # chat_client/login_dialog/main_window/chat_view/font_manager + QML backend/models .h
│   ├── src/               # 对应 .cpp
│   ├── qml/               # QML 迁移脚手架;当前仅 main.qml 占位和空分层目录
│   ├── resources/         # fonts/ images/ style.qss resources.qrc
│   └── CMakeLists.txt
├── server/                 # epoll 服务端 (Linux)
│   ├── include/           # epoll_server/thread_pool/client_session/protocol/database .h
│   ├── src/               # 对应 .cpp
│   ├── third_party/nlohmann/json.hpp  # vendored
│   └── CMakeLists.txt
├── tests/                  # Google Test (FetchContent)
│   ├── test_protocol.cpp / test_database.cpp / test_message_handler.cpp
│   └── CMakeLists.txt
├── docs/
│   ├── specs/prd.md       # 产品契约
│   ├── specs/blueprint.md # 本文件
│   ├── protocol.md        # 通信协议规范
│   ├── INDEX.md           # 生成式上下文快照
│   ├── JOURNAL.md         # 实施日志
│   ├── legacy/            # 迁移前文档备份 (CONTRACT/ARCHITECTURE/TODO/README .legacy.md)
│   └── prompts/PROMPT-INDEX.md
├── AGENTS.md              # CDD agent 契约
├── TODO.md                # 执行计划
└── README.md              # runbook 入口
```

## Runbook

### Setup — Linux 服务端
```bash
# 依赖 (Ubuntu/Debian)
sudo apt install build-essential cmake libsqlite3-dev libssl-dev

# 编译
cd server && mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release && make -j$(nproc)
# 运行 (默认端口 8080)
./linuxchat_server [--port 9090] [--workers 4] [--db linuxchat.db]
```

### Setup — Windows 客户端
```powershell
# 前置: Qt 6.8.3 msvc2022_64, Visual Studio 2022, CMake 3.16+
# 注意: 如果系统未将 CMake 加入环境变量，请使用 Qt 附带的 CMake (通常在 C:\Qt\Tools\CMake_64\bin\cmake.exe)。
#      编译必须在「x64 Native Tools Command Prompt for VS 2022」或正确配置了 MSVC 环境变量的终端下进行。
cd client; mkdir build; cd build

# 步骤 1：生成工程 (替换为你本地的 CMake 路径和 Qt 路径)
& "C:\Qt\Tools\CMake_64\bin\cmake.exe" .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="C:\Qt\6.8.3\msvc2022_64"

# 步骤 2：编译构建
& "C:\Qt\Tools\CMake_64\bin\cmake.exe" --build . --config Release

# 步骤 3：运行
cd Release
./linuxchat_client.exe
```

### Dev / Test
```bash
# 服务端测试 (Google Test, 需先 build tests/)
cd tests && mkdir build && cd build
cmake .. && make -j && ctest --output-on-failure
```

### Deploy
- 服务端:Linux 机器运行 `linuxchat_server`，已部署在远程 Linux 主机 `120.55.63.32:8080`。确保端口在防火墙和安全组均已放行。
- 客户端:Windows 运行 exe，目前 IP 已写死为 `120.55.63.32`。
- **已知网络问题**: 主机连接后端时偶尔会出现连接失败的情况，通常多尝试几次即可成功（可能由于网络波动、TCP 握手超时、或是服务端并发 accept 丢包导致）。

## Observability

- **Logs**: 服务端已全面重构为使用 `spdlog`，支持高并发环境下的线程安全日志输出。格式 `[Component] message`(`[Server]`/`[Handler]`/`[Database]`/`[Protocol]`)。客户端使用 `qDebug()` 输出。
- **Metrics**: 无(课设范围)
- **Error taxonomy**: 见 `docs/protocol.md` 错误码表

## Security

- **密码**: SHA-256(OpenSSL)哈希存储,不存明文(注:见 Known Issue #3,API 废弃)
- **传输**: 明文 TCP(非 TLS),课设可接受
- **SQL 注入**: 全部 prepared statement(sqlite3_prepare_v2 + bind)
- **权限**: 课设范围无 RBAC

## Rollback Plan

- 每个修复 Step 独立 commit,可 `git revert` 单步回滚
- `docs/legacy/` 保留迁移前全文,文档层可还原
- Database schema 用 `CREATE TABLE IF NOT EXISTS`,无破坏性迁移

## 已知偏差(相对课设原文)

- 课设原文要求客户端为 **Linux gnome** 图形界面,本实现为 **Windows Qt6**(架构仍为 C/S,仅运行平台/框架不同)。见 PRD Open Questions #1。
