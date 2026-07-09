/* DM1 V1 Teleporter/Pit — ReDMCSB MOVESENS.C F0276, F0267.
 * Generated via Q3.6, fixed by Opus (added x,y to structs, fixed pit chain). */
#include "dm1_v1_teleporter_pit_pc34_compat.h"
#include <string.h>

static int dm1_v1_normalize_direction_or_cell_pc34(int value) {
    return value & 3;
}

static unsigned int dm1_v1_get_group_value_pc34(unsigned int packed, int creatureIndex) {
    return (packed >> (creatureIndex * 2)) & 3u;
}

static unsigned int dm1_v1_set_group_value_pc34(unsigned int packed, int creatureIndex, unsigned int value) {
    unsigned int shift = (unsigned int)(creatureIndex * 2);
    packed &= ~(3u << shift);
    packed |= (value & 3u) << shift;
    return packed;
}

int DM1_V1_ApplyGroupTeleporterRotationF0262Pc34Compat(const DM1_V1_TeleporterDefPc34* teleporter,
                                   int creatureCountMinusOne,
                                   int creatureSize,
                                   unsigned int inDirections,
                                   unsigned int inCells,
                                   unsigned int* outDirections,
                                   unsigned int* outCells) {
    int i;
    int rotation;
    int absoluteRotation;
    unsigned int directions;
    unsigned int cells;

    if (!teleporter || !outDirections || !outCells || creatureCountMinusOne < 0) return 0;

    rotation = dm1_v1_normalize_direction_or_cell_pc34(teleporter->destFacing);
    absoluteRotation = teleporter->absoluteRotation != 0;
    directions = inDirections;
    cells = inCells;

    for (i = 0; i <= creatureCountMinusOne; ++i) {
        int oldDirection = (int)dm1_v1_get_group_value_pc34(inDirections, i);
        int newDirection = absoluteRotation
            ? rotation
            : dm1_v1_normalize_direction_or_cell_pc34(oldDirection + rotation);
        directions = dm1_v1_set_group_value_pc34(directions, i, (unsigned int)newDirection);

        if (inCells != DM1_V1_GROUP_CELL_SINGLE_CENTERED_PC34) {
            int cellRotation = 0;
            if (creatureSize == DM1_V1_CREATURE_SIZE_QUARTER_SQUARE_PC34) {
                cellRotation = absoluteRotation ? 0 : rotation;
            } else {
                cellRotation = absoluteRotation
                    ? dm1_v1_normalize_direction_or_cell_pc34(rotation - oldDirection)
                    : rotation;
            }
            if (cellRotation) {
                int oldCell = (int)dm1_v1_get_group_value_pc34(inCells, i);
                cells = dm1_v1_set_group_value_pc34(cells, i,
                    (unsigned int)dm1_v1_normalize_direction_or_cell_pc34(oldCell + cellRotation));
            }
        }
    }

    *outDirections = directions;
    *outCells = cells;
    return 1;
}

int DM1_V1_ApplyTeleporterRotationF0267Pc34Compat(int thingKind,
                                   int sourceMapX,
                                   const DM1_V1_TeleporterDefPc34* teleporter,
                                   int inDirection,
                                   int inCell,
                                   int* outDirection,
                                   int* outCell) {
    int rotation;
    int singleCenteredCell;

    if (!teleporter || !outDirection || !outCell) return 0;

    rotation = dm1_v1_normalize_direction_or_cell_pc34(teleporter->destFacing);
    singleCenteredCell = (inCell == DM1_V1_GROUP_CELL_SINGLE_CENTERED_PC34);
    *outDirection = dm1_v1_normalize_direction_or_cell_pc34(inDirection);
    *outCell = singleCenteredCell ? DM1_V1_GROUP_CELL_SINGLE_CENTERED_PC34 : dm1_v1_normalize_direction_or_cell_pc34(inCell);

    switch (thingKind) {
    case DM1_V1_TELEPORTER_ROTATE_THING_PARTY_PC34:
        if (teleporter->absoluteRotation) {
            *outDirection = rotation;
        } else {
            *outDirection = dm1_v1_normalize_direction_or_cell_pc34(*outDirection + rotation);
        }
        return 1;
    case DM1_V1_TELEPORTER_ROTATE_THING_PROJECTILE_PC34:
        if (teleporter->absoluteRotation) {
            *outDirection = rotation;
        } else {
            *outDirection = dm1_v1_normalize_direction_or_cell_pc34(*outDirection + rotation);
            *outCell = dm1_v1_normalize_direction_or_cell_pc34(*outCell + rotation);
        }
        return 1;
    case DM1_V1_TELEPORTER_ROTATE_THING_OBJECT_PC34:
        if (!teleporter->absoluteRotation &&
            sourceMapX != DM1_V1_MAPX_PROJECTILE_ASSOCIATED_OBJECT_PC34) {
            *outCell = dm1_v1_normalize_direction_or_cell_pc34(*outCell + rotation);
        }
        return 1;
    case DM1_V1_TELEPORTER_ROTATE_THING_GROUP_PC34: {
        unsigned int groupDirections;
        unsigned int groupCells;
        if (!DM1_V1_ApplyGroupTeleporterRotationF0262Pc34Compat(teleporter, 0,
                DM1_V1_CREATURE_SIZE_QUARTER_SQUARE_PC34, (unsigned int)*outDirection,
                (unsigned int)*outCell, &groupDirections, &groupCells)) {
            return 0;
        }
        *outDirection = (int)(groupDirections & 3u);
        *outCell = singleCenteredCell ? DM1_V1_GROUP_CELL_SINGLE_CENTERED_PC34 : (int)(groupCells & 3u);
        return 1;
    }
    default:
        return 0;
    }
}

