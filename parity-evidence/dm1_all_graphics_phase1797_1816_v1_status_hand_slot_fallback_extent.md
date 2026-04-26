# DM1 all-graphics parity — phase 1797–1816: V1 status-hand slot fallback extent

## Scope

Make the non-asset fallback path for compact champion status-hand slot boxes preserve the same 18×18 source extent as the GRAPHICS.DAT `C033/C034/C035` slot-box bitmaps.

## Source anchors

- `CHAMDRAW.C` / `F0291_CHAMPION_DrawSlot` draws the source slot-box bitmap at the hand-zone origin.
- GRAPHICS.DAT slot-box graphics `C033`, `C034`, and `C035` are 18×18.
- Layout-696 hand parent zones `C211..C218` remain 16×16; that parent size is for positioning the icon cell, not for clipping the slot-box bitmap.

## Implemented

- Routed the fallback fill/outline inside `m11_draw_v1_status_hand_slot(...)` through `M11_GameView_GetV1StatusHandSlotBoxZone(...)`.
- The fallback now covers 18×18 like the source bitmap path instead of stopping at the 16×16 parent zone.
- Added a no-assets draw invariant checking the right/bottom overdraw edge pixels of the ready-hand box.

## New invariant

- `INV_GV_15X`: V1 status hand slot fallback renders the 18×18 `C033` box extent, not the 16×16 parent zone.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`

## Probe excerpt

```text
PASS INV_GV_15X V1 status hand slot fallback renders the 18x18 C033 box extent, not the 16x16 parent zone
```
