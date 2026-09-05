# Pass509 - DM1 V1 movement reference anchor

Status: PASS509_DM1_V1_MOVEMENT_N2_REFERENCE_ANCHORED

Scope: DM1 V1 movement only. This binds the input-to-command-to-movement lane to repository ReDMCSB and authentic PC 3.4 ZIP members.

## ReDMCSB source audit

- PASS IO2.C:27-61 - F0540_INPUT_Crawcin: PC-34 shifted cursor input is normalized into K/L/M/P command-table codes before command enqueue.
- PASS COMMAND.C:106-121,636-685 - G0448/G0459 movement input tables: Mouse movement arrows and PC-34 keyboard rows map to C001/C002 turn and C003..C006 movement commands.
- PASS COMMAND.C:2045-2156 - F0380_COMMAND_ProcessQueue_CPSC: F0380 gates disabled movement before dequeue, then dispatches turn or move commands.
- PASS CLIKMENU.C:142-347 - F0365/F0366 turn and movement handlers: Turn changes party direction through sensor leave/enter; step resolves deltas, blockers, F0267 movement, and cooldown timing.
- PASS DUNGEON.C:1371-1391 - F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement: Relative stepping applies forward deltas, then a simulated-right-turn strafe delta.
- PASS MOVESENS.C:738-818 - F0267_MOVE_GetMoveResult_CPSCE: Accepted party movement records the result tuple, scent/timing state, and source-before-destination sensor order.

## Authentic PC 3.4 ZIP anchors

- PASS ~/.firestaff/data/dm1/Dungeon-Master_DOS_EN_Version-34.zip::DATA/DUNGEON.DAT sha256 d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85
- PASS ~/.firestaff/data/dm1/Dungeon-Master_DOS_EN_Version-34.zip::DATA/GRAPHICS.DAT sha256 2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e
- PASS ~/.firestaff/data/dm1/Dungeon-Master_DOS_EN_Version-34.zip::TITLE sha256 adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745

## Firestaff evidence consumed

- PASS tools/verify_pass423_dm1_v1_input_command_movement_pipeline_source_lock.py - existing movement gate/evidence remains present
- PASS tools/verify_pass507_dm1_v1_movement_stairs_group_timing_source_lock.py - existing movement gate/evidence remains present
- PASS parity-evidence/pass507_dm1_v1_movement_stairs_group_timing_source_lock.md - existing movement gate/evidence remains present

## Not claimed

- original DOS keyboard-buffer transcript
- representative original movement/HUD/viewport overlay parity
- viewport/wall or pass435 route promotion
