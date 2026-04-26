# DM1 all-graphics parity — phase 1277–1296: V1 status bar zone helper

## Scope

Harden compact champion HP/stamina/mana vertical bar placement by exposing the source layout-696 bar zones and routing bar drawing through the shared helper.

## Source anchors

- `zones_h_reconstruction.json` from GRAPHICS.DAT layout `C696`:
  - `C195`, `C199`, `C203` are the source HP/stamina/mana bar containers under `C187/C191`.
  - The V1 status-box-relative bar origins are `x=46/53/60`, `y=4`, size `4×25` in Firestaff's existing pass-43 anchoring.
- `CHAMDRAW.C F0287_CHAMPION_DrawBarGraphs`: draws three bottom-anchored vertical bars for each champion status box.
- Existing V1 status-box placement uses screen origin `(12,160)` and stride `69`, so slot 0 HP is `(58,164,4,25)` and slot 3 mana is `(279,164,4,25)`.

## Implemented

- Added `M11_GameView_GetV1StatusBarZone(...)` for probe-visible source bar geometry.
- Routed V1 vertical bar drawing through that helper instead of local `x + m11_v1_bar_graph_x(...)` placement.
- Added a direct invariant covering the first HP and fourth mana bar zones.

## New invariant

- `INV_GV_15E7`: V1 status bar graph zones match layout-696 `C195/C203` geometry.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`
- Probe result: `476/476 invariants passed`
- CTest result: `100% tests passed, 0 tests failed out of 5`

## Probe excerpt

```text
PASS INV_GV_15E7 V1 status bar graph zones match layout-696 C195/C203 geometry
# summary: 476/476 invariants passed
```
