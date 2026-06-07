#include "dm1_v1_inventory_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/* Runtime regression for the DM1 V1 backpack <-> chest path.
 *
 * ReDMCSB anchors:
 * - CHEST.C F0333_INVENTORY_OpenAndDrawChest lines 53-76 copies the first
 *   eight linked things into G0425_aT_ChestSlots.
 * - CHEST.C F0334_INVENTORY_CloseChest lines 113-132 clears G0426_T_OpenChest
 *   and recompacts non-empty G0425 slots back into container order.
 * - CHAMPION.C F0297/F0298 lines 243-298 put/remove a thing in the leader
 *   hand and update champion load.
 * - CHAMPION.C F0300/F0301 lines 511-515 and 606-614 remove/add C30+ chest
 *   slots through G0425_aT_ChestSlots while ordinary backpack slots use
 *   champion Slots[] storage.
 */

enum {
    ITEM_BACKPACK_A = 8101,
    ITEM_BACKPACK_B = 8102,
    ITEM_BACKPACK_C = 8103,
    ITEM_CHEST_A = 8201,
    ITEM_CHEST_B = 8202,
    ITEM_CHEST_C = 8203,
    WEIGHT_BACKPACK_A = 10,
    WEIGHT_BACKPACK_B = 20,
    WEIGHT_BACKPACK_C = 30,
    WEIGHT_CHEST_A = 7,
    WEIGHT_CHEST_B = 11,
    WEIGHT_CHEST_C = 13,
    CHEST_THING = 0x6bc1
};

static int g_assertions;
static int g_passes;

static M11_Item make_item(int itemType, int weight)
{
    M11_Item item;

    memset(&item, 0, sizeof(item));
    item.itemType = itemType;
    item.weight = weight;
    item.allowedSlots = DM1_PC34_ALLOWED_ANY_SLOT;
    return item;
}

static int expect_int(const char* label, int got, int want,
                      const char* anchor)
{
    ++g_assertions;
    if (!anchor || anchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n", label, got, want,
               anchor);
        return 0;
    }
    ++g_passes;
    return 1;
}

static int expect_slot_item(const char* label,
                            const M11_InventoryState* state,
                            int pc34Slot,
                            int wantItem,
                            int wantWeight,
                            const char* anchor)
{
    M11_Item item;
    int ok = 1;

    ok &= expect_int(label,
                     m11_inventory_get_item_in_pc34_source_slot(state, 0,
                                                                pc34Slot,
                                                                &item),
                     1, anchor);
    ok &= expect_int(label, item.itemType, wantItem, anchor);
    ok &= expect_int(label, item.weight, wantWeight, anchor);
    return ok;
}

static int expect_mouse_item(const char* label,
                             const M11_InventoryState* state,
                             int wantItem,
                             int wantWeight,
                             const char* anchor)
{
    M11_Item item;
    int ok = 1;

    ok &= expect_int(label, m11_inventory_get_mouse_item(state, 0, &item), 1,
                     anchor);
    ok &= expect_int(label, item.itemType, wantItem, anchor);
    ok &= expect_int(label, item.weight, wantWeight, anchor);
    return ok;
}

