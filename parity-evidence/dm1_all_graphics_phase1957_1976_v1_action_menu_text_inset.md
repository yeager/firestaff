# DM1 all-graphics parity — phase 1957–1976: V1 action-menu text inset

## Scope

Make the V1 action-menu text inset explicit and derive text origins from source zones instead of parallel absolute constants.

## Source anchors

- ReDMCSB `F0387_MENUS_DrawActionArea` prints the acting champion name and action names inside the header/row boxes with a small left/top inset.
- M11 source zones are already fixed as header `C080` and rows `C085..C087`; this pass binds text origins to those zones plus the `2×1` inset.

## Implemented

- Added `M11_GameView_GetV1ActionMenuTextInset(...)` returning `(2,1)`.
- Reworked `M11_GameView_GetV1ActionMenuTextOrigin(...)` to derive from header/row zones + inset.
- Strengthened invariant coverage to reject row text origin index `3`.

## Updated invariant

- `INV_GV_300J`: action menu text origins derive from source zones plus F0387 `2×1` inset.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`

## Probe excerpt

```text
PASS INV_GV_300J action menu text origins derive from source zones plus F0387 2x1 inset
```
