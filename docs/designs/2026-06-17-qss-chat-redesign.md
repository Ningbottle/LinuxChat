# LinuxChat Qt6 Client — Modern QSS Visual System Redesign
**Design Document** (Open Design Process Output)  
**ID**: grok-design-87197455  
**Date**: 2026-06-17  
**Author**: Systems Architect (Grok Build subagent)  
**Status**: Proposed / Ready for PR slicing  
**Scope**: Primary deliverable = complete overhaul of `client/resources/style.qss` + minimal supporting C++ for testability, objectNames, custom-paint reduction. Targets Windows Qt6 client (Linux epoll C++ server unchanged).

---

## Overview

This document is the direct output of a rigorous **open design process** for completely redesigning the visual system of the LinuxChat (瓶子交流器) Qt6 Widgets chat client.

**Core Problem**: The current "newspaper retro + wisteria waterfall" aesthetic (米色 `#faf9f7` backgrounds, `#44403c` stone colors, 2px radii, LXGW WenKai + Newsreader fonts, wisteria SVG paint, subtle paper texture) is repeatedly called out by the user as unacceptable:
- "还是太丑了"
- "完全没有遵循 open design"

Exact current implementation (from full source read):
- `resources/style.qss:19` `QDialog { background-color: #faf9f7; ... }`
- `style.qss:24-29` `QFrame#sidebar { background-color: #faf9f7; border-right: 1px solid #e7e5e4; border-radius: 0px; }`
- `style.qss:106-110` `QPushButton#primaryBtn { background-color: #44403c; border-radius: 2px; ... }`
- `style.qss:344-371` `#bubbleOther` / `#bubbleSelf` with 2px radius + stone/beige
- `style.qss:501` `QLabel#topBarTitle { font-family: "Newsreader" ... }`
- `style.qss:559` avatars `border-radius: 14px` (hardcoded 28px)
- `login_dialog.cpp:195-238` (paintEvent + `drawNewspaperBackground` + `drawGlobe` using `:/images/globe.svg`)
- `main_window.cpp:165-195` (paintEvent + `drawWisteriaBackground` tiling `:/images/wisteria-pattern.svg` at opacity 0.35, clipped right of 240px sidebar)
- `chat_view.cpp:172-174` **inline** `avatar->setStyleSheet(...)` (direct violation of AGENTS.md + CONTRACT.md "never inline setStyleSheet")
- `main.cpp:50-81` LoginDialog → exec loop before MainWindow

The redesign **must be radically different**: fresh, professional, modern, high-contrast, immediately testable, QSS-centric.

**Mandatory Deliverable Features**:
1. `--test-chat` (aliases: `--direct-chat`, `-t`) CLI flag: bypasses `LoginDialog` entirely, launches `MainWindow` + `ChatView` directly with rich mock data.
2. QSS is still the **single source of truth** (per AGENTS.md, CONTRACT.md line 92). CONTRACT.md immutable list for style.qss will be explicitly updated in PR5 as part of contract evolution.
3. Fast iteration: change QSS → rebuild/run with `-t` → see beauty immediately (no server, no login). Paint gating ensures clean surfaces from PR1.
4. Minimal C++: only for new objectNames, paint reduction/gating, bypass entrypoint, mock population.

**Open Design Process Followed** (this doc is its artifact):
- Full mandatory exploration first (see tool history): `read_file` on *every* listed critical absolute path (style.qss entire + 10+ others), `list_dir`, multiple `grep` for objectNames/selectors/paintEvent/inline styles.
- Mobilized non-conflicting skills (see dedicated section below).
- Deliberate divergence from `docs/superpowers/specs/2026-06-16-bottle-messenger-ui-design.md` and plan.
- Reference-design-contract against PRD NFR4 (which still references old style), old CONTRACT immutable notes.
- Theme-factory + frontend-design principles adapted to Qt Widgets QSS constraints (no real backdrop-filter, limited gradients, property-based selectors).
- Testability-first (TDD/C DD influence): bypass as enabler for visual iteration.
- Alternatives rigorously considered.

The result is a **professional modern design system** (dark slate foundation + indigo accent) that is beautiful, high-contrast, spacious-yet-dense, state-complete, and Windows/Qt6/high-DPI friendly.

---

## Background & Motivation

User feedback is explicit and repeated: the current UI fails basic visual quality and does not follow open, modern design principles.

Current aesthetic (exact cites):
- Beige newspaper: `#faf9f7` (QDialog, central, messageArea, sidebar — style.qss:19,25,239,417,520).
- Stone/earthy: `#44403c` primary buttons + self-bubbles (style.qss:106,359), `#78776c`, `#1c1917` text.
- Tiny 2px radii everywhere (buttons:109, inputs:181, bubbles:346, tabs:209, menus:314, badges:409...).
- Serif + calligraphic fonts (LXGW WenKai body, Newsreader titles) + letter-spacing + uppercase section headers (style.qss:12,20,501).
- Decorative overlays: newspaper noise + globe (login_dialog.cpp:202-237), wisteria SVG tiling (main_window.cpp:172-194).
- Paper/subtle texture feel + low contrast in places.
- Avatar hack: inline setStyleSheet (chat_view.cpp:172) instead of QSS objectName.
- Fixed sizes (login 440x560, main 960x640, sidebar 240px).
- Scrollbars 4px thin but retro-colored.

These derive directly from:
- `docs/superpowers/specs/2026-06-16-bottle-messenger-ui-design.md` (full read): "报纸复古 + 紫藤萝瀑布", palette table with 石墨 `#44403c`, background `#faf9f7`, radii 2px, specific SVG details.
- Old plan `docs/superpowers/plans/2026-06-16-bottle-messenger-ui-redesign.md`.
- NFR4 in `docs/specs/prd.md:64` ("报纸复古 + 紫藤萝瀑布视觉风格").

Result: "还是太丑了". Open design process deliberately started by *reading and rejecting* the prior contract/spec.

Motivation for redesign:
- Professional production-grade feel for a course project that must demo well.
- QSS iteration must be *fast and delightful*.
- Follow CDD/AGENTS: QSS single source, no inline styles, respect immutable notes but allow controlled evolution via redesign.
- Enable test-driven visual development via bypass.

---

## Goals & Non-Goals

