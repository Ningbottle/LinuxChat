# TODO

This file is the single source of truth for all planned, in-progress, and completed work.
Read this file before starting any session.

## Current Steps

### Step 00: Automated Checks & UAT
**Status:** `[x]` Completed
**Goal:** Ensure the project meets baseline health requirements.

- `[x]` Code compiles (`mkdir build && cd build && cmake .. && make`)
- `[x]` UI tests/loads properly (Client connects)

### Step 01: Finalize QML ChatWindow UI
**Status:** `[x]` Completed
**Goal:** Finish the QML rewrite of the main chat interface.

- `[x]` Implement ChatWindow.qml structure
- `[x]` Port Contact list to QML
- `[x]` Port Chat view (messages) to QML

### Step 02: Fix Server Logic Bugs
**Status:** `[x]` Completed
**Goal:** Fix history order, bidirectional private chat, const-correctness, and remove dead code.

#### Subtasks
- `[x]` Fix `get_history` query order and private history direction
- `[x]` Fix `is_rate_limited` const-correctness in `message_router`
- `[x]` Remove dead code (`crypto_utils.h`, `Database::verify_user`)
- `[x]` Update blueprint payload limit to 256KB

#### Verification
- `[x]` Automated tests pass (server compiles)
- `[x]` Server runs and history displays newest messages first

### Step 03: Architectural Audit & CDD Documentation
**Status:** `[x]` Completed
**Goal:** Align all project documentation with the finalized QML frontend and stabilized server logic.

#### Subtasks
- `[x]` Update `README.md` to reflect Chat-Driven Development (CDD).
- `[x]` Update `docs/INDEX.md` UI state and component diagrams to purely QML architecture.
- `[x]` Update `docs/specs/blueprint.md` with Windows MinGW/Ninja build instructions.
- `[x]` Update `docs/protocol.md` explicit maximum frame size (256KB).
- `[x]` Write audit completion entry in `docs/JOURNAL.md`.

#### Verification
- `[x]` Project docs are mutually consistent and match current codebase reality.

---

## Step Template

```markdown
### Step XX: [Brief Description]
**Status:** `[ ]` Not Started | `[/]` In Progress | `[x]` Completed
**Goal:** What this step accomplishes.

#### Subtasks
- `[ ]` Task 1
- `[ ]` Task 2

#### Verification
- `[ ]` Automated tests pass
- `[ ]` User Acceptance Testing (UAT) completed
```
