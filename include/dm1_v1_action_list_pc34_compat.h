#ifndef FIRESTAFF_DM1_V1_ACTION_LIST_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ACTION_LIST_PC34_COMPAT_H

/*
 * ReDMCSB MENU.C F0383_MENUS_SetActionList, PC 3.4 route.
 *
 * The original writes G0713_s_ActionList in place: its first action is
 * always retained, while the two optional actions are compacted only when
 * their charge and skill gates pass.  This is deliberately a state mutation
 * rather than a host-side menu approximation.
 */

#define DM1_V1_ACTION_LIST_CAPACITY_PC34 3
#define DM1_V1_ACTION_NONE_PC34 0xff
#define DM1_V1_ACTION_REQUIRES_CHARGE_PC34 0x0080

typedef struct {
    unsigned char actionIndices[DM1_V1_ACTION_LIST_CAPACITY_PC34];
    unsigned short actionProperties[DM1_V1_ACTION_LIST_CAPACITY_PC34 - 1];
} DM1_V1_ActionSetPc34;

typedef struct {
    unsigned char actionIndices[DM1_V1_ACTION_LIST_CAPACITY_PC34];
    unsigned short minimumSkillLevel[DM1_V1_ACTION_LIST_CAPACITY_PC34];
} DM1_V1_ActionListPc34;

typedef struct {
    const DM1_V1_ActionSetPc34 *actionSet;
    const unsigned short *actionSkillLevels;
    int actionSkillLevelCount;
    int actionObjectChargeCount;
    int actionObjectChargeCountKnown;
} DM1_V1_ActionListBuildInputPc34;

typedef struct {
    int valid;
    int actionCount;
} DM1_V1_ActionListBuildReceiptPc34;

/*
 * Applies F0383 to list in place. actionSkillLevels is indexed by the
 * Graphic560 action index and must cover every optional action considered.
 * A missing charge/skill receipt rejects the update and leaves list intact.
 */
int dm1_v1_action_list_set_f0383_pc34(
    DM1_V1_ActionListPc34 *list,
    const DM1_V1_ActionListBuildInputPc34 *input,
    DM1_V1_ActionListBuildReceiptPc34 *receipt);

#endif