const char* DM1_V1_TeleporterRotation_SourceEvidencePc34Compat(void) {
    return "ReDMCSB WIP20210206 Toolchains/Common/Source: MOVESENS.C:33-111 F0262 group teleporter direction/cell rotation; MOVESENS.C:120-133 F0263 projectile teleporter rotation; MOVESENS.C:316-322 F0267 source-map sentinel contract; MOVESENS.C:493-518 party absolute/relative teleporter rotation; MOVESENS.C:520-524 group audible buzz and F0262 dispatch; MOVESENS.C:526-531 projectile/object teleporter rotation and projectile-associated object exception";
}

int DM1_V1_PlanGroupMoveRemovalAfterPitTeleporterF0267Pc34Compat(
        int fallKilledGroup,
        int creatureAllowedOnDestinationMap,
        int sourceMapX,
        int sourceMapY,
        int destinationMapX,
        int destinationMapY,
        DM1_V1_GroupMoveRemovalPlanPc34* outPlan) {
    DM1_V1_GroupMoveRemovalPlanPc34 plan;

    if (!outPlan) return 0;
    memset(&plan, 0, sizeof(plan));
    plan.reason = DM1_V1_GROUP_MOVE_REMOVAL_REASON_NONE_PC34;
    plan.dropGroupPossessionsSoundMode = -1;
    plan.dropMapX = destinationMapX;
    plan.dropMapY = destinationMapY;
    plan.deleteMapX = sourceMapX;
    plan.deleteMapY = sourceMapY;

    if (!fallKilledGroup && creatureAllowedOnDestinationMap) {
        *outPlan = plan;
        return 1;
    }

    /* ReDMCSB MOVESENS.C F0267 lines 656-663: after pit/teleporter
     * resolution, a group killed by fall damage or disallowed on the
     * destination map drops moving fixed possessions, drops group possessions
     * at the destination with C02_MODE_PLAY_ONE_TICK_LATER, deletes the source
     * group only when it came from a square, and reports move prevented. */
    plan.movePrevented = 1;
    plan.reason = fallKilledGroup
        ? DM1_V1_GROUP_MOVE_REMOVAL_REASON_FALL_KILLED_PC34
        : DM1_V1_GROUP_MOVE_REMOVAL_REASON_NOT_ALLOWED_PC34;
    plan.dropMovingCreatureFixedPossessions = 1;
    plan.dropGroupPossessions = 1;
    plan.dropGroupPossessionsSoundMode = DM1_V1_GROUP_MOVE_REMOVAL_SOUND_ONE_TICK_LATER_PC34;
    plan.deleteSourceGroup = (sourceMapX >= 0);

    *outPlan = plan;
    return 1;
}

