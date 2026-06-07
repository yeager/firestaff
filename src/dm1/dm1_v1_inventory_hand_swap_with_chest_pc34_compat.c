#include "dm1_v1_inventory_hand_swap_with_chest_pc34_compat.h"

#include "dm1_v1_chest_occupied_slot_swap_pc34_compat.h"
#include "dm1_v1_inventory_chest_incompatible_swap_pc34_compat.h"
#include "dm1_v1_inventory_chest_load_pc34_compat.h"

#include <string.h>

static const char s_source_evidence[] =
    "CHEST.C F0333:31-67 opens G0426, draws C09_SLOT_BOX_INVENTORY_ACTION_HAND/C145_ICON_CONTAINER_CHEST_OPEN at line 45, blits C08/C072/C101 panel anchors at lines 48-51, and materializes C537..C544 into G0425\n"
    "CHEST.C F0334:113-132 closes G0426 by compacting non-empty G0425 chest slots back into the container link list\n"
    "CHAMPION.C F0297:250-298 and F0298:279-298 own leader-hand put/remove state and load changes\n"
    "CHAMPION.C F0300:511-584, F0301:606-660, F0302:677-710 route C30+ chest slots through G0425 and swap leader hand with the selected chest slot\n"
    "DATA.C:320-357 gives C30..C37 chest slots MASK0x0400_CONTAINER; DATA.C:1016-1023 maps C537..C544 panel zones";

const DM1_V1_InventoryHandSwapWithChestSpecPc34
    dm1_v1_inventory_hand_swap_with_chest_pc34_spec = {
        "Source-locked contract gate only; not full real-asset chest runtime parity.",
        9,
        145,
        DM1_PC34_SLOT_CHEST_1,
        DM1_PC34_SLOT_CHEST_8,
        30,
        37,
        DM1_PC34_ALLOWED_CONTAINER,
        0,
        1
    };

static M11_Item make_item(int itemType, int weight, int charges,
                          int allowedSlots)
{
    M11_Item item;

    memset(&item, 0, sizeof(item));
    item.itemType = itemType;
    item.weight = weight;
    item.charges = charges;
    item.identified = 1;
    item.allowedSlots = allowedSlots;
    return item;
}

static void copy_item_types(const M11_Item* items, int count, int* typesOut)
{
    int i;

    for (i = 0; i < DM1_PC34_HAND_SWAP_WITH_CHEST_SLOT_COUNT; ++i) {
        typesOut[i] =
            (items && i < count && items[i].itemType != 0) ?
            items[i].itemType : 0;
    }
}

static int count_type3(int a, int b, int c, int itemType)
{
    int count = 0;

    if (a == itemType) {
        ++count;
    }
    if (b == itemType) {
        ++count;
    }
    if (c == itemType) {
        ++count;
    }
    return count;
}

const char*
dm1_v1_inventory_hand_swap_with_chest_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const DM1_V1_InventoryHandSwapWithChestSpecPc34*
dm1_v1_inventory_hand_swap_with_chest_spec_pc34(void)
{
    return &dm1_v1_inventory_hand_swap_with_chest_pc34_spec;
}

