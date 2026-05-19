#include "memory_movement_pc34_compat.h"
#include <string.h>

static const unsigned char s_movementThingDataByteCount[16] = {
    4, 6, 4, 8, 16, 4, 4, 4, 4, 8, 4, 0, 0, 0, 8, 4
};

static int movement_square_index(
    const struct DungeonDatState_Compat* dungeon,
    int mapIndex,
    int mapX,
    int mapY)
{
    int base = 0;
    int i;

    if (!dungeon || mapIndex < 0 || mapIndex >= (int)dungeon->header.mapCount) {
        return -1;
    }
    for (i = 0; i < mapIndex; ++i) {
        base += (int)dungeon->maps[i].width * (int)dungeon->maps[i].height;
    }
    return base + mapX * (int)dungeon->maps[mapIndex].height + mapY;
}

static unsigned short movement_first_square_thing(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    int mapIndex,
    int mapX,
    int mapY)
{
    int squareIndex;

    if (!dungeon || !things || !things->squareFirstThings) {
        return THING_NONE;
    }
    squareIndex = movement_square_index(dungeon, mapIndex, mapX, mapY);
    if (squareIndex < 0 || squareIndex >= things->squareFirstThingCount) {
        return THING_NONE;
    }
    return things->squareFirstThings[squareIndex];
}

static unsigned short movement_next_thing(
    const struct DungeonThings_Compat* things,
    unsigned short thing)
{
    int type;
    int index;
    const unsigned char* raw;

    if (!things || thing == THING_NONE || thing == THING_ENDOFLIST) {
        return THING_NONE;
    }
    type = THING_GET_TYPE(thing);
    index = THING_GET_INDEX(thing);
    if (type < 0 || type >= 16 || !things->rawThingData[type] ||
        index < 0 || index >= things->thingCounts[type] ||
        s_movementThingDataByteCount[type] < 2) {
        return THING_NONE;
    }
    raw = things->rawThingData[type] + (index * s_movementThingDataByteCount[type]);
    return (unsigned short)(raw[0] | ((unsigned short)raw[1] << 8));
}


static int movement_get_location_after_level_change(
    const struct DungeonDatState_Compat* dungeon,
    int sourceMapIndex,
    int levelDelta,
    int* mapX,
    int* mapY)
{
    const struct DungeonMapDesc_Compat* sourceMap;
    int globalX;
    int globalY;
    int targetLevel;
    int i;

    if (!dungeon || !dungeon->maps || !mapX || !mapY ||
        sourceMapIndex < 0 || sourceMapIndex >= (int)dungeon->header.mapCount) {
        return -1;
    }

    sourceMap = &dungeon->maps[sourceMapIndex];
    globalX = (int)sourceMap->offsetMapX + *mapX;
    globalY = (int)sourceMap->offsetMapY + *mapY;
    targetLevel = (int)sourceMap->level + levelDelta;

    for (i = 0; i < (int)dungeon->header.mapCount; ++i) {
        const struct DungeonMapDesc_Compat* targetMap = &dungeon->maps[i];
        if ((int)targetMap->level == targetLevel &&
            globalX >= (int)targetMap->offsetMapX &&
            globalX < (int)targetMap->offsetMapX + (int)targetMap->width &&
            globalY >= (int)targetMap->offsetMapY &&
            globalY < (int)targetMap->offsetMapY + (int)targetMap->height) {
            *mapX = globalX - (int)targetMap->offsetMapX;
            *mapY = globalY - (int)targetMap->offsetMapY;
            return i;
        }
    }

    return -1;
}

