# JOB 1: Source-lock DM1 V1 — Forward Step

## What Triggers a Step

**Trigger:** Arrow-key (or UI click) producing command code `C003_COMMAND_MOVE_FORWARD` through `C006_COMMAND_MOVE_LEFT`.

- `C003` = Forward  `C004` = Right  `C005` = Backward  `C006` = Left
- Defined in `DEFS.H:240-243`

**Command dispatch chain (ReDMCSB WIP20210206):**

1. `COMMAND.C:2045-2200` — `F0380_COMMAND_ProcessQueue_CPSC`
   - Locks queue (`G0435 = C1_TRUE`), dequeues one command.
   - Gate: if `G0310_i_DisabledMovementTicks > 0` or `G0311_i_ProjectileDisabledMovementTicks > 0`, command stays queued; re-plays next tick.
   - Dispatches `C001/C002` → `F0365`; `C003-C006` → `F0366`.

2. `CLIKMENU.C:256` — `F0366_COMMAND_ProcessTypes3To6_MoveParty`
   - `AL1118 = P0735 - C003` → 0=Forward, 1=Right, 2=Backward, 3=Left.

## Direction Delta Tables

**Ref: CLIKMENU.C:236-250 (static initializers for G14ED+, else `COMPILE.H`):**
```
Forward:  forward_count=+1, right_count= 0  → dy=-1, dx= 0
Right:    forward_count= 0, right_count=+1  → dy= 0, dx=+1
Backward: forward_count=-1, right_count= 0  → dy=+1, dx= 0
Left:     forward_count= 0, right_count=-1  → dy= 0, dx=-1
```

**Ref: DUNGEON.C:1389-1391** — `F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement`:
```
Direction 0 (North): dy = -1,  dx =  0
Direction 1 (East):  dy =  0,  dx = +1
Direction 2 (South): dy = +1,  dx =  0
Direction 3 (West):  dy =  0,  dx = -1
```

## Coordinate Resolution

`CLIKMENU.C:269`:
```c
F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(
    G0308_i_PartyDirection,
    G0465_ai_Graphic561_MovementArrowToStepForwardCount[AL1118],
    G0466_ai_Graphic561_MovementArrowToStepRightCount[AL1118],
    &L1121_i_MapX, &L1122_i_MapY);
```

## Sensor Processing

`F0267_MOVE_GetMoveResult_CPSCE` (MOVESENS.C:316+) processes:
- Source square: `F0276_SENSOR_ProcessThingAdditionOrRemoval(..., C1_TRUE, C0_FALSE)` — departure sensors fire
- Destination square: `F0276_SENSOR_ProcessThingAdditionOrRemoval(..., C1_TRUE, C1_TRUE)` — arrival sensors fire
- Pit/teleporter chains resolve within the 100-iteration loop (line ~438)

## Firestaff Implementation

**Source files:**
- `src/dm1/dm1_v1_movement_pc34_compat.c` — step execution, command queue, forward-count deltas
- `src/dm1/dm1_v1_movement_pipeline_pc34_compat.c` — sensor traversal, thing-list mutation
- `src/dm1/dm1_v1_movement_command_core_pc34_compat.c` — stamina cost, wall-damage request, stairs dispatch
- `src/dm1/dm1_v1_movement_timing_pc34_compat.c` — cooldown decrement, tick computation

**Source locks present in all files** — each function cites exact `ReDMCSB_WIP20210206` file and line range.

## Data Structures

- **Party state:** `G0306/G0307` (mapX/Y), `G0308` (direction 0-3), `G0309` (mapIndex)
- **Queue:** `G0432[5]` command slots, `G0433` (first), `G0434` (last), `G0435` (locked)
- **Cooldown:** `G0310` ticks remaining for movement; `G0311` separate projectile-lockout ticks
- **Movement arrows:** `G0465/G0466[4]` forward/right step-count deltas per command index

## Status

**PASS** — ReDMCSB source fully traced: command code → queue → dispatch → F0366 → F0150 coordinate delta → F0267 sensor/movement result. Firestaff dm1_v1_movement_pc34_compat.c implements the complete chain with source citations.
