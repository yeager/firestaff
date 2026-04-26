# DM1 all-graphics parity — phase 2517–2536: V1 action icon parent zone id

## Scope

Expose the layout-696 source parent/template zone id for the right-column action-hand icon cell family.

## Source anchors

Layout-696 / ReDMCSB `DEFS.H`:

- `C088`: action-hand icon parent/template zone under `C011_ZONE_ACTION_AREA`.
- `C089..C092`: four action-hand icon cell zones.
- `C093..C096`: 16×16 inner icon zones.

`C088` has the source 20×35 cell-template dimensions; the four child cell zones place that template at the four champion offsets.

## Implemented

- Added `M11_GameView_GetV1ActionIconParentZoneId()` returning source zone `C088`.
- Routed `M11_GameView_GetV1ActionIconCellZoneId(slot)` through the parent-zone helper before returning `C089..C092`.

## Updated invariants

- `INV_GV_300D`: action icon zones now assert the parent `C088` id along with child ids and geometry.

## Verification

Passed in this batch:

```text
cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2
FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
# summary: 513/513 invariants passed
ctest --test-dir build --output-on-failure
# 100% tests passed, 0 tests failed out of 5
```
