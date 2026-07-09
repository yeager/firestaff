/* DM1 V1 Teleporter/Pit — ReDMCSB MOVESENS.C F0276, F0267.
 * Generated via Q3.6, fixed by Opus (added x,y to structs, fixed pit chain). */
#include "dm1_v1_teleporter_pit_pc34_compat.h"
#include <string.h>

static int m11_normalize_direction_or_cell(int value) {
    return value & 3;
}

static unsigned int m11_get_group_value(unsigned int packed, int creatureIndex) {
    return (packed >> (creatureIndex * 2)) & 3u;
}

static unsigned int m11_set_group_value(unsigned int packed, int creatureIndex, unsigned int value) {
    unsigned int shift = (unsigned int)(creatureIndex * 2);
    packed &= ~(3u << shift);
    packed |= (value & 3u) << shift;
    return packed;
}

int m11_apply_group_teleporter_rotation(const M11_TeleporterDef* teleporter,
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

    rotation = m11_normalize_direction_or_cell(teleporter->destFacing);
    absoluteRotation = teleporter->absoluteRotation != 0;
    directions = inDirections;
    cells = inCells;

    for (i = 0; i <= creatureCountMinusOne; ++i) {
        int oldDirection = (int)m11_get_group_value(inDirections, i);
        int newDirection = absoluteRotation
            ? rotation
            : m11_normalize_direction_or_cell(oldDirection + rotation);
        directions = m11_set_group_value(directions, i, (unsigned int)newDirection);

        if (inCells != M11_GROUP_CELL_SINGLE_CENTERED) {
            int cellRotation = 0;
            if (creatureSize == M11_CREATURE_SIZE_QUARTER_SQUARE) {
                cellRotation = absoluteRotation ? 0 : rotation;
            } else {
                cellRotation = absoluteRotation
                    ? m11_normalize_direction_or_cell(rotation - oldDirection)
                    : rotation;
            }
            if (cellRotation) {
                int oldCell = (int)m11_get_group_value(inCells, i);
                cells = m11_set_group_value(cells, i,
                    (unsigned int)m11_normalize_direction_or_cell(oldCell + cellRotation));
            }
        }
    }

    *outDirections = directions;
    *outCells = cells;
    return 1;
}

int m11_apply_teleporter_rotation(int thingKind,
                                   int sourceMapX,
                                   const M11_TeleporterDef* teleporter,
                                   int inDirection,
                                   int inCell,
                                   int* outDirection,
                                   int* outCell) {
    int rotation;
    int singleCenteredCell;

    if (!teleporter || !outDirection || !outCell) return 0;

    rotation = m11_normalize_direction_or_cell(teleporter->destFacing);
    singleCenteredCell = (inCell == M11_GROUP_CELL_SINGLE_CENTERED);
    *outDirection = m11_normalize_direction_or_cell(inDirection);
    *outCell = singleCenteredCell ? M11_GROUP_CELL_SINGLE_CENTERED : m11_normalize_direction_or_cell(inCell);

    switch (thingKind) {
    case M11_TELEPORTER_ROTATE_THING_PARTY:
        if (teleporter->absoluteRotation) {
            *outDirection = rotation;
        } else {
            *outDirection = m11_normalize_direction_or_cell(*outDirection + rotation);
        }
        return 1;
    case M11_TELEPORTER_ROTATE_THING_PROJECTILE:
        if (teleporter->absoluteRotation) {
            *outDirection = rotation;
        } else {
            *outDirection = m11_normalize_direction_or_cell(*outDirection + rotation);
            *outCell = m11_normalize_direction_or_cell(*outCell + rotation);
        }
        return 1;
    case M11_TELEPORTER_ROTATE_THING_OBJECT:
        if (!teleporter->absoluteRotation &&
            sourceMapX != M11_MAPX_PROJECTILE_ASSOCIATED_OBJECT) {
            *outCell = m11_normalize_direction_or_cell(*outCell + rotation);
        }
        return 1;
    case M11_TELEPORTER_ROTATE_THING_GROUP: {
        unsigned int groupDirections;
        unsigned int groupCells;
        if (!m11_apply_group_teleporter_rotation(teleporter, 0,
                M11_CREATURE_SIZE_QUARTER_SQUARE, (unsigned int)*outDirection,
                (unsigned int)*outCell, &groupDirections, &groupCells)) {
            return 0;
        }
        *outDirection = (int)(groupDirections & 3u);
        *outCell = singleCenteredCell ? M11_GROUP_CELL_SINGLE_CENTERED : (int)(groupCells & 3u);
        return 1;
    }
    default:
        return 0;
    }
}

