# DM1 all-graphics parity — phase 1757–1776: V1 status-hand icon zones

## Scope

Make the compact champion status-hand object-icon draw zones explicit and probe-visible.

## Source anchors

- Layout-696 hand slot zones `C211..C218` are already resolved by `M11_GameView_GetV1StatusHandZone(...)`.
- DM1 object icons are `16×16` cells.
- Status hand icons draw inside the hand slot, inset by one pixel from the slot origin.

## Implemented

- Added `M11_GameView_GetV1StatusHandIconZone(...)`.
- Routed status-hand object-icon drawing through the helper instead of inline `dstX + 1`, `dstY + 1` offsets.
- Added invariant coverage for champion 0 ready hand and champion 3 action hand icon zones.
- Action-hand right-column icon zones remain separate and unchanged.

## New invariant

- `INV_GV_15V`: V1 status hand icon zones inset 16x16 object icons within hand slots.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`

## Probe excerpt

```text
PASS INV_GV_15V V1 status hand icon zones inset 16x16 object icons within hand slots
```