int DM1_V1_PlanDeferredGroupMoveRouteF0267Pc34Compat(
        int fallKilledGroup,
        int creatureAllowedOnDestinationMap,
        int destinationBlocked,
        int audibleEvent,
        int chaosAdjacentAvailable,
        uint32_t currentFireAtTick,
        int destinationMapX,
        int destinationMapY,
        int chaosAdjacentMapX,
        int chaosAdjacentMapY,
        DM1_V1_GroupMoveRoutePlanPc34* outPlan) {
    DM1_V1_GroupMoveRoutePlanPc34 plan;

    if (!outPlan) return 0;
    memset(&plan, 0, sizeof(plan));
    plan.valid = 1;
    plan.mapX = destinationMapX;
    plan.mapY = destinationMapY;

    if (fallKilledGroup || !creatureAllowedOnDestinationMap) {
        plan.route = DM1_V1_GROUP_MOVE_ROUTE_REMOVE_PC34;
        plan.shouldEmitAudibleBuzz = audibleEvent ? 1 : 0;
        plan.removalReason = fallKilledGroup
            ? DM1_V1_GROUP_MOVE_REMOVAL_REASON_FALL_KILLED_PC34
            : DM1_V1_GROUP_MOVE_REMOVAL_REASON_NOT_ALLOWED_PC34;
        *outPlan = plan;
        return 1;
    }

    if (destinationBlocked) {
        if (chaosAdjacentAvailable) {
            plan.route = DM1_V1_GROUP_MOVE_ROUTE_CHAOS_ADJACENT_INSERT_PC34;
            plan.shouldEmitAudibleBuzz = audibleEvent ? 1 : 0;
            plan.mapX = chaosAdjacentMapX;
            plan.mapY = chaosAdjacentMapY;
        } else {
            plan.route = DM1_V1_GROUP_MOVE_ROUTE_RETRY_PC34;
            plan.shouldScheduleRetry = 1;
            plan.retryFireAtTick = currentFireAtTick + 5u;
        }
        *outPlan = plan;
        return 1;
    }

    plan.route = DM1_V1_GROUP_MOVE_ROUTE_INSERT_PC34;
    plan.shouldEmitAudibleBuzz = audibleEvent ? 1 : 0;
    *outPlan = plan;
    return 1;
}

int DM1_V1_PlanOrdinaryGroupMoveF0267Pc34Compat(
        int sourceMapX,
        int sourceMapY,
        int direction,
        int destinationPassable,
        int destinationBlocked,
        int killedByProjectile,
        uint32_t currentTick,
        DM1_V1_OrdinaryGroupMovePlanPc34* outPlan) {
    DM1_V1_OrdinaryGroupMovePlanPc34 plan;

    if (!outPlan) return 0;
    memset(&plan, 0, sizeof(plan));
    plan.valid = 1;
    plan.destinationMapX = sourceMapX;
    plan.destinationMapY = sourceMapY;

    switch (direction & 3) {
        case DM1_V1_DIRECTION_NORTH_PC34: plan.destinationMapY--; break;
        case DM1_V1_DIRECTION_EAST_PC34:  plan.destinationMapX++; break;
        case DM1_V1_DIRECTION_SOUTH_PC34: plan.destinationMapY++; break;
        case DM1_V1_DIRECTION_WEST_PC34:  plan.destinationMapX--; break;
    }

    if (!destinationPassable || destinationBlocked) {
        plan.route = DM1_V1_GROUP_MOVE_ROUTE_RETRY_PC34;
        plan.retryFireAtTick = currentTick + 1u;
        *outPlan = plan;
        return 1;
    }
    if (killedByProjectile) {
        plan.route = DM1_V1_GROUP_MOVE_ROUTE_KILLED_BY_PROJECTILE_PC34;
        *outPlan = plan;
        return 1;
    }

    plan.route = DM1_V1_GROUP_MOVE_ROUTE_INSERT_PC34;
    plan.retryFireAtTick = currentTick + 1u;

    /* ReDMCSB GROUP.C F0209 lines 1928/2175 enters MOVESENS.C F0267
     * from the creature's source square. MOVESENS.C F0267 lines 432-435
     * runs the projectile precheck before a successful move, and blocked
     * C37 movement is retried on the next tick in the Firestaff timeline
     * adapter. */
    *outPlan = plan;
    return 1;
}

