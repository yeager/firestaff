# DM1 all-graphics phase 86 — sync action-icon comments after source icon work

## Problem

Phases 84–85 changed action-hand rendering to use source object-icon atlases for both empty hands and ActionSet-backed items, but two nearby comments still described the old bounded behavior:

- empty hands as cyan-only / out-of-scope for C201
- action-hand item sprites as scaled viewport sprites

Those comments were now false and risked future regressions.

## Change

Updated the comments around:

- `m11_action_set_index_for_thing(...)`
- `m11_draw_dm_action_icon_cells(...)`

They now state the current source behavior:

- `THING_NONE`/`THING_ENDOFLIST` returns ActionSet 0 only because empty-hand icon selection is handled explicitly with `C201_ICON_ACTION_ICON_EMPTY_HAND`.
- living cells blit source object icons from graphics `42..48` for empty hands and `ActionSetIndex > 0` items.
- `ActionSetIndex == 0` items intentionally remain plain cyan.

## Gate

```text
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
# summary: 413/413 invariants passed
ctest --test-dir build --output-on-failure
5/5 PASS
```
