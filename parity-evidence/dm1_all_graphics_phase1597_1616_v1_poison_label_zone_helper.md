# DM1 all-graphics parity — phase 1597–1616: V1 poison label zone helper

## Scope

Make the V1 poisoned-label placement probe-visible as a source-backed status-box child zone.

## Source anchors

- GRAPHICS.DAT `C007/C008` champion status-box footprint: `67×29`.
- GRAPHICS.DAT `C032_GRAPHIC_POISONED`: `96×15`.
- Source presentation centers the poisoned label below the champion status box; because `96 > 67`, the label intentionally spills horizontally across neighboring status boxes.
- Existing `M11_GameView_GetV1StatusBoxZone(...)` resolves slot 0 as `(12,160,67,29)` and slot 3 as `(219,160,67,29)`.

## Implemented

- Added `M11_GameView_GetV1PoisonLabelZone(...)`.
- Routed V1 poison-label drawing through the helper.
- Added invariant coverage for slot 0 and slot 3 poisoned-label geometry using the verified `96×15` C032 footprint.
- V2/prebaked HUD positioning remains unchanged.

## New invariant

- `INV_GV_15R`: V1 poisoned label zone centers C032 under C007 status box geometry.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`

## Probe excerpt

```text
PASS INV_GV_15R V1 poisoned label zone centers C032 under C007 status box geometry
```
