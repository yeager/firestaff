# DM1 V1 input-to-movement source audit (2026-05-06)

Reference root: `<redmcsb-source>/ReDMCSB_WIP20210206/Toolchains/Common/Source/`.

Scope: DM1 V1 PC-34 movement input -> command queue -> command dispatch -> turning/stepping -> collision/blocking -> movement side effects -> timing cooldown.

## Audited ReDMCSB pipeline

- `COMMAND.C:F0361_COMMAND_ProcessKeyPress:1709-1813` converts primary/secondary keyboard rows into queued commands. Key details: it locks `G0435_B_CommandQueueLocked` at line 1737, reserves a queue slot at 1746-1748, writes `G0432_as_CommandQueue[...].Command` at 1761-1766 / 1788-1793, then unlocks and replays a pending click at 1810-1812.
- `COMMAND.C:F0359_COMMAND_ProcessClick_CPSC:1452-1661` is the mouse-side peer: when the queue is locked it records one pending click (`G0436..G0439`), otherwise it enqueues command/x/y. `COMMAND.C:F0360_COMMAND_ProcessPendingClick:1692-1707` replays that deferred click after unlock.
- `COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC:2075-2099` locks queue processing and leaves movement commands queued while `G0310_i_DisabledMovementTicks` is nonzero or the projectile cooldown matches the command-relative direction.
- `COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC:2118-2127` reads command coordinates, advances the circular queue, unlocks, and replays a pending click before dispatch.
- `COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC:2150-2156` dispatches turn commands (`C001`, `C002`) to `F0365_COMMAND_ProcessTypes1To2_TurnParty` and movement commands (`C003..C006`) to `F0366_COMMAND_ProcessTypes3To6_MoveParty`.
- `CLIKMENU.C:F0365_COMMAND_ProcessTypes1To2_TurnParty:156-173` sets `G0321_B_StopWaitingForPlayerInput`, consumes turns on stairs through `F0364_COMMAND_TakeStairs`, otherwise fires current-square leave sensor, calls `F0284_CHAMPION_SetPartyDirection`, then fires current-square enter sensor.
- `CHAMPION.C:F0284_CHAMPION_SetPartyDirection:117-130` rotates each champion cell/direction by the party-direction delta and stores `G0308_i_PartyDirection`.
- `CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty:224-233` defines command-relative movement vectors: forward `(1,0)`, right `(0,1)`, backward `(-1,0)`, left `(0,-1)`.
- `DUNGEON.C:F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement:1389-1391` applies forward deltas, simulates a right turn, then applies right-step deltas using `G0233_ai_Graphic559_DirectionToStepEastCount` and `G0234_ai_Graphic559_DirectionToStepNorthCount`.
- `CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty:264-276` handles stairs consequences before normal blocking: backward while already on stairs takes stairs immediately; target stairs removes the party, places it on the stairs, then calls `F0364_COMMAND_TakeStairs` and returns.
- `CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty:278-323` is the party-step blocker gate: walls block; doors block unless state is open, one-fourth closed, or destroyed; fakewalls block unless open or imaginary; groups block when party has champions. Blocked steps call `F0357_COMMAND_DiscardAllInput`, wait one VBlank on PC-family targets, clear stop-wait, and return before `F0267_MOVE_GetMoveResult_CPSCE`.
- `CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty:325-347` commits successful non-stairs movement through `F0267_MOVE_GetMoveResult_CPSCE`, computes the maximum living champion movement ticks, writes `G0310_i_DisabledMovementTicks`, and clears `G0311_i_ProjectileDisabledMovementTicks`.
- `CHAMPION.C:F0310_CHAMPION_GetMovementTicks:1180-1215` is the movement cadence source: base ticks depend on load vs max load, `BUG0_72` keeps exact max-load in the slow path, wounded feet add ticks, and Boots of Speed subtract one tick.
- `MOVESENS.C:F0267_MOVE_GetMoveResult_CPSCE:738-783` records move-result globals and updates scent/`G0362_l_LastPartyMovementTime` only for real party square changes while champions exist.
- `MOVESENS.C:F0267_MOVE_GetMoveResult_CPSCE:799-818` applies party source leave and destination enter sensors after successful movement; destination group deletion happens before destination enter sensor processing at 811-818.
- `GAMELOOP.C:F0002_MAIN_GameLoop_CPSDF:124-155` increments `G0313_ul_GameTime` and decrements `G0310_i_DisabledMovementTicks` / `G0311_i_ProjectileDisabledMovementTicks` once per game-loop tick.

## Firestaff parity seams checked

- `dm1_v1_input_command_queue_pc34_compat.c` mirrors keyboard/mouse command enqueue, queue locking, one pending click replay, movement-disabled retention, and blocked-step input discard.
- `dm1_v1_movement_command_core_pc34_compat.c` mirrors the F0380/F0365/F0366 boundary: turns bypass movement cooldown, stairs short-circuit before regular blockers, blocked movement discards input before side effects, successful steps run leave/enter sensors and timing.
- `dm1_v1_movement_timing_pc34_compat.c` mirrors successful-step scent/last-movement timing and per-loop cooldown decrements.
- `dm1_v1_movement_pipeline_pc34_compat.c` wires input command core, movement results, sensor side effects, timing, and viewport-dirty state for the runtime seam.

## Verification commands

Run from `<firestaff-repo>`:

```sh
python3 tools/verify_dm1_v1_movement_source_lock.py
python3 tools/verify_dm1_v1_movement_command_gate_source_lock.py
python3 tools/verify_dm1_v1_movement_timing_source_lock.py
python3 tools/verify_dm1_v1_party_movement_sensor_order_source_lock.py
cmake --build build --target test_dm1_v1_input_command_queue_pc34_compat test_dm1_v1_movement_command_core_pc34_compat test_dm1_v1_movement_pipeline_pc34_compat test_dm1_v1_movement_timing_pc34_compat test_dm1_v1_command_movement_sensor_timing_pc34_compat -j2
./build/test_dm1_v1_input_command_queue_pc34_compat
./build/test_dm1_v1_movement_command_core_pc34_compat
./build/test_dm1_v1_movement_pipeline_pc34_compat
./build/test_dm1_v1_movement_timing_pc34_compat
./build/test_dm1_v1_command_movement_sensor_timing_pc34_compat
git diff --check
```
