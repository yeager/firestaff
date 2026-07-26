/*
 * DM1 V1 Actuator Execution — pc34 compat layer.
 *
 * Source: ReDMCSB MOVESENS.C F0268_SENSOR_AddEvent lines 1136-1232
 *         TIMELINE.C C024_EVENT_DOOR handler lines ~600-660
 *
 * Consumes SensorActuatorDispatch_Compat from the sensor trigger chain
 * and mutates dungeon square bytes to open/close doors, toggle pits,
 * and flip fakewalls.
 */

#include "dm1_v1_actuator_execution_pc34_compat.h"
#include "dm1_v1_sensor_trigger_pc34_compat.h"
#include "dm1_v1_collision_door_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

static int square_index(int mapWidth, int mapHeight, int mapX, int mapY)
{
    if (mapX < 0 || mapX >= mapWidth || mapY < 0 || mapY >= mapHeight)
        return -1;
    return mapX * mapHeight + mapY;
}

static int execute_door(
    const struct SensorActuatorDispatch_Compat* dispatch,
    struct DungeonMapTiles_Compat* tiles,
    int idx,
    struct Dm1V1ActuatorResult* out)
{
    unsigned char sq = tiles->squareData[idx];
    int oldState = dm1_v1_actuator_read_door_state(sq);
    int newState = oldState;

    switch (dispatch->action) {
    case DM1_ACTUATOR_ACTION_OPEN:
        newState = DM1_DOOR_STATE_OPEN;
        break;
    case DM1_ACTUATOR_ACTION_CLOSE:
        newState = DM1_DOOR_STATE_CLOSED;
        break;
    case DM1_ACTUATOR_ACTION_TOGGLE:
        newState = (oldState == DM1_DOOR_STATE_OPEN)
            ? DM1_DOOR_STATE_CLOSED : DM1_DOOR_STATE_OPEN;
        break;
    default:
        return 0;
    }

    tiles->squareData[idx] = dm1_v1_actuator_write_door_state(sq, newState);

    out->applied = 1;
    out->previousState = oldState;
    out->newState = newState;
    out->animationNeeded = (oldState != newState);
    return 1;
}

static int execute_pit(
    const struct SensorActuatorDispatch_Compat* dispatch,
    struct DungeonMapTiles_Compat* tiles,
    int idx,
    struct Dm1V1ActuatorResult* out)
{
    unsigned char sq = tiles->squareData[idx];
    int oldOpen = dm1_v1_actuator_read_pit_open(sq);
    int newOpen = oldOpen;

    switch (dispatch->action) {
    case DM1_ACTUATOR_ACTION_OPEN:
        newOpen = 1;
        break;
    case DM1_ACTUATOR_ACTION_CLOSE:
        newOpen = 0;
        break;
    case DM1_ACTUATOR_ACTION_TOGGLE:
        newOpen = !oldOpen;
        break;
    default:
        return 0;
    }

    tiles->squareData[idx] = dm1_v1_actuator_write_pit_open(sq, newOpen);

    out->applied = 1;
    out->previousState = oldOpen;
    out->newState = newOpen;
    out->animationNeeded = 0;
    return 1;
}

static int execute_fakewall(
    const struct SensorActuatorDispatch_Compat* dispatch,
    struct DungeonMapTiles_Compat* tiles,
    int idx,
    struct Dm1V1ActuatorResult* out)
{
    unsigned char sq = tiles->squareData[idx];
    int oldElement = dm1_v1_actuator_read_element(sq);
    int newElement = oldElement;

    switch (dispatch->action) {
    case DM1_ACTUATOR_ACTION_OPEN:
        newElement = DUNGEON_ELEMENT_CORRIDOR;
        break;
    case DM1_ACTUATOR_ACTION_CLOSE:
        newElement = DUNGEON_ELEMENT_FAKEWALL;
        break;
    case DM1_ACTUATOR_ACTION_TOGGLE:
        newElement = (oldElement == DUNGEON_ELEMENT_FAKEWALL)
            ? DUNGEON_ELEMENT_CORRIDOR : DUNGEON_ELEMENT_FAKEWALL;
        break;
    default:
        return 0;
    }

    tiles->squareData[idx] = dm1_v1_actuator_write_element(sq, newElement);

    out->applied = 1;
    out->previousState = oldElement;
    out->newState = newElement;
    out->animationNeeded = 0;
    return 1;
}

int dm1_v1_actuator_execute_dispatch_pc34(
    const struct SensorActuatorDispatch_Compat* dispatch,
    struct DungeonMapTiles_Compat* tiles,
    int mapWidth,
    int mapHeight,
    struct Dm1V1ActuatorResult* outResult)
{
    int idx;

    if (!dispatch || !tiles || !tiles->squareData || !outResult)
        return 0;

    if (!dispatch->valid)
        return 0;

    idx = square_index(mapWidth, mapHeight,
                       dispatch->targetMapX, dispatch->targetMapY);
    if (idx < 0 || idx >= tiles->squareCount)
        return 0;

    outResult->applied = 0;
    outResult->previousState = 0;
    outResult->newState = 0;
    outResult->animationNeeded = 0;
    outResult->targetMapX = dispatch->targetMapX;
    outResult->targetMapY = dispatch->targetMapY;

    if (dispatch->actuatorKindMask & DM1_ACTUATOR_KIND_DOOR)
        return execute_door(dispatch, tiles, idx, outResult);

    if (dispatch->actuatorKindMask & DM1_ACTUATOR_KIND_PIT)
        return execute_pit(dispatch, tiles, idx, outResult);

    if (dispatch->actuatorKindMask & DM1_ACTUATOR_KIND_FAKEWALL)
        return execute_fakewall(dispatch, tiles, idx, outResult);

    return 0;
}

int dm1_v1_actuator_door_animation_step_pc34(
    struct DungeonMapTiles_Compat* tiles,
    int mapWidth,
    int mapHeight,
    int mapX,
    int mapY,
    int targetState)
{
    int idx;
    unsigned char sq;
    int currentState;
    int nextState;

    if (!tiles || !tiles->squareData)
        return 0;

    idx = square_index(mapWidth, mapHeight, mapX, mapY);
    if (idx < 0 || idx >= tiles->squareCount)
        return 0;

    sq = tiles->squareData[idx];
    currentState = dm1_v1_actuator_read_door_state(sq);

    if (currentState == targetState)
        return 0;

    if (targetState < currentState)
        nextState = currentState - 1;
    else
        nextState = currentState + 1;

    tiles->squareData[idx] = dm1_v1_actuator_write_door_state(sq, nextState);

    return (nextState != targetState) ? 1 : 0;
}

const char* dm1_v1_actuator_execution_source_evidence_pc34(void)
{
    return "ReDMCSB MOVESENS.C F0268_SENSOR_AddEvent lines 1136-1232; "
           "TIMELINE.C C024_EVENT_DOOR handler; DEFS.H M036_DOOR_STATE "
           "(bits 2:0), MASK0x0008_PIT_OPEN (bit 3), M034_SQUARE_TYPE "
           "(bits 7:5). Actuator dispatch consumes "
           "SensorActuatorDispatch_Compat from sensor trigger chain and "
           "mutates dungeon square bytes.";
}
