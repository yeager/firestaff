# DM1 V1 movement command gate source lock

Status: **pass**

Scope: source-lock command queue -> movement dispatch -> blocked/success side effects.

## ReDMCSB citations

- `COMMAND.C:6-16` — command queue storage, first/last indexes, lock, and pending-click fields
- `COMMAND.C:106-121` — secondary mouse movement rows map visible arrows/viewport to C001..C006/C080/C083
- `COMMAND.C:252-260` — legacy movement keyboard rows map keypad 4/5/6/1/2/3 to turn/step commands
- `COMMAND.C:677-685` — PC-34 movement keyboard rows map normalized 0x004B..0x0051 codes to turn/step commands
- `IO2.C:39-61` — PC-34 input normalizes shifted extended arrow scancodes before command-table lookup
- `COMMAND.C:1452-1662` — mouse click path stores pending clicks when locked and enqueues resolved commands otherwise
- `COMMAND.C:1692-1707` — pending click replay occurs after the queue is unlocked
- `COMMAND.C:1709-1813` — keyboard path locks the queue, checks primary then secondary tables, stores matching command, unlocks, and replays pending click
- `COMMAND.C:2045-2156` — F0380 locks the queue, leaves gated movement queued, dequeues commands, replays pending clicks, then dispatches turns/moves
- `CLIKMENU.C:142-174` — turn dispatch changes direction and processes stairs/current-square sensor boundary
- `CLIKMENU.C:180-347` — movement dispatch maps command to relative delta, blocks before F0267, discards input on block, and applies timing only after success
- `MOVESENS.C:738-783` — successful move records destination result and party movement timing/scent state
- `MOVESENS.C:799-818` — successful party move processes source leave then destination enter sensors
- `MOVESENS.C:1553-1794` — sensor walker traverses square thing-list and triggers matching sensor effects in source order
- `DUNGEON.C:1371-1440` — relative movement math updates map coordinates and square lookup returns current map data
- `CHAMPION.C:1180-1214` — champion load/wound/boots movement tick calculation feeds post-success disabled movement ticks

## Order checks

- `COMMAND.C:2045-2156` — queue gate before dequeue/replay/dispatch
- `CLIKMENU.C:180-347` — blocked path returns before successful F0267/timing path
- `MOVESENS.C:738-818` — successful move result/timing before leave/enter sensor side effects

## Firestaff evidence

- `dm1_v1_input_command_queue_pc34_compat.c` — compat queue models keyboard/mouse input rows, lock, pending replay, movement-disabled gate, and dispatch fields
- `dm1_v1_movement_command_core_pc34_compat.c` — compat command core owns F0380->F0365/F0366 parity boundaries: turns bypass movement gates, blocked steps discard input before side effects, accepted steps process leave/enter and timing
- `test_dm1_v1_input_command_queue_pc34_compat.c` — input queue probe covers keypad, PC-34 normalized arrow rows, shifted arrows, pending mouse replay, turn gate bypass, movement gate retention, and C5 regular-command queue capacity
- `test_dm1_v1_command_movement_sensor_timing_pc34_compat.c` — integration probe covers successful movement side effects, blocked movement side-effect absence, command gating, and command-core turn/step/collision/redraw seams
- `test_dm1_v1_movement_command_core_pc34_compat.c` — focused command-core probe proves PC34 keyboard movement rows survive dequeue into turn, successful movement, cooldown clearing, and blocked-movement queue discard
- `tools/verify_dm1_v1_command_movement_sensor_timing_source_lock.py` — broader source-lock gate already ties command queue, movement legality, sensor order, and timing to Firestaff files
