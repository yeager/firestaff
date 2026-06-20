#ifndef FIRESTAFF_DM1_V1_ANIMTOWN_COLOR_LIGHT3_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ANIMTOWN_COLOR_LIGHT3_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for global init var
 * G8154_LIGHT3[17] (COLOR_DEF {Index, R, G, B}).
 *
 * G8154_LIGHT3 is the 17-entry VGA palette for the light3 dungeon-viewport
 * lighting preset.
 * BUG0_00 quirk: Entry 7 has Index=0x18 (should be 0x17 per surrounding pattern) — captured as BUG0_00 quirk.
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-877.
 */

#define DM1_V1_ANIMTOWN_COLOR_LIGHT3_PC34_COMPAT_SIZE 17

typedef struct DM1_V1_AnimtownColorLight3EntryPc34 {
    unsigned char index;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} DM1_V1_AnimtownColorLight3EntryPc34;

typedef struct DM1_V1_AnimtownColorLight3ResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_ANIMTOWN_COLOR_LIGHT3_PC34_COMPAT_SIZE * 4];
    int tableSize;
    int tableMatchesDeclaration;
    int firstEntryIndex0x10;
    int lastEntrySentinelIndex0xFF;
    int allRgbInByteRange;
    int allIndicesNonZeroExceptLast;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_AnimtownColorLight3ResultPc34;

const unsigned char *
dm1_v1_animtown_color_light3_table_pc34(void);

int
dm1_v1_animtown_color_light3_size_pc34(void);

int
dm1_v1_animtown_color_light3_get_pc34(int entry_index, int field_index);

int
dm1_v1_animtown_color_light3_run_pc34(
    DM1_V1_AnimtownColorLight3ResultPc34 *out);

#endif
