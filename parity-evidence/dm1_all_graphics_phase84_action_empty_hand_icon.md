# DM1 all-graphics phase 84 — source empty-hand action icon

## Problem

The DM1 action-hand cells were structurally source-bound, but living champions with empty action hands still rendered as a plain cyan inner cell. ReDMCSB `ACTIDRAW.C:F0386_MENUS_DrawActionIcon` instead uses:

```c
C201_ICON_ACTION_ICON_EMPTY_HAND
F0036_OBJECT_ExtractIconFromBitmap(...)
F0662_ApplyPaletteChanges(... G0498 ...)
```

So the empty hand is an actual 16x16 object icon, not just a cyan block.

## Change

Added a small source-style object icon extractor for the action area:

- object icon graphics start at graphic `42`
- `32` icons per graphic
- each icon is `16x16`
- empty hand icon index is `201`, so it comes from graphic `48`, local icon `9`
- applies the action-area palette change from `G0498`: color `12 -> C04 cyan`

`m11_draw_dm_action_icon_cells(...)` now draws the empty-hand icon when the action hand is `THING_NONE`/`THING_ENDOFLIST`. Items with `ActionSetIndex == 0` still intentionally remain plain cyan, matching F0386.

## Gate

Added invariant:

- `INV_GV_300B` — empty living hand blits source empty-hand icon.

Also fixed the focused action-icon probe fixture to initialize champion inventories to `THING_NONE`, not zero-valued synthetic things.

```text
PASS INV_GV_300 action-hand icon cells: both living champions get cyan backdrop (or no assets)
PASS INV_GV_300B action-hand icon cells: empty living hand blits source empty-hand icon
# summary: 412/412 invariants passed
ctest --test-dir build --output-on-failure
5/5 PASS
```
