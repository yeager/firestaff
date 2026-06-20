#ifndef FIRESTAFF_DM1_V1_G0485_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_G0485_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 560 init var
 * G0485_auc_Graphic560_...[24] (PC 3.4 EN init).
 *
 * G0485 is the uint8_t[4][6] for the symbol base mana cost action/spell table.
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-896.
 */

#define DM1_V1_G0485_PC34_COMPAT_SIZE 24

typedef struct DM1_V1_G0485ResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_G0485_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int allInByteRange;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_G0485ResultPc34;

const unsigned char *
dm1_v1_graphic560_symbol_base_mana_cost_table_pc34(void);

int
dm1_v1_graphic560_symbol_base_mana_cost_size_pc34(void);

int
dm1_v1_graphic560_symbol_base_mana_cost_get_pc34(int entry_index);

int
dm1_v1_graphic560_symbol_base_mana_cost_run_pc34(
    DM1_V1_G0485ResultPc34 *out);

#endif
