# LinuxChat — Architecture

## System Overview

```
┌─────────────────────────────────────────────────────────────────┐
│  Windows Client (Qt6 + Glassmorphism Dark)                      │
│  ┌────────────┐  ┌──────────────┐  ┌────────────────────────┐  │
│  │LoginDialog  │  │ ChatClient   │  │ MainWindow             │  │
│  │ (login/reg) │  │ (TCP+JSON)   │  │ (sidebar+chatTabs)     │  │
│  └─────┬──────┘  └──────┬───────┘  └───────────┬────────────┘  │
│        │                │                       │               │
│        └────────────────┼───────────────────────┘               │
│                         │ QTcpSocket                            │
└─────────────────────────┼───────────────────────────────────────┘
                          │ JSON-over-TCP (4B BE prefix)
                          │
┌─────────────────────────┼───────────────────────────────────────┐
│  Linux Server (epoll)   │                                       │
│  ┌──────────────────────┴──────────────────────────────────┐    │
│  │ EpollServer (main thread: epoll_wait loop)              │    │
│  │  ├── accept() → ClientSession (shared_ptr)              │    │
│  │  ├── recv_msgs() → parse frames (drain until EAGAIN)    │    │
│  │  └── pool_.enqueue() → dispatch to workers              │    │
│  └─────────────────────┬───────────────────────────────────┘    │
│                        │                                        │
│  ┌─────────────────────┴───────────────────────────────────┐    │
│  │ ThreadPool (N workers)                                   │    │
│  │  └── MessageRouter::route(session, json)                │    │
│  │       ├── REGISTER/LOGIN → sha256_hex + Database        │    │
│  │       ├── BROADCAST → Database + broadcast()            │    │
│  │       ├── PRIVATE → Database + send_to_fd()             │    │
│  │       ├── HISTORY_REQ → Database::get_history()         │    │
│  │       └── LOGOUT → online_users_ + broadcast_user_list  │    │
│  └─────────────────────┬───────────────────────────────────┘    │
│                        │                                        │
│  ┌─────────────────────┴───────────────────────────────────┐    │
│  │ Database (SQLite3, WAL mode, mutex-protected)           │    │
│  │  ├── users(username PK, password_hash, created_at)      │    │
│  │  └── messages(id, from_user, to_user, content, ts)      │    │
│  └─────────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────┘
```

## Architecture Diagram (Simplified)

```
main.cpp
  ├── Database        (SQLite3 WAL, 持久化存储)
  ├── EpollServer     (epoll LT 事件循环 + ThreadPool)
  └── MessageRouter   (消息路由: 类型分发 + 在线用户管理)
       ├── route()           → 根据 type 字段分发
       ├── handle_register() → SHA-256 + Database
       ├── handle_login()    → 认证 + finish_login
       ├── handle_broadcast()→ Database + broadcast
       ├── handle_private()  → Database + send_to_fd
       └── handle_logout()   → online_users_ + notify
```

## Client Architecture

```
MainWindow (主窗口)
  ├── ChatClient (TCP 协议封装)
  │    └── QTcpSocket → JSON-over-TCP
  ├── LoginDialog (登录/注册 UI)
  ├── ChatView (消息显示 + 输入)
  └── Sidebar (用户列表 + 会话切换)
```

## Data Flow

### Registration Flow
```
Client                          Server
  │                               │
  ├──REGISTER{from,pwd}──────────►│ ThreadPool worker:
  │                               │   1. MessageRouter::route()
  │                               │   2. sha256_hex(password)
  │                               │   3. db.register_user()
  │◄──────LOGIN_OK{from}──────────┤   4. finish_login()
  │◄──────USER_LIST{data:[...]}───┤   5. broadcast_user_list()
  │◄──────HISTORY_RESP{data:[]}───┤   6. push_history()
```

### Broadcast Chat Flow
```
Alice                   Server                    Bob
  │                       │                        │
  ├──BROADCAST{from,      │                        │
  │   content}───────────►│ worker:
  │                       │  1. MessageRouter::route()
  │                       │  2. db.store_message(to="__room__")
  │                       │  3. add timestamp
  │◄──────────────────────┤  4. broadcast(msg)
  │  BROADCAST{from,      │                        │
  │   content, ts}────────┼───────────────────────►│
```