int M11_V1_Inventory_HandSwapWithChest_ApplyPc34(
    const DM1_V1_InventoryHandSwapWithChestHandPc34* hand,
    const DM1_V1_InventoryHandSwapWithChestSlotPc34* chestSlot,
    DM1_V1_InventoryHandSwapWithChestResultPc34* out)
{
    M11_InventoryState* state;
    int championIndex;
    int pc34Slot;

    if (!hand || !chestSlot || !out || !hand->inventory) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    state = hand->inventory;
    championIndex = hand->championIndex;
    if (championIndex < 0 || championIndex >= state->championCount ||
        hand->actionHandIndex !=
            dm1_v1_inventory_hand_swap_with_chest_pc34_spec.actionHandIndex ||
        chestSlot->chestSlotIndex < 0 ||
        chestSlot->chestSlotIndex >= DM1_PC34_CHEST_SLOT_COUNT) {
        return 0;
    }

    out->openChestThingBefore =
        m11_inventory_get_open_chest_thing(state, championIndex);
    if (out->openChestThingBefore == 0 ||
        out->openChestThingBefore != hand->expectedOpenChestThing) {
        return 0;
    }
    pc34Slot = DM1_PC34_SLOT_CHEST_1 + chestSlot->chestSlotIndex;
    out->pc34ChestSlot = pc34Slot;

    if (!m11_inventory_get_mouse_item(state, championIndex, &out->handBefore) ||
        !m11_inventory_get_item_in_chest_slot(
            state, championIndex, chestSlot->chestSlotIndex,
            &out->originalChestOccupant)) {
        return 0;
    }

    out->visibleWeightBefore =
        m11_inventory_pc34_open_chest_visible_contents_weight(
            state, championIndex);

    if (out->handBefore.itemType != 0 &&
        !m11_inventory_can_equip(&out->handBefore, pc34Slot)) {
        out->incompatibleRejected = 1;
        out->visibleWeightAfter = out->visibleWeightBefore;
        out->handAfter = out->handBefore;
        out->chestSlotAfter = out->originalChestOccupant;
        return 1;
    }

    /* ReDMCSB CHAMPION.C F0302 lines 688-710 reads the leader hand and C30+
     * G0425 slot, rejects incompatible masks, then delegates the real swap to
     * F0298/F0300/F0297/F0301; CHEST.C F0333 lines 31-67 supplies G0425. */
    out->accepted = m11_inventory_click_open_chest_slot_for_thing(
        state, championIndex, hand->expectedOpenChestThing,
        chestSlot->chestSlotIndex);
    if (!m11_inventory_get_mouse_item(state, championIndex, &out->handAfter) ||
        !m11_inventory_get_item_in_chest_slot(
            state, championIndex, chestSlot->chestSlotIndex,
            &out->chestSlotAfter)) {
        return 0;
    }
    out->visibleWeightAfter =
        m11_inventory_pc34_open_chest_visible_contents_weight(
            state, championIndex);
    out->originalChestOccupantReturnedToHand =
        out->accepted &&
        out->handAfter.itemType == out->originalChestOccupant.itemType;
    out->previousHandStoredInChestSlot =
        out->accepted &&
        out->handBefore.itemType != 0 &&
        out->chestSlotAfter.itemType == out->handBefore.itemType;
    out->chestSlotCleared =
        out->accepted &&
        out->handBefore.itemType == 0 &&
        out->chestSlotAfter.itemType == 0;
    return 1;
}

