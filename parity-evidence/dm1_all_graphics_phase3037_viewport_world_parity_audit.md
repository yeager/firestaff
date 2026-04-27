# DM1 all-graphics phase 3037 — viewport/world parity audit

Date: 2026-04-27
Scope: Firestaff V1 DM1 viewport, walls, floor items, ornaments, projectiles, and creatures only.  HUD/champion top-row, original overlay/capture, V2, and DM1 V1 broader-original work are intentionally out of scope.

## What changed

Corrected stale probe labels for the DM1 viewport base-panel asset checks:

- `INV_GV_104` now describes GRAPHICS.DAT `C078` as the **floor** panel (`224x97`).
- `INV_GV_105` now describes GRAPHICS.DAT `C079` as the **ceiling** panel (`224x39`).

The checked graphic numbers, dimensions, renderer constants, and runtime behavior were already correct; this is evidence/probe text cleanup so the local gate no longer contradicts the source-bound viewport base seam (`INV_GV_414`).

## Source anchors rechecked

- ReDMCSB `DEFS.H`: `M650_GRAPHIC_FLOOR_SET_0_FLOOR = 78`, `M651_GRAPHIC_FLOOR_SET_0_CEILING = 79`.
- ReDMCSB `DUNVIEW.C F0128_DUNGEONVIEW_Draw_CPSF`: draws `C079` ceiling at viewport-local `(0,0)` and `C078` floor at `(0,39)` with parity-dependent horizontal flips.
- Firestaff `m11_game_view.c`: `M11_GFX_FLOOR_PANEL = 78`, `M11_GFX_CEILING_PANEL = 79`; viewport aperture remains `(0,33,224,136)`.
- Layout/source-zone seams already covered by the probe:
  - `INV_GV_413`: `C2500` object, `C2900` projectile, and `C3200` creature source points.
  - `INV_GV_414`: `C079` ceiling then `C078` floor inside the aperture.
  - `INV_GV_415`: current viewport source draw-order seam.
  - `INV_GV_38A..38K`, `38R..38AB`: focused viewport feature/change/clip gates for pits, fields, floor ornaments, wall ornaments, items, projectiles, and creatures.
  - `INV_GV_114B..114E3`, `245B..245F`, `256B..256D`: source-backed object/projectile/creature aspect and placement seams.

## Verification

Command run after the label correction:

```sh
cmake --build build --target firestaff_m11_game_view_probe -- -j2 && \
FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
```

Result:

```text
[100%] Built target firestaff_m11_game_view_probe
PASS INV_GV_104 floor panel graphic 78 loads as 224x97
PASS INV_GV_105 ceiling panel graphic 79 loads as 224x39
PASS INV_GV_413 viewport content placement seams expose source C2500 object, C3200 creature, and C2900 projectile points inside C007
PASS INV_GV_414 viewport base uses source ceiling C079 224x39 then floor C078 224x97 inside the 224x136 aperture
PASS INV_GV_415 viewport source draw-order seam is pinned from base/pits/ornaments/walls through doors/buttons
PASS INV_GV_38A focused viewport: D1C normal pit source blit changes the corridor frame
PASS INV_GV_38X focused viewport: D1C normal pit clips inside the DM1 viewport rectangle
PASS INV_GV_38I focused viewport: all visibly drawable floor ornament positions change their corridor frames
PASS INV_GV_38K focused viewport: all source-bound wall ornament specs change their wall frames
PASS INV_GV_38L focused viewport: D1C Trolin creature sprite changes the corridor frame
PASS INV_GV_38AB focused viewport: D1C Trolin creature clips inside the DM1 viewport rectangle
PASS INV_GV_38M focused viewport: D1C fireball projectile sprite changes the corridor frame
PASS INV_GV_38T focused viewport: D1C fireball projectile clips inside the DM1 viewport rectangle
PASS INV_GV_38N focused viewport: D1C dagger object sprite changes the corridor frame
PASS INV_GV_38V focused viewport: D1C dagger object clips inside the DM1 viewport rectangle
PASS INV_GV_245F projectile placement binds C2900 layout-696 source zone samples
PASS INV_GV_256B creature placement binds C3200 layout-696 source zone samples
PASS INV_GV_256D side-cell creature placement binds C3200 left/right source zone samples
# summary: 577/577 invariants passed
```

## Current evidence status

Source/data seams for this worker's region are strong: viewport aperture, floor/ceiling base graphics, source draw order, wall and floor ornament source placement, object/projectile/creature source coordinate families, and viewport clipping gates all pass locally with `GRAPHICS.DAT` available.

This still does **not** close pixel-perfect DM1 runtime parity.  The remaining blocker is unchanged: use a semantically matched original DM1 gameplay capture/route before treating pixel deltas as final parity evidence.
