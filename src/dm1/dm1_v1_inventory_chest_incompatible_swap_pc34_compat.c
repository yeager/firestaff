#include "dm1_v1_inventory_chest_incompatible_swap_pc34_compat.h"

#include <string.h>

static DM1_V1_ItemPc34 make_item(int itemType, int allowedSlots)
{
    DM1_V1_ItemPc34 item;
    memset(&item, 0, sizeof(item));
    item.itemType = itemType;
    item.weight = 1;
    item.allowedSlots = allowedSlots;
    return item;
}

static int copy_item_types(const DM1_V1_ItemPc34* items, int count, int* typesOut)
{
    int i;

    if (!items || !typesOut || count < 0) {
        return 0;
    }
    for (i = 0; i < count && i < DM1_PC34_TEST_CHEST_SWAP_SLOT_COUNT; ++i) {
        typesOut[i] = items[i].itemType;
    }
    return 1;
}

const char* dm1_inventory_chest_incompatible_swap_source_evidence_pc34(void)
{
    return
        "CHAMPION.C:688-699 F0302 reads leader hand, reads C30+ G0425 chest "
        "slot, and rejects incompatible AllowedSlots/SlotMasks before swap\n"
        "CHAMPION.C:700-710 F0302 performs the accepted leader-hand/slot swap\n"
        "CHEST.C:53-67 F0333 copies the first visible linked chest items into "
        "G0425_aT_ChestSlots\n"
        "CHEST.C:117-132 F0334 rewrites the open chest linked list from the "
        "visible G0425 slots\n"
        "DATA.C:1080-1087 gives C30..C37 chest slots MASK0x0400_CONTAINER\n"
        "DUNGEON.C:108 gives Staff Of Claws AllowedSlots 0x0040 Quiver 1";
}

int DM1_V1_InventoryChestIncompatibleSwap_RunProbePc34Compat(
    DM1_V1_InventoryChestIncompatibleSwapProbePc34* out)
{
    enum {
        CHEST_THING = 0x3456,
        C538_INDEX = 1,
        C544_INDEX = 7,
        C538_FIRST_ITEM = 700,
        C544_FIRST_ITEM = 800,
        C544_REPLACEMENT_ITEM = 900
    };
    DM1_V1_InventoryStatePc34 state;
    DM1_V1_ItemPc34 linked[9];
    DM1_V1_ItemPc34 closed[DM1_PC34_TEST_CHEST_SWAP_SLOT_COUNT];
    DM1_V1_ItemPc34 item;
    DM1_V1_ItemPc34 staffOfClaws;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    memset(closed, 0, sizeof(closed));

    DM1_V1_Inventory_InitPc34Compat(&state, 1);
    for (i = 0; i < DM1_PC34_TEST_CHEST_SWAP_SLOT_COUNT; ++i) {
        linked[i] = make_item(C538_FIRST_ITEM + i, DM1_PC34_ALLOWED_CONTAINER);
    }
    if (!DM1_V1_Inventory_OpenChestPc34Compat(&state, 0, CHEST_THING, linked,
                                  DM1_PC34_TEST_CHEST_SWAP_SLOT_COUNT)) {
        return 0;
    }

    out->c538SlotMask = DM1_V1_Inventory_Pc34SlotMaskCompat(DM1_PC34_SLOT_CHEST_2);
    staffOfClaws = make_item(DM1_PC34_TEST_STAFF_OF_CLAWS_OBJECT_INFO,
                             DM1_PC34_ALLOWED_QUIVER_LINE1);
    out->c538StaffCanEquip = DM1_V1_Inventory_CanEquipPc34Compat(&staffOfClaws,
                                                     DM1_PC34_SLOT_CHEST_2);
    out->c538LeaderHandBefore = DM1_PC34_TEST_STAFF_OF_CLAWS_OBJECT_INFO;
    if (!DM1_V1_Inventory_SetMouseItemPc34Compat(&state, 0,
                                      DM1_PC34_TEST_STAFF_OF_CLAWS_OBJECT_INFO,
                                      1, 0, DM1_PC34_ALLOWED_QUIVER_LINE1)) {
        return 0;
    }
    if (!DM1_V1_Inventory_GetItemInChestSlotPc34Compat(&state, 0, C538_INDEX, &item)) {
        return 0;
    }
    out->c538SlotBefore = item.itemType;
    out->c538ClickResult = DM1_V1_Inventory_ClickPc34SourceSlotCompat(
        &state, 0, DM1_PC34_SLOT_CHEST_2);
    if (!DM1_V1_Inventory_GetMouseItemPc34Compat(&state, 0, &item)) {
        return 0;
    }
    out->c538LeaderHandAfter = item.itemType;
    if (!DM1_V1_Inventory_GetItemInChestSlotPc34Compat(&state, 0, C538_INDEX, &item)) {
        return 0;
    }
    out->c538SlotAfter = item.itemType;
    out->c538ClosedCount = DM1_V1_Inventory_CloseChestPc34Compat(
        &state, 0, closed, DM1_PC34_TEST_CHEST_SWAP_SLOT_COUNT);
    if (out->c538ClosedCount < 0 ||
        !copy_item_types(closed, DM1_PC34_TEST_CHEST_SWAP_SLOT_COUNT,
                         out->c538ClosedTypes)) {
        return 0;
    }

    memset(closed, 0, sizeof(closed));
    DM1_V1_Inventory_InitPc34Compat(&state, 1);
    for (i = 0; i < 9; ++i) {
        linked[i] = make_item(C544_FIRST_ITEM + i, DM1_PC34_ALLOWED_CONTAINER);
    }
    out->c544HiddenTailInput = linked[8].itemType;
    if (!DM1_V1_Inventory_OpenChestPc34Compat(&state, 0, CHEST_THING, linked, 9)) {
        return 0;
    }
    item = make_item(C544_REPLACEMENT_ITEM, DM1_PC34_ALLOWED_CONTAINER);
    out->c544ReplacementCanEquip = DM1_V1_Inventory_CanEquipPc34Compat(
        &item, DM1_PC34_SLOT_CHEST_8);
    if (!DM1_V1_Inventory_SetMouseItemPc34Compat(&state, 0, C544_REPLACEMENT_ITEM,
                                      1, 0, DM1_PC34_ALLOWED_CONTAINER)) {
        return 0;
    }
    out->c544ClickResult = DM1_V1_Inventory_ClickPc34SourceSlotCompat(
        &state, 0, DM1_PC34_SLOT_CHEST_8);
    if (!DM1_V1_Inventory_GetMouseItemPc34Compat(&state, 0, &item)) {
        return 0;
    }
    out->c544LeaderHandAfter = item.itemType;
    if (!DM1_V1_Inventory_GetItemInChestSlotPc34Compat(&state, 0, C544_INDEX, &item)) {
        return 0;
    }
    out->c544SlotAfter = item.itemType;
    out->c544ClosedCount = DM1_V1_Inventory_CloseChestPc34Compat(
        &state, 0, closed, DM1_PC34_TEST_CHEST_SWAP_SLOT_COUNT);
    if (out->c544ClosedCount < 0 ||
        !copy_item_types(closed, DM1_PC34_TEST_CHEST_SWAP_SLOT_COUNT,
                         out->c544ClosedTypes)) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_TEST_CHEST_SWAP_SLOT_COUNT; ++i) {
        if (out->c544ClosedTypes[i] == out->c544HiddenTailInput) {
            out->c544HiddenTailClosed = 1;
        }
    }

    return 1;
}
