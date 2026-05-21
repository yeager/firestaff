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

static int expect_item(const char* label, const M11_Item* item, int itemType, int weight) {
    if (item->itemType != itemType || item->weight != weight) {
        fprintf(stderr, "FAIL: %s got item=%d weight=%d expected item=%d weight=%d\n",
                label, item->itemType, item->weight, itemType, weight);
        return 0;
    }
    return 1;
}

int main(void) {
    M11_InventoryState state;
    M11_Item item;
    int ok = 1;
    int health[4] = { 100, 100, 100, 100 };
    int championIndex = -1;
    int pc34SourceSlot = -1;

    printf("probe=dm1_v1_inventory_equip_slots_pc34_compat\n");
    printf("sourceEvidence=%s\n", dm1_inventory_pass601_inventory_source_evidence());

    ok &= expect_int("status slotbox 0 routes champion 0 ready hand",
                     m11_inventory_resolve_status_hand_slot_box(0, 4, 0, 0, health,
                                                                &championIndex, &pc34SourceSlot), 1);
    ok &= expect_int("slotbox 0 champion", championIndex, 0);
    ok &= expect_int("slotbox 0 source slot", pc34SourceSlot, DM1_PC34_SLOT_READY_HAND);
    ok &= expect_int("status slotbox 3 routes champion 1 action hand",
                     m11_inventory_resolve_status_hand_slot_box(3, 4, 0, 0, health,
                                                                &championIndex, &pc34SourceSlot), 1);
    ok &= expect_int("slotbox 3 champion", championIndex, 1);
    ok &= expect_int("slotbox 3 source slot", pc34SourceSlot, DM1_PC34_SLOT_ACTION_HAND);
    ok &= expect_int("slotbox rejects champion outside party count",
                     m11_inventory_resolve_status_hand_slot_box(6, 3, 0, 0, health,
                                                                &championIndex, &pc34SourceSlot), 0);
    ok &= expect_int("slotbox rejects currently open inventory champion",
                     m11_inventory_resolve_status_hand_slot_box(2, 4, 2, 0, health,
                                                                &championIndex, &pc34SourceSlot), 0);
    ok &= expect_int("slotbox rejects candidate champion flow",
                     m11_inventory_resolve_status_hand_slot_box(2, 4, 0, 1, health,
                                                                &championIndex, &pc34SourceSlot), 0);
    health[2] = 0;
    ok &= expect_int("slotbox rejects dead champion",
                     m11_inventory_resolve_status_hand_slot_box(4, 4, 0, 0, health,
                                                                &championIndex, &pc34SourceSlot), 0);
    health[2] = 100;

    ok &= expect_int("ready hand mask", m11_inventory_pc34_slot_mask(DM1_PC34_SLOT_READY_HAND),
                     DM1_PC34_ALLOWED_ANY_SLOT);
    ok &= expect_int("head mask", m11_inventory_pc34_slot_mask(DM1_PC34_SLOT_HEAD),
                     DM1_PC34_ALLOWED_HEAD);
    ok &= expect_int("neck mask", m11_inventory_pc34_slot_mask(DM1_PC34_SLOT_NECK),
                     DM1_PC34_ALLOWED_NECK);
    ok &= expect_int("torso mask", m11_inventory_pc34_slot_mask(DM1_PC34_SLOT_TORSO),
                     DM1_PC34_ALLOWED_TORSO);
    ok &= expect_int("legs mask", m11_inventory_pc34_slot_mask(DM1_PC34_SLOT_LEGS),
                     DM1_PC34_ALLOWED_LEGS);
    ok &= expect_int("feet mask", m11_inventory_pc34_slot_mask(DM1_PC34_SLOT_FEET),
                     DM1_PC34_ALLOWED_FEET);

    m11_inventory_init(&state, 1);

    ok &= m11_inventory_set_mouse_item(&state, 0, 100, 7, 0, DM1_PC34_ALLOWED_HEAD);
    ok &= m11_inventory_click_pc34_source_slot(&state, 0, DM1_PC34_SLOT_HEAD);
    ok &= m11_inventory_get_item_in_pc34_source_slot(&state, 0, DM1_PC34_SLOT_HEAD, &item);
    ok &= expect_item("head equip", &item, 100, 7);
    ok &= m11_inventory_get_mouse_item(&state, 0, &item);
    ok &= expect_item("leader hand empty after equip", &item, 0, 0);
    ok &= expect_int("load after head equip", m11_inventory_get_load(&state, 0), 7);

    ok &= m11_inventory_set_mouse_item(&state, 0, 101, 9, 0, DM1_PC34_ALLOWED_HEAD);
    ok &= expect_int("head-only object rejected from torso",
                     m11_inventory_click_pc34_source_slot(&state, 0, DM1_PC34_SLOT_TORSO), 0);
    ok &= m11_inventory_get_mouse_item(&state, 0, &item);
    ok &= expect_item("leader hand preserved after rejected torso equip", &item, 101, 9);

    ok &= expect_int("head swap accepts head mask",
                     m11_inventory_click_pc34_source_slot(&state, 0, DM1_PC34_SLOT_HEAD), 1);
    ok &= m11_inventory_get_item_in_pc34_source_slot(&state, 0, DM1_PC34_SLOT_HEAD, &item);
    ok &= expect_item("new head item after swap", &item, 101, 9);
    ok &= m11_inventory_get_mouse_item(&state, 0, &item);
    ok &= expect_item("old head item moved to leader hand", &item, 100, 7);
    ok &= expect_int("load after head swap", m11_inventory_get_load(&state, 0), 9);

    ok &= m11_inventory_set_mouse_item(&state, 0, 0, 0, 0, 0);
    ok &= expect_int("empty leader hand picks up head item",
                     m11_inventory_click_pc34_source_slot(&state, 0, DM1_PC34_SLOT_HEAD), 1);
    ok &= m11_inventory_get_item_in_pc34_source_slot(&state, 0, DM1_PC34_SLOT_HEAD, &item);
    ok &= expect_item("head empty after unequip", &item, 0, 0);
    ok &= m11_inventory_get_mouse_item(&state, 0, &item);
    ok &= expect_item("unequipped head in leader hand", &item, 101, 9);
    ok &= expect_int("load after unequip", m11_inventory_get_load(&state, 0), 0);

    ok &= m11_inventory_set_item_in_pc34_source_slot(&state, 0, DM1_PC34_SLOT_NECK,
                                                     200, 1, 0, DM1_PC34_ALLOWED_NECK);
    ok &= m11_inventory_set_mouse_item(&state, 0, 201, 3, 0,
                                       DM1_PC34_ALLOWED_NECK | DM1_PC34_ALLOWED_TORSO);
    ok &= expect_int("neck-compatible item swaps with neck slot",
                     m11_inventory_click_pc34_source_slot(&state, 0, DM1_PC34_SLOT_NECK), 1);
    ok &= m11_inventory_get_item_in_pc34_source_slot(&state, 0, DM1_PC34_SLOT_NECK, &item);
    ok &= expect_item("new neck item after swap", &item, 201, 3);
    ok &= m11_inventory_get_mouse_item(&state, 0, &item);
    ok &= expect_item("old neck item moved to leader hand", &item, 200, 1);

    printf("inventoryEquipBodySlotInvariantOk=%d\n", ok ? 1 : 0);
    return ok ? 0 : 1;
}
