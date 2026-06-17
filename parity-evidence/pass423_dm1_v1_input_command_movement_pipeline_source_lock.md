# Pass423 DM1 V1 input → command → movement pipeline source lock

Status: **PASS423_DM1_V1_INPUT_COMMAND_MOVEMENT_PIPELINE_SOURCE_LOCKED**

Scope: PC-34 raw input -> command enqueue -> F0380 gate/dequeue -> F0365/F0366 turn/move dispatch.

## ReDMCSB citations

- `IO2.C:27-61` — PC-34 reads IODRV keyboard input and normalizes shifted extended arrows to command-table codes K/L/M/P.
- `COMMAND.C:636-685` — PC-34 movement keyboard table maps normalized codes to C001/C002 turn and C003..C006 move commands.
- `COMMAND.C:106-121` — Movement-panel mouse rows map arrow boxes and viewport/right-click routes to movement/view/inventory commands.
- `COMMAND.C:1709-1813` — F0361 locks the queue, searches primary then secondary keyboard tables, writes a matched command, unlocks, then replays pending click.
- `COMMAND.C:1452-1662` — F0359 either records one pending click while locked or resolves primary/secondary mouse rows into a queued command.
- `COMMAND.C:2045-2156` — F0380 locks, tests empty/movement-disabled state before dequeue, then unlocks/replays and dispatches turn/move commands.
- `CLIKMENU.C:142-174` — F0365 sets the input wait stop flag and changes party direction through the source sensor boundary.
- `CLIKMENU.C:180-347` — F0366 computes relative destination, blocks walls/doors/fakewalls/groups before side effects, discards input on block, and calls F0267 on success.
- `GAMELOOP.C:150-219` — Main loop ages movement cooldowns before polling keyboard and processing exactly the queued command path.

## Order checks

- `COMMAND.C:2045-2156` — F0380 movement gate happens before X/Y read and queue index advance, so a gated step remains queued.
- `CLIKMENU.C:180-347` — F0366 blocked route returns before successful F0267/timing route.

## Firestaff evidence

- `src/dm1/dm1_v1_input_command_queue_pc34_compat.c` — compat queue models PC-34 key normalization rows, mouse rows, lock/pending replay, movement gate retention, dequeue, and turn/move dispatch flags
- `src/dm1/dm1_v1_movement_command_core_pc34_compat.c` — command core consumes queue results into turn/move handling, blocks before successful movement side effects, and requests input discard/redraw at the F0366 boundary
- `tests/test_dm1_v1_input_command_queue_pc34_compat.c` — regression covers PC-34 K/L/M/O/P/Q rows, IO2 shifted arrows, pending replay, movement-gate retention, and reserved queue slots
- `tests/test_dm1_v1_movement_command_core_pc34_compat.c` — focused command-core regression proves PC-34 queue output reaches turn, successful movement, cooldown clearing, and blocked-step queue discard

## Gates

- `cmake --build /Users/bosse/.openclaw/workspace-main/build --target test_dm1_v1_input_command_queue_pc34_compat test_dm1_v1_movement_command_core_pc34_compat test_dm1_v1_command_movement_sensor_timing_pc34_compat -j2` — rc 0
- `ctest --test-dir /Users/bosse/.openclaw/workspace-main/build --output-on-failure -R dm1_v1_input_command_queue_pc34_compat|dm1_v1_movement_command_core_pc34_compat|dm1_v1_command_movement_sensor_timing_pc34_compat` — rc 0
- `git diff --check` — rc 0

## Not claimed

- new original DOSBox runtime hit
- pixel parity
- full gameplay parity beyond this input-command-movement seam
