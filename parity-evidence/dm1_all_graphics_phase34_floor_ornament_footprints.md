# DM1 all-graphics phase 34 — special footprints floor ornament

Date: 2026-04-25 15:12 Europe/Stockholm
Scope: Firestaff V1 / DM1 floor ornament 15 (footprints) graphic family.

## Change

Added explicit handling for the special footprints floor ornament.

In ReDMCSB/DM1 PC, regular current-map floor ornaments start at `M616_GRAPHIC_FIRST_FLOOR_ORNAMENT = 385`, but footprints (`C15_FLOOR_ORNAMENT_FOOTPRINTS`) are special and live before that family:

- `M639_GRAPHIC_FLOOR_ORNAMENT_15_D3L_FOOTPRINTS = 379`
- `M749_GRAPHIC_FLOOR_ORNAMENT_15_D3C_FOOTPRINTS = 380`
- `M750_GRAPHIC_FLOOR_ORNAMENT_15_D2L_FOOTPRINTS = 381`
- `M751_GRAPHIC_FLOOR_ORNAMENT_15_D2C_FOOTPRINTS = 382`
- `M752_GRAPHIC_FLOOR_ORNAMENT_15_D1L_FOOTPRINTS = 383`
- `M753_GRAPHIC_FLOOR_ORNAMENT_15_D1C_FOOTPRINTS = 384`

Updated both floor ornament paths:

- rectangle fallback helper `m11_draw_floor_ornament(...)`
- source-zone helper `m11_draw_dm1_floor_ornaments(...)`

If `ornGlobalIdx == 15`, the renderer now uses base `379` instead of `385 + 15*6`.

## New invariant

Added:

- `INV_GV_38J` — special footprints floor ornament family renders from pre-base graphics

The focused scene forces per-map floor ornament index `15` and verifies it visibly changes an empty corridor frame.

## Verification

```sh
cmake --build build -j4
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
ctest --test-dir build --output-on-failure
```

Results:

- game-view probe: `372/372 invariants passed`
- CTest: `4/4 PASS`

Relevant output:

```text
PASS INV_GV_38I focused viewport: all visibly drawable floor ornament positions change their corridor frames
PASS INV_GV_38J focused viewport: special footprints floor ornament family renders from pre-base graphics
```

## Remaining work

- Footprints center-view flip policy: source flips center footprints when `G0076_B_UseFlippedWallAndFootprintsBitmaps` is set. That global/runtime condition is not modeled yet.
- Pixel-lock focused footprints scenes against original captures.
