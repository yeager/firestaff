# DM1 all-graphics parity — phase 1917–1936: V1 spell label cell source zones

## Scope

Make the V1 spell/rune label cell source graphic and sub-rows explicit and probe-visible.

## Source anchors

- ReDMCSB `DEFS.H` names `C011_GRAPHIC_MENU_SPELL_AREA_LINES` as the spell-area line/cell overlay graphic.
- The M11 GRAPHICS.DAT asset is `14×39`, i.e. three `14×13` rows.
- M11 uses the lower two rows for visible rune-label cell states:
  - available row: `x=0,y=13,w=14,h=13`
  - selected/entered row: `x=0,y=26,w=14,h=13`

## Implemented

- Added `M11_GameView_GetV1SpellAreaLinesGraphicId()`.
- Added `M11_GameView_GetV1SpellLabelCellSourceZone(...)`.
- Routed `m11_blit_spell_label_cell(...)` through the helpers instead of hard-coded source coordinates.
- Added invariant coverage for the exact C011 graphic id and source rectangles.

## New invariant

- `INV_GV_300Q`: V1 spell label cells use source C011 lines graphic rows for available/selected states.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`

## Probe excerpt

```text
PASS INV_GV_300Q V1 spell label cells use source C011 lines graphic rows for available/selected states
```
