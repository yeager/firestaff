#include "dm1_v1_action_list_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
        ++failures; \
    } \
} while (0)

static DM1_V1_ActionListBuildInputPc34 input_for(
    const DM1_V1_ActionSetPc34 *actionSet,
    const unsigned short *skills)
{
    DM1_V1_ActionListBuildInputPc34 input;
    memset(&input, 0, sizeof(input));
    input.actionSet = actionSet;
    input.actionSkillLevels = skills;
    input.actionSkillLevelCount = 44;
    input.actionObjectChargeCount = 1;
    input.actionObjectChargeCountKnown = 1;
    return input;
}

static void test_compacts_only_source_eligible_actions(void)
{
    DM1_V1_ActionSetPc34 actionSet = {{2, 17, 32}, {3, 0x0084}};
    DM1_V1_ActionListPc34 list = {{99, 99, 99}, {99, 99, 99}};
    DM1_V1_ActionListBuildReceiptPc34 receipt;
    unsigned short skills[44] = {0};
    DM1_V1_ActionListBuildInputPc34 input;

    skills[17] = 3;
    skills[32] = 4;
    input = input_for(&actionSet, skills);
    CHECK(dm1_v1_action_list_set_f0383_pc34(&list, &input, &receipt));
    CHECK(receipt.valid && receipt.actionCount == 3);
    CHECK(list.actionIndices[0] == 2 && list.minimumSkillLevel[0] == 1);
    CHECK(list.actionIndices[1] == 17 && list.minimumSkillLevel[1] == 3);
    CHECK(list.actionIndices[2] == 32 && list.minimumSkillLevel[2] == 4);
}

static void test_charge_and_skill_gates_compact_without_fallback(void)
{
    DM1_V1_ActionSetPc34 actionSet = {{2, 17, 32}, {0x0083, 4}};
    DM1_V1_ActionListPc34 list = {{99, 99, 99}, {99, 77, 88}};
    DM1_V1_ActionListBuildReceiptPc34 receipt;
    unsigned short skills[44] = {0};
    DM1_V1_ActionListBuildInputPc34 input;

    skills[17] = 3;
    skills[32] = 3;
    input = input_for(&actionSet, skills);
    input.actionObjectChargeCount = 0;
    CHECK(dm1_v1_action_list_set_f0383_pc34(&list, &input, &receipt));
    CHECK(receipt.actionCount == 1);
    CHECK(list.actionIndices[0] == 2);
    CHECK(list.actionIndices[1] == DM1_V1_ACTION_NONE_PC34);
    CHECK(list.actionIndices[2] == DM1_V1_ACTION_NONE_PC34);
    CHECK(list.minimumSkillLevel[1] == 77 && list.minimumSkillLevel[2] == 88);
}

static void test_incomplete_runtime_receipt_rejects_without_mutation(void)
{
    DM1_V1_ActionSetPc34 actionSet = {{2, 17, DM1_V1_ACTION_NONE_PC34}, {3, 0}};
    DM1_V1_ActionListPc34 list = {{9, 8, 7}, {6, 5, 4}};
    DM1_V1_ActionListPc34 original = list;
    unsigned short skills[17] = {0};
    DM1_V1_ActionListBuildInputPc34 input = input_for(&actionSet, skills);

    input.actionSkillLevelCount = 17;
    CHECK(!dm1_v1_action_list_set_f0383_pc34(&list, &input, NULL));
    CHECK(memcmp(&list, &original, sizeof(list)) == 0);
}

static void test_unknown_charge_receipt_rejects_without_mutation(void)
{
    DM1_V1_ActionSetPc34 actionSet = {{2, 17, DM1_V1_ACTION_NONE_PC34}, {0x0083, 0}};
    DM1_V1_ActionListPc34 list = {{9, 8, 7}, {6, 5, 4}};
    DM1_V1_ActionListPc34 original = list;
    unsigned short skills[44] = {0};
    DM1_V1_ActionListBuildInputPc34 input = input_for(&actionSet, skills);

    skills[17] = 3;
    input.actionObjectChargeCountKnown = 0;
    CHECK(!dm1_v1_action_list_set_f0383_pc34(&list, &input, NULL));
    CHECK(memcmp(&list, &original, sizeof(list)) == 0);
}

int main(void)
{
    test_compacts_only_source_eligible_actions();
    test_charge_and_skill_gates_compact_without_fallback();
    test_incomplete_runtime_receipt_rejects_without_mutation();
    test_unknown_charge_receipt_rejects_without_mutation();
    printf("dm1 F0383 action list: %s\n", failures ? "FAIL" : "PASS");
    return failures != 0;
}
