#include "dm1_v1_dungeon_stairs_pc34_compat.h"

#include <string.h>

static int dm1_v1_f0154_location_after_level_change(
    const struct DungeonDatState_Compat *dungeon,
    int mapIndex,
    int levelDelta,
    int *mapX,
    int *mapY)
{
    const struct DungeonMapDesc_Compat *sourceMap;
    int globalX;
    int globalY;
    int targetLevel;
    int targetMapIndex;

    if (!dungeon || !dungeon->maps || !mapX || !mapY || mapIndex < 0 ||
        mapIndex >= (int)dungeon->header.mapCount) {
        return -1;
    }

    sourceMap = &dungeon->maps[mapIndex];
    globalX = (int)sourceMap->offsetMapX + *mapX;
    globalY = (int)sourceMap->offsetMapY + *mapY;
    targetLevel = (int)sourceMap->level + levelDelta;

    /* DUNGEON.C F0154:1508-1558.  Firestaff descriptors store actual
     * dimensions, so the source's inclusive Offset + storedWidth check is
     * represented by the equivalent exclusive actual-dimension bound. */
    for (targetMapIndex = 0;
         targetMapIndex < (int)dungeon->header.mapCount;
         ++targetMapIndex) {
        const struct DungeonMapDesc_Compat *targetMap =
            &dungeon->maps[targetMapIndex];
        if ((int)targetMap->level == targetLevel &&
            globalX >= (int)targetMap->offsetMapX &&
            globalX < (int)targetMap->offsetMapX + (int)targetMap->width &&
            globalY >= (int)targetMap->offsetMapY &&
            globalY < (int)targetMap->offsetMapY + (int)targetMap->height) {
            *mapX = globalX - (int)targetMap->offsetMapX;
            *mapY = globalY - (int)targetMap->offsetMapY;
            return targetMapIndex;
        }
    }
    return -1;
}

static int dm1_v1_f0155_stairs_exit_direction(
    const struct DungeonDatState_Compat *dungeon,
    int mapIndex,
    int mapX,
    int mapY)
{
    const struct DungeonMapDesc_Compat *map;
    const unsigned char *squares;
    unsigned char square;
    int northSouthOriented;
    int checkX;
    int checkY;
    int squareType;

    if (!dungeon || !dungeon->maps || !dungeon->tiles ||
        !dungeon->tilesLoaded || mapIndex < 0 ||
        mapIndex >= (int)dungeon->header.mapCount) {
        return -1;
    }
    map = &dungeon->maps[mapIndex];
    squares = dungeon->tiles[mapIndex].squareData;
    if (!squares || mapX < 0 || mapX >= (int)map->width ||
        mapY < 0 || mapY >= (int)map->height) {
        return -1;
    }

    square = squares[mapX * (int)map->height + mapY];
    northSouthOriented = (square & 0x08u) == 0;
    checkX = mapX + (northSouthOriented ? 1 : 0);
    checkY = mapY + (northSouthOriented ? 0 : -1);

    /* F0155 calls F0151: an unavailable neighbor is a wall. */
    if (checkX < 0 || checkX >= (int)map->width || checkY < 0 ||
        checkY >= (int)map->height) {
        return (1 << 1) + northSouthOriented;
    }
    squareType = (squares[checkX * (int)map->height + checkY] &
                  DUNGEON_SQUARE_MASK_TYPE) >> 5;
    return (((squareType == DUNGEON_ELEMENT_WALL ||
              squareType == DUNGEON_ELEMENT_STAIRS) << 1) +
            northSouthOriented);
}

int dm1_v1_dungeon_resolve_stairs_transition_pc34(
    const struct DungeonDatState_Compat *dungeon,
    const struct PartyState_Compat *party,
    struct StairsTransitionResult_Compat *outResult)
{
    const struct DungeonMapDesc_Compat *map;
    unsigned char square;
    int targetMapIndex;
    int exitDirection;

    if (!outResult) return 0;
    memset(outResult, 0, sizeof(*outResult));
    if (!dungeon || !party || !dungeon->maps || !dungeon->tiles ||
        !dungeon->tilesLoaded || party->mapIndex < 0 ||
        party->mapIndex >= (int)dungeon->header.mapCount) {
        return 0;
    }

    outResult->fromMapIndex = party->mapIndex;
    outResult->toMapIndex = party->mapIndex;
    outResult->newMapX = party->mapX;
    outResult->newMapY = party->mapY;
    outResult->newDirection = party->direction;
    map = &dungeon->maps[party->mapIndex];
    if (!dungeon->tiles[party->mapIndex].squareData || party->mapX < 0 ||
        party->mapX >= (int)map->width || party->mapY < 0 ||
        party->mapY >= (int)map->height) {
        return 0;
    }

    square = dungeon->tiles[party->mapIndex].squareData[
        party->mapX * (int)map->height + party->mapY];
    if (((square & DUNGEON_SQUARE_MASK_TYPE) >> 5) != DUNGEON_ELEMENT_STAIRS) {
        return 0;
    }

    outResult->stairUp = (square & 0x04u) != 0;
    targetMapIndex = dm1_v1_f0154_location_after_level_change(
        dungeon, party->mapIndex, outResult->stairUp ? -1 : 1,
        &outResult->newMapX, &outResult->newMapY);
    if (targetMapIndex < 0) return 0;

    exitDirection = dm1_v1_f0155_stairs_exit_direction(
        dungeon, targetMapIndex, outResult->newMapX, outResult->newMapY);
    if (exitDirection < 0) return 0;

    outResult->transitioned = 1;
    outResult->toMapIndex = targetMapIndex;
    outResult->newDirection = exitDirection;
    return 1;
}

const char *dm1_v1_dungeon_stairs_source_evidence_pc34(void)
{
    return "ReDMCSB DUNGEON.C:F0154_DUNGEON_GetLocationAfterLevelChange:1508-1558; "
           "F0155_DUNGEON_GetStairsExitDirection:1560-1582; "
           "CLIKMENU.C:F0364_COMMAND_TakeStairs";
}
