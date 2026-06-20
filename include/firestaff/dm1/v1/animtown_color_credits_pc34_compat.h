#ifndef FIRESTAFF_DM1_V1_ANIMTOWNCOLORCREDITS_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ANIMTOWNCOLORCREDITS_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for global init var
 * G8147_CREDITS[17] (COLOR_DEF {Index, R, G, B}).
 *
 * G8147 is the 17-entry VGA palette for the credits screen.
 * Color 0x00..0x0F are the foreground colors; entry 16 (0x10)
 * is black background; entry 17 (0xFF) is the sentinel terminator.
 *
 * Read site: ANIMTOWN.C:393-426 G8176_PaletteTable[] dispatch
 * (G8147_CREDITS = index 6 in the table); palette-walk loop at
 * ANIMTOWN.C:624-629 reads until Index == 0xFF.
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 */

#define DM1_V1_ANIMTOWN_COLOR_CREDITS_PC34_COMPAT_SIZE 17

typedef struct DM1_V1_AnimtownColorCreditsEntryPc34 {
    unsigned char index;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} DM1_V1_AnimtownColorCreditsEntryPc34;

typedef struct DM1_V1_AnimtownColorCreditsResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_ANIMTOWN_COLOR_CREDITS_PC34_COMPAT_SIZE * 4];
    int tableSize;
    int tableMatchesDeclaration;
    int firstEntryIndex0;
    int firstEntryColorBlack;
    int secondEntryIndex1DarkColor;
    int lastEntrySentinelIndex0xFF;
    int allRgbInByteRange;
    int allIndicesNonZeroExceptLast;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_AnimtownColorCreditsResultPc34;

const unsigned char *
dm1_v1_animtown_color_credits_table_pc34(void);

int
dm1_v1_animtown_color_credits_size_pc34(void);

int
dm1_v1_animtown_color_credits_get_pc34(int entry_index, int field_index);

int
dm1_v1_animtown_color_credits_run_pc34(
    DM1_V1_AnimtownColorCreditsResultPc34 *out);

#endif