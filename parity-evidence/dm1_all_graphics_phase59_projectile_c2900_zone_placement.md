# DM1 all-graphics phase 59 — projectile C2900 zone placement

## Change

Bound projectile sprite placement to the ReDMCSB/layout-696 `C2900_ZONE_` source coordinates:

```c
C2900_ZONE_ + scaleIndex * 4 + viewCell
```

This replaces the previous approximate quadrant positioning in the main viewport with the source projectile zone points. Side-pane/fallback drawing still uses the older bounded placement path.

## Gate

Added invariant:

- `INV_GV_245F` — projectile placement binds C2900 layout-696 source zone samples

Pinned source samples:

- scale 0, cell 2 → `(129,47)`
- scale 1, cell 3 → `(25,47)`
- scale 4, cell 3 → `(202,47)`

## Visual capture

Focused screenshots:

- `verification-m11/c2900-projectile-zone-20260425-144047/36_focused_d1c_fireball_projectile_vga.png`
- `verification-m11/c2900-projectile-zone-20260425-144047/40_focused_d1c_lightning_projectile_vga.png`

Visual review:

- Fireball and lightning projectile sprites are visible.
- No obvious opaque mask rectangles.
- No clipping observed.
- Minor remaining issue: projectile vertical placement reads a little high/floating, especially fireball; this likely needs full F0115 zone/scale reconciliation rather than another invented offset.

## Verification

```text
PASS INV_GV_245F projectile placement binds C2900 layout-696 source zone samples
# summary: 395/395 invariants passed
ctest: 4/4 PASS
```

## Remaining

- Bind creature placement to `C3200_ZONE_`.
- Reconcile projectile apparent vertical height against full F0115 scaled bitmap dimensions / F0791 shift behavior.