static int movement_get_stairs_exit_direction(
    const struct DungeonDatState_Compat* dungeon,
    int mapIndex,
    int mapX,
    int mapY)
{
    const struct DungeonMapDesc_Compat* map;
    unsigned char squareByte;
    int northSouth;
    int checkX;
    int checkY;
    int blocked = 1;

    if (!dungeon || !dungeon->maps || !dungeon->tiles || !dungeon->tilesLoaded ||
        mapIndex < 0 || mapIndex >= (int)dungeon->header.mapCount) {
        return DIR_NORTH;
    }
    map = &dungeon->maps[mapIndex];
    if (mapX < 0 || mapX >= (int)map->width || mapY < 0 || mapY >= (int)map->height ||
        !dungeon->tiles[mapIndex].squareData) {
        return DIR_NORTH;
    }

    squareByte = dungeon->tiles[mapIndex].squareData[mapX * (int)map->height + mapY];
    northSouth = (squareByte & 0x08) ? 0 : 1;
    /* ReDMCSB F0155: if NS-oriented, check EAST neighbor (+1,0);
     * if EW-oriented, check NORTH neighbor (0,-1).
     * Previous code had these swapped, causing wrong exit direction. */
    checkX = mapX + (northSouth ? 1 : 0);
    checkY = mapY + (northSouth ? 0 : -1);
    if (checkX >= 0 && checkX < (int)map->width && checkY >= 0 && checkY < (int)map->height) {
        int checkType = (dungeon->tiles[mapIndex].squareData[checkX * (int)map->height + checkY] &
                         DUNGEON_SQUARE_MASK_TYPE) >> 5;
        blocked = (checkType == DUNGEON_ELEMENT_WALL || checkType == DUNGEON_ELEMENT_STAIRS);
    }
    return (blocked << 1) + northSouth;
}

static int movement_find_teleporter_on_square(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    int mapIndex,
    int mapX,
    int mapY,
    struct DungeonTeleporter_Compat* outTeleporter)
{
    unsigned short thing;
    int safety = 0;

    if (!things || !outTeleporter) {
        return 0;
    }
    thing = movement_first_square_thing(dungeon, things, mapIndex, mapX, mapY);
    while (thing != THING_NONE && thing != THING_ENDOFLIST && safety < 64) {
        int type = THING_GET_TYPE(thing);
        int index = THING_GET_INDEX(thing);
        if (type == THING_TYPE_TELEPORTER && index >= 0 && index < things->teleporterCount) {
            *outTeleporter = things->teleporters[index];
            return 1;
        }
        thing = movement_next_thing(things, thing);
        ++safety;
    }
    return 0;
}

/*
 * Movement implementation — pure functions, no side effects.
 * Source: MOVESENS.C direction/step logic.
 */

int F0700_MOVEMENT_TurnDirection_Compat(int currentDir, int turnRight) {
    if (turnRight) {
        return (currentDir + 1) & 3;  /* N→E→S→W→N */
    } else {
        return (currentDir + 3) & 3;  /* N→W→S→E→N  (equiv: -1 mod 4) */
    }
}

/*
 * DM coordinate system (from MOVESENS.C / DEFS.H):
 *   North = direction 0 → dy = -1
 *   East  = direction 1 → dx = +1
 *   South = direction 2 → dy = +1
 *   West  = direction 3 → dx = -1
 *
 * Movement actions are relative to facing:
 *   Forward  = step in facing direction
 *   Right    = step 90° clockwise from facing
 *   Backward = step opposite to facing
 *   Left     = step 90° counter-clockwise from facing
 */

static const int s_dx[4] = {  0,  1,  0, -1 };  /* N, E, S, W */
static const int s_dy[4] = { -1,  0,  1,  0 };

void F0701_MOVEMENT_GetStepDelta_Compat(
    int direction,
    int moveAction,
    int* outDx,
    int* outDy)
{
    int stepDir;

    switch (moveAction) {
        case MOVE_FORWARD:  stepDir = direction; break;
        case MOVE_RIGHT:    stepDir = (direction + 1) & 3; break;
        case MOVE_BACKWARD: stepDir = (direction + 2) & 3; break;
        case MOVE_LEFT:     stepDir = (direction + 3) & 3; break;
        default:
            *outDx = 0;
            *outDy = 0;
            return;
    }

    *outDx = s_dx[stepDir];
    *outDy = s_dy[stepDir];
}

/*
 * Shared square-passability owner.  Mirrors the square-element branches
 * used by F0267_MOVE_GetMoveResult_CPSCE / DUNGEON.C helpers when deciding
 * whether a party or creature step may enter a target square.
 *
 * Passable:
 *   CORRIDOR, PIT, TELEPORTER, STAIRS (stairs act as a consequence
 *   square, entering them triggers a map transition).
 *   FAKEWALL: only when the OPEN (0x04) or IMAGINARY (0x01) bit is set.
 *   DOOR: when the low 3 door-state bits are 0 (fully open), 1
 *   (closed one-fourth, source allows entry), or 5 (destroyed).
 *   Closed/opening/closing states 2..4 block movement.
 * Blocked:
 *   WALL, out-of-bounds, any unknown element type.
 */
