#include "dm1_v1_inventory_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static int expect_int(const char* label,
                      int got,
                      int want,
                      const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        ++g_failures;
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n",
               label, got, want, redmcsbAnchor);
        return 0;
    }
    printf("PASS %s=%d anchor=%s\n", label, got, redmcsbAnchor);
    return 1;
}

static M11_Item make_item(int ordinal)
{
    M11_Item item;

    memset(&item, 0, sizeof(item));
    item.itemType = 0x5100 + ordinal;
    item.weight = 3 + ordinal;
    item.charges = ordinal;
    item.allowedSlots = DM1_PC34_ALLOWED_CONTAINER;
    return item;
}

static int expect_chest_slot(const M11_InventoryState* state,
                             int slot,
                             int wantItemType,
                             const char* redmcsbAnchor)
{
    M11_Item got;
    int ok = 1;

    ok &= expect_int("get visible chest slot",
                     m11_inventory_get_item_in_chest_slot(state, 0, slot, &got),
                     1, redmcsbAnchor);
    ok &= expect_int("visible chest slot item type",
                     got.itemType, wantItemType, redmcsbAnchor);
    return ok;
}

int main(void)
{
    const char* f0333FirstEight =
        "ReDMCSB CHEST.C F0333:53-76 copies only the first eight linked "
        "container things into G0425_aT_ChestSlots and clears the remaining "
        "visible C537..C544 slots with C0xFFFF_THING_NONE.";
    const char* f0333Replace =
        "ReDMCSB CHEST.C F0333:34-43 closes a different open G0426 chest "
        "before assigning the replacement chest and rematerializing G0425.";
    const char* f0333LimitedOut =
        "ReDMCSB CHEST.C F0333:53-76 copies only eight visible slots; output "
        "buffers for previous items are capped by maxPreviousItemsOut.";
    const char* f0334Compact =
        "ReDMCSB CHEST.C F0334:117-132 compacts non-empty G0425 slots back "
        "to the container list and ignores entries that are C0xFFFF_THING_NONE.";
    M11_InventoryState state;
    M11_Item overfull[10];
    M11_Item shortChest[3];
    M11_Item closed[10];
    M11_Item limitedOut[2];
    int ok = 1;
    int i;

    printf("probe=dm1_v1_chest_open_visible_slots_pc34_compat\n");
    printf("sourceEvidence=%s\n", f0333FirstEight);

    for (i = 0; i < 10; ++i) {
        overfull[i] = make_item(i + 1);
    }
    for (i = 0; i < 3; ++i) {
        shortChest[i] = make_item(40 + i);
    }

    m11_inventory_init(&state, 1);

    ok &= expect_int("open overfull chest",
                     m11_inventory_open_chest(&state, 0, 0x7001,
                                              overfull, 10),
                     1, f0333FirstEight);
    ok &= expect_int("panel content is chest",
                     m11_inventory_get_panel_content_pc34(&state),
                     DM1_PC34_PANEL_CHEST, f0333FirstEight);
    ok &= expect_int("open chest token",
                     m11_inventory_get_open_chest_thing(&state, 0),
                     0x7001, f0333FirstEight);
    for (i = 0; i < DM1_PC34_CHEST_SLOT_COUNT; ++i) {
        ok &= expect_chest_slot(&state, i, overfull[i].itemType,
                                f0333FirstEight);
    }

    memset(closed, 0, sizeof(closed));
    ok &= expect_int("replace returns eight visible previous items",
                     m11_inventory_open_chest_replacing_current(
                         &state, 0, 0x7002, shortChest, 3, closed, 10),
                     DM1_PC34_CHEST_SLOT_COUNT, f0333Replace);
    for (i = 0; i < DM1_PC34_CHEST_SLOT_COUNT; ++i) {
        ok &= expect_int("previous close preserved first-eight order",
                         closed[i].itemType, overfull[i].itemType,
                         f0334Compact);
    }
    ok &= expect_int("ninth linked source object was not visible",
                     closed[8].itemType, 0, f0333FirstEight);
    ok &= expect_int("tenth linked source object was not visible",
                     closed[9].itemType, 0, f0333FirstEight);

    for (i = 0; i < 3; ++i) {
        ok &= expect_chest_slot(&state, i, shortChest[i].itemType,
                                f0333FirstEight);
    }
    for (i = 3; i < DM1_PC34_CHEST_SLOT_COUNT; ++i) {
        ok &= expect_chest_slot(&state, i, 0, f0333FirstEight);
    }

    memset(closed, 0, sizeof(closed));
    ok &= expect_int("short replacement closes as three visible items",
                     m11_inventory_close_chest(&state, 0, closed, 10),
                     3, f0334Compact);
    for (i = 0; i < 3; ++i) {
        ok &= expect_int("short close order",
                         closed[i].itemType, shortChest[i].itemType,
                         f0334Compact);
    }
    ok &= expect_int("short close ignores cleared tail",
                     closed[3].itemType, 0, f0334Compact);
    ok &= expect_int("G0426 cleared after close",
                     m11_inventory_get_open_chest_thing(&state, 0),
                     0, f0334Compact);

    m11_inventory_init(&state, 1);
    ok &= expect_int("limited output scenario opens baseline chest",
                     m11_inventory_open_chest(&state, 0, 0x7003, overfull, 10),
                     1, f0333FirstEight);
    memset(limitedOut, 0, sizeof(limitedOut));
    ok &= expect_int("limited output scenario replaces current chest",
                     m11_inventory_open_chest_replacing_current(
                         &state, 0, 0x7004, shortChest, 3, limitedOut, 2),
                     DM1_PC34_CHEST_SLOT_COUNT, f0333LimitedOut);
    ok &= expect_int("limited output scenario first previous item",
                     limitedOut[0].itemType, overfull[0].itemType,
                     f0333LimitedOut);
    ok &= expect_int("limited output scenario second previous item",
                     limitedOut[1].itemType, overfull[1].itemType,
                     f0333LimitedOut);

    ok &= expect_int("limited output scenario new open chest",
                     m11_inventory_get_open_chest_thing(&state, 0),
                     0x7004, f0333Replace);
    for (i = 0; i < 3; ++i) {
        ok &= expect_chest_slot(&state, i, shortChest[i].itemType,
                                f0333LimitedOut);
    }
    for (i = 3; i < DM1_PC34_CHEST_SLOT_COUNT; ++i) {
        ok &= expect_chest_slot(&state, i, 0, f0333LimitedOut);
    }

    memset(closed, 0, sizeof(closed));
    ok &= expect_int("limited output scenario closes as three visible items",
                     m11_inventory_close_chest(&state, 0, closed, 10),
                     3, f0334Compact);
    for (i = 0; i < 3; ++i) {
        ok &= expect_int("limited output scenario close order",
                         closed[i].itemType, shortChest[i].itemType,
                         f0334Compact);
    }
    ok &= expect_int("limited output scenario ignores cleared tail",
                     closed[3].itemType, 0, f0334Compact);

    printf("assertionCount=%d\n", g_assertions);
    printf("dm1V1ChestOpenVisibleSlotsInvariantOk=%d\n",
           ok && g_failures == 0);
    return ok && g_failures == 0 ? 0 : 1;
}
