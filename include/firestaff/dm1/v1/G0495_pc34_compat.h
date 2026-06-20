#ifndef FIRESTAFF_DM1_V1_G0495_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_G0495_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 560 init var
 * G0495_auc_Graphic560_...[44] (PC 3.4 EN init).
 *
 * G0495 is the int8_t[44] for the action defense action/spell table.
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-896.
 */

#define DM1_V1_G0495_PC34_COMPAT_SIZE 44

typedef struct DM1_V1_G0495ResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_G0495_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int allInByteRange;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_G0495ResultPc34;

const unsigned char *
dm1_v1_graphic560_action_defense_table_pc34(void);

int
dm1_v1_graphic560_action_defense_size_pc34(void);

int
dm1_v1_graphic560_action_defense_get_pc34(int entry_index);

int
dm1_v1_graphic560_action_defense_run_pc34(
    DM1_V1_G0495ResultPc34 *out);

#endif
