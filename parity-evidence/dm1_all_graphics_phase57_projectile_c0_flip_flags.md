# DM1 all-graphics phase 57 — projectile C0 back/rotation flip flags

## Change

Projectile rendering now carries DM1-style bitmap flip flags through the normal viewport draw path:

- bit 0: horizontal flip
- bit 1: vertical flip

Added a scaled blitter that supports combined horizontal + vertical flips, then passed projectile flip flags from sampled viewport cells into both center-lane and side-pane projectile sprite drawing.

This begins binding the remaining ReDMCSB `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF` projectile path beyond bitmap-index deltas:

- C0 projectile aspect type (`HAS_BACK_GRAPHIC_AND_ROTATION`) can now request vertical flips.
- C3 projectile aspect type (`NO_BACK_GRAPHIC_AND_NO_ROTATION`, e.g. fireball/explosion-style) remains unflipped.
- Existing G0210 bitmap delta behavior remains intact.

## Gate

Added invariant:

- `INV_GV_245E` — projectile C0 back/rotation aspect applies horizontal+vertical flip flags while C3 fireball stays unflipped

The gate verifies representative C0 parity/cell cases and confirms fireball's C3 aspect does not pick up accidental flip flags.

## Verification

```text
PASS INV_GV_245E projectile C0 back/rotation aspect applies horizontal+vertical flip flags while C3 fireball stays unflipped
# summary: 393/393 invariants passed
ctest: 4/4 PASS
```

## Remaining

This does not yet replace approximate projectile placement with the exact `C2500_ZONE_ | MASK0x8000_SHIFT_OBJECTS_AND_CREATURES` coordinate path. That remains the next major projectile/object viewport pass.
