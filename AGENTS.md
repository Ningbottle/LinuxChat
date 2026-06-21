# 1. Methodology

All development in this repository follows **Continuous Driven Development (CDD)**.

CDD is a documentation-first methodology. Instead of jumping straight into code, we maintain a small, authoritative set of markdown files that define exactly what we are building, what decisions we've made, and what needs to be done next. Code is simply the exhaust of this process.

### The CDD Contract

You are bound by these rules. Read them carefully before taking any action.

1. **Read before you write:** Begin every session or task by reading this file (`AGENTS.md`), `TODO.md`, and any relevant specs under `docs/specs/`.
2. **Never break the build:** Ensure all code compiles and tests pass before concluding your work.
3. **No rogue files:** Every file and module must have a clear architectural purpose documented in `docs/specs/blueprint.md`. Do not create orphaned utility scripts or fragmented components.
4. **Update the docs:** Whenever you make a design decision, fix a bug, or change the architecture, update the relevant markdown files (e.g., `docs/JOURNAL.md`, `docs/specs/prd.md`, `docs/specs/blueprint.md`).
5. **Mark tasks complete:** When a task in `TODO.md` is done, mark it with `[x]` and ensure its Acceptance Criteria are fully met.

## 2. Source of Truth

The authoritative state of the project is distributed across the following files:

- `README.md` — Project runbook, setup instructions, and entrypoint.
- `TODO.md` — The single source of truth for all planned, in-progress, and completed work.
- `AGENTS.md` — This file. Defines the rules of engagement and methodology.
- `docs/JOURNAL.md` — Chronological log of development sessions, key decisions, and context handoffs.
- `docs/specs/prd.md` — Product Requirements Document. Defines *what* we are building and *why*.
- `docs/specs/blueprint.md` — Technical Blueprint. Defines *how* we are building it (architecture, data models, critical paths).
- `docs/prompts/PROMPT-INDEX.md` — Index of repeatable workflows and prompts.

## 3. Communication & Handoffs

If you run out of context, hit a hard technical blocker, or need user input to proceed:
1. Update `docs/JOURNAL.md` with the current state, what was tried, and what is blocking progress.
2. Formulate a concise, actionable question or set of options for the user.
3. Do not spin indefinitely or guess the user's intent on critical architectural decisions.

## 4. Project Specifics

- **Project:** LinuxChat
- **Tech Stack:** C++ (Server), Qt/QML (Client), SQLite3
- **Primary Server Port:** 8080
- **Client Architecture:** Qt 6 with `ApplicationWindow` and frameless window controls.