static int test_backpack_chest_round_trip(void)
{
    const char* f0333 = "ReDMCSB CHEST.C F0333 lines 53-76";
    const char* f0334 = "ReDMCSB CHEST.C F0334 lines 113-132";
    const char* f0297 = "ReDMCSB CHAMPION.C F0297/F0298 lines 243-298";
    const char* f0301 =
        "ReDMCSB CHAMPION.C F0300/F0301 lines 511-515,606-614";
    M11_InventoryState state;
    M11_Item chest[DM1_PC34_CHEST_SLOT_COUNT];
    M11_Item closed[DM1_PC34_CHEST_SLOT_COUNT];
    int ok = 1;
    const int backpackLoad =
        WEIGHT_BACKPACK_A + WEIGHT_BACKPACK_B + WEIGHT_BACKPACK_C;
    const int chestLoad = WEIGHT_CHEST_A + WEIGHT_CHEST_B + WEIGHT_CHEST_C;
    const int initialLoad = backpackLoad + chestLoad;

    memset(chest, 0, sizeof(chest));
    memset(closed, 0, sizeof(closed));
    m11_inventory_init(&state, 1);

    ok &= expect_int("backpack slot 1 storage",
                     m11_inventory_pc34_source_slot_to_storage_slot(
                         DM1_PC34_SLOT_BACKPACK_LINE1_1),
                     DM1_SLOT_BACKPACK1, f0301);
    ok &= expect_int("backpack slot 2 storage",
                     m11_inventory_pc34_source_slot_to_storage_slot(
                         DM1_PC34_SLOT_BACKPACK_LINE1_2),
                     DM1_SLOT_BACKPACK10, f0301);
    ok &= expect_int("backpack slot 3 storage",
                     m11_inventory_pc34_source_slot_to_storage_slot(
                         DM1_PC34_SLOT_BACKPACK_LINE1_3),
                     DM1_SLOT_BACKPACK11, f0301);

    ok &= expect_int("seed backpack A",
                     m11_inventory_set_item_in_pc34_source_slot(
                         &state, 0, DM1_PC34_SLOT_BACKPACK_LINE1_1,
                         ITEM_BACKPACK_A, WEIGHT_BACKPACK_A, 0,
                         DM1_PC34_ALLOWED_ANY_SLOT),
                     1, f0301);
    ok &= expect_int("seed backpack B",
                     m11_inventory_set_item_in_pc34_source_slot(
                         &state, 0, DM1_PC34_SLOT_BACKPACK_LINE1_2,
                         ITEM_BACKPACK_B, WEIGHT_BACKPACK_B, 0,
                         DM1_PC34_ALLOWED_ANY_SLOT),
                     1, f0301);
    ok &= expect_int("seed backpack C",
                     m11_inventory_set_item_in_pc34_source_slot(
                         &state, 0, DM1_PC34_SLOT_BACKPACK_LINE1_3,
                         ITEM_BACKPACK_C, WEIGHT_BACKPACK_C, 0,
                         DM1_PC34_ALLOWED_ANY_SLOT),
                     1, f0301);
    ok &= expect_int("load before chest open",
                     m11_inventory_get_load(&state, 0), backpackLoad, f0301);

    chest[0] = make_item(ITEM_CHEST_A, WEIGHT_CHEST_A);
    chest[1] = make_item(ITEM_CHEST_B, WEIGHT_CHEST_B);
    chest[3] = make_item(ITEM_CHEST_C, WEIGHT_CHEST_C);
    ok &= expect_int("open chest",
                     m11_inventory_open_chest(&state, 0, CHEST_THING, chest,
                                              DM1_PC34_CHEST_SLOT_COUNT),
                     1, f0333);
    ok &= expect_int("open chest load includes visible chest slots",
                     m11_inventory_get_load(&state, 0), initialLoad, f0333);

    ok &= expect_int("pick backpack A into leader hand",
                     m11_inventory_click_pc34_source_slot(
                         &state, 0, DM1_PC34_SLOT_BACKPACK_LINE1_1),
                     1, f0297);
    ok &= expect_mouse_item("leader hand carries backpack A", &state,
                            ITEM_BACKPACK_A, WEIGHT_BACKPACK_A, f0297);
    ok &= expect_slot_item("backpack A slot emptied", &state,
                           DM1_PC34_SLOT_BACKPACK_LINE1_1, 0, 0, f0301);
    ok &= expect_int("load drops by backpack A while carried",
                     m11_inventory_get_load(&state, 0),
                     initialLoad - WEIGHT_BACKPACK_A, f0297);

    ok &= expect_int("drop backpack A into empty chest slot",
                     m11_inventory_click_pc34_source_slot(
                         &state, 0, DM1_PC34_SLOT_CHEST_3),
                     1, f0301);
    ok &= expect_mouse_item("leader hand clears after chest add", &state, 0, 0,
                            f0301);
    ok &= expect_slot_item("chest slot 3 receives backpack A", &state,
                           DM1_PC34_SLOT_CHEST_3, ITEM_BACKPACK_A,
                           WEIGHT_BACKPACK_A, f0301);
    ok &= expect_int("load restored after backpack A enters chest",
                     m11_inventory_get_load(&state, 0), initialLoad, f0301);

    ok &= expect_int("pick different chest item B",
                     m11_inventory_click_pc34_source_slot(
                         &state, 0, DM1_PC34_SLOT_CHEST_2),
                     1, f0297);
    ok &= expect_mouse_item("leader hand carries chest B", &state,
                            ITEM_CHEST_B, WEIGHT_CHEST_B, f0297);
    ok &= expect_slot_item("chest B slot emptied", &state,
                           DM1_PC34_SLOT_CHEST_2, 0, 0, f0301);
    ok &= expect_int("load drops by chest B while carried",
                     m11_inventory_get_load(&state, 0),
                     initialLoad - WEIGHT_CHEST_B, f0297);

    ok &= expect_int("return chest B to freed backpack slot",
                     m11_inventory_click_pc34_source_slot(
                         &state, 0, DM1_PC34_SLOT_BACKPACK_LINE1_1),
                     1, f0301);
    ok &= expect_mouse_item("leader hand clears after backpack add", &state,
                            0, 0, f0301);
    ok &= expect_slot_item("backpack slot 1 receives chest B", &state,
                           DM1_PC34_SLOT_BACKPACK_LINE1_1, ITEM_CHEST_B,
                           WEIGHT_CHEST_B, f0301);
    ok &= expect_int("round-trip load restored",
                     m11_inventory_get_load(&state, 0), initialLoad, f0301);

    ok &= expect_int("pick backpack B for occupied chest swap",
                     m11_inventory_click_pc34_source_slot(
                         &state, 0, DM1_PC34_SLOT_BACKPACK_LINE1_2),
                     1, f0297);
    ok &= expect_int("load drops by backpack B while carried",
                     m11_inventory_get_load(&state, 0),
                     initialLoad - WEIGHT_BACKPACK_B, f0297);
    ok &= expect_int("swap backpack B with occupied chest slot 1",
                     m11_inventory_click_pc34_source_slot(
                         &state, 0, DM1_PC34_SLOT_CHEST_1),
                     1, f0301);
    ok &= expect_slot_item("occupied chest slot now holds backpack B", &state,
                           DM1_PC34_SLOT_CHEST_1, ITEM_BACKPACK_B,
                           WEIGHT_BACKPACK_B, f0301);
    ok &= expect_mouse_item("leader hand receives displaced chest A", &state,
                            ITEM_CHEST_A, WEIGHT_CHEST_A, f0297);
    ok &= expect_int("load excludes displaced chest A while carried",
                     m11_inventory_get_load(&state, 0),
                     initialLoad - WEIGHT_CHEST_A, f0297);

    ok &= expect_int("return displaced chest A to backpack slot 2",
                     m11_inventory_click_pc34_source_slot(
                         &state, 0, DM1_PC34_SLOT_BACKPACK_LINE1_2),
                     1, f0301);
    ok &= expect_mouse_item("leader hand clears after displaced add", &state,
                            0, 0, f0301);
    ok &= expect_slot_item("backpack slot 2 receives displaced chest A",
                           &state, DM1_PC34_SLOT_BACKPACK_LINE1_2,
                           ITEM_CHEST_A, WEIGHT_CHEST_A, f0301);
    ok &= expect_slot_item("backpack slot 3 remains stable", &state,
                           DM1_PC34_SLOT_BACKPACK_LINE1_3, ITEM_BACKPACK_C,
                           WEIGHT_BACKPACK_C, f0301);
    ok &= expect_int("swap round-trip load restored",
                     m11_inventory_get_load(&state, 0), initialLoad, f0301);

    ok &= expect_int("close chest recompacts sparse slots",
                     m11_inventory_close_chest(&state, 0, closed,
                                               DM1_PC34_CHEST_SLOT_COUNT),
                     3, f0334);
    ok &= expect_int("closed slot 0 holds swapped backpack B",
                     closed[0].itemType, ITEM_BACKPACK_B, f0334);
    ok &= expect_int("closed slot 1 skips empty old chest B slot",
                     closed[1].itemType, ITEM_BACKPACK_A, f0334);
    ok &= expect_int("closed slot 2 preserves later chest C",
                     closed[2].itemType, ITEM_CHEST_C, f0334);
    ok &= expect_int("closed slot 3 remains empty after recompaction",
                     closed[3].itemType, 0, f0334);
    ok &= expect_int("load after close is backpack only",
                     m11_inventory_get_load(&state, 0),
                     WEIGHT_CHEST_B + WEIGHT_CHEST_A + WEIGHT_BACKPACK_C,
                     f0334);

    ok &= expect_int("reopen compacted chest",
                     m11_inventory_open_chest(&state, 0, CHEST_THING, closed,
                                              DM1_PC34_CHEST_SLOT_COUNT),
                     1, f0333);
    ok &= expect_slot_item("reopen slot 1 is compacted backpack B", &state,
                           DM1_PC34_SLOT_CHEST_1, ITEM_BACKPACK_B,
                           WEIGHT_BACKPACK_B, f0333);
    ok &= expect_slot_item("reopen slot 2 is compacted backpack A", &state,
                           DM1_PC34_SLOT_CHEST_2, ITEM_BACKPACK_A,
                           WEIGHT_BACKPACK_A, f0333);
    ok &= expect_slot_item("reopen slot 3 is compacted chest C", &state,
                           DM1_PC34_SLOT_CHEST_3, ITEM_CHEST_C,
                           WEIGHT_CHEST_C, f0333);
    ok &= expect_slot_item("reopen slot 4 stays empty", &state,
                           DM1_PC34_SLOT_CHEST_4, 0, 0, f0333);
    ok &= expect_int("final load matches open compacted state",
                     m11_inventory_get_load(&state, 0), initialLoad, f0333);

    return ok;
}

int main(void)
{
    int ok;

    printf("probe=dm1_v1_inventory_backpack_chest_round_trip_pc34_compat\n");
    printf("sourceEvidence=CHEST.C F0333:53-76; CHEST.C F0334:113-132; "
           "CHAMPION.C F0297/F0298:243-298; "
           "CHAMPION.C F0300/F0301:511-515,606-614\n");
    ok = test_backpack_chest_round_trip();
    if (!ok) {
        printf("FAIL dm1_v1_inventory_backpack_chest_round_trip_pc34_compat "
               "%d/%d assertions\n", g_passes, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_inventory_backpack_chest_round_trip_pc34_compat "
           "%d/%d assertions\n", g_passes, g_assertions);
    return 0;
}
