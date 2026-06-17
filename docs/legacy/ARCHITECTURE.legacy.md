# LinuxChat — Architecture

## System Overview

```
┌─────────────────────────────────────────────────────────────────┐
│  Windows Client (Qt6 + FluentUI)                                │
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
│  │  ├── accept() → ClientSession                           │    │
│  │  ├── recv_msgs() → parse frames                         │    │
│  │  └── pool_.enqueue() → dispatch to workers              │    │
│  └─────────────────────┬───────────────────────────────────┘    │
│                        │                                        │
│  ┌─────────────────────┴───────────────────────────────────┐    │
│  │ ThreadPool (N workers)                                   │    │
│  │  └── handle_message(session, json)                      │    │
│  │       ├── REGISTER / LOGIN → Database                   │    │
│  │       ├── BROADCAST → Database + broadcast()            │    │
│  │       ├── PRIVATE → Database + send_to_fd()             │    │
│  │       └── HISTORY_REQ → Database::get_history()         │    │
│  └─────────────────────┬───────────────────────────────────┘    │
│                        │                                        │
│  ┌─────────────────────┴───────────────────────────────────┐    │
│  │ Database (SQLite3, WAL mode, mutex-protected)           │    │
│  │  ├── users(username PK, password_hash, created_at)      │    │
│  │  └── messages(id, from_user, to_user, content, ts)      │    │
│  └─────────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────┘
```

## Data Flow

### Registration Flow
```
Client                          Server
  │                               │
  ├──REGISTER{from,pwd}──────────►│ ThreadPool worker:
  │                               │   1. SHA-256(password)
  │                               │   2. db.register_user()
  │◄──────LOGIN_OK{from}──────────┤   3. session.username = from
  │◄──────USER_LIST{data:[...]}───┤   4. broadcast USER_LIST
  │◄──────HISTORY_RESP{data:[]}───┤   5. push history (empty for new user)
```

### Broadcast Chat Flow
```
Alice                   Server                    Bob
  │                       │                        │
  ├──BROADCAST{from,      │                        │
  │   content}───────────►│ worker:
  │                       │  1. db.store_message(to="__room__")
  │                       │  2. add timestamp
  │◄──────────────────────┤  3. broadcast(msg)
  │  BROADCAST{from,      │                        │
  │   content, ts}────────┼───────────────────────►│
```

## Key Design Decisions

| Decision | Choice | Rationale |
|----------|--------|-----------|
| Event model | epoll (level-triggered) | Sufficient for course project scale; simpler than edge-triggered |
| Threading | Main epoll + ThreadPool workers | Non-blocking I/O on main thread; business logic on workers |
| Frame format | 4-byte BE length + JSON | Simple, solves TCP fragmentation; defined in protocol.md |
| Password storage | SHA-256 (OpenSSL) | Secure enough for course project; no plaintext storage |
| Database | SQLite3 WAL mode | Lightweight, no server process; WAL allows concurrent reads |
| Client UI | Qt6 Widgets + FluentUI | Modern look, C++ native, matches course requirement |
| Test framework | Google Test | Industry standard, FetchContent for easy setup |

## File Dependencies

```
server/
├── main.cpp                    ← depends on: epoll_server.h, database.h
├── include/
│   ├── client_session.h        ← standalone (no deps)
│   ├── thread_pool.h           ← standalone (no deps)
│   ├── protocol.h              ← depends on: client_session.h
│   ├── epoll_server.h          ← depends on: thread_pool.h, client_session.h
│   └── database.h              ← depends on: nlohmann/json, sqlite3
├── src/
│   ├── protocol.cpp            ← implements: protocol.h
│   ├── thread_pool.cpp         ← implements: thread_pool.h
│   ├── epoll_server.cpp        ← implements: epoll_server.h, uses: protocol.h
│   └── database.cpp            ← implements: database.h
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
    ├── style.qss               ← design system (immutable)
    └── resources.qrc           ← Qt resource file (immutable)
```
