#include "memory_door_action_pc34_compat.h"

#include <string.h>

/*
 * Helper: fetch a square byte for (mapIndex, mapX, mapY) without mutating
 * state.  Returns 1 on success, 0 on out-of-range / unloaded tiles.
 */
static int door_action_read_square(
    const struct DungeonDatState_Compat* dungeon,
    int mapIndex,
    int mapX,
    int mapY,
    unsigned char* outByte)
{
    const struct DungeonMapDesc_Compat* map;
    if (!dungeon || !dungeon->tilesLoaded || !dungeon->tiles) return 0;
    if (mapIndex < 0 || mapIndex >= (int)dungeon->header.mapCount) return 0;
    map = &dungeon->maps[mapIndex];
    if (mapX < 0 || mapX >= map->width || mapY < 0 || mapY >= map->height) return 0;
    if (!dungeon->tiles[mapIndex].squareData) return 0;
    if (outByte) {
        *outByte = dungeon->tiles[mapIndex].squareData[mapX * map->height + mapY];
    }
    return 1;
}

int F0715_DOOR_ResolveToggleAction_Compat(
    const struct DungeonDatState_Compat* dungeon,
    int mapIndex,
    int mapX,
    int mapY,
    struct DoorToggleResult_Compat* outResult)
{
    unsigned char squareByte = 0;
    int elementType;
    int doorState;

    if (!outResult) return 0;
    memset(outResult, 0, sizeof(*outResult));
    outResult->mapIndex = mapIndex;
    outResult->mapX = mapX;
    outResult->mapY = mapY;
    outResult->oldDoorState = -1;
    outResult->newDoorState = -1;

    if (!door_action_read_square(dungeon, mapIndex, mapX, mapY, &squareByte)) {
        return 0;
    }

    elementType = (squareByte & DUNGEON_SQUARE_MASK_TYPE) >> 5;
    if (elementType != DUNGEON_ELEMENT_DOOR) {
        return 0;
    }

    doorState = squareByte & 0x07;
    outResult->oldDoorState = doorState;
    outResult->doorVertical = (squareByte & 0x08) != 0;

    /* State 5 = destroyed door.  No mutation; surface for caller to
     * emit the "destroyed" status line. */
    if (doorState == 5) {
        outResult->kind = DOOR_ACTION_DESTROYED;
        outResult->newDoorState = -1;
        return 1;
    }

    /* State 0 = fully open.  Toggling closes it (target state = 4).
     * Any other state (closed or animating) toggles to fully open.
     *
     * Source-faithful animation (opening/closing with intermediate
     * states 1..3) is tracked as future work; the existing Firestaff
     * convention snaps to 0/4 and this preserves it.
     */
    if (doorState == 0) {
        outResult->kind = DOOR_ACTION_CLOSE;
        outResult->newDoorState = 4;
    } else {
        outResult->kind = DOOR_ACTION_OPEN;
        outResult->newDoorState = 0;
    }
    return 1;
}

int F0716_DOOR_RouteFrontCellClick_Compat(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    const struct PartyState_Compat* party,
    struct ClickOnWallResult_Compat* outResult)
{
    static const int s_dx[4] = {  0,  1,  0, -1 };
    static const int s_dy[4] = { -1,  0,  1,  0 };
    unsigned char squareByte = 0;
    int fx, fy;
    int elementType;

    if (!outResult) return 0;
    memset(outResult, 0, sizeof(*outResult));
    outResult->kind = CLICK_ON_WALL_NONE;
    outResult->doorState = -1;

    if (!dungeon || !party) return 0;

    if (party->direction < 0 || party->direction > 3) return 0;
    fx = party->mapX + s_dx[party->direction];
    fy = party->mapY + s_dy[party->direction];

    outResult->mapIndex = party->mapIndex;
    outResult->mapX = fx;
    outResult->mapY = fy;

    if (!door_action_read_square(dungeon, party->mapIndex, fx, fy, &squareByte)) {
        return 0;   /* out of range; caller treats as CLICK_ON_WALL_NONE */
    }

    elementType = (squareByte & DUNGEON_SQUARE_MASK_TYPE) >> 5;
    outResult->elementType = elementType;

    if (elementType == DUNGEON_ELEMENT_DOOR) {
        outResult->doorState = squareByte & 0x07;
        outResult->kind = CLICK_ON_WALL_FRONT_DOOR_TOGGLE;
        /* A door square may additionally carry a sensor thing list
         * (e.g. a door actuator).  We still report FRONT_DOOR_TOGGLE
         * as the primary routing hint; Pass 32 runtime is responsible
         * for running the sensor effects as part of door actuation. */
        if ((squareByte & DUNGEON_SQUARE_MASK_THING_LIST) && things) {
            struct SensorOnSquare_Compat sensor;
            if (F0703_MOVEMENT_IdentifySensorsOnSquare_Compat(
                    dungeon, things, party->mapIndex, fx, fy, &sensor) &&
                sensor.found) {
                outResult->hasSensor = 1;
            }
        }
        return 1;
    }

    /* Non-door front cell.  If the square carries sensor(s), surface
     * them for the caller.  A wall square with an attached sensor is
     * the button / pressure-plate-equivalent wall-click case. */
    if ((squareByte & DUNGEON_SQUARE_MASK_THING_LIST) && things) {
        struct SensorOnSquare_Compat sensor;
        if (F0703_MOVEMENT_IdentifySensorsOnSquare_Compat(
                dungeon, things, party->mapIndex, fx, fy, &sensor) &&
            sensor.found) {
            outResult->hasSensor = 1;
            outResult->kind = CLICK_ON_WALL_FRONT_CELL_SENSOR;
            return 1;
        }
    }

    outResult->kind = CLICK_ON_WALL_NONE;
    return 0;
}
