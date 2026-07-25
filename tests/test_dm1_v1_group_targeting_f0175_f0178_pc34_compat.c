#include "dm1_v1_group_targeting_f0175_f0178_pc34_compat.h"

#include <assert.h>
#include <string.h>

static uint16_t thing_ref(unsigned int type, unsigned int index)
{
    return (uint16_t)(((type & 0x0fu) << 10) | (index & 0x03ffu));
}

static DM1_V1_GroupCellContextF0176Pc34 make_group(void)
{
    DM1_V1_GroupCellContextF0176Pc34 group;
    memset(&group, 0, sizeof(group));
    group.creatureCountMinusOne = 3;
    group.groupDirection = 0;
    group.creatureSize = DM1_V1_F0176_CREATURE_SIZE_QUARTER_SQUARE_PC34;
    group.packedCells = 0;
    group.packedCells = (uint8_t)F0178_GROUP_GetGroupValueUpdatedWithCreatureValue(
        group.packedCells, 0, 0);
    group.packedCells = (uint8_t)F0178_GROUP_GetGroupValueUpdatedWithCreatureValue(
        group.packedCells, 1, 1);
    group.packedCells = (uint8_t)F0178_GROUP_GetGroupValueUpdatedWithCreatureValue(
        group.packedCells, 2, 2);
    group.packedCells = (uint8_t)F0178_GROUP_GetGroupValueUpdatedWithCreatureValue(
        group.packedCells, 3, 3);
    group.creatureHealth[0] = 12;
    group.creatureHealth[1] = 18;
    group.creatureHealth[2] = 24;
    group.creatureHealth[3] = 30;
    return group;
}

static void test_f0178_packed_update(void)
{
    uint16_t value = 0;

    value = F0178_GROUP_GetGroupValueUpdatedWithCreatureValue(value, 0, 3);
    assert(value == 0x0003u);
    value = F0178_GROUP_GetGroupValueUpdatedWithCreatureValue(value, 1, 2);
    assert(value == 0x000bu);
    value = F0178_GROUP_GetGroupValueUpdatedWithCreatureValue(value, 0, 1);
    assert(value == 0x0009u);
    value = F0178_GROUP_GetGroupValueUpdatedWithCreatureValue(value, 3, 7);
    assert(value == 0x00c9u);
    assert(F0178_GROUP_GetGroupValueUpdatedWithCreatureValue(value, -1, 0) ==
           value);
    assert(F0178_GROUP_GetGroupValueUpdatedWithCreatureValue(value, 4, 0) ==
           value);
}

static void test_f0175_group_scan(void)
{
    DM1_V1_GroupThingNodeF0175Pc34 nodes[5];
    DM1_V1_GroupThingResultF0175Pc34 result;
    (void)result;
    uint16_t first = thing_ref(5, 1);
    (void)first;
    uint16_t groupThing = thing_ref(DM1_V1_F0175_THING_TYPE_GROUP_PC34, 3);

    memset(nodes, 0, sizeof(nodes));
    nodes[1].thing = thing_ref(5, 1);
    nodes[1].nextThing = thing_ref(10, 2);
    nodes[2].thing = thing_ref(10, 2);
    nodes[2].nextThing = groupThing;
    nodes[3].thing = groupThing;
    nodes[3].nextThing = thing_ref(6, 4);
    nodes[4].thing = thing_ref(6, 4);
    nodes[4].nextThing = DM1_V1_F0175_THING_END_OF_LIST_PC34;

    assert(F0175_GROUP_GetThing(nodes, 5, first, &result) == groupThing);
    assert(result.valid == 1);
    assert(result.thing == groupThing);
    assert(result.nodeIndex == 3);
    assert(result.scannedNodeCount == 3);

    assert(F0175_GROUP_GetThing(
               nodes, 5, thing_ref(6, 4), &result) ==
           DM1_V1_F0175_THING_END_OF_LIST_PC34);
    assert(result.valid == 0);
    assert(result.thing == DM1_V1_F0175_THING_END_OF_LIST_PC34);

    assert(F0175_GROUP_GetThing(
               nodes, 5, thing_ref(7, 9), &result) ==
           DM1_V1_F0175_THING_END_OF_LIST_PC34);
    assert(result.valid == 0);
}

