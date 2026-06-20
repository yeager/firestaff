#ifndef FIRESTAFF_DM1_V1_ANIMTOWN_COLOR_DUNGEON_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ANIMTOWN_COLOR_DUNGEON_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for global init var
 * G8160_DUNGEON[9] (COLOR_DEF {Index, R, G, B}).
 *
 * G8160_DUNGEON is the COLOR_DEF palette for the Dungeon viewport/title
 * /swoosh animation.
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-883.
 */

#define DM1_V1_ANIMTOWN_COLOR_DUNGEON_PC34_COMPAT_SIZE 9

typedef struct DM1_V1_AnimtownColorDungeonEntryPc34 {
    unsigned char index;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} DM1_V1_AnimtownColorDungeonEntryPc34;

typedef struct DM1_V1_AnimtownColorDungeonResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_ANIMTOWN_COLOR_DUNGEON_PC34_COMPAT_SIZE * 4];
    int tableSize;
    int tableMatchesDeclaration;
    int lastEntrySentinelIndex0xFF;
    int allRgbInByteRange;
    int allIndicesNonZeroExceptLast;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_AnimtownColorDungeonResultPc34;

const unsigned char *
dm1_v1_animtown_color_dungeon_table_pc34(void);

int
dm1_v1_animtown_color_dungeon_size_pc34(void);

int
dm1_v1_animtown_color_dungeon_get_pc34(int entry_index, int field_index);

int
dm1_v1_animtown_color_dungeon_run_pc34(
    DM1_V1_AnimtownColorDungeonResultPc34 *out);

#endif