const char* m11_teleporter_rotation_source_evidence(void) {
    return "ReDMCSB WIP20210206 Toolchains/Common/Source: MOVESENS.C:33-111 F0262 group teleporter direction/cell rotation; MOVESENS.C:120-133 F0263 projectile teleporter rotation; MOVESENS.C:316-322 F0267 source-map sentinel contract; MOVESENS.C:493-518 party absolute/relative teleporter rotation; MOVESENS.C:520-524 group audible buzz and F0262 dispatch; MOVESENS.C:526-531 projectile/object teleporter rotation and projectile-associated object exception";
}

int m11_plan_group_move_removal_after_pit_teleporter(
        int fallKilledGroup,
        int creatureAllowedOnDestinationMap,
        int sourceMapX,
        int sourceMapY,
        int destinationMapX,
        int destinationMapY,
        M11_GroupMoveRemovalPlan* outPlan) {
    M11_GroupMoveRemovalPlan plan;

    if (!outPlan) return 0;
    memset(&plan, 0, sizeof(plan));
    plan.reason = M11_GROUP_MOVE_REMOVAL_REASON_NONE;
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
        ? M11_GROUP_MOVE_REMOVAL_REASON_FALL_KILLED
        : M11_GROUP_MOVE_REMOVAL_REASON_NOT_ALLOWED;
    plan.dropMovingCreatureFixedPossessions = 1;
    plan.dropGroupPossessions = 1;
    plan.dropGroupPossessionsSoundMode = M11_GROUP_MOVE_REMOVAL_SOUND_ONE_TICK_LATER;
    plan.deleteSourceGroup = (sourceMapX >= 0);

    *outPlan = plan;
    return 1;
}

int m11_plan_deferred_group_move_route_f0267(
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
        M11_GroupMoveRoutePlan* outPlan) {
    M11_GroupMoveRoutePlan plan;

    if (!outPlan) return 0;
    memset(&plan, 0, sizeof(plan));
    plan.valid = 1;
    plan.mapX = destinationMapX;
    plan.mapY = destinationMapY;

    if (fallKilledGroup || !creatureAllowedOnDestinationMap) {
        plan.route = M11_GROUP_MOVE_ROUTE_REMOVE;
        plan.shouldEmitAudibleBuzz = audibleEvent ? 1 : 0;
        plan.removalReason = fallKilledGroup
            ? M11_GROUP_MOVE_REMOVAL_REASON_FALL_KILLED
            : M11_GROUP_MOVE_REMOVAL_REASON_NOT_ALLOWED;
        *outPlan = plan;
        return 1;
    }

    if (destinationBlocked) {
        if (chaosAdjacentAvailable) {
            plan.route = M11_GROUP_MOVE_ROUTE_CHAOS_ADJACENT_INSERT;
            plan.shouldEmitAudibleBuzz = audibleEvent ? 1 : 0;
            plan.mapX = chaosAdjacentMapX;
            plan.mapY = chaosAdjacentMapY;
        } else {
            plan.route = M11_GROUP_MOVE_ROUTE_RETRY;
            plan.shouldScheduleRetry = 1;
            plan.retryFireAtTick = currentFireAtTick + 5u;
        }
        *outPlan = plan;
        return 1;
    }

    plan.route = M11_GROUP_MOVE_ROUTE_INSERT;
    plan.shouldEmitAudibleBuzz = audibleEvent ? 1 : 0;
    *outPlan = plan;
    return 1;
}

