#include "dm1_v1_movement_command_core_pc34_compat.h"
#include "dm1_v1_action_xp_graphic560_pc34_compat.h"
#include "dm1_v1_dungeon_stairs_pc34_compat.h"
#include "memory_mov05_f0284_cell_rotation_pc34_compat.h"

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

static int dm1_v1_turn_target_direction(int currentDirection, int command)
{
    if (command == DM1_V1_COMMAND_TURN_RIGHT) {
        return (currentDirection + 1) & 3;
    }
    if (command == DM1_V1_COMMAND_TURN_LEFT) {
        return (currentDirection + 3) & 3;
    }
    return currentDirection & 3;
}

static void dm1_v1_apply_party_turn_receipt(
    struct PartyState_Compat* party,
    int command,
    struct Dm1V1MovementTurnReceiptPc34Compat* outReceipt)
{
    int oldDirection;
    int newDirection;
    int delta;

    if (!party || !outReceipt) {
        return;
    }

    oldDirection = party->direction & 3;
    newDirection = dm1_v1_turn_target_direction(oldDirection, command);
    delta = newDirection - oldDirection;
    if (delta < 0) {
        delta += 4;
    }

    memset(outReceipt, 0, sizeof(*outReceipt));
    outReceipt->applied = 1;
    outReceipt->command = command;
    outReceipt->oldDirection = oldDirection;
    outReceipt->newDirection = newDirection;
    outReceipt->delta = delta;
    outReceipt->quarterTurnSteps = 1;
    outReceipt->animationFrames = 1;
    outReceipt->intermediateFrames = 0;
    outReceipt->stopWaitingForPlayerInput = 1;
    outReceipt->redrawOnNextGameLoop = 1;
    outReceipt->wallBlockCheck = 0;
    outReceipt->highlightLeft = (command == DM1_V1_COMMAND_TURN_LEFT);
    outReceipt->highlightRight = (command == DM1_V1_COMMAND_TURN_RIGHT);

    /* ReDMCSB CHAMPION.C F0284 lines 117-130: F0365 applies a
     * direction delta through F0284, rotating each present champion's
     * Cell and Direction before storing G0308_i_PartyDirection. */
    (void)F0284_CHAMPION_SetPartyDirection_Compat(party, newDirection);
}

static int dm1_v1_normalize_cell(int cell)
{
    return cell & 3;
}

int DM1_V1_MovementCommandCore_BlockedResolutionPlanPc34Compat(
    const struct PartyState_Compat* party,
    int movementArrowIndex,
    int movementResultCode,
    int blockedByGroup,
    struct Dm1V1MovementBlockedResolutionPlanPc34Compat* outPlan)
{
    int firstCell;

    if (!outPlan) {
        return 0;
    }
    memset(outPlan, 0, sizeof(*outPlan));
    outPlan->valid = 1;
    outPlan->movementBlocked = 1;
    outPlan->inputDiscardRequested = 1;
    outPlan->blockedMovementVblankWaitRequested = 1;

    if (blockedByGroup) {
        /* ReDMCSB CLIKMENU.C F0366 lines 312-313 requests a party-adjacent
         * group reaction before discarding input and waiting one VBlank. */
        outPlan->blockedByGroup = 1;
        outPlan->groupReactionPartyAdjacentRequested = 1;
        return 1;
    }

    if (party && party->championCount > 0 &&
        (movementResultCode == MOVE_BLOCKED_WALL ||
         movementResultCode == MOVE_BLOCKED_DOOR)) {
        /* ReDMCSB CLIKMENU.C F0366 lines 285-299: wall/closed-door/closed
         * fake-wall blocks request F0321 self damage with attack=1,
         * torso|legs wounds, and the two leading cells. */
        firstCell = dm1_v1_normalize_cell(
            movementArrowIndex + party->direction + 2);
        outPlan->blockedByWallOrDoorDamageRequested = 1;
        outPlan->blockedByWallOrDoorDamageAttack = 1;
        outPlan->blockedByWallOrDoorDamageAttackTypeSelf = 2;
        outPlan->blockedByWallOrDoorDamageAllowedWounds = 0x0018u;
        outPlan->blockedByWallOrDoorDamageFirstCell = firstCell;
        outPlan->blockedByWallOrDoorDamageSecondCell =
            dm1_v1_normalize_cell(firstCell + 1);
    }
    return 1;
}

