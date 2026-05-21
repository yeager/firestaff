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
 *   Resolves relative step destination via F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement
 *   (DUNGEON.C:1389-1391); rejects walls (M034_SQUARE_TYPE == C00_ELEMENT_WALL), closed
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


static int pipeline_sft_index_for_square(
    const struct DungeonDatState_Compat* dungeon,
    int mapIndex,
    int mapX,
    int mapY)
{
    const struct DungeonMapDesc_Compat* map;
    int squareIdx;
    int total = 0;
    int m;

    if (!dungeon || !dungeon->tilesLoaded || !dungeon->tiles) return -1;
    if (mapIndex < 0 || mapIndex >= (int)dungeon->header.mapCount) return -1;
    map = &dungeon->maps[mapIndex];
    if (mapX < 0 || mapX >= map->width || mapY < 0 || mapY >= map->height) return -1;
    if (!dungeon->tiles[mapIndex].squareData) return -1;
    squareIdx = mapX * map->height + mapY;
    if (!(dungeon->tiles[mapIndex].squareData[squareIdx] & DUNGEON_SQUARE_MASK_THING_LIST)) return -1;

    for (m = 0; m < mapIndex; ++m) {
        int sq;
        int count = dungeon->maps[m].width * dungeon->maps[m].height;
        if (!dungeon->tiles[m].squareData) return -1;
        for (sq = 0; sq < count; ++sq) {
            if (dungeon->tiles[m].squareData[sq] & DUNGEON_SQUARE_MASK_THING_LIST) ++total;
        }
    }
    {
        int sq;
        int count = map->width * map->height;
        for (sq = 0; sq <= squareIdx && sq < count; ++sq) {
            if (dungeon->tiles[mapIndex].squareData[sq] & DUNGEON_SQUARE_MASK_THING_LIST) ++total;
        }
    }
    return total - 1;
}

static unsigned short pipeline_next_decoded_thing(
    const struct DungeonThings_Compat* things,
    unsigned short thingRef)
{
    int type;
    int index;

    if (!things || thingRef == THING_NONE || thingRef == THING_ENDOFLIST) return THING_NONE;
    type = THING_GET_TYPE(thingRef);
    index = THING_GET_INDEX(thingRef);

    switch (type) {
    case THING_TYPE_DOOR:
        return (index >= 0 && index < things->doorCount) ? things->doors[index].next : THING_NONE;
    case THING_TYPE_TELEPORTER:
        return (index >= 0 && index < things->teleporterCount) ? things->teleporters[index].next : THING_NONE;
    case THING_TYPE_TEXTSTRING:
        return (index >= 0 && index < things->textStringCount) ? things->textStrings[index].next : THING_NONE;
    case THING_TYPE_SENSOR:
        return (index >= 0 && index < things->sensorCount) ? things->sensors[index].next : THING_NONE;
    case THING_TYPE_GROUP:
        return (index >= 0 && index < things->groupCount) ? things->groups[index].next : THING_NONE;
    case THING_TYPE_WEAPON:
        return (index >= 0 && index < things->weaponCount) ? things->weapons[index].next : THING_NONE;
    case THING_TYPE_ARMOUR:
        return (index >= 0 && index < things->armourCount) ? things->armours[index].next : THING_NONE;
    case THING_TYPE_SCROLL:
        return (index >= 0 && index < things->scrollCount) ? things->scrolls[index].next : THING_NONE;
    case THING_TYPE_POTION:
        return (index >= 0 && index < things->potionCount) ? things->potions[index].next : THING_NONE;
    case THING_TYPE_CONTAINER:
        return (index >= 0 && index < things->containerCount) ? things->containers[index].next : THING_NONE;
    case THING_TYPE_JUNK:
        return (index >= 0 && index < things->junkCount) ? things->junks[index].next : THING_NONE;
    case THING_TYPE_PROJECTILE:
        return (index >= 0 && index < things->projectileCount) ? things->projectiles[index].next : THING_NONE;
    case THING_TYPE_EXPLOSION:
        return (index >= 0 && index < things->explosionCount) ? things->explosions[index].next : THING_NONE;
    default:
        return THING_NONE;
    }
}


