#include "dm1_v1_inventory_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_init(void)
{
    DM1_V1_InventoryStatePc34 state;

    DM1_V1_Inventory_InitPc34Compat(&state, 4);
    assert(state.championCount == 4);
    assert(state.panelContent == DM1_PC34_PANEL_INVENTORY);
}

static void test_set_get_item(void)
{
    DM1_V1_InventoryStatePc34 state;
    DM1_V1_ItemPc34 out;
    int rc;

    DM1_V1_Inventory_InitPc34Compat(&state, 4);
    rc = DM1_V1_Inventory_SetItemPc34Compat(&state, 0, DM1_SLOT_HAND_RIGHT, 5, 10, 0);
    (void)rc;
    assert(rc == 1);

    memset(&out, 0, sizeof(out));
    rc = DM1_V1_Inventory_GetItemPc34Compat(&state, 0, DM1_SLOT_HAND_RIGHT, &out);
    assert(rc == 1);
    assert(out.itemType == 5);
    assert(out.weight == 10);
}

static void test_remove_item(void)
{
    DM1_V1_InventoryStatePc34 state;
    DM1_V1_ItemPc34 out;
    int rc;

    DM1_V1_Inventory_InitPc34Compat(&state, 4);
    DM1_V1_Inventory_SetItemPc34Compat(&state, 0, DM1_SLOT_HEAD, 1, 5, 0);
    rc = DM1_V1_Inventory_RemoveItemPc34Compat(&state, 0, DM1_SLOT_HEAD);
    (void)rc;
    assert(rc == 1);

    memset(&out, 0, sizeof(out));
    rc = DM1_V1_Inventory_GetItemPc34Compat(&state, 0, DM1_SLOT_HEAD, &out);
    assert(out.itemType == 0);
}

static void test_swap_hand(void)
{
    DM1_V1_InventoryStatePc34 state;
    int rc;

    DM1_V1_Inventory_InitPc34Compat(&state, 4);
    DM1_V1_Inventory_SetItemPc34Compat(&state, 0, DM1_SLOT_HAND_RIGHT, 5, 10, 0);
    rc = DM1_V1_Inventory_SwapHandPc34Compat(&state, 0);
    (void)rc;
    assert(rc == 1);
}

static void test_mouse_item(void)
{
    DM1_V1_InventoryStatePc34 state;
    DM1_V1_ItemPc34 out;
    int rc;

    DM1_V1_Inventory_InitPc34Compat(&state, 4);
    rc = DM1_V1_Inventory_SetMouseItemPc34Compat(&state, 0, 3, 7, 2, DM1_PC34_ALLOWED_ANY_SLOT);
    (void)rc;
    assert(rc == 1);

    memset(&out, 0, sizeof(out));
    rc = DM1_V1_Inventory_GetMouseItemPc34Compat(&state, 0, &out);
    assert(rc == 1);
    assert(out.itemType == 3);
    assert(out.weight == 7);
    assert(out.charges == 2);
}

static void test_load_recalc(void)
{
    DM1_V1_InventoryStatePc34 state;
    int load;

    DM1_V1_Inventory_InitPc34Compat(&state, 4);
    DM1_V1_Inventory_SetItemPc34Compat(&state, 0, DM1_SLOT_HEAD, 1, 5, 0);
    DM1_V1_Inventory_SetItemPc34Compat(&state, 0, DM1_SLOT_TORSO, 2, 15, 0);
    DM1_V1_Inventory_RecalcLoadPc34Compat(&state, 0);
    load = DM1_V1_Inventory_GetLoadPc34Compat(&state, 0);
    (void)load;
    assert(load == 20);
}

static void test_pc34_slot_mapping(void)
{
    int mask;

    mask = DM1_V1_Inventory_Pc34SlotMaskCompat(DM1_PC34_SLOT_HEAD);
    (void)mask;
    assert(mask == DM1_PC34_ALLOWED_HEAD);

    mask = DM1_V1_Inventory_Pc34SlotMaskCompat(DM1_PC34_SLOT_NECK);
    assert(mask == DM1_PC34_ALLOWED_NECK);

    mask = DM1_V1_Inventory_Pc34SlotMaskCompat(DM1_PC34_SLOT_FEET);
    assert(mask == DM1_PC34_ALLOWED_FEET);
}

static void test_pc34_source_to_storage(void)
{
    int storage;

    storage = DM1_V1_Inventory_Pc34SourceSlotToStorageSlotCompat(DM1_PC34_SLOT_HEAD);
    (void)storage;
    assert(storage == DM1_SLOT_HEAD);
}

static void test_is_backpack_slot(void)
{
    int rc;

    rc = DM1_V1_Inventory_Pc34IsBackpackSourceSlotCompat(DM1_PC34_SLOT_BACKPACK_LINE1_1);
    (void)rc;
    assert(rc == 1);

    rc = DM1_V1_Inventory_Pc34IsBackpackSourceSlotCompat(DM1_PC34_SLOT_HEAD);
    assert(rc == 0);
}

