#ifndef FIRESTAFF_DM1_V1_ANIMTOWN_COLOR_LIGHT5_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ANIMTOWN_COLOR_LIGHT5_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for global init var
 * G8156_LIGHT5[17] (COLOR_DEF {Index, R, G, B}).
 *
 * G8156_LIGHT5 is the 17-entry VGA palette for the light5 dungeon-viewport
 * lighting preset.
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-877.
 */

#define DM1_V1_ANIMTOWN_COLOR_LIGHT5_PC34_COMPAT_SIZE 17

typedef struct DM1_V1_AnimtownColorLight5EntryPc34 {
    unsigned char index;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} DM1_V1_AnimtownColorLight5EntryPc34;

typedef struct DM1_V1_AnimtownColorLight5ResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_ANIMTOWN_COLOR_LIGHT5_PC34_COMPAT_SIZE * 4];
    int tableSize;
    int tableMatchesDeclaration;
    int firstEntryIndex0x10;
    int lastEntrySentinelIndex0xFF;
    int allRgbInByteRange;
    int allIndicesNonZeroExceptLast;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_AnimtownColorLight5ResultPc34;

const unsigned char *
dm1_v1_animtown_color_light5_table_pc34(void);

int
dm1_v1_animtown_color_light5_size_pc34(void);

int
dm1_v1_animtown_color_light5_get_pc34(int entry_index, int field_index);

int
dm1_v1_animtown_color_light5_run_pc34(
    DM1_V1_AnimtownColorLight5ResultPc34 *out);

#endif
