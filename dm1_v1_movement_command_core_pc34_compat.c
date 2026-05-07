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

static void dm1_v1_apply_stairs_transition_result(
    struct PartyState_Compat* party,
    const struct StairsTransitionResult_Compat* stairs,
    struct Dm1V1MovementCommandCoreResultPc34Compat* outResult)
{
    party->mapIndex = stairs->toMapIndex;
    party->mapX = stairs->newMapX;
    party->mapY = stairs->newMapY;
    party->direction = stairs->newDirection;
    outResult->movement.resultCode = MOVE_OK;
    outResult->movement.newMapX = party->mapX;
    outResult->movement.newMapY = party->mapY;
    outResult->movement.newDirection = party->direction;
    outResult->movement.newMapIndex = party->mapIndex;
    outResult->stairTransitionApplied = 1;
    outResult->stopWaitingForPlayerInput = 1;
    outResult->viewportRedrawRequested = 1;
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
        struct StairsTransitionResult_Compat stairs;
        outResult->commandHandled = 1;

        /* Source lock: CLIKMENU.C:167-169 consumes a turn command on a stairs
         * square via F0364_COMMAND_TakeStairs and returns before normal
         * current-square sensor leave/enter and F0284 turn rotation.
         */
        if (F0705_MOVEMENT_ResolveStairsTransition_Compat(dungeon, party, &stairs) && stairs.transitioned) {
            dm1_v1_apply_stairs_transition_result(party, &stairs, outResult);
            outResult->movement.resultCode = MOVE_TURN_ONLY;
            outResult->turnApplied = 1;
            return 1;
        }

        (void)F0718_SENSOR_ProcessPartyEnterLeave_Compat(
            dungeon, things, party->mapIndex, party->mapX, party->mapY,
            SENSOR_EVENT_WALK_OFF, &outResult->leaveEffects);
    /* CHAMPION.C:117-130 rotates champion Cell/Direction and stores party direction. */
        outResult->turning = m11_v1_turning_apply_party_original_presentation_pc34_compat(
            M11_V1_TURNING_PRESENTATION_MODE_ORIGINAL,
            outResult->queue.command,
            party);
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

    /* Source lock: CLIKMENU.C:264-267 consumes MOVE_BACKWARD while already
     * standing on stairs by taking those stairs immediately, before the
     * relative coordinate step, blocker checks, or cooldown assignment.
     */
    if (action == MOVE_BACKWARD) {
        struct StairsTransitionResult_Compat stairs;
        if (F0705_MOVEMENT_ResolveStairsTransition_Compat(dungeon, party, &stairs) && stairs.transitioned) {
            dm1_v1_apply_stairs_transition_result(party, &stairs, outResult);
            return 1;
        }
    }

    if (!F0702_MOVEMENT_TryMove_Compat(dungeon, party, action, &outResult->movement)) {
        outResult->movementBlocked = 1;
        outResult->inputDiscardRequested = 1;
        DM1_V1_InputCommandQueue_DiscardAllInputPc34Compat(queue);
        return 1;
    }

    /* Source lock: CLIKMENU.C:271-276 treats a target stairs square as a
     * consequence square: the party is placed on the stairs and F0364 takes
     * the level transition, returning before normal destination walk-on
     * sensors, group collision, or G0310 cooldown assignment.
     */
    {
        struct PartyState_Compat targetParty = *party;
        struct StairsTransitionResult_Compat stairs;
        targetParty.mapIndex = outResult->movement.newMapIndex;
        targetParty.mapX = outResult->movement.newMapX;
        targetParty.mapY = outResult->movement.newMapY;
        targetParty.direction = outResult->movement.newDirection;
        if (F0705_MOVEMENT_ResolveStairsTransition_Compat(dungeon, &targetParty, &stairs) && stairs.transitioned) {
            dm1_v1_apply_stairs_transition_result(party, &stairs, outResult);
            return 1;
        }
    }

    if (F0708_MOVEMENT_IsPartyStepBlockedByGroup_Compat(dungeon, things, party, action)) {
        outResult->movementBlocked = 1;
        outResult->blockedByGroup = 1;
        outResult->inputDiscardRequested = 1;
        DM1_V1_InputCommandQueue_DiscardAllInputPc34Compat(queue);
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
    return "ReDMCSB Toolchains/Common/Source source lock: "
           "COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC:2075-2099 locks/empty-checks/movement-disabled gate, 2118-2127 dequeues, 2150-2156 dispatches turn/move; "
           "CLIKMENU.C:F0364_COMMAND_TakeStairs:135-139 removes party then resolves level/direction, CLIKMENU.C:F0365_COMMAND_ProcessTypes1To2_TurnParty:156-173 stop-wait/turn/sensor leave-enter, CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty:224-233 arrow deltas, 264-276 stairs special cases, 269-323 relative step/block/discard, 325-346 move-result and cooldown; "
           "DUNGEON.C:F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement:1389-1391 applies forward/right deltas; "
           "CHAMPION.C:F0284_CHAMPION_SetPartyDirection:117-130 rotates champion cells/directions and party direction; "
           "MOVESENS.C:F0267_MOVE_GetMoveResult_CPSCE:316-328 signature/source-destination contract, 433-443 projectile-impact precheck and party coordinate write, 738-783 move-result globals and party scent/last-movement update.";
}
