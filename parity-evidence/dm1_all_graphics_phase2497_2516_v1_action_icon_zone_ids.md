# DM1 all-graphics parity — phase 2497–2516: V1 action icon zone ids

## Scope

Expose the layout-696 source zone ids for the four right-column action-hand icon cells and their inner icon cells.

## Source anchors

Layout-696 / ReDMCSB `DEFS.H`:

- `C089..C092`: action-hand icon cell parent zones.
- `C093..C096`: 16×16 inner icon zones within those cells.

The V1 action icon cells remain `(233,86,20,35)` through `(299,86,20,35)`, with inner icons `(235,95,16,16)` through `(301,95,16,16)`.

## Implemented

- Added `M11_GameView_GetV1ActionIconCellZoneId(slot)` returning `C089..C092`.
- Added `M11_GameView_GetV1ActionIconInnerZoneId(slot)` returning `C093..C096`.
- Routed the cell/inner geometry helpers through those source zone-id helpers.

## Updated invariants

- `INV_GV_300D`: action icon zones now assert both source ids and geometry.

## Verification

Passed in this batch:

```text
cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2
FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
# summary: 513/513 invariants passed
ctest --test-dir build --output-on-failure
# 100% tests passed, 0 tests failed out of 5
```
