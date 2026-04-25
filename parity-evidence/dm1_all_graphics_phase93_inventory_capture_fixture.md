# DM1 all-graphics phase 93 — deterministic inventory icon capture fixture

## Problem

`capture_firestaff_ingame_series.c` produced an inventory-panel screenshot, but the synthetic champion's ready-hand item reused whatever weapon subtype happened to be at thing index 0 in the loaded dungeon. That made the visual fixture non-deterministic for the phase-91 inventory source-icon work.

## Change

The capture fixture now forces weapon thing index 0 to a deterministic dagger:

```text
THING_TYPE_WEAPON index 0
weapon.type = 8  -> dagger -> source icon 32
chargeCount = 0
lit = 0
CHAMPION_SLOT_HAND_LEFT = weaponThing
```

This makes `06_ingame_inventory_panel_latest` exercise the same source object-icon path as `F0038_OBJECT_DrawIconInSlotBox`, rather than an arbitrary dungeon-loaded subtype.

## Gate

```text
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
# summary: 418/418 invariants passed
ctest --test-dir build --output-on-failure
5/5 PASS
```

The regular post-pass screenshot run also now emits a deterministic inventory panel at:

```text
06_ingame_inventory_panel_latest.png
```
