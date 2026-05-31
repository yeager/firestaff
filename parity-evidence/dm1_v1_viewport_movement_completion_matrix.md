# DM1 V1 viewport/movement completion aggregate

Status: `PASS`
Generated: `2026-05-31T02:49:46.104183+00:00`

## ReDMCSB source audit
- `PASS` `post_command_redraw_loop` — `GAMELOOP.C:55-90` `GAMELOOP main input/redraw loop`: main loop redraws viewport from the current party tuple before entering the input wait cycle
- `PASS` `cooldown_age_before_f0380` — `GAMELOOP.C:150-155` `GAMELOOP cooldown tick`: old movement/projectile cooldowns age once per loop tick before queued input processing
- `PASS` `queue_dispatch_turn_move_and_blocked_gates` — `COMMAND.C:2045-2156` `F0380_COMMAND_ProcessQueue_CPSC`: eligible queued turn/move commands dispatch through source handlers after movement-disabled checks
- `PASS` `movement_legality_and_result_chain` — `CLIKMENU.C:180-351` `F0366/F0267 movement blockers and side effects`: wall/door/fakewall/group blockers return before movement side effects; successful moves enter F0267 timing/results
- `PASS` `viewport_far_to_near_wall_replay` — `DUNVIEW.C:8318-8618` `F0128_DUNGEONVIEW_Draw_CPSF`: viewport squares replay source-order far-to-near and then request presentation
- `PASS` `wall_door_alcove_occlusion_contract` — `DUNVIEW.C:6421-6840` `F0116/F0117/F0118 wall and door branches`: wall squares normally occlude/return, while alcoves and front doors explicitly hand contents to F0115
- `PASS` `d2c_floor_field_and_front_door_order` — `DUNVIEW.C:7244-7388` `F0121_DUNGEONVIEW_DrawSquareD2C`: D2C keeps source floor/stairs/field/door/front-content ordering, including the recent D2C field/floor regression surface
- `PASS` `f0115_thing_layer_handoff` — `DUNVIEW.C:4547-5938` `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF`: objects/creatures/projectiles are processed per packed cell, while explosion handling restarts after the packed-cell pass
- `PASS` `drawview_palette_and_present_cadence` — `DRAWVIEW.C:709-900` `F0097_DUNGEONVIEW_DrawViewport`: viewport present uses the single source dungeon palette index and vblank copy cadence rather than invented depth dimming

## Executable gates
- `PASS` `pass381_movement_viewport_walls_source_lock` rc=`0`: command queue -> movement/turn state -> viewport wall redraw and presentation source chain
- `PASS` `pass423_input_command_movement_pipeline_source_lock` rc=`0` status `PASS423_DM1_V1_INPUT_COMMAND_MOVEMENT_PIPELINE_SOURCE_LOCKED`: PC34 input, queue, F0380, F0365/F0366 and command-core regressions
- `PASS` `pass402_movement_cooldown_order` rc=`0` status `ok`: cooldown ageing before F0380 and no same-tick post-decrement
- `PASS` `pass406_movement_legality_completion_gate` rc=`0`: party target-square legality, collision blockers, pits/teleporters/groups, and movement-result chain
- `PASS` `pass406_game_loop_redraw_cadence` rc=`0` status `PASS406_DM1_V1_GAME_LOOP_REDRAW_CADENCE_SOURCE_LOCKED`: game-loop redraw cadence, viewport dirty publication, draw/present/vblank ordering
- `PASS` `pass395_viewport_walls_source_runtime_lock` rc=`0`: wall replay, door two-pass, F0115 handoff, and post-command redraw metadata/runtime contract
- `PASS` `pass405_projectile_explosion_layer_occlusion` rc=`0` status `PASS_PASS405_DM1_V1_VIEWPORT_PROJECTILE_EXPLOSION_LAYER_OCCLUSION`: projectile/explosion layer split, deferred explosion pass, and center/side occlusion guards
- `PASS` `viewport_square_collision_source_lock` rc=`0`: visible viewport cells are map-backed and agree with movement collision square state
- `PASS` `viewport_field_zone_aspect_clip_gate` rc=`0`: field/teleporter zone/aspect clipping behind nearer blockers
- `PASS` `viewport_palette_source_lock_gate` rc=`0`: single ReDMCSB dungeon palette cadence; rejects invented depth palette dimming
- `PASS` `pass434_original_viewport_crop_readiness_gate` rc=`0` status `PASS_PASS434_ORIGINAL_VIEWPORT_CROP_READINESS`: original viewport crop/source readiness is available

## Expected blockers
- `CONFIRMED` `pass435_semantic_original_route_readiness_gate` expected `BLOCKED_PASS435_SEMANTIC_ORIGINAL_ROUTE_NOT_READY` observed `BLOCKED_PASS435_SEMANTIC_ORIGINAL_ROUTE_NOT_READY`: remaining original semantic route blocker: F0365/F0366 dispatch + six non-duplicate semantic route states not yet proven
  - {
  -   "status": "BLOCKED_PASS435_SEMANTIC_ORIGINAL_ROUTE_NOT_READY",
  -   "blockers": [
  -     "pass376 original-route artifacts are quarantined as non-promotable duplicate/non-semantic evidence"
  -   ],
  -   "manifest": "parity-evidence/verification/pass435_dm1_v1_semantic_original_route_readiness_gate/manifest.json",
  -   "report": "parity-evidence/pass435_dm1_v1_semantic_original_route_readiness_gate.md"
  - }

## Decision
Current movement/viewport source-lock gates are green, pass434 crop readiness is green, and pass435 confirms the remaining blocker is original semantic route readiness; no original pixel/route parity is claimed.

Manifest: `parity-evidence/verification/dm1_v1_viewport_movement_completion_matrix/manifest.json`
