# DM1 all-graphics phase 58 — object C2500 zone placement

## Change

Bound the normal viewport object sprite placement path to the ReDMCSB/layout-696 `C2500_ZONE_` source coordinates for non-alcove objects/creatures:

```c
(C2500_ZONE_ | MASK0x8000_SHIFT_OBJECTS_AND_CREATURES)
  + scaleIndex * 4
  + viewCell
```

The existing G2030 scale bucket selection and G0217/G0223 pile shifts are preserved, but the base floor-placement point now comes from the source layout table instead of the earlier approximate face-rectangle quadrant placement when drawing in the main viewport.

## Gate

Added invariant:

- `INV_GV_114C3` — object placement binds C2500 layout-696 source zone samples

Pinned source samples:

- scale 0, cell 2 → `(127,70)`
- scale 1, cell 3 → `(25,70)`
- scale 4, cell 3 → `(222,70)`

## Visual capture

Focused screenshots:

- `verification-m11/c2500-object-zone-20260425-143900/37_focused_d1c_dagger_object_vga.png`
- `verification-m11/c2500-object-zone-20260425-143900/39_focused_d1c_multi_object_shift_vga.png`

Visual review:

- Object sprites are visible on the floor plane.
- No obvious opaque mask rectangles.
- No major clipping observed.
- Distant tiny object remains hard to judge precisely, but does not look obviously wall/ceiling-misplaced.

## Verification

```text
PASS INV_GV_114C3 object placement binds C2500 layout-696 source zone samples
# summary: 394/394 invariants passed
ctest: 4/4 PASS
```

## Remaining

- Extend the same exact-zone binding to projectile `C2900_ZONE_` and creature `C3200_ZONE_` paths.
- Replace remaining side-pane/legacy fallback placement where it is still intentionally approximate.
