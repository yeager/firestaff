# DM1 all-graphics phase 85 — action-hand source object icons

## Problem

Phase 84 fixed empty-hand action icons, but non-empty action-hand items with `ActionSetIndex > 0` still used the viewport/object sprite resolver (`M612` family) scaled to 16×16. ReDMCSB `ACTIDRAW.C:F0386_MENUS_DrawActionIcon` does not do that. It calls:

```c
F0033_OBJECT_GetIconIndex(thing)
F0036_OBJECT_ExtractIconFromBitmap(iconIndex, ...)
F0662_ApplyPaletteChanges(... G0498 ...)
```

So the right-column action cell must use source object icon atlases graphics `42..48`, not viewport sprites.

## Change

Added source-backed object icon resolution for action cells:

- resolves the same ObjectInfo index ranges as `F0141_DUNGEON_GetObjectInfoIndex`
- uses the `Type` column of `G0237_as_Graphic559_ObjectInfo[180]` as the icon index
- extracts 16×16 icons from graphics `42..48` using the existing action-icon extractor
- keeps phase-84 palette change behavior (`G0498`: color `12 -> C04 cyan`)

`m11_draw_dm_action_icon_cells(...)` now uses source object icons for `ActionSetIndex > 0` items. `ActionSetIndex == 0` items still remain plain cyan as in F0386.

## Gate

Added invariant:

- `INV_GV_305` — dagger/action-set item changes the inner icon from empty-hand baseline by blitting the source object icon.

```text
PASS INV_GV_300 action-hand icon cells: both living champions get cyan backdrop (or no assets)
PASS INV_GV_300B action-hand icon cells: empty living hand blits source empty-hand icon
PASS INV_GV_305 action-hand icon cells: ActionSetIndex>0 item blits source object icon
# summary: 413/413 invariants passed
ctest --test-dir build --output-on-failure
5/5 PASS
```
