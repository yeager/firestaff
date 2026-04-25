# DM1 all-graphics phase 124 — overlay matrix/blocker sync

## Problem

Runtime passes 115–123 materially changed overlay parity status:

- dialog now has source message zones, source-width split, choice zones, input flow, and patch graphics
- endgame now has source `THE END`, champion mirror zones, restart/quit boxes, and champion names

But `PARITY_MATRIX_DM1_V1.md` and `V1_BLOCKERS.md` still described dialog/endgame mostly at the earlier pass-113 state.

## Change

Updated `PARITY_MATRIX_DM1_V1.md`:

- dialog row now records the pass 107–121 source-backed dialog path
- endgame row text now records the pass 122–123 source-backed endgame subset
- bottom line updated to pass 124

Updated `V1_BLOCKERS.md`:

- dialog blocker section now records C000/C450/C469/C471/C462–C467/M621–M623/input-flow coverage
- endgame narrowing section now records C006/C346/C412–C415/restart-quit/champion-name coverage
- representative gate count updated to `437/437`
- remaining gaps narrowed to endgame portraits/title/skill list and original overlay comparison captures

## Gate

Documentation-only pass, backed by previous runtime gates:

```text
PASS INV_GV_165C V1 endgame uses source C006 The End graphic
PASS INV_GV_165D V1 endgame uses source champion mirror graphic zone
PASS INV_GV_165E V1 endgame prints champion name at source coordinate
PASS INV_GV_172O V1 two/four-choice dialogs apply source M622/M623 patches
# summary: 437/437 invariants passed
ctest: 5/5 PASS
```