static void pipeline_set_decoded_thing_next(
    struct DungeonThings_Compat* things,
    unsigned short thingRef,
    unsigned short nextThing)
{
    int type;
    int index;

    if (!things || thingRef == THING_NONE || thingRef == THING_ENDOFLIST) return;
    type = THING_GET_TYPE(thingRef);
    index = THING_GET_INDEX(thingRef);

    switch (type) {
    case THING_TYPE_DOOR:
        if (index >= 0 && index < things->doorCount) things->doors[index].next = nextThing;
        break;
    case THING_TYPE_TELEPORTER:
        if (index >= 0 && index < things->teleporterCount) things->teleporters[index].next = nextThing;
        break;
    case THING_TYPE_TEXTSTRING:
        if (index >= 0 && index < things->textStringCount) things->textStrings[index].next = nextThing;
        break;
    case THING_TYPE_SENSOR:
        if (index >= 0 && index < things->sensorCount) things->sensors[index].next = nextThing;
        break;
    case THING_TYPE_GROUP:
        if (index >= 0 && index < things->groupCount) things->groups[index].next = nextThing;
        break;
    case THING_TYPE_WEAPON:
        if (index >= 0 && index < things->weaponCount) things->weapons[index].next = nextThing;
        break;
    case THING_TYPE_ARMOUR:
        if (index >= 0 && index < things->armourCount) things->armours[index].next = nextThing;
        break;
    case THING_TYPE_SCROLL:
        if (index >= 0 && index < things->scrollCount) things->scrolls[index].next = nextThing;
        break;
    case THING_TYPE_POTION:
        if (index >= 0 && index < things->potionCount) things->potions[index].next = nextThing;
        break;
    case THING_TYPE_CONTAINER:
        if (index >= 0 && index < things->containerCount) things->containers[index].next = nextThing;
        break;
    case THING_TYPE_JUNK:
        if (index >= 0 && index < things->junkCount) things->junks[index].next = nextThing;
        break;
    case THING_TYPE_PROJECTILE:
        if (index >= 0 && index < things->projectileCount) things->projectiles[index].next = nextThing;
        break;
    case THING_TYPE_EXPLOSION:
        if (index >= 0 && index < things->explosionCount) things->explosions[index].next = nextThing;
        break;
    default:
        break;
    }
}

static int pipeline_delete_group_on_square_before_enter_sensors(
    const struct DungeonDatState_Compat* dungeon,
    struct DungeonThings_Compat* things,
    int mapIndex,
    int mapX,
    int mapY,
    unsigned short* outDeletedThing)
{
    int sftIdx;
    unsigned short thingRef;
    unsigned short previous = THING_NONE;
    int safety = 0;

    if (outDeletedThing) *outDeletedThing = THING_ENDOFLIST;
    if (!dungeon || !things || !things->loaded || !things->squareFirstThings) return 0;

    sftIdx = pipeline_sft_index_for_square(dungeon, mapIndex, mapX, mapY);
    if (sftIdx < 0 || sftIdx >= things->squareFirstThingCount) return 0;

    thingRef = things->squareFirstThings[sftIdx];
    while (thingRef != THING_NONE && thingRef != THING_ENDOFLIST && safety++ < 64) {
        unsigned short nextThing = pipeline_next_decoded_thing(things, thingRef);
        if (THING_GET_TYPE(thingRef) == THING_TYPE_GROUP) {
            if (previous == THING_NONE) {
                things->squareFirstThings[sftIdx] = nextThing;
            } else {
                pipeline_set_decoded_thing_next(things, previous, nextThing);
            }
            if (outDeletedThing) *outDeletedThing = thingRef;
            return 1;
        }
        previous = thingRef;
        thingRef = nextThing;
    }
    return 0;
}

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

int DM1_V1_MovementPipeline_EnqueueCommandPc34Compat(
    struct Dm1V1MovementPipelinePc34Compat* pipeline,
    int command,
    int x,
    int y)
{
    if (!pipeline) return 0;
    return DM1_V1_InputCommandQueue_EnqueueCommandPc34Compat(
        &pipeline->commandQueue, command, x, y);
}

int DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(
    struct Dm1V1MovementPipelinePc34Compat* pipeline,
    const struct DungeonDatState_Compat* dungeon,
    struct DungeonThings_Compat* things,
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
            int i;
            party->mapX = outResult->postMove.finalMapX;
            party->mapY = outResult->postMove.finalMapY;
            party->direction = outResult->postMove.finalDirection;
            party->mapIndex = outResult->postMove.finalMapIndex;
            for (i = 0; i < CHAMPION_MAX_PARTY; ++i) {
                if (outResult->postMove.championFallDamage[i] > 0 &&
                    party->champions[i].present &&
                    party->champions[i].hp.current > 0) {
                    int hp = (int)party->champions[i].hp.current -
                        outResult->postMove.championFallDamage[i];
                    party->champions[i].hp.current = (int16_t)((hp > 0) ? hp : 0);
                }
            }

            /* Source lock: MOVESENS.C:438-606 mutates the party through
             * teleporter/pit chains before MOVESENS.C:799-818 runs party
             * leave/enter sensors.  MOVESENS.C:810-818 also deletes a
             * group already on the final party square before firing the
             * destination enter sensor.  The command core initially validates and
             * applies the legal adjacent step; when F0267 post-move
             * chaining changes the final party tuple, discard those
             * preliminary adjacent-square sensor effects and replay the
             * source/final pair in original F0267 order.  The full pipeline
             * owns this F0267 post-move ordering and must publish sensors for the
             * final square, not for an intermediate pit/teleporter square.
             */
            memset(&outResult->core.leaveEffects, 0, sizeof(outResult->core.leaveEffects));
            memset(&outResult->core.enterEffects, 0, sizeof(outResult->core.enterEffects));
            (void)F0718_SENSOR_ProcessPartyEnterLeave_Compat(
                dungeon, things, outResult->core.sourceMapIndex,
                outResult->core.sourceMapX, outResult->core.sourceMapY,
                SENSOR_EVENT_WALK_OFF, &outResult->core.leaveEffects);
            outResult->postMoveDestinationGroupDeleted =
                pipeline_delete_group_on_square_before_enter_sensors(
                    dungeon, things, party->mapIndex, party->mapX, party->mapY,
                    &outResult->postMoveDeletedGroupThing);
            (void)F0718_SENSOR_ProcessPartyEnterLeave_Compat(
                dungeon, things, party->mapIndex, party->mapX, party->mapY,
                SENSOR_EVENT_WALK_ON, &outResult->core.enterEffects);
        } else {
            /* Source lock: MOVESENS.C:810-818 deletes a group already on
             * the final party destination before firing destination enter
             * sensors.  CLIKMENU.C:291-313 skips the pre-F0267 group-block
             * check when the party has no champions, so the non-transition
             * empty-party collision path must still perform the F0267 group
             * deletion/replay here, not only after pit/teleporter chains.
             */
            outResult->postMoveDestinationGroupDeleted =
                pipeline_delete_group_on_square_before_enter_sensors(
                    dungeon, things, party->mapIndex, party->mapX, party->mapY,
                    &outResult->postMoveDeletedGroupThing);
            if (outResult->postMoveDestinationGroupDeleted) {
                memset(&outResult->core.enterEffects, 0, sizeof(outResult->core.enterEffects));
                (void)F0718_SENSOR_ProcessPartyEnterLeave_Compat(
                    dungeon, things, party->mapIndex, party->mapX, party->mapY,
                    SENSOR_EVENT_WALK_ON, &outResult->core.enterEffects);
            }
        }

        /* Source lock: CLIKMENU.C:330-346 computes movement cooldown only
         * after F0267 returns.  Recompute timing from the final party state
         * so pit/teleporter side effects (including fall damage/death) are
         * ordered before G0310/G0311 publication.
         */
        outResult->core.timing =
            DM1_V1_MovementTiming_ApplySuccessfulStepPc34Compat(
                party, outResult->core.sourceMapIndex,
                outResult->core.sourceMapX, outResult->core.sourceMapY,
                pipeline->gameTick, pipeline->lastPartyMovementTime,
                footwearIcons);
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
    outResult->anyMovementOccurred =
        outResult->core.stepApplied || outResult->core.stairTransitionApplied ||
        outResult->postMove.transitioned;
    outResult->anyTurnOccurred = outResult->core.turnApplied;
    outResult->viewportDirty = outResult->core.viewportRedrawRequested;
    outResult->blockedMovementVblankWaitRequested =
        outResult->core.blockedMovementVblankWaitRequested;
    outResult->blockedMovementVblankWaitCount =
        outResult->core.blockedMovementVblankWaitRequested ? 1 : 0;
    outResult->blockedMovementKeepsInputWaitArmed =
        outResult->core.blockedMovementVblankWaitRequested &&
        !outResult->core.stopWaitingForPlayerInput;

    /* Phase 5: Source-locked provenance trace.
     *
     * This is deliberately compat provenance only: it records that Firestaff
     * accepted a command, applied movement, and requested/published the viewport
     * according to the source-locked ReDMCSB path.  It is not DOSBox/original
     * pixel evidence and must not be promoted as such.
     */
    outResult->provenance.commandAccepted =
        outResult->core.commandHandled && outResult->core.queue.dequeued &&
        !outResult->core.movementBlocked &&
        (outResult->core.stepApplied || outResult->core.turnApplied ||
         outResult->core.stairTransitionApplied);
    outResult->provenance.movementApplied =
        outResult->core.stepApplied || outResult->core.stairTransitionApplied ||
        outResult->postMove.transitioned;
    outResult->provenance.viewportPresent =
        outResult->viewportDirty &&
        (outResult->anyMovementOccurred || outResult->anyTurnOccurred);
    outResult->provenance.originalRuntimeObserved = 0;
    outResult->provenance.noPixelParityClaim = 1;
    outResult->provenance.commandAcceptedEvidence =
        "COMMAND.C:2075-2099 queue/gate; COMMAND.C:2118-2127 dequeue; COMMAND.C:2150-2156 movement dispatch";
    outResult->provenance.movementAppliedEvidence =
        "CLIKMENU.C:325-328 calls F0267_MOVE_GetMoveResult_CPSCE; MOVESENS.C:438-606 resolves open teleporter/pit chains and party HP side effects before sensors; MOVESENS.C:810-818 deletes destination group before final enter sensors; CLIKMENU.C:330-346 assigns G0310/G0311 after accepted step";
    outResult->provenance.viewportPresentEvidence =
        "GAMELOOP.C:90 redraws F0128_DUNGEONVIEW_Draw_CPSF from party state; DRAWVIEW.C:721-722 requests viewport blit and waits for vblank; DRAWVIEW.C:1056-1068 blits G0296 viewport to screen; CLIKMENU.C:317-323 blocked movement instead discards input, waits one PC-34 VBlank, and keeps G0321 false";

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
    return "ReDMCSB WIP20210206 Toolchains/Common/Source source lock: "
           "COMMAND.C:106-114 mouse movement zones, COMMAND.C:252-260 keyboard movement map, "
           "COMMAND.C:F0359/F0361 enqueue paths, COMMAND.C:2075-2099 F0380 lock/empty/movement-disabled gate, "
           "COMMAND.C:2118-2127 F0380 dequeue/unlock/replay, COMMAND.C:2150-2156 F0380 dispatches F0365/F0366; "
           "CLIKMENU.C:142-174 F0365 turn + sensor leave/enter, "
           "CLIKMENU.C:224-233 F0366 movement-arrow forward/right deltas, "
           "CLIKMENU.C:264-276 stairs special cases, CLIKMENU.C:278-323 wall/door/fakewall/group collision, discard, one PC-34 blocked-movement VBlank, and G0321 false, "
           "CLIKMENU.C:325-346 F0267 move-result call and G0310/G0311 cooldown write; "
           "DUNGEON.C:1389-1391 F0150 relative coordinate math; "
           "MOVESENS.C:316-328 F0267 signature/source-destination contract, "
           "MOVESENS.C:433-435 movement/projectile impact precheck, MOVESENS.C:475-535 open/scoped teleporter chain and party rotation, MOVESENS.C:538-606 open non-imaginary pit fall and party damage/rope reset, CHAMPION.C:1991-2022 F0324 all-champion damage fanout, CHAMPION.C:1689-1737 F0320 pending damage kill threshold, CHAMPION.C:1552-1668 F0319 champion kill/party-dead flag, MOVESENS.C:438-606 party coordinate teleporter/pit chain before sensors, MOVESENS.C:760-783 scent/G0362_l_LastPartyMovementTime, MOVESENS.C:799-818 F0276 source leave and final destination enter sensors, MOVESENS.C:810-818 destination group deletion before enter sensors, MOVESENS.C:830-887 group/party occupancy interlock and active-group refresh/defer, MOVESENS.C:893-897 projectile/explosion destination sensor exception; "
           "CHAMPION.C:1180-1215 F0310 movement ticks; "
           "GAMELOOP.C:90 redraws F0128_DUNGEONVIEW_Draw_CPSF from party state, GAMELOOP.C:150-155 cooldown decrement; "
           "DUNVIEW.C:8446-8542 F0128 back-to-front viewport wall/object draw order, "
           "DUNVIEW.C:8577-8579 restores native wall set, DUNVIEW.C:8609-8610 calls F0097 viewport blit; "
           "DRAWVIEW.C:721-722 viewport blit request/vblank, DRAWVIEW.C:1056-1068 G0296 viewport screen blit; "
           "Pipeline wires: dm1_v1_input_command_queue, dm1_v1_movement_command_core, "
           "dm1_v1_movement_timing, memory_movement (F0700-F0709), memory_sensor_execution (F0710-F0718), "
           "dm1_v1_viewport_3d wall draw-order metadata";
}
