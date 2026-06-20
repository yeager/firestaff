#ifndef FIRESTAFF_DM1_V1_G0490_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_G0490_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 560 init var
 * G0490_ac_Graphic560_ActionNames[300] (PC 3.4 EN action names string).
 *
 * G0490 is the 300-byte null-separated action name string. 44 names
 * packed: "N\0BLOCK\0CHOP\0X\0BLOW HORN\0..." (288 actual chars
 * for 44 names, plus 12 trailing zero-padding to fill char[300]).
 *
 * Read sites: MENU.C F0452/F0456/F0412 (action dispatch).
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-1036.
 */

#define DM1_V1_G0490_PC34_COMPAT_SIZE 300

typedef struct DM1_V1_G0490ResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_G0490_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int entry0IsN;
    int entry1IsBLOCK;
    int entry43IsFUSE;
    int exactly44Names;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_G0490ResultPc34;

const unsigned char *
dm1_v1_g0490_table_pc34(void);

int
dm1_v1_g0490_size_pc34(void);

int
dm1_v1_g0490_get_pc34(int byte_index);

int
dm1_v1_g0490_run_pc34(
    DM1_V1_G0490ResultPc34 *out);

#endif
