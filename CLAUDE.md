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
mkdir build; cd build
cmake .. -G "Visual Studio 18 2026" -DCMAKE_PREFIX_PATH="D:/Qt/6.8.0/msvc2022_64"
cmake --build . --config Release
cd Release
./linuxchat_client.exe          # 正常登录模式
./linuxchat_client.exe --test-chat  # 跳过登录，直连 mock 数据测试 QSS
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
| `server/main.cpp` | Entry point, message handler (`handle_message`), SHA-256 (EVP API), signal handling |
| `server/src/epoll_server.cpp` | epoll event loop, connection mgmt, recv drain, dispatch to workers |
| `server/src/protocol.cpp` | Frame encode/decode, `recv_msgs` drain loop |
| `server/include/client_session.h` | Per-connection state: fd, username, recv_buf, generation counter |
| `server/include/thread_pool.h` | Fixed-size worker pool with condition variable |
| `server/src/database.cpp` | SQLite3 CRUD, mutex-protected |
| `client/src/chat_client.cpp` | TCP protocol encapsulation, Qt signals for all message types |
| `client/src/login_dialog.cpp` | Login/Register UI, connect + login timeout handling |
| `client/src/main_window.cpp` | Sidebar + chat tabs layout, settings, globe panel |
| `client/src/chat_view.cpp` | Message bubbles, input area, system messages |
| `client/resources/style.qss` | Single source of truth for all client UI styles |

## Conventions

- **QSS-only styling**: All client styles via `style.qss` using QSS objectName selectors. Never use inline `setStyleSheet()`.
- **Immutable files**: `protocol.cpp`, `thread_pool.cpp`, all server `.h` headers, `style.qss`, `resources.qrc`, `docs/protocol.md`.
- **CDD workflow**: Follow AGENTS.md contract. Use TODO.md as execution index. Log non-trivial changes to docs/JOURNAL.md.
- **Logging format**: `[Component] LEVEL EventName key1={value1}` — stable, grep-friendly.
- **Shared JSON**: Both server and client use vendored `server/third_party/nlohmann/json.hpp`.

## Current State

CDD Phase 1 审计发现 3 个连接层缺陷（Step 01-03 in TODO.md）：epoll drain 不足导致帧错位、fd 复用导致 worker 串话、SHA-256 使用废弃 API。Step 01-03 代码修复已完成，UAT 待验证。Step 05 QSS 重构进行中。
