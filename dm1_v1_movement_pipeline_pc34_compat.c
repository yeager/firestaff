#include "dm1_v1_movement_pipeline_pc34_compat.h"

#include <string.h>

/*
 * DM1 V1 Movement Command Pipeline implementation.
 *
 * Source lock (ReDMCSB WIP20210206, Toolchains/Common/Source):
 *
 * COMMAND.C:2045-2156 F0380_COMMAND_ProcessQueue_CPSC:
 *   Locks the queue, checks if empty (unlock+replay), checks movement-
 *   disabled gate (G0310/G0311), dequeues one command, unlocks+replays,
 *   then dispatches turns to F0365 and steps to F0366.
 *
 * CLIKMENU.C:142-179 F0365_COMMAND_ProcessTypes1To2_TurnParty:
 *   Removes/re-adds party on same square around F0284_CHAMPION_SetPartyDirection.
 *   F0276_SENSOR_ProcessThingAdditionOrRemoval fires walk-off then walk-on
 *   sensors for the turn.
 *
 * CLIKMENU.C:180-347 F0366_COMMAND_ProcessTypes3To6_MoveParty:
 *   Resolves relative step destination via F0150_DUNGEON_GetUnobstructedSquare
 *   helper; rejects walls (M034_SQUARE_TYPE == C00_ELEMENT_WALL), closed
 *   doors (door-state bits 2..4), closed real fake-walls (not OPEN and
 *   not IMAGINARY).  Checks F0175_GROUP_GetThing for group collision.
 *   Calls F0267_MOVE_GetMoveResult_CPSCE for source/destination sensor
 *   mutation + pit/teleporter chain.  Sets movement cooldown G0310 to
 *   max(F0310_CHAMPION_GetMovementTicks) across living champions.
 *
 * MOVESENS.C:F0267_MOVE_GetMoveResult_CPSCE:
 *   The master movement resolver: handles projectile impacts, teleporter
 *   chains (up to 1000/100 iterations), pit falls with level change,
 *   stairs traversal, sensor processing via F0276, scent recording,
 *   and map transition.
 *
 * GAMELOOP.C:150-155:
 *   Decrements G0310_i_DisabledMovementTicks and
 *   G0311_i_ProjectileDisabledMovementTicks independently once per tick.
 */

void DM1_V1_MovementPipeline_InitPc34Compat(
    struct Dm1V1MovementPipelinePc34Compat* pipeline)
{
    if (!pipeline) return;
    memset(pipeline, 0, sizeof(*pipeline));
    DM1_V1_InputCommandQueue_InitPc34Compat(&pipeline->commandQueue);
}

int DM1_V1_MovementPipeline_EnqueueInputPc34Compat(
    struct Dm1V1MovementPipelinePc34Compat* pipeline,
    struct Dm1V1InputEventPc34Compat event)
{
    if (!pipeline) return 0;
    return DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(
        &pipeline->commandQueue, event);
}

int DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(
    struct Dm1V1MovementPipelinePc34Compat* pipeline,
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    struct PartyState_Compat* party,
    const int footwearIcons[CHAMPION_MAX_PARTY],
    struct Dm1V1MovementPipelineResultPc34Compat* outResult)
{
    if (!pipeline || !party || !outResult) return 0;
    memset(outResult, 0, sizeof(*outResult));

    /* Phase 1: Dequeue one command and run the movement command core.
     *
     * This mirrors COMMAND.C F0380: lock queue, check movement gates,
     * dequeue, dispatch to F0365 (turn) or F0366 (step).
     *
     * The movement command core internally calls:
     *   - F0700_MOVEMENT_TurnDirection_Compat (turns)
     *   - F0702_MOVEMENT_TryMove_Compat (step validation)
     *   - F0708_MOVEMENT_IsPartyStepBlockedByGroup_Compat (group gate)
     *   - F0718_SENSOR_ProcessPartyEnterLeave_Compat (walk-off/walk-on)
     *   - DM1_V1_MovementTiming_ApplySuccessfulStepPc34Compat (cooldown)
     */
    (void)DM1_V1_MovementCommandCore_ProcessOnePc34Compat(
        &pipeline->commandQueue,
        dungeon,
        things,
        party,
        pipeline->disabledMovementTicks,
        pipeline->projectileDisabledMovementTicks,
        pipeline->lastProjectileDisabledMovementDirection,
        pipeline->gameTick,
        pipeline->lastPartyMovementTime,
        footwearIcons,
        &outResult->core);

