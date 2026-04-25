# DM1 all-graphics phase 100 — blocker addendum for icon/capture work

## Problem

The recent all-graphics work had updated the parity matrix and evidence files, but `V1_BLOCKERS.md` still only described the older pass-44/pass-45 spell/font state and the title/audio follow-ups. The blocker file needed a durable addendum for the new source-backed object-icon, inventory fixture, and C011 capture-smoke gates.

## Change

Added an all-graphics addendum to `V1_BLOCKERS.md` covering passes 84–99:

- action empty-hand icon `C201`
- source object icon resolver for action cells
- `ActionSetIndex` gating
- dynamic `F0033_OBJECT_GetIconIndex` variants
- inventory slot `F0038_OBJECT_DrawIconInSlotBox` semantics
- action `G0498` palette remap vs inventory direct blit
- deterministic capture champion with dagger fixture
- capture-smoke checks for:
  - action dagger cyan coverage
  - inventory dagger source dark-gray preservation
  - selected-rune C011 brown/red pattern
- updated matrix status/counts
- remaining honest gaps: pointer/held-object surfaces, original placement overlays, viewport content/draw-order parity

## Gate

Documentation-only pass. It records the immediately preceding verified state:

```text
firestaff_m11_game_view_probe: 418/418 invariants passed
ctest: 5/5 PASS
run_firestaff_m11_ingame_capture_smoke.sh: PASS
```
