# DM1 all-graphics parity — phase 2417–2436: V1 status bar value zone ids

## Scope

Expose the layout-696 source zone ids for champion status bar-graph containers and per-stat bar value zones, then route status-bar geometry validation through those ids.

## Source anchors

Layout-696 / CHAMDRAW.C `F0287_CHAMPION_DrawBarGraphs` / `F0292_CHAMPION_DrawChampionState` uses:

- bar-graph containers: `C187..C190`
- HP value zones: `C195..C198`
- stamina value zones: `C199..C202`
- mana value zones: `C203..C206`

Each bar value is `4×25`, with champion status-box stride `69` pixels.

## Implemented

- Added `M11_GameView_GetV1StatusBarGraphZoneId()` for `C187..C190`.
- Added `M11_GameView_GetV1StatusBarValueZoneId()` for champion/stat-specific `C195..C206` zones.
- Routed `M11_GameView_GetV1StatusBarZone()` through the source value zone-id helper while preserving the existing first-champion stat id helper.

## Updated invariants

- `INV_GV_15E7`: V1 status bar graph zones expose layout-696 `C187-C190` and `C195-C206` ids plus geometry.

## Verification

Passed in this batch:

```text
cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2
FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
# summary: 513/513 invariants passed
ctest --test-dir build --output-on-failure
# 100% tests passed, 0 tests failed out of 5
```
