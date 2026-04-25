# DM1 all-graphics phase 94 — deterministic action-icon screenshot fixture

## Problem

Phase 93 stabilized the inventory-panel screenshot fixture, but the earlier in-game screenshots in `capture_firestaff_ingame_series.c` could still start from the actual loaded party state. If that state had no champion or an arbitrary hand item, the standard post-pass screenshot did not reliably exercise the action-hand object-icon cells that phases 84–91 fixed.

## Change

Added `ensure_deterministic_capture_champion(...)` to the capture fixture and call it before the first in-game screenshot and again before the inventory screenshot.

The capture champion is now deterministic across the whole screenshot series:

```text
champion 0: HALK, alive, active
right/action hand: THING_TYPE_WEAPON index 0
weapon.type = 8  -> dagger -> source icon 32
chargeCount = 0
lit = 0
```

This makes both:

- `02_ingame_turn_right_latest`
- `06_ingame_inventory_panel_latest`

exercise the source object-icon path in a reproducible way.

## Gate

```text
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
# summary: 418/418 invariants passed
ctest --test-dir build --output-on-failure
5/5 PASS
```
