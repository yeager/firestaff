#include "dm1_v1_inventory_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static M11_Item make_item(int itemType, int weight, int allowedSlots) {
    M11_Item item;
    memset(&item, 0, sizeof(item));
    item.itemType = itemType;
    item.weight = weight;
    item.allowedSlots = allowedSlots;
    return item;
}

static int expect_int(const char* label, int actual, int expected) {
    if (actual != expected) {
        fprintf(stderr, "FAIL: %s got %d expected %d\n", label, actual, expected);
        return 0;
    }
    return 1;
}

static int expect_item_type(const char* label, const M11_Item* item, int expected) {
    if (item->itemType != expected) {
        fprintf(stderr, "FAIL: %s got item=%d expected item=%d\n", label, item->itemType, expected);
        return 0;
    }
    return 1;
}

int main(void) {
    M11_InventoryState state;
    M11_Item item;
    M11_Item linked[10];
    M11_Item closed[8];
    M11_Item sparse[8];
    M11_Item limited[2];
    M11_Item replacement[8];
    M11_Item replaced[8];
    M11_Item statChest[8];
    M11_Item zeroMaskItem;
    M11_InventoryState statState;
    int ok = 1;

    printf("probe=dm1_v1_inventory_backpack_chest_pc34_compat\n");
    printf("sourceEvidence=%s\n", dm1_inventory_pass601_inventory_source_evidence());

    ok &= expect_int("inventory source slot count", DM1_PC34_INVENTORY_SLOT_COUNT, 30);
    ok &= expect_int("backpack source slot count", DM1_PC34_BACKPACK_SLOT_COUNT, 17);
    ok &= expect_int("chest source slot count", DM1_PC34_CHEST_SLOT_COUNT, 8);
    ok &= expect_int("backpack first predicate", m11_inventory_pc34_is_backpack_source_slot(DM1_PC34_SLOT_BACKPACK_LINE1_1), 1);
    ok &= expect_int("backpack last predicate", m11_inventory_pc34_is_backpack_source_slot(DM1_PC34_SLOT_BACKPACK_LINE1_9), 1);
    ok &= expect_int("chest first predicate", m11_inventory_pc34_is_chest_source_slot(DM1_PC34_SLOT_CHEST_1), 1);
    ok &= expect_int("chest last predicate", m11_inventory_pc34_is_chest_source_slot(DM1_PC34_SLOT_CHEST_8), 1);
    ok &= expect_int("backpack line2_9 storage", m11_inventory_pc34_source_slot_to_storage_slot(DM1_PC34_SLOT_BACKPACK_LINE2_9), DM1_SLOT_BACKPACK9);
    ok &= expect_int("backpack line1_9 storage", m11_inventory_pc34_source_slot_to_storage_slot(DM1_PC34_SLOT_BACKPACK_LINE1_9), DM1_SLOT_BACKPACK17);
    ok &= expect_int("chest source slot is not champion storage", m11_inventory_pc34_source_slot_to_storage_slot(DM1_PC34_SLOT_CHEST_1), -1);
    ok &= expect_int("chest slot mask", m11_inventory_pc34_slot_mask(DM1_PC34_SLOT_CHEST_8), DM1_PC34_ALLOWED_CONTAINER);

    /* ReDMCSB CHAMPION.C F0299 lines 343-346 applies C137 Rabbit's Foot
     * luck only when P0624_ui_SlotIndex < C30_SLOT_CHEST_1.  DEFS.H lines
     * 806-810 make C30 the first chest slot, so open G0425 chest contents
     * must not contribute the +10 luck modifier or set the old BUG0_37
     * panel-refresh path described at CHAMPION.C lines 335-341. */
    memset(statChest, 0, sizeof(statChest));
    m11_inventory_init(&statState, 1);
    statChest[0] = make_item(DM1_PC34_ICON_JUNK_RABBITS_FOOT, 1, DM1_PC34_ALLOWED_CONTAINER);
    ok &= expect_int("open rabbit foot chest for stat isolation",
                     m11_inventory_open_chest(&statState, 0, 0x4567, statChest, 8), 1);
    ok &= expect_int("rabbit foot helper rejects C30 chest slot",
                     m11_inventory_pc34_applies_rabbits_foot_luck_modifier(&statChest[0],
                                                                           DM1_PC34_SLOT_CHEST_1), 0);
    ok &= expect_int("rabbit foot in open chest does not add luck",
                     m11_inventory_pc34_get_rabbits_foot_luck_bonus(&statState, 0), 0);
    ok &= expect_int("ordinary inventory rabbit foot accepted for luck",
                     m11_inventory_set_item_in_pc34_source_slot(&statState, 0,
                                                                DM1_PC34_SLOT_BACKPACK_LINE1_1,
                                                                DM1_PC34_ICON_JUNK_RABBITS_FOOT,
                                                                1, 0,
                                                                DM1_PC34_ALLOWED_ANY_SLOT), 1);
    ok &= expect_int("ordinary inventory rabbit foot adds +10 luck",
                     m11_inventory_pc34_get_rabbits_foot_luck_bonus(&statState, 0),
                     DM1_PC34_RABBITS_FOOT_LUCK_BONUS);

    m11_inventory_init(&state, 1);
    for (int i = 0; i < 10; i++) {
        linked[i] = make_item(200 + i, 1, DM1_PC34_ALLOWED_CONTAINER);
    }
    memset(closed, 0, sizeof(closed));

    ok &= expect_int("open chest", m11_inventory_open_chest(&state, 0, 0x1234, linked, 10), 1);
    ok &= expect_int("open chest thing", m11_inventory_get_open_chest_thing(&state, 0), 0x1234);
    ok &= m11_inventory_get_item_in_chest_slot(&state, 0, 0, &item);
    ok &= expect_item_type("open copies first linked object", &item, 200);
    ok &= m11_inventory_get_item_in_chest_slot(&state, 0, 7, &item);
    ok &= expect_item_type("open caps visible chest slots at eight", &item, 207);
    ok &= expect_int("open chest load includes eight visible things", m11_inventory_get_load(&state, 0), 8);

    /* ReDMCSB CHAMPION.C F0302 lines 694-699 gates every leader-hand slot
     * placement, including C30+ chest slots, with
     * AllowedSlots & DATA.C G0038_ai_Graphic562_SlotMasks[slot].  A zero
     * AllowedSlots mask is therefore rejected rather than treated as
     * unrestricted. */
    zeroMaskItem = make_item(777, 6, 0);
    ok &= expect_int("zero allowed-slot mask cannot equip into chest",
                     m11_inventory_can_equip(&zeroMaskItem, DM1_PC34_SLOT_CHEST_4), 0);
    ok &= expect_int("zero allowed-slot chest equip rejected",
                     m11_inventory_equip(&state, 0, DM1_PC34_SLOT_CHEST_4, &zeroMaskItem), 0);
    ok &= m11_inventory_get_item_in_chest_slot(&state, 0, 3, &item);
    ok &= expect_item_type("rejected zero-mask item leaves chest slot unchanged", &item, 203);

    ok &= m11_inventory_set_item_in_chest_slot(&state, 0, 0, 999, 2, 0, DM1_PC34_ALLOWED_CONTAINER);
    ok &= expect_int("reopen same chest is guard no-op", m11_inventory_open_chest(&state, 0, 0x1234, linked, 10), 1);
    ok &= m11_inventory_get_item_in_chest_slot(&state, 0, 0, &item);
    ok &= expect_item_type("same chest open preserves edited slot", &item, 999);

    /* ReDMCSB CHEST.C F0333 calls F0334 only when replacing a different
     * chest. Replacing the currently open chest id should be a no-op: it
     * must keep the open chest thing and should not populate previousItemsOut.
     */
    closed[0].itemType = 777;
    ok &= expect_int("replacing same chest is a no-op", m11_inventory_open_chest_replacing_current(
                             &state,
                             0,
                             0x1234,
                             linked,
                             10,
                             closed,
                             DM1_PC34_CHEST_SLOT_COUNT),
                         0);
    ok &= expect_int("same-chest replacement keeps open chest thing", m11_inventory_get_open_chest_thing(&state, 0), 0x1234);
    ok &= m11_inventory_get_item_in_chest_slot(&state, 0, 0, &item);
    ok &= expect_item_type("same-chest replacement preserves edited open slot", &item, 999);
    ok &= expect_int("same-chest replacement does not overwrite output", closed[0].itemType, 777);

    ok &= expect_int("pc34 source setter writes open chest slot",
                     m11_inventory_set_item_in_pc34_source_slot(&state, 0, DM1_PC34_SLOT_CHEST_8,
                                                                555, 1, 2, DM1_PC34_ALLOWED_CONTAINER), 1);
    ok &= m11_inventory_get_item_in_chest_slot(&state, 0, 7, &item);
    ok &= expect_item_type("pc34 source getter reads written chest slot", &item, 555);

    ok &= m11_inventory_set_mouse_item(&state, 0, 300, 4, 0, DM1_PC34_ALLOWED_CONTAINER);
    ok &= expect_int("container-compatible leader hand swaps into chest", m11_inventory_click_pc34_source_slot(&state, 0, DM1_PC34_SLOT_CHEST_3), 1);
    ok &= m11_inventory_get_item_in_chest_slot(&state, 0, 2, &item);
    ok &= expect_item_type("chest slot receives leader hand object", &item, 300);
    ok &= m11_inventory_get_mouse_item(&state, 0, &item);
    ok &= expect_item_type("old chest item moves to leader hand", &item, 202);
    ok &= expect_int("chest swap updates load", m11_inventory_get_load(&state, 0), 12);

    /* ReDMCSB CHAMPION.C F0302 lines 688-710 performs the occupied C30+
     * chest-slot swap through the leader hand.  F0300 lines 511-515 removes
     * the old G0425_aT_ChestSlots occupant; F0301 lines 606-613 allows the
     * next slot click to add that same thing back into ordinary champion
     * Slots[] storage.  CHEST.C F0334 lines 112-133 later compacts only the
     * visible chest slots, so this backpack reinsertion must not rewrite the
     * replacement object left in the chest. */
    ok &= expect_int("old chest occupant reinserts into party backpack",
                     m11_inventory_click_pc34_source_slot(&state, 0,
                                                          DM1_PC34_SLOT_BACKPACK_LINE1_1), 1);
    ok &= m11_inventory_get_item_in_pc34_source_slot(&state, 0,
                                                     DM1_PC34_SLOT_BACKPACK_LINE1_1, &item);
    ok &= expect_item_type("reinserted old chest occupant is in backpack", &item, 202);
    ok &= m11_inventory_get_item_in_chest_slot(&state, 0, 2, &item);
    ok &= expect_item_type("chest replacement survives old-occupant reinsertion", &item, 300);
    ok &= m11_inventory_get_mouse_item(&state, 0, &item);
    ok &= expect_item_type("leader hand clears after backpack reinsertion", &item, 0);
    ok &= expect_int("backpack reinsertion updates load", m11_inventory_get_load(&state, 0), 13);
    ok &= expect_int("test isolation removes reinserted backpack object",
                     m11_inventory_remove_item(&state, 0, DM1_SLOT_BACKPACK1), 1);
    ok &= expect_int("load returns to chest-only after isolation remove",
                     m11_inventory_get_load(&state, 0), 12);

    ok &= m11_inventory_set_mouse_item(&state, 0, 301, 5, 0, DM1_PC34_ALLOWED_HEAD);
    ok &= expect_int("head-only item rejected from chest", m11_inventory_click_pc34_source_slot(&state, 0, DM1_PC34_SLOT_CHEST_4), 0);
    ok &= m11_inventory_get_mouse_item(&state, 0, &item);
    ok &= expect_item_type("rejected item remains in leader hand", &item, 301);

    ok &= m11_inventory_set_mouse_item(&state, 0, 0, 0, 0, 0);
    ok &= expect_int("empty leader hand picks up chest slot", m11_inventory_click_pc34_source_slot(&state, 0, DM1_PC34_SLOT_CHEST_3), 1);
    ok &= m11_inventory_get_item_in_chest_slot(&state, 0, 2, &item);
    ok &= expect_item_type("picked chest slot becomes empty", &item, 0);
    ok &= m11_inventory_get_mouse_item(&state, 0, &item);
    ok &= expect_item_type("picked chest item moves to leader hand", &item, 300);

    ok &= expect_int("close compacts non-empty chest slots", m11_inventory_close_chest(&state, 0, closed, 8), 7);
    ok &= expect_int("open chest cleared", m11_inventory_get_open_chest_thing(&state, 0), 0);
    ok &= expect_item_type("closed slot 0 preserves edited first slot", &closed[0], 999);
    ok &= expect_item_type("closed list skips empty chest slot", &closed[1], 201);
    ok &= expect_item_type("closed list preserves later order", &closed[5], 206);
    ok &= expect_item_type("closed list includes pc34-set chest item", &closed[6], 555);
    ok &= expect_int("closed chest no longer exposes panel slots", m11_inventory_get_item_in_chest_slot(&state, 0, 0, &item), 0);

    memset(sparse, 0, sizeof(sparse));
    memset(limited, 0, sizeof(limited));
    sparse[0] = make_item(401, 2, DM1_PC34_ALLOWED_CONTAINER);
    sparse[2] = make_item(402, 3, DM1_PC34_ALLOWED_CONTAINER);
    sparse[5] = make_item(403, 4, DM1_PC34_ALLOWED_CONTAINER);
    sparse[7] = make_item(404, 5, DM1_PC34_ALLOWED_CONTAINER);
    ok &= expect_int("open sparse chest", m11_inventory_open_chest(&state, 0, 0x2345, sparse, 8), 1);
    ok &= expect_int("sparse open load includes only visible non-empty things", m11_inventory_get_load(&state, 0), 14);
    ok &= expect_int("limited close returns total compacted count",
                     m11_inventory_close_chest(&state, 0, limited, 2), 4);
    ok &= expect_item_type("limited close writes first compacted item", &limited[0], 401);
    ok &= expect_item_type("limited close writes second compacted item", &limited[1], 402);
    ok &= expect_int("limited close clears open chest", m11_inventory_get_open_chest_thing(&state, 0), 0);
    ok &= expect_int("limited close clears chest load", m11_inventory_get_load(&state, 0), 0);
    ok &= expect_int("limited close hides chest slots", m11_inventory_get_item_in_chest_slot(&state, 0, 5, &item), 0);

    ok &= expect_int("reopen sparse chest for zero-output close",
                     m11_inventory_open_chest(&state, 0, 0x3456, sparse, 8), 1);
    ok &= expect_int("zero-output close counts without buffer",
                     m11_inventory_close_chest(&state, 0, NULL, 0), 4);
    ok &= expect_int("zero-output close clears open chest", m11_inventory_get_open_chest_thing(&state, 0), 0);

    memset(replacement, 0, sizeof(replacement));
    memset(replaced, 0, sizeof(replaced));
    replacement[0] = make_item(501, 6, DM1_PC34_ALLOWED_CONTAINER);
    ok &= expect_int("open old chest before replacement",
                     m11_inventory_open_chest(&state, 0, 0x4567, sparse, 8), 1);
    ok &= expect_int("edit old chest before replacement",
                     m11_inventory_set_item_in_chest_slot(&state, 0, 1, 405, 7, 0,
                                                          DM1_PC34_ALLOWED_CONTAINER), 1);
    /* ReDMCSB CHEST.C F0333 lines 34-39 calls F0334 when a different chest is
     * already open; F0334 lines 112-133 compacts non-empty G0425 slots before
     * F0333 lines 53-76 copies the requested chest's first eight links. */
    ok &= expect_int("replacement closes previous chest first",
                     m11_inventory_open_chest_replacing_current(&state, 0, 0x5678,
                                                                replacement, 8,
                                                                replaced, 8), 5);
    ok &= expect_item_type("replacement compact preserves old slot 0", &replaced[0], 401);
    ok &= expect_item_type("replacement compact includes edited old slot", &replaced[1], 405);
    ok &= expect_item_type("replacement compact keeps later old order", &replaced[4], 404);
    ok &= expect_int("replacement opens requested chest", m11_inventory_get_open_chest_thing(&state, 0), 0x5678);
    ok &= m11_inventory_get_item_in_chest_slot(&state, 0, 0, &item);
    ok &= expect_item_type("replacement loads new first slot", &item, 501);
    ok &= expect_int("replacement load is new chest only", m11_inventory_get_load(&state, 0), 6);

    replacement[0] = make_item(601, 8, DM1_PC34_ALLOWED_CONTAINER);
    /* ReDMCSB CHEST.C F0333 lines 34-39 still routes a different open chest
     * through F0334 even when the caller does not keep the closed output list.
     * F0334 lines 113-132 clears G0426/G0425 and returns the compacted count;
     * F0333 lines 53-76 then materializes the newly requested chest. */
    ok &= expect_int("zero-output replacement closes previous chest first",
                     m11_inventory_open_chest_replacing_current(&state, 0, 0x6789,
                                                                replacement, 8,
                                                                NULL, 0), 1);
    ok &= expect_int("zero-output replacement opens requested chest",
                     m11_inventory_get_open_chest_thing(&state, 0), 0x6789);
    ok &= m11_inventory_get_item_in_chest_slot(&state, 0, 0, &item);
    ok &= expect_item_type("zero-output replacement loads new first slot", &item, 601);
    ok &= expect_int("zero-output replacement load is new chest only",
                     m11_inventory_get_load(&state, 0), 8);

    printf("inventoryBackpackChestInvariantOk=%d\n", ok ? 1 : 0);
    return ok ? 0 : 1;
}
