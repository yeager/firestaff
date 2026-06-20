#ifndef FIRESTAFF_DM1_V1_ANIMTOWNCOLORINVENTORY_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ANIMTOWNCOLORINVENTORY_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for global init var
 * G8150_INVENTORY[17] (COLOR_DEF {Index, R, G, B}).
 *
 * G8150_INVENTORY is the 17-entry VGA palette for the inventory screen.
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-873.
 */

#define DM1_V1_ANIMTOWN_COLOR_INVENTORY_PC34_COMPAT_SIZE 17

typedef struct DM1_V1_AnimtownColorInventoryEntryPc34 {
    unsigned char index;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} DM1_V1_AnimtownColorInventoryEntryPc34;

typedef struct DM1_V1_AnimtownColorInventoryResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_ANIMTOWN_COLOR_INVENTORY_PC34_COMPAT_SIZE * 4];
    int tableSize;
    int tableMatchesDeclaration;
    int firstEntryIndexZero;
    int lastEntrySentinelIndex0xFF;
    int allRgbInByteRange;
    int allIndicesNonZeroExceptLast;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_AnimtownColorInventoryResultPc34;

const unsigned char *
dm1_v1_animtown_color_inventory_table_pc34(void);

int
dm1_v1_animtown_color_inventory_size_pc34(void);

int
dm1_v1_animtown_color_inventory_get_pc34(int entry_index, int field_index);

int
dm1_v1_animtown_color_inventory_run_pc34(
    DM1_V1_AnimtownColorInventoryResultPc34 *out);

#endif