int DM1_V1_PlanOrdinaryGroupMoveApplyF0267Pc34Compat(
        const DM1_V1_OrdinaryGroupMovePlanPc34* movePlan,
        int sourceMapIndex,
        int direction,
        int groupCells,
        uint32_t currentTick,
        DM1_V1_OrdinaryGroupMoveApplyPlanPc34* outPlan) {
    DM1_V1_OrdinaryGroupMoveApplyPlanPc34 plan;

    if (!movePlan || !outPlan || !movePlan->valid) return 0;
    memset(&plan, 0, sizeof(plan));
    plan.valid = 1;
    plan.groupDirection = direction & 3;
    plan.activeMapIndex = sourceMapIndex;
    plan.activeMapX = movePlan->destinationMapX;
    plan.activeMapY = movePlan->destinationMapY;
    plan.activeCells = groupCells;
    plan.nextFireAtTick = currentTick + 1u;
    plan.nextEventMapX = movePlan->destinationMapX;
    plan.nextEventMapY = movePlan->destinationMapY;

    if (movePlan->route == DM1_V1_GROUP_MOVE_ROUTE_KILLED_BY_PROJECTILE_PC34) {
        plan.shouldUnlinkSource = 1;
        plan.shouldRemoveActiveGroup = 1;
        *outPlan = plan;
        return 1;
    }
    if (movePlan->route == DM1_V1_GROUP_MOVE_ROUTE_INSERT_PC34) {
        plan.shouldUnlinkSource = 1;
        plan.shouldLinkDestination = 1;
        plan.shouldRequeue = 1;
        *outPlan = plan;
        return 1;
    }
    if (movePlan->route == DM1_V1_GROUP_MOVE_ROUTE_RETRY_PC34) {
        plan.shouldRequeue = 1;
        plan.nextEventMapX = movePlan->destinationMapX;
        plan.nextEventMapY = movePlan->destinationMapY;
    }

    /* ReDMCSB GROUP.C F0209 plus MOVESENS.C F0267: once an ordinary
     * C37 group move survives the projectile precheck, the source group is
     * unlinked, reinserted at the destination, active-group coordinates are
     * advanced, and the next behavior event carries the destination square. */
    *outPlan = plan;
    return 1;
}

int DM1_V1_PlanGroupPitFallSquareF0267Pc34Compat(
        int squareType,
        int pitSquareType,
        int pitOpen,
        int pitImaginary,
        DM1_V1_GroupPitFallSquarePlanPc34* outPlan) {
    DM1_V1_GroupPitFallSquarePlanPc34 plan;

    if (!outPlan) return 0;
    memset(&plan, 0, sizeof(plan));
    plan.valid = 1;
    plan.shouldFall =
        squareType == pitSquareType && pitOpen && !pitImaginary;

    /* ReDMCSB: MOVESENS.C F0267 lines 538-574 follows only open,
     * non-imaginary pit squares during group movement resolution. */
    *outPlan = plan;
    return 1;
}

int DM1_V1_PlanGroupTeleporterDestinationF0267Pc34Compat(
        int squareType,
        int teleporterSquareType,
        int teleporterOpen,
        int teleporterFound,
        int teleporterScope,
        int teleporterAudible,
        int targetMapIndex,
        int targetMapX,
        int targetMapY,
        int sourceMapIndex,
        int sourceMapX,
        int sourceMapY,
        int mapCount,
        DM1_V1_GroupTeleporterDestinationPlanPc34* outPlan) {
    DM1_V1_GroupTeleporterDestinationPlanPc34 plan;

    if (!outPlan) return 0;
    memset(&plan, 0, sizeof(plan));
    plan.valid = 1;
    plan.targetMapIndex = sourceMapIndex;
    plan.targetMapX = sourceMapX;
    plan.targetMapY = sourceMapY;

    if (squareType != teleporterSquareType || !teleporterOpen ||
        !teleporterFound || !(teleporterScope & DM1_V1_TELEPORTER_SCOPE_CREATURES_PC34)) {
        *outPlan = plan;
        return 1;
    }
    if (targetMapIndex < 0 || targetMapIndex >= mapCount) {
        *outPlan = plan;
        return 1;
    }

    plan.shouldTeleport = 1;
    plan.shouldEmitAudibleBuzz = teleporterAudible ? 1 : 0;
    plan.shouldStopChain =
        targetMapIndex == sourceMapIndex &&
        targetMapX == sourceMapX &&
        targetMapY == sourceMapY;
    plan.targetMapIndex = targetMapIndex;
    plan.targetMapX = targetMapX;
    plan.targetMapY = targetMapY;

    /* ReDMCSB MOVESENS.C F0267 lines 474-492: group teleporter
     * chaining accepts only open creature-scope teleporters, switches to
     * TargetMap/TargetX/TargetY, and stops on a self-targeting teleporter.
     * Lines 520-524 request the target-square M560 buzz for audible hops. */
    *outPlan = plan;
    return 1;
}

