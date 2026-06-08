#include "dm1_v1_chest_same_open_noop_pc34_compat.h"

#include "dm1_v1_inventory_chest_load_pc34_compat.h"

#include <string.h>

static M11_Item make_item(int itemType, int weight)
{
    M11_Item item;

    memset(&item, 0, sizeof(item));
    item.itemType = itemType;
    item.weight = weight;
    item.allowedSlots = DM1_PC34_ALLOWED_CONTAINER;
    return item;
}

static void copy_chest_slot_types(
    const M11_InventoryState* state,
    int champ,
    int* outTypes)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_SAME_OPEN_SLOT_COUNT; ++i) {
        M11_Item item;

        outTypes[i] = 0;
        if (m11_inventory_get_item_in_chest_slot(state, champ, i, &item)) {
            outTypes[i] = item.itemType;
        }
    }
}

static int count_b_items_in_open_slots(const M11_InventoryState* state,
                                       int champ)
{
    int count = 0;
    int i;

    for (i = 0; i < DM1_PC34_CHEST_SAME_OPEN_SLOT_COUNT; ++i) {
        M11_Item item;

        if (m11_inventory_get_item_in_chest_slot(state, champ, i, &item) &&
            item.itemType >= DM1_PC34_CHEST_SAME_OPEN_ITEM_B1 &&
            item.itemType < DM1_PC34_CHEST_SAME_OPEN_ITEM_B1 + 8) {
            ++count;
        }
    }
    return count;
}

const char* dm1_v1_chest_same_open_noop_source_evidence_pc34(void)
{
    return
        "CHEST.C F0333 line 28 sets M569_PANEL_CHEST before same-open return\n"
        "CHEST.C F0333 lines 30-32 returns when the requested chest is already G0426_T_OpenChest\n"
        "CHEST.C F0333 lines 53-76 materializes linked contents into G0425_aT_ChestSlots only on the real open path\n"
        "CHEST.C F0334 lines 113-132 closes/relinks only when the open path reaches the different-chest close guard\n"
        "CHAMPION.C F0302 lines 688-710 swaps leader hand with C30+ G0425 chest slots";
}

int dm1_v1_chest_same_open_noop_run_pc34(
    DM1_V1_ChestSameOpenNoopProbePc34* out)
{
    enum {
        CHAMPION = 0
    };
    M11_InventoryState state;
    M11_Item initial[4];
    M11_Item alternate[8];
    M11_Item hand;
    M11_Item c538;
    M11_Item closed[DM1_PC34_CHEST_SAME_OPEN_SLOT_COUNT];
    int i;

    if (!out) {
        return 0;
    }

    memset(out, 0, sizeof(*out));
    memset(closed, 0, sizeof(closed));
    m11_inventory_init(&state, 1);

    for (i = 0; i < 4; ++i) {
        initial[i] = make_item(DM1_PC34_CHEST_SAME_OPEN_ITEM_A1 + i,
                               10 + (i * 10));
    }
    for (i = 0; i < 8; ++i) {
        alternate[i] = make_item(DM1_PC34_CHEST_SAME_OPEN_ITEM_B1 + i,
                                 100 + i);
    }

    out->setupResult = 1;
    out->firstOpenResult = m11_inventory_open_chest(
        &state, CHAMPION, DM1_PC34_CHEST_SAME_OPEN_THING, initial, 4);
    if (!out->firstOpenResult) {
        return 0;
    }
    out->openThingAfterFirstOpen =
        m11_inventory_get_open_chest_thing(&state, CHAMPION);
    copy_chest_slot_types(&state, CHAMPION, out->firstOpenSlotTypes);
    out->firstOpenVisibleWeight =
        m11_inventory_pc34_open_chest_visible_contents_weight(&state,
                                                              CHAMPION);

    /* ReDMCSB CHAMPION.C F0302 lines 688-710 reads the current G0425 C538
     * entry and swaps it into the leader hand, leaving a sparse open panel. */
    out->pickupC538Result = m11_inventory_click_pc34_source_slot(
        &state, CHAMPION, DM1_PC34_SLOT_CHEST_2);
    if (!out->pickupC538Result) {
        return 0;
    }
    if (m11_inventory_get_mouse_item(&state, CHAMPION, &hand)) {
        out->leaderHandAfterPickup = hand.itemType;
    }
    if (m11_inventory_get_item_in_chest_slot(&state, CHAMPION, 1, &c538)) {
        out->c538AfterPickup = c538.itemType;
    }
    out->visibleWeightAfterPickup =
        m11_inventory_pc34_open_chest_visible_contents_weight(&state,
                                                              CHAMPION);
    out->loadAfterPickup = m11_inventory_get_load(&state, CHAMPION);

    /* ReDMCSB CHEST.C F0333 lines 30-32 must return before re-reading the
     * alternate chain; otherwise the sparse G0425 view is compacted/replaced. */
    out->sameOpenResult = m11_inventory_open_chest(
        &state, CHAMPION, DM1_PC34_CHEST_SAME_OPEN_THING, alternate, 8);
    out->openThingAfterSameOpen =
        m11_inventory_get_open_chest_thing(&state, CHAMPION);
    if (m11_inventory_get_mouse_item(&state, CHAMPION, &hand)) {
        out->leaderHandAfterSameOpen = hand.itemType;
    }
    out->c537AfterSameOpen = state.champions[CHAMPION].chestSlots[0].itemType;
    out->c538AfterSameOpen = state.champions[CHAMPION].chestSlots[1].itemType;
    out->c539AfterSameOpen = state.champions[CHAMPION].chestSlots[2].itemType;
    out->c540AfterSameOpen = state.champions[CHAMPION].chestSlots[3].itemType;

    m11_inventory_set_panel_content_pc34(&state, DM1_PC34_PANEL_SCROLL);
    out->panelContentBeforeReplacingSameOpen =
        m11_inventory_get_panel_content_pc34(&state);
    out->replacingSameOpenResult = m11_inventory_open_chest_replacing_current(
        &state, CHAMPION, DM1_PC34_CHEST_SAME_OPEN_THING, alternate, 8,
        closed, DM1_PC34_CHEST_SAME_OPEN_SLOT_COUNT);
    out->panelContentAfterReplacingSameOpen =
        m11_inventory_get_panel_content_pc34(&state);
    out->openThingAfterReplacingSameOpen =
        m11_inventory_get_open_chest_thing(&state, CHAMPION);
    out->c538AfterReplacingSameOpen =
        state.champions[CHAMPION].chestSlots[1].itemType;

    out->bItemsLeakedAfterSameOpen =
        count_b_items_in_open_slots(&state, CHAMPION);
    out->visibleWeightAfterSameOpen =
        m11_inventory_pc34_open_chest_visible_contents_weight(&state,
                                                              CHAMPION);
    out->loadAfterSameOpen = m11_inventory_get_load(&state, CHAMPION);

    out->closeCountAfterSameOpen = m11_inventory_close_chest(
        &state, CHAMPION, closed, DM1_PC34_CHEST_SAME_OPEN_SLOT_COUNT);
    for (i = 0; i < DM1_PC34_CHEST_SAME_OPEN_SLOT_COUNT; ++i) {
        out->closedItemTypes[i] = closed[i].itemType;
    }

    return 1;
}
