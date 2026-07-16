#include "dm1_v1_group_targeting_f0175_f0178_pc34_compat.h"

#include <string.h>

static const int kOrderedCellsToAttack[8][4] = {
    { 0, 1, 3, 2 },
    { 1, 0, 2, 3 },
    { 1, 2, 0, 3 },
    { 2, 1, 3, 0 },
    { 3, 2, 0, 1 },
    { 2, 3, 1, 0 },
    { 0, 3, 1, 2 },
    { 3, 0, 2, 1 }
};

const char *DM1_V1_F0175_F0178_SourceEvidencePc34(void)
{
    return
        "GROUP.C:52-70 F0175_GROUP_GetThing walks a square thing list and "
        "returns the first C04 group THING\n"
        "GROUP.C:69-107 F0176_GROUP_GetCreatureOrdinalInCell scans group "
        "creature cells, handles C0xFF single-centered groups, and returns a "
        "one-based creature ordinal\n"
        "GROUP.C:109-158 F0177_GROUP_GetMeleeTargetCreatureOrdinal builds "
        "ordered attack cells, then calls F0176 for the first target ordinal\n"
        "GROUP.C F0178 updates one two-bit creature field inside a packed "
        "group byte/word value";
}

static uint16_t thing_type(uint16_t thing)
{
    return (uint16_t)((thing >> 10) & 0x000fu);
}

static uint16_t thing_index(uint16_t thing)
{
    return (uint16_t)(thing & 0x03ffu);
}

static int valid_creature_count_minus_one(int countMinusOne)
{
    return countMinusOne >= 0 &&
        countMinusOne < DM1_V1_F0176_MAX_CREATURE_SLOTS_PC34;
}

static int creature_cell(uint8_t packedCells, int creatureIndex)
{
    if (creatureIndex < 0 ||
        creatureIndex >= DM1_V1_F0176_MAX_CREATURE_SLOTS_PC34) {
        return -1;
    }
    return (int)((packedCells >> (creatureIndex * 2)) & 0x03u);
}

static void clear_f0175(DM1_V1_GroupThingResultF0175Pc34 *out)
{
    if (out) {
        memset(out, 0, sizeof(*out));
        out->thing = DM1_V1_F0175_THING_END_OF_LIST_PC34;
        out->nodeIndex = -1;
    }
}

static void clear_f0176(DM1_V1_CreatureOrdinalResultF0176Pc34 *out)
{
    if (out) {
        memset(out, 0, sizeof(*out));
        out->creatureOrdinal = 0;
        out->creatureIndex = -1;
        out->queryCell = -1;
        out->matchedCell = -1;
    }
}

static void clear_f0177(DM1_V1_MeleeTargetResultF0177Pc34 *out)
{
    if (out) {
        memset(out, 0, sizeof(*out));
        out->creatureOrdinal = 0;
        out->creatureIndex = -1;
        out->firstLivingCreatureIndex = -1;
    }
}

uint16_t F0178_GROUP_GetGroupValueUpdatedWithCreatureValue(
    uint16_t groupValue,
    int creatureIndex,
    uint16_t creatureValue)
{
    unsigned shift;
    uint16_t mask;

    if (creatureIndex < 0 ||
        creatureIndex >= DM1_V1_F0176_MAX_CREATURE_SLOTS_PC34) {
        return groupValue;
    }
    shift = (unsigned)(creatureIndex * 2);
    mask = (uint16_t)(0x0003u << shift);
    return (uint16_t)((groupValue & (uint16_t)~mask) |
                      ((creatureValue & 0x0003u) << shift));
}

uint16_t F0175_GROUP_GetThing(
    const DM1_V1_GroupThingNodeF0175Pc34 *nodes,
    size_t nodeCount,
    uint16_t firstThing,
    DM1_V1_GroupThingResultF0175Pc34 *out)
{
    uint16_t thing = firstThing;
    size_t guard = 0;

    clear_f0175(out);
    if (!nodes) {
        return DM1_V1_F0175_THING_END_OF_LIST_PC34;
    }

    while (thing != DM1_V1_F0175_THING_END_OF_LIST_PC34 && guard < nodeCount) {
        uint16_t index = thing_index(thing);
        if (index >= nodeCount) {
            break;
        }
        if (thing_type(thing) == DM1_V1_F0175_THING_TYPE_GROUP_PC34) {
            if (out) {
                out->valid = 1;
                out->thing = thing;
                out->nodeIndex = (int)index;
                out->scannedNodeCount = (int)guard + 1;
            }
            return thing;
        }
        thing = nodes[index].nextThing;
        ++guard;
    }

    if (out) {
        out->scannedNodeCount = (int)guard;
    }
    return DM1_V1_F0175_THING_END_OF_LIST_PC34;
}

