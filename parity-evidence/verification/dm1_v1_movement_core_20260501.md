# DM1 V1 movement core — source-locked broad invariant pass (2026-05-01)

Scope: input event -> command enqueue/dequeue -> movement dispatch for DM1 V1, plus movement gate behavior and directly-routed square blockers.

## Primary ReDMCSB citations

- `COMMAND.C:2045-2156` (`F0380_COMMAND_ProcessQueue_CPSC`): command queue lock, empty/gated queue behavior, pending-click replay after unlock, turn dispatch to `F0365_COMMAND_ProcessTypes1To2_TurnParty`, movement dispatch to `F0366_COMMAND_ProcessTypes3To6_MoveParty`.
- `COMMAND.C:2095-2100` / `COMMAND.C:2104-2110`: movement command remains queued while `G0310_i_DisabledMovementTicks` or matching `G0311_i_ProjectileDisabledMovementTicks` gate is active.
- `CLIKMENU.C:180-347` (`F0366_COMMAND_ProcessTypes3To6_MoveParty`): movement arrow index, relative target calculation, stairs/pit/teleporter consequence routing, wall/door/fake-wall blockers, group collision, discard-on-block, cooldown assignment after successful movement.
- `CLIKMENU.C:224-233`: forward/right/back/left relative step tables.
- `CLIKMENU.C:278-288`: wall blocks; door states other than open/one-fourth/destroyed block; closed real fake-walls block; pits/teleporters fall through as passable square types.
- `DUNGEON.C:1371-1391` (`F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement`): direction-relative forward/right coordinate math.
- `GAMELOOP.C:150-155`: movement/projectile movement locks decrement once per tick.
- `CHAMPION.C:93-130`: party turn direction update baseline.
- `CHAMPION.C:1180-1214`: source movement tick computation cited by existing movement source-lock gate.

## Firestaff changes

- Added `test_dm1_v1_movement_core_pc34_compat.c` and CTest target `dm1_v1_movement_core_pc34_compat`.
- Extended `tools/verify_dm1_v1_movement_source_lock.py` so the source-lock JSON now verifies the broad queue-to-move invariant probe labels/citations.
- Wrote JSON evidence: `parity-evidence/verification/dm1_v1_movement_core_20260501.json`.

## Covered invariants

- Keyboard forward queues, dequeues, dispatches as movement, and reaches the expected target coordinate.
- `disabledMovementTicks` blocks cardinal movement without dequeuing the command.
- Projectile movement gate blocks only matching absolute movement direction and allows nonmatching direction.
- Turns bypass movement/projectile gates and dispatch as turns.
- Relative step deltas cover forward/right/back/left against multiple party facings.
- Wall target blocks as `MOVE_BLOCKED_WALL`.
- Closed door state blocks as `MOVE_BLOCKED_DOOR`; one-fourth and destroyed door states are passable.
- Closed real fake-wall blocks; open and imaginary fake-wall bits are passable.
- Pit and teleporter target squares are passable by the movement dispatch seam.

## Verification run

```text
ctest --test-dir build -R "dm1_v1_(movement_core_pc34_compat|input_command_queue_pc34_compat|movement_source_lock|input_command_queue_source_lock|projectile_movement_interlock_source_lock)" --output-on-failure
5/5 tests passed
```

## Remaining big blockers vs finetune

- This pass is a pure deterministic compat seam; it does not execute full runtime side effects after `F0366` (sound, UI highlighting, sensor/group reaction scheduling, stamina decrement, actual cooldown mutation). Those remain owned by the tick orchestrator/source-lock gates.
- Pit/teleporter consequence chains are verified here as passable dispatch targets, not full fall/teleport resolution chains.
- Stairs have existing source-lock coverage, but this broad core probe intentionally keeps stairs out to avoid mixing level transition consequences into command dequeue/blocked-move invariants.
