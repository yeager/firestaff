#include "dm1_v1_chest_close_rewire_runtime_pc34_compat.h"

#include "dm1_v1_inventory_chest_load_pc34_compat.h"

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

static int copy_visible_chest_types(const M11_InventoryState* state, int champ,
                                    int* typesOut)
{
    int i;

    if (!state || !typesOut) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_CLOSE_REWIRE_TEST_SLOT_COUNT; ++i) {
        M11_Item item;
        if (!m11_inventory_get_item_in_chest_slot(state, champ, i, &item)) {
            return 0;
        }
        typesOut[i] = item.itemType;
    }
    return 1;
}

static int copy_item_types(const M11_Item* items, int count, int* typesOut)
{
    int i;

    if (!items || !typesOut || count < 0) {
        return 0;
    }
    for (i = 0; i < count && i < DM1_PC34_CHEST_CLOSE_REWIRE_TEST_SLOT_COUNT; ++i) {
        typesOut[i] = items[i].itemType;
    }
    return 1;
}

static int count_visible_items(const int* types)
{
    int count = 0;
    int i;

    if (!types) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_CLOSE_REWIRE_TEST_SLOT_COUNT; ++i) {
        if (types[i] != 0) {
            ++count;
        }
    }
    return count;
}

static int contains_type(const int* types, int count, int itemType)
{
    int i;

    if (!types || count < 0) {
        return 0;
    }
    for (i = 0; i < count && i < DM1_PC34_CHEST_CLOSE_REWIRE_TEST_SLOT_COUNT; ++i) {
        if (types[i] == itemType) {
            return 1;
        }
    }
    return 0;
}

const char* dm1_v1_chest_close_rewire_runtime_source_evidence_pc34(void)
{
    return
        "CHEST.C:53-67 F0333 copies the first visible chest links into G0425 slots\n"
        "CHEST.C:117-132 F0334 rewrites the linked list from visible G0425 slots only\n"
        "CHAMPION.C:263-265 F0297 and 582-615 F0300/F0301 adjust Load through F0140\n"
        "CHAMPION.C:688-710 F0302 validates slot masks before swapping leader hand and slot\n"
        "DUNGEON.C:1114-1120 F0140 gives containers base weight 50 plus linked contents\n"
        "DUNGEON.C:108 gives Staff Of Claws AllowedSlots 0x0040";
}