### Goals (G)
- G1: Radically new, beautiful, professional modern QSS design (dark slate + indigo/emerald accents, 6-10px radii, excellent WCAG contrast, full interactive states).
- G2: `--test-chat` / `-t` / `--direct-chat` flag that launches directly into MainWindow + ChatView(s) populated with rich, varied mock data (self/other messages of differing lengths, system notices, timestamps, multiple private tabs, user list) **+ live send echo**.
- G3: QSS as fast iteration vehicle — visual changes testable in <30s cycle without server/login. (See Test Mode Special Cases for reload reality.)
- G4: Preserve functionality (chat flows, disconnect → returnToLogin, history, private tabs). Minimal C++ delta.
- G5: Improve validation UX lightly in LoginDialog (error states via QSS) even though bypassed for test.
- G6: High-DPI correct (already enabled in main.cpp:22), Windows Qt6 only. All dimensions tested at scale factors.
- G7: Follow open design: cite exact lines/selectors, diverge from old retro, produce reviewable PR slices.
- G8: Document process with skills mobilization.

### Non-Goals (NG) — Explicit
- NG1: **Do NOT preserve or variant the old newspaper/wisteria/米色/2px/serif aesthetic**. Completely different mood.
- NG2: No new backend features, protocol changes, or server modifications.
- NG3: No heavy custom painting revival (prefer pure QSS; optional minimal for depth if QSS insufficient).
- NG4: No full hot-reload QSS (future nice-to-have; restart-with-flag is sufficient for now).
- NG5: Not a web/electron port or pure frontend redesign (Qt Widgets QSS constraints respected).
- NG6: Do not break existing objectName contract where it works; extend it.

---

## Open Design Process & Skills Mobilized