int F0706_MOVEMENT_IsSquarePassable_Compat(
    const struct DungeonDatState_Compat* dungeon,
    int mapIndex,
    int mapX,
    int mapY)
{
    const struct DungeonMapDesc_Compat* map;
    unsigned char squareByte;
    int elementType;
    int doorState;

    if (!dungeon || !dungeon->tilesLoaded || !dungeon->tiles) return 0;
    if (mapIndex < 0 || mapIndex >= (int)dungeon->header.mapCount) return 0;
    map = &dungeon->maps[mapIndex];
    if (mapX < 0 || mapX >= map->width || mapY < 0 || mapY >= map->height) return 0;
    if (!dungeon->tiles[mapIndex].squareData) return 0;

    squareByte = dungeon->tiles[mapIndex].squareData[mapX * map->height + mapY];
    elementType = (squareByte & DUNGEON_SQUARE_MASK_TYPE) >> 5;

    switch (elementType) {
        case DUNGEON_ELEMENT_CORRIDOR:
        case DUNGEON_ELEMENT_PIT:
        case DUNGEON_ELEMENT_TELEPORTER:
        case DUNGEON_ELEMENT_STAIRS:
            return 1;
        case DUNGEON_ELEMENT_FAKEWALL:
            return ((squareByte & 0x04) || (squareByte & 0x01)) ? 1 : 0;
        case DUNGEON_ELEMENT_DOOR:
            doorState = squareByte & 0x07;
            /* 0 = open, 1 = closed one-fourth, 5 = destroyed; other states block. */
            return (doorState == 0 || doorState == 1 || doorState == 5) ? 1 : 0;
        case DUNGEON_ELEMENT_WALL:
        default:
            return 0;
    }
}

/*
 * Pass 39: context-aware square passability.
 *
 * Shares element/door decoding with F0706 so creature and party
 * walkability cannot drift.  The only behavioral difference vs F0706
 * is that the creature context treats a stairs square as blocked, to
 * match ReDMCSB creature-movement legality
 * (GROUP.C / F0264_MOVE_IsSquareAccessibleForCreature): creatures in
 * DM1 PC 3.4 never step onto stairs.  Party context is identical to
 * F0706.
 *
 * This keeps M11 creature walkability source-faithful *and* unified
 * with the compat movement legality path used by F0702 / F0706, which
 * is the pass-39 acceptance requirement in V1_BLOCKERS.md §3.
 */
int F0707_MOVEMENT_IsSquarePassableForContext_Compat(
    const struct DungeonDatState_Compat* dungeon,
    int mapIndex,
    int mapX,
    int mapY,
    int passContext)
{
    const struct DungeonMapDesc_Compat* map;
    unsigned char squareByte;
    int elementType;
    int doorState;

    if (!dungeon || !dungeon->tilesLoaded || !dungeon->tiles) return 0;
    if (mapIndex < 0 || mapIndex >= (int)dungeon->header.mapCount) return 0;
    map = &dungeon->maps[mapIndex];
    if (mapX < 0 || mapX >= map->width || mapY < 0 || mapY >= map->height) return 0;
    if (!dungeon->tiles[mapIndex].squareData) return 0;

    squareByte = dungeon->tiles[mapIndex].squareData[mapX * map->height + mapY];
    elementType = (squareByte & DUNGEON_SQUARE_MASK_TYPE) >> 5;

    switch (elementType) {
        case DUNGEON_ELEMENT_CORRIDOR:
        case DUNGEON_ELEMENT_PIT:
        case DUNGEON_ELEMENT_TELEPORTER:
            return 1;
        case DUNGEON_ELEMENT_FAKEWALL:
            return ((squareByte & 0x04) || (squareByte & 0x01)) ? 1 : 0;
        case DUNGEON_ELEMENT_STAIRS:
            /* Party: stairs are a legal consequence square.
             * Creature: stairs are blocked. */
            return (passContext == MOVEMENT_PASS_CTX_CREATURE) ? 0 : 1;
        case DUNGEON_ELEMENT_DOOR:
            doorState = squareByte & 0x07;
            /* 0 = open, 1 = closed one-fourth, 5 = destroyed; other states block. */
            return (doorState == 0 || doorState == 1 || doorState == 5) ? 1 : 0;
        case DUNGEON_ELEMENT_WALL:
        default:
            return 0;
    }
}


