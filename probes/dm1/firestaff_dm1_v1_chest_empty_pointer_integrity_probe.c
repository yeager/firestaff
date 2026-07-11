#include <stdio.h>

#include "dm1_v1_inventory_pc34_compat.h"

/*
 * DM1 V1 empty-chest inventory runtime probe.
 *
 * Source lock: ReDMCSB WIP20210206 Toolchains/Common/Source.
 * - CHEST.C F0333_INVENTORY_OpenAndDrawChest lines 30-32: reopening the
 *   already-open chest returns before rewriting visible chest slots.
 * - CHEST.C F0333_INVENTORY_OpenAndDrawChest lines 53-76: a chest with no
 *   linked things fills all eight G0425_aT_ChestSlots with THING_NONE.
 * - CHEST.C F0334_INVENTORY_CloseChest lines 112-134: closing an open chest
 *   clears G0426_T_OpenChest and rewrites only non-empty visible slots.
 * - CHAMPION.C F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox lines
 *   688-695: an empty chest slot with an empty leader hand is a no-op.
 */

static int expect_int(const char* label, int got, int want)
{
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    return 1;
}

static int expect_item_type(const char* label, const DM1_V1_ItemPc34* item, int want)
{
    if (!item || item->itemType != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n",
                label, item ? item->itemType : -1, want);
        return 0;
    }
    return 1;
}

int main(void)
{
    DM1_V1_InventoryStatePc34 state;
    DM1_V1_ItemPc34 item;
    DM1_V1_ItemPc34 closed[DM1_PC34_CHEST_SLOT_COUNT];
    int ok = 1;
    int i;

    printf("probe=firestaff_dm1_v1_chest_empty_pointer_integrity_probe\n");
    printf("primarySource=ReDMCSB_WIP20210206/Toolchains/Common/Source\n");
    printf("sourceEvidence=CHEST.C:F0333:30-32,53-76;CHEST.C:F0334:112-134;CHAMPION.C:F0302:688-695\n");

    DM1_V1_Inventory_InitPc34Compat(&state, 2);
    ok &= expect_int("champion count initialized", state.championCount, 2);
    ok &= expect_int("leader action hand sentinel set",
                     DM1_V1_Inventory_SetItemInPc34SourceSlotCompat(
                         &state, 0, DM1_PC34_SLOT_ACTION_HAND,
                         144, 3, 0, DM1_PC34_ALLOWED_ANY_SLOT), 1);
    ok &= expect_int("second champion ready hand sentinel set",
                     DM1_V1_Inventory_SetItemInPc34SourceSlotCompat(
                         &state, 1, DM1_PC34_SLOT_READY_HAND,
                         77, 5, 0, DM1_PC34_ALLOWED_ANY_SLOT), 1);

    ok &= expect_int("empty chest opens",
                     DM1_V1_Inventory_OpenChestPc34Compat(&state, 0, 0x1234, NULL, 0), 1);
    ok &= expect_int("open chest thing set",
                     DM1_V1_Inventory_GetOpenChestThingPc34Compat(&state, 0), 0x1234);
    ok &= expect_int("empty chest does not change champion count",
                     state.championCount, 2);
    ok &= expect_int("empty chest visible slots add no load",
                     DM1_V1_Inventory_GetLoadPc34Compat(&state, 0), 3);

    for (i = 0; i < DM1_PC34_CHEST_SLOT_COUNT; ++i) {
        ok &= expect_int("empty visible chest slot readable",
                         DM1_V1_Inventory_GetItemInChestSlotPc34Compat(&state, 0, i, &item), 1);
        ok &= expect_item_type("empty visible chest slot remains empty", &item, 0);
    }

    ok &= expect_int("empty hand plus empty C537 no-ops",
                     DM1_V1_Inventory_ClickPc34SourceSlotCompat(
                         &state, 0, DM1_PC34_SLOT_CHEST_1), 0);
    ok &= expect_int("empty-slot no-op keeps chest open",
                     DM1_V1_Inventory_GetOpenChestThingPc34Compat(&state, 0), 0x1234);
    ok &= expect_int("reopen same empty chest is guarded no-op",
                     DM1_V1_Inventory_OpenChestPc34Compat(&state, 0, 0x1234, NULL, 0), 1);
    ok &= expect_int("guarded reopen keeps open chest thing",
                     DM1_V1_Inventory_GetOpenChestThingPc34Compat(&state, 0), 0x1234);

    ok &= expect_int("closing empty chest returns zero linked items",
                     DM1_V1_Inventory_CloseChestPc34Compat(&state, 0, closed,
                                               DM1_PC34_CHEST_SLOT_COUNT), 0);
    ok &= expect_int("close clears open chest",
                     DM1_V1_Inventory_GetOpenChestThingPc34Compat(&state, 0), 0);
    ok &= expect_int("closed chest hides chest source slots",
                     DM1_V1_Inventory_GetItemInChestSlotPc34Compat(&state, 0, 0, &item), 0);

    ok &= expect_int("leader action hand survives empty open-close",
                     DM1_V1_Inventory_GetItemInPc34SourceSlotCompat(
                         &state, 0, DM1_PC34_SLOT_ACTION_HAND, &item), 1);
    ok &= expect_item_type("leader action hand item unchanged", &item, 144);
    ok &= expect_int("second champion ready hand survives empty open-close",
                     DM1_V1_Inventory_GetItemInPc34SourceSlotCompat(
                         &state, 1, DM1_PC34_SLOT_READY_HAND, &item), 1);
    ok &= expect_item_type("second champion item unchanged", &item, 77);
    ok &= expect_int("leader load after close only counts champion item",
                     DM1_V1_Inventory_GetLoadPc34Compat(&state, 0), 3);
    ok &= expect_int("second champion load unchanged",
                     DM1_V1_Inventory_GetLoadPc34Compat(&state, 1), 5);

    ok &= expect_int("empty chest can reopen after close",
                     DM1_V1_Inventory_OpenChestPc34Compat(&state, 0, 0x5678, NULL, 0), 1);
    ok &= expect_int("reopened chest uses new thing",
                     DM1_V1_Inventory_GetOpenChestThingPc34Compat(&state, 0), 0x5678);
    ok &= expect_int("reopened empty chest closes cleanly",
                     DM1_V1_Inventory_CloseChestPc34Compat(&state, 0, NULL, 0), 0);
    ok &= expect_int("final close clears reopened chest",
                     DM1_V1_Inventory_GetOpenChestThingPc34Compat(&state, 0), 0);

    printf("result=%s\n", ok ? "ok" : "fail");
    return ok ? 0 : 1;
}
