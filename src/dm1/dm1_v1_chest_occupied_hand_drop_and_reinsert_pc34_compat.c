#include "dm1/dm1_v1_chest_occupied_hand_drop_and_reinsert_pc34_compat.h"

#include <string.h>

static const char s_f0333_anchor[] =
    "CHEST.C:F0333_INVENTORY_OpenAndDrawChest:31-67 opens the requested "
    "G0426 chest, draws C145_ICON_CONTAINER_CHEST_OPEN at C09, and copies "
    "visible links into G0425_aT_ChestSlots.";

static const char s_f0334_anchor[] =
    "CHEST.C:F0334_INVENTORY_CloseChest:113-132 closes G0426, writes "
    "Container->Slot to C0xFFFE_THING_ENDOFLIST, skips C0xFFFF empty slots, "
    "and relinks only non-empty G0425 entries.";

static const char s_f0300_anchor[] =
    "CHAMPION.C:F0300_CHAMPION_GetObjectRemovedFromSlot:511-514 removes "
    "C30+ chest slot things from G0425_aT_ChestSlots and writes NONE.";

static const char s_f0301_anchor[] =
    "CHAMPION.C:F0301_CHAMPION_AddObjectInSlot:606-615 writes accepted "
    "C30+ slot things back into G0425_aT_ChestSlots and updates Load.";

static const char s_f0302_anchor[] =
    "CHAMPION.C:F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox:688-710 "
    "reads leader hand and selected slot, rejects empty/empty and incompatible "
    "AllowedSlots, then swaps via F0298/F0300/F0297/F0301.";

static const char s_data_slot_mask_anchor[] =
    "DATA.C:G0038_ai_Graphic562_SlotMasks:1080-1087 gives C30..C37 chest "
    "slots MASK0x0400_CONTAINER.";

static const char s_object_info_anchor[] =
    "DUNGEON.C:G0237_as_Graphic559_ObjectInfo:80-83 gives container/chest "
    "object AllowedSlots as Hands, not chest-container slots.";

static const char s_f0140_anchor[] =
    "DUNGEON.C:F0140_DUNGEON_GetObjectWeight:1114-1120 gives containers "
    "base weight plus linked contents for load/encumbrance accounting.";

static const char s_unavailable_destroy_anchor[] =
    "CHEST.C:F0335_INVENTORY_DestroyChest:unavailable-in-local-WIP; "
    "ReDMCSB_WIP20210206 CHEST.C defines F0333/F0334 only, and PANEL.C "
    "F0335 is DrawPanel_ObjectDescriptionString.";

static const char s_source_summary[] =
    "contract_only=1; source-locked no-real-asset runtime gate for empty "
    "chest-slot hand drop, occupied chest-slot pickup, close/reopen sentinel "
    "rewrite, allowed-slot rejection, and weight rejection.";

const DM1_V1_ChestOccupiedHandDropAndReinsertSpecPc34
    dm1_v1_chest_occupied_hand_drop_and_reinsert_pc34_spec = {
        1,
        DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT3_INDEX,
        DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT5_INDEX,
        DM1_PC34_CHEST_OCCUPIED_HAND_DROP_COMPACTED_HAND_INDEX,
        DM1_PC34_ICON_CONTAINER_CHEST_OPEN_PC34,
        DM1_PC34_CHEST_OCCUPIED_HAND_DROP_MAX_ACCEPTED_WEIGHT,
        { DM1_PC34_CHEST_OCCUPIED_HAND_DROP_LEADER_HAND_ITEM,
          11,
          DM1_PC34_ALLOWED_CONTAINER },
        { DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT3_ITEM,
          7,
          DM1_PC34_ALLOWED_CONTAINER },
        { DM1_PC34_CHEST_OCCUPIED_HAND_DROP_CHEST_ONLY_CONTAINER,
          50,
          DM1_PC34_ALLOWED_HANDS },
        { DM1_PC34_CHEST_OCCUPIED_HAND_DROP_TOO_HEAVY_ITEM,
          DM1_PC34_CHEST_OCCUPIED_HAND_DROP_TOO_HEAVY_WEIGHT,
          DM1_PC34_ALLOWED_CONTAINER },
        s_f0333_anchor,
        s_f0334_anchor,
        s_f0300_anchor,
        s_f0301_anchor,
        s_f0302_anchor,
        s_data_slot_mask_anchor,
        s_object_info_anchor,
        s_f0140_anchor,
        s_unavailable_destroy_anchor,
        s_source_summary
    };

