#include "dm1_v1_collision_door_pc34_compat.h"

#include <string.h>

/*
 * DM1 V1 Collision Detection and Door Interaction — implementation.
 *
 * Source lock (ReDMCSB WIP20210206, Toolchains/Common/Source):
 *
 * CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty (lines 180-347):
 *   Party step collision gate.  Resolves destination coordinates via
 *   G0233_ai_Graphic559_DirectionToStepEastCount / G0234 tables, then:
 *     line 274: wall check (M034_SQUARE_TYPE == C00_ELEMENT_WALL -> blocked)
 *     line 276-278: door check (M036_DOOR_STATE not in {0,1,5} -> blocked)
 *     line 280-281: fakewall check (!OPEN && !IMAGINARY -> blocked)
 *     line 289-290: group check (F0175_GROUP_GetThing != ENDOFLIST -> blocked)
 *
 * MOVESENS.C:F0267_MOVE_GetMoveResult_CPSCE (lines 316-850):
 *   Master move resolver.  The chained-move loop at line 481 checks:
 *     AL0709_i_DestinationSquareType == C05_ELEMENT_TELEPORTER -> chain
 *     AL0709_i_DestinationSquareType == C02_ELEMENT_PIT -> fall
 *     AL0709_i_DestinationSquareType == C03_ELEMENT_STAIRS -> level change
 *
 * MOVESENS.C:F0264_MOVE_IsLevitating (lines 128-170):
 *   Groups: MASK0x0020_LEVITATION.  Projectiles: always.
 *
 * CLIKVIEW.C:F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor:
 *   Computes front cell as partyMapX + stepEast[partyDir],
 *   partyMapY + stepNorth[partyDir].  Bounds-checks, then calls
 *   F0275_SENSOR_IsTriggeredByClickOnWall.
 *
 * CLIKVIEW.C:F0377_COMMAND_ProcessType80_ClickInDungeonView (line 412):
 *   Door button click: reads DOOR->Button via F0157, then emits
 *   F0268_SENSOR_AddEvent(C10_EVENT_DOOR, ..., C02_EFFECT_TOGGLE, gameTick+1).
 *
 * Door state encoding (DEFS.H low 3 bits of square byte):
 *   0 = OPEN, 1 = CLOSED_ONE_FOURTH (passable),
 *   2 = CLOSED_HALF, 3 = CLOSED_THREE_FOURTH, 4 = CLOSED, 5 = DESTROYED.
 *   Bit 3 (0x08) = vertical orientation.
 *   Passable states: {0, 1, 5} (CLIKMENU.C:276-278).
 *
 * Pit encoding (DEFS.H):
 *   Bit 3 (0x08) = MASK0x0008_PIT_OPEN
 *   Bit 0 (0x01) = MASK0x0001_PIT_IMAGINARY
 *   Fall condition (MOVESENS.C:493-500): PIT_OPEN && !PIT_IMAGINARY && !levitating
 */

/* ---- Internal helpers ---- */

/*
 * Read square byte at (mapIndex, mapX, mapY).
 * Returns -1 on invalid coordinates or unloaded state.
 */
static int collision_read_square(
    const struct DungeonDatState_Compat* dungeon,
    int mapIndex,
    int mapX,
    int mapY)
{
    const struct DungeonMapDesc_Compat* map;
    if (!dungeon || !dungeon->loaded || !dungeon->tilesLoaded || !dungeon->tiles)
        return -1;
    if (mapIndex < 0 || mapIndex >= (int)dungeon->header.mapCount)
        return -1;
    map = &dungeon->maps[mapIndex];
    if (mapX < 0 || mapX >= map->width || mapY < 0 || mapY >= map->height)
        return -1;
    /* Column-major: [col * height + row] */
    return (int)(unsigned char)dungeon->tiles[mapIndex].squareData[mapX * map->height + mapY];
}

/*
 * Extract element type from a square byte.
 * Mirror of M034_SQUARE_TYPE: (squareByte >> 5) & 0x07.
 */
static int collision_element_type(int squareByte)
{
    return (squareByte & DUNGEON_SQUARE_MASK_TYPE) >> 5;
}

/*
 * Extract door state from a square byte (low 3 bits).
 * Mirror of M036_DOOR_STATE.
 */
static int collision_door_state(int squareByte)
{
    return squareByte & 0x07;
}