static void dm1_v1_apply_blocked_resolution_plan(
    struct Dm1V1MovementCommandCoreResultPc34Compat* outResult,
    const struct Dm1V1MovementBlockedResolutionPlanPc34Compat* plan)
{
    if (!outResult || !plan || !plan->valid) {
        return;
    }
    outResult->movementBlocked = plan->movementBlocked;
    outResult->blockedByGroup = plan->blockedByGroup;
    outResult->blockedByWallOrDoorDamageRequested =
        plan->blockedByWallOrDoorDamageRequested;
    outResult->blockedByWallOrDoorDamageAttack =
        plan->blockedByWallOrDoorDamageAttack;
    outResult->blockedByWallOrDoorDamageAttackTypeSelf =
        plan->blockedByWallOrDoorDamageAttackTypeSelf;
    outResult->blockedByWallOrDoorDamageAllowedWounds =
        plan->blockedByWallOrDoorDamageAllowedWounds;
    outResult->blockedByWallOrDoorDamageFirstCell =
        plan->blockedByWallOrDoorDamageFirstCell;
    outResult->blockedByWallOrDoorDamageSecondCell =
        plan->blockedByWallOrDoorDamageSecondCell;
    outResult->groupReactionPartyAdjacentRequested =
        plan->groupReactionPartyAdjacentRequested;
    outResult->inputDiscardRequested = plan->inputDiscardRequested;
    outResult->blockedMovementVblankWaitRequested =
        plan->blockedMovementVblankWaitRequested;
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

int DM1_V1_MovementCommandCore_PreStepStaminaApplyPlanPc34Compat(
    const struct PartyState_Compat* party,
    struct Dm1V1MovementPreStepStaminaApplyPlanPc34Compat* outPlan)
{
    int i;

    if (!outPlan) {
        return 0;
    }
    memset(outPlan, 0, sizeof(*outPlan));
    outPlan->valid = 1;
    if (!party) {
        return 1;
    }

    for (i = 0; i < party->championCount && i < CHAMPION_MAX_PARTY; ++i) {
        const struct ChampionState_Compat* champion = &party->champions[i];
        DM1_ActionF0325StaminaInputPc34 staminaIn;
        DM1_ActionF0325StaminaPlanPc34 staminaPlan;
        int cost;

        /* Source lock: CLIKMENU.C:237-255 applies F0325 only to living
         * champions before movement-arrow/blocker/stairs resolution.
         */
        if (champion->hp.current == 0) {
            continue;
        }

        cost = dm1_v1_compute_step_stamina_cost(champion);
        memset(&staminaIn, 0, sizeof(staminaIn));
        staminaIn.currentStamina = (int)champion->stamina.current;
        staminaIn.maximumStamina = (int)champion->stamina.maximum;
        staminaIn.currentHealth = (int)champion->hp.current;
        staminaIn.decrement = cost;
        if (!dm1_v1_action_stamina_apply_plan_f0325_pc34(
                &staminaIn, &staminaPlan) ||
            !staminaPlan.valid) {
            continue;
        }

        outPlan->shouldApply[i] = 1;
        outPlan->staminaCost[i] = cost;
        outPlan->staminaAfter[i] = staminaPlan.currentStaminaAfter;
        outPlan->healthAfter[i] = staminaPlan.currentHealthAfter;
        outPlan->staminaDamage[i] = staminaPlan.pendingHealthDamage;
        outPlan->staminaDamageFlash[i] = staminaPlan.shouldDamageFlash;
        outPlan->staminaAppliedAttributeMask[i] =
            staminaPlan.appliedAttributeMask;
        outPlan->affectedCount++;
    }
    return 1;
}

static void dm1_v1_apply_pre_step_stamina_plan(
    struct PartyState_Compat* party,
    const struct Dm1V1MovementPreStepStaminaApplyPlanPc34Compat* plan,
    struct Dm1V1MovementCommandCoreResultPc34Compat* outResult)
{
    int i;

    if (!party || !plan || !plan->valid || !outResult) {
        return;
    }

    for (i = 0; i < CHAMPION_MAX_PARTY; ++i) {
        struct ChampionState_Compat* champion;
        if (!plan->shouldApply[i]) {
            continue;
        }
        champion = &party->champions[i];
        outResult->staminaCost[i] = plan->staminaCost[i];
        outResult->staminaDamage[i] = plan->staminaDamage[i];
        outResult->staminaDamageFlash[i] = plan->staminaDamageFlash[i];
        outResult->staminaAppliedAttributeMask[i] =
            plan->staminaAppliedAttributeMask[i];
        champion->stamina.current = (unsigned short)plan->staminaAfter[i];
        champion->hp.current = (unsigned short)plan->healthAfter[i];
    }
    outResult->staminaAffectedCount = plan->affectedCount;
}

static void dm1_v1_apply_pre_step_stamina_cost(
    struct PartyState_Compat* party,
    struct Dm1V1MovementCommandCoreResultPc34Compat* outResult)
{
    (void)party;
    (void)outResult;
}

static void m11_v1_turning_apply_party_original_presentation_pc34_compat(
    struct PartyState_Compat* party,
    int command,
    struct Dm1V1MovementCommandCoreResultPc34Compat* outResult)
{
    dm1_v1_apply_party_turn_receipt(party, command, &outResult->turning);
}

static int dm1_v1_party_source_square_is_stairs(
    const struct DungeonDatState_Compat* dungeon,
    const struct PartyState_Compat* party)
{
    const struct DungeonMapDesc_Compat* map;
    unsigned char squareByte;

    if (!dungeon || !party || !dungeon->tilesLoaded || !dungeon->tiles) {
        return 0;
    }
    if (party->mapIndex < 0 || party->mapIndex >= (int)dungeon->header.mapCount) {
        return 0;
    }
    map = &dungeon->maps[party->mapIndex];
    if (party->mapX < 0 || party->mapX >= map->width ||
        party->mapY < 0 || party->mapY >= map->height ||
        !dungeon->tiles[party->mapIndex].squareData) {
        return 0;
    }

    squareByte = dungeon->tiles[party->mapIndex].squareData[
        party->mapX * map->height + party->mapY];
    return ((squareByte & DUNGEON_SQUARE_MASK_TYPE) >> 5) == DUNGEON_ELEMENT_STAIRS;
}

int DM1_V1_MovementCommandCore_StairsApplyPlanPc34Compat(
    const struct StairsTransitionResult_Compat* stairs,
    struct Dm1V1MovementStairsApplyPlanPc34Compat* outPlan)
{
    if (!outPlan) {
        return 0;
    }
    memset(outPlan, 0, sizeof(*outPlan));
    if (!stairs || !stairs->transitioned) {
        return 1;
    }

    outPlan->valid = 1;
    outPlan->fromMapIndex = stairs->fromMapIndex;
    outPlan->toMapIndex = stairs->toMapIndex;
    outPlan->newMapX = stairs->newMapX;
    outPlan->newMapY = stairs->newMapY;
    outPlan->newDirection = stairs->newDirection;
    outPlan->movementResultCode = MOVE_OK;
    outPlan->stairTransitionApplied = 1;
    outPlan->stairDestinationEnterDeferred =
        (stairs->toMapIndex != stairs->fromMapIndex) ? 1 : 0;
    outPlan->stopWaitingForPlayerInput = 1;
    outPlan->viewportRedrawRequested = 1;
    return 1;
}

static void dm1_v1_apply_stairs_transition_plan(
    struct PartyState_Compat* party,
    const struct Dm1V1MovementStairsApplyPlanPc34Compat* plan,
    struct Dm1V1MovementCommandCoreResultPc34Compat* outResult)
{
    if (!party || !plan || !plan->valid || !outResult) {
        return;
    }

    party->mapIndex = plan->toMapIndex;
    party->mapX = plan->newMapX;
    party->mapY = plan->newMapY;
    party->direction = plan->newDirection;
    outResult->movement.resultCode = plan->movementResultCode;
    outResult->movement.newMapX = party->mapX;
    outResult->movement.newMapY = party->mapY;
    outResult->movement.newDirection = party->direction;
    outResult->movement.newMapIndex = party->mapIndex;
    outResult->stairTransitionApplied = plan->stairTransitionApplied;
    outResult->stairDestinationEnterDeferred = plan->stairDestinationEnterDeferred;
    outResult->stopWaitingForPlayerInput = plan->stopWaitingForPlayerInput;
    outResult->viewportRedrawRequested = plan->viewportRedrawRequested;
}

int DM1_V1_MovementCommandCore_SuccessfulStepApplyPlanPc34Compat(
    const struct MovementResult_Compat* movement,
    struct Dm1V1MovementSuccessfulStepApplyPlanPc34Compat* outPlan)
{
    if (!outPlan) {
        return 0;
    }
    memset(outPlan, 0, sizeof(*outPlan));
    if (!movement || movement->resultCode != MOVE_OK) {
        return 1;
    }

    outPlan->valid = 1;
    outPlan->newMapIndex = movement->newMapIndex;
    outPlan->newMapX = movement->newMapX;
    outPlan->newMapY = movement->newMapY;
    outPlan->newDirection = movement->newDirection;
    outPlan->movementResultCode = MOVE_OK;
    outPlan->stepApplied = 1;
    outPlan->stopWaitingForPlayerInput = 1;
    outPlan->viewportRedrawRequested = 1;
    return 1;
}

static void dm1_v1_apply_successful_step_plan(
    struct PartyState_Compat* party,
    const struct Dm1V1MovementSuccessfulStepApplyPlanPc34Compat* plan,
    struct Dm1V1MovementCommandCoreResultPc34Compat* outResult)
{
    if (!party || !plan || !plan->valid || !outResult) {
        return;
    }

    party->mapIndex = plan->newMapIndex;
    party->mapX = plan->newMapX;
    party->mapY = plan->newMapY;
    party->direction = plan->newDirection;
    outResult->movement.resultCode = plan->movementResultCode;
    outResult->movement.newMapIndex = party->mapIndex;
    outResult->movement.newMapX = party->mapX;
    outResult->movement.newMapY = party->mapY;
    outResult->movement.newDirection = party->direction;
    outResult->stepApplied = plan->stepApplied;
    outResult->stopWaitingForPlayerInput = plan->stopWaitingForPlayerInput;
    outResult->viewportRedrawRequested = plan->viewportRedrawRequested;
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
        if (dm1_v1_dungeon_resolve_stairs_transition_pc34(dungeon, party, &stairs) && stairs.transitioned) {
            struct Dm1V1MovementStairsApplyPlanPc34Compat stairsPlan;
            if (dm1_v1_process_stair_walk_off_append(
                    dungeon, things, party->mapIndex, party->mapX, party->mapY,
                    &outResult->leaveEffects)) {
                outResult->stairSourceLeaveProcessed = 1;
            }
            (void)DM1_V1_MovementCommandCore_StairsApplyPlanPc34Compat(
                &stairs, &stairsPlan);
            dm1_v1_apply_stairs_transition_plan(party, &stairsPlan, outResult);
            outResult->movement.resultCode = MOVE_TURN_ONLY;
            outResult->turnApplied = 1;
            return 1;
        }

        (void)F0718_SENSOR_ProcessPartyEnterLeave_Compat(
            dungeon, things, party->mapIndex, party->mapX, party->mapY,
            SENSOR_EVENT_WALK_OFF, &outResult->leaveEffects);
        m11_v1_turning_apply_party_original_presentation_pc34_compat(
            party,
            outResult->queue.command,
            outResult);
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
    {
        struct Dm1V1MovementPreStepStaminaApplyPlanPc34Compat staminaPlan;
        (void)DM1_V1_MovementCommandCore_PreStepStaminaApplyPlanPc34Compat(
            party, &staminaPlan);
        dm1_v1_apply_pre_step_stamina_plan(party, &staminaPlan, outResult);
        dm1_v1_apply_pre_step_stamina_cost(party, outResult);
    }
    action = dm1_v1_command_to_move_action(outResult->queue.command);

    /* Source lock: CLIKMENU.C:264-267 consumes MOVE_BACKWARD while already
     * standing on stairs by taking those stairs immediately, before the
     * relative coordinate step, blocker checks, or cooldown assignment.
     */
    if (action == MOVE_BACKWARD) {
        struct StairsTransitionResult_Compat stairs;
        if (dm1_v1_dungeon_resolve_stairs_transition_pc34(dungeon, party, &stairs) && stairs.transitioned) {
            struct Dm1V1MovementStairsApplyPlanPc34Compat stairsPlan;
            if (dm1_v1_process_stair_walk_off_append(
                    dungeon, things, party->mapIndex, party->mapX, party->mapY,
                    &outResult->leaveEffects)) {
                outResult->stairSourceLeaveProcessed = 1;
            }
            (void)DM1_V1_MovementCommandCore_StairsApplyPlanPc34Compat(
                &stairs, &stairsPlan);
            dm1_v1_apply_stairs_transition_plan(party, &stairsPlan, outResult);
            return 1;
        }
    }

    if (!F0702_MOVEMENT_TryMove_Compat(dungeon, party, action, &outResult->movement)) {
        struct Dm1V1MovementBlockedResolutionPlanPc34Compat blockPlan;
        outResult->movementBlocked = 1;
        outResult->inputDiscardRequested = 1;
        outResult->blockedMovementVblankWaitRequested = 1;
        (void)DM1_V1_MovementCommandCore_BlockedResolutionPlanPc34Compat(
            party, action, outResult->movement.resultCode, 0, &blockPlan);
        dm1_v1_apply_blocked_resolution_plan(outResult, &blockPlan);
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
        if (dm1_v1_dungeon_resolve_stairs_transition_pc34(dungeon, &targetParty, &stairs) && stairs.transitioned) {
            struct Dm1V1MovementStairsApplyPlanPc34Compat stairsPlan;
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
            (void)DM1_V1_MovementCommandCore_StairsApplyPlanPc34Compat(
                &stairs, &stairsPlan);
            dm1_v1_apply_stairs_transition_plan(party, &stairsPlan, outResult);
            return 1;
        }
    }

    if (F0708_MOVEMENT_IsPartyStepBlockedByGroup_Compat(dungeon, things, party, action)) {
        struct Dm1V1MovementBlockedResolutionPlanPc34Compat blockPlan;
        (void)DM1_V1_MovementCommandCore_BlockedResolutionPlanPc34Compat(
            party, action, outResult->movement.resultCode, 1, &blockPlan);
        dm1_v1_apply_blocked_resolution_plan(outResult, &blockPlan);
        DM1_V1_InputCommandQueue_DiscardAllInputPc34Compat(queue);
        return 1;
    }

    /* Source lock: CLIKMENU.C:325-328 calls F0267 with a non-square source
     * when the party steps from stairs to a normal square.  That suppresses
     * source-stairs WALK_OFF processing while preserving the destination
     * WALK_ON pass and the normal post-step cooldown path.
     */
    if (dm1_v1_party_source_square_is_stairs(dungeon, party)) {
        outResult->sourceStairsWalkOffSkipped = 1;
    } else {
        (void)F0718_SENSOR_ProcessPartyEnterLeave_Compat(
            dungeon, things, party->mapIndex, party->mapX, party->mapY,
            SENSOR_EVENT_WALK_OFF, &outResult->leaveEffects);
    }

    {
        struct Dm1V1MovementSuccessfulStepApplyPlanPc34Compat stepPlan;
        (void)DM1_V1_MovementCommandCore_SuccessfulStepApplyPlanPc34Compat(
            &outResult->movement, &stepPlan);
        dm1_v1_apply_successful_step_plan(party, &stepPlan, outResult);
    }
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
           "COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC:2075-2099 locks/empty-checks/movement-disabled gate and blocks projectile cooldown only when G0312 matches normalized absolute movement direction, 2118-2127 dequeues, 2150-2156 dispatches turn/move; "
           "CLIKMENU.C:F0364_COMMAND_TakeStairs:135-139 removes party via F0267 then resolves level/direction, CLIKMENU.C:F0365_COMMAND_ProcessTypes1To2_TurnParty:156-173 stop-wait/turn/sensor leave-enter, CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty:176-179 stairs sensor-order comment, CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty:237-255 living-champion stamina decrement before movement resolution, 224-233 arrow deltas, 264-276 stairs special cases, 269-323 relative step/block/self-damage-and-wounds request/discard/group-adjacent reaction/one PC-34 blocked-movement VBlank/keep input wait armed, 325-346 move-result and cooldown; CHAMPION.C:F0325_CHAMPION_DecrementStamina:2025-2048 clamps stamina and damages on underflow; "
           "DUNGEON.C:F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement:1389-1391 applies forward/right deltas; "
           "CHAMPION.C:F0284_CHAMPION_SetPartyDirection:117-130 rotates champion cells/directions and party direction; "
           "MOVESENS.C:F0267_MOVE_GetMoveResult_CPSCE:316-328 signature/source-destination contract, 433-435 projectile-impact precheck, 738-741 move-result globals, 752-783 party-square/scent/last-movement update, 799-822 party walk-off/walk-on processing and cross-map new-party-map deferral.";
}

/* ══════════════════════════════════════════════════════════════════════
 * Pass602 — Remaining CLIKMENU.C function citations for parity
 *
 *   CLIKMENU.C:58 F0006_MAIN_H
 *   CLIKMENU.C:366 F0277_CPSE_I
 * ══════════════════════════════════════════════════════════════════════ */
