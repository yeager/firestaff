# DM1 all-graphics parity — phase 1937–1956: V1 action-menu row count

## Scope

Make the V1 action-menu three-row contract explicit and probe-visible.

## Source anchors

- ReDMCSB `F0387_MENUS_DrawActionArea` prints exactly three action names from the selected `ActionSet` into the action-menu row zones.
- M11 already used the three row zones; this pass removes the remaining inline `3` row-count use from renderer flow and validates the invalid fourth row.

## Implemented

- Added `M11_GameView_GetV1ActionMenuRowCount()` returning `3`.
- Routed action-menu row bounds/looping through the helper.
- Strengthened `INV_GV_300F` to assert row count `3` and reject row index `3`.

## Updated invariant

- `INV_GV_300F`: action menu row zones match F0387 source three-row geometry.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`

## Probe excerpt

```text
PASS INV_GV_300F action menu row zones match F0387 source three-row geometry
```