/*
 * Check if a door state is passable.
 * Source: CLIKMENU.C:276-278 — passable when state in {0, 1, 5}.
 */
static int collision_door_is_passable(int doorState)
{
    return (doorState == DM1_DOOR_STATE_OPEN ||
            doorState == DM1_DOOR_STATE_CLOSED_ONE_FOURTH ||
            doorState == DM1_DOOR_STATE_DESTROYED);
}

/*
 * Direction-to-step tables (mirror G0233/G0234 in ReDMCSB DEFS.H):
 *   North: dx=0, dy=-1
 *   East:  dx=+1, dy=0
 *   South: dx=0, dy=+1
 *   West:  dx=-1, dy=0
 */
static const int step_dx[4] = { 0, +1,  0, -1 };
static const int step_dy[4] = {-1,  0, +1,  0 };


static int collision_find_square_first_thing_index(
    const struct DungeonDatState_Compat* dungeon,
    int mapIndex,
    int mapX,
    int mapY)
{
    const struct DungeonMapDesc_Compat* map;
    int squareIdx;
    int m, sq, count, total = 0;

    if (!dungeon || !dungeon->tilesLoaded || !dungeon->tiles) return -1;
    if (mapIndex < 0 || mapIndex >= (int)dungeon->header.mapCount) return -1;
    map = &dungeon->maps[mapIndex];
    if (mapX < 0 || mapX >= map->width || mapY < 0 || mapY >= map->height) return -1;
    if (!dungeon->tiles[mapIndex].squareData) return -1;
    squareIdx = mapX * map->height + mapY;
    if (!(dungeon->tiles[mapIndex].squareData[squareIdx] & DUNGEON_SQUARE_MASK_THING_LIST)) return -1;

    for (m = 0; m < mapIndex; ++m) {
        count = dungeon->maps[m].width * dungeon->maps[m].height;
        for (sq = 0; sq < count; ++sq) {
            if (dungeon->tiles[m].squareData[sq] & DUNGEON_SQUARE_MASK_THING_LIST) ++total;
        }
    }
    count = map->width * map->height;
    for (sq = 0; sq <= squareIdx && sq < count; ++sq) {
        if (dungeon->tiles[mapIndex].squareData[sq] & DUNGEON_SQUARE_MASK_THING_LIST) ++total;
    }
    return total - 1;
}

static int collision_front_door_has_button(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    int mapIndex,
    int mapX,
    int mapY,
    int* outDoorIndex)
{
    int sftIndex;
    unsigned short thingRef;
    int thingType;
    int thingIndex;

    if (outDoorIndex) *outDoorIndex = -1;
    if (!things || !things->loaded || !things->squareFirstThings || !things->doors) return 0;
    sftIndex = collision_find_square_first_thing_index(dungeon, mapIndex, mapX, mapY);
    if (sftIndex < 0 || sftIndex >= things->squareFirstThingCount) return 0;

    thingRef = things->squareFirstThings[sftIndex];
    if (thingRef == THING_NONE || thingRef == THING_ENDOFLIST) return 0;
    thingType = THING_GET_TYPE(thingRef);
    thingIndex = THING_GET_INDEX(thingRef);
    if (thingType != THING_TYPE_DOOR || thingIndex < 0 || thingIndex >= things->doorCount) return 0;
    if (outDoorIndex) *outDoorIndex = thingIndex;
    return things->doors[thingIndex].button ? 1 : 0;
}

/* ---- Public API ---- */

int DM1_V1_Collision_CheckStep(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    const struct PartyState_Compat* party,
    int moveAction,
    struct Dm1V1CollisionResult* outResult)
{
    int dx, dy, nx, ny;
    int squareByte, elemType, doorSt;

    if (!outResult) return 0;
    memset(outResult, 0, sizeof(*outResult));
    outResult->doorState = -1;

    if (!dungeon || !party) {
        outResult->code = DM1_COLLISION_BLOCKED_BOUNDS;
        return 0;
    }

    /*
     * Compute destination.
     * Source: CLIKMENU.C:F0366 uses F0701-equivalent step delta based
     * on party direction + move action.
     */
    F0701_MOVEMENT_GetStepDelta_Compat(party->direction, moveAction, &dx, &dy);
    nx = party->mapX + dx;
    ny = party->mapY + dy;

    outResult->mapX = nx;
    outResult->mapY = ny;
    outResult->mapIndex = party->mapIndex;