## Key Design Decisions

| Decision | Choice | Rationale |
|----------|--------|-----------|
| Event model | epoll (level-triggered) | Sufficient for course project scale; simpler than edge-triggered |
| Threading | Main epoll + ThreadPool workers | Non-blocking I/O on main thread; business logic on workers |
| Message routing | MessageRouter class | 从 main.cpp 提取，430 行 → 113 行，可测试性大幅提升 |
| Frame format | 4-byte BE length + JSON | Simple, solves TCP fragmentation; defined in protocol.md |
| Password storage | SHA-256 (OpenSSL EVP API) | OpenSSL 3.x 兼容，无废弃 API 告警 |
| Database | SQLite3 WAL mode | Lightweight, no server process; WAL allows concurrent reads |
| Session safety | shared_ptr\<ClientSession\> | 消除 use-after-free，generation token 防止 fd 复用串话 |
| Client UI | Qt6 Widgets + Glassmorphism Dark | 现代暗色主题，半透明毛玻璃效果 |
| Test framework | Google Test | Industry standard, FetchContent for easy setup |

## CDD (Chat-Driven-Development) 工作流

本项目采用 CDD 工作流：

1. **AGENTS.md** — 定义 AI 代理行为规范
2. **docs/specs/prd.md** — 产品需求文档
3. **docs/specs/blueprint.md** — 架构蓝图
4. **CONTRACT.md** — 项目合约（不可变约束）
5. **TODO.md** — 执行计划（Step-based）
6. **docs/JOURNAL.md** — 实施日志

工作流命令：`$cdd-boot` → `$cdd-audit` → `$cdd-plan` + `$cdd-implement` → `$cdd-maintain`

## File Dependencies

```
server/
├── main.cpp                    ← depends on: epoll_server.h, database.h, message_router.h
├── include/
│   ├── client_session.h        ← standalone (no deps)
│   ├── thread_pool.h           ← standalone (no deps)
│   ├── protocol.h              ← depends on: client_session.h
│   ├── epoll_server.h          ← depends on: thread_pool.h, client_session.h
│   ├── database.h              ← depends on: nlohmann/json, sqlite3
│   └── message_router.h        ← depends on: database.h, epoll_server.h
├── src/
│   ├── protocol.cpp            ← implements: protocol.h
│   ├── thread_pool.cpp         ← implements: thread_pool.h
│   ├── epoll_server.cpp        ← implements: epoll_server.h, uses: protocol.h
│   ├── database.cpp            ← implements: database.h
│   └── message_router.cpp      ← implements: message_router.h (SHA-256 + 路由)
└── third_party/nlohmann/
    └── json.hpp                ← vendored dependency

client/
├── main.cpp                    ← depends on: login_dialog.h, main_window.h, chat_client.h
├── include/
│   ├── chat_client.h           ← depends on: Qt6 Network, nlohmann/json
│   ├── login_dialog.h          ← depends on: Qt6 Widgets, chat_client.h
│   ├── main_window.h           ← depends on: Qt6 Widgets, chat_client.h, chat_view.h
│   └── chat_view.h             ← depends on: Qt6 Widgets
├── src/
│   ├── chat_client.cpp         ← implements: chat_client.h
│   ├── login_dialog.cpp        ← implements: login_dialog.h
│   ├── main_window.cpp         ← implements: main_window.h
│   └── chat_view.cpp           ← implements: chat_view.h
└── resources/
    ├── style.qss               ← Glassmorphism dark theme
    └── resources.qrc           ← Qt resource file

tests/
├── test_protocol.cpp           ← 19 tests (frame encode/decode)
├── test_database.cpp           ← 27 tests (CRUD, history)
├── test_message_handler.cpp    ← 15 tests (handler logic)
├── test_message_router.cpp     ← 27 tests (router logic)
├── test_crypto.cpp             ← 9 tests (SHA-256)
└── test_thread_pool.cpp        ← 11 tests (thread pool)
                               ─────────────────────────
                               Total: 108 tests
```
