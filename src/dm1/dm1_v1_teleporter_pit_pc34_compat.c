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

const char* m11_group_move_removal_source_evidence(void) {
    return "ReDMCSB WIP20210206 Toolchains/Common/Source: "
           "MOVESENS.C F0267 lines 608-624 damages falling groups and drops moving fixed possessions on partial death; "
           "MOVESENS.C F0267 lines 656-663 handles fall-killed or destination-map-disallowed group removal via F0187/F0188/F0189; "
           "GROUP.C F0187 lines 648-674 drops moving creature fixed possessions; "
           "GROUP.C F0188 lines 676-737 drops group possessions with caller sound mode; "
           "GROUP.C F0189 lines 739-762 deletes the group from the source square";
}
