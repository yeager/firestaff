# DM1 all-graphics parity — phase 1857–1876: V1 live status-box fill color

## Scope

Make the live champion status-box background fill color explicit and probe-visible instead of keeping it as an inline draw constant.

## Source anchors

- ReDMCSB `CHAMDRAW.C` / `F0292_CHAMPION_DrawChampionState` clears the live champion status box before drawing the status-name zone, bars, hands, and overlays.
- The source clear color is `C12_COLOR_DARKEST_GRAY`; in the M11 DM PC VGA palette this is palette slot `12`.
- Dead status boxes and V2 vertical-slice fallback behavior are unchanged.

## Implemented

- Added `M11_GameView_GetV1StatusBoxFillColor()`.
- Routed live V1 status-box background fill through the helper.
- Added probe coverage that validates both the helper value and rendered live status-box pixels.

## New invariant

- `INV_GV_15AA`: V1 live status-box fill uses source `C12` darkest-gray before overlays.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`

## Probe excerpt

```text
PASS INV_GV_15AA V1 live status-box fill uses source C12 darkest-gray before overlays
```
