#ifndef REDMCSB_MEMORY_DOOR_ACTION_PC34_COMPAT_H
#define REDMCSB_MEMORY_DOOR_ACTION_PC34_COMPAT_H

/*
 * Door / click-on-wall action layer for ReDMCSB PC 3.4 — Pass 31.
 *
 * Source-faithful owners for:
 *   - resolving the target door-state when the party toggles / clicks
 *     the front-cell door (door_action path in MOVESENS.C / DUNGEON.C,
 *     adjacent to F0275_SENSOR_IsTriggeredByClickOnWall);
 *   - routing a click on a front wall/cell to the correct category of
 *     actuator or sensor trigger.
 *
 * These functions are pure: they read square bytes / things tables,
 * produce a structured result, and never mutate dungeon/party state
 * themselves.  Callers (compat runtime or thin M11 shims) apply the
 * result — this is what moves door state ownership out of
 * m11_game_view.c and back into the compat layer.
 *
 * v1 coverage (this phase):
 *   - Front-door toggle: OPEN (state 0) <-> CLOSED (state 4).  DESTROYED
 *     (state 5) is surfaced with DOOR_ACTION_KIND_DESTROYED so the caller
 *     can emit the "no longer blocking" message without re-mutating the
 *     square.  Animating intermediate states (1..3) are currently
 *     snapped to 0/4 per the existing Firestaff rule; source-faithful
 *     animation batching is tracked as future work.
 *   - Click-on-wall routing: distinguishes FRONT_DOOR_TOGGLE vs
 *     FRONT_CELL_SENSOR_TRIGGER (routed through the sensor compat
 *     identification helper) vs NO_ACTION.  Does not execute the
 *     sensor effect itself; Pass 32 wires that into the runtime.
 */

#include "memory_movement_pc34_compat.h"

/* ---- Door action kinds ---- */
#define DOOR_ACTION_NONE                 0
#define DOOR_ACTION_OPEN                 1  /* door was closed, target state = 0 */
#define DOOR_ACTION_CLOSE                2  /* door was open,   target state = 4 */
#define DOOR_ACTION_DESTROYED            3  /* destroyed; no state change */

/* ---- Click-on-wall routing kinds ---- */
#define CLICK_ON_WALL_NONE               0  /* nothing to do (empty wall, out of range) */
#define CLICK_ON_WALL_FRONT_DOOR_TOGGLE  1  /* front cell is a door; caller should run the door action */
#define CLICK_ON_WALL_FRONT_CELL_SENSOR  2  /* front cell hosts a sensor thing list */

/* ---- Door toggle result ---- */
struct DoorToggleResult_Compat {
    int kind;               /* DOOR_ACTION_* */
    int mapIndex;           /* target square map index */
    int mapX;               /* target square X */
    int mapY;               /* target square Y */
    int oldDoorState;       /* previous low-nibble state (0..7), -1 if N/A */
    int newDoorState;       /* new low-nibble state (0..7), -1 if no mutation */
    int doorVertical;       /* 1 if the door is vertical orientation (bit 3) */
};

/* ---- Click-on-wall routing result ---- */
struct ClickOnWallResult_Compat {
    int kind;               /* CLICK_ON_WALL_* */
    int mapIndex;
    int mapX;
    int mapY;
    int elementType;        /* DUNGEON_ELEMENT_* of the clicked square */
    int doorState;          /* low nibble for DOOR; -1 otherwise */
    int hasSensor;          /* 1 if the clicked square has a sensor thing list */
};

/*
 * Resolve the target door action for a toggle at (mapIndex, mapX, mapY).
 *
 * Pure: reads the square byte via the dungeon tile state and fills in
 * outResult.  Never mutates dungeon or party.  Returns 1 if the target
 * square is a door (result populated), 0 otherwise.
 *
 * The caller is expected to:
 *   1. apply result.newDoorState to the square (via the existing
 *      compat square accessor) if result.kind is OPEN or CLOSE;
 *   2. skip mutation for DESTROYED (just emit the "destroyed" message);
 *   3. emit the door-animation / sound notifications it already does.
 *
 * Source mapping: door actuator branch referenced by
 * F0275_SENSOR_IsTriggeredByClickOnWall and the door-type helpers in
 * DUNGEON.C.
 */
int F0715_DOOR_ResolveToggleAction_Compat(
    const struct DungeonDatState_Compat* dungeon,
    int mapIndex,
    int mapX,
    int mapY,
    struct DoorToggleResult_Compat* outResult);

/*
 * Route a click on the front cell (relative to the party) to its
 * source-faithful action category.  Pure: reads square byte + thing-list
 * bit and fills outResult.  Does not execute a sensor or toggle a door.
 *
 * Callers that want the door toggled apply the DoorToggleResult
 * returned via F0715; callers that want the sensor run route through
 * the Pass 32 sensor runtime owner.
 *
 * Source mapping: F0275_SENSOR_IsTriggeredByClickOnWall in MOVESENS.C.
 */
int F0716_DOOR_RouteFrontCellClick_Compat(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    const struct PartyState_Compat* party,
    struct ClickOnWallResult_Compat* outResult);

#endif
