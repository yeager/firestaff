# DM1 all-graphics parity — phase 2537–2556: V1 spell caster zone ids

## Scope

Expose the layout-696 source zone ids for the spell-area magic-caster selector at the top of the right-column spell area.

## Source anchors

ReDMCSB `DEFS.H` / layout-696:

- `C013_ZONE_SPELL_AREA` — right-column spell area at `(224,90,87,25)`.
- `C221_ZONE_SPELL_AREA_SET_MAGIC_CASTER` — magic-caster selector panel under `C013`; reconstructed dimensions `87×8`.
- `C224_ZONE_SPELL_AREA_MAGIC_CASTER_TAB` — visible caster tab under the selector panel; reconstructed dimensions `45×8`.

## Implemented

- Added `M11_GameView_GetV1SpellCasterPanelZoneId()` / `Zone()` for source `C221`.
- Added `M11_GameView_GetV1SpellCasterTabZoneId()` / `Zone()` for source `C224`.
- Routed tab validation through the parent panel helper.

## Updated invariants

- `INV_GV_300AC`: asserts `C221/C224` ids and expected top-of-spell-area geometry.

## Verification

Passed in this batch:

```text
cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2
FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
# summary: 514/514 invariants passed
ctest --test-dir build --output-on-failure
# 100% tests passed, 0 tests failed out of 5
```