int F0709_MOVEMENT_BuildIntermediaryProjectileImpactCells_Compat(
    int sourceMapX,
    int sourceMapY,
    int destinationMapX,
    int destinationMapY,
    const unsigned char sourceOrdinalInCell[4],
    unsigned char destinationOrdinalInCell[4],
    unsigned char intermediaryOrdinalInCell[4])
{
    int dx;
    int dy;
    int distance;
    int primaryDirection;
    int secondaryDirection;
    int i;

    if (!sourceOrdinalInCell || !destinationOrdinalInCell || !intermediaryOrdinalInCell) {
        return 0;
    }

    for (i = 0; i < 4; ++i) {
        destinationOrdinalInCell[i] = sourceOrdinalInCell[i];
        intermediaryOrdinalInCell[i] = 0;
    }

    if (destinationMapX < 0) {
        return 0;
    }

    dx = sourceMapX - destinationMapX;
    dy = sourceMapY - destinationMapY;
    distance = (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy);
    if (distance != 1) {
        return 0;
    }

    if (destinationMapY < sourceMapY) {
        primaryDirection = DIR_NORTH;
    } else if (destinationMapX > sourceMapX) {
        primaryDirection = DIR_EAST;
    } else if (destinationMapY > sourceMapY) {
        primaryDirection = DIR_SOUTH;
    } else {
        primaryDirection = DIR_WEST;
    }

    secondaryDirection = (primaryDirection + 1) & 3;

    /* MOVESENS.C:276-280: edge occupants cross through the opposite
     * intermediary cells, enabling destination-square projectile impacts
     * that would otherwise be missed while moving between adjacent squares. */
    intermediaryOrdinalInCell[(primaryDirection + 3) & 3] =
        sourceOrdinalInCell[primaryDirection];
    intermediaryOrdinalInCell[(secondaryDirection + 1) & 3] =
        sourceOrdinalInCell[secondaryDirection];

    /* MOVESENS.C:282-287: carry rear-edge occupants into the front-edge
     * destination cells when those destination-edge cells are empty. */
    if (!destinationOrdinalInCell[primaryDirection]) {
        destinationOrdinalInCell[primaryDirection] =
            destinationOrdinalInCell[(primaryDirection + 3) & 3];
    }
    if (!destinationOrdinalInCell[secondaryDirection]) {
        destinationOrdinalInCell[secondaryDirection] =
            destinationOrdinalInCell[(secondaryDirection + 1) & 3];
    }

    return (intermediaryOrdinalInCell[(primaryDirection + 3) & 3] ||
            intermediaryOrdinalInCell[(secondaryDirection + 1) & 3]) ? 1 : 0;
}

int F0702_MOVEMENT_TryMove_Compat(
    const struct DungeonDatState_Compat* dungeon,
    const struct PartyState_Compat* party,
    int moveAction,
    struct MovementResult_Compat* outResult)
{
    int dx, dy, nx, ny;
    const struct DungeonMapDesc_Compat* map;
    int elementType;
    unsigned char squareByte;
    int doorState;

    memset(outResult, 0, sizeof(*outResult));
    outResult->newMapX = party->mapX;
    outResult->newMapY = party->mapY;
    outResult->newDirection = party->direction;
    outResult->newMapIndex = party->mapIndex;

    /* Handle turns */
    if (moveAction == MOVE_TURN_RIGHT) {
        outResult->newDirection = F0700_MOVEMENT_TurnDirection_Compat(party->direction, 1);
        outResult->resultCode = MOVE_TURN_ONLY;
        return 1;
    }
    if (moveAction == MOVE_TURN_LEFT) {
        outResult->newDirection = F0700_MOVEMENT_TurnDirection_Compat(party->direction, 0);
        outResult->resultCode = MOVE_TURN_ONLY;
        return 1;
    }

    /* Step actions */
    F0701_MOVEMENT_GetStepDelta_Compat(party->direction, moveAction, &dx, &dy);
    nx = party->mapX + dx;
    ny = party->mapY + dy;

    /* Bounds check */
    if (!dungeon || party->mapIndex < 0 ||
        party->mapIndex >= (int)dungeon->header.mapCount) {
        outResult->resultCode = MOVE_BLOCKED_BOUNDS;
        return 0;
    }
    map = &dungeon->maps[party->mapIndex];

    if (nx < 0 || nx >= map->width || ny < 0 || ny >= map->height) {
        outResult->resultCode = MOVE_BLOCKED_BOUNDS;
        return 0;
    }

