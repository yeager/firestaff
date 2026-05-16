# Pass507 - DM1 V1 movement/stairs/group/timing source lock

Status: PASS507_DM1_V1_MOVEMENT_STAIRS_GROUP_TIMING_SOURCE_LOCKED

Lane: movement-related verifiers, parity evidence, and CTest wiring only. No viewport/pass435 dependency.

## ReDMCSB source audit
- COMMAND.C:2075-2155 - F0380 locks queue, gates movement before dequeue, replays pending click, then dispatches turn/move handlers.
- CLIKMENU.C:237-256 - F0366 applies living-champion stamina before arrow/stair/blocker handling.
- CLIKMENU.C:224-233 - Movement arrow tables source-lock forward/right/back/left deltas.
- DUNGEON.C:1371-1391 - Relative movement applies forward deltas, then simulated-right-turn strafe deltas.
- CLIKMENU.C:264-275 - Current-square and target-square stairs route through F0364/F0267 before ordinary blocker flow.
- CLIKMENU.C:279-287 - Wall, closed-door and closed-real-fakewall blockers happen before accepted movement.
- CLIKMENU.C:291-313 - Empty-party group collision is skipped; non-empty parties block on F0175_GROUP_GetThing and trigger reaction.
- CLIKMENU.C:293-326 - Blocked movement discards input and returns before F0267/timing/sensor side effects.
- CLIKMENU.C:328-346 - Successful movement sets disabled ticks from the slowest living champion and clears projectile movement cooldown.
- CHAMPION.C:1180-1214 - Champion movement cadence depends on load/maxLoad, feet wounds, and Boots of Speed.
- MOVESENS.C:738-779 - F0267 records result coordinates and last-movement/scent only for real party square changes with champions.
- MOVESENS.C:801-818 - Accepted party movement processes source leave before destination enter; party-on-group deletes the group.
- MOVESENS.C:453-844 - Group moves cannot enter party/occupied group squares; a later group-move event is scheduled.
- GAMELOOP.C:150-215 - Main loop ages movement/projectile cooldown before processing queued commands.

## Firestaff coverage
- src/dm1/dm1_v1_input_command_queue_pc34_compat.c:286 - F0380 move gate/replay/dispatch seam.
- src/memory/memory_movement_pc34_compat.c:200 - relative step delta seam.
- src/memory/memory_movement_pc34_compat.c:820 - empty-party exception and group block seam.
- src/dm1/dm1_v1_movement_command_core_pc34_compat.c:182 - stamina/stairs/group/input-discard/timing command seam.
- src/dm1/dm1_v1_movement_timing_pc34_compat.c:65 - successful-step timing seam.
- src/memory/memory_sensor_execution_pc34_compat.c:325 - source-ordered enter/leave sensor walking.
- tests/test_dm1_v1_movement_core_pc34_compat.c - covers queue gates, tile blockers, pits, and empty-party group bug.
- tests/test_dm1_v1_command_movement_sensor_timing_pc34_compat.c - covers blocked side-effect suppression, group collision, empty-party bug, and timing cooldowns.

## Chained static gates
- dm1_v1_command_movement_sensor_timing_source_lock rc=0
- dm1_v1_party_movement_sensor_order_source_lock rc=0
- dm1_v1_movement_timing_source_lock rc=0
- v1_movement_legality_source_lock rc=0

Manifest: parity-evidence/verification/pass507_dm1_v1_movement_stairs_group_timing_source_lock/manifest.json
