# DM1 V1 movement core lane source lock

Status: **PASS**

Scope: input -> command queue -> turning/collision -> timing/sensors. Viewport/wall rendering is intentionally out of scope.

## ReDMCSB citations

- COMMAND.C:106-121 - mouse movement rows map visible arrows and dungeon viewport to command ids
- COMMAND.C:677-685 - PC-34 keyboard movement rows map keypad/arrow codes to turn and step commands
- IO2.C:5-61 - PC input normalizes shifted extended movement keys before command-table lookup
- COMMAND.C:2045-2156 - F0380 locks the queue, keeps gated movement queued, dequeues one command, then dispatches turns and steps
- CLIKMENU.C:142-174 - F0365 turning sets stop-wait, handles stairs as a special case, otherwise applies direction change with sensor boundaries
- CLIKMENU.C:180-347 - F0366 steps charge stamina before resolution, blocks walls/doors/fakewalls/groups before F0267 for non-empty parties, preserves BUG0_85 by skipping group collision when the party is empty, and applies cooldown only after accepted movement
- DUNGEON.C:1371-1440 - relative forward/right deltas are applied from direction tables before current-map square lookup
- CHAMPION.C:1180-1215 - movement cadence is derived from load, feet wounds, and Boots of Speed
- MOVESENS.C:316-443 - F0267 owns the source/destination move-result contract and projectile-impact precheck
- MOVESENS.C:738-818 - accepted party moves record result/timing/scent then run source leave and destination enter sensor passes

## Firestaff evidence

- dm1_v1_input_command_queue_pc34_compat.c - PC-34 input rows, queue lock, gated movement retention, pending-click replay, and blocked-input discard
- dm1_v1_movement_command_core_pc34_compat.c - F0380-to-F0365/F0366 seam: turns bypass movement gates, steps apply stamina/collision/group blocking/timing
- memory_movement_pc34_compat.c - pure movement legality and target-square result semantics behind F0366, including the empty-party group-collision bug
- dm1_v1_movement_timing_pc34_compat.c - post-step cadence mirrors F0310/F0366 timing side effects
- test_dm1_v1_movement_command_core_pc34_compat.c - behavior coverage for queued input through turn/step dispatch, collision, stamina, timing, and sensor order
- test_dm1_v1_command_movement_sensor_timing_pc34_compat.c - integration coverage for PC-34 movement queue, collision, turn, sensor, timing edges, and BUG0_85 empty-party group passage

## Required gates

- dm1_v1_input_command_queue_pc34_compat
- dm1_v1_movement_command_core_pc34_compat
- dm1_v1_movement_timing_pc34_compat
- dm1_v1_command_movement_sensor_timing_pc34_compat
- dm1_v1_movement_command_gate_source_lock
- dm1_v1_command_movement_sensor_timing_source_lock
