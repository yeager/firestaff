#include "dm1_v1_inventory_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static M11_Item make_item(int itemType, int weight, int allowedSlots)
{
    M11_Item item;
    memset(&item, 0, sizeof(item));
    item.itemType = itemType;
    item.weight = weight;
    item.allowedSlots = allowedSlots;
    return item;
}

static int expect_int(const char* label, int got, int want)
{
    if (got != want) {
        printf("FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    printf("ok %s=%d\n", label, got);
    return 1;
}

static int expect_item_type(const char* label, const M11_Item* item, int want)
{
    if (item->itemType != want) {
        printf("FAIL %s got item=%d want item=%d\n", label, item->itemType, want);
        return 0;
    }
    printf("ok %s.itemType=%d\n", label, item->itemType);
    return 1;
}

int main(void)
{
    enum {
        OLD_CHEST = 0x1234,
        NEW_CHEST = 0x2345
    };

    M11_InventoryState state;
    M11_Item oldLinked[8];
    M11_Item newLinked[8];
    M11_Item previous[8];
    M11_Item item;
    int ok = 1;

    printf("probe=dm1_v1_inventory_chest_stale_click_pc34_compat\n");
    printf("sourceEvidence=%s\n", dm1_inventory_chest_stale_click_source_evidence_pc34());
    ok &= expect_int("source evidence present",
                     dm1_inventory_chest_stale_click_source_evidence_pc34() != NULL, 1);

    for (int i = 0; i < 8; ++i) {
        oldLinked[i] = make_item(700 + i, 1, DM1_PC34_ALLOWED_CONTAINER);
        newLinked[i] = make_item(800 + i, 2, DM1_PC34_ALLOWED_CONTAINER);
    }
    memset(previous, 0, sizeof(previous));

    m11_inventory_init(&state, 1);
    ok &= expect_int("open old chest",
                     m11_inventory_open_chest(&state, 0, OLD_CHEST, oldLinked, 8), 1);
    ok &= expect_int("close old chest",
                     m11_inventory_close_chest(&state, 0, previous, 8), 8);
    ok &= expect_int("stale click after close rejected",
                     m11_inventory_click_open_chest_slot_for_thing(&state, 0,
                                                                   OLD_CHEST, 7), 0);
    ok &= m11_inventory_get_mouse_item(&state, 0, &item);
    ok &= expect_item_type("leader hand remains empty after closed stale click", &item, 0);
    ok &= expect_int("closed chest exposes no slot",
                     m11_inventory_get_item_in_chest_slot(&state, 0, 7, &item), 0);

    ok &= expect_int("reopen old chest",
                     m11_inventory_open_chest(&state, 0, OLD_CHEST, oldLinked, 8), 1);
    ok &= expect_int("replace open chest",
                     m11_inventory_open_chest_replacing_current(&state, 0, NEW_CHEST,
                                                                newLinked, 8,
                                                                previous, 8), 8);
    ok &= expect_int("open chest is replacement",
                     m11_inventory_get_open_chest_thing(&state, 0), NEW_CHEST);
    ok &= expect_int("old chest click after replacement rejected",
                     m11_inventory_click_open_chest_slot_for_thing(&state, 0,
                                                                   OLD_CHEST, 7), 0);
    ok &= m11_inventory_get_item_in_chest_slot(&state, 0, 7, &item);
    ok &= expect_item_type("replacement C544 item preserved after stale old click", &item, 807);
    ok &= m11_inventory_get_mouse_item(&state, 0, &item);
    ok &= expect_item_type("leader hand remains empty after replacement stale click", &item, 0);

    ok &= expect_int("invalid chest slot rejected",
                     m11_inventory_click_open_chest_slot_for_thing(&state, 0,
                                                                   NEW_CHEST, 8), 0);
    ok &= expect_int("current chest click accepts C544",
                     m11_inventory_click_open_chest_slot_for_thing(&state, 0,
                                                                   NEW_CHEST, 7), 1);
    ok &= m11_inventory_get_item_in_chest_slot(&state, 0, 7, &item);
    ok &= expect_item_type("current C544 slot empties after pickup", &item, 0);
    ok &= m11_inventory_get_mouse_item(&state, 0, &item);
    ok &= expect_item_type("current C544 item reaches leader hand", &item, 807);

    printf("inventoryChestStaleClickInvariantOk=%d\n", ok ? 1 : 0);
    return ok ? 0 : 1;
}
