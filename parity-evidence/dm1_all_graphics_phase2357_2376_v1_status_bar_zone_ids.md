# DM1 all-graphics parity — phase 2357–2376: V1 status bar zone ids

## Scope

Expose the layout-696 source zone ids for the V1 compact champion HP/stamina/mana vertical bar containers and route status-bar validation through those ids.

## Source anchors

Layout-696 / CHAMDRAW.C `F0287_CHAMPION_DrawBarGraphs` uses these status-box-local bar zones:

- HP: `C195`
- stamina: `C199`
- mana: `C203`

Each bar container is `4×25` and bottom-anchored under the `C187/C191` bar-graph region.

## Implemented

- Added `M11_GameView_GetV1StatusBarZoneId()`.
- Routed `M11_GameView_GetV1StatusBarZone()` stat validation through the source zone id helper.

## Updated invariant

- `INV_GV_15E7`: V1 status bar graph zones expose layout-696 `C195/C199/C203` ids and geometry.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2` — OK
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` — OK, `# summary: 513/513 invariants passed`
- `ctest --test-dir build --output-on-failure` — OK, `100% tests passed, 0 tests failed out of 5`

## Probe excerpt

```text
PASS INV_GV_15E7 V1 status bar graph zones expose layout-696 C195/C199/C203 ids and geometry
```
