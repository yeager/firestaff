# HUD spell/action candidate — C013 spell-area overlay

## Scope

Bounded V1 HUD pass for the spell/action/right-column area.

## Source anchors

- ReDMCSB `CASTER.C`:
  - `F0733_FillZoneByIndex(C013_ZONE_SPELL_AREA, C00_COLOR_BLACK)` when no caster is selected.
  - `F0660_(C009_GRAPHIC_MENU_SPELL_AREA_LINES, C013_ZONE_SPELL_AREA, CM1_COLOR_NO_TRANSPARENCY)` before drawing controls/symbols.
  - `F0393_MENUS_DrawSpellAreaControls`, `F0397_MENUS_DrawAvailableSymbols`, and `F0398_MENUS_DrawChampionSymbols` then stay within the spell-area zone family.
- ReDMCSB `DEFS.H` / layout-696 zone family already exposed in Firestaff:
  - `C013_ZONE_SPELL_AREA` = right-column spell area `(224,90,87,25)`.
  - `C221/C224` caster panel/tab.
  - `C245..C260` available-symbol zones.
  - `C261..C264` champion/selected-symbol zones.
- Firestaff already source-bound the C011 line-cell graphic rows: available row `(0,13,14,13)`, selected row `(0,26,14,13)`.

## Implemented

- Normal V1 (`showDebugHUD=0`, non-V2) no longer draws the old large procedural spell workbench panel over the viewport.
- Added a compact V1 spell overlay that stays inside `C013_ZONE_SPELL_AREA` and uses the source-backed `C009` spell backdrop plus selected/available `C011` 14×13 cells.
- Debug/V2 paths retain the older workbench panel for diagnostics/prototype behaviour.
- Updated the in-game capture smoke to look for the selected C011 cell in the right-column C013 area instead of the old viewport-modal coordinates.

## Probe coverage

- Added `INV_GV_300AQ`: opens the spell panel in normal V1, verifies a selected C011 cell appears at the right-column spell area, and checks the old modal-panel brown top border is not present.

## Remaining blocker

Exact per-symbol placement/glyph rendering inside `C245..C264` still needs a full ZONES.H/PANEL.C pixel walk or original runtime overlay. This pass intentionally avoids inventing a final rune glyph layout; it only removes the clearly invented viewport modal from normal V1 and keeps the surface bounded to the source spell area.