    /* Phase 2: Post-move environment resolution.
     *
     * If a step was accepted, resolve pit falls and teleporter chains.
     * This mirrors the F0267_MOVE_GetMoveResult_CPSCE loop in MOVESENS.C
     * which chains up to 100/1000 consecutive pits and teleporters.
     *
     * F0704_MOVEMENT_ResolvePostMoveEnvironment_Compat handles:
     *   - Open pit detection (MASK0x0008_PIT_OPEN && !MASK0x0001_PIT_IMAGINARY)
     *   - Level change via F0154_DUNGEON_GetLocationAfterLevelChange semantics
     *   - Teleporter scope/rotation/target resolution
     *   - Champion fall damage (20 HP per pit, matching MOVESENS.C)
     */
    if (outResult->core.stepApplied && dungeon) {
        (void)F0704_MOVEMENT_ResolvePostMoveEnvironment_Compat(
            dungeon, things, party, (uint32_t)pipeline->gameTick,
            &outResult->postMove);
        outResult->postMoveResolved = 1;

        if (outResult->postMove.transitioned) {
            party->mapX = outResult->postMove.finalMapX;
            party->mapY = outResult->postMove.finalMapY;
            party->direction = outResult->postMove.finalDirection;
            party->mapIndex = outResult->postMove.finalMapIndex;
        }
    }

    /* Phase 3: Update timing state.
     *
     * CLIKMENU.C:330-346 sets G0310_i_DisabledMovementTicks after a
     * successful step.  Turns do not set cooldowns.
     */
    if (outResult->core.stepApplied) {
        outResult->newDisabledMovementTicks =
            outResult->core.timing.disabledMovementTicks;
        outResult->newProjectileDisabledMovementTicks =
            outResult->core.timing.projectileDisabledMovementTicks;
        outResult->newLastPartyMovementTime =
            outResult->core.timing.lastPartyMovementTime;

        pipeline->disabledMovementTicks =
            outResult->newDisabledMovementTicks;
        pipeline->projectileDisabledMovementTicks =
            outResult->newProjectileDisabledMovementTicks;
        pipeline->lastPartyMovementTime =
            outResult->newLastPartyMovementTime;
    } else {
        outResult->newDisabledMovementTicks =
            pipeline->disabledMovementTicks;
        outResult->newProjectileDisabledMovementTicks =
            pipeline->projectileDisabledMovementTicks;
        outResult->newLastPartyMovementTime =
            pipeline->lastPartyMovementTime;
    }

    /* Phase 4: Aggregate flags for the game loop. */
    outResult->anyMovementOccurred = outResult->core.stepApplied;
    outResult->anyTurnOccurred = outResult->core.turnApplied;
    outResult->viewportDirty = outResult->core.viewportRedrawRequested;

    pipeline->gameTick++;
    return 1;
}

void DM1_V1_MovementPipeline_DecrementCooldownsPc34Compat(
    struct Dm1V1MovementPipelinePc34Compat* pipeline)
{
    if (!pipeline) return;
    /* Source: GAMELOOP.C:150-155 decrements both cooldowns independently. */
    DM1_V1_MovementTiming_DecrementCooldownsPc34Compat(
        &pipeline->disabledMovementTicks,
        &pipeline->projectileDisabledMovementTicks);
}

const char* DM1_V1_MovementPipeline_SourceEvidencePc34Compat(void)
{
    return "COMMAND.C:2045-2156; CLIKMENU.C:142-179,180-347,330-346; "
           "MOVESENS.C:F0267,F0276; CHAMPION.C:F0310; GAMELOOP.C:150-155; "
           "Pipeline wires: dm1_v1_input_command_queue, "
           "dm1_v1_movement_command_core, dm1_v1_movement_timing, "
           "memory_movement (F0700-F0709), memory_sensor_execution (F0710-F0718)";
}
