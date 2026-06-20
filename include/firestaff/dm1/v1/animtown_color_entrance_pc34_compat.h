#ifndef FIRESTAFF_DM1_V1_ANIMTOWNCOLORENTRANCE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ANIMTOWNCOLORENTRANCE_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for global init var
 * G8148_ENTRANCE[17] (COLOR_DEF {Index, R, G, B}).
 *
 * G8148_ENTRANCE is the 17-entry VGA palette for the entrance screen.
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-873.
 */

#define DM1_V1_ANIMTOWN_COLOR_ENTRANCE_PC34_COMPAT_SIZE 17

typedef struct DM1_V1_AnimtownColorEntranceEntryPc34 {
    unsigned char index;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} DM1_V1_AnimtownColorEntranceEntryPc34;

typedef struct DM1_V1_AnimtownColorEntranceResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_ANIMTOWN_COLOR_ENTRANCE_PC34_COMPAT_SIZE * 4];
    int tableSize;
    int tableMatchesDeclaration;
    int firstEntryIndexZero;
    int lastEntrySentinelIndex0xFF;
    int allRgbInByteRange;
    int allIndicesNonZeroExceptLast;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_AnimtownColorEntranceResultPc34;

const unsigned char *
dm1_v1_animtown_color_entrance_table_pc34(void);

int
dm1_v1_animtown_color_entrance_size_pc34(void);

int
dm1_v1_animtown_color_entrance_get_pc34(int entry_index, int field_index);

int
dm1_v1_animtown_color_entrance_run_pc34(
    DM1_V1_AnimtownColorEntranceResultPc34 *out);

#endif
