#include "dm1_v1_chest_runtime_encumbrance_pc34_compat.h"

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

static void copy_closed_items(
    const M11_Item* closed,
    DM1_V1_ChestRuntimeEncumbranceProbePc34* out)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_RUNTIME_ENCUMBRANCE_SLOT_COUNT; ++i) {
        out->closedItemTypes[i] = closed[i].itemType;
        out->closedItemWeights[i] = closed[i].weight;
    }
}

const char* dm1_v1_chest_runtime_encumbrance_source_evidence_pc34(void)
{
    return
        "CHEST.C:53-76 F0333 copies linked chest contents into G0425 slots\n"
        "CHEST.C:113-132 F0334 closes G0426, compacts G0425, and clears open slots\n"
        "CHAMPION.C:263-265 F0297 adjusts leader Load for hand objects\n"
        "CHAMPION.C:582-615 F0300/F0301 remove/add slot weight and mark Load dirty\n"
        "CHAMPION.C:1157-1205 F0309/F0310 consume champion Load for encumbrance\n"
        "COMMAND.C:2174-2180 routes slot-box commands through F0302 when a leader exists";
}

int dm1_v1_chest_runtime_encumbrance_run_pc34(
    DM1_V1_ChestRuntimeEncumbranceProbePc34* out)
{
    enum {
        LEADER = 0,
        BYSTANDER = 1,
        TEST_CHEST_THING = 0x6E71,
        LEADER_PACK_ITEM = 0x4101,
        BYSTANDER_PACK_ITEM = 0x4201
    };
    M11_InventoryState state;
    M11_Item linked[3];
    M11_Item closed[DM1_PC34_CHEST_RUNTIME_ENCUMBRANCE_SLOT_COUNT];

    if (!out) {
        return 0;
    }

    memset(out, 0, sizeof(*out));
    memset(closed, 0, sizeof(closed));
    m11_inventory_init(&state, 2);

    /* ReDMCSB CHAMPION.C F0301 lines 609-615 updates the per-champion Load
     * when ordinary inventory slots receive objects. */
    out->leaderBaseSetResult = m11_inventory_set_item_in_pc34_source_slot(
        &state, LEADER, DM1_PC34_SLOT_BACKPACK_LINE1_1, LEADER_PACK_ITEM, 15,
        0, DM1_PC34_ALLOWED_ANY_SLOT);
    out->bystanderBaseSetResult = m11_inventory_set_item_in_pc34_source_slot(
        &state, BYSTANDER, DM1_PC34_SLOT_BACKPACK_LINE1_1,
        BYSTANDER_PACK_ITEM, 77, 0, DM1_PC34_ALLOWED_ANY_SLOT);
    if (!out->leaderBaseSetResult || !out->bystanderBaseSetResult) {
        return 0;
    }
    out->leaderBaseLoad = m11_inventory_get_load(&state, LEADER);
    out->bystanderLoadBeforeOpen = m11_inventory_get_load(&state, BYSTANDER);

    linked[0] = make_item(0x5101, 10);
    linked[1] = make_item(0x5102, 20);
    linked[2] = make_item(0x5103, 30);

    /* ReDMCSB CHEST.C F0333 lines 53-76 materializes linked chest contents
     * into the open G0425 slots that the runtime inventory path weighs. */
    out->openResult = m11_inventory_open_chest(
        &state, LEADER, TEST_CHEST_THING, linked, 3);
    if (!out->openResult) {
        return 0;
    }
    out->openChestThingAfterOpen =
        m11_inventory_get_open_chest_thing(&state, LEADER);
    out->visibleContentsWeight =
        m11_inventory_pc34_open_chest_visible_contents_weight(&state, LEADER);
    out->openContainerWeight =
        m11_inventory_pc34_open_chest_container_weight(&state, LEADER);
    out->leaderLoadAfterOpen = m11_inventory_get_load(&state, LEADER);

    /* ReDMCSB CHEST.C F0334 lines 117-132 closes by compacting G0425 and
     * clearing G0426; CHAMPION.C F0300/F0301 lines 582-615 require the leader
     * Load/encumbrance view to be recomputed after those transient slots drop. */
    out->closeCount = m11_inventory_pc34_close_chest_with_weight_snapshot(
        &state, LEADER, closed,
        DM1_PC34_CHEST_RUNTIME_ENCUMBRANCE_SLOT_COUNT,
        &out->closeContainerWeightSnapshot);
    if (out->closeCount < 0) {
        return 0;
    }
    copy_closed_items(closed, out);
    out->closeContainerWeightAfter =
        m11_inventory_pc34_open_chest_container_weight(&state, LEADER);
    out->openChestThingAfterClose =
        m11_inventory_get_open_chest_thing(&state, LEADER);
    out->leaderLoadAfterClose = m11_inventory_get_load(&state, LEADER);
    out->bystanderLoadAfterClose = m11_inventory_get_load(&state, BYSTANDER);

    return 1;
}