    /* Check tile walkability */
    if (!dungeon->tilesLoaded || !dungeon->tiles) {
        outResult->resultCode = MOVE_BLOCKED_WALL;
        return 0;
    }

    /* Column-major: [col * height + row] */
    squareByte = dungeon->tiles[party->mapIndex].squareData[nx * map->height + ny];
    elementType = (squareByte & DUNGEON_SQUARE_MASK_TYPE) >> 5;

    if (elementType == DUNGEON_ELEMENT_WALL) {
        outResult->resultCode = MOVE_BLOCKED_WALL;
        return 0;
    }

    /* Source-semantic door/fake-wall passability check
     * (CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty). */
    if (elementType == DUNGEON_ELEMENT_DOOR) {
        doorState = squareByte & 0x07;
        if (doorState != 0 && doorState != 1 && doorState != 5) {
            outResult->resultCode = MOVE_BLOCKED_DOOR;
            return 0;
        }
    } else if (elementType == DUNGEON_ELEMENT_FAKEWALL) {
        if (!(squareByte & 0x04) && !(squareByte & 0x01)) {
            outResult->resultCode = MOVE_BLOCKED_WALL;
            return 0;
        }
    }

    /* Corridor / pit / teleporter / open-or-imaginary fake-wall / stairs / open door: pass.
     * Stairs traversal consequence itself is resolved by F0705 after the
     * step has been committed. */
    outResult->newMapX = nx;
    outResult->newMapY = ny;
    outResult->resultCode = MOVE_OK;
    return 1;
}

int F0703_MOVEMENT_IdentifySensorsOnSquare_Compat(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    int mapIndex,
    int mapX,
    int mapY,
    struct SensorOnSquare_Compat* outSensor)
{
    const struct DungeonMapDesc_Compat* map;
    int squareIdx, sftIdx;
    unsigned short thingRef;
    unsigned char squareByte;
    int sensorCount;

    memset(outSensor, 0, sizeof(*outSensor));

    if (!dungeon || !dungeon->loaded || !dungeon->tilesLoaded) return 0;
    if (!things || !things->loaded) return 0;
    if (mapIndex < 0 || mapIndex >= (int)dungeon->header.mapCount) return 0;

    map = &dungeon->maps[mapIndex];
    if (mapX < 0 || mapX >= map->width || mapY < 0 || mapY >= map->height) return 0;

    /* Column-major indexing */
    squareByte = dungeon->tiles[mapIndex].squareData[mapX * map->height + mapY];

    /* Check if this square has a thing list */
    if (!(squareByte & DUNGEON_SQUARE_MASK_THING_LIST)) return 0;

    squareIdx = mapX * map->height + mapY;

    /* Find the square-first-thing index by counting thing-list squares
     * up to this one (inclusive). Bit 4 of squareByte = has thing list. */
    {
        int m, totalThingSquaresBefore = 0;
        /* Count thing-list squares in maps before this one */
        for (m = 0; m < mapIndex; m++) {
            int sq;
            int count = dungeon->maps[m].width * dungeon->maps[m].height;
            for (sq = 0; sq < count; sq++) {
                if (dungeon->tiles[m].squareData[sq] & DUNGEON_SQUARE_MASK_THING_LIST) {
                    totalThingSquaresBefore++;
                }
            }
        }
        /* Count thing-list squares in this map before (and including) squareIdx */
        {
            int sq;
            int count = map->width * map->height;
            for (sq = 0; sq < count && sq <= squareIdx; sq++) {
                if (dungeon->tiles[mapIndex].squareData[sq] & DUNGEON_SQUARE_MASK_THING_LIST) {
                    totalThingSquaresBefore++;
                }
            }
        }
        /* The index is totalThingSquaresBefore - 1 (we counted our square) */
        sftIdx = totalThingSquaresBefore - 1;
    }

    if (sftIdx < 0 || sftIdx >= things->squareFirstThingCount) return 0;

    thingRef = things->squareFirstThings[sftIdx];
    sensorCount = 0;

