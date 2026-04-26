# DM1 all-graphics parity — phase 1517–1536: V1 status shield border zone routing

## Scope

Route V1 status-box shield overlay blits through the same source status-box dimensions as base status boxes and child HUD zones.

## Source anchors

- GRAPHICS.DAT `C037/C038/C039` shield border overlays: `67×29`, same footprint as the compact champion status box.
- Existing `M11_GameView_GetV1StatusBoxZone(...)` resolves the source status-box rectangle used for base clears, dead boxes, names, bars, and hand slots.
- `M11_GameView_GetV1StatusShieldBorderGraphic(...)` selects the overlay graphic by source priority.

## Implemented

- Routed status shield border asset dimension checks through `slotW/slotH` from `M11_GameView_GetV1StatusBoxZone(...)`.
- Routed shield border blit region dimensions through `slotW/slotH` instead of repeating literal `67×29` at the overlay draw site.
- Reused `INV_GV_15E9` and `INV_GV_15P` coverage for geometry and graphic priority.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`
- Probe result: `485/485 invariants passed`
- CTest result: `100% tests passed, 0 tests failed out of 5`

## Probe excerpt

```text
PASS INV_GV_15E9 V1 champion HUD status box zones match C007/source stride geometry
PASS INV_GV_15P V1 status shield border graphic priority follows spell/fire/party source order
# summary: 485/485 invariants passed
```
