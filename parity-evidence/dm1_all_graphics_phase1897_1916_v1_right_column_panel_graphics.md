# DM1 all-graphics parity — phase 1897–1916: V1 right-column panel graphics

## Scope

Make the V1 right-column action/spell background graphic IDs explicit and probe-visible.

## Source anchors

- ReDMCSB `DEFS.H` names:
  - `C009_GRAPHIC_MENU_SPELL_AREA_BACKGROUND` — spell area background (`87×25`).
  - `C010_GRAPHIC_MENU_ACTION_AREA` — action area background (`87×45`).
- ReDMCSB `F0387_MENUS_DrawActionArea` and `F0394_MENUS_SetMagicCasterAndDrawSpellArea` place these in the right-column `C011`/`C013` zones.

## Implemented

- Added `M11_GameView_GetV1ActionAreaGraphicId()`.
- Added `M11_GameView_GetV1SpellAreaBackgroundGraphicId()`.
- Routed the V1 right-column panel blits through these helpers.
- Added probe coverage for the exact source graphic IDs.

## New invariant

- `INV_GV_300P`: right-column V1 panel graphics use source `C010` action and `C009` spell-area backgrounds.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`

## Probe excerpt

```text
PASS INV_GV_300P right-column V1 panel graphics use source C010 action and C009 spell-area backgrounds
```
