# DM1 all-graphics parity — phase 2677–2696: V1 inventory food/water zones

## Scope

Expose the layout-696 source zone ids and resolved rectangles for the inventory panel plus its food/water subpanel zones.

## Source anchors

ReDMCSB `DEFS.H` / layout-696:

- `C101_ZONE_PANEL` — panel graphic `C020`, centered at `(152,89)` through parent `C100` (`144×73`) → resolved `(80,52,144,73)`.
- `C103_ZONE_FOOD_BAR` — child of `C102`, resolved for the `C030` food label (`34×9`) as `(113,69,34,6)` with source-y clip `2`.
- `C104_ZONE_FOOD_WATER` — child of `C102`, resolved for the `C031` water label (`46×9`) as `(113,92,46,6)` with source-y clip `2`.

Resolution was cross-checked with `tools/resolve_dm1_zone.py` against `zones_h_reconstruction.json`.

## Implemented

- Added `M11_GameView_GetV1InventoryPanelZoneId()` / `M11_GameView_GetV1InventoryPanelZone()`.
- Added `M11_GameView_GetV1FoodBarZoneId()` / `M11_GameView_GetV1FoodBarZone()`.
- Added `M11_GameView_GetV1FoodWaterPanelZoneId()` / `M11_GameView_GetV1FoodWaterPanelZone()`.

## Updated invariants

- `INV_GV_300AI`: asserts `C101/C103/C104` ids and resolved geometry/source clip values.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2` — passed.
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` — `520/520 invariants passed`.
- `ctest --test-dir build --output-on-failure` — `5/5` tests passed.
