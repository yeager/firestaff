# DM1 all-graphics parity — phase 2377–2396: V1 status name zone ids

## Scope

Expose the layout-696 source zone ids for compact champion status-name clear and text zones, then route name-zone validation through those ids.

## Source anchors

Layout-696 / CHAMDRAW.C `F0292_CHAMPION_DrawChampionState` uses:

- clear zones: `C159..C162`
- centered text child zones: `C163..C166`

The clear zones are `43×7`; the text child zones are `42×7` at `+1` x-offset.

## Implemented

- Added `M11_GameView_GetV1StatusNameClearZoneId()`.
- Added `M11_GameView_GetV1StatusNameTextZoneId()`.
- Routed the clear/text zone helpers through the source zone id helpers.

## Updated invariants

- `INV_GV_15E2`: V1 champion HUD name clear zones expose layout-696 `C159-C162` ids and geometry.
- `INV_GV_15E8`: V1 status name text zones expose layout-696 `C163-C166` ids and geometry.

## Verification

Passed in this batch:

```text
cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2
FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
# summary: 513/513 invariants passed
ctest --test-dir build --output-on-failure
# 100% tests passed, 0 tests failed out of 5
```
