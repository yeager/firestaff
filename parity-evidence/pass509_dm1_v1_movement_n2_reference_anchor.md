# Pass509 - DM1 V1 movement N2 reference anchor

Status: PASS509_DM1_V1_MOVEMENT_N2_REFERENCE_ANCHORED

Scope: movement/forflyttning only. This binds the input->command->movement source-lock lane to N2-local ReDMCSB, Greatstone, and original DM1 PC34 anchors.

## ReDMCSB source audit

- PASS IO2.C:27-61 - F0540_INPUT_Crawcin: PC-34 shifted cursor input is normalized into K/L/M/P command-table codes before command enqueue.
- PASS COMMAND.C:106-121,636-685 - G0448/G0459 movement input tables: Mouse movement arrows and PC-34 keyboard rows map to C001/C002 turn and C003..C006 movement commands.
- PASS COMMAND.C:2045-2156 - F0380_COMMAND_ProcessQueue_CPSC: F0380 gates disabled movement before dequeue, then dispatches turn or move commands.
- PASS CLIKMENU.C:142-347 - F0365/F0366 turn and movement handlers: Turn changes party direction through sensor leave/enter; step resolves deltas, blockers, F0267 movement, and cooldown timing.
- PASS DUNGEON.C:1371-1391 - F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement: Relative stepping applies forward deltas, then a simulated-right-turn strafe delta.
- PASS MOVESENS.C:738-818 - F0267_MOVE_GetMoveResult_CPSCE: Accepted party movement records the result tuple, scent/timing state, and source-before-destination sensor order.

## N2-local reference anchors

- PASS /Users/bosse/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/DUNGEON.DAT sha256 d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85
- PASS /Users/bosse/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/GRAPHICS.DAT sha256 2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e
- PASS /Users/bosse/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/TITLE sha256 adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745
- PASS /Users/bosse/.openclaw/data/firestaff-original-games/DM/_manifests/dm_pc34_greatstone_item_by_item_diff_20260510.json result PASS with 0 mismatches.
- PASS Greatstone PC34 graphics URL: http://greatstone.free.fr/dm/db_data/dm_pc_34/graphics.dat/graphics.dat.html
- PASS Greatstone PC34 dungeon URL: http://greatstone.free.fr/dm/db_data/dm_pc_34/dungeon.dat/dungeon.html

## Firestaff evidence consumed

- PASS tools/verify_pass423_dm1_v1_input_command_movement_pipeline_source_lock.py - existing movement gate/evidence remains present
- PASS tools/verify_pass507_dm1_v1_movement_stairs_group_timing_source_lock.py - existing movement gate/evidence remains present
- PASS parity-evidence/pass507_dm1_v1_movement_stairs_group_timing_source_lock.md - existing movement gate/evidence remains present

## Not claimed

- original DOS keyboard-buffer transcript
- representative original movement/HUD/viewport overlay parity
- viewport/wall or pass435 route promotion
