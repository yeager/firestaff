# Pass404 — DM1 V1 side contents center-blocker occlusion gate

Status: `PASS404_DM1_V1_SIDE_CONTENTS_CENTER_BLOCKER_OCCLUSION_PROVEN`

## ReDMCSB-first source audit
- ReDMCSB square draw order: `DUNVIEW.C:8491-8533`
- CSBWin corroboration: `Viewport.cpp:331-342 and DrawViewport loop at 6762`

## Firestaff guards
- `side contents gate before item/creature/projectile draws: m11_game_view.c:15179`
- `side explosion gate before deferred side explosion draw: m11_game_view.c:15440`

## Verdict
- Closed blocker: side contents and deferred side explosions are bounded by the nearest non-open center wall/door before drawing item/creature/projectile/explosion primitives.
- Scope guard: source/order closure only; no original pixel-parity or DOS runtime capture claim.

Manifest: `parity-evidence/verification/pass404_dm1_v1_side_contents_center_blocker_occlusion_gate/manifest.json`