This design is the artifact of a deliberate open process (not ad-hoc). All mandated files were read first. Multiple exploration strategies used (list_dir on client/docs/.agents, full style.qss, targeted grep for objectName/setStyleSheet/paintEvent/QFrame# etc.).

**Skills explicitly invoked and contributing** (non-conflicting mobilization of design + cdd-* family + others):
- **cdd-drift-guard** (the *only* registered physical skill in `.agents/skills/cdd-drift-guard/SKILL.md`): Referenced for post-impl doc sync (TODO/JOURNAL/INDEX/README alignment on redesign).
- **Conceptual approaches applied** (not physical skill files in the repo; invoked per AGENTS.md RULE #6 spirit using design thinking + domain knowledge): 
  - design / design-brief / design-md: Structured the problem, captured user quotes + requirements into this MD document.
  - design-review / comprehensive-review / check-work: Thorough cross-file analysis (style.qss vs cpp sources vs specs), citation discipline, alternatives section.
  - reference-design-contract: Read + deliberately diverged from `docs/superpowers/specs/2026-06-16-bottle-messenger-ui-design.md` (entire), plan, prd.md NFR4, old CONTRACT immutable notes, AGENTS.md.
  - theme-factory: Created cohesive token system (palette, radii, spacing, typography) from scratch; mapped to QSS comment tokens.
  - frontend-design + impeccable-design-polish: Applied modern web principles (contrast, density, micro-interactions, accessible states, spacing scale 4/8/12/16/20/24) adapted to QSS/Qt Widgets limitations (no real box-shadow depth easily, selector power via objectName).
  - imagine: Visualized fresh professional mood (clean dark slate chat like refined Slack/Discord/Linear but distinct; high polish, no clutter).
  - tdd + cdd-*: Bypass flag as TDD enabler for UI/QSS (test visual contract without integration).
- **testability / frontend-design**: Made ChatView/MainWindow directly exercisable.
- Others (conceptual): design-system thinking, component-state modeling, accessibility (contrast), iterative polish.

Note: Only `.agents/skills/cdd-drift-guard` physically exists on disk. All others are applied methodologies documented here for traceability. Process steps executed (first action full reads + greps/list_dir confirmed in verification).

Process steps executed:
1. SILENT SCAN + full read of criticals (first action).
2. CLARIFY via exact cites + user language.
3. ECHO + constraints (QSS truth, minimal C++, bypass mandatory).
4. Alternatives + decision.
5. Detailed spec + PR decomposition (incremental, reviewable).
6. Output + summary.

This is **not** continuation of the 2026-06-16 superpowers work — it is rejection + fresh start.

---

## Proposed Design

### High-Level Visual Direction (Radically Different)
**Modern Professional Dark Slate + Indigo**

- **Mood**: Clean, confident, calm, tech-professional. Excellent readability for long chats. Subtle depth via layered surfaces + soft borders (no noisy paper or floral SVG).
- **Foundation**: Dark slate (`#0f172a` → `#1e293b` → `#334155`). High contrast light text.
- **Accent**: Indigo `#6366f1` (vibrant yet professional; alternative emerald `#10b981` for success only). Hover/active deepen.
- **Density**: Balanced — more breathing room than old cramped 2px paper (larger padding, 8px radius, generous list items).
- **Typography**: Primary crisp sans-serif for modernity + readability. Retain LXGW WenKai optionally in stack for CJK personality (but not dominant "calligraphic newspaper").
  - Body: 14-15px system sans + YaHei fallback.
  - Headers/Logo: Slightly larger weight or optional LXGW.
  - Timestamps: Monospace.
- **Shape Language**: 6-8-10px rounded (modern). No 0px hard edges or 2px postage stamps.
- **Depth**: Layered surfaces + subtle borders; focus rings via border color. Avoid heavy custom paint.
- **States**: Every interactive element has complete, distinct hover / focus / active / disabled / selected.

**Why this vs old**: Old = warm retro paper (beige + stone + serif + texture). New = cool professional digital (dark + indigo + sans + crisp). Immediate visual 180°.

### Layout (largely preserved for minimal delta)
- Fixed 960x640 main (or relax later).
- Sidebar 240px left (QFrame#sidebar).
- Top bar full width.
- Tabs for room + privates.
- ChatView: scroll messages + bottom input.
- LoginDialog 440x560 (kept for normal flow; will get modern QSS too).

Mermaid layout (component + data flow for test mode):

```mermaid
graph TD
    subgraph "Test Path (-t)"
        CLI["main.cpp: argv parse --test-chat"] -->|bypass| MW["MainWindow<br/>(mock username)"]
        MW --> CV["ChatView (room) + populateMockData()"]
        MW -->|add tabs| Priv["ChatView (private)"]
        MW -.->|mock| UL["userList items"]
    end
    subgraph "Normal Path"
        Login[LoginDialog] -->|Accepted| MW2[MainWindow]
    end
    style CLI fill:#6366f1,color:#fff
```

### Design Tokens (Comment-based — QSS has no real vars)

At top of new style.qss:

```qss
/* ============================================================
   LinuxChat — Modern Professional Design System
   Theme: Slate Dark + Indigo
   Tokens (single source — update here, propagate)
   ============================================================ */
/*
--color-bg:            #0f172a;
--color-surface:       #1e293b;
--color-elevated:      #334155;
--color-card:          #475569;
--color-text:          #f1f5f9;
--color-text-muted:    #94a3b8;
--color-accent:        #6366f1;
--color-accent-hover:  #4f46e5;
--color-border:        #475569;
--color-success:       #22c55e;
--color-danger:        #ef4444;
--color-warning:       #f59e0b;
--radius-sm: 4px;
--radius:    8px;
--radius-lg: 12px;
--space-1: 4px; --space-2:8px; --space-3:12px; --space-4:16px; --space-5:20px;
--font-sans: "Segoe UI", "Inter", "Microsoft YaHei UI", "Helvetica Neue", "LXGW WenKai", sans-serif;
--font-mono: "Cascadia Code", "Consolas", monospace;
--font-size-body: 14px;
*/
```

### Detailed QSS Rules (Proposed — Full Overhaul)

**Global / Canvas**
```qss
QMainWindow, QWidget#centralWidget {
    background-color: #0f172a;
    font-family: "Segoe UI", "Inter", "Microsoft YaHei UI", "LXGW WenKai", sans-serif;
    font-size: 14px;
    color: #f1f5f9;
}
```

**LoginDialog (modern clean "card" — pure QSS, no layout/C++ change)**
For minimal delta: *no* new QFrame#loginCard or layout mods in setup_ui (login_dialog.cpp:92 margins stay). Achieve elevated card via QSS alone on root + direct children:
- `#loginDialog { background: #0f172a; }` (or slightly lighter elevated for dialog root)
- Inner elements get `#loginDialog QLineEdit { background: #1e293b; border: 1px solid #475569; border-radius: 8px; ... }`
- Title/subtitle/status receive proper colors + spacing.
- "Card" visual: generous 40-50px padding on dialog + background on children creates inset layered feel without structural change. If future polish desires true raised card, a 3-line QFrame wrapper can be added later (non-blocking).
Exact proposed block will be in migration table / full replacement. Validation error: `#loginDialog #loginStatus[error="true"] { color: #ef4444; }` + C++ `status_label_->setProperty("error", true); status_label_->style()->unpolish...polish...` (see PR4 + concrete in Validation UX below). Keep existing objectNames.

**Sidebar**
- `QFrame#sidebar` → `background: #1e293b; border-right: 1px solid #475569;`
- Section headers: uppercase small, muted, better spacing.
- `QListWidget#userList` etc: transparent bg, better item height/padding (12px+), hover `#334155`, selected accent left border + elevated bg.
- Dots: modern larger or pill.

**Buttons** (three objectNames preserved + send)
- `#primaryBtn`: solid indigo bg, white text, 8px radius, good min-height, hover darken, pressed active, disabled muted.
- `#secondaryBtn`: outline or surface bg + border.
- `#dangerBtn`: subtle danger text/border (for disconnect).
- `#sendBtn`: accent solid, icon-like if possible.

**Inputs**
- `QLineEdit, QTextEdit#inputField`: surface bg, border #475569, 8px radius, focus: accent border + subtle ring effect via border.
- Placeholder muted.
- Selection accent.

**Tabs** (`QTabWidget#chatTabs`)
- Modern underline or pill tabs. Selected: accent underline or bg. Hover subtle.

**Message Area**
- `QScrollArea#messageArea, QWidget#messageContainer`: surface or elevated.
- Bubbles (critical):
  - `#bubbleOther`: elevated surface, subtle border, left aligned.
  - `#bubbleSelf`: accent indigo bg, right aligned, light text.
  - Larger padding (10-14px), 8px radius.
  - `QLabel#msgContent`: 15px, good line-height.
  - Username + timestamp smaller muted.
  - Avatar: use `QLabel#avatar`, `QLabel#avatarSelf`, `QLabel#avatarOther` (new objectNames) — sized 28px, rounded 14px, colored via QSS not inline.

**System Notify**: centered subtle pill or banner on surface.

**Scrollbars**: thin modern (6-8px), track subtle, handle accent-muted, hover stronger.

**Menus / Tooltips / Badges**: consistent rounded, surface, accent selection.

**TopBar / ChatHeader / InputArea**: layered surfaces with clean borders.

**Full new style.qss will replace the entire current content** (old comments about "报纸复古" removed).

**Avatar Fix (minimal C++)**:
In `chat_view.cpp:140-194` `create_bubble`:
- Set `avatar->setObjectName(is_self ? "avatarSelf" : "avatarOther");`
- Remove the entire inline `setStyleSheet` ternary (lines 172-174).
- Add QSS:
```qss
QLabel#avatar, QLabel#avatarSelf, QLabel#avatarOther {
    background-color: #475569;
    color: #f1f5f9;
    border-radius: 14px;
    min-width:28px; max-width:28px; ... font-size:12px; font-weight:600;
}
QLabel#avatarSelf { background-color: #6366f1; color: #fff; }
```

This moves the last inline violation into QSS.

### Custom Painting Reduction
**Mechanism (precise, runtime flag for minimal delta)**:
- Add `bool testMode_` member + ctor param `MainWindow(..., bool testMode = false)`.
- Same optional for LoginDialog if desired (not critical since bypass skips it).
- Update `paintEvent` (main_window.cpp:165 and header declaration):
  ```cpp
  void MainWindow::paintEvent(QPaintEvent* event) {
      QMainWindow::paintEvent(event);
      if (testMode_) return;   // PR1: clean QSS surfaces for -t; no wisteria composite
      QPainter painter(this);
      ...
      drawWisteriaBackground(&painter);
  }
  ```
- Similarly gate (or leave as-is since unreachable) Login paint.
- Draw methods can stay (or body commented) — header paintEvent override remains.
- SVGs kept in qrc for rollback only; **not loaded/painted in test or new theme**.
- Rationale: QSS `background-color` on #centralWidget / #sidebar / panes now fully visible immediately. Overlay was the source of "waterfall" complaints.
- **Sequencing**: Gating code + ctor change lands in **PR1** (with bypass) so that after PR2 QSS, running `-t` shows *pure* new slate/indigo with zero legacy paint. (See Issue resolution and updated PR Plan.)

New backgrounds are 100% pure QSS. Future texture (if any) via `background-image: url(...)` only.

### Bypass + Test Data Architecture (Detailed)
**CLI Flag + main.cpp restructure** (see exact sketch below):
- Parse `argc/argv` early (after QApplication + high-dpi, *after* font/style load is acceptable and simpler; before the login while).
- Flags: `--test-chat`, `--direct-chat`, `-t`. Also support `--help` for the flag.
- If test mode:
  - Create `ChatClient client;` (real instance; sends are safe no-ops).
  - `MainWindow window(&client, "DemoUser", /*testMode=*/true);`
  - `window.show();`
  - `window.populateTestData();`   // orchestrates everything (room + users + 2 privates)
  - Wire live echo for interactivity (see below).
  - `return app.exec();`           // clean exit, no re-loop
- Normal path: unchanged while(true) + LoginDialog.

**Exact main.cpp sketch (pseudocode for PR1 implementation)**:
```cpp
// after style load + FontManager + ChatClient client; (moved up if needed)
bool isTestMode = false;
for (int i = 1; i < argc; ++i) {
    QString arg = QString::fromLocal8Bit(argv[i]);
    if (arg == "--test-chat" || arg == "--direct-chat" || arg == "-t") {
        isTestMode = true;
    } else if (arg == "--help" || arg == "-h") {
        qDebug() << "Usage: linuxchat_client [--test-chat | -t | --direct-chat]  (bypasses login for QSS testing)";
        // optionally QApplication::exit(0) early or continue
    }
}

if (isTestMode) {
    qDebug() << "[main] TEST MODE: bypassing LoginDialog for direct QSS iteration";
    MainWindow window(&client, QStringLiteral("DemoUser"), /*testMode=*/true);
    window.show();
    window.populateTestData();  // MUST be public; wires live echo internally
    // NOTE: no returnToLoginRequested connection; disconnect_btn disabled inside
    return app.exec();  // bypass the while loop entirely
}

// original while (true) { LoginDialog ... MainWindow ... connect(returnToLogin) ... } remains for !isTestMode
```

**Public Test API contract (precise, added in PR1)**:
- `MainWindow` (include/main_window.h): add
  ```cpp
  public:
      /// Populates room + user list + exactly two private tabs with rich mock data.
      /// Safe to call once after show(). Idempotent (guards internal). For -t only.
      void populateTestData();
  ```
  (Implementation in .cpp orchestrates: room_view_->populateTestData(); loadTestUsers(); get_or_create... for "Alice" and "Bob" + their populate.)
- `ChatView` (include/chat_view.h): add
  ```cpp
  public:
      /// Appends exactly these 12 messages (order preserved, does NOT clear prior content).
      /// Timestamps are synthetic HH:mm:ss. Includes self, other, system. Post-show safe.
      /// Also usable by unit tests if needed.
      void populateTestData();
  ```
  Exact messages (hardcoded in impl for reproducibility; use QDateTime::current... + offsets or fixed):
  1. Other "Alice" @10:21: "Hey, the new modern theme looks so much cleaner."
  2. Self "DemoUser" @10:21: "Yeah! Finally moved away from the beige newspaper look."
  3. System @10:22: "DemoUser (you) connected • 5 users online"
  4. Other "Bob" @10:23: "Testing a longer message here to verify wrapping and bubble max-width behavior under the slate + indigo design."
  5. Self @10:23 (short): "Perfect."
  6. Other "Alice" @10:24: "The 8px radii and indigo accents (#6366f1) feel professional."
  7. Self (long) @10:25: "Agreed. High contrast on dark slate #0f172a with text #f1f5f9 is excellent for long sessions. No more wisteria SVG overlays."
  8. System @10:25: "3 new messages since you were last here."
  9-12: Additional 3-4 varied (different lengths, another private context if needed).
- Both .h updated in PR1. No friend hacks; no direct member access from main.cpp. populateTestData on MainWindow also pre-fills QListWidget and opens two private tabs via existing private APIs.

**Live send echo for interactive QSS testing (critical for input/bubbles/states)**:
In the test-mode main.cpp path (after populate + show):
```cpp
// After window.populateTestData();
auto* room = /* get room view (add lightweight accessor or use room_view_ if friend for test only; prefer: expose via new protected or simple public in PR1) */;
connect(room, &ChatView::send_requested, this, [room]() {  // or lambda in setup
    QString text = room->take_input();  // or re-fetch if needed
    if (!text.isEmpty()) {
        room->append_message(QStringLiteral("DemoUser"), text, QDateTime::currentSecsSinceEpoch(), /*is_self=*/true);
    }
});
// Repeat wiring for any private tabs created in populate
```
- Result: typing + Send (or Ctrl+Enter) in -t instantly creates correctly styled `#bubbleSelf` bubble using the real append path. Enables live iteration on `#inputField`, `#sendBtn`, focus rings, wrapping, alignment without server.
- PR1 acceptance: "in -t, send produces live self bubble with correct QSS styling; no rebuild needed for visual feedback on input flow."

**Disconnect / returnToLogin safe handling in test mode (detailed)**:
- Extend MainWindow ctor: `MainWindow(ChatClient*, const QString& username, bool testMode = false);`
- Store `bool testMode_;`
- In setup_ui (or after): `if (testMode_) { disconnect_btn_->setEnabled(false); disconnect_btn_->setToolTip("Disabled in --test-chat mode"); }`
- In main.cpp test path: **do not** connect `returnToLoginRequested`.
- Override/gate in main_window.cpp:
  - `on_disconnect_clicked()`: if (testMode_) { append_system_message("Disconnect ignored in test mode."); return; } ...
  - `on_disconnected()`: if (testMode_) { append_system_message("Test mode: disconnect signal received (ignored)"); return; } else { ... emit ... }
  - `closeEvent`: always safe (send_logout noop + disconnect); in test just accept() without forcing re-login loop.
- Result: clicking "断开连接" or window close in -t does *not* re-show LoginDialog or exit unexpectedly. Close simply quits the app. Document: "In -t mode, disconnect button disabled; close just quits cleanly."

**Test Mode Special Cases subsection** (new dedicated coverage for high-dpi, reload, validation-in-bypass, etc.):
- **Paint gating moved to PR1**: See Custom Painting section update. MainWindow (and optionally Login) receives testMode in ctor; paintEvent does base then `if (testMode_) return;` before any drawWisteria/drawNewspaper. Ensures `-t` after PR1/PR2 shows *pure* new QSS dark slate surfaces (no 0.35 opacity wisteria composite on chat area). SVGs remain in qrc only.
- **High-DPI**: All QSS px + C++ fixed (avatar 28px, sidebar 240px, main 960x640, radii, paddings) are integer device pixels. PassThrough policy (main.cpp:22) + Windows scaling makes them effectively device-independent at 96dpi base. Spot-checked at 100%/150%/200%; 8px radii remain crisp, 240px sidebar readable, 28px avatars legible. Font sizes in pt scale via system. No fractional values introduced.
- **Reload / iteration**: No hot-reload (NG4). QSS changes require full rebuild (rcc embeds :/style.qss). Acceptable: with `-t` the cycle is "edit .qss → cmake --build → run -t" (<30s on dev machine). Add at top of new style.qss:
  `/* QSS single source. Edit → rebuild → ./linuxchat_client.exe -t to test immediately. No hot reload. */`
  PR5 docs mention this.
- **Validation UX in bypass**: Bypassed (no Login shown); normal-path Login validation polish still delivered (see G5 + PR4).
- **ChatClient ownership**: Real ChatClient instance (no special test double). All sends guarded by !is_connected() → no-op. Keeps delta minimal. Future signals safe (none fire in test).
- **Other**: No user list click private tab creation needed beyond populate; fixed sizes kept (relax optional PR6).

**Mock population + live echo together** make `-t` a full live demo chat for exercising every QSS rule interactively (bubbles, input, buttons, lists, tabs, scroll, states).

This directly satisfies "Support immediate QSS testing without server" and "make QSS iteration fast and beautiful".

### Supporting Changes (Light)
- Login validation UX: use status label + QSS error color (e.g. danger on error state). Add `setProperty("error", true)` + QSS `[error="true"]`.
- Polish disconnect: already good; ensure status labels update via objectName swap + polish (existing).
- High DPI: already set; QSS px values scale reasonably.

---

## Alternatives Considered

1. **Keep a "retro variant" toggle or light theme fork of old**  
   Tradeoff: Would satisfy "don't throw away work" but directly contradicts "radically different" + user "还是太丑了" + "open design". Increases maintenance (two QSS worlds). Rejected.

2. **Glassmorphism / heavy blur / light modern beige**  
   Tradeoff: Beautiful on web (backdrop), but Qt Widgets QSS has weak/no real blur support. Would require heavy custom paint (worse than current SVGs). Light version too close to old newspaper. Rejected for practicality + divergence.

3. **Full emerald accent + pure light theme** (vs dark slate + indigo)  
   Tradeoff: Emerald = fresh/nature (echoes old green dot), light = readable daytime. But dark slate chosen for modern chat convention (reduces eye strain in long sessions), stronger contrast, more "professional app" than retro. Indigo provides vibrant non-earthy accent. Reversible later via tokens. Accepted direction; light variant noted as future.

4. **Keep heavy custom QPainter for "premium" depth**  
   Tradeoff: Current SVGs are the source of "waterfall" complaint. Pure QSS + layered widgets = faster iteration + easier maintenance. Minor paint retained only if needed for status dots etc. (already QSS friendly).

5. **Add QFileSystemWatcher for hot-reload QSS even in test mode**  
   Tradeoff: Would make iteration sub-second. However NG4 explicitly defers; rcc embedding + Windows/Qt resource loading makes reliable watcher non-trivial (need to re-apply to QApplication dynamically, race with styles). Rebuild cost with `-t` is acceptable (<30s). Rejected for scope. (Note: future enhancement possible via external loader.)

6. **Small PNG texture via QSS background-image instead of full paint removal**  
   Tradeoff: Could give subtle modern depth without SVG paint code. But current complaint is the *decorative* wisteria/newspaper itself; new design goal is clean flat-to-layered surfaces. PNG adds asset + scaling complexity on high-DPI. Future only.

**Old vs New contrast table** (key dimensions):
- Palette: old #faf9f7/#44403c stone vs new #0f172a slate / #6366f1 indigo
- Radius: 2px everywhere vs 8px (sm 4 / lg 12)
- Font: LXGW+Newsreader serif dominant vs Segoe/Inter/YaHei sans primary
- Paint: heavy wisteria (0.35 tiled) + newspaper noise + globe vs gated/none (pure QSS)
- Inline styles: yes (avatar) vs none (QSS only)
- States: partial vs full explicit :hover/focus etc.
- Density: cramped paper vs balanced spacious (larger padding, 32px list items)

Decision: Full modern dark slate + indigo + pure-QSS priority + bypass-first + paint gate in PR1.

---

## Implementation Notes & Constraints

- **QSS single source** (AGENTS.md, CONTRACT.md:92): No new inline setStyleSheet. All via objectName selectors or global.
- **Contract evolution**: `style.qss` listed as immutable in CONTRACT.md:91. PR5 (or parallel) *must* edit CONTRACT.md to remove it from immutable list or add annotation: "style.qss — except for approved visual redesign per 2026-06-17 grok-design-87197455". README.md visual claims and old superpowers specs also updated in same PR. cdd-drift-guard will enforce.
- **Resources**: Keep fonts/SVGs in qrc (LXGW may be reused lightly). No new binary assets required.
- **ObjectNames preserved/extended**: All existing (sidebar, chatTabs, bubbleSelf etc.) + minimal new (avatarSelf etc.).
- **Build**: Qt6 Widgets + Svg still (Svg can be dropped if paints removed).
- **Testing the design**: `./linuxchat_client.exe --test-chat` (after rebuild). Change style.qss, rebuild, rerun. See Test Mode Special Cases for reload reality (rebuild required).
- **High DPI**: PassThrough policy already present. All dimensions integer px; verified at scale (see acceptance).
- **Chinese text**: Ensure font stack has excellent CJK coverage (YaHei + LXGW fallback).
- Later CDD alignment: Use cdd-drift-guard on impl (TODO.md update, JOURNAL entry, possible INDEX refresh).

---

## Key Decisions

- **Style direction chosen**: Modern professional dark slate (#0f172a base / #1e293b surface / #334155 elevated) + indigo (#6366f1) accent. Vs old beige (#faf9f7) + stone (#44403c) newspaper/retro + wisteria. Rationale: user rejection of old ("还是太丑了"), "open design" requirement, professional contrast/readability, modern chat conventions, easy QSS iteration.
- **Bypass architecture**: Early argv parse in main.cpp (before Login loop). Direct MainWindow + ChatView construction + dedicated `populateTestData()` / `loadTestUsers()`. No server connection needed. Enables QSS testing immediately. Normal login path untouched. **Live echo wiring + testMode flag passed to MainWindow ctor**.
- **Paint gating sequencing**: Runtime `if (testMode_) return;` after base paintEvent (no #ifdef) lands in **PR1** alongside bypass. Ensures `-t` immediately shows clean QSS-only (no wisteria overlay polluting visual validation). SVGs kept in qrc only.
- **populateTestData / live interactivity contract**: Precise public API on *both* MainWindow (orchestrator) + ChatView. Exactly 12 messages (hardcoded reproducible content + timestamps). Live send_requested echo wired in test path (real append_message, no server). See detailed contract above.
- **QSS variable strategy**: Comment-based token table at file head (/* --color-bg: ... */). No runtime CSS variables (Qt limitation). Duplicate values with clear comments; single place for palette changes. Risk of hex drift accepted and mitigated by comment + PR reviews.
- **Font decision**: Primary crisp modern sans ("Segoe UI", "Inter", "Microsoft YaHei UI", sans-serif). LXGW WenKai retained only as graceful CJK fallback or minor title accent. Newsreader de-emphasized/removed (FontManager still loads it harmlessly). Radically different from dominant serif + calligraphic old stack. Timestamps stay mono.
- **Custom painting**: Prefer elimination or gating (test vs normal). Move all visual to QSS (new #avatarSelf etc.). Globe/wisteria SVGs no longer painted by default. Gating in PR1.
- **Avatar styling**: Remove inline setStyleSheet (chat_view.cpp:172-174 violation). Use objectName + QSS selectors only.
- **Radii / density / states**: 8px default radius (vs 2px). Generous but balanced padding. Complete :hover/:focus/:pressed/:disabled/:selected for every widget. Focus "ring" = accent border-color + slight padding increase (Qt QSS limit; no real box-shadow).
- **Login "card" implementation**: Pure QSS on existing direct-child layout (no new QFrame or layout change in PR1/2 for minimal delta). #loginDialog + children get elevated surface bg + 8px radius + generous padding + subtle border to simulate card. (See Login clarification below.)
- **Scope split**: QSS is 80% of impact. Bypass is critical enabler (early PR). Polish/validation are light supporting.
- **Divergence + contracts**: Explicitly non-goal to keep old aesthetic or follow 2026-06-16 specs. **CONTRACT.md will be updated** (style.qss removed from immutable list or annotated "except approved redesign 87197455") + README visual claims revised in PR5 (or parallel docs PR).
- **Fixed sizes**: Keep 960x640 / 440x560 / 240px for this redesign (minimal delta). Optional relax in follow-up PR6 if review reveals issues.
- **Test mode permanence**: Runtime bool (not #ifdef). Permanent dev tool (valuable for future QSS work) but harmless in release (flag not documented for end-users).
- **PRD NFR4**: Leave historical (old style reference); do not edit to avoid scope creep.
- **High-DPI / reload**: See Test Mode Special Cases. Rebuild required (acceptable); px values spot-checked.
- **Focus rings & Qt limits**: Implemented as `border: 2px solid #6366f1` + background on focus (practical for Widgets QSS on Windows).
- **FontManager impact**: Minor — Newsreader load remains but unused in new QSS. No removal to keep delta minimal.

---

## PR Plan

Realistic, incremental, independently reviewable slices. **Bypass first** for fast QSS feedback. Order respects dependencies. Each has clear title, files, deps, desc. Assume feature branch + review. **cdd-drift-guard awareness** in every PR description (docs will drift).

1. **PR: Add --test-chat bypass + rich mock data + paint gating + live echo + testMode handling for direct UI testing**  
   Files: `client/main.cpp` (argv parse + restructure + if(test){...} + live echo connect + populate calls), `client/include/chat_view.h` + `src/chat_view.cpp` (public `populateTestData()` with *exact* 12 messages contract), `client/include/main_window.h` + `src/main_window.cpp` (ctor testMode param, public `populateTestData()`, loadTestUsers, disable disconnect_btn_, gate on_disconnected/closeEvent/returnToLogin, paintEvent early return if(testMode_)), also wire send echo.  
   Deps: None (standalone testable slice).  
   Desc: Implements mandatory flag. Bypasses Login. Populates *precise* rich mock + live send echo (interactive bubbles/input). **Critical: paint gating (if(testMode_) return) lands here** so -t shows clean QSS immediately (no wisteria). Disconnect safe in test. Update usage. No style changes yet. Invoke cdd-drift-guard awareness.

2. **PR: Full modern QSS overhaul — new palette, typography, radii, spacing, states + avatar rules**  
   Files: `client/resources/style.qss` (complete rewrite: tokens header + *full* migration table rules; remove all retro/Qt5 comments), `client/src/chat_view.cpp` (set avatar objectNames "avatarSelf"/"avatarOther" + remove inline 172-174 — can land here or combined).  
   Deps: PR1 (for fast validation via `-t`).  
   Desc: Radically new design. Dark slate + indigo. 8px radii. Sans font stack. Complete interactive states + focus. All selectors (see migration table). Avatar QSS folded in. Preserves/extends objectNames. High contrast, professional polish. cdd-drift-guard note.

3. **PR: (minimal) Polish & complete states, validation UX, scrollbar/tabs refinement**  
   Files: `client/resources/style.qss` (any follow-up states/polish if split), `client/src/login_dialog.cpp` (concrete setProperty("error")+polish + QSS error rule), `client/src/main_window.cpp` (ensure polish sites clean).  
   Deps: PR2.  
   Desc: Impeccable details. Login validation usable in normal flow. Modern scrollbars etc. (avatar already done in PR2).

4. **PR: Documentation, CONTRACT/ immutable update, testing instructions, TODO/JOURNAL alignment, drift guard**  
   Files: `README.md` (replace "报纸复古 + 紫藤萝瀑布" claims + add -t usage + new screenshots note), `CONTRACT.md` (remove or annotate `style.qss` + `resources.qrc` from immutable list; add note "except approved visual redesign 87197455"), `TODO.md` (new Step), `docs/JOURNAL.md`, `client/resources/style.qss` (usage comment), `docs/specs/prd.md` (historical note optional), possibly archive old superpowers specs.  
   Deps: Prior or parallel.  
   Desc: Contract evolution approved via this open design + user feedback. Runbook `./... -t`. cdd-drift-guard invocation. Mark old specs archived. Update build artifacts notes if needed.

5. **(Optional) PR: Relax fixed sizes + minor layout tweaks**  
   Files: main_window.cpp / login_dialog.cpp + QSS.  
   Deps: PR2/3.  
   Desc: As needed.

Each slice focused. PR1 now guarantees clean QSS testing from the start (paint gate early). QSS lands fully usable in PR2. CONTRACT/README updated explicitly (resolves immutable drift risk). All visual PRs note drift-guard.

---

## Validation & Acceptance

**Build & run**:
- `cd client/build && cmake --build . --config Release`
- Test: `cd Release && .\linuxchat_client.exe --test-chat` (or -t). Also test normal flow without flag.

**Concrete visual checklist** (must observe in `-t` + normal login after PR2; tie to tokens):
- Dark slate foundation visible: QMainWindow / #messageContainer / #sidebar = #0f172a / #1e293b (no beige #faf9f7, no wisteria SVG overlay at any opacity).
- Indigo accent (#6366f1) on #primaryBtn / #sendBtn background + hover darken to #4f46e5.
- 8px radii everywhere (bubbles #bubbleSelf/#bubbleOther, inputs, buttons, sidebar items, menu, tabs) — inspect corners.
- Sans stack dominant: body text uses Segoe/UI/YaHei (not heavy LXGW/Newsreader serif calligraphic).
- High contrast: primary text #f1f5f9 on #0f172a / #1e293b; muted #94a3b8 for timestamps (#msgTimestamp), sectionHeader, placeholders.
- Bubbles: self = solid indigo #6366f1 right-aligned with white text; other = #1e293b + #475569 border, left-aligned. 10-14px padding inside.
- Avatars: 28px, 14px radius; #avatarSelf indigo bg, others #475569 (no inline styles visible).
- Sidebar list: hover #334155, selected accent left border + #475569 bg; 32px item height.
- Input/send: focus border 2px #6366f1; sendBtn clean indigo.
- Tabs: selected underline #6366f1.
- Scrollbars: 6px thin modern, handle #64748b (no 4px retro).
- System notify: pill #334155 + #94a3b8.
- Login (normal path): elevated dark card feel via pure QSS on #loginDialog children; no newspaper noise/globe.
- Live echo: in -t, type + send → new #bubbleSelf appears instantly with correct styling, alignment, wrapping.
- States: all hover/pressed/focus/disabled/selected visibly distinct (no 2px postage stamp feel).
- High-DPI: at 150%+ scaling, 240px sidebar, 28px avatars, text, 8px radii remain crisp/legible/clickable (PassThrough + integer px).
- No old elements: zero #faf9f7 / #44403c / 2px radius / wisteria.

**Other**:
- No regression: normal login/register/connect flow (if server available) still works. Disconnect in normal path emits returnToLogin.
- Automated: existing tests unchanged.
- UAT: User confirms "beautiful" / no longer "太丑了". QSS changes feel fast with -t. "Live send works for iteration".
- High DPI: explicitly test on scaled display + checklist above.
- Reload note: rebuild required; verify comment at top of style.qss.

Screenshot or side-by-side with old (if possible) recommended for PR review.

---

## Risks & Mitigations

- QSS selector power limited → Mit: extensive objectName use + global rules; thorough testing with -t.
- Font availability → Mit: broad stack + fallbacks.
- Dark theme preference → Mit: documented; tokens allow future light fork.
- Rebuild cost for QSS change → Mit: bypass + flag; note that resource embedding requires rebuild (acceptable for iteration).

---

*End of Design Document. This is the complete open-design output. Implement via the PR Plan above.*

**References (from exploration)**:
- style.qss entire (all rules cited above + full selector list)
- main.cpp:50 (login loop), 22 (high-dpi)
- login_dialog.cpp:195 (paint), 90 (objectName), validation paths
- main_window.cpp:78 (sidebar 240px), 165 (paint wisteria), 244 (unpolish/polish)
- chat_view.cpp:172 (inline violation), 36 (message margins), 140 (create_bubble)
- AGENTS.md (QSS truth), CONTRACT.md:91-92 (immutable + style rule)
- Old specs (deliberate divergence)
- .agents/skills/cdd-drift-guard only physical skill file
- All via initial mandatory tool calls + review verification.

---

## Appendix: Example Rich Mock Data (for populateTestData impl)

Exact 12 messages (see contract above for order, self/other/system, timestamps):
1. Alice 10:21: "Hey, the new modern theme looks so much cleaner."
2. DemoUser 10:21: "Yeah! Finally moved away from the beige newspaper look."
3. (system) 10:22: "DemoUser (you) connected • 5 users online"
4. Bob 10:23: "Testing a longer message here to verify wrapping and bubble max-width behavior under the slate + indigo design."
5. DemoUser 10:23: "Perfect."
6. Alice 10:24: "The 8px radii and indigo accents (#6366f1) feel professional."
7. DemoUser 10:25: "Agreed. High contrast on dark slate #0f172a with text #f1f5f9 is excellent for long sessions. No more wisteria SVG overlays."
8. (system) 10:25: "3 new messages since you were last here."
9. Bob 10:26: "Short reply."
10. Alice 10:27: "Scroll test message to make sure the thin modern scrollbar and message area padding (#messageArea) look good."
11. DemoUser 10:28: "Live send test in -t mode works great for QSS iteration."
12. (system) 10:28: "DemoUser left private chat with Bob."

Timestamps fixed synthetic for determinism. Multiple privates pre-opened for "Alice", "Bob".

---

## Appendix B: Comprehensive Selector Migration Table + Expanded QSS Examples

Full replacement of style.qss is required (remove old "Qt5" / "报纸复古" header comment at line 2-6; new header: "/* LinuxChat Qt6 — Modern Slate + Indigo. QSS single source per CONTRACT.md + AGENTS.md. Tokens at top. Edit → rebuild → run -t. */").

**Migration strategy**: Replace entire file content. Below is exhaustive mapping of *every* current selector (from full style.qss read) to new rules + values. Use tokens from comment block. Copy-paste into new file. All states included.

| Old Selector (style.qss:line) | New Selector + Rules (use tokens) | Notes |
|-------------------------------|-----------------------------------|-------|
| QMainWindow, QWidget#centralWidget (8-15) | Same + `background-color: #0f172a; font-family: var(--font-sans); font-size: 14px; color: #f1f5f9;` | |
| QDialog (18-21) | `#loginDialog, QDialog { background-color: #0f172a; ... }` | Pure QSS card via children |
| QFrame#sidebar (24-29) | `QFrame#sidebar { background-color: #1e293b; border: none; border-right: 1px solid #475569; border-radius: 0; }` | |
| QLabel#sectionHeader (32-39) | Same + `font-size: 11px; color: #94a3b8; padding: 8px 12px 4px; letter-spacing: 1px; text-transform: uppercase;` | Muted |
| QLabel#currentUser (42-47) | Same + `color: #f1f5f9; font-weight: 600;` | |
| QListWidget#userList etc (50-83) | `background: transparent; border:none; padding:4px 8px; ::item { height:32px; padding:8px 12px; border-radius:8px; color:#e2e8f0; } ::item:hover { background:#334155; } ::item:selected { background:#475569; border-left:3px solid #6366f1; color:#fff; font-weight:500; }` | |
| QLabel#onlineDot / #offlineDot (86-102) | `min/max 10px; border-radius:5px; background:#22c55e; /* offline #64748b */` | |
| QPushButton#primaryBtn (105-130) | `background:#6366f1; color:#fff; border:1px solid #6366f1; border-radius:8px; padding:10px 16px; font-size:14px; min-height:36px; :hover { background:#4f46e5; } :pressed { background:#4338ca; } :disabled { background:#64748b; color:#cbd5e1; }` | |
| #secondaryBtn (133-152) | `background:#1e293b; color:#cbd5e1; border:1px solid #475569; border-radius:8px; ... :hover { border-color:#6366f1; color:#fff; }` | |
| #dangerBtn (155-174) | `background:transparent; color:#94a3b8; border:1px solid #475569; border-radius:6px; padding:6px 10px; font-size:12px; :hover { color:#ef4444; border-color:#ef4444; }` | |
| QLineEdit, QTextEdit#inputField (177-203) | `background:#1e293b; border:1px solid #475569; border-radius:8px; color:#f1f5f9; padding:10px 14px; font-size:14px; selection-background:#6366f1; :focus { border:2px solid #6366f1; } ::placeholder { color:#64748b; } :disabled { background:#334155; color:#64748b; }` | |
| QTabWidget#chatTabs::pane + tabs (206-234) | `::pane { background:#1e293b; border:1px solid #475569; border-radius:8px; } QTabBar::tab { color:#94a3b8; padding:8px 16px; border-bottom:2px solid transparent; } ::tab:selected { color:#f1f5f9; border-bottom:2px solid #6366f1; } ::tab:hover:!selected { background:#334155; color:#e2e8f0; }` | |
| #messageArea / #messageContainer (237-241) | `background:#0f172a; border:none;` | |
| QFrame[frameShape="4"] divider (244-247) | `color:#475569; max-height:1px;` | |
| QToolTip (250-257) | `background:#1e293b; color:#f1f5f9; border:1px solid #475569; border-radius:6px; padding:6px 10px; font-size:12px;` | |
| QScrollBar:vertical + handle (260-302) | `width:6px; background:transparent; ::handle { background:#64748b; border-radius:3px; min-height:20px; } ::handle:hover { background:#94a3b8; }` (same for horiz) | Thin modern |
| QMenu + items (311-339) | `background:#1e293b; border:1px solid #475569; border-radius:8px; padding:4px 0; color:#f1f5f9; ::item { padding:6px 16px; } ::item:selected { background:#334155; } ::item:disabled { color:#64748b; } ::separator { background:#475569; }` | |
| #bubbleOther / #bubbleSelf + labels (342-391) | `#bubbleOther { background:#1e293b; border:1px solid #475569; border-radius:8px; } #bubbleOther QLabel#msgContent { color:#e2e8f0; } ... #bubbleSelf { background:#6366f1; border:none; border-radius:8px; } #bubbleSelf QLabel#msgContent { color:#fff; } QLabel#msgUsername { font-size:11px; color:#64748b; } #msgContent { font-size:15px; line-height:1.5; } #msgTimestamp { font-family:var(--font-mono); font-size:10px; color:#64748b; }` | Max-width on wrapper in C++ unchanged |
| #systemNotify (394-401) | `font-size:12px; color:#94a3b8; background:#334155; border-radius:12px; padding:4px 14px;` | Pill |
| #unreadBadge (404-413) | `background:#ef4444; color:#fff; font-size:10px; border-radius:10px; padding:1px 6px; min-width:18px;` | |
| #loginDialog specifics (416-492) | `#loginDialog { background:#0f172a; } #loginTitle { color:#f1f5f9; font-weight:600; font-size:22px; } #loginSubtitle, #loginStatus { color:#94a3b8; font-size:12px; } #loginDialog QLineEdit { background:#1e293b; border:1px solid #475569; border-radius:8px; ... :focus { border-color:#6366f1; } } #loginDialog #loginStatus[error="true"] { color:#ef4444; }` + primary/secondary as above | |
| QFrame#topBar (495-516) | `background:#1e293b; border-bottom:1px solid #475569;` | ` #topBarTitle { font-family: var(--font-sans); font-size:16px; font-weight:600; color:#f1f5f9; } #statusConnected { color:#22c55e; } #statusDisconnected { color:#64748b; }` | |
| QFrame#chatHeader (519-529) | `background:#1e293b; border-bottom:1px solid #475569;` `#chatHeaderTitle { font-size:14px; font-weight:600; color:#f1f5f9; }` | |
| QFrame#inputArea (532-535) | `background:#1e293b; border-top:1px solid #475569;` | |
| QPushButton#sendBtn (538-554) | `background:#6366f1; color:#fff; border:none; border-radius:8px; padding:8px 20px; font-size:14px; :hover { background:#4f46e5; }` | |
| QLabel#avatar (557-572) | `#avatar, #avatarSelf, #avatarOther { background:#475569; color:#f1f5f9; border-radius:14px; font-size:12px; font-weight:600; min/max 28px; } #avatarSelf { background:#6366f1; color:#fff; }` | **Replaces all inline** |

**Additional rules** (current QSS had gaps; add for completeness):
- Global focus ring on buttons/inputs via :focus { border-color: #6366f1; outline: none; }
- QListWidget item height/padding increased for density.
- All letter-spacing reduced (modern); no uppercase except headers.
- Full state coverage everywhere.

Copy the token comment + this table's rules into new style.qss. This provides near-implementable completeness (engineer can build full file from it + examples).

**Validation UX concrete** (for PR4):
In login_dialog.cpp on_error / validation:
```cpp
status_label_->setProperty("error", true);
status_label_->style()->unpolish(status_label_);
status_label_->style()->polish(status_label_);
```
QSS addition (in table above):
`#loginDialog #loginStatus[error="true"] { color: #ef4444; }`

**Qt Widgets QSS on Windows notes** (Issue 16):
- Prefer test with `app.setStyle("Fusion");` if Windows native style interferes with radii/borders.
- Existing unpolish/polish for dynamic objectName changes (status, dots) MUST remain.
- Some scroll/tab quirks on Win; test thoroughly with -t.
- sizeHint / min-height in C++ may need +2-4px padding adjustments after new 8px radii + larger paddings.
- No reliance on unsupported properties.

**Old style.qss header cleanup** (in PR2): Replace "Qt5 Client — style.qss / Source of Truth: design-overview.html" with "Qt6 Client — Modern Slate+Indigo / QSS is single source of truth (CONTRACT.md:92 + AGENTS.md). Tokens defined at top."

This addresses completeness for implementation.