static int creature_occupies_cell(
    const DM1_V1_GroupCellContextF0176Pc34 *group,
    int creatureIndex,
    int cell,
    int *matchedCell)
{
    int sourceCell;
    int queryCell = cell & 3;

    if (matchedCell) {
        *matchedCell = -1;
    }
    if (!group || creatureIndex < 0 ||
        creatureIndex > group->creatureCountMinusOne ||
        creatureIndex >= DM1_V1_F0176_MAX_CREATURE_SLOTS_PC34 ||
        group->creatureHealth[creatureIndex] <= 0) {
        return 0;
    }
    if (group->packedCells == DM1_V1_F0176_SINGLE_CENTERED_CREATURE_PC34) {
        if (creatureIndex == 0) {
            if (matchedCell) {
                *matchedCell = queryCell;
            }
            return 1;
        }
        return 0;
    }

    sourceCell = creature_cell(group->packedCells, creatureIndex);
    if (group->creatureSize == DM1_V1_F0176_CREATURE_SIZE_HALF_SQUARE_PC34) {
        if ((group->groupDirection & 1) == (queryCell & 1)) {
            queryCell = (queryCell + 3) & 3;
        }
        if (sourceCell == queryCell || sourceCell == ((queryCell + 1) & 3)) {
            if (matchedCell) {
                *matchedCell = queryCell;
            }
            return 1;
        }
        return 0;
    }

    if (sourceCell == queryCell) {
        if (matchedCell) {
            *matchedCell = queryCell;
        }
        return 1;
    }
    return 0;
}

int F0176_GROUP_GetCreatureOrdinalInCell(
    const DM1_V1_GroupCellContextF0176Pc34 *group,
    int cell,
    DM1_V1_CreatureOrdinalResultF0176Pc34 *out)
{
    int i;

    clear_f0176(out);
    if (!group || !valid_creature_count_minus_one(group->creatureCountMinusOne) ||
        cell < 0 || cell > 3) {
        return 0;
    }
    if (group->packedCells == DM1_V1_F0176_SINGLE_CENTERED_CREATURE_PC34) {
        if (group->creatureHealth[0] > 0) {
            if (out) {
                out->valid = 1;
                out->creatureOrdinal = 1;
                out->creatureIndex = 0;
                out->queryCell = cell;
                out->matchedCell = cell;
                out->singleCentered = 1;
            }
            return 1;
        }
        return 0;
    }

    for (i = group->creatureCountMinusOne; i >= 0; --i) {
        int matchedCell = -1;
        if (creature_occupies_cell(group, i, cell, &matchedCell)) {
            if (out) {
                out->valid = 1;
                out->creatureOrdinal = i + 1;
                out->creatureIndex = i;
                out->queryCell = cell;
                out->matchedCell = matchedCell;
            }
            return i + 1;
        }
    }
    return 0;
}

int F0177_GROUP_GetMeleeTargetCreatureOrdinal(
    const DM1_V1_MeleeTargetInputF0177Pc34 *input,
    DM1_V1_MeleeTargetResultF0177Pc34 *out)
{
    unsigned int cellSource;
    unsigned int tableIndex;
    const int *row;
    int i;

    clear_f0177(out);
    if (!input || !input->group ||
        !valid_creature_count_minus_one(input->group->creatureCountMinusOne) ||
        input->championCell < 0 || input->championCell > 3) {
        return 0;
    }

    if (out) {
        out->valid = 1;
        for (i = 0; i <= input->group->creatureCountMinusOne; ++i) {
            if (out->firstLivingCreatureIndex < 0 &&
                input->group->creatureHealth[i] > 0) {
                out->firstLivingCreatureIndex = i;
            }
        }
    }

    if (input->group->packedCells ==
        DM1_V1_F0176_SINGLE_CENTERED_CREATURE_PC34) {
        if (input->group->creatureHealth[0] > 0) {
            if (out) {
                out->creatureOrdinal = 1;
                out->creatureIndex = 0;
                out->singleCentered = 1;
            }
            return 1;
        }
        return 0;
    }

    cellSource = (unsigned int)(input->championCell & 3);
    if (((input->targetDirection & 1) == 0) &&
        ((input->targetDirection & 3) < 4)) {
        cellSource = (cellSource + 1) & 3;
    }
    tableIndex = ((unsigned int)(input->targetDirection & 3) << 1) |
        ((cellSource >> 1) & 1u);
    if (tableIndex > 7u) {
        tableIndex = 0u;
    }
    row = kOrderedCellsToAttack[tableIndex];
    if (out) {
        for (i = 0; i < 4; ++i) {
            out->orderedCells[i] = row[i];
        }
        out->orderedCellCount = 4;
    }

    for (i = 0; i < 4; ++i) {
        DM1_V1_CreatureOrdinalResultF0176Pc34 ordinal;
        int result = F0176_GROUP_GetCreatureOrdinalInCell(
            input->group,
            row[i],
            &ordinal);
        if (result != 0) {
            if (out) {
                out->creatureOrdinal = ordinal.creatureOrdinal;
                out->creatureIndex = ordinal.creatureIndex;
            }
            return result;
        }
    }
    return 0;
}
