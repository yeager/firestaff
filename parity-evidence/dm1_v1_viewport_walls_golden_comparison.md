# DM1 V1 viewport/walls golden comparison source lock

Status: `PASS_SOURCE_LOCKED_ORIGINAL_CAPTURE_BLOCKED`

This is a focused golden-comparison lock for viewport/walls only. It verifies ReDMCSB source facts and current Firestaff evidence gates; it does **not** claim original pixel parity.

## ReDMCSB citations verified

- PASS `viewport-base-and-far-to-near-square-order` — DUNVIEW.C:8318-8542: Viewport composition starts with source floor/ceiling and then draws far-to-near D3/D2/D1/D0 square functions.
- PASS `left-side-door-detail-layering-d3-d2-d1` — DUNVIEW.C:6361-6480, DUNVIEW.C:6900-7031, DUNVIEW.C:7391-7536: Front side doors draw floor/detail pass 1, frame/door detail, then a second F0115 foreground pass with depth-specific door zones.
- PASS `door-ornament-mask-layering-inside-door-bitmap` — DUNVIEW.C:4013-4117, DUNVIEW.C:4218-4335: Door ornaments, thieves-eye holes, and destroyed masks are composed into the temporary door bitmap before viewport blit.
- PASS `open-cell-contents-deferred-draw-stack` — DUNVIEW.C:4547-4582, DUNVIEW.C:4820-4855, DUNVIEW.C:5690-5701: F0115 defers groups/projectiles/explosions while drawing objects first, then creatures, projectiles, explosions/fluxcages.
- PASS `f0115-row-clipping-and-source-zone-families` — DUNVIEW.C:360-380, DUNVIEW.C:4547-4582, DUNVIEW.C:4800-5090, DUNVIEW.C:5658-5688: Object/projectile rows are selected through G2028 and rejected for source far/near edge cases before C2500/C2900 zone blits.
- PASS `zone-family-definitions-for-comparison-seam` — DEFS.H:4228-4260: The comparison seam is source zone-index based: C2500/C2900 content rows and depth-specific M624/M627/M630/M631 door zones.

## Existing evidence joined

- Movement/viewport/wall golden: `PASS`; entry `{'direction': 2, 'mapIndex': 0, 'mapX': 1, 'mapY': 3}`; representative cases `5`.
- Draw stack artifact: `PASS`.
- Original capture integration: `PASS_SOURCE_LOCKED_ORIGINAL_RUNTIME_BLOCKED_ON_ADDRESS_MAP`; capture route `tooling-recovered-source-locked-not-overlay-ready`.
- Original route follow-up: `BLOCKED_MOVEMENT_VIEWPORT_ROUTE_NOT_PROMOTABLE`.

## Boundary

- Source-locked: side-door/detail layering, far-to-near draw order, F0115 object/creature/projectile stack, and C2500/C2900 row clipping.
- Original capture: available artifacts are blocker evidence only; the current route is not promotable as original-faithful movement/viewport pixel reference.
- No push, no <private-host> use, no original-vs-Firestaff pixel parity claim.
