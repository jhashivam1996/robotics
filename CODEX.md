# CODEX.md

## Scope

This file defines repository-level working rules for Codex and other coding agents.

Keep this file focused on:

- code hygiene
- editing discipline
- repository workflow
- documentation placement

Do not put robot-specific hardware behavior, wiring, or tuning rules here. Those belong in project documentation.

## Repository Layout

1. Keep the main implementation in the existing project structure unless the user asks for a reorganization.
2. Do not create new top-level directories or files without a clear reason.
3. Prefer updating existing files over adding parallel duplicates.

## Editing Rules

1. Make minimal, targeted changes.
2. Preserve existing behavior unless the user asks to change it.
3. Avoid broad rewrites when a local fix is sufficient.
4. Keep code ASCII unless a file already requires non-ASCII content.
5. Add comments only when they clarify non-obvious logic or constraints.
6. Keep constants grouped and easy to tune.
7. Prefer small helper functions over repeating logic inline.
8. Do not silently change externally meaningful values such as pin assignments, file names, or interfaces without documenting the change.

## Safety Rules

1. Do not remove protective checks or guardrails without explicit user intent.
2. Treat hardware-facing code changes as behavior-sensitive.
3. Prefer conservative, reviewable changes over clever compact rewrites.

## Documentation Rules

1. `CODEX.md` is for repository coding rules only.
2. Project-specific behavior and hardware assumptions must live in separate docs.
3. `README.md` should remain high-level.
4. If a project needs detailed operational rules, create a dedicated document with a specific name such as:
   - `docs/robot-rules.md`
   - `arduino-uno-car/BEHAVIOR.md`
   - `HARDWARE.md`

## Verification Rules

1. If a change affects runtime behavior, prefer adding or preserving lightweight debug visibility where practical.
2. If hardware validation is required and cannot be run locally, state that clearly.
3. After behavior changes, recommend concrete manual checks instead of claiming full verification.

## Collaboration Rules

1. Keep changes easy for a human to review.
2. When requirements are ambiguous, preserve the current structure and ask only if the ambiguity is blocking.
3. When separating concerns, prefer moving domain-specific instructions into dedicated project docs rather than overloading repository policy files.
