# DM1 all-graphics parity — phase 1477–1496: V1 status box base graphic

## Scope

Make V1 compact status-box base graphic selection explicit and probe-visible: living champions use a source dark-gray clear, while dead champions use the source dead status-box bitmap.

## Source anchors

- ReDMCSB `CHAMPION.C F0292_CHAMPION_DrawChampionState`: living status boxes are cleared and rebuilt from name/bars/hands; `C007_GRAPHIC_STATUS_BOX` is marked unused for living status.
- GRAPHICS.DAT `C008_GRAPHIC_STATUS_BOX_DEAD_CHAMPION`: dead champion status-box bitmap, `67×29`.
- Existing V1 drawing already used `C008` for dead champions; this pass extracts the selection into a verified helper.

## Implemented

- Added `M11_GameView_GetV1StatusBoxBaseGraphic(...)`.
- Routed dead-status-box asset selection through the helper.
- Added invariant coverage for living champion → no base graphic and dead champion → `C008`.

## New invariant

- `INV_GV_15Q`: V1 status box base graphic uses source dead box only for dead champions.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`
- Probe result: `485/485 invariants passed`
- CTest result: `100% tests passed, 0 tests failed out of 5`

## Probe excerpt

```text
PASS INV_GV_15Q V1 status box base graphic uses source dead box only for dead champions
# summary: 485/485 invariants passed
```
