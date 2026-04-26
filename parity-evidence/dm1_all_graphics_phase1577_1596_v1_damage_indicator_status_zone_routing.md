# DM1 all-graphics parity — phase 1577–1596: V1 damage indicator status-zone routing

## Scope

Route V1 champion damage overlay placement through the resolved source status-box dimensions instead of centering against repeated `67×29` literals.

## Source anchors

- GRAPHICS.DAT `C007/C008` champion status-box footprint: `67×29`.
- GRAPHICS.DAT `C015_GRAPHIC_DAMAGE_TO_CHAMPION_SMALL`: `45×7` damage banner.
- ReDMCSB `CHAMPION.C F0291` overlays the small damage indicator on the champion status box.
- Existing `M11_GameView_GetV1StatusBoxZone(...)` resolves the C007 status-box rectangle.

## Implemented

- V1 small damage banner centering now uses `slotW/slotH` from `M11_GameView_GetV1StatusBoxZone(...)`.
- Damage banner blit dimensions now use the verified asset dimensions after checking `45×7`.
- Damage-number text center now uses the same status-box dimensions.
- V2/prebaked HUD positioning is preserved with the prior `67×29` fallback constants.
- No startup-menu or multi-renderer paths changed.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`

## Probe excerpt

```text
PASS INV_GV_15E9 V1 champion HUD status box zones match C007/source stride geometry
PASS INV_GV_216 damage to champion small (graphic 15) loads as 45x7 from GRAPHICS.DAT
PASS INV_GV_219 per-champion damage indicator drawn when timer > 0
```
