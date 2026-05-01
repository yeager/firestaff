#include "dm1_v1_movement_command_core_pc34_compat.h"

#include <string.h>

static int dm1_v1_command_to_move_action(int command)
{
    return command - DM1_V1_COMMAND_MOVE_FORWARD;
}

static int dm1_v1_is_turn_command(int command)
{
    return command == DM1_V1_COMMAND_TURN_LEFT || command == DM1_V1_COMMAND_TURN_RIGHT;
}

static int dm1_v1_is_step_command(int command)
{
    return command >= DM1_V1_COMMAND_MOVE_FORWARD && command <= DM1_V1_COMMAND_MOVE_LEFT;
}

int DM1_V1_MovementCommandCore_ProcessOnePc34Compat(
    struct Dm1V1InputCommandQueuePc34Compat* queue,
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    struct PartyState_Compat* party,
    int disabledMovementTicks,
    int projectileDisabledMovementTicks,
    int lastProjectileDisabledMovementDirection,
    unsigned long currentGameTick,
    unsigned long previousLastPartyMovementTime,
    const int footwearIcons[CHAMPION_MAX_PARTY],
    struct Dm1V1MovementCommandCoreResultPc34Compat* outResult)
{
    int action;

    if (!queue || !party || !outResult) {
        return 0;
    }
    memset(outResult, 0, sizeof(*outResult));
    outResult->sourceMapIndex = party->mapIndex;
    outResult->sourceMapX = party->mapX;
    outResult->sourceMapY = party->mapY;
    outResult->sourceDirection = party->direction;

    outResult->queue = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(
        queue,
        party->direction,
        disabledMovementTicks,
        projectileDisabledMovementTicks,
        lastProjectileDisabledMovementDirection);

    if (!outResult->queue.dequeued) {
        return 1;
    }

    if (dm1_v1_is_turn_command(outResult->queue.command)) {
        outResult->commandHandled = 1;
        (void)F0718_SENSOR_ProcessPartyEnterLeave_Compat(
            dungeon, things, party->mapIndex, party->mapX, party->mapY,
            SENSOR_EVENT_WALK_OFF, &outResult->leaveEffects);
        party->direction = F0700_MOVEMENT_TurnDirection_Compat(
            party->direction,
            outResult->queue.command == DM1_V1_COMMAND_TURN_RIGHT);
        (void)F0718_SENSOR_ProcessPartyEnterLeave_Compat(
            dungeon, things, party->mapIndex, party->mapX, party->mapY,
            SENSOR_EVENT_WALK_ON, &outResult->enterEffects);
        outResult->movement.resultCode = MOVE_TURN_ONLY;
        outResult->movement.newMapX = party->mapX;
        outResult->movement.newMapY = party->mapY;
        outResult->movement.newDirection = party->direction;
        outResult->movement.newMapIndex = party->mapIndex;
        outResult->turnApplied = 1;
        outResult->stopWaitingForPlayerInput = 1;
        outResult->viewportRedrawRequested = 1;
        return 1;
    }

    if (!dm1_v1_is_step_command(outResult->queue.command)) {
        return 1;
    }

    outResult->commandHandled = 1;
    action = dm1_v1_command_to_move_action(outResult->queue.command);
    if (!F0702_MOVEMENT_TryMove_Compat(dungeon, party, action, &outResult->movement)) {
        outResult->movementBlocked = 1;
        outResult->inputDiscardRequested = 1;
        return 1;
    }
    if (F0708_MOVEMENT_IsPartyStepBlockedByGroup_Compat(dungeon, things, party, action)) {
        outResult->movementBlocked = 1;
        outResult->blockedByGroup = 1;
        outResult->inputDiscardRequested = 1;
        return 1;
    }

    (void)F0718_SENSOR_ProcessPartyEnterLeave_Compat(
        dungeon, things, party->mapIndex, party->mapX, party->mapY,
        SENSOR_EVENT_WALK_OFF, &outResult->leaveEffects);

    party->mapIndex = outResult->movement.newMapIndex;
    party->mapX = outResult->movement.newMapX;
    party->mapY = outResult->movement.newMapY;
    party->direction = outResult->movement.newDirection;

    (void)F0718_SENSOR_ProcessPartyEnterLeave_Compat(
        dungeon, things, party->mapIndex, party->mapX, party->mapY,
        SENSOR_EVENT_WALK_ON, &outResult->enterEffects);

    outResult->timing = DM1_V1_MovementTiming_ApplySuccessfulStepPc34Compat(
        party,
        outResult->sourceMapIndex,
        outResult->sourceMapX,
        outResult->sourceMapY,
        currentGameTick,
        previousLastPartyMovementTime,
        footwearIcons);
    outResult->stepApplied = 1;
    outResult->stopWaitingForPlayerInput = 1;
    outResult->viewportRedrawRequested = 1;
    return 1;
}

const char* DM1_V1_MovementCommandCore_SourceEvidencePc34Compat(void)
{
    return "COMMAND.C:2045-2156; CLIKMENU.C:142-179,180-347; MOVESENS.C:752-783,1553-1794; GAMELOOP.C:90,215-219; DRAWVIEW.C:709-724";
}
