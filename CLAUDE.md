# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

LinuxChat（瓶子交流器）— C/S 架构即时通讯：Linux epoll 服务端 + Windows Qt6 客户端，自定义 JSON-over-TCP 协议。《Linux 操作系统与程序设计》课程设计作品。

## Build Commands

### Server (Linux)
```bash
cd server && mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
./linuxchat_server              # 默认端口 8080
./linuxchat_server --port 9090  # 自定义端口
```

### Client (Windows)
```powershell
cd client
cmake -B build -G "Visual Studio 18 2026" -DCMAKE_PREFIX_PATH="C:/Qt/6.8.3/msvc2022_64"
cmake --build build --config Release
cd build/Release
./linuxchat_client.exe          # 正常登录模式
./linuxchat_client.exe --test-chat  # 跳过登录，直连 mock 数据测试 QML
```

### Tests
```bash
cd tests && mkdir -p build && cd build
cmake ..
make -j$(nproc)
ctest --output-on-failure
# 运行单个测试:
./test_protocol
./test_database
./test_handler
```

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│  Windows Client (Qt6 Widgets + QSS)                         │
│  LoginDialog → ChatClient (QTcpSocket) → MainWindow         │
│                  │ JSON-over-TCP (4B BE length + JSON)       │
└──────────────────┼──────────────────────────────────────────┘
                   │
┌──────────────────┼──────────────────────────────────────────┐
│  Linux Server (epoll LT + ThreadPool)                        │
│  EpollServer::epoll_wait → drain recv → pool_.enqueue        │
│  ThreadPool workers → handle_message → Database (SQLite3)    │
└─────────────────────────────────────────────────────────────┘
```

**Event model**: epoll level-triggered. Main thread runs epoll_wait + accept + recv drain. Business logic dispatched to fixed ThreadPool workers.

**Protocol**: 4-byte big-endian length prefix + JSON body, 16MB max. Message types: REGISTER, LOGIN, BROADCAST, PRIVATE, HISTORY_REQ/RESP, USER_LIST, NOTIFY, LOGOUT.

**Database**: SQLite3 WAL mode. Two tables: `users(username PK, password_hash, created_at)`, `messages(id, from_user, to_user, content, ts)`. `to="__room__"` for broadcast history.

## Key Files

| File | Role |
|------|------|
| `server/main.cpp` | Entry point, MessageRouter wiring, CLI parsing, signal handling |
| `server/src/epoll_server.cpp` | epoll event loop, connection mgmt, recv drain, dispatch to workers |
| `server/src/protocol.cpp` | Frame encode/decode, `recv_msgs` drain loop |
| `server/include/client_session.h` | Per-connection state: fd, username, recv_buf, generation counter |
| `server/include/thread_pool.h` | Fixed-size worker pool with condition variable |
| `server/src/database.cpp` | SQLite3 CRUD, mutex-protected |
| `client/src/chat_client.cpp` | TCP protocol encapsulation, Qt signals for all message types |
| `client/src/login_dialog.cpp` | Login/Register UI, connect + login timeout handling |
| `client/src/main_window.cpp` | Sidebar + chat tabs layout, settings, globe panel |
| `client/src/chat_view.cpp` | Message bubbles, input area, system messages |
| `client/src/backend.cpp` | QML facade wrapping ChatClient for migration scaffold |
| `client/src/message_model.cpp` | QML message list model |
| `client/src/user_model.cpp` | QML online-user list model |
| `client/qml/main.qml` | Minimal QML pipeline placeholder |
| `client/resources/style.qss` | Single source of truth for all client UI styles |

## Conventions

- **QSS-only styling**: All client styles via `style.qss` using QSS objectName selectors. Never use inline `setStyleSheet()`.
- **High-care files**: `protocol.cpp`, `thread_pool.cpp`, server public headers, `style.qss`, `resources.qrc`, `docs/protocol.md`. Check `docs/specs/blueprint.md` and `docs/INDEX.md` before changing their contracts.
- **CDD workflow**: Follow AGENTS.md contract. Use TODO.md as execution index. Log non-trivial changes to docs/JOURNAL.md.
- **Logging format**: `[Component] LEVEL EventName key1={value1}` — stable, grep-friendly.
- **Shared JSON**: Both server and client use vendored `server/third_party/nlohmann/json.hpp`.

## Current State

TODO Step 01-07 已完成：epoll drain、fd generation guard、OpenSSL EVP SHA-256、MessageRouter、连接防重入、108 个测试和 Widgets/QSS UI 均已落地。当前客户端运行入口仍为 Qt Widgets + 水墨国风 QSS；Step 08 QML 迁移脚手架进行中（Backend、MessageModel、UserModel、`qml/main.qml`、`HAS_QML` 可选构建），尚未替代 `client/main.cpp` 的 Widgets 登录流程。`docs/INDEX.md` 已初始化为真实项目上下文索引。
