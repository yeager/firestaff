# DM1 all-graphics parity — phase 1497–1516: V1 status box zone routing

## Scope

Finish routing V1 compact status-box base drawing through the shared status-box zone dimensions, so live fills and dead-box blits use the same source rectangle as the child-zone helpers.

## Source anchors

- GRAPHICS.DAT `C007/C008` status-box dimensions: `67×29`.
- `DEFS.H C69_CHAMPION_STATUS_BOX_SPACING`: source stride is `69` px.
- Existing `M11_GameView_GetV1StatusBoxZone(...)` resolves slot rectangles such as `(12,160,67,29)` and `(219,160,67,29)`.

## Implemented

- Carried status-box height from `M11_GameView_GetV1StatusBoxZone(...)` through the party HUD loop.
- Routed dead status-box asset dimension checks/blits through `slotW/slotH`.
- Routed living status-box dark-gray clears through `slotW/slotH`.
- Reused invariant `INV_GV_15E9` for zone geometry and `INV_GV_15Q` for living/dead base graphic selection.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`
- Probe result: `485/485 invariants passed`
- CTest result: `100% tests passed, 0 tests failed out of 5`

## Probe excerpt

```text
PASS INV_GV_15E9 V1 champion HUD status box zones match C007/source stride geometry
PASS INV_GV_15Q V1 status box base graphic uses source dead box only for dead champions
# summary: 485/485 invariants passed
```