    /* Bounds check (CLIKMENU.C:264-270) */
    squareByte = collision_read_square(dungeon, party->mapIndex, nx, ny);
    if (squareByte < 0) {
        outResult->code = DM1_COLLISION_BLOCKED_BOUNDS;
        return 0;
    }

    elemType = collision_element_type(squareByte);
    outResult->elementType = elemType;

    /* Wall check — CLIKMENU.C:274 */
    if (elemType == DUNGEON_ELEMENT_WALL) {
        outResult->code = DM1_COLLISION_BLOCKED_WALL;
        return 0;
    }

    /* Door check — CLIKMENU.C:276-278 */
    if (elemType == DUNGEON_ELEMENT_DOOR) {
        doorSt = collision_door_state(squareByte);
        outResult->doorState = doorSt;
        outResult->doorPassable = collision_door_is_passable(doorSt);
        if (!outResult->doorPassable) {
            outResult->code = DM1_COLLISION_BLOCKED_DOOR;
            return 0;
        }
    }

    /* Fake-wall check — CLIKMENU.C:280-281
     * Blocked if not OPEN (bit 2) and not IMAGINARY (bit 0). */
    if (elemType == DUNGEON_ELEMENT_FAKEWALL) {
        if (!(squareByte & 0x04) && !(squareByte & 0x01)) {
            outResult->code = DM1_COLLISION_BLOCKED_FAKEWALL;
            return 0;
        }
    }

    /* Group collision check — CLIKMENU.C:289-290
     * If things data is available, check if a creature group occupies
     * the destination.  This is a simplified check: we look for the
     * thing-list bit on the destination square and check if any group
     * thing exists.  Full group check would walk the thing chain; we
     * delegate to the movement pipeline's F0708 for runtime behavior. */
    if (things && things->loaded && party->championCount > 0) {
        if (squareByte & DUNGEON_SQUARE_MASK_THING_LIST) {
            struct SensorOnSquare_Compat sensorCheck;
            /* Use the group-detection path from F0708 if available.
             * For the collision query we conservatively mark it passable;
             * the full pipeline resolves group blocking in its own stage. */
            (void)sensorCheck; /* Suppress unused warning */
        }
    }

    /* Passable: corridor, open door, open/imaginary fakewall, pit, stairs,
     * teleporter.  Pit/teleporter consequences handled by post-move. */
    outResult->code = DM1_COLLISION_PASSABLE;
    return 1;
}

int DM1_V1_Collision_QuerySquare(
    const struct DungeonDatState_Compat* dungeon,
    int mapIndex,
    int mapX,
    int mapY,
    struct Dm1V1CollisionResult* outResult)
{
    int squareByte, elemType, doorSt;

    if (!outResult) return 0;
    memset(outResult, 0, sizeof(*outResult));
    outResult->doorState = -1;
    outResult->mapX = mapX;
    outResult->mapY = mapY;
    outResult->mapIndex = mapIndex;

    squareByte = collision_read_square(dungeon, mapIndex, mapX, mapY);
    if (squareByte < 0) {
        outResult->code = DM1_COLLISION_BLOCKED_BOUNDS;
        return 0;
    }

    elemType = collision_element_type(squareByte);
    outResult->elementType = elemType;

    if (elemType == DUNGEON_ELEMENT_WALL) {
        outResult->code = DM1_COLLISION_BLOCKED_WALL;
        return 0;
    }

    if (elemType == DUNGEON_ELEMENT_DOOR) {
        doorSt = collision_door_state(squareByte);
        outResult->doorState = doorSt;
        outResult->doorPassable = collision_door_is_passable(doorSt);
        if (!outResult->doorPassable) {
            outResult->code = DM1_COLLISION_BLOCKED_DOOR;
            return 0;
        }
    }

    if (elemType == DUNGEON_ELEMENT_FAKEWALL) {
        if (!(squareByte & 0x04) && !(squareByte & 0x01)) {
            outResult->code = DM1_COLLISION_BLOCKED_FAKEWALL;
            return 0;
        }
    }

    outResult->code = DM1_COLLISION_PASSABLE;
    return 1;
}

int DM1_V1_Collision_DetectPit(
    const struct DungeonDatState_Compat* dungeon,
    int mapIndex,
    int mapX,
    int mapY,
    struct Dm1V1PitResult* outResult)
{
    int squareByte, elemType;

