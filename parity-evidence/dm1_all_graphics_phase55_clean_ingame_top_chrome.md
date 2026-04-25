# DM1 all-graphics phase 55 — clean in-game top chrome

## Change

Normal V1 gameplay no longer draws the Firestaff/Dungeon title/debug text bar above the viewport. The top title/debug strip is now diagnostic-only behind `showDebugHUD`.

This keeps normal in-game captures focused on source-style viewport + right/bottom panels instead of looking like a debug screenshot.

## Gate

Added invariant:

- `INV_GV_350` — normal V1 top chrome strip contains no title/debug text pixels

The gate draws a non-debug V1 frame with GRAPHICS.DAT action/spell assets loaded and verifies the former title strip has no white/yellow text pixels.

## Visual capture

Fresh capture series:

- `verification-m11/ui-clean-ingame-20260425-142905/01_ingame_start_latest.png`
- `verification-m11/ui-clean-ingame-20260425-142905/04_ingame_spell_panel_latest.png`
- `verification-m11/ui-clean-ingame-20260425-142905/05_ingame_after_cast_latest.png`

Visual review:

- Top Firestaff/title/debug text strip is gone in normal gameplay.
- Best current presentable frame: `05_ingame_after_cast_latest.png`.
- Remaining issue: viewport corruption/noise still exists and needs a separate viewport asset/composition pass.
- Bottom/status text can still appear in spell/cast flows and should be source-bound or suppressed in a later UI cleanup pass.

## Verification

```text
PASS INV_GV_350 normal V1 top chrome strip contains no title/debug text pixels
# summary: 391/391 invariants passed
ctest: 4/4 PASS
```
