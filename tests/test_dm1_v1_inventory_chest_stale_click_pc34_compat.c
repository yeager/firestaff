#include "dm1_v1_inventory_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static DM1_V1_ItemPc34 make_item(int itemType, int weight, int allowedSlots)
{
    DM1_V1_ItemPc34 item;
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

static int expect_item_type(const char* label, const DM1_V1_ItemPc34* item, int want)
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

    DM1_V1_InventoryStatePc34 state;
    DM1_V1_ItemPc34 oldLinked[8];
    DM1_V1_ItemPc34 newLinked[8];
    DM1_V1_ItemPc34 previous[8];
    DM1_V1_ItemPc34 item;
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

    DM1_V1_Inventory_InitPc34Compat(&state, 1);
    ok &= expect_int("open old chest",
                     DM1_V1_Inventory_OpenChestPc34Compat(&state, 0, OLD_CHEST, oldLinked, 8), 1);
    ok &= expect_int("close old chest",
                     DM1_V1_Inventory_CloseChestPc34Compat(&state, 0, previous, 8), 8);
    ok &= expect_int("stale click after close rejected",
                     DM1_V1_Inventory_ClickOpenChestSlotForThingPc34Compat(&state, 0,
                                                                   OLD_CHEST, 7), 0);
    ok &= DM1_V1_Inventory_GetMouseItemPc34Compat(&state, 0, &item);
    ok &= expect_item_type("leader hand remains empty after closed stale click", &item, 0);
    ok &= expect_int("closed chest exposes no slot",
                     DM1_V1_Inventory_GetItemInChestSlotPc34Compat(&state, 0, 7, &item), 0);

    ok &= expect_int("reopen old chest",
                     DM1_V1_Inventory_OpenChestPc34Compat(&state, 0, OLD_CHEST, oldLinked, 8), 1);
    ok &= expect_int("replace open chest",
                     DM1_V1_Inventory_OpenChestReplacingCurrentPc34Compat(&state, 0, NEW_CHEST,
                                                                newLinked, 8,
                                                                previous, 8), 8);
    ok &= expect_int("open chest is replacement",
                     DM1_V1_Inventory_GetOpenChestThingPc34Compat(&state, 0), NEW_CHEST);
    ok &= expect_int("old chest click after replacement rejected",
                     DM1_V1_Inventory_ClickOpenChestSlotForThingPc34Compat(&state, 0,
                                                                   OLD_CHEST, 7), 0);
    ok &= DM1_V1_Inventory_GetItemInChestSlotPc34Compat(&state, 0, 7, &item);
    ok &= expect_item_type("replacement C544 item preserved after stale old click", &item, 807);
    ok &= DM1_V1_Inventory_GetMouseItemPc34Compat(&state, 0, &item);
    ok &= expect_item_type("leader hand remains empty after replacement stale click", &item, 0);

    ok &= expect_int("invalid chest slot rejected",
                     DM1_V1_Inventory_ClickOpenChestSlotForThingPc34Compat(&state, 0,
                                                                   NEW_CHEST, 8), 0);
    ok &= expect_int("current chest click accepts C544",
                     DM1_V1_Inventory_ClickOpenChestSlotForThingPc34Compat(&state, 0,
                                                                   NEW_CHEST, 7), 1);
    ok &= DM1_V1_Inventory_GetItemInChestSlotPc34Compat(&state, 0, 7, &item);
    ok &= expect_item_type("current C544 slot empties after pickup", &item, 0);
    ok &= DM1_V1_Inventory_GetMouseItemPc34Compat(&state, 0, &item);
    ok &= expect_item_type("current C544 item reaches leader hand", &item, 807);

    printf("inventoryChestStaleClickInvariantOk=%d\n", ok ? 1 : 0);
    return ok ? 0 : 1;
}
