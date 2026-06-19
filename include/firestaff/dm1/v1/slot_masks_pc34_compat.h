#ifndef FIRESTAFF_DM1_V1_SLOT_MASKS_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_SLOT_MASKS_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0038_ai_Graphic562_SlotMasks[38].
 *
 * G0038 is the per-slot allowed-slots bitmask table used by
 * CHAMPION.C and REVIVE.C to validate whether a given thing is
 * allowed to be placed in a given slot. Each entry maps a slot
 * index to a bitmask of allowed thing types.
 *
 * Partition (38 entries):
 *   0..7 : 8 status-box hands (matching G0030 status-hand partition)
 *   8..37: 30 inventory slots + 8 chest slots (matching G0030
 *           inventory + chest partitions; though G0038 has 30 + 8
 *           same as G0030's 30 inventory + 8 chest = 38 entries)
 *
 * Init value (DATA.C:320-358, PC 3.4 + post-1.3 Atari):
 *   - Ready Hand, Action Hand: MASK0xFFFF_ANY_SLOT (any item)
 *   - Head, Torso, Legs, Feet: 0x0002/0x0008/0x0010/0x0020 (single bit each)
 *   - Pouch: 0x0100 (pouch + pass-through doors)
 *   - Quiver Line1/Line2: 0x0040/0x0080
 *   - Neck: 0x0004
 *   - Backpack Line1/Line2 (all 18 slots): 0xFFFF (any item)
 *   - Chest 1..8: 0x0400 (container)
 *
 * Read sites:
 * - CHAMPION.C:697 - leader-hand object placement check:
 *   AllowedSlots & G0038[SlotIndex]
 * - REVIVE.C:307/310/338 - resurrect placement check:
 *   ObjectAllowedSlots & G0038[SlotIndex]
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801/802/803/804/805/806/807/808 (Graphics.dat init-table gates
 * batches 1+2+3+4). This gate is a non-mirror-candidate contract
 * for the G0038 slot-mask table.
 */

#define DM1_V1_SLOT_MASKS_PC34_COMPAT_COUNT 38

#define DM1_V1_SLOT_MASKS_ANY_SLOT          0xFFFF
#define DM1_V1_SLOT_MASKS_HEAD              0x0002
#define DM1_V1_SLOT_MASKS_TORSO             0x0008
#define DM1_V1_SLOT_MASKS_LEGS              0x0010
#define DM1_V1_SLOT_MASKS_FEET              0x0020
#define DM1_V1_SLOT_MASKS_NECK              0x0004
#define DM1_V1_SLOT_MASKS_QUIVER_LINE1      0x0040
#define DM1_V1_SLOT_MASKS_QUIVER_LINE2      0x0080
#define DM1_V1_SLOT_MASKS_POUCH             0x0100
#define DM1_V1_SLOT_MASKS_CONTAINER         0x0400

typedef struct DM1_V1_SlotMasksResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_SLOT_MASKS_PC34_COMPAT_COUNT];
    int tableSize;
    int tableMatchesDeclaration;
    int readyHandMaskIsAny;
    int actionHandMaskIsAny;
    int bodyPartMasksSingleBit;
    int neckMaskIsNeck;
    int quiverLine1MaskIsQuiverLine1;
    int quiverLine2MaskIsQuiverLine2;
    int pouchMaskIsPouch;
    int backpackMasksAreAny;
    int chestMasksAreContainer;
    int lookupFunctionInRange;
    int lookupOutOfRangeReturnsZero;
} DM1_V1_SlotMasksResultPc34;

const int *
dm1_v1_slot_masks_table_pc34(void);

int
dm1_v1_slot_masks_size_pc34(void);

int
dm1_v1_slot_masks_pc34(int slot_index);

int
dm1_v1_slot_masks_is_compatible_pc34(int slot_mask, int slot_index);

int
dm1_v1_slot_masks_run_pc34(
    DM1_V1_SlotMasksResultPc34 *out);

#endif