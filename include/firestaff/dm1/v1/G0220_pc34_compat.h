#ifndef FIRESTAFF_DM1_V1_G0220_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_G0220_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 558 init var
 * G0220_as_Graphic558_CreatureReplacementColorSets[13] (CREATURE_REPLACEMENT_COLOR_SET struct).
 *
 * Each CREATURE_REPLACEMENT_COLOR_SET entry = {RGBColor[6] (12 bytes
 * big-endian), D2ReplacementColor (1 byte), D3ReplacementColor (1 byte)}
 * = 14 bytes/entry. Total = 13 × 14 = 182 bytes.
 *
 * Read sites: DUNVIEW.C F0099/F0654 (creature palette replacement).
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-1042.
 */

#define DM1_V1_G0220_PC34_COMPAT_SIZE 182

typedef struct DM1_V1_G0220ResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_G0220_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int allBytesInByteRange;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_G0220ResultPc34;

const unsigned char *
dm1_v1_g0220_table_pc34(void);

int
dm1_v1_g0220_size_pc34(void);

int
dm1_v1_g0220_get_pc34(int entry_index, int byte_offset);

int
dm1_v1_g0220_run_pc34(
    DM1_V1_G0220ResultPc34 *out);

#endif
