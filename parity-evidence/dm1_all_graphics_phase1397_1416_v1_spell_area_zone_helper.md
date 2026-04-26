# DM1 all-graphics parity — phase 1397–1416: V1 spell area zone helper

## Scope

Expose and reuse the source `C013_ZONE_SPELL_AREA` right-column spell panel rectangle for V1 frame blits and fallback clearing.

## Source anchors

- `DEFS.H C013_ZONE_SPELL_AREA`: DM1 right-column spell area.
- GRAPHICS.DAT `C009_GRAPHIC_MENU_SPELL_AREA_BACKGROUND`: `87×25` spell panel bitmap.
- Firestaff V1 placement for the spell background is `(224,90,87,25)`, directly below the `C011` action area.

## Implemented

- Added `M11_GameView_GetV1SpellAreaZone(...)` for probe-visible spell-area geometry.
- Routed V1 spell-frame blit through the helper.
- Routed partial-frame fallback clear height through the helper, using `spellY + spellH` instead of duplicating the spell bounds.

## New invariant

- `INV_GV_300I`: spell area zone matches source `C013` right-column geometry.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`
- Probe result: `481/481 invariants passed`
- CTest result: `100% tests passed, 0 tests failed out of 5`

## Probe excerpt

```text
PASS INV_GV_300I spell area zone matches source C013 right-column geometry
# summary: 481/481 invariants passed
```
