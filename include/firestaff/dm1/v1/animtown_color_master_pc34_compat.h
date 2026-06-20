#ifndef FIRESTAFF_DM1_V1_ANIMTOWN_COLOR_MASTER_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ANIMTOWN_COLOR_MASTER_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for global init var
 * G8161_MASTER[2] (COLOR_DEF {Index, R, G, B}).
 *
 * G8161_MASTER is the COLOR_DEF palette for the Master viewport/title
 * /swoosh animation.
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-883.
 */

#define DM1_V1_ANIMTOWN_COLOR_MASTER_PC34_COMPAT_SIZE 2

typedef struct DM1_V1_AnimtownColorMasterEntryPc34 {
    unsigned char index;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} DM1_V1_AnimtownColorMasterEntryPc34;

typedef struct DM1_V1_AnimtownColorMasterResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_ANIMTOWN_COLOR_MASTER_PC34_COMPAT_SIZE * 4];
    int tableSize;
    int tableMatchesDeclaration;
    int lastEntrySentinelIndex0xFF;
    int allRgbInByteRange;
    int allIndicesNonZeroExceptLast;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_AnimtownColorMasterResultPc34;

const unsigned char *
dm1_v1_animtown_color_master_table_pc34(void);

int
dm1_v1_animtown_color_master_size_pc34(void);

int
dm1_v1_animtown_color_master_get_pc34(int entry_index, int field_index);

int
dm1_v1_animtown_color_master_run_pc34(
    DM1_V1_AnimtownColorMasterResultPc34 *out);

#endif