    /* Walk the linked list, looking for sensors */
    while (thingRef != THING_NONE && thingRef != THING_ENDOFLIST) {
        int type = THING_GET_TYPE(thingRef);
        int index = THING_GET_INDEX(thingRef);

        if (type == THING_TYPE_SENSOR && index < things->sensorCount) {
            const struct DungeonSensor_Compat* sensor = &things->sensors[index];

            if (sensorCount == 0) {
                /* Report first sensor */
                outSensor->found = 1;
                outSensor->sensorIndex = index;
                outSensor->sensorType = sensor->sensorType;
                outSensor->sensorData = sensor->sensorData;
                outSensor->isLocal = sensor->localEffect;

                if (!sensor->localEffect) {
                    outSensor->targetMapX = sensor->targetMapX;
                    outSensor->targetMapY = sensor->targetMapY;
                    outSensor->targetCell = sensor->targetCell;
                } else {
                    outSensor->targetMapX = mapX;
                    outSensor->targetMapY = mapY;
                    outSensor->targetCell = 0;
                }
            }
            sensorCount++;

            /* Follow sensor's next pointer */
            thingRef = sensor->next;
        } else {
            /* Follow the thing's next pointer based on type */
            unsigned short nextThing = THING_NONE;

            switch (type) {
                case THING_TYPE_DOOR:
                    if (index < things->doorCount)
                        nextThing = things->doors[index].next;
                    break;
                case THING_TYPE_TELEPORTER:
                    if (index < things->teleporterCount)
                        nextThing = things->teleporters[index].next;
                    break;
                case THING_TYPE_TEXTSTRING:
                    if (index < things->textStringCount)
                        nextThing = things->textStrings[index].next;
                    break;
                case THING_TYPE_SENSOR:
                    /* already handled above */
                    break;
                case THING_TYPE_GROUP:
                    if (index < things->groupCount)
                        nextThing = things->groups[index].next;
                    break;
                case THING_TYPE_WEAPON:
                    if (index < things->weaponCount)
                        nextThing = things->weapons[index].next;
                    break;
                case THING_TYPE_ARMOUR:
                    if (index < things->armourCount)
                        nextThing = things->armours[index].next;
                    break;
                case THING_TYPE_SCROLL:
                    if (index < things->scrollCount)
                        nextThing = things->scrolls[index].next;
                    break;
                case THING_TYPE_POTION:
                    if (index < things->potionCount)
                        nextThing = things->potions[index].next;
                    break;
                case THING_TYPE_CONTAINER:
                    if (index < things->containerCount)
                        nextThing = things->containers[index].next;
                    break;
                case THING_TYPE_JUNK:
                    if (index < things->junkCount)
                        nextThing = things->junks[index].next;
                    break;
                case THING_TYPE_PROJECTILE:
                    if (index < things->projectileCount)
                        nextThing = things->projectiles[index].next;
                    break;
                case THING_TYPE_EXPLOSION:
                    if (index < things->explosionCount)
                        nextThing = things->explosions[index].next;
                    break;
                default:
                    nextThing = THING_NONE;
                    break;
            }
            thingRef = nextThing;
        }
    }

    outSensor->totalSensorsOnSquare = sensorCount;
    return (sensorCount > 0) ? 1 : 0;
}

int F0705_MOVEMENT_ResolveStairsTransition_Compat(
    const struct DungeonDatState_Compat* dungeon,
    const struct PartyState_Compat* party,
    struct StairsTransitionResult_Compat* outResult)
{
    const struct DungeonMapDesc_Compat* map;
    unsigned char squareByte;
    int elementType;
    int targetLevel;
    int stairUp;

    if (!outResult) return 0;
    memset(outResult, 0, sizeof(*outResult));

    if (!dungeon || !party) return 0;
    outResult->fromMapIndex = party->mapIndex;
    outResult->toMapIndex = party->mapIndex;
    outResult->newMapX = party->mapX;
    outResult->newMapY = party->mapY;
    outResult->newDirection = party->direction;

    if (party->mapIndex < 0 || party->mapIndex >= (int)dungeon->header.mapCount) return 0;
    map = &dungeon->maps[party->mapIndex];
    if (party->mapX < 0 || party->mapX >= map->width ||
        party->mapY < 0 || party->mapY >= map->height) return 0;
    if (!dungeon->tilesLoaded || !dungeon->tiles ||
        !dungeon->tiles[party->mapIndex].squareData) return 0;

    squareByte = dungeon->tiles[party->mapIndex].squareData[
        party->mapX * map->height + party->mapY];
    elementType = (squareByte & DUNGEON_SQUARE_MASK_TYPE) >> 5;
    if (elementType != DUNGEON_ELEMENT_STAIRS) return 0;

