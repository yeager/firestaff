# DM1 all-graphics parity — phase 1557–1576: V1 poison label status-zone routing

## Scope

Route V1 poisoned-label placement through the resolved source status-box zone dimensions instead of repeating the `67×29` C007 footprint at the label draw site.

## Source anchors

- GRAPHICS.DAT `C007/C008` champion status-box footprint: `67×29`.
- GRAPHICS.DAT `C032_GRAPHIC_POISONED`: `96×15` label.
- ReDMCSB inventory/status drawing places the poisoned label immediately below the champion status box; because the label is wider than `67`, it intentionally spills across adjacent status boxes.
- Existing `M11_GameView_GetV1StatusBoxZone(...)` resolves the C007 status-box rectangle.

## Implemented

- V1 poisoned-label centering now uses `slotW` from `M11_GameView_GetV1StatusBoxZone(...)`.
- V1 poisoned-label Y placement now uses `slotH` from `M11_GameView_GetV1StatusBoxZone(...)`.
- V2/prebaked HUD positioning is preserved with the previous `67×29` fallback constants.
- No startup-menu or multi-renderer paths changed.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`

## Probe excerpt

```text
PASS INV_GV_15E9 V1 champion HUD status box zones match C007/source stride geometry
PASS INV_GV_215 POISONED label drawn when champion poisonDose > 0
```