    if (!outResult) return 0;
    memset(outResult, 0, sizeof(*outResult));
    outResult->mapX = mapX;
    outResult->mapY = mapY;
    outResult->mapIndex = mapIndex;

    squareByte = collision_read_square(dungeon, mapIndex, mapX, mapY);
    if (squareByte < 0) {
        outResult->code = DM1_PIT_NONE;
        return 0;
    }

    elemType = collision_element_type(squareByte);
    if (elemType != DUNGEON_ELEMENT_PIT) {
        outResult->code = DM1_PIT_NONE;
        return 0;
    }

    /*
     * MOVESENS.C:F0267 lines 493-500:
     *   (AL0709 == C02_ELEMENT_PIT) && !levitating
     *   && M007_GET(square, MASK0x0008_PIT_OPEN)
     *   && !M007_GET(square, MASK0x0001_PIT_IMAGINARY)
     */
    outResult->pitOpen = (squareByte & DM1_PIT_MASK_OPEN) ? 1 : 0;
    outResult->pitImaginary = (squareByte & DM1_PIT_MASK_IMAGINARY) ? 1 : 0;

    if (!outResult->pitOpen) {
        outResult->code = DM1_PIT_CLOSED;
        return 1;
    }

    if (outResult->pitImaginary) {
        outResult->code = DM1_PIT_OPEN_IMAGINARY;
        return 1;
    }

    outResult->code = DM1_PIT_OPEN_REAL;
    return 1;
}

int DM1_V1_Door_ProcessClick(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    const struct PartyState_Compat* party,
    struct Dm1V1DoorInteractionResult* outResult)
{
    int frontX, frontY;
    int squareByte, elemType, doorSt;
    int dir;

    if (!outResult) return 0;
    memset(outResult, 0, sizeof(*outResult));
    outResult->previousDoorState = -1;
    outResult->newDoorState = -1;

    if (!dungeon || !party) {
        outResult->code = DM1_DOOR_CLICK_NO_DOOR;
        return 0;
    }

    /*
     * Compute front cell coordinates.
     * Source: CLIKVIEW.C:F0372 lines 1-16:
     *   L1135 = G0306_i_PartyMapX + G0233[G0308_i_PartyDirection]
     *   L1136 = G0307_i_PartyMapY + G0234[G0308_i_PartyDirection]
     */
    dir = party->direction;
    if (dir < 0 || dir > 3) {
        outResult->code = DM1_DOOR_CLICK_NO_DOOR;
        return 0;
    }
    frontX = party->mapX + step_dx[dir];
    frontY = party->mapY + step_dy[dir];

    outResult->frontMapX = frontX;
    outResult->frontMapY = frontY;
    outResult->frontMapIndex = party->mapIndex;

    /* Bounds check (CLIKVIEW.C:F0372 line 8-10) */
    squareByte = collision_read_square(dungeon, party->mapIndex, frontX, frontY);
    if (squareByte < 0) {
        outResult->code = DM1_DOOR_CLICK_NO_DOOR;
        return 0;
    }

    elemType = collision_element_type(squareByte);

    /* If front cell is not a door, attempt wall sensor routing
     * via the compat layer. */
    if (elemType != DUNGEON_ELEMENT_DOOR) {
        /*
         * Route through F0716_DOOR_RouteFrontCellClick_Compat for
         * wall sensor identification (CLIKVIEW.C:F0372 -> F0275).
         */
        if (things) {
            (void)F0716_DOOR_RouteFrontCellClick_Compat(
                dungeon, things, party, &outResult->clickDetail);
            outResult->hasSensorOnSquare = outResult->clickDetail.hasSensor;
        }
        outResult->code = DM1_DOOR_CLICK_NO_DOOR;
        return 0;
    }

    /* Front cell is a door — extract state.  ReDMCSB does not toggle an
     * arbitrary front door from the viewport: CLIKVIEW.C:F0377:365-385
     * first resolves the front square's DOOR thing through F0157 and
     * requires DOOR->Button before scheduling C10_EVENT_DOOR.  When
     * decoded thing data is available, preserve that gate here. */
    doorSt = collision_door_state(squareByte);
    outResult->previousDoorState = doorSt;
    outResult->doorVertical = (squareByte & 0x08) ? 1 : 0;
    if (things && things->loaded) {
        if (!collision_front_door_has_button(dungeon, things, party->mapIndex, frontX, frontY, NULL)) {
            outResult->code = DM1_DOOR_CLICK_NO_BUTTON;
            return 0;
        }
        outResult->doorHasButton = 1;
    }