int M11_V1_Inventory_HandSwapWithChest_RunPc34(
    DM1_V1_InventoryHandSwapWithChestProbePc34* out)
{
    M11_InventoryState state;
    M11_Item mainInput[DM1_PC34_HAND_SWAP_WITH_CHEST_SLOT_COUNT];
    M11_Item otherInput[DM1_PC34_HAND_SWAP_WITH_CHEST_SLOT_COUNT];
    M11_Item closed[DM1_PC34_HAND_SWAP_WITH_CHEST_SLOT_COUNT];
    int closedTypes[DM1_PC34_HAND_SWAP_WITH_CHEST_SLOT_COUNT];
    DM1_V1_InventoryHandSwapWithChestHandPc34 hand;
    DM1_V1_InventoryHandSwapWithChestSlotPc34 slot;
    DM1_V1_InventoryHandSwapWithChestResultPc34 result;
    M11_Item item;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    memset(closed, 0, sizeof(closed));
    memset(closedTypes, 0, sizeof(closedTypes));
    out->sourceLockedContractOnly =
        dm1_v1_chest_occupied_slot_swap_spec_pc34() != NULL &&
        dm1_inventory_chest_incompatible_swap_source_evidence_pc34() != NULL &&
        dm1_inventory_chest_load_source_evidence_pc34() != NULL ? 1 : 0;
    out->stackingNotApplicable =
        dm1_v1_inventory_hand_swap_with_chest_pc34_spec.stackingNotApplicable;

    m11_inventory_init(&state, 3);
    for (i = 0; i < DM1_PC34_HAND_SWAP_WITH_CHEST_SLOT_COUNT; ++i) {
        mainInput[i] =
            make_item(DM1_PC34_HAND_SWAP_WITH_CHEST_FIRST_ITEM + i,
                      2 + i, 0, DM1_PC34_ALLOWED_CONTAINER);
        otherInput[i] =
            make_item(DM1_PC34_HAND_SWAP_WITH_CHEST_OTHER_FIRST_ITEM + i,
                      3 + i, DM1_PC34_HAND_SWAP_WITH_CHEST_STACK_CHARGES,
                      DM1_PC34_ALLOWED_CONTAINER);
    }

    /* ReDMCSB CHEST.C F0333 lines 31-67 opens the action-hand chest and
     * copies C537..C544; DATA.C lines 1016-1023 defines those panel zones. */
    out->mainOpenResult = m11_inventory_open_chest(
        &state, 0, DM1_PC34_HAND_SWAP_WITH_CHEST_MAIN_THING,
        mainInput, DM1_PC34_HAND_SWAP_WITH_CHEST_SLOT_COUNT);
    out->mainOpenThing = m11_inventory_get_open_chest_thing(&state, 0);
    hand.inventory = &state;
    hand.championIndex = 0;
    hand.actionHandIndex = 0;
    hand.expectedOpenChestThing = DM1_PC34_HAND_SWAP_WITH_CHEST_MAIN_THING;

    /* ReDMCSB CHAMPION.C F0302 lines 694-710 allows empty leader-hand pickup
     * from a C30+ chest slot and leaves the selected G0425 entry empty. */
    slot.chestSlotIndex = DM1_PC34_HAND_SWAP_WITH_CHEST_EMPTY_INDEX;
    if (!M11_V1_Inventory_HandSwapWithChest_ApplyPc34(
            &hand, &slot, &result)) {
        return 0;
    }
    out->emptySwapResult = result.accepted;
    out->emptyHandBefore = result.handBefore.itemType;
    out->emptyHandAfter = result.handAfter.itemType;
    out->emptyOriginalChestOccupant = result.originalChestOccupant.itemType;
    out->emptyChestSlotAfter = result.chestSlotAfter.itemType;
    out->emptyClosedCount = m11_inventory_close_chest(
        &state, 0, closed, DM1_PC34_HAND_SWAP_WITH_CHEST_SLOT_COUNT);
    copy_item_types(closed, out->emptyClosedCount, closedTypes);
    out->emptyReopenResult = m11_inventory_open_chest(
        &state, 0, DM1_PC34_HAND_SWAP_WITH_CHEST_MAIN_THING + 1,
        closed, out->emptyClosedCount);
    if (!m11_inventory_get_item_in_chest_slot(
            &state, 0, DM1_PC34_HAND_SWAP_WITH_CHEST_EMPTY_INDEX,
            &item)) {
        return 0;
    }
    out->emptyReopenedSlotContainsNextVisible =
        item.itemType == DM1_PC34_HAND_SWAP_WITH_CHEST_FIRST_ITEM +
            DM1_PC34_HAND_SWAP_WITH_CHEST_EMPTY_INDEX + 1 ? 1 : 0;
    if (!m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    out->emptyReopenedOriginalStillInHand =
        item.itemType == out->emptyOriginalChestOccupant ? 1 : 0;

    /* ReDMCSB CHAMPION.C F0302 lines 688-710 swaps a full leader hand with
     * occupied C541/G0425, preserving charges through F0301 lines 606-660. */
    out->fullHandSetupResult = m11_inventory_set_mouse_item(
        &state, 0, DM1_PC34_HAND_SWAP_WITH_CHEST_FULL_HAND_ITEM,
        17, DM1_PC34_HAND_SWAP_WITH_CHEST_STACK_CHARGES,
        DM1_PC34_ALLOWED_CONTAINER);
    hand.expectedOpenChestThing = DM1_PC34_HAND_SWAP_WITH_CHEST_MAIN_THING + 1;
    slot.chestSlotIndex = DM1_PC34_HAND_SWAP_WITH_CHEST_FULL_INDEX;
    if (!out->fullHandSetupResult ||
        !M11_V1_Inventory_HandSwapWithChest_ApplyPc34(
            &hand, &slot, &result)) {
        return 0;
    }
    out->fullSwapResult = result.accepted;
    out->fullHandBefore = result.handBefore.itemType;
    out->fullOriginalChestOccupant = result.originalChestOccupant.itemType;
    out->fullHandAfter = result.handAfter.itemType;
    out->fullChestSlotAfter = result.chestSlotAfter.itemType;
    out->fullOriginalReturnedToHand =
        result.originalChestOccupantReturnedToHand;
    out->fullPreviousHandStoredInSlot =
        result.previousHandStoredInChestSlot;
    out->chargesPreserved =
        result.chestSlotAfter.charges ==
        DM1_PC34_HAND_SWAP_WITH_CHEST_STACK_CHARGES ? 1 : 0;
    memset(closed, 0, sizeof(closed));
    out->fullClosedCount = m11_inventory_close_chest(
        &state, 0, closed, DM1_PC34_HAND_SWAP_WITH_CHEST_SLOT_COUNT);
    out->fullReopenResult = m11_inventory_open_chest(
        &state, 0, DM1_PC34_HAND_SWAP_WITH_CHEST_MAIN_THING + 2,
        closed, out->fullClosedCount);
    if (!m11_inventory_get_item_in_chest_slot(
            &state, 0, DM1_PC34_HAND_SWAP_WITH_CHEST_FULL_INDEX,
            &item)) {
        return 0;
    }
    out->fullReopenedSlotContainsPreviousHand =
        item.itemType == DM1_PC34_HAND_SWAP_WITH_CHEST_FULL_HAND_ITEM ? 1 : 0;

    /* ReDMCSB CHAMPION.C F0302 lines 688-710 applies the same C541 swap in
     * reverse, restoring the original occupant before CHEST.C F0334 closes. */
    hand.expectedOpenChestThing = DM1_PC34_HAND_SWAP_WITH_CHEST_MAIN_THING + 2;
    if (!M11_V1_Inventory_HandSwapWithChest_ApplyPc34(
            &hand, &slot, &result)) {
        return 0;
    }
    out->restoreSwapResult = result.accepted;
    out->restoreReturnedPreviousHand =
        result.handAfter.itemType ==
        DM1_PC34_HAND_SWAP_WITH_CHEST_FULL_HAND_ITEM ? 1 : 0;
    out->restoreSlotOriginalOccupant =
        result.chestSlotAfter.itemType == out->fullOriginalChestOccupant ?
        1 : 0;
    memset(closed, 0, sizeof(closed));
    out->restoreClosedCount = m11_inventory_close_chest(
        &state, 0, closed, DM1_PC34_HAND_SWAP_WITH_CHEST_SLOT_COUNT);
    out->restoreReopenResult = m11_inventory_open_chest(
        &state, 0, DM1_PC34_HAND_SWAP_WITH_CHEST_MAIN_THING + 3,
        closed, out->restoreClosedCount);
    if (!m11_inventory_get_item_in_chest_slot(
            &state, 0, DM1_PC34_HAND_SWAP_WITH_CHEST_FULL_INDEX,
            &item)) {
        return 0;
    }
    out->restoreReopenedSlotOriginalOccupant =
        item.itemType == out->fullOriginalChestOccupant ? 1 : 0;

    /* ReDMCSB CHAMPION.C F0302 lines 697-699 rejects non-container masks for
     * C30+ chest slots before F0298/F0300/F0301 mutate hand or G0425 state. */
    hand.expectedOpenChestThing = DM1_PC34_HAND_SWAP_WITH_CHEST_MAIN_THING + 3;
    if (!m11_inventory_set_mouse_item(
            &state, 0, DM1_PC34_TEST_STAFF_OF_CLAWS_OBJECT_INFO,
            1, 0, DM1_PC34_ALLOWED_QUIVER_LINE1) ||
        !M11_V1_Inventory_HandSwapWithChest_ApplyPc34(
            &hand, &slot, &result)) {
        return 0;
    }
    out->incompatibleSwapResult = result.accepted;
    out->incompatibleRejected = result.incompatibleRejected;
    out->incompatibleHandAfter = result.handAfter.itemType;
    out->incompatibleSlotAfter = result.chestSlotAfter.itemType;

    /* ReDMCSB CHAMPION.C F0302 lines 677-686 routes the clicked slot box to
     * the requested champion; CHEST.C F0333 lines 31-67 keeps each synthetic
     * champion inventory's open G0425 window independent in this gate. */
    out->otherChampionOpenResult = m11_inventory_open_chest(
        &state, 1, DM1_PC34_HAND_SWAP_WITH_CHEST_OTHER_THING,
        otherInput, DM1_PC34_HAND_SWAP_WITH_CHEST_SLOT_COUNT);
    if (!m11_inventory_set_mouse_item(
            &state, 1, DM1_PC34_HAND_SWAP_WITH_CHEST_OTHER_HAND_ITEM,
            23, 0, DM1_PC34_ALLOWED_CONTAINER)) {
        return 0;
    }
    hand.championIndex = 1;
    hand.expectedOpenChestThing = DM1_PC34_HAND_SWAP_WITH_CHEST_OTHER_THING;
    slot.chestSlotIndex = 0;
    if (!M11_V1_Inventory_HandSwapWithChest_ApplyPc34(
            &hand, &slot, &result)) {
        return 0;
    }
    out->otherChampionSwapResult = result.accepted;
    out->otherChampionHandAfter = result.handAfter.itemType;
    out->otherChampionSlotAfter = result.chestSlotAfter.itemType;
    if (!m11_inventory_get_item_in_chest_slot(&state, 0,
            DM1_PC34_HAND_SWAP_WITH_CHEST_FULL_INDEX, &item)) {
        return 0;
    }
    out->mainChampionUnaffectedSlot = item.itemType;

    hand.actionHandIndex = 1;
    out->nonActionHandRejected =
        M11_V1_Inventory_HandSwapWithChest_ApplyPc34(
            &hand, &slot, &result) ? 0 : 1;
    hand.actionHandIndex = 0;
    hand.expectedOpenChestThing = DM1_PC34_HAND_SWAP_WITH_CHEST_OTHER_THING + 9;
    out->staleChestRejected =
        M11_V1_Inventory_HandSwapWithChest_ApplyPc34(
            &hand, &slot, &result) ? 0 : 1;
    out->noDuplicateTrackedItems =
        count_type3(out->otherChampionHandAfter, out->otherChampionSlotAfter,
                    out->mainChampionUnaffectedSlot,
                    DM1_PC34_HAND_SWAP_WITH_CHEST_OTHER_HAND_ITEM) == 1 &&
        count_type3(out->otherChampionHandAfter, out->otherChampionSlotAfter,
                    out->mainChampionUnaffectedSlot,
                    DM1_PC34_HAND_SWAP_WITH_CHEST_OTHER_FIRST_ITEM) == 1;
    return 1;
}
