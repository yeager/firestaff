# DM1 all-graphics phase 96 — spell C011 capture gate + matrix update

## Problem

The parity matrix still listed rune glyphs as `UNPROVEN / Unknown mapping status`, even though the current spell-panel slice already uses the source rune-label graphic:

- `C011_GRAPHIC_MENU_SPELL_AREA_LINES`
- 14×39 total
- three 14×13 cells
- rune encoding from `SYMBOL.C:F0399`: `0x60 + 6*row + col`

The capture smoke also did not prove the spell-panel screenshot was actually showing the native C011 selected-rune cell rather than the procedural fallback.

## Change

Updated `PARITY_MATRIX_DM1_V1.md` rune row to record current status:

- `MATCHED` for C011 cell identity, split, and rune encoding in the current spell-panel surface
- still `UNPROVEN` for exact original spell-panel placement / pixel overlay

Extended `run_firestaff_m11_ingame_capture_smoke.sh` to inspect `04_ingame_spell_panel_latest.ppm` after the scripted first rune input:

- selected rune cell region: `x=83..96`, `y=43..55`
- requires source C011-style brown/red coverage
- catches fallback/procedural regressions

## Gate

```text
./run_firestaff_m11_ingame_capture_smoke.sh
In-game capture smoke PASS: 6 screenshots
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
# summary: 418/418 invariants passed
ctest --test-dir build --output-on-failure
5/5 PASS
```
