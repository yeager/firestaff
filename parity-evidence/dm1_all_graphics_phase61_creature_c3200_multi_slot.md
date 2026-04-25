# DM1 all-graphics phase 61 — creature C3200 multi-slot placement

## Change

Extended creature placement so multi-creature duplicate slots also resolve through the layout-696 `C3200_ZONE_` helper instead of the older Graphic558 midpoint approximation.

Phase 60 anchored single center-lane creatures to C3200; this pass makes the multi-creature branch use the same source coordinate family before local face-rect conversion.

## Gate

Added invariant:

- `INV_GV_256C` — creature draw path prefers C3200 over older G0224 midpoint for single front slot

Pinned distinction:

- older `G0224` front slot for coord set 0/D1/single: `(109,111)`
- source `C3200` layout point for coord set 0/D1/single: `(112,111)`

This prevents regressions back to the older approximate midpoint table.

## Verification

```text
PASS INV_GV_256B creature placement binds C3200 layout-696 source zone samples
PASS INV_GV_256C creature draw path prefers C3200 over older G0224 midpoint for single front slot
# summary: 397/397 invariants passed
ctest: 4/4 PASS
```

## Remaining

- Side-cell creature placement still needs the non-center C3200 groups, not just center groups.
- Full multi-creature screenshot coverage can be added once a stable focused multi-creature fixture exists.
