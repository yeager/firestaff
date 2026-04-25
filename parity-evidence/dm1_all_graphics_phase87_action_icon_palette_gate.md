# DM1 all-graphics phase 87 — action icon palette-change gate

## Problem

Phase 85 switched action-hand item rendering to source object icons from graphics `42..48` and applied the source `G0498_auc_Graphic560_PaletteChanges_ActionAreaObjectIcon` palette change (`12 -> C04 cyan`). The behavior was implemented, but the invariant only proved that a dagger icon differed from the empty-hand baseline. It did not directly prove the G0498 palette change happened.

## Change

Extended the dagger/action-cell focused gate:

- `INV_GV_305` still proves an `ActionSetIndex > 0` item blits a source object icon.
- New `INV_GV_306` counts cyan pixels in the 16×16 dagger action icon and requires substantial cyan coverage, proving source icon color `12` was remapped to `C04` by the action-area palette rule.

This is specifically source-bound to ReDMCSB `ACTIDRAW.C`:

```c
F0662_ApplyPaletteChanges(..., G0498_auc_Graphic560_PaletteChanges_ActionAreaObjectIcon)
G0498 = { ..., [12] = 4, ... }
```

## Gate

```text
PASS INV_GV_305 action-hand icon cells: ActionSetIndex>0 item blits source object icon
PASS INV_GV_306 action-hand icon cells: source object icon applies G0498 color-12-to-cyan palette change
# summary: 414/414 invariants passed
ctest --test-dir build --output-on-failure
5/5 PASS
```
