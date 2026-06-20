#ifndef FIRESTAFF_DM1_V1_SLOTDROPORDER_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_SLOTDROPORDER_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0057_ai_Graphic562_SlotDropOrder[30].
 *
 * G0057 is the 30-entry slot drop-order table (the priority order in
 * which F0300_CHAMPION_GetObjectRemovedFromSlot should remove objects
 * from a full champion inventory during a forced drop). PC 3.4 EN
 * init = { C05_SLOT_FEET=5, C04_SLOT_LEGS=4, C09_SLOT_QUIVER_LINE2_2=9,
 *   C08_SLOT_QUIVER_LINE1_2=8, C07_SLOT_QUIVER_LINE2_1=7,
 *   C12_SLOT_QUIVER_LINE1_1=12, C06_SLOT_POUCH_2=6, C11_SLOT_POUCH_1=11,
 *   C03_SLOT_TORSO=3, C13..C29_BACKPACK_LINE[1|2][1-9]=13..29,
 *   C10_SLOT_NECK=10, C02_SLOT_HEAD=2, C00_SLOT_READY_HAND=0,
 *   C01_SLOT_ACTION_HAND=1 }.
 *
 * Read site: CHAMPION.C:1546 (F0300_CHAMPION_GetObjectRemovedFromSlot
 * using G0057[SlotIndex]).
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), all earlier
 * Graphics.dat init-table gates pass798/800-806/811-859.
 */

#define DM1_V1_SLOT_DROP_ORDER_PC34_COMPAT_SIZE 30

typedef struct DM1_V1_SlotDropOrderResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_SLOT_DROP_ORDER_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int firstEntryFeetSlot5;
    int lastEntryActionHandSlot1;
    int allValuesInByteRange;
    int allValuesDistinct;
    int allBackpackSlotsCovered;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_SlotDropOrderResultPc34;

const int *
dm1_v1_slot_drop_order_table_pc34(void);

int
dm1_v1_slot_drop_order_size_pc34(void);

int
dm1_v1_slot_drop_order_get_pc34(int slot_index);

int
dm1_v1_slot_drop_order_first_pc34(void);

int
dm1_v1_slot_drop_order_last_pc34(void);

int
dm1_v1_slot_drop_order_run_pc34(
    DM1_V1_SlotDropOrderResultPc34 *out);

#endif