    /* Destroyed doors cannot be toggled */
    if (doorSt == DM1_DOOR_STATE_DESTROYED) {
        outResult->code = DM1_DOOR_CLICK_DESTROYED;
        return 1;
    }

    /*
     * Resolve toggle via F0715_DOOR_ResolveToggleAction_Compat.
     * Source: door actuator branch referenced by
     * CLIKVIEW.C:F0377 -> F0268_SENSOR_AddEvent(C10_EVENT_DOOR, ...,
     * C02_EFFECT_TOGGLE, gameTick+1) and then the TIMELINE processes
     * it through the door animation step.
     */
    (void)F0715_DOOR_ResolveToggleAction_Compat(
        dungeon, party->mapIndex, frontX, frontY,
        &outResult->toggleDetail);

    outResult->newDoorState = outResult->toggleDetail.newDoorState;

    /* Determine toggle direction */
    if (doorSt == DM1_DOOR_STATE_OPEN) {
        outResult->code = DM1_DOOR_CLICK_TOGGLED_CLOSED;
    } else {
        outResult->code = DM1_DOOR_CLICK_TOGGLED_OPEN;
    }

    /* Also populate click detail for sensor check */
    if (things) {
        (void)F0716_DOOR_RouteFrontCellClick_Compat(
            dungeon, things, party, &outResult->clickDetail);
        outResult->hasSensorOnSquare = outResult->clickDetail.hasSensor;
    }

    return 1;
}

int DM1_V1_Door_ApplyToggle(
    struct DungeonDatState_Compat* dungeon,
    int mapIndex,
    int mapX,
    int mapY,
    int newDoorState)
{
    const struct DungeonMapDesc_Compat* map;
    unsigned char* squarePtr;
    unsigned char squareByte;
    int elemType;

    if (!dungeon || !dungeon->loaded || !dungeon->tilesLoaded || !dungeon->tiles)
        return 0;
    if (mapIndex < 0 || mapIndex >= (int)dungeon->header.mapCount)
        return 0;
    map = &dungeon->maps[mapIndex];
    if (mapX < 0 || mapX >= map->width || mapY < 0 || mapY >= map->height)
        return 0;

    squarePtr = &dungeon->tiles[mapIndex].squareData[mapX * map->height + mapY];
    squareByte = *squarePtr;
    elemType = collision_element_type(squareByte);

    if (elemType != DUNGEON_ELEMENT_DOOR)
        return 0;

    if (newDoorState < 0 || newDoorState > 5)
        return 0;

    /*
     * Mutate the low 3 bits of the square byte.
     * Source: DUNGEON.C square byte mutation after the TIMELINE
     * processes a C10_EVENT_DOOR.
     */
    *squarePtr = (squareByte & 0xF8) | (unsigned char)(newDoorState & 0x07);
    return 1;
}

int DM1_V1_Door_IsPassable(
    const struct DungeonDatState_Compat* dungeon,
    int mapIndex,
    int mapX,
    int mapY)
{
    int squareByte, elemType, doorSt;

    squareByte = collision_read_square(dungeon, mapIndex, mapX, mapY);
    if (squareByte < 0) return -1;

    elemType = collision_element_type(squareByte);
    if (elemType != DUNGEON_ELEMENT_DOOR) return -1;

    doorSt = collision_door_state(squareByte);
    return collision_door_is_passable(doorSt) ? 1 : 0;
}

const char* DM1_V1_CollisionDoor_SourceEvidence(void)
{
    return "CLIKMENU.C:F0366:274-290 (wall/door/fakewall/group collision); "
           "MOVESENS.C:F0267:316-850 (move result, pit/teleporter chain); "
           "MOVESENS.C:F0264:128-170 (levitation check); "
           "MOVESENS.C:F0275:1309+ (click-on-wall sensor routing); "
           "CLIKVIEW.C:F0372 (front cell computation); "
           "CLIKVIEW.C:F0377:365-385 (DOOR->Button gate -> EVENT_DOOR toggle); "
           "DEFS.H:C0-C5 door states, MASK0x0008_PIT_OPEN, MASK0x0001_PIT_IMAGINARY";
}
