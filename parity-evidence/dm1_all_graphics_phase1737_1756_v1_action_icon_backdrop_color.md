# DM1 all-graphics parity — phase 1737–1756: V1 action-icon backdrop color

## Scope

Make the per-champion action-hand icon cell backdrop color explicit and probe-visible.

## Source anchors

- `MENUS.C F0386_MENUS_DrawActionIcon` fills living champion action cells with cyan before optional icon blitting.
- Dead champion action cells are filled black and return without an icon.
- Absent champion slots have no action cell.

## Implemented

- Added `M11_GameView_GetV1ActionIconCellBackdropColor(...)`.
- Routed living/dead action-cell fills through the helper.
- Added invariant coverage for living/cyan, dead/black, and absent/ignored states.
- Existing hatching, source-cell geometry, and object-icon rendering remain unchanged.

## New invariant

- `INV_GV_300N`: V1 action icon cell backdrop color is cyan for living, black for dead, absent ignored.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`

## Probe excerpt

```text
PASS INV_GV_300N V1 action icon cell backdrop color is cyan for living, black for dead, absent ignored
```
