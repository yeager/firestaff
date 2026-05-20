# DM1 V1 movement timing/command pipeline source lock - 2026-05-20

Scope: N2-only source lock of the DM1 V1 party movement command pipeline against ReDMCSB WIP20210206. This packet covers command acceptance, turn/step timing, blocked-step behavior, and redraw cadence. It does not make an original-runtime pixel parity claim.

## Inputs

- Firestaff worktree: `/home/trv2/work/firestaff-worktrees/dm1-v1-movement-timing-source-lock-20260520`
- Base: latest `origin/main` at `77878680fdeecb9403d16473430ed62236c738c8`
- ReDMCSB primary source: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/`
- Original DM data root: `/home/trv2/.openclaw/data/firestaff-original-games/DM/`
- Canonical DM1 anchors: `/home/trv2/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/`

Canonical DM1 hashes used as provenance anchors:

- `DUNGEON.DAT` sha256 `d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85`
- `GRAPHICS.DAT` sha256 `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e`
- `TITLE` sha256 `adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745`

## ReDMCSB source rules

### Command acceptance and dispatch

- `COMMAND.C:2075-2100` locks the command queue, reads the first queued command, and leaves `C003_COMMAND_MOVE_FORWARD` through `C006_COMMAND_MOVE_LEFT` queued when `G0310_i_DisabledMovementTicks` is nonzero or when projectile-disabled ticks match the normalized absolute movement direction. The gate exits before dequeue.
- `COMMAND.C:2118-2127` dequeues one accepted command, unlocks the queue, and replays pending clicks.
- `COMMAND.C:2150-2156` dispatches accepted turns to `F0365_COMMAND_ProcessTypes1To2_TurnParty` and accepted steps to `F0366_COMMAND_ProcessTypes3To6_MoveParty`.

Firestaff coverage: `tools/verify_dm1_v1_movement_command_gate_source_lock.py`, `tests/test_dm1_v1_input_command_queue_pc34_compat.c`, `tests/test_dm1_v1_movement_command_core_pc34_compat.c`, and `tests/test_dm1_v1_command_movement_sensor_timing_pc34_compat.c` cover queue retention under movement/projectile gates, dequeue-before-dispatch, pending replay, and separate turn/step dispatch.

### Turn and step timing

- `CLIKMENU.C:156-173` makes turns stop the input wait, handles stairs if present, processes current-square sensors around `F0284_CHAMPION_SetPartyDirection`, and does not install step cooldown ticks.
- `CLIKMENU.C:237-269` makes steps stop the input wait, applies living champion stamina cost before legality resolution, maps movement commands with `G0465_ai_Graphic561_MovementArrowToStepForwardCount` and `G0466_ai_Graphic561_MovementArrowToStepRightCount`, then computes the target square.
- `CLIKMENU.C:330-346` installs `G0310_i_DisabledMovementTicks` only after a successful step, using the maximum `F0310_CHAMPION_GetMovementTicks` over living champions, and clears `G0311_i_ProjectileDisabledMovementTicks`.
- `CHAMPION.C:1180-1215` defines the movement tick formula: under max load starts at 2 ticks, heavy load adds 1, load equal to max uses overloaded cadence due `BUG0_72`, feet wounds add wound ticks, and Boots of Speed subtract 1.

Firestaff coverage: `tools/verify_dm1_v1_movement_timing_source_lock.py`, `tools/verify_dm1_v1_turn_step_timing_gate_source_lock.py`, `tests/test_dm1_v1_movement_timing_pc34_compat.c`, and `tests/test_dm1_v1_turn_step_timing_gate_pc34_compat.c` cover max-living-champion cooldown, dead champion exclusion, equal-to-max load behavior, wound/boot modifiers, turn bypass of step cooldown, projectile gate retention, and cooldown decrement separation.

### Blocked-step behavior

- `CLIKMENU.C:278-323` blocks walls, closed doors, closed real fake-walls, and groups before any successful `F0267_MOVE_GetMoveResult_CPSCE` side effects. On block it discards queued input, waits one PC-34 VBlank, sets `G0321_B_StopWaitingForPlayerInput = C0_FALSE`, and returns before sensors or cooldown assignment.
- `GROUP.C:52-67` `F0175_GROUP_GetThing` returns the first group thing on the destination square by thing type alone. The party movement block in `CLIKMENU.C:311-313` does not inspect group HP.
- `MOVESENS.C:738-783` records move-result globals, scent, and `G0362_l_LastPartyMovementTime` only for real party square changes with champions.
- `MOVESENS.C:799-818` processes source leave and destination enter sensors only after accepted movement reaches `F0267_MOVE_GetMoveResult_CPSCE`.

Firestaff coverage: `tools/verify_pass551_dm1_v1_blocked_movement_lifecycle.py`, `tests/test_dm1_v1_movement_command_core_pc34_compat.c`, and `tests/test_dm1_v1_command_movement_sensor_timing_pc34_compat.c` cover wall/door/fake-wall/group block lifecycle, queue discard, VBlank request, input wait staying armed, no sensors, no cooldown, and no viewport dirty on blocked group steps.

This pass found a source drift in `F0708_MOVEMENT_IsPartyStepBlockedByGroup_Compat`: it required at least one living creature in the destination group. ReDMCSB `F0175_GROUP_GetThing` does not do that HP filter for party movement blocking. The local fix removes the HP filter and blocks on the destination group thing itself.

### Redraw cadence

- `GAMELOOP.C:80-90` draws the dungeon view once per main loop tick when the party is not resting and the inventory is not open.
- `DUNVIEW.C:8608-8611` calls `F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW)` after building the dungeon view when the party is not in the entrance map.
- `DRAWVIEW.C:709-723` requests viewport presentation and waits one VBlank on ST-like ports; `DRAWVIEW.C:821-858` covers the PC/I34-style palette/blit path.
- `GAMELOOP.C:150-155` decrements movement and projectile cooldowns after redraw/action refresh and before the command wait loop.
- `GAMELOOP.C:164-219` drains keyboard input and calls `F0380_COMMAND_ProcessQueue_CPSC` inside the wait loop until the stop-wait and game-time gates are satisfied.

Firestaff coverage: `probes/dm1/firestaff_dm1_v1_game_loop_redraw_cadence_probe.c` and `tools/verify_pass351_dm1_v1_live_viewport_redraw_parity_sweep.py` source-lock the redraw-before-cooldown-before-command-wait order, F0128-to-F0097 presentation path, and the live route to viewport dirty after accepted turn/step effects.

## Coverage verdict

Existing movement timing probes were broad enough to catch the group-block source drift, so no new CTest was added. The focused code change restores the ReDMCSB rule, and this packet records the source citations and verification commands that prove the relevant coverage.

## Verification summary

- `cmake -S . -B build-dm1-v1-movement-timing-source-lock-20260520 -DCMAKE_BUILD_TYPE=Release` - pass
- `cmake --build build-dm1-v1-movement-timing-source-lock-20260520 --target test_dm1_v1_movement_timing_pc34_compat test_dm1_v1_turn_step_timing_gate_pc34_compat test_dm1_v1_command_movement_sensor_timing_pc34_compat test_dm1_v1_movement_command_core_pc34_compat firestaff_dm1_v1_game_loop_redraw_cadence_probe -j2` - pass
- Initial relevant CTest sweep failed before the fix on `dm1_v1_movement_command_core_pc34_compat` and `dm1_v1_command_movement_sensor_timing_pc34_compat`, both on destination group blocking.
- After the fix, targeted rebuild and CTest for those two tests passed.
- Final full relevant CTest sweep, `git diff --check`, and staged secret scan are recorded in `manifest.json`.