static M11_Item make_item(int itemType, int weight, int allowedSlots)
{
    M11_Item item;

    memset(&item, 0, sizeof(item));
    item.itemType = itemType;
    item.weight = weight;
    item.identified = 1;
    item.allowedSlots = allowedSlots;
    return item;
}

static void make_sparse_chest(M11_Item* linked)
{
    int i;

    for (i = 0; i < 6; ++i) {
        memset(&linked[i], 0, sizeof(linked[i]));
    }
    linked[0] = make_item(DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT0_ITEM,
                          2, DM1_PC34_ALLOWED_CONTAINER);
    linked[1] = make_item(DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT1_ITEM,
                          3, DM1_PC34_ALLOWED_CONTAINER);
    linked[2] = make_item(DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT2_ITEM,
                          5, DM1_PC34_ALLOWED_CONTAINER);
    linked[3] = make_item(DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT3_ITEM,
                          7, DM1_PC34_ALLOWED_CONTAINER);
    linked[4] = make_item(DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT4_ITEM,
                          11, DM1_PC34_ALLOWED_CONTAINER);
}

static int copy_open_types(const M11_InventoryState* state, int* outTypes)
{
    int i;

    if (!state || !outTypes) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT_COUNT; ++i) {
        M11_Item item;

        if (!m11_inventory_get_item_in_chest_slot(state, 0, i, &item)) {
            return 0;
        }
        outTypes[i] = item.itemType;
    }
    return 1;
}

static void copy_closed_types(const M11_Item* closed, int count, int* outTypes)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT_COUNT; ++i) {
        outTypes[i] =
            (closed && i < count) ? closed[i].itemType : 0;
    }
}

static int contains_type(const int* types, int count, int itemType)
{
    int i;

    if (!types || count < 0) {
        return 0;
    }
    for (i = 0;
         i < count && i < DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT_COUNT;
         ++i) {
        if (types[i] == itemType) {
            return 1;
        }
    }
    return 0;
}

static int find_type_index(const int* types, int count, int itemType)
{
    int i;

    if (!types || count < 0) {
        return -1;
    }
    for (i = 0;
         i < count && i < DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT_COUNT;
         ++i) {
        if (types[i] == itemType) {
            return i;
        }
    }
    return -1;
}

static int click_chest_slot_with_weight_gate(M11_InventoryState* state,
                                             int slotIndex,
                                             int maxAcceptedWeight,
                                             int* outRejectedByWeight)
{
    M11_Item hand;

    if (outRejectedByWeight) {
        *outRejectedByWeight = 0;
    }
    if (!state ||
        !m11_inventory_get_mouse_item(state, 0, &hand) ||
        slotIndex < 0 ||
        slotIndex >= DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT_COUNT) {
        return 0;
    }
    if (hand.itemType != 0 && hand.weight > maxAcceptedWeight) {
        if (outRejectedByWeight) {
            *outRejectedByWeight = 1;
        }
        return 0;
    }
    return m11_inventory_click_pc34_source_slot(
        state, 0, DM1_PC34_SLOT_CHEST_1 + slotIndex);
}

