# DM1 all-graphics phase 60 — creature C3200 zone placement

## Change

Bound center-lane creature placement to ReDMCSB/layout-696 `C3200_ZONE_` source coordinates for the three DM1 creature coordinate sets used by the renderer.

For single visible center-lane creature groups, the renderer now anchors the draw rectangle from C3200 center-X / bottom-Y instead of using the generic face-rectangle midpoint. Existing creature pose selection, replacement colors, transparent keying, and GRAPHICS.DAT sprite selection remain unchanged.

## Gate

Added invariant:

- `INV_GV_256B` — creature placement binds C3200 layout-696 source zone samples

Pinned source samples:

- coord set 0, D1, single → `(112,111)`
- coord set 1, D2, pair slot 1 → `(132,90)`
- coord set 2, D3, single → `(112,60)`

## Visual capture

Focused screenshot:

- `verification-m11/c3200-creature-zone-20260425-144405/35_focused_d1c_trolin_creature_vga.png`

Visual review:

- Creature visible and centered in corridor.
- Feet/body sit plausibly on the floor plane.
- No obvious clipping into walls/UI.
- No mask rectangle or bounding-box artifact.

## Verification

```text
PASS INV_GV_256B creature placement binds C3200 layout-696 source zone samples
# summary: 396/396 invariants passed
ctest: 4/4 PASS
```

## Remaining

- Extend exact C3200 anchoring to multi-creature duplicate rectangles instead of the older G0224 approximation.
- Reconcile side-cell creature placement with full C3200 side groups.
