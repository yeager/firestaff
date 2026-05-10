# DM1 V1 movement source-lock follow-up

Status: **source-audited and gated** on N2, 2026-05-07.

Scope: input -> command queue -> turn/step dispatch -> collision/blocking -> move-result side effects -> timing.  ReDMCSB remains the primary authority for this lane.

## ReDMCSB anchors

- `COMMAND.C:106-121` — movement mouse rows route visible arrows / viewport clicks to `C001`, `C003`, `C002`, `C006`, `C005`, `C004`, `C080`, and `C083`.
- `COMMAND.C:677-685` and `IO2.C:39-61` — PC-34 movement keyboard rows and shifted-arrow normalization map to `0x004B..0x0051` before command-table lookup.
- `COMMAND.C:2045-2156` — `F0380_COMMAND_ProcessQueue_CPSC` locks the queue, checks empty/movement-disabled state before dequeue, unlocks/replays pending clicks, then dispatches turns to `F0365_COMMAND_ProcessTypes1To2_TurnParty` and steps to `F0366_COMMAND_ProcessTypes3To6_MoveParty`.
- `CLIKMENU.C:142-174` — `F0365_COMMAND_ProcessTypes1To2_TurnParty` sets stop-wait, handles stairs by `F0364_COMMAND_TakeStairs`, otherwise runs current-square leave/enter sensor boundaries around `F0284_CHAMPION_SetPartyDirection`.
- `CLIKMENU.C:180-347` — `F0366_COMMAND_ProcessTypes3To6_MoveParty` applies arrow-to-relative-step tables, handles stairs special cases, blocks walls/closed doors/closed fake walls/groups before `F0267`, discards queued input on block, and only then applies movement ticks and clears projectile movement cooldown on success.
- `DUNGEON.C:1371-1440` — `F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement` applies forward and right-step deltas using direction tables, then `F0151_DUNGEON_GetSquare` reads the current map square.
- `CHAMPION.C:93-130` — `F0284_CHAMPION_SetPartyDirection` rotates champion cells/directions and the global party direction.
- `CHAMPION.C:1180-1214` — `F0310_CHAMPION_GetMovementTicks` derives post-step disabled movement ticks from load, feet wounds, and boots of speed.
- `MOVESENS.C:315-443` — `F0267_MOVE_GetMoveResult_CPSCE` owns the party move-result contract, projectile-impact precheck, and party coordinate write.
- `MOVESENS.C:738-818` — successful movement records result/timing/scent before source leave and destination enter sensor calls.
- `MOVESENS.C:1553-1794` — `F0276_SENSOR_ProcessThingAdditionOrRemoval` walks square sensor things and triggers matching effects in source order.

## Firestaff lock points

- `dm1_v1_input_command_queue_pc34_compat.c` models keyboard/mouse command mapping, queue lock, movement-disabled retention, pending-click replay, and blocked-input discard.
- `dm1_v1_movement_command_core_pc34_compat.c` models the `F0380 -> F0365/F0366` boundary: turns bypass movement gates, blocked steps discard input before side effects, and successful steps run leave/enter sensor plus timing paths.
- `memory_movement_pc34_compat.c`, `memory_sensor_execution_pc34_compat.c`, `dm1_v1_movement_timing_pc34_compat.c`, and `memory_champion_lifecycle_pc34_compat.c` hold the narrower movement legality, sensor, and timing seams.
- `test_dm1_v1_movement_command_core_pc34_compat.c` and `test_dm1_v1_command_movement_sensor_timing_pc34_compat.c` cover PC-34 input rows, turn/step dispatch, wall/door/fakewall/group blocks, empty-party group bug, stairs seams, source/destination sensors, scent/timing, cooldown clearing, and viewport redraw flags.
- `tools/verify_dm1_v1_command_movement_sensor_timing_source_lock.py` and `tools/verify_dm1_v1_movement_command_gate_source_lock.py` statically lock the cited ReDMCSB ranges and Firestaff evidence files.

## N2 re-audit — 2026-05-10 05:28 GMT+2

Scope: DM1 V1 movement core input -> command -> movement pipeline, including collision, turning, stepping, stairs, and timing.  No implementation drift was found in the already-landed Firestaff seams; this pass refreshes the source-lock evidence and gates from N2.

### ReDMCSB evidence checked before probing