static int run_runtime_case(
    DM1_V1_ChestOccupiedHandDropRuntimePc34* out)
{
    M11_InventoryState state;
    M11_Item linked[6];
    M11_Item closed[DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT_COUNT];
    M11_Item item;
    int compactedIndex;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    memset(closed, 0, sizeof(closed));
    make_sparse_chest(linked);

    m11_inventory_init(&state, 1);

    /* ReDMCSB CHEST.C:F0333:31-67 copies the linked chest view into G0425. */
    out->openResult = m11_inventory_open_chest(
        &state, 0, DM1_PC34_CHEST_OCCUPIED_HAND_DROP_CHEST_THING,
        linked, 6);
    out->openThing = m11_inventory_get_open_chest_thing(&state, 0);
    if (!out->openResult || !copy_open_types(&state, out->openedTypes)) {
        return 0;
    }
    out->slot3BeforeDrop =
        out->openedTypes[DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT3_INDEX];
    out->slot5BeforeDrop =
        out->openedTypes[DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT5_INDEX];

    if (!m11_inventory_set_mouse_item(
            &state, 0, DM1_PC34_CHEST_OCCUPIED_HAND_DROP_LEADER_HAND_ITEM,
            11, 0, DM1_PC34_ALLOWED_CONTAINER) ||
        !m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    out->leaderHandBeforeDrop = item.itemType;
    out->leaderHandCanEnterSlot5 =
        m11_inventory_can_equip(
            &item,
            DM1_PC34_SLOT_CHEST_1 +
                DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT5_INDEX);

    /* ReDMCSB CHAMPION.C:F0302:688-710 accepts a full leader hand into an
     * empty C542/G0425 slot, leaving the leader hand empty. */
    out->emptySlot5ClickResult = m11_inventory_click_pc34_source_slot(
        &state, 0,
        DM1_PC34_SLOT_CHEST_1 +
            DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT5_INDEX);
    if (!out->emptySlot5ClickResult ||
        !m11_inventory_get_item_in_chest_slot(
            &state, 0, DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT5_INDEX,
            &item)) {
        return 0;
    }
    out->slot5AfterHandDrop = item.itemType;
    if (!m11_inventory_get_mouse_item(&state, 0, &item) ||
        !m11_inventory_get_item_in_chest_slot(
            &state, 0, DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT3_INDEX,
            &linked[0])) {
        return 0;
    }
    out->leaderHandAfterHandDrop = item.itemType;
    out->slot3AfterHandDrop = linked[0].itemType;

    /* ReDMCSB CHAMPION.C:F0300:511-514 and F0302:704-706 move the occupied
     * C540/G0425 entry into the leader hand and clear the visible slot. */
    out->occupiedSlot3ClickResult = m11_inventory_click_pc34_source_slot(
        &state, 0,
        DM1_PC34_SLOT_CHEST_1 +
            DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT3_INDEX);
    if (!out->occupiedSlot3ClickResult ||
        !m11_inventory_get_item_in_chest_slot(
            &state, 0, DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT3_INDEX,
            &item)) {
        return 0;
    }
    out->slot3AfterPickup = item.itemType;
    if (!m11_inventory_get_item_in_chest_slot(
            &state, 0, DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT5_INDEX,
            &item)) {
        return 0;
    }
    out->slot5AfterPickup = item.itemType;
    if (!m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    out->leaderHandAfterSlot3Pickup = item.itemType;

    /* ReDMCSB CHEST.C:F0334:113-132 rewrites only non-empty visible slots.
     * The former hand item survives the close even though the emptied C540
     * slot is skipped by the sentinel rewrite. */
    out->closeCount = m11_inventory_close_chest(
        &state, 0, closed,
        DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT_COUNT);
    if (out->closeCount < 0) {
        return 0;
    }
    copy_closed_types(closed, out->closeCount, out->closedTypes);
    out->closedContainsFormerHand =
        contains_type(out->closedTypes, out->closeCount,
                      DM1_PC34_CHEST_OCCUPIED_HAND_DROP_LEADER_HAND_ITEM);
    out->closedContainsOriginalSlot3 =
        contains_type(out->closedTypes, out->closeCount,
                      DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT3_ITEM);
    out->closedSkipsEmptySlot3 =
        out->closedContainsOriginalSlot3 ? 0 : 1;
    out->closeClearsOpenChest =
        m11_inventory_get_open_chest_thing(&state, 0) == 0 ? 1 : 0;

    /* ReDMCSB CHEST.C:F0333:53-67 rematerializes the F0334-compacted links. */
    out->reopenResult = m11_inventory_open_chest(
        &state, 0, DM1_PC34_CHEST_OCCUPIED_HAND_DROP_REOPEN_THING,
        closed, out->closeCount);
    out->reopenThing = m11_inventory_get_open_chest_thing(&state, 0);
    if (!out->reopenResult || !copy_open_types(&state, out->reopenedTypes) ||
        !m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    out->leaderHandAfterReopen = item.itemType;
    out->reopenedFormerHandIndex =
        find_type_index(out->reopenedTypes,
                        DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT_COUNT,
                        DM1_PC34_CHEST_OCCUPIED_HAND_DROP_LEADER_HAND_ITEM);
    out->reopenedContainsFormerHand =
        out->reopenedFormerHandIndex >= 0 ? 1 : 0;
    out->reopenedContainsOriginalSlot3 =
        contains_type(out->reopenedTypes,
                      DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT_COUNT,
                      DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT3_ITEM);
    out->originalSlot5EmptyAfterCompaction =
        out->reopenedTypes[DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT5_INDEX] == 0 ?
        1 : 0;

    compactedIndex = out->reopenedFormerHandIndex;
    if (compactedIndex < 0) {
        return 0;
    }

    /* ReDMCSB CHAMPION.C:F0302:700-710 swaps the occupied compacted former
     * hand entry with the leader-hand object that originally came from C540. */
    out->postReopenSwapClickResult = m11_inventory_click_pc34_source_slot(
        &state, 0, DM1_PC34_SLOT_CHEST_1 + compactedIndex);
    if (!out->postReopenSwapClickResult ||
        !m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    out->leaderHandAfterPostReopenSwap = item.itemType;
    if (!m11_inventory_get_item_in_chest_slot(
            &state, 0, compactedIndex, &item)) {
        return 0;
    }
    out->compactedSlotAfterPostReopenSwap = item.itemType;
    if (!m11_inventory_get_item_in_chest_slot(
            &state, 0, DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT3_INDEX,
            &item)) {
        return 0;
    }
    out->originalSlot3UnaffectedByPostReopenSwap =
        item.itemType == DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT4_ITEM ? 1 : 0;

    /* ReDMCSB CHAMPION.C:F0302:700-710 is symmetric: a second click on the
     * same occupied compacted slot returns the former hand item to the chest
     * entry and the original C540 item to the leader hand. */
    out->secondCycleClickResult = m11_inventory_click_pc34_source_slot(
        &state, 0, DM1_PC34_SLOT_CHEST_1 + compactedIndex);
    if (!out->secondCycleClickResult ||
        !m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    out->leaderHandAfterSecondCycle = item.itemType;
    if (!m11_inventory_get_item_in_chest_slot(
            &state, 0, compactedIndex, &item)) {
        return 0;
    }
    out->compactedSlotAfterSecondCycle = item.itemType;
    if (!m11_inventory_get_item_in_chest_slot(
            &state, 0, DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT3_INDEX,
            &item)) {
        return 0;
    }
    out->originalSlot3UnaffectedBySecondCycle =
        item.itemType == DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT4_ITEM ? 1 : 0;

    return 1;
}

static int run_allowed_reject_case(
    DM1_V1_ChestOccupiedHandDropAllowedSlotsRejectPc34* out)
{
    M11_InventoryState state;
    M11_Item linked[6];
    M11_Item item;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    make_sparse_chest(linked);
    m11_inventory_init(&state, 1);
    if (!m11_inventory_open_chest(
            &state, 0, DM1_PC34_CHEST_OCCUPIED_HAND_DROP_CHEST_THING,
            linked, 6) ||
        !m11_inventory_set_mouse_item(
            &state, 0,
            DM1_PC34_CHEST_OCCUPIED_HAND_DROP_CHEST_ONLY_CONTAINER,
            50, 0, DM1_PC34_ALLOWED_HANDS) ||
        !m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    out->incompatibleLeaderHandBefore = item.itemType;
    out->incompatibleCanEnterChestSlot =
        m11_inventory_can_equip(
            &item,
            DM1_PC34_SLOT_CHEST_1 +
                DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT5_INDEX);
    if (!m11_inventory_get_item_in_chest_slot(
            &state, 0, DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT5_INDEX,
            &item)) {
        return 0;
    }
    out->incompatibleSlot5Before = item.itemType;
    out->incompatibleClickResult = m11_inventory_click_pc34_source_slot(
        &state, 0,
        DM1_PC34_SLOT_CHEST_1 +
            DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT5_INDEX);
    if (!m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    out->incompatibleLeaderHandAfter = item.itemType;
    if (!m11_inventory_get_item_in_chest_slot(
            &state, 0, DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT5_INDEX,
            &item)) {
        return 0;
    }
    out->incompatibleSlot5After = item.itemType;
    return 1;
}

static int run_weight_reject_case(
    DM1_V1_ChestOccupiedHandDropWeightRejectPc34* out)
{
    M11_InventoryState state;
    M11_Item linked[6];
    M11_Item item;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    make_sparse_chest(linked);
    m11_inventory_init(&state, 1);
    if (!m11_inventory_open_chest(
            &state, 0, DM1_PC34_CHEST_OCCUPIED_HAND_DROP_CHEST_THING,
            linked, 6) ||
        !m11_inventory_set_mouse_item(
            &state, 0, DM1_PC34_CHEST_OCCUPIED_HAND_DROP_TOO_HEAVY_ITEM,
            DM1_PC34_CHEST_OCCUPIED_HAND_DROP_TOO_HEAVY_WEIGHT,
            0, DM1_PC34_ALLOWED_CONTAINER) ||
        !m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    out->heavyLeaderHandBefore = item.itemType;
    out->heavyWeight = item.weight;
    out->maxAcceptedWeight =
        DM1_PC34_CHEST_OCCUPIED_HAND_DROP_MAX_ACCEPTED_WEIGHT;
    out->heavyCanEnterChestSlot =
        m11_inventory_can_equip(
            &item,
            DM1_PC34_SLOT_CHEST_1 +
                DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT5_INDEX);
    if (!m11_inventory_get_item_in_chest_slot(
            &state, 0, DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT5_INDEX,
            &item)) {
        return 0;
    }
    out->heavySlot5Before = item.itemType;
    out->heavyClickResult = click_chest_slot_with_weight_gate(
        &state, DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT5_INDEX,
        DM1_PC34_CHEST_OCCUPIED_HAND_DROP_MAX_ACCEPTED_WEIGHT,
        &out->heavyRejectedByWeightGate);
    if (!m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    out->heavyLeaderHandAfter = item.itemType;
    if (!m11_inventory_get_item_in_chest_slot(
            &state, 0, DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT5_INDEX,
            &item)) {
        return 0;
    }
    out->heavySlot5After = item.itemType;
    return 1;
}

const char*
dm1_v1_chest_occupied_hand_drop_and_reinsert_source_evidence_pc34(void)
{
    return s_source_summary;
}

const DM1_V1_ChestOccupiedHandDropAndReinsertSpecPc34*
dm1_v1_chest_occupied_hand_drop_and_reinsert_spec_pc34(void)
{
    return &dm1_v1_chest_occupied_hand_drop_and_reinsert_pc34_spec;
}

int dm1_v1_chest_occupied_hand_drop_and_reinsert_pc34(
    DM1_V1_ChestOccupiedHandDropAndReinsertProbePc34* out)
{
    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    out->contract_only = 1;
    if (!run_runtime_case(&out->runtime) ||
        !run_allowed_reject_case(&out->allowedReject) ||
        !run_weight_reject_case(&out->weightReject)) {
        return 0;
    }
    return 1;
}
