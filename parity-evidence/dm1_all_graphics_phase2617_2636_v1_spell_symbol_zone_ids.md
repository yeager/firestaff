# DM1 all-graphics parity — phase 2617–2636: V1 spell symbol zone ids

## Scope

Expose source zone-id mappings for V1 spell-area symbol cells.

## Source anchors

ReDMCSB `DEFS.H` / layout-696:

- `C245..C250` — six parent zones for spell-area symbols 1–6.
- `C255..C260` — six available-symbol draw zones under those parents.
- `C261..C264` — selected/champion spell-symbol draw zones.

## Implemented

- Added `M11_GameView_GetV1SpellAvailableSymbolParentZoneId(index)` for `C245..C250`.
- Added `M11_GameView_GetV1SpellAvailableSymbolZoneId(index)` for `C255..C260`, routed through the parent helper.
- Added `M11_GameView_GetV1SpellChampionSymbolZoneId(index)` for `C261..C264`.

## Updated invariants

- `INV_GV_300AG`: asserts first/last source ids and out-of-range rejection for available and selected spell symbol ids.

## Verification

Passed in this batch:

```text
cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2
FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
# summary: 518/518 invariants passed
ctest --test-dir build --output-on-failure
# 100% tests passed, 0 tests failed out of 5
```
