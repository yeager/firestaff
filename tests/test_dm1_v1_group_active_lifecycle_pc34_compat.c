#include "dm1_v1_group_active_lifecycle_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_constants(void)
{
    int mask = DM1_V1_F0182_ATTACKING_ASPECT_MASK_PC34;
    int present = DM1_V1_F0195_SQUARE_THING_LIST_PRESENT_PC34;
    int groupType = DM1_V1_F0195_GROUP_THING_TYPE_PC34;
    int endOfList = DM1_V1_F0195_THING_END_OF_LIST_PC34;
    int minCount = DM1_V1_F0196_MINIMUM_ACTIVE_GROUP_COUNT_PC34;
    (void)mask;
    (void)present;
    (void)groupType;
    (void)endOfList;
    (void)minCount;
    assert(mask == 0x80);
    assert(present == 0x10);
    assert(groupType == 4);
    assert(endOfList == 0xFFFE);
    assert(minCount == 110);
}

static void test_stop_attacking(void)
{
    DM1_V1_F0182_ActiveGroupPc34Compat ag;
    memset(&ag, 0, sizeof(ag));
    ag.aspect[0] = 0x80;
    F0182_GROUP_StopAttacking(&ag, 0, 0, NULL, NULL);
    assert((ag.aspect[0] & 0x80) == 0);
}

static void test_initialize_active_groups(void)
{
    DM1_V1_F0196_InitializeActiveGroupsInputPc34Compat input;
    DM1_V1_F0196_ActiveGroupSlotPc34Compat slots[110];
    int rc;

    memset(&input, 0, sizeof(input));
    memset(slots, 0, sizeof(slots));
    input.newGame = 1;
    input.maximumActiveGroupCount = 110;
    input.slots = slots;
    input.slotCapacity = 110;
    rc = F0196_GROUP_InitializeActiveGroups(&input);
    (void)rc;
    assert(rc >= 0);
}

static void test_source_evidence(void)
{
    const char *ev = DM1_V1_GroupActiveLifecycle_SourceEvidencePc34();
    (void)ev;
    assert(ev != NULL);
    assert(ev[0] != '\0');
}

int main(void)
{
    test_constants();
    test_stop_attacking();
    test_initialize_active_groups();
    test_source_evidence();
    puts("ok: DM1 group active lifecycle (Q-DM1-04) 4 tests passed");
    return 0;
}
