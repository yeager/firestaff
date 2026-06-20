#ifndef FIRESTAFF_DM1_V1_ANIMTOWN_COLOR_SWOOSH10_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ANIMTOWN_COLOR_SWOOSH10_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for global init var
 * G8171_SWOOSH10[3] (COLOR_DEF {Index, R, G, B}).
 *
 * G8171_SWOOSH10 is the COLOR_DEF palette for the Swoosh10 viewport/title
 * /swoosh animation.
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-883.
 */

#define DM1_V1_ANIMTOWN_COLOR_SWOOSH10_PC34_COMPAT_SIZE 3

typedef struct DM1_V1_AnimtownColorSwoosh10EntryPc34 {
    unsigned char index;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} DM1_V1_AnimtownColorSwoosh10EntryPc34;

typedef struct DM1_V1_AnimtownColorSwoosh10ResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_ANIMTOWN_COLOR_SWOOSH10_PC34_COMPAT_SIZE * 4];
    int tableSize;
    int tableMatchesDeclaration;
    int lastEntrySentinelIndex0xFF;
    int allRgbInByteRange;
    int allIndicesNonZeroExceptLast;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_AnimtownColorSwoosh10ResultPc34;

const unsigned char *
dm1_v1_animtown_color_swoosh10_table_pc34(void);

int
dm1_v1_animtown_color_swoosh10_size_pc34(void);

int
dm1_v1_animtown_color_swoosh10_get_pc34(int entry_index, int field_index);

int
dm1_v1_animtown_color_swoosh10_run_pc34(
    DM1_V1_AnimtownColorSwoosh10ResultPc34 *out);

#endif
