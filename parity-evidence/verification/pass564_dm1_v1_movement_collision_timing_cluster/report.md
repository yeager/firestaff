# Pass564 - DM1 V1 Movement Collision Timing Cluster

Status: pass

No DUNGEON.DAT/GRAPHICS.DAT variants are read by this verifier.

## ReDMCSB Anchors

- COMMAND.C:2045-2156 - F0380 queue lock, movement cooldown gate, dequeue, pending replay, turn/step dispatch
- CLIKMENU.C:142-174 - F0365 turn/stairs/current-square sensor boundary
- CLIKMENU.C:180-347 - F0366 stamina, relative step, blockers, discard, F0267, cooldown install
- DUNGEON.C:1371-1391 - F0150 relative forward/right coordinate math
- MOVESENS.C:438-444 - F0267 immediate party coordinate update
- MOVESENS.C:738-783 - F0267 move-result globals, scent, G0362 timing
- MOVESENS.C:799-822 - F0267 source leave, destination group deletion, destination enter/defer
- CHAMPION.C:1180-1215 - F0310 movement ticks formula
- GAMELOOP.C:150-155 - G0310/G0311 independent per-loop decrement

## Firestaff Anchors

- src/dm1/dm1_v1_input_command_queue_pc34_compat.c - queued-command gate and pending replay
- src/dm1/dm1_v1_movement_command_core_pc34_compat.c - turn/step dispatch, blockers, coordinate update, cooldown
- src/dm1/dm1_v1_movement_timing_pc34_compat.c - movement ticks, G0362 update, cooldown decrement
- tests/test_dm1_v1_movement_command_core_pc34_compat.c - focused command core coverage
- tests/test_dm1_v1_movement_pipeline_pc34_compat.c - pipeline cooldown decrement/release coverage

## Registered CTests

- dm1_v1_movement_command_core_pc34_compat
- dm1_v1_movement_pipeline_pc34_compat
- pass564_dm1_v1_movement_collision_timing_cluster
