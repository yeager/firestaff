# DM1 all-graphics phase 62 — creature C3200 side-cell placement

## Change

Bound side-cell creature placement to the layout-696 `C3200_ZONE_` left/right source groups.

Center-lane creatures were moved to C3200 in phases 60–61. This pass adds side-cell C3200 source points and uses them in the side-pane creature draw path for both single and duplicate visible creatures.

When a C3200 side point is available, the side draw path anchors the pane rectangle from the source center-X / bottom-Y and suppresses the older inward-edge side-pane positioning adjustment.

## Gate

Added invariant:

- `INV_GV_256D` — side-cell creature placement binds C3200 left/right source zone samples

Pinned source samples:

- coord set 0, D1, left, single → `(-21,111)`
- coord set 0, D1, right, single → `(244,111)`
- coord set 1, D2, left, pair slot 1 → `(35,90)`

## Verification

```text
PASS INV_GV_256D side-cell creature placement binds C3200 left/right source zone samples
# summary: 398/398 invariants passed
ctest: 4/4 PASS
```

## Remaining

- Add focused side-cell creature screenshot fixture once the probe has a stable side-creature setup.
- Review side-cell clipping at extreme negative/right C3200 coordinates against original F0791 clipping behavior.
