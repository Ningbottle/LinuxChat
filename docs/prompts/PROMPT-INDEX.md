ROLE: You are a diligent assistant that prepares **Chat-Driven-Development (CDD)-ready documentation** for a given repository.

GOAL: Generate or update a perfectly formatted `docs/INDEX.md` (and its sibling bodies under `docs/index/` if INDEX split is active) by following a three-phase process of analysis, generation, and emission, with strict quality assurance. The INDEX should be a comprehensive, high-level overview of the project's architecture, components, and user flows.

──────────────────────────────────────────────
Phase 1: Analysis & Code Comprehension
──────────────────────────────────────────────

1) Overview
- Read `README.md`, `TODO.md` to understand the project and its progress.
- Read `docs/specs/blueprint.md` and connected specs.
- If `docs/INDEX.md` already exists: treat this as an **update** task. If `docs/index/` already exists, read its siblings for the prior layout.

2) Identify core language & frameworks
- Determine primary languages and frameworks.
- Scan dependency files: `package.json`, `requirements.txt`, etc.

3) Map the codebase
- Enumerate directories.
- Identify main source directories (e.g. `src`, `apps`, `packages`).

4) Read source files
- Focus entrypoints, config, and core business logic.
- Build a file inventory table with LOC.
- Tag files >760 LOC as `refactor-candidate`.
- Do this separately for app source vs tests vs other (specs/assets/scripts/etc.).

5) Extract architectural components
- Entrypoints/processes
- Core services/logic
- Data models
- API layers
- Client-side logic
- Integrations

──────────────────────────────────────────────
Phase 2: Diagram & Content Generation
──────────────────────────────────────────────

Generate 2–4 mermaid diagrams as appropriate:
- System Context (C4 L1)
- User Flow / Sequence Diagram
- Component Interaction (C4 L3)
- Dependency Map

Mermaid rules (GitHub-safe):
- Node IDs must be alphanumeric/hyphen only.
- Use consistent arrows (prefer `-->`).
- No complex shapes.
- Use `<br/>` for line breaks.

──────────────────────────────────────────────
Phase 3: Layout & Emission
──────────────────────────────────────────────

Default to **single-file emission**: write everything into `docs/INDEX.md` (Executive Summary, Project Snapshot, Diagrams, File Inventory, Dependency Map, Glossary, Last Generated).

Switch to **split-file emission** when any of the following holds:
- `docs/INDEX.md` would exceed ~300 lines.
- A single inventory section grows unboundedly with the codebase (e.g. app source rows that strictly increase with every regen).
- `docs/index/` already exists from a prior regeneration — preserve the layout.

In split-file mode, emit a slim entrypoint plus siblings under `docs/index/`:

- `docs/INDEX.md` — Executive Summary, Project Snapshot, **Layout** pointer block, Dependency Map (full mermaid), Glossary, Last Generated. Stays under ~300 lines.
- `docs/index/DIAGRAMS.md` — every architecture / flow / component mermaid diagram from Phase 2. One H3 per diagram with the same title used in the Layout pointer.
- `docs/index/INVENTORY-<area>.md` — file & API inventory tables split by area. Typical defaults:
  - `INVENTORY-source.md` for the primary source tree (`src/`, `lib/`, `apps/`, or the project's equivalent).
  - `INVENTORY-tests.md` for tests and fixtures.
  - `INVENTORY-other.md` for everything else (contracts/specs, assets, scripts, third-party manifests). Omit this file if there is no third-tier inventory worth tracking.

Rules:
- Do not duplicate any inventory row across siblings — each path belongs in exactly one file.
- The Dependency Map and Glossary stay in `docs/INDEX.md`. Do not move them.
- Each sibling starts with a one-line back-pointer: `> Body of [docs/INDEX.md](../INDEX.md) — <section name>.`
- Only `docs/INDEX.md` carries the `## Last Generated` footer.
- If a sibling has no rows in this regen, still emit the file with the back-pointer and an `_(empty this regen)_` line so the layout pointer stays valid.
- Once split mode is active, keep it. Do not collapse back to single-file in a future regen.

──────────────────────────────────────────────
Quality Assurance (self-grade)
──────────────────────────────────────────────

Grade yourself 0–12. If <11.5, repeat analysis+generation.

Checklist:
1. Executive summary + project snapshot present in `docs/INDEX.md` (+1)
2. File inventory with LOC — in `docs/INDEX.md` (single mode) or across `docs/index/INVENTORY-*.md` (split mode) (+2)
3. Refactor candidates flagged (+1)
4. Markdown structure correct; in split mode, `docs/INDEX.md` ≤ ~300 lines (+1)
5. System context diagram present — in `docs/INDEX.md` or `docs/index/DIAGRAMS.md` (+1)
6. User flow diagram present — in `docs/INDEX.md` or `docs/index/DIAGRAMS.md` (+1)
7. Component interaction diagram present — in `docs/INDEX.md` or `docs/index/DIAGRAMS.md` (+1)
8. Dependency map in `docs/INDEX.md` (+1)
9. Mermaid syntax correct (+1)
10. Glossary + Last Generated present in `docs/INDEX.md` (+1)
11. High information density (+1)

──────────────────────────────────────────────
TEMPLATE — single-file mode (default)
──────────────────────────────────────────────

# Context for `{{PROJECT_NAME}}`

## Executive Summary
...

## Project Snapshot
- **Frameworks**: ...
- **Languages**: ...
- **Key Libraries**: ...
- **Architecture**: ...

## Diagrams

### System Context
```mermaid
...
```

## File & API Inventory

| Path | Role | LOC | Key Tags, Symbols, Names |
| :--- | :--- | --: | :----------------------- |
| ... | ... | ... | ... |

## Dependency Map
```mermaid
...
```

## Glossary
- ...

## Last Generated
- **Lines of Code (App/Tests)**: ...
- **Last Commit Msg**: ...
- **Timestamp**: ...
- **LLM Model**: ...
- **Grade**: ...

──────────────────────────────────────────────
TEMPLATE — split-file mode (`docs/INDEX.md` slim entrypoint)
──────────────────────────────────────────────

# Context for `{{PROJECT_NAME}}`

## Executive Summary
...

## Project Snapshot
- **Frameworks**: ...
- **Languages**: ...
- **Key Libraries**: ...
- **Architecture**: ...

## Layout

Diagrams → [`docs/index/DIAGRAMS.md`](index/DIAGRAMS.md)
- <System Context>
- <Next diagram title>
- ...

Inventory
- [`docs/index/INVENTORY-source.md`](index/INVENTORY-source.md) — primary source tree
- [`docs/index/INVENTORY-tests.md`](index/INVENTORY-tests.md) — tests and fixtures
- [`docs/index/INVENTORY-other.md`](index/INVENTORY-other.md) — contracts, assets, scripts (omit pointer if no `other` file emitted)

## Dependency Map
```mermaid
...
```

## Glossary
- ...

## Last Generated
- **Lines of Code (App/Tests)**: ...
- **Tracked files**: ...
- **Last Commit Msg**: ...
- **Timestamp**: ...
- **LLM Model**: ...
- **Grade**: ...

──────────────────────────────────────────────
TEMPLATE — split-file mode (`docs/index/DIAGRAMS.md`)
──────────────────────────────────────────────

# INDEX body — Diagrams

> Body of [docs/INDEX.md](../INDEX.md) — architecture, flow, and component diagrams.

### System Context
```mermaid
...
```

### <Next diagram title>
```mermaid
...
```

──────────────────────────────────────────────
TEMPLATE — split-file mode (`docs/index/INVENTORY-<area>.md`)
──────────────────────────────────────────────

# INDEX body — <area>

> Body of [docs/INDEX.md](../INDEX.md) — <one-line description>.

| Path | Role | LOC | Key Tags, Symbols, Names |
| :--- | :--- | --: | :----------------------- |
| ... | ... | ... | ... |

If the file groups two or more inventory tables (typical for `INVENTORY-other.md`), give each table its own `## <Section heading>` and table header.
