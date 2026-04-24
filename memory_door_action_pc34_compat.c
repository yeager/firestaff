#include "memory_door_action_pc34_compat.h"

#include <string.h>

#include "memory_timeline_pc34_compat.h"

/*
 * Door animation owner (Pass 38) — helpers shared below.
 */
static int door_action_write_square_state(
    struct DungeonDatState_Compat* dungeon,
    int mapIndex,
    int mapX,
    int mapY,
    int newState);

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

/*
 * F0712 / F0713 / F0714 — Pass 38 animation owners.
 *
 * These three functions together move animating door states (1..3) out
 * of the "snapped to 0/4" Pass-31 model into a source-faithful walk
 * driven by the timeline queue.  They model the core step/reschedule
 * loop in F0241_TIMELINE_ProcessEvent1_DoorAnimation; hazard branches
 * (champion damage from a closing door, creature damage/kills) are
 * explicitly out of Pass 38 scope and are not implemented here.
 */

static int door_action_read_state_and_vertical(
    const struct DungeonDatState_Compat* dungeon,
    int mapIndex,
    int mapX,
    int mapY,
    int* outState,
    int* outVertical)
{
    unsigned char squareByte = 0;
    int elementType;
    if (!door_action_read_square(dungeon, mapIndex, mapX, mapY, &squareByte)) {
        return 0;
    }
    elementType = (squareByte & DUNGEON_SQUARE_MASK_TYPE) >> 5;
    if (elementType != DUNGEON_ELEMENT_DOOR) {
        return 0;
    }
    if (outState)    *outState    = squareByte & 0x07;
    if (outVertical) *outVertical = (squareByte & 0x08) ? 1 : 0;
    return 1;
}

static int door_action_write_square_state(
    struct DungeonDatState_Compat* dungeon,
    int mapIndex,
    int mapX,
    int mapY,
    int newState)
{
    const struct DungeonMapDesc_Compat* map;
    unsigned char* squarePtr;
    int index;
    if (!dungeon || !dungeon->tilesLoaded || !dungeon->tiles) return 0;
    if (mapIndex < 0 || mapIndex >= (int)dungeon->header.mapCount) return 0;
    map = &dungeon->maps[mapIndex];
    if (mapX < 0 || mapX >= map->width || mapY < 0 || mapY >= map->height) return 0;
    if (!dungeon->tiles[mapIndex].squareData) return 0;
    index = mapX * map->height + mapY;
    squarePtr = &dungeon->tiles[mapIndex].squareData[index];
    *squarePtr = (unsigned char)((*squarePtr & ~0x07U) | (unsigned char)(newState & 0x07));
    return 1;
}

int F0714_DOOR_ResolveAnimationEffect_Compat(
    const struct DungeonDatState_Compat* dungeon,
    int mapIndex,
    int mapX,
    int mapY,
    int requestedEffect,
    int* outEffect,
    int* outCurrentState)
{
    int state = -1;
    int vertical = 0;
    int effect;

    if (!outEffect) return 0;
    if (!door_action_read_state_and_vertical(dungeon, mapIndex, mapX, mapY,
                                             &state, &vertical)) {
        return 0;
    }
    if (outCurrentState) *outCurrentState = state;

    /* Destroyed doors do not animate. */
    if (state == 5 /* DESTROYED */) {
        return 0;
    }

    if (requestedEffect == DOOR_EFFECT_TOGGLE) {
        /* Mirror F0244: an open door gets CLEAR (closing), else SET (opening).
         * An animating-intermediate state therefore snaps direction to "keep
         * opening" (SET) on a toggle click; this matches the ReDMCSB rule
         * that the current state only becomes relevant when evaluating
         * "already at target". */
        effect = (state == 0 /* OPEN */) ? DOOR_EFFECT_CLEAR : DOOR_EFFECT_SET;
    } else if (requestedEffect == DOOR_EFFECT_SET) {
        if (state == 0 /* OPEN */) {
            /* Already at target — no animation needed. */
            *outEffect = DOOR_EFFECT_SET;
            return 0;
        }
        effect = DOOR_EFFECT_SET;
    } else if (requestedEffect == DOOR_EFFECT_CLEAR) {
        if (state == 4 /* CLOSED */) {
            *outEffect = DOOR_EFFECT_CLEAR;
            return 0;
        }
        effect = DOOR_EFFECT_CLEAR;
    } else {
        return 0;
    }

    *outEffect = effect;
    return 1;
}

