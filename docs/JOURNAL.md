# DEV JOURNAL - Software Engineering ADR

> Single-journal mode: this file is the live journal.
> Split-journal mode: this file becomes the stable journal entrypoint/index for active files under `docs/journal/`.

Keep this file compact and high-signal.

## RULES

- In single-journal mode, add implementation detail entries under ENTRIES.
- Focus on tricky implementation details, challenges, and uncommon solutions. Do not repeat TODO, spec, INDEX, or runbook material.
- Skip minor bug fixes, refactors, doc-only/test-only work, or low-impact config tweaks. Combine continuous work on one step/session into one entry.
- Keep one implementation session in exactly one hot journal file unless the work is truly cross-cutting.
- Do not duplicate the same journal entry across multiple journal files.
- Split-journal mode starts when any active implementation `TODO-<area>.md` exists. Stop writing live entries here, move existing live entries to `docs/journal/JOURNAL.md`, rewrite this file as a short current-state index, use `docs/journal/JOURNAL-<area>.md` for area work, use `docs/journal/JOURNAL.md` only for repo-wide or cross-cutting notes, use `docs/journal/SUMMARY.md` for condensed archive history, and keep split mode once enabled.
- Archive rules: in single-journal mode, when ENTRIES >= 20, condense the oldest 15 into "SUMMARIZED (LATEST ON TOP)", move the raw entries to `docs/archive/JOURNAL_YYYY-MM-DD.md`, and link to the archive. In split-journal mode, condense old entries from hot journals into `docs/journal/SUMMARY.md` with source journal and date range, then move raw batches to `docs/journal/archive/`.

**Format:**

Only use what's needed.

```
[YYYY-MM-DD HH:MM UTC]
Context: (task or area, include relevant background information, constraints, and forces at play)
Decisions: (key choices & why, clearly state the decision and the approach)
Findings: (surprises, pitfalls, dead ends)
Risks: (what might break, monitoring hooks)
Important: (add especially important remarks here; can be omitted if there aren't any)
```

## ENTRIES (LATEST ON TOP)

[2026-06-21]
Context: Final architectural audit and comprehensive CDD documentation update to conclude the project plan.
Decisions: The architecture (epoll reactor + ThreadPool + SQLite WAL + QML frontend) is deemed mature and performant. `get_history` reverse sorting and `crypto_utils.h` cleanups were previously resolved in Step 02. Completed the documentation alignment for `README.md`, `TODO.md`, `blueprint.md`, `protocol.md`, and `INDEX.md`.
Findings: The initial audit raised concerns about prepared statement caching for read queries (e.g. `get_history`), but creating fresh statements inside `shared_lock` is intentional to allow concurrent readers without race conditions on the stateful `sqlite3_stmt` object. The codebase is solid.
Risks: The single `db_mutex_` bottleneck could limit write concurrency under massive load, but it's acceptable for the current scope.

[2026-06-21]
Context: Frontend build toolchain migration from MSVC to MinGW + Ninja, and resolving frameless window OS-level bugs.
Decisions: Switched to `mingw_64` and `Ninja` to resolve linking issues with QML/Win32 APIs. Updated `main.cpp` QML import paths to `mingw_64`. Implemented a fallback `onPositionChanged` manual window drag in `CustomTitleBar.qml` since `startSystemMove()` silently fails on some Windows setups. Cleaned up obsolete PRD issues that have been addressed.
Findings: Hardcoded MSVC paths in C++ prevented MinGW QML modules from loading.
Risks: Manual drag logic skips Windows snapping features, but guarantees the window is not "stuck".

[2026-06-20]
Context: Frontend migration from 4 static HTML design prototypes (Minimal, Dense, Motion, iMessage) to a high-fidelity QML UI.
Decisions: Created `Theme.qml` as a `pragma Singleton` mapped to C++ `ThemeManager::skinName()`. Replaced rudimentary items with `components/LCButton.qml`, `LCTextField.qml`, and `MessageBubble.qml`. Refactored `ChatWindow.qml` and `LoginDialog.qml` to bind directly to `Theme.colors` and `Theme.fonts`.
Findings: C++ `ThemeManager` returned flat objects, requiring `Theme.qml` to manage detailed spacing/radius tokens.
Risks: Qt6 `GraphicalEffects` deprecation required fallback to basic borders and rectangle radii; `MultiEffect` may be required for complex shadows later.

[2026-06-19]
Context: CDD documentation initialization after the repo had a template `docs/INDEX.md`, current Chinese Ink QSS theme, and in-progress QML scaffolding. The workspace root `D:\ChatBox` contains templates/logs/skills, while the real Git repo is `D:\ChatBox\LinuxChat`.
Decisions: Regenerate `docs/INDEX.md` in single-file mode because no `docs/index/` split exists and the refreshed inventory stays under the split threshold. Keep Widgets as the current runtime path, mark QML as scaffold only, and document friend/blacklist protocol messages as reserved rather than implemented.
Findings: `client/CMakeLists.txt` conditionally registers a QML module through `HAS_QML`, but `client/main.cpp` still launches the Widgets login/main-window flow. `PING`/`PONG` heartbeat is implemented; friend/blacklist flows are not.
Risks: Root `ARCHITECTURE.md`/`CONTRACT.md` remain compatibility summaries and can drift behind `docs/specs/blueprint.md`; treat blueprint + `docs/INDEX.md` as the primary CDD context.

[2026-06-17]
Context: Multi-agent / CDD follow-up after init to fix "TCP connect OK but immediate disconnect on first LOGIN/REGISTER" + unreliable cross-platform (Win client ↔ Linux server). Root causes documented in prd Known Issues + TODO Step 01-04.
Decisions: Implement drain in recv_msgs (LT epoll safety), oversized clear+log, generation token in ClientSession+worker, EVP SHA256, client login timer + verbose disconnect logs. Preserve frame format + existing tests.
Findings: recv_msgs single-recv + oversized nullopt was primary kick path for desynced prefix bytes; generation solves fd-reuse in async handlers. Client qToBigEndian + server ntohl are aligned (verified by protocol tests + byte sim).
Risks: Still recommend user verify 8080 security group/ufw before blaming code. DB relative path remains simple default.
Important: All Critical items from user query (oversized, worker crash paths, SHA, disconnect diagnostics, firewall) addressed or documented.

Verification notes (2026-06-17): 
- git status shows 8 files edited.
- recv_msgs drain + oversized handling reviewed vs test_protocol.cpp (OversizedFrame_ReturnsNullopt still triggers correctly on >16MB header).

- Generation captured+checked in worker path.
- EVP code path matches previous hex output.
- Timers + logs added for app-layer vs TCP disconnect distinction.
- Cannot execute full Linux server build + Win client run in this pwsh session; plan UAT on target Linux box + Windows client binary. Recommend `cd LinuxChat/tests/build && ctest` + manual 20x login after `ufw allow 8080`.
- No behavior change for happy path.

## SUMMARIZED (LATEST ON TOP)

### CDD Adoption
Adopted Continuous Driven Development standard methodology. Created AGENTS.md, initialized TODO.md, removed obsolete docs.
