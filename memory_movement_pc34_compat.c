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
 *   CORRIDOR, PIT, TELEPORTER, FAKEWALL, STAIRS (stairs act as a
 *   consequence square, entering them triggers a map transition).
 *   DOOR: only when the low 3 door-state bits are 0 (fully open) or the
 *   door is destroyed (state 5).  Closed/opening/closing/animating doors
 *   block movement.
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
        case DUNGEON_ELEMENT_FAKEWALL:
        case DUNGEON_ELEMENT_STAIRS:
            return 1;
        case DUNGEON_ELEMENT_DOOR:
            doorState = squareByte & 0x07;
            /* 0 = fully open, 5 = destroyed; other states block. */
            return (doorState == 0 || doorState == 5) ? 1 : 0;
        case DUNGEON_ELEMENT_WALL:
        default:
            return 0;
    }
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

    /* Source-semantic door passability check (MOVESENS.C / DUNGEON.C).
     * A closed or animating door blocks the step. */
    if (elementType == DUNGEON_ELEMENT_DOOR) {
        doorState = squareByte & 0x07;
        if (doorState != 0 && doorState != 5) {
            outResult->resultCode = MOVE_BLOCKED_DOOR;
            return 0;
        }
    }

    /* Corridor / pit / teleporter / fake-wall / stairs / open door: pass.
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
    const struct DungeonMapDesc_Compat* targetMap;
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

    /* Attribute bit 0 of the low nibble selects direction (Fontanel/ReDMCSB):
     *   0 = stairs down (mapIndex + 1)
     *   1 = stairs up   (mapIndex - 1)
     */
    stairUp = (squareByte & 0x01);
    targetLevel = stairUp ? party->mapIndex - 1 : party->mapIndex + 1;
    if (targetLevel < 0 || targetLevel >= (int)dungeon->header.mapCount) return 0;

    targetMap = &dungeon->maps[targetLevel];
    outResult->transitioned = 1;
    outResult->stairUp = stairUp ? 1 : 0;
    outResult->toMapIndex = targetLevel;
    outResult->newMapX = party->mapX;
    outResult->newMapY = party->mapY;
    outResult->newDirection = party->direction;
    if (outResult->newMapX >= (int)targetMap->width) {
        outResult->newMapX = (int)targetMap->width - 1;
    }
    if (outResult->newMapY >= (int)targetMap->height) {
        outResult->newMapY = (int)targetMap->height - 1;
    }
    if (outResult->newMapX < 0) outResult->newMapX = 0;
    if (outResult->newMapY < 0) outResult->newMapY = 0;
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

        if (elementType == DUNGEON_ELEMENT_PIT) {
            int targetLevel = cursor.mapIndex + 1;
            const struct DungeonMapDesc_Compat* targetMap;
            int i;

            if (targetLevel < 0 || targetLevel >= (int)dungeon->header.mapCount) {
                break;
            }
            targetMap = &dungeon->maps[targetLevel];
            cursor.mapIndex = targetLevel;
            if (cursor.mapX >= targetMap->width) cursor.mapX = targetMap->width - 1;
            if (cursor.mapY >= targetMap->height) cursor.mapY = targetMap->height - 1;
            if (cursor.mapX < 0) cursor.mapX = 0;
            if (cursor.mapY < 0) cursor.mapY = 0;
            for (i = 0; i < party->championCount && i < CHAMPION_MAX_PARTY; ++i) {
                if (party->champions[i].present && party->champions[i].hp.current > 0) {
                    outResolution->championFallDamage[i] += 5 + (int)(gameTick % 11u);
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

            if (!things || !things->teleporters ||
                !movement_find_teleporter_on_square(dungeon, things,
                    cursor.mapIndex, cursor.mapX, cursor.mapY, &tp)) {
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
