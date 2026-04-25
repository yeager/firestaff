# DM1 all-graphics phase 82 — parity matrix summary supersession

## Problem

Phase 81 updated the viewport row in `PARITY_MATRIX_DM1_V1.md`, but the matrix summary and old pass-40 evidence still contained current-looking references to viewport `KNOWN_DIFF` / `(12,24,196,118)` as if that state still applied.

## Change

- Updated parity matrix summary counts:
  - `MATCHED` 10 → 11, adding viewport rectangle bounds.
  - `KNOWN_DIFF` 11 → 10, removing the superseded viewport drift.
- Rewrote the bottom-line paragraph to reflect current 2026-04-25 state: source-matched viewport rectangle bounds are now landed, but full DM1/V1 parity remains incomplete.
- Clarified `pass40_viewport_lock.md` §4/§5 as historical pass-40 rationale, explicitly superseded by the all-graphics viewport migration.

## Gate

```text
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
# summary: 410/410 invariants passed
ctest --test-dir build --output-on-failure
5/5 PASS
```
