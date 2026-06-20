#ifndef FIRESTAFF_DM1_V1_ANIMTOWNCOLORICON_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ANIMTOWNCOLORICON_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for global init var
 * G8149_ICON[17] (COLOR_DEF {Index, R, G, B}).
 *
 * G8149_ICON is the 17-entry VGA palette for the icon screen.
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-873.
 */

#define DM1_V1_ANIMTOWN_COLOR_ICON_PC34_COMPAT_SIZE 17

typedef struct DM1_V1_AnimtownColorIconEntryPc34 {
    unsigned char index;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} DM1_V1_AnimtownColorIconEntryPc34;

typedef struct DM1_V1_AnimtownColorIconResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_ANIMTOWN_COLOR_ICON_PC34_COMPAT_SIZE * 4];
    int tableSize;
    int tableMatchesDeclaration;
    int firstEntryIndexZero;
    int lastEntrySentinelIndex0xFF;
    int allRgbInByteRange;
    int allIndicesNonZeroExceptLast;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_AnimtownColorIconResultPc34;

const unsigned char *
dm1_v1_animtown_color_icon_table_pc34(void);

int
dm1_v1_animtown_color_icon_size_pc34(void);

int
dm1_v1_animtown_color_icon_get_pc34(int entry_index, int field_index);

int
dm1_v1_animtown_color_icon_run_pc34(
    DM1_V1_AnimtownColorIconResultPc34 *out);

#endif
