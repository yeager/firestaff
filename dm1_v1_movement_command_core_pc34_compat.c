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

static int dm1_v1_compute_step_stamina_cost(const struct ChampionState_Compat* champion)
{
    int maxLoad;

    if (!champion) {
        return 0;
    }
    maxLoad = champion->maxLoad ? (int)champion->maxLoad : 1;
    return (int)(((unsigned long)champion->load * 3ul) / (unsigned long)maxLoad) + 1;
}

static void dm1_v1_apply_pre_step_stamina_cost(
    struct PartyState_Compat* party,
    struct Dm1V1MovementCommandCoreResultPc34Compat* outResult)
{
    int i;

    if (!party || !outResult) {
        return;
    }

    for (i = 0; i < party->championCount && i < CHAMPION_MAX_PARTY; ++i) {
        struct ChampionState_Compat* champion = &party->champions[i];
        int cost;
        int staminaAfter;

        /* Source lock: CLIKMENU.C:237-255 applies F0325 only to living
         * champions before movement-arrow/blocker/stairs resolution.
         */
        if (champion->hp.current == 0) {
            continue;
        }

        cost = dm1_v1_compute_step_stamina_cost(champion);
        staminaAfter = (int)champion->stamina.current - cost;
        outResult->staminaCost[i] = cost;
        outResult->staminaAffectedCount++;

        if (staminaAfter <= 0) {
            int damage = (-staminaAfter) >> 1;
            champion->stamina.current = 0;
            outResult->staminaDamage[i] = damage;
            if (damage > 0) {
                champion->hp.current = (champion->hp.current > damage)
                    ? (unsigned short)(champion->hp.current - damage)
                    : 0;
            }
        } else if (staminaAfter > (int)champion->stamina.maximum) {
            champion->stamina.current = champion->stamina.maximum;
        } else {
            champion->stamina.current = (unsigned short)staminaAfter;
        }
    }
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

static int dm1_v1_process_stair_walk_off_append(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    int mapIndex,
    int mapX,
    int mapY,
    struct SensorEffectList_Compat* outEffects)
{
    struct SensorEffectList_Compat tmp;
    int i;

    if (!outEffects) {
        return 0;
    }
    memset(&tmp, 0, sizeof(tmp));
    if (!F0718_SENSOR_ProcessPartyEnterLeave_Compat(
            dungeon, things, mapIndex, mapX, mapY,
            SENSOR_EVENT_WALK_OFF, &tmp)) {
        return 0;
    }
    for (i = 0; i < tmp.count && outEffects->count < SENSOR_EFFECT_LIST_MAX_COUNT; ++i) {
        outEffects->effects[outEffects->count++] = tmp.effects[i];
    }
    return 1;
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
         * square via F0364_COMMAND_TakeStairs and returns before F0284 turn
         * rotation.  F0364 still calls MOVESENS.C:F0267 with the current
         * stairs square as source (CLIKMENU.C:135), so the stairs walk-off
         * sensor pass is preserved without a same-square walk-on pass.
         */
        if (F0705_MOVEMENT_ResolveStairsTransition_Compat(dungeon, party, &stairs) && stairs.transitioned) {
            if (dm1_v1_process_stair_walk_off_append(
                    dungeon, things, party->mapIndex, party->mapX, party->mapY,
                    &outResult->leaveEffects)) {
                outResult->stairSourceLeaveProcessed = 1;
            }
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
    dm1_v1_apply_pre_step_stamina_cost(party, outResult);
    action = dm1_v1_command_to_move_action(outResult->queue.command);

    /* Source lock: CLIKMENU.C:264-267 consumes MOVE_BACKWARD while already
     * standing on stairs by taking those stairs immediately, before the
     * relative coordinate step, blocker checks, or cooldown assignment.
     */
    if (action == MOVE_BACKWARD) {
        struct StairsTransitionResult_Compat stairs;
        if (F0705_MOVEMENT_ResolveStairsTransition_Compat(dungeon, party, &stairs) && stairs.transitioned) {
            if (dm1_v1_process_stair_walk_off_append(
                    dungeon, things, party->mapIndex, party->mapX, party->mapY,
                    &outResult->leaveEffects)) {
                outResult->stairSourceLeaveProcessed = 1;
            }
            dm1_v1_apply_stairs_transition_result(party, &stairs, outResult);
            return 1;
        }
    }

    if (!F0702_MOVEMENT_TryMove_Compat(dungeon, party, action, &outResult->movement)) {
        outResult->movementBlocked = 1;
        outResult->inputDiscardRequested = 1;
        outResult->blockedMovementVblankWaitRequested = 1;
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
            if (dm1_v1_process_stair_walk_off_append(
                    dungeon, things, party->mapIndex, party->mapX, party->mapY,
                    &outResult->leaveEffects)) {
                outResult->stairSourceLeaveProcessed = 1;
            }
            if (dm1_v1_process_stair_walk_off_append(
                    dungeon, things, targetParty.mapIndex, targetParty.mapX, targetParty.mapY,
                    &outResult->leaveEffects)) {
                outResult->stairTargetLeaveProcessed = 1;
            }
            dm1_v1_apply_stairs_transition_result(party, &stairs, outResult);
            return 1;
        }
    }

    if (F0708_MOVEMENT_IsPartyStepBlockedByGroup_Compat(dungeon, things, party, action)) {
        outResult->movementBlocked = 1;
        outResult->blockedByGroup = 1;
        /* Source lock: CLIKMENU.C:312-313 calls F0209_GROUP_ProcessEvents29to41
         * with CM1_EVENT_CREATE_REACTION_EVENT_31_PARTY_IS_ADJACENT when
         * movement is blocked by a group.  This command-core seam records
         * the required reaction request; creature timeline materialization is
         * handled by the creature-AI/timeline layer.
         */
        outResult->groupReactionPartyAdjacentRequested = 1;
        outResult->inputDiscardRequested = 1;
        outResult->blockedMovementVblankWaitRequested = 1;
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
           "CLIKMENU.C:F0364_COMMAND_TakeStairs:135-139 removes party via F0267 then resolves level/direction, CLIKMENU.C:F0365_COMMAND_ProcessTypes1To2_TurnParty:156-173 stop-wait/turn/sensor leave-enter, CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty:176-179 stairs sensor-order comment, CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty:237-255 living-champion stamina decrement before movement resolution, 224-233 arrow deltas, 264-276 stairs special cases, 269-323 relative step/block/discard/group-adjacent reaction/one PC-34 blocked-movement VBlank/keep input wait armed, 325-346 move-result and cooldown; CHAMPION.C:F0325_CHAMPION_DecrementStamina:2025-2048 clamps stamina and damages on underflow; "
           "DUNGEON.C:F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement:1389-1391 applies forward/right deltas; "
           "CHAMPION.C:F0284_CHAMPION_SetPartyDirection:117-130 rotates champion cells/directions and party direction; "
           "MOVESENS.C:F0267_MOVE_GetMoveResult_CPSCE:316-328 signature/source-destination contract, 433-435 projectile-impact precheck, 738-741 move-result globals, 752-783 party-square/scent/last-movement update, 799-818 party walk-off/walk-on sensor processing.";
}
