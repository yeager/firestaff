# DM1 V1 movement core source audit (2026-05-06)

Scope: DM1 V1 PC-34 input -> command queue -> movement dispatch -> turning, stepping, collision/blockers, sensors, and timing.

Reference root: `<redmcsb-source>/ReDMCSB_WIP20210206/Toolchains/Common/Source/`.

## ReDMCSB anchors audited before implementation

- `COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC:2075-2099` locks the queue, detects empty/gated movement, and leaves movement commands queued while `G0310_i_DisabledMovementTicks` or matching `G0311_i_ProjectileDisabledMovementTicks` is active.
- `COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC:2118-2127` reads command coordinates, advances the circular queue, unlocks, and replays pending clicks before dispatch.
- `COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC:2150-2156` dispatches turn commands to `F0365_COMMAND_ProcessTypes1To2_TurnParty` and C003..C006 movement commands to `F0366_COMMAND_ProcessTypes3To6_MoveParty`.
- `CLIKMENU.C:F0365_COMMAND_ProcessTypes1To2_TurnParty:156-173` sets stop-wait, handles current-square stairs on turns, then fires leave sensor, rotates through `F0284_CHAMPION_SetPartyDirection`, and fires enter sensor.
- `CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty:224-233` defines command-relative step vectors: forward `(1,0)`, right `(0,1)`, backward `(-1,0)`, left `(0,-1)`.
- `CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty:264-276` handles backward-while-on-stairs and target-stairs consequences before normal blocker/cooldown flow.
- `CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty:269-323` applies relative coordinate math, rejects walls/closed doors/closed real fakewalls/groups, preserves BUG0_85 empty-party group skip, discards input on block, and returns before move-result side effects.
- `CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty:325-347` calls `F0267_MOVE_GetMoveResult_CPSCE`, computes max living champion movement ticks, stores `G0310_i_DisabledMovementTicks`, and clears projectile cooldown.
- `DUNGEON.C:F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement:1371-1391` applies forward deltas, simulates turning right, then applies right-step deltas.
- `CHAMPION.C:F0284_CHAMPION_SetPartyDirection:117-130` rotates champion cells/directions by the party-direction delta and stores `G0308_i_PartyDirection`.
- `CHAMPION.C:F0310_CHAMPION_GetMovementTicks:1180-1215` implements the original movement cadence, including BUG0_72 (`maxLoad > load`, not `>=`), wounded-feet penalty, and Boots of Speed reduction.
- `MOVESENS.C:F0267_MOVE_GetMoveResult_CPSCE:738-783` records move-result globals and updates scent/`G0362_l_LastPartyMovementTime` only on true party square changes with champions.
- `MOVESENS.C:F0267_MOVE_GetMoveResult_CPSCE:799-818` performs party source leave and destination enter sensor processing, with group deletion on party destination square.
- `GAMELOOP.C:150-155` decrements movement and projectile-disabled movement cooldowns independently once per loop.

## Firestaff seams checked

- `dm1_v1_input_command_queue_pc34_compat.c` models command-table input, queue locking, pending click replay, movement-disabled gate retention, and discard-all-input.
- `dm1_v1_movement_command_core_pc34_compat.c` composes the ReDMCSB F0380/F0365/F0366 boundary: turns bypass movement gates, stairs consequences short-circuit correctly, blocked steps discard queued input before side effects, and successful non-stairs steps run leave/enter plus timing.
- `dm1_v1_movement_timing_pc34_compat.c` mirrors successful-step cooldown/scents and per-loop cooldown decrements.
- `dm1_v1_movement_pipeline_pc34_compat.c` wires the command core, post-move environment resolver, timing state, and viewport dirty flags for the runtime pipeline.

## Verification

Run on N2 in `<firestaff-repo>` after this audit:

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
