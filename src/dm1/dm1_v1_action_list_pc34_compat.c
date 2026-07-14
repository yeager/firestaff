#include "dm1_v1_action_list_pc34_compat.h"

#include <string.h>

static int dm1_v1_action_list_has_skill_receipt(
    const DM1_V1_ActionListBuildInputPc34 *input,
    unsigned char actionIndex)
{
    return input->actionSkillLevels &&
           actionIndex != DM1_V1_ACTION_NONE_PC34 &&
           actionIndex < input->actionSkillLevelCount;
}

int dm1_v1_action_list_set_f0383_pc34(
    DM1_V1_ActionListPc34 *list,
    const DM1_V1_ActionListBuildInputPc34 *input,
    DM1_V1_ActionListBuildReceiptPc34 *receipt)
{
    DM1_V1_ActionListPc34 next;
    unsigned int nextAvailableIndex;
    unsigned int actionListIndex;

    if (receipt) memset(receipt, 0, sizeof(*receipt));
    if (!list || !input || !input->actionSet ||
        !input->actionSkillLevels || input->actionSkillLevelCount <= 0) {
        return 0;
    }

    if (input->actionSet->actionIndices[0] == DM1_V1_ACTION_NONE_PC34) {
        return 0;
    }

    /* Preserve the source's untouched minimum-skill words in vacant slots. */
    next = *list;
    next.actionIndices[0] = input->actionSet->actionIndices[0];
    next.minimumSkillLevel[0] = 1;
    nextAvailableIndex = 1;

    for (actionListIndex = 1;
         actionListIndex < DM1_V1_ACTION_LIST_CAPACITY_PC34;
         ++actionListIndex) {
        unsigned char actionIndex = input->actionSet->actionIndices[actionListIndex];
        unsigned short minimumSkillLevel;

        if (actionIndex == DM1_V1_ACTION_NONE_PC34) continue;
        if (!dm1_v1_action_list_has_skill_receipt(input, actionIndex)) return 0;

        minimumSkillLevel = input->actionSet->actionProperties[actionListIndex - 1];
        if ((minimumSkillLevel & DM1_V1_ACTION_REQUIRES_CHARGE_PC34) != 0) {
            if (!input->actionObjectChargeCountKnown) return 0;
            if (input->actionObjectChargeCount <= 0) continue;
        }
        minimumSkillLevel &= (unsigned short)~DM1_V1_ACTION_REQUIRES_CHARGE_PC34;
        if (input->actionSkillLevels[actionIndex] < minimumSkillLevel) continue;

        next.actionIndices[nextAvailableIndex] = actionIndex;
        next.minimumSkillLevel[nextAvailableIndex] = minimumSkillLevel;
        ++nextAvailableIndex;
    }

    for (actionListIndex = nextAvailableIndex;
         actionListIndex < DM1_V1_ACTION_LIST_CAPACITY_PC34;
         ++actionListIndex) {
        next.actionIndices[actionListIndex] = DM1_V1_ACTION_NONE_PC34;
    }

    *list = next;
    if (receipt) {
        receipt->valid = 1;
        receipt->actionCount = (int)nextAvailableIndex;
    }
    return 1;
}
