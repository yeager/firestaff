# Pass404 — DM1 V1 side contents center-blocker occlusion gate

Status: `PASS404_DM1_V1_SIDE_CONTENTS_CENTER_BLOCKER_OCCLUSION_PROVEN`

## ReDMCSB-first source audit
- ReDMCSB square draw order: `DUNVIEW.C:8491-8533`
- CSBWin corroboration: `Viewport.cpp:331-342 and DrawViewport loop at 6762`

## Firestaff guards
- `side contents gate before item/creature/projectile draws: tokens=m11_draw_dm1_side_contents_at_depth/blockingCenterDepth>=depth/m11_draw_dm1_f0115_floor_item_sprite (current m11_game_view.c:28228)`
- `F0128 per-depth interleave: side contents dispatch precedes the same-depth center contents route in m11_draw_viewport (current m11_game_view.c:38924)`
- `side explosion gate before deferred side explosion draw: tokens=m11_draw_dm1_deferred_explosion_pass/m11_dm1_nearest_blocking_center_depth_index/blockingCenterDepth>=depth/m11_draw_dm1_deferred_side_explosion (current m11_game_view.c:28568)`

## Verdict
- Closed blocker: side contents and deferred side explosions are bounded by the nearest non-open center wall/door before drawing item/creature/projectile/explosion primitives.
- Scope guard: source/order closure only; no original pixel-parity or DOS runtime capture claim.

Manifest: `parity-evidence/verification/pass404_dm1_v1_side_contents_center_blocker_occlusion_gate/manifest.json`
