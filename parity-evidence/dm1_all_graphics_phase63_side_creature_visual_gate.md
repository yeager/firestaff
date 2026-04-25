# DM1 all-graphics phase 63 — side-cell creature visual gate

## Change

Added a focused D1L side-cell Trolin creature screenshot fixture so the C3200 side-cell placement pass has visual proof, not only coordinate invariants.

New capture:

- `41_focused_d1l_trolin_creature_vga`

## Gate

Added invariant:

- `INV_GV_38R` — focused viewport D1L side-cell Trolin creature differs from empty and center creature frames

This proves the side-cell creature path produces a distinct visible frame after C3200 side-zone binding.

## Visual capture

Fresh screenshot series:

- `verification-m11/c3200-side-creature-20260425-144859/35_focused_d1c_trolin_creature_vga.png`
- `verification-m11/c3200-side-creature-20260425-144859/41_focused_d1l_trolin_creature_vga.png`

Visual review:

- Side-cell creature is visible.
- Placement is plausible for the left-adjacent corridor/side position.
- No obvious mask rectangle.
- No clear clipping into wall/floor.
- Minor issue: creature is small/dark and appears closer to center than strongly left; needs later comparison against original C3200/F0791 clipping behavior.

## Verification

```text
PASS INV_GV_38R focused viewport: D1L side-cell Trolin creature differs from empty and center creature frames
# summary: 399/399 invariants passed
ctest: 4/4 PASS
```
