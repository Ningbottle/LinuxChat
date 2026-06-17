# LinuxChat — TODO

> Tracks implementation progress. Update status after each task.

## Phase 1: Server Core

| # | Task | Status | Notes |
|---|------|--------|-------|
| 0 | CDD Project Init | ✅ | CONTRACT.md, TODO.md, ARCHITECTURE.md |
| 1 | Test Infrastructure | ✅ | tests/CMakeLists.txt + test_protocol.cpp |
| 2a | Database Tests (Red) | ✅ | test_database.cpp |
| 2b | Database Implementation (Green) | ✅ | database.h + database.cpp |
| 3 | epoll_server.cpp | ✅ | Event loop, connection mgmt, dispatch |
| 4 | server/main.cpp | ✅ | Message handler, SHA-256, signals |
| 5 | Handler Tests | ✅ | test_message_handler.cpp |

## Phase 2: Client

| # | Task | Status | Notes |
|---|------|--------|-------|
| 6 | CMake Qt6 Upgrade | ✅ | client/CMakeLists.txt modifications |
| 7 | ChatClient Protocol | ✅ | chat_client.h + chat_client.cpp |
| 8 | UI Components | ✅ | LoginDialog, MainWindow, ChatView |
| 9 | Client main.cpp | ✅ | App entry, style loading |

## Phase 3: Integration

| # | Task | Status | Notes |
|---|------|--------|-------|
| 10 | Code Review | ✅ | 3-way review: server/client/tests |
| 11 | Polish & Fixes | ✅ | Fixed deadlock, QRC path, QSS rules, includes |
