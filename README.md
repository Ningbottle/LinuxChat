# LinuxChat

LinuxChat is a modern, high-performance client-server chat application designed with a robust C++ `epoll`-based backend and a sleek, ethereal QML frontend.

## Architecture

### Backend (Server)
- **Core Technology**: C++17, epoll (Linux), multi-threading.
- **Concurrency Model**: A main reactor (EpollServer) dispatches connection events to a thread pool for processing.
- **Protocol**: Custom JSON-over-TCP protocol with proper length-prefixed framing.
- **Storage**: SQLite for user accounts, session state, and offline messages.

### Frontend (Client)
- **Core Technology**: C++17, Qt 6.8.
- **UI Framework**: Qt Quick (QML) using a custom "Ethereal Frosted Glass" design system.
- **Platform**: Cross-platform (developed and tested on Windows).
- **Design Paradigm**: CDD (Chat-Driven Development) and strict separation of backend logic from UI rendering.

## Setup & Build

### Requirements
- CMake 3.16+
- Qt 6.8+
- MinGW 13.1.0 64-bit / GCC 9+
- Ninja (Recommended)

### Build Client (Windows)
```powershell
cd client
mkdir build; cd build

# 步骤 1：生成 Ninja 工程 (使用 MinGW)
cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release ..

# 步骤 2：编译构建
cmake --build .
```

## Development Workflow

The project is developed using the **CDD (Continuous Documentation-Driven Development)** framework.
- Core architecture docs: `docs/specs/blueprint.md`, `docs/specs/prd.md`
- Working agents instructions: `AGENTS.md`
- Task tracking: `TODO.md`
- Progress logging: `docs/JOURNAL.md`

All architectural changes must be reviewed against `AGENTS.md` rules and recorded in `JOURNAL.md`.
