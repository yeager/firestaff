# DM1 all-graphics parity — phase 1777–1796: V1 status-hand slot-box zones

## Scope

Make the compact champion status-hand slot-box bitmap draw zones explicit and probe-visible.

## Source anchors

- Layout-696 hand slot parent zones `C211..C218` are 16×16 at status-box-relative `(4,10)` and `(24,10)`.
- ReDMCSB `CHAMDRAW.C` / `F0291_CHAMPION_DrawSlot` draws the hand-slot box bitmap first, then draws the object/empty-hand icon inset inside it.
- GRAPHICS.DAT slot-box graphics `C033`, `C034`, and `C035` are 18×18 and intentionally overdraw the 16×16 parent zone by one pixel on the right/bottom.

## Implemented

- Added `M11_GameView_GetV1StatusHandSlotBoxZone(...)`.
- Routed V1 status-hand slot drawing through the 18×18 slot-box helper instead of reusing only the 16×16 parent hand zone.
- Kept the previous `M11_GameView_GetV1StatusHandIconZone(...)` 16×16 inset helper unchanged.
- Added probe invariant coverage for champion 0 ready-hand box and champion 3 action-hand box.

## New invariant

- `INV_GV_15W`: V1 status hand slot-box zones expose 18×18 `C033/C034/C035` overdraw at hand origins.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`

## Probe excerpt

```text
PASS INV_GV_15W V1 status hand slot-box zones expose 18x18 C033/C034/C035 overdraw at hand origins
```
