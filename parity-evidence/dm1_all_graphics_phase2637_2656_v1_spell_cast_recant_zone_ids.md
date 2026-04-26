# DM1 all-graphics parity — phase 2637–2656: V1 spell cast/recant zone ids

## Scope

Expose the source zone ids for the spell-area `CAST SPELL` and recant-symbol controls.

## Source anchors

ReDMCSB `DEFS.H` / layout-696:

- `C252_ZONE_SPELL_AREA_CAST_SPELL` — cast button/control zone.
- `C254_ZONE_SPELL_AREA_RECANT_SYMBOL` — recant/backspace symbol control zone.

These belong to the same V1 spell-area source zone family as `C245..C264` landed in the previous batch.

## Implemented

- Added `M11_GameView_GetV1SpellCastZoneId()` returning source `C252`.
- Added `M11_GameView_GetV1SpellRecantZoneId()` returning source `C254`.

## Updated invariants

- Extended `INV_GV_300AG` to assert `C252` and `C254` alongside the existing spell-symbol zone ids.

## Verification

Passed in this batch:

```text
cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2
FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
# summary: 518/518 invariants passed
ctest --test-dir build --output-on-failure
# 100% tests passed, 0 tests failed out of 5
```