- `COMMAND.C:375-405` — `G0447_as_Graphic561_PrimaryMouseInput_Interface` is searched before `G0448_as_Graphic561_SecondaryMouseInput_Movement`; movement mouse rows map to `C001/C003/C002/C006/C005/C004/C080/C083`.
- `COMMAND.C:636-685` and `IO2.C:27-61` — PC-34 keyboard movement rows use `0x004B..0x0051`; `F0540_INPUT_Crawcin` normalizes shifted arrow scancodes into the same table values.
- `COMMAND.C:2075-2100` and `COMMAND.C:2118-2156` — `F0380_COMMAND_ProcessQueue_CPSC` locks the queue, keeps movement commands queued while `G0310_i_DisabledMovementTicks` or matching projectile lock is active, dequeues one command, replays pending clicks, dispatches turns to `F0365_COMMAND_ProcessTypes1To2_TurnParty`, and dispatches steps to `F0366_COMMAND_ProcessTypes3To6_MoveParty`.
- `CLIKMENU.C:124-139` — `F0364_COMMAND_TakeStairs` removes the party with `F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, ..., CM1_MAPX_NOT_ON_A_SQUARE, 0)`, resolves the destination with `F0154_DUNGEON_GetLocationAfterLevelChange`, switches maps with `F0173_DUNGEON_SetCurrentMap`, and sets exit facing with `F0155_DUNGEON_GetStairsExitDirection` through `F0284_CHAMPION_SetPartyDirection`.
- `CLIKMENU.C:142-174` — `F0365_COMMAND_ProcessTypes1To2_TurnParty` sets `G0321_B_StopWaitingForPlayerInput = C1_TRUE`, takes stairs instead of rotating when standing on stairs, otherwise runs party removal/addition around `F0284_CHAMPION_SetPartyDirection`.
- `CLIKMENU.C:180-347` — `F0366_COMMAND_ProcessTypes3To6_MoveParty` decrements living champion stamina before resolution (`237-255`), derives relative step deltas (`224-233`, `256-269`), handles stairs special cases (`264-276`, `325-329`), applies wall/door/fakewall/group collision and blocked-input discard (`278-323`), then applies movement cooldown and projectile-cooldown clear only after accepted non-stairs movement (`330-346`).
- `CHAMPION.C:1180-1215` — `F0310_CHAMPION_GetMovementTicks` owns post-step cadence, including load/max-load, feet wounds, Boots of Speed, and `BUG0_72`.
- `CHAMPION.C:2025-2048` — `F0325_CHAMPION_DecrementStamina` clamps stamina, applies underflow damage, and marks load/statistics dirty.
- `MOVESENS.C:752-783` and `MOVESENS.C:799-818` — `F0267_MOVE_GetMoveResult_CPSCE` handles true party-square change timing/scent and source/destination sensor boundaries.

### Firestaff lock points rechecked

- `dm1_v1_input_command_queue_pc34_compat.c` / `.h` — PC-34 input rows, queue depth, locked pending-click replay, movement-disabled retention, and blocked-input discard.
- `dm1_v1_movement_command_core_pc34_compat.c` / `.h` — `F0380 -> F0365/F0366` command boundary, turn sensor sandwich, stairs special cases, pre-resolution stamina, collision/group block handling, accepted-step timing, and viewport/input-wait flags.
- `memory_movement_pc34_compat.c`, `dm1_v1_collision_door_pc34_compat.c`, `dm1_v1_movement_timing_pc34_compat.c`, and `memory_champion_lifecycle_pc34_compat.c` — passability, door/fakewall semantics, movement ticks, scent timing, and champion load/timing formulas.

### N2 verification gates

```sh
cmake --build build --target test_dm1_v1_movement_command_core_pc34_compat test_dm1_v1_movement_pipeline_pc34_compat test_dm1_v1_input_command_queue_pc34_compat test_dm1_v1_movement_timing_pc34_compat test_dm1_v1_command_movement_sensor_timing_pc34_compat firestaff_dm1_v1_movement_core_probe -j2
ctest --test-dir build --output-on-failure -R "dm1_v1_(movement_command_core_pc34_compat|input_command_queue_pc34_compat|movement_pipeline_pc34_compat|movement_timing_pc34_compat|command_movement_sensor_timing_pc34_compat|movement_command_gate_source_lock|command_movement_sensor_timing_source_lock)|firestaff_dm1_v1_movement_core_probe"
```

Result: **8/8 passed** on N2.

## Verification run

Command run on N2:

```sh
cmake --build build --target test_dm1_v1_movement_command_core_pc34_compat test_dm1_v1_command_movement_sensor_timing_pc34_compat firestaff_dm1_v1_movement_core_probe
ctest --test-dir build --output-on-failure -R "dm1_v1_(movement_command_core_pc34_compat|command_movement_sensor_timing_pc34_compat|movement_command_gate_source_lock|command_movement_sensor_timing_source_lock)|firestaff_dm1_v1_movement_core_probe"
```

Result: **5/5 passed**.
