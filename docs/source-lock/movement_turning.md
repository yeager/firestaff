# JOB 2: Source-lock DM1 V1 — Turning (90-Degree Turns)

## What Triggers a Turn

**Trigger:** Arrow-key or UI click producing `C001_COMMAND_TURN_LEFT` or `C002_COMMAND_TURN_RIGHT`.

- `C001` = Turn Left   `C002` = Turn Right
- Defined in `DEFS.H:238-239`

## Command Dispatch

**Ref: COMMAND.C:2104** — `F0380_COMMAND_ProcessQueue_CPSC`:
```c
if (L1160 == C002 || L1160 == C001)
    F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160);
```

## Turn Logic

**Ref: CLIKMENU.C:142-179** — `F0365_COMMAND_ProcessTypes1To2_TurnParty`:
```c
G0321_B_StopWaitingForPlayerInput = C1_TRUE;

// Check: if currently standing on stairs, taking stairs ends the function
if (M034_SQUARE_TYPE(L1114 = F0151_DUNGEON_GetSquare(G0306, G0307)) == C03_ELEMENT_STAIRS) {
    F0364_COMMAND_TakeStairs(M007_GET(L1114, MASK0x0004_STAIRS_UP));
    return;
}

// Fire departure sensors on current square (party about to turn in place)
F0276_SENSOR_ProcessThingAdditionOrRemoval(G0306, G0307, C0xFFFF_THING_PARTY, C1_TRUE, C0_FALSE);

// Update facing direction
F0284_CHAMPION_SetPartyDirection(
    M021_NORMALIZE(G0308_i_PartyDirection + ((P0734 == C002_COMMAND_TURN_RIGHT) ? 1 : 3))
);

// Fire arrival sensors on same square (party turned in place)
F0276_SENSOR_ProcessThingAdditionOrRemoval(G0306, G0307, C0xFFFF_THING_PARTY, C1_TRUE, C1_TRUE);
```

**Key detail:** `+1` for right turn (clockwise), `+3` (equivalent to `-1`) for left turn (counter-clockwise), via `M021_NORMALIZE` (mod 4).

## No Wall Collision Check on Turn

Turn commands never reach the wall-collision path in `F0366`. They are dispatched entirely in `F0365` and do not call `F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement` — party X/Y does not change.

## Viewport Rotation

`G0308_i_PartyDirection` is the authoritative facing. The dungeon view renderer reads this to rotate the 3D viewport. No separate viewport-rotation flag exists — direction *is* the viewport orientation.

## Stairs While Turning

`CLIKMENU.C:163-165`: If the party is standing on stairs and attempts a turn, `F0364_COMMAND_TakeStairs` is invoked immediately, which initiates a level transition instead of a simple direction change.

## No Movement Cooldown on Turn

Turn commands do NOT set `G0310_i_DisabledMovementTicks`. `G0321_B_StopWaitingForPlayerInput` is set `C1_TRUE`, allowing the game loop to proceed without waiting for input, but no step-cost timer is charged.

## Firestaff Implementation

**Source:** `src/dm1/dm1_v1_movement_pc34_compat.c`
- `dm1v1_movement_execute_step()` dispatches turn commands separately from step commands
- Direction delta: `normalize_dir(facing + (cmd == RIGHT ? 1 : 3))`

**Source citations** present: `CLIKMENU.C:142-179`, `COMMAND.C:2104`, `F0284_CHAMPION_SetPartyDirection`, `F0276_SENSOR_ProcessThingAdditionOrRemoval`, `F0364_COMMAND_TakeStairs`.

## Status

**PASS** — Turn is a pure direction change (no movement, no coordinate update). Fires departure+arrival sensors on same square. Stairs-on-turn triggers level change. No wall check, no movement cooldown. ReDMCSB fully traced.
