#ifndef FIRESTAFF_DM1_V1_G0492_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_G0492_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 560 init var
 * G0492_auc_Graphic560_...[44] (PC 3.4 EN init).
 *
 * G0492 is the uint8_t[44] for the action damage factor action/spell table.
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-896.
 */

#define DM1_V1_G0492_PC34_COMPAT_SIZE 44

typedef struct DM1_V1_G0492ResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_G0492_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int allInByteRange;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_G0492ResultPc34;

const unsigned char *
dm1_v1_graphic560_action_damage_factor_table_pc34(void);

int
dm1_v1_graphic560_action_damage_factor_size_pc34(void);

int
dm1_v1_graphic560_action_damage_factor_get_pc34(int entry_index);

int
dm1_v1_graphic560_action_damage_factor_run_pc34(
    DM1_V1_G0492ResultPc34 *out);

#endif