int m11_plan_ordinary_group_move_f0267(
        int sourceMapX,
        int sourceMapY,
        int direction,
        int destinationPassable,
        int destinationBlocked,
        int killedByProjectile,
        uint32_t currentTick,
        M11_OrdinaryGroupMovePlan* outPlan) {
    M11_OrdinaryGroupMovePlan plan;

    if (!outPlan) return 0;
    memset(&plan, 0, sizeof(plan));
    plan.valid = 1;
    plan.destinationMapX = sourceMapX;
    plan.destinationMapY = sourceMapY;

    switch (direction & 3) {
        case M11_DIRECTION_NORTH: plan.destinationMapY--; break;
        case M11_DIRECTION_EAST:  plan.destinationMapX++; break;
        case M11_DIRECTION_SOUTH: plan.destinationMapY++; break;
        case M11_DIRECTION_WEST:  plan.destinationMapX--; break;
    }

    if (!destinationPassable || destinationBlocked) {
        plan.route = M11_GROUP_MOVE_ROUTE_RETRY;
        plan.retryFireAtTick = currentTick + 1u;
        *outPlan = plan;
        return 1;
    }
    if (killedByProjectile) {
        plan.route = M11_GROUP_MOVE_ROUTE_KILLED_BY_PROJECTILE;
        *outPlan = plan;
        return 1;
    }

    plan.route = M11_GROUP_MOVE_ROUTE_INSERT;
    plan.retryFireAtTick = currentTick + 1u;

    /* ReDMCSB GROUP.C F0209 lines 1928/2175 enters MOVESENS.C F0267
     * from the creature's source square. MOVESENS.C F0267 lines 432-435
     * runs the projectile precheck before a successful move, and blocked
     * C37 movement is retried on the next tick in the Firestaff timeline
     * adapter. */
    *outPlan = plan;
    return 1;
}

int m11_plan_ordinary_group_move_apply_f0267(
        const M11_OrdinaryGroupMovePlan* movePlan,
        int sourceMapIndex,
        int direction,
        int groupCells,
        uint32_t currentTick,
        M11_OrdinaryGroupMoveApplyPlan* outPlan) {
    M11_OrdinaryGroupMoveApplyPlan plan;

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

    if (movePlan->route == M11_GROUP_MOVE_ROUTE_KILLED_BY_PROJECTILE) {
        plan.shouldUnlinkSource = 1;
        plan.shouldRemoveActiveGroup = 1;
        *outPlan = plan;
        return 1;
    }
    if (movePlan->route == M11_GROUP_MOVE_ROUTE_INSERT) {
        plan.shouldUnlinkSource = 1;
        plan.shouldLinkDestination = 1;
        plan.shouldRequeue = 1;
        *outPlan = plan;
        return 1;
    }
    if (movePlan->route == M11_GROUP_MOVE_ROUTE_RETRY) {
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

int m11_plan_group_pit_fall_square_f0267(
        int squareType,
        int pitSquareType,
        int pitOpen,
        int pitImaginary,
        M11_GroupPitFallSquarePlan* outPlan) {
    M11_GroupPitFallSquarePlan plan;

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

int m11_plan_group_teleporter_destination_f0267(
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
        M11_GroupTeleporterDestinationPlan* outPlan) {
    M11_GroupTeleporterDestinationPlan plan;

    if (!outPlan) return 0;
    memset(&plan, 0, sizeof(plan));
    plan.valid = 1;
    plan.targetMapIndex = sourceMapIndex;
    plan.targetMapX = sourceMapX;
    plan.targetMapY = sourceMapY;

    if (squareType != teleporterSquareType || !teleporterOpen ||
        !teleporterFound || !(teleporterScope & M11_TELEPORTER_SCOPE_CREATURES)) {
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

int m11_plan_lord_chaos_adjacent_retry_f0252(
        int creatureType,
        int randomGate,
        int randomDirection,
        int sourceMapX,
        int sourceMapY,
        int candidateAllowedSquare,
        int candidateBlocked,
        M11_LordChaosAdjacentRetryPlan* outPlan) {
    M11_LordChaosAdjacentRetryPlan plan;

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

const char* m11_group_move_removal_source_evidence(void) {
    return "ReDMCSB WIP20210206 Toolchains/Common/Source: "
           "MOVESENS.C F0267 lines 608-624 damages falling groups and drops moving fixed possessions on partial death; "
           "MOVESENS.C F0267 lines 656-663 handles fall-killed or destination-map-disallowed group removal via F0187/F0188/F0189; "
           "GROUP.C F0187 lines 648-674 drops moving creature fixed possessions; "
           "GROUP.C F0188 lines 676-737 drops group possessions with caller sound mode; "
           "GROUP.C F0189 lines 739-762 deletes the group from the source square; "
           "TIMELINE.C F0252 lines 1527-1567 inserts event60/61 groups, tries Lord Chaos adjacent placement, or retries at +5 ticks";
}
