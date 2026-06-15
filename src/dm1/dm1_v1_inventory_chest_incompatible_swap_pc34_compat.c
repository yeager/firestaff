#include "dm1_v1_inventory_chest_incompatible_swap_pc34_compat.h"

#include <string.h>

static M11_Item make_item(int itemType, int allowedSlots)
{
    M11_Item item;
    memset(&item, 0, sizeof(item));
    item.itemType = itemType;
    item.weight = 1;
    item.allowedSlots = allowedSlots;
    return item;
}

static int copy_item_types(const M11_Item* items, int count, int* typesOut)
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

int m11_inventory_pc34_probe_chest_incompatible_swap(
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
    M11_InventoryState state;
    M11_Item linked[9];
    M11_Item closed[DM1_PC34_TEST_CHEST_SWAP_SLOT_COUNT];
    M11_Item item;
    M11_Item staffOfClaws;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    memset(closed, 0, sizeof(closed));

    m11_inventory_init(&state, 1);
    for (i = 0; i < DM1_PC34_TEST_CHEST_SWAP_SLOT_COUNT; ++i) {
        linked[i] = make_item(C538_FIRST_ITEM + i, DM1_PC34_ALLOWED_CONTAINER);
    }
    if (!m11_inventory_open_chest(&state, 0, CHEST_THING, linked,
                                  DM1_PC34_TEST_CHEST_SWAP_SLOT_COUNT)) {
        return 0;
    }

    out->c538SlotMask = m11_inventory_pc34_slot_mask(DM1_PC34_SLOT_CHEST_2);
    staffOfClaws = make_item(DM1_PC34_TEST_STAFF_OF_CLAWS_OBJECT_INFO,
                             DM1_PC34_ALLOWED_QUIVER_LINE1);
    out->c538StaffCanEquip = m11_inventory_can_equip(&staffOfClaws,
                                                     DM1_PC34_SLOT_CHEST_2);
    out->c538LeaderHandBefore = DM1_PC34_TEST_STAFF_OF_CLAWS_OBJECT_INFO;
    if (!m11_inventory_set_mouse_item(&state, 0,
                                      DM1_PC34_TEST_STAFF_OF_CLAWS_OBJECT_INFO,
                                      1, 0, DM1_PC34_ALLOWED_QUIVER_LINE1)) {
        return 0;
    }
    if (!m11_inventory_get_item_in_chest_slot(&state, 0, C538_INDEX, &item)) {
        return 0;
    }
    out->c538SlotBefore = item.itemType;
    out->c538ClickResult = m11_inventory_click_pc34_source_slot(
        &state, 0, DM1_PC34_SLOT_CHEST_2);
    if (!m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    out->c538LeaderHandAfter = item.itemType;
    if (!m11_inventory_get_item_in_chest_slot(&state, 0, C538_INDEX, &item)) {
        return 0;
    }
    out->c538SlotAfter = item.itemType;
    out->c538ClosedCount = m11_inventory_close_chest(
        &state, 0, closed, DM1_PC34_TEST_CHEST_SWAP_SLOT_COUNT);
    if (out->c538ClosedCount < 0 ||
        !copy_item_types(closed, DM1_PC34_TEST_CHEST_SWAP_SLOT_COUNT,
                         out->c538ClosedTypes)) {
        return 0;
    }

    memset(closed, 0, sizeof(closed));
    m11_inventory_init(&state, 1);
    for (i = 0; i < 9; ++i) {
        linked[i] = make_item(C544_FIRST_ITEM + i, DM1_PC34_ALLOWED_CONTAINER);
    }
    out->c544HiddenTailInput = linked[8].itemType;
    if (!m11_inventory_open_chest(&state, 0, CHEST_THING, linked, 9)) {
        return 0;
    }
    item = make_item(C544_REPLACEMENT_ITEM, DM1_PC34_ALLOWED_CONTAINER);
    out->c544ReplacementCanEquip = m11_inventory_can_equip(
        &item, DM1_PC34_SLOT_CHEST_8);
    if (!m11_inventory_set_mouse_item(&state, 0, C544_REPLACEMENT_ITEM,
                                      1, 0, DM1_PC34_ALLOWED_CONTAINER)) {
        return 0;
    }
    out->c544ClickResult = m11_inventory_click_pc34_source_slot(
        &state, 0, DM1_PC34_SLOT_CHEST_8);
    if (!m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    out->c544LeaderHandAfter = item.itemType;
    if (!m11_inventory_get_item_in_chest_slot(&state, 0, C544_INDEX, &item)) {
        return 0;
    }
    out->c544SlotAfter = item.itemType;
    out->c544ClosedCount = m11_inventory_close_chest(
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