static void test_is_chest_slot(void)
{
    int rc;

    rc = DM1_V1_Inventory_Pc34IsChestSourceSlotCompat(DM1_PC34_SLOT_CHEST_1);
    (void)rc;
    assert(rc == 1);

    rc = DM1_V1_Inventory_Pc34IsChestSourceSlotCompat(DM1_PC34_SLOT_HEAD);
    assert(rc == 0);
}

static void test_can_equip(void)
{
    DM1_V1_ItemPc34 item;
    int rc;

    memset(&item, 0, sizeof(item));
    item.allowedSlots = DM1_PC34_ALLOWED_HEAD;
    rc = DM1_V1_Inventory_CanEquipPc34Compat(&item, DM1_PC34_SLOT_HEAD);
    (void)rc;
    assert(rc == 1);

    rc = DM1_V1_Inventory_CanEquipPc34Compat(&item, DM1_PC34_SLOT_FEET);
    assert(rc == 0);
}

static void test_panel_content(void)
{
    DM1_V1_InventoryStatePc34 state;
    int rc;

    DM1_V1_Inventory_InitPc34Compat(&state, 4);
    rc = DM1_V1_Inventory_SetPanelContentPc34Compat(&state, DM1_PC34_PANEL_CHEST);
    (void)rc;
    assert(rc == 1);
    assert(DM1_V1_Inventory_GetPanelContentPc34Compat(&state) == DM1_PC34_PANEL_CHEST);
}

static void test_open_close_chest(void)
{
    DM1_V1_InventoryStatePc34 state;
    DM1_V1_ItemPc34 items[2];
    DM1_V1_ItemPc34 closedItems[8];
    int rc;

    DM1_V1_Inventory_InitPc34Compat(&state, 4);
    memset(items, 0, sizeof(items));
    items[0].itemType = 10;
    items[0].weight = 3;
    items[1].itemType = 11;
    items[1].weight = 4;

    rc = DM1_V1_Inventory_OpenChestPc34Compat(&state, 0, 42, items, 2);
    (void)rc;
    assert(rc == 1);
    assert(DM1_V1_Inventory_GetOpenChestThingPc34Compat(&state, 0) == 42);

    memset(closedItems, 0, sizeof(closedItems));
    rc = DM1_V1_Inventory_CloseChestPc34Compat(&state, 0, closedItems, 8);
    assert(rc >= 0);
}

static void test_source_evidence(void)
{
    const char *ev = dm1_inventory_pass601_inventory_source_evidence();
    (void)ev;
    assert(ev != NULL);
    assert(strlen(ev) > 0);
}

static void test_chest_stale_evidence(void)
{
    const char *ev = dm1_inventory_chest_stale_click_source_evidence_pc34();
    (void)ev;
    assert(ev != NULL);
    assert(strlen(ev) > 0);
}

static void test_out_of_range_champion(void)
{
    DM1_V1_InventoryStatePc34 state;
    DM1_V1_ItemPc34 out;
    int rc;

    DM1_V1_Inventory_InitPc34Compat(&state, 2);
    rc = DM1_V1_Inventory_SetItemPc34Compat(&state, 5, DM1_SLOT_HEAD, 1, 1, 0);
    (void)rc;
    assert(rc == 0);

    memset(&out, 0, sizeof(out));
    rc = DM1_V1_Inventory_GetItemPc34Compat(&state, 5, DM1_SLOT_HEAD, &out);
    assert(rc == 0);
}

static void test_pickup_drop_mouse(void)
{
    DM1_V1_InventoryStatePc34 state;
    int rc;

    DM1_V1_Inventory_InitPc34Compat(&state, 4);
    DM1_V1_Inventory_SetItemPc34Compat(&state, 0, DM1_SLOT_HAND_RIGHT, 5, 10, 0);
    rc = DM1_V1_Inventory_PickupMousePc34Compat(&state, 0, DM1_SLOT_HAND_RIGHT);
    (void)rc;
    assert(rc == 1);

    rc = DM1_V1_Inventory_DropMousePc34Compat(&state, 0, DM1_SLOT_HAND_LEFT);
    assert(rc == 1);
}

int main(void)
{
    test_init();
    test_set_get_item();
    test_remove_item();
    test_swap_hand();
    test_mouse_item();
    test_load_recalc();
    test_pc34_slot_mapping();
    test_pc34_source_to_storage();
    test_is_backpack_slot();
    test_is_chest_slot();
    test_can_equip();
    test_panel_content();
    test_open_close_chest();
    test_source_evidence();
    test_chest_stale_evidence();
    test_out_of_range_champion();
    test_pickup_drop_mouse();

    puts("ok: DM1 inventory (Q-DM1-06) 17 tests passed");
    return 0;
}