int m11_inventory_pc34_probe_chest_close_rewire_runtime(
    DM1_V1_ChestCloseRewireRuntimeProbePc34* out)
{
    enum {
        CHEST_THING = 0x5A5A,
        REOPENED_CHEST_THING = 0x5A5B,
        BASE_BACKPACK_ITEM = 501,
        FIRST_VISIBLE_ITEM = 800,
        REPLACEMENT_ITEM = 900,
        C538_INDEX = 1,
        C544_INDEX = 7
    };
    M11_InventoryState state;
    M11_Item linked[9];
    M11_Item closed[DM1_PC34_CHEST_CLOSE_REWIRE_TEST_SLOT_COUNT];
    M11_Item item;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    memset(closed, 0, sizeof(closed));

    m11_inventory_init(&state, 1);
    for (i = 0; i < 9; ++i) {
        linked[i] = make_item(FIRST_VISIBLE_ITEM + i, 2 + i,
                              DM1_PC34_ALLOWED_CONTAINER);
    }
    out->hiddenTailInput = linked[8].itemType;

    out->baseLoadSetResult = m11_inventory_set_item_in_pc34_source_slot(
        &state, 0, DM1_PC34_SLOT_BACKPACK_LINE1_1, BASE_BACKPACK_ITEM, 13, 0,
        DM1_PC34_ALLOWED_ANY_SLOT);
    if (!out->baseLoadSetResult) {
        return 0;
    }
    out->baseLoad = m11_inventory_get_load(&state, 0);

    /* ReDMCSB CHEST.C F0333 lines 53-67 materializes the first eight visible
     * linked objects into G0425; the ninth input item remains a hidden tail. */
    out->openResult = m11_inventory_open_chest(&state, 0, CHEST_THING, linked, 9);
    if (!out->openResult ||
        !copy_visible_chest_types(&state, 0, out->visibleBeforeRejectTypes)) {
        return 0;
    }
    out->openVisibleWeight =
        m11_inventory_pc34_open_chest_visible_contents_weight(&state, 0);
    out->openContainerWeight =
        m11_inventory_pc34_open_chest_container_weight(&state, 0);
    out->loadAfterOpen = m11_inventory_get_load(&state, 0);

    out->incompatibleSlotMask = m11_inventory_pc34_slot_mask(DM1_PC34_SLOT_CHEST_2);
    item = make_item(DM1_PC34_CHEST_CLOSE_REWIRE_TEST_STAFF_OF_CLAWS_OBJECT_INFO,
                     1, DM1_PC34_ALLOWED_QUIVER_LINE1);
    out->incompatibleStaffCanEquip =
        m11_inventory_can_equip(&item, DM1_PC34_SLOT_CHEST_2);
    out->incompatibleLeaderHandBefore = item.itemType;
    if (!m11_inventory_set_mouse_item(&state, 0, item.itemType, item.weight,
                                      item.charges, item.allowedSlots) ||
        !m11_inventory_get_item_in_chest_slot(&state, 0, C538_INDEX, &item)) {
        return 0;
    }
    out->incompatibleSlotBefore = item.itemType;

    /* ReDMCSB CHAMPION.C F0302 lines 688-699 reads the leader hand and C30+
     * G0425 slot, then rejects incompatible AllowedSlots before any mutation. */
    out->incompatibleClickResult = m11_inventory_click_pc34_source_slot(
        &state, 0, DM1_PC34_SLOT_CHEST_2);
    if (!m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    out->incompatibleLeaderHandAfter = item.itemType;
    if (!m11_inventory_get_item_in_chest_slot(&state, 0, C538_INDEX, &item)) {
        return 0;
    }
    out->incompatibleSlotAfter = item.itemType;
    if (!copy_visible_chest_types(&state, 0, out->visibleAfterRejectTypes)) {
        return 0;
    }
    out->loadAfterIncompatibleAttempt = m11_inventory_get_load(&state, 0);

    item = make_item(REPLACEMENT_ITEM, 17, DM1_PC34_ALLOWED_CONTAINER);
    out->replacementCanEquip =
        m11_inventory_can_equip(&item, DM1_PC34_SLOT_CHEST_8);
    if (!m11_inventory_set_mouse_item(&state, 0, item.itemType, item.weight,
                                      item.charges, item.allowedSlots)) {
        return 0;
    }

    /* ReDMCSB CHAMPION.C F0302 lines 700-710 performs the accepted C544 swap,
     * moving the previous final visible item to leader hand and writing the
     * replacement into G0425 before F0334 closes the chest. */
    out->replacementClickResult = m11_inventory_click_pc34_source_slot(
        &state, 0, DM1_PC34_SLOT_CHEST_8);
    if (!m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    out->leaderHandAfterReplacement = item.itemType;
    if (!m11_inventory_get_item_in_chest_slot(&state, 0, C544_INDEX, &item)) {
        return 0;
    }
    out->finalSlotAfterReplacement = item.itemType;
    out->replacementVisibleWeight =
        m11_inventory_pc34_open_chest_visible_contents_weight(&state, 0);
    out->replacementContainerWeight =
        m11_inventory_pc34_open_chest_container_weight(&state, 0);
    out->loadAfterReplacement = m11_inventory_get_load(&state, 0);

    /* ReDMCSB CHEST.C F0334 lines 117-132 rewrites only the eight visible
     * G0425 slots and clears them; DUNGEON.C F0140 lines 1114-1120 container
     * weight is sampled before that transient state is reset. */
    out->closeCount = m11_inventory_pc34_close_chest_with_weight_snapshot(
        &state, 0, closed, DM1_PC34_CHEST_CLOSE_REWIRE_TEST_SLOT_COUNT,
        &out->closeContainerWeightSnapshot);
    if (out->closeCount < 0 ||
        !copy_item_types(closed, DM1_PC34_CHEST_CLOSE_REWIRE_TEST_SLOT_COUNT,
                         out->closedTypes)) {
        return 0;
    }
    out->hiddenTailClosed = contains_type(out->closedTypes, out->closeCount,
                                          out->hiddenTailInput);
    out->closeContainerWeightAfter =
        m11_inventory_pc34_open_chest_container_weight(&state, 0);
    out->loadAfterClose = m11_inventory_get_load(&state, 0);

    /* ReDMCSB CHEST.C F0333 lines 53-67 reopens from the F0334-produced link
     * order; the excluded hidden tail is not part of the visible G0425 copy. */
    out->reopenResult = m11_inventory_open_chest(
        &state, 0, REOPENED_CHEST_THING, closed, out->closeCount);
    if (!out->reopenResult ||
        !copy_visible_chest_types(&state, 0, out->reopenedTypes)) {
        return 0;
    }
    out->reopenVisibleCount = count_visible_items(out->reopenedTypes);
    out->hiddenTailReopened = contains_type(out->reopenedTypes,
                                            out->reopenVisibleCount,
                                            out->hiddenTailInput);
    out->reopenVisibleWeight =
        m11_inventory_pc34_open_chest_visible_contents_weight(&state, 0);
    out->reopenContainerWeight =
        m11_inventory_pc34_open_chest_container_weight(&state, 0);
    out->loadAfterReopen = m11_inventory_get_load(&state, 0);

    return 1;
}
