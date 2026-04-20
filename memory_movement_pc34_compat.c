#include "memory_movement_pc34_compat.h"
#include <string.h>

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
    if (party->mapIndex < 0 || party->mapIndex >= (int)dungeon->header.mapCount) {
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

    /* For now, treat all non-wall tiles as passable.
     * Door open/closed checks and fake-wall logic are future phases. */
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
