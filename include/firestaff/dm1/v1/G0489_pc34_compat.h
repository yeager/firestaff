#ifndef FIRESTAFF_DM1_V1_G0489_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_G0489_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 560 init var
 * G0489_as_Graphic560_ActionSets[44] (ACTION_SET struct array).
 *
 * G0489 is the 44-entry action-set definition table. Each ACTION_SET
 * is {ActionIndices[3], ActionProperties[2], Useless} = 6 bytes × 44
 * = 264 bytes total. Note: PC 3.4 EN has 6 bytes per entry (no padding)
 * vs Amiga 7 bytes per entry (with 1 byte of padding inserted by compiler).
 *
 * Read sites: MENU.C F0452_ACTIONS_GetActionSet +
 * F0412_MENUS_GetChampionSpellCastResult.
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-1035.
 */

#define DM1_V1_G0489_PC34_COMPAT_SIZE 264

typedef struct DM1_V1_G0489ResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_G0489_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int entry0AllZeroValid;
    int allBytesInByteRange;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_G0489ResultPc34;

const unsigned char *
dm1_v1_g0489_table_pc34(void);

int
dm1_v1_g0489_size_pc34(void);

int
dm1_v1_g0489_get_pc34(int action_index, int byte_offset);

int
dm1_v1_g0489_run_pc34(
    DM1_V1_G0489ResultPc34 *out);

#endif
