# Pass372 — DM1 V1 movement runtime route

Status: `PASS372_DM1_V1_MOVEMENT_RUNTIME_ROUTE_SOURCE_LOCKED`

## Decision

Firestaff's DM1 V1 runtime movement route is source-locked from input resolution through command queue and movement dispatch. The residual keypad/keyboard-buffer scope remains only the original DOSBox/FIRES pre-`F0361` keyboard-buffer adapter, not the Firestaff live route.

## ReDMCSB source audit anchors

- `IO2.C:27-61` — I34E F0540 reads IODRV_00_GetKeyboardInput and normalizes shifted extended arrows to K/L/M/P before returning to the main loop. ok=`True`
- `GAMELOOP.C:164-168,215` — The game loop drains buffered keyboard characters through F0361, then processes the command queue through F0380. ok=`True`
- `COMMAND.C:636-685` — I34E movement keyboard table binds K/L/M/O/P/Q to C001..C006. ok=`True`
- `COMMAND.C:1709-1813` — F0361 scans primary then secondary keyboard tables and enqueues the matched command. ok=`True`
- `COMMAND.C:2045-2156` — F0380 locks/dequeues the queue, gates movement ticks, and dispatches turns to F0365 and steps to F0366. ok=`True`
- `CLIKMENU.C:142-174` — F0365 turns the party and fires leave/enter sensors around F0284_CHAMPION_SetPartyDirection. ok=`True`
- `CLIKMENU.C:180-330` — F0366 maps C003..C006 to relative step deltas, validates collision, moves, and applies movement timing. ok=`True`
- `STARTUP2.C:1179-1183` — After dungeon load, secondary keyboard input is installed to the movement table. ok=`True`

## Firestaff route anchors

- `main_loop_m11.c` — SDL NumLock-on keypad symbols key:kp1..key:kp6 are accepted before WASD convenience aliases. ok=`True`
- `m11_game_view.c` — The live game view converts M12 movement inputs to C001..C006 and queues resolved command ids directly, so product movement is not blocked by original-DOS keyboard-buffer delivery. ok=`True`
- `dm1_v1_input_command_queue_pc34_compat.c` — The compat queue still covers original keyboard rows when an original-shaped keycode is supplied. ok=`True`
- `dm1_v1_movement_pipeline_pc34_compat.c` — The compat pipeline wires enqueue/dequeue/gate/turn/move/post-move processing under ReDMCSB citations. ok=`True`
- `test_dm1_v1_input_command_queue_pc34_compat.c` — Regression covers PC34 K/L/M/O/P/Q table rows, IO2 shifted arrows, pending replay, and four-command capacity. ok=`True`

## Prior runtime evidence reused

- `pass348_dm1_v1_numlock_keypad_blocker_closure` status=`RECLASSIFIED_PASS348_NUMLOCK_KEYPAD_BLOCKER_NARROWED_NOT_CLOSED`
- `pass349_dm1_v1_full_launcher_keypad_runtime_route` status=`FULL_LAUNCHER_KEYPAD_RUNTIME_ROUTE_PROVED`
- `pass352_dm1_v1_movement_route_regression_matrix` status=`PASS_DM1_V1_MOVEMENT_ROUTE_REGRESSION_MATRIX_CONSOLIDATED`
- `pass359_dm1_v1_movement_route_runtime_blocker_followup` status=`PASS_DM1_V1_MOVEMENT_ROUTE_RUNTIME_BLOCKER_CLASSIFIED`

## Residual scope

- Firestaff runtime route: closed: SDL/script keypad -> M12 input -> resolved C001..C006 command -> F0380/F0365/F0366 compat pipeline
- Original DOS residual: still unclaimed: DOSBox/FIRES keyboard-buffer adapter causing M527/M528 or IODRV_00_GetKeyboardInput to return 0x004B/0x004C/0x004D/0x004F/0x0050/0x0051 in a bounded original run
- Route tokens audited: `/down`, `/left`, `/right`, and combined quality token `/down/left/right` remain covered by M12 movement input -> C001..C006 queue dispatch.
- `CMakeLists.txt:466` / `CMakeLists.txt:467` audited as unrelated DM1 V2 movement-command-adapter test wiring; no DM1 V1 route change needed there.
- Decision: do not spend Firestaff M11 route work on original-DOS keyboard-buffer residual; treat it as an original-runtime transcript/debugger task

## Gates

- `ctest --test-dir build -R 'dm1_v1_input_command_queue_pc34_compat|dm1_v1_movement_command_core_pc34_compat|dm1_v1_movement_pipeline_pc34_compat|dm1_v1_input_command_queue_source_lock' --output-on-failure` ok=`True`
- Manifest: `parity-evidence/verification/pass372_dm1_v1_movement_runtime_route/manifest.json`
