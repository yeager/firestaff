#include "dm1_v1_inventory_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int expect_int(const char* label, int actual, int expected) {
    if (actual != expected) {
        fprintf(stderr, "FAIL: %s got %d expected %d\n", label, actual, expected);
        return 0;
    }
    return 1;
}

static int expect_item(const char* label, const DM1_V1_ItemPc34* item, int itemType, int weight) {
    if (item->itemType != itemType || item->weight != weight) {
        fprintf(stderr, "FAIL: %s got item=%d weight=%d expected item=%d weight=%d\n",
                label, item->itemType, item->weight, itemType, weight);
        return 0;
    }
    return 1;
}

int main(void) {
    DM1_V1_InventoryStatePc34 state;
    DM1_V1_ItemPc34 item;
    int ok = 1;
    int health[4] = { 100, 100, 100, 100 };
    int championIndex = -1;
    int pc34SourceSlot = -1;

    printf("probe=dm1_v1_inventory_equip_slots_pc34_compat\n");
    printf("sourceEvidence=%s\n", dm1_inventory_pass601_inventory_source_evidence());

    ok &= expect_int("status slotbox 0 routes champion 0 ready hand",
                     DM1_V1_Inventory_ResolveStatusHandSlotBoxPc34Compat(0, 4, 0, 0, health,
                                                                &championIndex, &pc34SourceSlot), 1);
    ok &= expect_int("slotbox 0 champion", championIndex, 0);
    ok &= expect_int("slotbox 0 source slot", pc34SourceSlot, DM1_PC34_SLOT_READY_HAND);
    ok &= expect_int("status slotbox 3 routes champion 1 action hand",
                     DM1_V1_Inventory_ResolveStatusHandSlotBoxPc34Compat(3, 4, 0, 0, health,
                                                                &championIndex, &pc34SourceSlot), 1);
    ok &= expect_int("slotbox 3 champion", championIndex, 1);
    ok &= expect_int("slotbox 3 source slot", pc34SourceSlot, DM1_PC34_SLOT_ACTION_HAND);
    ok &= expect_int("slotbox rejects champion outside party count",
                     DM1_V1_Inventory_ResolveStatusHandSlotBoxPc34Compat(6, 3, 0, 0, health,
                                                                &championIndex, &pc34SourceSlot), 0);
    ok &= expect_int("slotbox rejects currently open inventory champion",
                     DM1_V1_Inventory_ResolveStatusHandSlotBoxPc34Compat(2, 4, 2, 0, health,
                                                                &championIndex, &pc34SourceSlot), 0);
    ok &= expect_int("slotbox rejects candidate champion flow",
                     DM1_V1_Inventory_ResolveStatusHandSlotBoxPc34Compat(2, 4, 0, 1, health,
                                                                &championIndex, &pc34SourceSlot), 0);
    health[2] = 0;
    ok &= expect_int("slotbox rejects dead champion",
                     DM1_V1_Inventory_ResolveStatusHandSlotBoxPc34Compat(4, 4, 0, 0, health,
                                                                &championIndex, &pc34SourceSlot), 0);
    health[2] = 100;

    ok &= expect_int("ready hand mask", DM1_V1_Inventory_Pc34SlotMaskCompat(DM1_PC34_SLOT_READY_HAND),
                     DM1_PC34_ALLOWED_ANY_SLOT);
    ok &= expect_int("head mask", DM1_V1_Inventory_Pc34SlotMaskCompat(DM1_PC34_SLOT_HEAD),
                     DM1_PC34_ALLOWED_HEAD);
    ok &= expect_int("neck mask", DM1_V1_Inventory_Pc34SlotMaskCompat(DM1_PC34_SLOT_NECK),
                     DM1_PC34_ALLOWED_NECK);
    ok &= expect_int("torso mask", DM1_V1_Inventory_Pc34SlotMaskCompat(DM1_PC34_SLOT_TORSO),
                     DM1_PC34_ALLOWED_TORSO);
    ok &= expect_int("legs mask", DM1_V1_Inventory_Pc34SlotMaskCompat(DM1_PC34_SLOT_LEGS),
                     DM1_PC34_ALLOWED_LEGS);
    ok &= expect_int("feet mask", DM1_V1_Inventory_Pc34SlotMaskCompat(DM1_PC34_SLOT_FEET),
                     DM1_PC34_ALLOWED_FEET);

    DM1_V1_Inventory_InitPc34Compat(&state, 1);

    ok &= DM1_V1_Inventory_SetMouseItemPc34Compat(&state, 0, 100, 7, 0, DM1_PC34_ALLOWED_HEAD);
    ok &= DM1_V1_Inventory_ClickPc34SourceSlotCompat(&state, 0, DM1_PC34_SLOT_HEAD);
    ok &= DM1_V1_Inventory_GetItemInPc34SourceSlotCompat(&state, 0, DM1_PC34_SLOT_HEAD, &item);
    ok &= expect_item("head equip", &item, 100, 7);
    ok &= DM1_V1_Inventory_GetMouseItemPc34Compat(&state, 0, &item);
    ok &= expect_item("leader hand empty after equip", &item, 0, 0);
    ok &= expect_int("load after head equip", DM1_V1_Inventory_GetLoadPc34Compat(&state, 0), 7);

    ok &= DM1_V1_Inventory_SetMouseItemPc34Compat(&state, 0, 101, 9, 0, DM1_PC34_ALLOWED_HEAD);
    ok &= expect_int("head-only object rejected from torso",
                     DM1_V1_Inventory_ClickPc34SourceSlotCompat(&state, 0, DM1_PC34_SLOT_TORSO), 0);
    ok &= DM1_V1_Inventory_GetMouseItemPc34Compat(&state, 0, &item);
    ok &= expect_item("leader hand preserved after rejected torso equip", &item, 101, 9);

    ok &= expect_int("head swap accepts head mask",
                     DM1_V1_Inventory_ClickPc34SourceSlotCompat(&state, 0, DM1_PC34_SLOT_HEAD), 1);
    ok &= DM1_V1_Inventory_GetItemInPc34SourceSlotCompat(&state, 0, DM1_PC34_SLOT_HEAD, &item);
    ok &= expect_item("new head item after swap", &item, 101, 9);
    ok &= DM1_V1_Inventory_GetMouseItemPc34Compat(&state, 0, &item);
    ok &= expect_item("old head item moved to leader hand", &item, 100, 7);
    ok &= expect_int("load after head swap", DM1_V1_Inventory_GetLoadPc34Compat(&state, 0), 9);

    ok &= DM1_V1_Inventory_SetMouseItemPc34Compat(&state, 0, 0, 0, 0, 0);
    ok &= expect_int("empty leader hand picks up head item",
                     DM1_V1_Inventory_ClickPc34SourceSlotCompat(&state, 0, DM1_PC34_SLOT_HEAD), 1);
    ok &= DM1_V1_Inventory_GetItemInPc34SourceSlotCompat(&state, 0, DM1_PC34_SLOT_HEAD, &item);
    ok &= expect_item("head empty after unequip", &item, 0, 0);
    ok &= DM1_V1_Inventory_GetMouseItemPc34Compat(&state, 0, &item);
    ok &= expect_item("unequipped head in leader hand", &item, 101, 9);
    ok &= expect_int("load after unequip", DM1_V1_Inventory_GetLoadPc34Compat(&state, 0), 0);

    ok &= DM1_V1_Inventory_SetItemInPc34SourceSlotCompat(&state, 0, DM1_PC34_SLOT_NECK,
                                                     200, 1, 0, DM1_PC34_ALLOWED_NECK);
    ok &= DM1_V1_Inventory_SetMouseItemPc34Compat(&state, 0, 201, 3, 0,
                                       DM1_PC34_ALLOWED_NECK | DM1_PC34_ALLOWED_TORSO);
    ok &= expect_int("neck-compatible item swaps with neck slot",
                     DM1_V1_Inventory_ClickPc34SourceSlotCompat(&state, 0, DM1_PC34_SLOT_NECK), 1);
    ok &= DM1_V1_Inventory_GetItemInPc34SourceSlotCompat(&state, 0, DM1_PC34_SLOT_NECK, &item);
    ok &= expect_item("new neck item after swap", &item, 201, 3);
    ok &= DM1_V1_Inventory_GetMouseItemPc34Compat(&state, 0, &item);
    ok &= expect_item("old neck item moved to leader hand", &item, 200, 1);

    printf("inventoryEquipBodySlotInvariantOk=%d\n", ok ? 1 : 0);
    return ok ? 0 : 1;
}
