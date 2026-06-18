# LinuxChat — Project Contract

> Version: 1.1
> Status: Active
> Last Updated: 2026-06-17

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
| Client Framework | Qt (Widgets + Glassmorphism) | Qt 6.x |
| Client Language | C++ | C++17 |
| Database | SQLite3 | WAL mode |
| Crypto | OpenSSL (SHA-256 EVP) | 3.x |
| Build System | CMake | 3.16+ |
| JSON | nlohmann/json | 3.x |
| Test Framework | Google Test | 1.14+ |

## 3. Module Boundaries

### Server (`LinuxChat/server/`)

| Module | File(s) | Responsibility |
|--------|---------|----------------|
| Protocol | `include/protocol.h`, `src/protocol.cpp` | JSON-over-TCP framing (4-byte BE prefix + JSON body), EAGAIN 重试 |
| ThreadPool | `include/thread_pool.h`, `src/thread_pool.cpp` | Fixed-size task queue for worker threads |
| ClientSession | `include/client_session.h` | Per-connection state (fd, username, recv_buf, generation) |
| EpollServer | `include/epoll_server.h`, `src/epoll_server.cpp` | epoll event loop, connection management, shared_ptr sessions |
| Database | `include/database.h`, `src/database.cpp` | SQLite3 persistence (users, messages) |
| MessageRouter | `include/message_router.h`, `src/message_router.cpp` | 消息路由、SHA-256、在线用户管理 |

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
| Protocol | `test_protocol.cpp` | 19 tests: Frame encoding/decoding, edge cases |
| Database | `test_database.cpp` | 27 tests: CRUD operations, history queries |
| MessageHandler | `test_message_handler.cpp` | 15 tests: Handler business logic |
| MessageRouter | `test_message_router.cpp` | 27 tests: Router dispatch, auth, online management |
| Crypto | `test_crypto.cpp` | 9 tests: SHA-256 EVP API |
| ThreadPool | `test_thread_pool.cpp` | 11 tests: Thread pool task dispatch |
| **Total** | | **108 tests** |

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

### MessageRouter API
```cpp
class MessageRouter {
    explicit MessageRouter(Database& db);
    void set_server(EpollServer* server);
    void route(ClientSession& session, const nlohmann::json& msg);

    // Individual handlers (public for testing)
    void handle_register(ClientSession& session, const nlohmann::json& msg);
    void handle_login(ClientSession& session, const nlohmann::json& msg);
    void handle_logout(ClientSession& session);
    void handle_broadcast(ClientSession& session, const nlohmann::json& msg);
    void handle_private(ClientSession& session, const nlohmann::json& msg);
    void handle_history_req(ClientSession& session, const nlohmann::json& msg);

    // Online user management
    nlohmann::json make_user_list_msg();
    void broadcast_user_list();
    bool is_user_online(const std::string& username) const;
};
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

- **Immutable files**: `thread_pool.cpp`, all server `.h` headers, `resources.qrc`, `protocol.md`
- **Modified files** (已修改，不再是 immutable): `protocol.cpp` (EAGAIN 重试), `style.qss` (Glassmorphism 重写)
- **Style rule**: All client UI styles via QSS objectName selectors; never inline `setStyleSheet()`
- **Protocol**: `to="__room__"` denotes broadcast history; `to=username` denotes private history
- **Feature scope (Phase 1)**: Register, Login, Broadcast, Private, OnlineUserList, History
- **Deferred**: Friend system, blacklist, offline messages, file transfer

## 6. Acceptance Criteria

- [x] Server compiles and runs on Linux
- [x] Client compiles and runs on Windows (Qt6)
- [x] All 108 protocol/database/handler/router/crypto/thread_pool tests pass
- [x] Register → Login → Broadcast → Private chat flow works end-to-end
- [x] History messages load on login
- [x] Online user list updates on connect/disconnect
- [x] MessageRouter 正确路由所有消息类型
- [x] shared_ptr 消除 use-after-free
- [x] broadcast 锁优化（复制-释放-发送）
- [x] write_all EAGAIN 重试机制

## 7. Protocol Version

- **Version**: 1.0 (JSON-over-TCP)
- **Frame format**: 4-byte big-endian length prefix + JSON body
- **Max frame size**: 16 MB
- **Message types**: REGISTER, LOGIN, LOGOUT, BROADCAST, PRIVATE, HISTORY_REQ, HISTORY_RESP, USER_LIST, NOTIFY, ERROR, LOGIN_OK
- **Changes in v1.0.1**: `protocol.cpp` 添加 EAGAIN/EINTR 重试，`send_all` 循环处理部分写入
