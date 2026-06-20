#ifndef FIRESTAFF_DM1_V1_G0500_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_G0500_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 560 init var
 * G0500_auc_Graphic560_...[4] (PC 3.4 EN init).
 *
 * G0500 is the int16_t[4] = {L, R, T, B} box for the box action area 2 actions menu action/spell table.
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-896.
 */

#define DM1_V1_G0500_PC34_COMPAT_SIZE 4

typedef struct DM1_V1_G0500ResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_G0500_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int allInByteRange;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_G0500ResultPc34;

const unsigned char *
dm1_v1_graphic560_box_action_area_2_actions_menu_table_pc34(void);

int
dm1_v1_graphic560_box_action_area_2_actions_menu_size_pc34(void);

int
dm1_v1_graphic560_box_action_area_2_actions_menu_get_pc34(int entry_index);

int
dm1_v1_graphic560_box_action_area_2_actions_menu_run_pc34(
    DM1_V1_G0500ResultPc34 *out);

#endif
