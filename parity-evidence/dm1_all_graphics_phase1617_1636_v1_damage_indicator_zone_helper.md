# DM1 all-graphics parity — phase 1617–1636: V1 damage indicator zone helper

## Scope

Make the V1 champion damage indicator placement probe-visible as a source-backed status-box child zone.

## Source anchors

- GRAPHICS.DAT `C007/C008` champion status-box footprint: `67×29`.
- GRAPHICS.DAT `C015_GRAPHIC_DAMAGE_TO_CHAMPION_SMALL`: `45×7`.
- ReDMCSB `CHAMPION.C F0291` overlays the small damage indicator centered inside the champion status box.
- Existing `M11_GameView_GetV1StatusBoxZone(...)` resolves slot 0 as `(12,160,67,29)` and slot 3 as `(219,160,67,29)`.

## Implemented

- Added `M11_GameView_GetV1DamageIndicatorZone(...)`.
- Routed V1 small damage-banner blitting through the helper.
- Added invariant coverage for slot 0 and slot 3 damage-banner geometry using the verified `45×7` C015 footprint.
- V2/prebaked HUD positioning remains unchanged.

## New invariant

- `INV_GV_15S`: V1 champion damage indicator zone centers C015 inside C007 status box geometry.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`

## Probe excerpt

```text
PASS INV_GV_15S V1 champion damage indicator zone centers C015 inside C007 status box geometry
```