int F0713_DOOR_BuildAnimationEvent_Compat(
    int mapIndex,
    int mapX,
    int mapY,
    int effect,
    uint32_t startTick,
    struct TimelineEvent_Compat* outEvent)
{
    if (!outEvent) return 0;
    if (effect != DOOR_EFFECT_SET && effect != DOOR_EFFECT_CLEAR) return 0;
    if (mapX < 0 || mapY < 0 || mapIndex < 0) return 0;
    memset(outEvent, 0, sizeof(*outEvent));
    outEvent->kind       = TIMELINE_EVENT_DOOR_ANIMATE;
    /* fireAtTick = startTick — the first animation step runs on the
     * same tick the toggle was requested; chained steps are rescheduled
     * by F0887 at gameTick+1. */
    outEvent->fireAtTick = startTick;
    outEvent->mapIndex   = mapIndex;
    outEvent->mapX       = mapX;
    outEvent->mapY       = mapY;
    outEvent->cell       = 0;
    outEvent->aux0       = -1;      /* first step — dispatcher reads state from square */
    outEvent->aux1       = effect;  /* SET or CLEAR */
    outEvent->aux2       = 0;
    outEvent->aux3       = 0;
    outEvent->aux4       = 0;
    return 1;
}

int F0712_DOOR_StepAnimation_Compat(
    struct DungeonDatState_Compat* dungeon,
    int mapIndex,
    int mapX,
    int mapY,
    int effect,
    int mutateSquare,
    struct DoorAnimationStep_Compat* outStep)
{
    int state = -1;
    int vertical = 0;
    int newState;

    if (!outStep) return 0;
    memset(outStep, 0, sizeof(*outStep));
    outStep->kind         = DOOR_ANIM_STEP_NO_CHANGE;
    outStep->mapIndex     = mapIndex;
    outStep->mapX         = mapX;
    outStep->mapY         = mapY;
    outStep->effect       = effect;
    outStep->oldDoorState = -1;
    outStep->newDoorState = -1;
    outStep->doorVertical = 0;

    if (effect != DOOR_EFFECT_SET && effect != DOOR_EFFECT_CLEAR) return 0;
    if (!door_action_read_state_and_vertical(dungeon, mapIndex, mapX, mapY,
                                             &state, &vertical)) {
        return 0;
    }
    outStep->oldDoorState = state;
    outStep->doorVertical = vertical;

    /* DESTROYED never animates. */
    if (state == 5 /* DESTROYED */) {
        outStep->kind         = DOOR_ANIM_STEP_NO_CHANGE;
        outStep->newDoorState = state;
        return 1;
    }

    if (effect == DOOR_EFFECT_SET) {
        /* Opening: walk 4 -> 3 -> 2 -> 1 -> 0.  Mirror of
         * L0596_i_DoorState += -1 in F0241.
         */
        if (state == 0 /* OPEN: already at target */) {
            outStep->kind         = DOOR_ANIM_STEP_REACHED_TARGET;
            outStep->newDoorState = 0;
            return 1;
        }
        newState = state - 1;
        if (newState < 0) newState = 0;
        outStep->newDoorState = newState;
        outStep->kind = (newState == 0 /* OPEN */)
                          ? DOOR_ANIM_STEP_REACHED_TARGET
                          : DOOR_ANIM_STEP_ADVANCED;
    } else {
        /* Closing: walk 0 -> 1 -> 2 -> 3 -> 4.  Mirror of
         * L0596_i_DoorState += +1 in F0241.
         */
        if (state == 4 /* CLOSED: already at target */) {
            outStep->kind         = DOOR_ANIM_STEP_REACHED_TARGET;
            outStep->newDoorState = 4;
            return 1;
        }
        newState = state + 1;
        if (newState > 4) newState = 4;
        outStep->newDoorState = newState;
        outStep->kind = (newState == 4 /* CLOSED */)
                          ? DOOR_ANIM_STEP_REACHED_TARGET
                          : DOOR_ANIM_STEP_ADVANCED;
    }

    if (mutateSquare &&
        (outStep->kind == DOOR_ANIM_STEP_ADVANCED ||
         outStep->kind == DOOR_ANIM_STEP_REACHED_TARGET)) {
        (void)door_action_write_square_state(dungeon, mapIndex, mapX, mapY,
                                             outStep->newDoorState);
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
