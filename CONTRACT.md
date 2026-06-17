# LinuxChat — Project Contract

> Version: 1.0
> Status: Active
> Last Updated: 2026-06-16

---

## 1. Project Overview

**LinuxChat** is a network instant messaging tool based on Linux platform with a Windows Qt client.
It implements real-time chat over TCP using a custom JSON-over-TCP protocol.

## 2. Technology Stack

| Layer | Technology | Version |
|-------|-----------|---------|
| Server Runtime | Linux (epoll) | Kernel 4.x+ |
| Server Language | C++ | C++17 |
| Client Runtime | Windows | 10/11 |
| Client Framework | Qt (Widgets + FluentUI) | Qt 6.x |
| Client Language | C++ | C++17 |
| Database | SQLite3 | WAL mode |
| Crypto | OpenSSL (SHA-256) | 1.1+ |
| Build System | CMake | 3.16+ |
| JSON | nlohmann/json | 3.x |
| Test Framework | Google Test | 1.14+ |

## 3. Module Boundaries

### Server (`LinuxChat/server/`)

| Module | File(s) | Responsibility |
|--------|---------|----------------|
| Protocol | `include/protocol.h`, `src/protocol.cpp` | JSON-over-TCP framing (4-byte BE prefix + JSON body) |
| ThreadPool | `include/thread_pool.h`, `src/thread_pool.cpp` | Fixed-size task queue for worker threads |
| ClientSession | `include/client_session.h` | Per-connection state (fd, username, recv_buf) |
| EpollServer | `include/epoll_server.h`, `src/epoll_server.cpp` | epoll event loop, connection management, message dispatch |
| Database | `include/database.h`, `src/database.cpp` | SQLite3 persistence (users, messages) |
| Main/Handler | `main.cpp` | Message handler logic, SHA-256, signal handling |

### Client (`LinuxChat/client/`)

| Module | File(s) | Responsibility |
|--------|---------|----------------|
| ChatClient | `include/chat_client.h`, `src/chat_client.cpp` | TCP protocol encapsulation, Qt signals |
| LoginDialog | `include/login_dialog.h`, `src/login_dialog.cpp` | Login/Register UI |
| MainWindow | `include/main_window.h`, `src/main_window.cpp` | Main chat window layout |
| ChatView | `include/chat_view.h`, `src/chat_view.cpp` | Message display + input area |
| Main | `main.cpp` | Application entry, style loading |

### Tests (`LinuxChat/tests/`)

| Module | File | Tests |
|--------|------|-------|
| Protocol | `test_protocol.cpp` | Frame encoding/decoding, edge cases |
| Database | `test_database.cpp` | CRUD operations, history queries |
| Handler | `test_message_handler.cpp` | Message handler business logic |

## 4. Interface Contracts

### Database API
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

### MessageHandler Signature
```cpp
void handle_message(ClientSession& session, const nlohmann::json& msg);
void handle_disconnect(ClientSession& session);
```

### ChatClient Signals
```
login_ok(username), error_received(code, content),
broadcast_received(from, content, timestamp),
private_received(from, to, content, timestamp),
user_list_updated(users), history_received(to, messages),
notify_received(content)
```

## 5. Constraints

- **Immutable files**: `protocol.cpp`, `thread_pool.cpp`, all server `.h` headers, `style.qss`, `resources.qrc`, `protocol.md`
- **Style rule**: All client UI styles via QSS objectName selectors; never inline `setStyleSheet()`
- **Protocol**: `to="__room__"` denotes broadcast history; `to=username` denotes private history
- **Feature scope (Phase 1)**: Register, Login, Broadcast, Private, OnlineUserList, History
- **Deferred**: Friend system, blacklist, offline messages, file transfer

## 6. Acceptance Criteria

- [ ] Server compiles and runs on Linux
- [ ] Client compiles and runs on Windows (Qt6)
- [ ] All protocol tests pass
- [ ] All database tests pass
- [ ] All message handler tests pass
- [ ] Register → Login → Broadcast → Private chat flow works end-to-end
- [ ] History messages load on login
- [ ] Online user list updates on connect/disconnect
