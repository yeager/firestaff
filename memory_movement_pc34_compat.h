#ifndef REDMCSB_MEMORY_MOVEMENT_PC34_COMPAT_H
#define REDMCSB_MEMORY_MOVEMENT_PC34_COMPAT_H

/*
 * Movement data layer for ReDMCSB PC 3.4.
 *
 * Pure functions for party/thing movement — NO side effects, NO UI,
 * NO render, NO sound, NO sensor execution, NO combat.
 *
 * Source: MOVESENS.C, DEFS.H from the original DM/CSB source.
 *
 * v2-design: Movement is a pure function of (position, input) → new position.
 * Sensor identification is separate from execution.
 */

#include "memory_champion_state_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

/* ---- Movement input actions ---- */
#define MOVE_FORWARD    0
#define MOVE_RIGHT      1
#define MOVE_BACKWARD   2
#define MOVE_LEFT       3
#define MOVE_TURN_RIGHT 4
#define MOVE_TURN_LEFT  5
#define MOVE_ACTION_COUNT 6

/* ---- Movement result codes ---- */
#define MOVE_OK             0
#define MOVE_BLOCKED_WALL   1
#define MOVE_BLOCKED_DOOR   2
#define MOVE_BLOCKED_BOUNDS 3
#define MOVE_TURN_ONLY      4  /* Turn succeeded, no position change */

/* ---- Movement result struct ---- */
struct MovementResult_Compat {
    int resultCode;       /* MOVE_OK, MOVE_BLOCKED_*, MOVE_TURN_ONLY */
    int newMapX;          /* New X after move (or unchanged if blocked) */
    int newMapY;          /* New Y after move */
    int newDirection;     /* New direction after move/turn */
    int newMapIndex;      /* New map index (same unless teleporter — future) */
};

/*
 * Compute next direction after a turn.
 * Pure function, no side effects.
 *
 * turnRight: DIR_NORTH→DIR_EAST→DIR_SOUTH→DIR_WEST→DIR_NORTH
 * turnLeft:  DIR_NORTH→DIR_WEST→DIR_SOUTH→DIR_EAST→DIR_NORTH
 */
int F0700_MOVEMENT_TurnDirection_Compat(int currentDir, int turnRight);

/*
 * Compute the map offset (dx, dy) for a movement action given current direction.
 *
 * Forward/North:  dx=0, dy=-1
 * Forward/East:   dx=+1, dy=0
 * Forward/South:  dx=0, dy=+1
 * Forward/West:   dx=-1, dy=0
 *
 * Left/Right/Backward are relative to facing direction.
 */
void F0701_MOVEMENT_GetStepDelta_Compat(
    int direction,
    int moveAction,
    int* outDx,
    int* outDy);

/*
 * Attempt a movement action for the party.
 *
 * Pure function: reads dungeon tile data to check walkability,
 * writes result to outResult.  Does NOT modify party state.
 *
 * For turns (MOVE_TURN_LEFT, MOVE_TURN_RIGHT): always succeeds,
 * sets resultCode = MOVE_TURN_ONLY.
 *
 * For steps (MOVE_FORWARD/BACKWARD/LEFT/RIGHT): checks bounds,
 * checks tile type (wall = blocked, everything else = ok for now).
 * Does NOT check door open/closed state (future phase).
 *
 * Returns 1 if result is MOVE_OK or MOVE_TURN_ONLY, 0 if blocked.
 */
int F0702_MOVEMENT_TryMove_Compat(
    const struct DungeonDatState_Compat* dungeon,
    const struct PartyState_Compat* party,
    int moveAction,
    struct MovementResult_Compat* outResult);

/* ---- Sensor identification on square ---- */

/*
 * Sensor identification result — which sensors exist on a square.
 * Does NOT execute the sensor, just identifies it.
 */
struct SensorOnSquare_Compat {
    int found;                   /* 1 if at least one sensor found */
    int sensorIndex;             /* Index into things.sensors[] of FIRST sensor */
    int sensorType;              /* Type code of first sensor */
    unsigned short sensorData;   /* Data of first sensor */
    /* Remote target (if applicable) */
    int targetMapX;
    int targetMapY;
    int targetCell;
    int isLocal;                 /* 1 if localEffect, 0 if remote */
    int totalSensorsOnSquare;    /* How many sensors chained on this square */
};

/*
 * Identify sensors on a given square.
 *
 * Walks the thing linked-list for the given square and finds
 * all SENSOR things.  Reports the FIRST sensor found.
 *
 * Does NOT execute any sensor logic.
 *
 * Returns 1 if at least one sensor found, 0 otherwise.
 */
int F0703_MOVEMENT_IdentifySensorsOnSquare_Compat(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    int mapIndex,
    int mapX,
    int mapY,
    struct SensorOnSquare_Compat* outSensor);

#endif