    /* MASK0x0004_STAIRS_UP selects the level delta used by
     * F0364_COMMAND_TakeStairs: up => -1 level, otherwise +1 level.
     * F0154_DUNGEON_GetLocationAfterLevelChange maps through global
     * offsetMapX/offsetMapY coordinates, not raw map-index +/- 1.
     */
    stairUp = (squareByte & 0x04) ? 1 : 0;
    targetLevel = movement_get_location_after_level_change(
        dungeon, party->mapIndex, stairUp ? -1 : 1,
        &outResult->newMapX, &outResult->newMapY);
    if (targetLevel < 0 || targetLevel >= (int)dungeon->header.mapCount) return 0;

    outResult->transitioned = 1;
    outResult->stairUp = stairUp;
    outResult->toMapIndex = targetLevel;
    outResult->newDirection = movement_get_stairs_exit_direction(
        dungeon, targetLevel, outResult->newMapX, outResult->newMapY);
    return 1;
}

int F0704_MOVEMENT_ResolvePostMoveEnvironment_Compat(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    const struct PartyState_Compat* party,
    uint32_t gameTick,
    struct PostMoveResolution_Compat* outResolution)
{
    struct PartyState_Compat cursor;
    int remaining = MOVEMENT_POST_MOVE_CHAIN_LIMIT;

    if (!dungeon || !party || !outResolution) {
        return 0;
    }
    (void)gameTick;

    memset(outResolution, 0, sizeof(*outResolution));
    cursor = *party;
    outResolution->finalMapX = cursor.mapX;
    outResolution->finalMapY = cursor.mapY;
    outResolution->finalDirection = cursor.direction;
    outResolution->finalMapIndex = cursor.mapIndex;

    while (remaining-- > 0) {
        const struct DungeonMapDesc_Compat* map;
        unsigned char squareByte;
        int squareIndex;
        int elementType;

        if (cursor.mapIndex < 0 || cursor.mapIndex >= (int)dungeon->header.mapCount) {
            break;
        }
        map = &dungeon->maps[cursor.mapIndex];
        if (cursor.mapX < 0 || cursor.mapX >= map->width ||
            cursor.mapY < 0 || cursor.mapY >= map->height) {
            break;
        }
        if (!dungeon->tilesLoaded || !dungeon->tiles || !dungeon->tiles[cursor.mapIndex].squareData) {
            break;
        }

        squareIndex = cursor.mapX * map->height + cursor.mapY;
        squareByte = dungeon->tiles[cursor.mapIndex].squareData[squareIndex];
        elementType = (squareByte & DUNGEON_SQUARE_MASK_TYPE) >> 5;

        if (elementType == DUNGEON_ELEMENT_PIT &&
            (squareByte & 0x08) && !(squareByte & 0x01)) {
            int targetLevel;
            int i;

            targetLevel = movement_get_location_after_level_change(
                dungeon, cursor.mapIndex, 1, &cursor.mapX, &cursor.mapY);
            if (targetLevel < 0 || targetLevel >= (int)dungeon->header.mapCount) {
                break;
            }
            cursor.mapIndex = targetLevel;
            for (i = 0; i < party->championCount && i < CHAMPION_MAX_PARTY; ++i) {
                if (party->champions[i].present && party->champions[i].hp.current > 0) {
                    outResolution->championFallDamage[i] += 20;
                }
            }
            outResolution->transitioned = 1;
            outResolution->chainCount += 1;
            outResolution->pitCount += 1;
            continue;
        }

        if (elementType == DUNGEON_ELEMENT_TELEPORTER) {
            struct DungeonTeleporter_Compat tp;
            const struct DungeonMapDesc_Compat* targetMap;
            int targetMapIndex;
            int targetX;
            int targetY;

            if (!(squareByte & 0x08) || !things || !things->teleporters ||
                !movement_find_teleporter_on_square(dungeon, things,
                    cursor.mapIndex, cursor.mapX, cursor.mapY, &tp) ||
                !(tp.scope & 0x02)) {
                break;
            }
            targetMapIndex = (int)tp.targetMapIndex;
            if (targetMapIndex < 0 || targetMapIndex >= (int)dungeon->header.mapCount) {
                break;
            }
            targetMap = &dungeon->maps[targetMapIndex];
            targetX = (int)tp.targetMapX;
            targetY = (int)tp.targetMapY;
            if (targetX >= targetMap->width) targetX = targetMap->width - 1;
            if (targetY >= targetMap->height) targetY = targetMap->height - 1;
            if (targetX < 0) targetX = 0;
            if (targetY < 0) targetY = 0;
            cursor.mapIndex = targetMapIndex;
            cursor.mapX = targetX;
            cursor.mapY = targetY;
            if (tp.absoluteRotation) {
                cursor.direction = (int)(tp.rotation & 3);
            } else if (tp.rotation != 0) {
                cursor.direction = (cursor.direction + (int)(tp.rotation & 3)) & 3;
            }
            outResolution->transitioned = 1;
            outResolution->chainCount += 1;
            outResolution->teleporterCount += 1;
            continue;
        }

        break;
    }

