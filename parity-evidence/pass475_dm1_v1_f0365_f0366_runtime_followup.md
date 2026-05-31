# Pass475 — DM1 V1 input → F0365/F0366 runtime follow-up

Status: **PASS475_FIRESTAFF_DM1_V1_INPUT_TO_F0365_F0366_ROUTE_CLOSED**

Closed blocker: Firestaff live M11 movement input now has a source-locked and regression-tested route through command ids into the F0380 -> F0365/F0366 compat boundary; no direct-move bypass remains in this seam.

## ReDMCSB source audit

- `GAMELOOP.C:150-219` — PC34 game loop drains keyboard input into F0361, then processes the command queue with F0380 until the input/tick gate is satisfied.
- `COMMAND.C:636-685` — I34E/I34M secondary keyboard rows map PC34 movement key codes to C001/C002 turns and C003..C006 movement commands.
- `COMMAND.C:1709-1813` — F0361 resolves primary/secondary keyboard input, writes the matched command to G0432, unlocks, and replays a pending click.
- `COMMAND.C:2045-2156` — F0380 locks the queue, keeps movement queued while cooldown/projectile gates apply, dequeues exactly one command, then dispatches turns to F0365 and steps to F0366.
- `CLIKMENU.C:142-174` — F0365 is the source turn boundary: stop-wait flag, stair special case, sensor removal/addition, and direction update.
- `CLIKMENU.C:180-347` — F0366 is the source step/strafe boundary: stamina cost, relative destination, wall/door/fakewall/group blocking, blocked-input discard, F0267 commit, and cooldown side effects.

## Ordering locks

- `COMMAND.C:2045-2156` — The movement-disabled gate is tested before X/Y read and queue index advance; dispatch to F0365/F0366 occurs only after unlock/pending-click replay.
- `CLIKMENU.C:180-347` — F0366 discards input and returns on blocked movement before F0267/cooldown side effects; accepted movement reaches F0267 and then arms cooldown.

## Firestaff runtime locks

- `src/engine/m11_game_view.c` — product runtime maps live M12 movement inputs to source command ids and feeds the DM1 V1 pipeline instead of moving directly
- `src/dm1/dm1_v1_movement_pipeline_pc34_compat.c` — pipeline API accepts either input events or command ids and processes one F0380/F0365/F0366-compatible tick
- `src/dm1/dm1_v1_movement_command_core_pc34_compat.c` — command core consumes the queued command result into turn or step handling, including blocked-input discard and accepted-step side effects
- `src/engine/main_loop_m11.c` — runtime probe exports dequeued command, turn/step flags, movement/turn occurrence, and viewportDirty for live verification

## Gates

- `cmake --build /Users/bosse/.openclaw/workspace-main/build --target test_dm1_v1_input_command_queue_pc34_compat test_dm1_v1_movement_command_core_pc34_compat test_dm1_v1_movement_pipeline_pc34_compat test_dm1_v1_command_movement_sensor_timing_pc34_compat -j2` — rc 0
- `ctest --test-dir /Users/bosse/.openclaw/workspace-main/build --output-on-failure -R dm1_v1_input_command_queue_pc34_compat|dm1_v1_movement_command_core_pc34_compat|dm1_v1_movement_pipeline_pc34_compat|dm1_v1_command_movement_sensor_timing_pc34_compat` — rc 0
- `git diff --check` — rc 0

## Still not claimed

- Original stock FIRES keyboard-buffer/debugger hit and semantically party-control-ready original overlay/pixel parity are still unclaimed; they require a separate DOSBox/FIRES capture or symbol-address run, not more Firestaff route plumbing.
