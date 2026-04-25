# DM1 all-graphics phase 101 — ornament matrix narrowing

## Problem

`PARITY_MATRIX_DM1_V1.md` still treated panel backgrounds / ornaments as generic `UNPROVEN`, even though the M11 renderer already has source-backed ornament data plumbing and focused gates for the currently implemented draw paths.

## Change

Updated the panel backgrounds / ornaments row to distinguish proven plumbing from remaining overlay work.

Recorded as proven/narrowed:

- per-map wall/floor/door ornament index cache from `DUNGEON.DAT` metadata
- wall ornament base `259` (`M615`)
- floor ornament sets from `247`, plus special footprints `379..384`
- door ornament resolution through per-map door ornament tables
- floor ornaments drawn below floor items / creatures / projectiles
- center and side wall/door ornament draw paths
- focused gates:
  - `INV_GV_38I/J/K` floor, footprints, wall ornament visible changes
  - `INV_GV_114` wall ornament graphic availability
  - `INV_GV_234/235` wall/door depth-scaling monotonicity
  - `INV_GV_238` side-pane ornament path
  - `INV_GV_248` floor ornament cache storage

Still open:

- exact original panel placement
- clipping
- z-order details beyond current focused gates
- screenshot overlay against original ornament-heavy runtime scenes

Status is now `KNOWN_DIFF (narrowed)`, not generic `UNPROVEN`.

## Gate

Documentation-only pass, backed by existing probe coverage and latest green baseline:

```text
firestaff_m11_game_view_probe: 418/418 invariants passed
ctest: 5/5 PASS
```