int DM1_V1_PlanGeneratedGroupPlacementF0183F0180Pc34Compat(
        int partyMapIndex,
        int mapIndex,
        int mapX,
        int mapY,
        int groupIndex,
        int creatureType,
        int groupCells,
        int groupDirection,
        int activeGroupCount,
        int activeGroupCapacity,
        uint32_t currentTick,
        DM1_V1_GeneratedGroupPlacementPlanPc34* outPlan) {
    DM1_V1_GeneratedGroupPlacementPlanPc34 plan;

    if (!outPlan) return 0;
    memset(&plan, 0, sizeof(plan));
    plan.valid = 1;
    plan.activeTargetChampionIndex = -1;
    plan.activeLastSeenPartyMapX = -1;
    plan.activeLastSeenPartyMapY = -1;
    plan.activeLastSeenPartyTick = -1;

    plan.shouldScheduleWanderEvent = 1;
    plan.wanderFireAtTick = currentTick + 1u;
    plan.wanderMapIndex = mapIndex;
    plan.wanderMapX = mapX;
    plan.wanderMapY = mapY;
    plan.wanderGroupIndex = groupIndex;
    plan.wanderCreatureType = creatureType;
    plan.wanderEventType = DM1_V1_AI_STATE_WANDER_PC34;

    if (mapIndex == partyMapIndex && activeGroupCapacity > 0 && activeGroupCount >= 0) {
        if (activeGroupCount >= activeGroupCapacity) {
            return 0;
        }
        plan.shouldCreateActiveState = 1;
        plan.activeStateKind = DM1_V1_AI_STATE_WANDER_PC34;
        plan.activeCreatureType = creatureType;
        plan.activeMapIndex = mapIndex;
        plan.activeMapX = mapX;
        plan.activeMapY = mapY;
        plan.activeCells = groupCells;
        plan.activeDirection = groupDirection & 3;
        plan.activeReservedGroupIndex = groupIndex;
    }

    /* ReDMCSB GROUP.C F0183 lines 414-447 creates ACTIVE_GROUP state for
     * generated groups on the party map. GROUP.C F0180 lines 311-338 starts
     * newly placed groups wandering by scheduling C37 at game time +1. */
    *outPlan = plan;
    return 1;
}

int DM1_V1_PlanLordChaosAdjacentRetryF0252Pc34Compat(
        int creatureType,
        int randomGate,
        int randomDirection,
        int sourceMapX,
        int sourceMapY,
        int candidateAllowedSquare,
        int candidateBlocked,
        DM1_V1_LordChaosAdjacentRetryPlanPc34* outPlan) {
    DM1_V1_LordChaosAdjacentRetryPlanPc34 plan;

    if (!outPlan) return 0;
    memset(&plan, 0, sizeof(plan));
    plan.valid = 1;
    plan.candidateMapX = sourceMapX;
    plan.candidateMapY = sourceMapY;
    if (creatureType != 23 || randomGate != 0) {
        *outPlan = plan;
        return 1;
    }

    plan.shouldAttempt = 1;
    switch (randomDirection & 3) {
        case 0: plan.candidateMapX--; break;
        case 1: plan.candidateMapX++; break;
        case 2: plan.candidateMapY--; break;
        case 3: plan.candidateMapY++; break;
    }
    if (candidateAllowedSquare < 0 || candidateBlocked < 0) {
        *outPlan = plan;
        return 1;
    }
    plan.shouldInsertAdjacent =
        candidateAllowedSquare && !candidateBlocked;

    /* ReDMCSB: TIMELINE.C F0252 lines 1536-1555 gives Lord Chaos one
     * 1/4 random adjacent insertion attempt before retrying event60/61. */
    *outPlan = plan;
    return 1;
}

const char* DM1_V1_GroupMoveRemoval_SourceEvidencePc34Compat(void) {
    return "ReDMCSB WIP20210206 Toolchains/Common/Source: "
           "MOVESENS.C F0267 lines 608-624 damages falling groups and drops moving fixed possessions on partial death; "
           "MOVESENS.C F0267 lines 656-663 handles fall-killed or destination-map-disallowed group removal via F0187/F0188/F0189; "
           "GROUP.C F0187 lines 648-674 drops moving creature fixed possessions; "
           "GROUP.C F0188 lines 676-737 drops group possessions with caller sound mode; "
           "GROUP.C F0189 lines 739-762 deletes the group from the source square; "
           "TIMELINE.C F0252 lines 1527-1567 inserts event60/61 groups, tries Lord Chaos adjacent placement, or retries at +5 ticks";
}
