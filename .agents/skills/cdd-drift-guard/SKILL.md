---
name: cdd-drift-guard
description: Use when a task changes code, commands, structure, docs, or workflow in this repo and may leave README.md, TODO.md, TODO-<area>.md, docs/INDEX.md, docs/index/**, docs/specs/*, docs/JOURNAL.md, or docs/journal/* out of date. Keep CDD docs aligned with implementation and route journaling correctly in single or split mode, and apply INDEX split when the entrypoint outgrows a single file.
---

# CDD Drift Guard

Use for changes that affect behavior, layout, entrypoints, workflow, architecture, or CDD doc structure.

## Update only what drifted

- `README.md` — project description, top-level entrypoints, setup/dev/test/build commands, repo guarantees
- `TODO.md` — root execution entrypoint, active work index, sequencing, automated checks, UAT
- `TODO-<area>.md` — area-specific execution steps when work is split
- `docs/specs/blueprint.md` — architecture, invariants, interfaces, data structures, operational changes
- `docs/specs/prd.md` — user-visible scope, use cases, user stories, requirements, vision
- `docs/INDEX.md` — single file in small repos; slim entrypoint once INDEX splits. Regenerate from `docs/prompts/PROMPT-INDEX.md` (together with siblings under `docs/index/**` when split) on structure, entrypoint, diagram, or inventory drift
- `docs/index/DIAGRAMS.md` — mermaid diagram bodies for INDEX when split
- `docs/index/INVENTORY-<area>.md` — file & API inventory bodies for INDEX when split (typical areas: `source`, `tests`, `other`)
- `docs/JOURNAL.md` — live journal in single-journal mode; stable journal entrypoint/index once journals split
- `docs/journal/JOURNAL.md` — cross-cutting journal when work is split
- `docs/journal/JOURNAL-<area>.md` — area journal aligned to `TODO-<area>.md`
- `docs/journal/SUMMARY.md` — condensed archive across split journals

## Scaling rules

- Keep `README.md` current. It should describe current reality first; future direction can stay brief.
- Keep `docs/specs/blueprint.md` as the root spec. Add a spec index there before branching into `docs/specs/<area>-definition.md` or domain subfolders such as `client/`, `server/`, or `ops/`.
- Keep root `TODO.md` as the entrypoint. Split into `TODO-<area>.md` files only when needed. Each step belongs in exactly one TODO file.
- When any active implementation `TODO-<area>.md` exists, enable split-journal mode. Keep `docs/JOURNAL.md` as the stable journal entrypoint, create `docs/journal/`, move the live journal to `docs/journal/JOURNAL.md`, create matching `docs/journal/JOURNAL-<area>.md` files for active workstreams, and rewrite `docs/JOURNAL.md` as a short current-state index that stays meaningful after the split.
- Once split-journal mode starts, keep it. Do not auto-collapse back to a single hot journal. Use `docs/journal/JOURNAL.md` only for repo-wide or cross-cutting notes; area-specific notes belong in exactly one matching `docs/journal/JOURNAL-<area>.md`.
- Introduce `docs/RUNBOOK.md` when exact commands or env vars crowd `README.md`; `README.md` stays the top-level runbook entrypoint.
- Treat `docs/INDEX.md` as a generated context snapshot. Regenerate it when architecture, entrypoints, diagrams, or file inventory drift.
- When `docs/INDEX.md` exceeds ~300 lines or its diagram/inventory sections grow unboundedly with the codebase, enable INDEX split. Keep `docs/INDEX.md` as a slim entrypoint (executive summary, project snapshot, layout pointers, dependency map, glossary, footer), create `docs/index/`, move diagram bodies to `docs/index/DIAGRAMS.md`, and move file inventory tables into `docs/index/INVENTORY-<area>.md` (typical areas: `source`, `tests`, `other`). Regenerate entrypoint and siblings together via `docs/prompts/PROMPT-INDEX.md`.
- Once INDEX split starts, keep it. Do not auto-collapse back to a single file. Each inventory row belongs in exactly one sibling. Dependency Map and Glossary stay in `docs/INDEX.md`.
- If journals are not split, keep `docs/JOURNAL.md` high-signal and follow its single-file archive rule under `docs/archive/`. If journals are split, archive hot journals through `docs/journal/SUMMARY.md` and move raw archived batches under `docs/journal/archive/`.

## Operating rules

- Prefer the smallest doc edit that removes ambiguity.
- Do not rewrite stable docs to mirror low-signal code churn.
- One implementation session belongs in exactly one hot journal file unless the work is truly cross-cutting.
- Do not duplicate the same journal entry across multiple journal files.
- If no doc change is justified, say so explicitly.
- In template/bootstrap state, keep updates proportional, do not invent project-specific detail, and do not precreate split journal or split INDEX files before the split mode is active.
- End by stating: changed docs, checked-but-unchanged docs, and exact validation commands.