    outResolution->finalMapX = cursor.mapX;
    outResolution->finalMapY = cursor.mapY;
    outResolution->finalDirection = cursor.direction;
    outResolution->finalMapIndex = cursor.mapIndex;
    return 1;
}

int F0708_MOVEMENT_IsPartyStepBlockedByGroup_Compat(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    const struct PartyState_Compat* party,
    int moveAction)
{
    struct MovementResult_Compat moveResult;
    const struct DungeonMapDesc_Compat* map;
    unsigned char squareByte;
    int squareIndex;
    int sftIndex;
    int m;
    unsigned short thing;
    int safety = 0;

    if (!dungeon || !things || !party || !things->squareFirstThings) return 0;
    if (party->championCount <= 0) return 0;
    if (moveAction == MOVE_TURN_LEFT || moveAction == MOVE_TURN_RIGHT) return 0;

    if (!F0702_MOVEMENT_TryMove_Compat(dungeon, party, moveAction, &moveResult) ||
        moveResult.resultCode != MOVE_OK) {
        return 0;
    }
    if (moveResult.newMapIndex < 0 ||
        moveResult.newMapIndex >= (int)dungeon->header.mapCount ||
        !dungeon->tilesLoaded || !dungeon->tiles ||
        !dungeon->tiles[moveResult.newMapIndex].squareData) {
        return 0;
    }

    map = &dungeon->maps[moveResult.newMapIndex];
    if (moveResult.newMapX < 0 || moveResult.newMapX >= map->width ||
        moveResult.newMapY < 0 || moveResult.newMapY >= map->height) {
        return 0;
    }

    squareIndex = moveResult.newMapX * map->height + moveResult.newMapY;
    squareByte = dungeon->tiles[moveResult.newMapIndex].squareData[squareIndex];
    if (!(squareByte & DUNGEON_SQUARE_MASK_THING_LIST)) return 0;

    sftIndex = 0;
    for (m = 0; m < moveResult.newMapIndex; ++m) {
        int i;
        int count;
        if (!dungeon->tiles[m].squareData) return 0;
        count = dungeon->maps[m].width * dungeon->maps[m].height;
        for (i = 0; i < count; ++i) {
            if (dungeon->tiles[m].squareData[i] & DUNGEON_SQUARE_MASK_THING_LIST) {
                ++sftIndex;
            }
        }
    }
    {
        int i;
        int count = map->width * map->height;
        for (i = 0; i < count && i < squareIndex; ++i) {
            if (dungeon->tiles[moveResult.newMapIndex].squareData[i] &
                DUNGEON_SQUARE_MASK_THING_LIST) {
                ++sftIndex;
            }
        }
    }
    if (sftIndex < 0 || sftIndex >= things->squareFirstThingCount) return 0;

    thing = things->squareFirstThings[sftIndex];
    while (thing != THING_NONE && thing != THING_ENDOFLIST && safety++ < 64) {
        if (THING_GET_TYPE(thing) == THING_TYPE_GROUP) {
            /* ReDMCSB F0267/F0708: only LIVING creature groups block.
             * Dead groups (all health == 0) should not obstruct movement.
             * Previously, dead groups blocked passage indefinitely. */
            int gIdx = THING_GET_INDEX(thing);
            if (gIdx >= 0 && gIdx < things->groupCount) {
                int anyAlive = 0;
                int ci;
                for (ci = 0; ci <= (int)things->groups[gIdx].count; ++ci) {
                    if (things->groups[gIdx].health[ci] > 0) {
                        anyAlive = 1;
                        break;
                    }
                }
                if (anyAlive) return 1;
            }
        }
        thing = movement_next_thing(things, thing);
    }
    return 0;
}
