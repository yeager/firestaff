# DM1 all-graphics parity — phase 2397–2416: V1 status box zone ids

## Scope

Expose the layout-696 source zone ids for the compact champion status-box roots, then route status-box geometry validation through those ids.

## Source anchors

Layout-696 / CHAMDRAW.C `F0292_CHAMPION_DrawChampionState` status-box root zones:

- `C151_ZONE_CHAMPION_0_STATUS_BOX_NAME_HANDS`
- `C152_ZONE_CHAMPION_1_STATUS_BOX_NAME_HANDS`
- `C153_ZONE_CHAMPION_2_STATUS_BOX_NAME_HANDS`
- `C154_ZONE_CHAMPION_3_STATUS_BOX_NAME_HANDS`

Each root status-box zone is `67×29`, with the source champion stride of `69` pixels.

## Implemented

- Added `M11_GameView_GetV1StatusBoxZoneId()`.
- Routed `M11_GameView_GetV1StatusBoxZone()` through the source zone-id helper.

## Updated invariants

- `INV_GV_15E9`: V1 champion HUD status-box zones expose layout-696 `C151-C154` ids and geometry.

## Verification

Passed in this batch:

```text
cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2
FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
# summary: 513/513 invariants passed
ctest --test-dir build --output-on-failure
# 100% tests passed, 0 tests failed out of 5
```