static void test_f0176_cell_ordinals(void)
{
    DM1_V1_GroupCellContextF0176Pc34 group = make_group();
    DM1_V1_CreatureOrdinalResultF0176Pc34 result;
    (void)result;

    assert(F0176_GROUP_GetCreatureOrdinalInCell(&group, 2, &result) == 3);
    assert(result.valid == 1);
    assert(result.creatureOrdinal == 3);
    assert(result.creatureIndex == 2);
    assert(result.queryCell == 2);
    assert(result.matchedCell == 2);

    group.creatureHealth[2] = 0;
    assert(F0176_GROUP_GetCreatureOrdinalInCell(&group, 2, &result) == 0);
    assert(result.valid == 0);

    group.creatureHealth[1] = 18;
    group.creatureHealth[2] = 24;
    group.creatureSize = DM1_V1_F0176_CREATURE_SIZE_HALF_SQUARE_PC34;
    group.groupDirection = 0;
    assert(F0176_GROUP_GetCreatureOrdinalInCell(&group, 2, &result) == 3);
    assert(result.creatureOrdinal == 3);
    assert(result.creatureIndex == 2);
    assert(result.matchedCell == 1);

    group.packedCells = DM1_V1_F0176_SINGLE_CENTERED_CREATURE_PC34;
    assert(F0176_GROUP_GetCreatureOrdinalInCell(&group, 3, &result) == 1);
    assert(result.singleCentered == 1);
    assert(result.creatureOrdinal == 1);

    group.creatureHealth[0] = 0;
    assert(F0176_GROUP_GetCreatureOrdinalInCell(&group, 0, &result) == 0);
    assert(F0176_GROUP_GetCreatureOrdinalInCell(&group, -1, &result) == 0);
}

static void test_f0177_melee_target(void)
{
    DM1_V1_GroupCellContextF0176Pc34 group = make_group();
    DM1_V1_MeleeTargetInputF0177Pc34 input;
    DM1_V1_MeleeTargetResultF0177Pc34 result;
    (void)result;

    memset(&input, 0, sizeof(input));
    input.group = &group;
    input.championCell = 2;
    input.targetDirection = 0;

    assert(F0177_GROUP_GetMeleeTargetCreatureOrdinal(&input, &result) == 2);
    assert(result.valid == 1);
    assert(result.orderedCellCount == 4);
    assert(result.orderedCells[0] == 1);
    assert(result.orderedCells[1] == 0);
    assert(result.orderedCells[2] == 2);
    assert(result.orderedCells[3] == 3);
    assert(result.creatureOrdinal == 2);
    assert(result.creatureIndex == 1);
    assert(result.firstLivingCreatureIndex == 0);

    group.creatureHealth[1] = 0;
    assert(F0177_GROUP_GetMeleeTargetCreatureOrdinal(&input, &result) == 1);
    assert(result.creatureIndex == 0);

    group.packedCells = DM1_V1_F0176_SINGLE_CENTERED_CREATURE_PC34;
    group.creatureHealth[0] = 12;
    assert(F0177_GROUP_GetMeleeTargetCreatureOrdinal(&input, &result) == 1);
    assert(result.singleCentered == 1);
    assert(result.creatureIndex == 0);

    group.creatureHealth[0] = 0;
    assert(F0177_GROUP_GetMeleeTargetCreatureOrdinal(&input, &result) == 0);
    assert(result.valid == 1);
    assert(result.creatureIndex == -1);
}

static void test_fail_closed(void)
{
    DM1_V1_GroupCellContextF0176Pc34 group = make_group();
    DM1_V1_MeleeTargetInputF0177Pc34 input;
    DM1_V1_MeleeTargetResultF0177Pc34 target;
    (void)target;
    DM1_V1_CreatureOrdinalResultF0176Pc34 ordinal;
    (void)ordinal;

    group.creatureCountMinusOne = 4;
    assert(F0176_GROUP_GetCreatureOrdinalInCell(&group, 0, &ordinal) == 0);

    memset(&input, 0, sizeof(input));
    input.group = &group;
    input.championCell = 0;
    assert(F0177_GROUP_GetMeleeTargetCreatureOrdinal(&input, &target) == 0);

    assert(F0176_GROUP_GetCreatureOrdinalInCell(0, 0, &ordinal) == 0);
    assert(F0177_GROUP_GetMeleeTargetCreatureOrdinal(0, &target) == 0);
    assert(F0177_GROUP_GetMeleeTargetCreatureOrdinal(&input, 0) == 0);
}

int main(void)
{
    const char *evidence = DM1_V1_F0175_F0178_SourceEvidencePc34();
    (void)evidence;
    assert(strstr(evidence, "F0175_GROUP_GetThing") != 0);
    assert(strstr(evidence, "F0176_GROUP_GetCreatureOrdinalInCell") != 0);
    assert(strstr(evidence, "F0177_GROUP_GetMeleeTargetCreatureOrdinal") != 0);
    assert(strstr(evidence, "F0178") != 0);

    test_f0178_packed_update();
    test_f0175_group_scan();
    test_f0176_cell_ordinals();
    test_f0177_melee_target();
    test_fail_closed();

    return 0;
}
