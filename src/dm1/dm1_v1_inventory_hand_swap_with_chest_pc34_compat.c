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

static DM1_V1_ItemPc34 make_item(int itemType, int weight, int charges,
                          int allowedSlots)
{
    DM1_V1_ItemPc34 item;

    memset(&item, 0, sizeof(item));
    item.itemType = itemType;
    item.weight = weight;
    item.charges = charges;
    item.identified = 1;
    item.allowedSlots = allowedSlots;
    return item;
}

static void copy_item_types(const DM1_V1_ItemPc34* items, int count, int* typesOut)
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

int DM1_V1_InventoryHandSwapWithChest_ApplyPc34Compat(
    const DM1_V1_InventoryHandSwapWithChestHandPc34* hand,
    const DM1_V1_InventoryHandSwapWithChestSlotPc34* chestSlot,
    DM1_V1_InventoryHandSwapWithChestResultPc34* out)
{
    DM1_V1_InventoryStatePc34* state;
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
        DM1_V1_Inventory_GetOpenChestThingPc34Compat(state, championIndex);
    if (out->openChestThingBefore == 0 ||
        out->openChestThingBefore != hand->expectedOpenChestThing) {
        return 0;
    }
    pc34Slot = DM1_PC34_SLOT_CHEST_1 + chestSlot->chestSlotIndex;
    out->pc34ChestSlot = pc34Slot;

    if (!DM1_V1_Inventory_GetMouseItemPc34Compat(state, championIndex, &out->handBefore) ||
        !DM1_V1_Inventory_GetItemInChestSlotPc34Compat(
            state, championIndex, chestSlot->chestSlotIndex,
            &out->originalChestOccupant)) {
        return 0;
    }

    out->visibleWeightBefore =
        DM1_V1_InventoryChestLoad_OpenChestVisibleContentsWeightPc34Compat(
            state, championIndex);

    if (out->handBefore.itemType != 0 &&
        !DM1_V1_Inventory_CanEquipPc34Compat(&out->handBefore, pc34Slot)) {
        out->incompatibleRejected = 1;
        out->visibleWeightAfter = out->visibleWeightBefore;
        out->handAfter = out->handBefore;
        out->chestSlotAfter = out->originalChestOccupant;
        return 1;
    }

    /* ReDMCSB CHAMPION.C F0302 lines 688-710 reads the leader hand and C30+
     * G0425 slot, rejects incompatible masks, then delegates the real swap to
     * F0298/F0300/F0297/F0301; CHEST.C F0333 lines 31-67 supplies G0425. */
    out->accepted = DM1_V1_Inventory_ClickOpenChestSlotForThingPc34Compat(
        state, championIndex, hand->expectedOpenChestThing,
        chestSlot->chestSlotIndex);
    if (!DM1_V1_Inventory_GetMouseItemPc34Compat(state, championIndex, &out->handAfter) ||
        !DM1_V1_Inventory_GetItemInChestSlotPc34Compat(
            state, championIndex, chestSlot->chestSlotIndex,
            &out->chestSlotAfter)) {
        return 0;
    }
    out->visibleWeightAfter =
        DM1_V1_InventoryChestLoad_OpenChestVisibleContentsWeightPc34Compat(
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

int DM1_V1_InventoryHandSwapWithChest_RunPc34Compat(
    DM1_V1_InventoryHandSwapWithChestProbePc34* out)
{
    DM1_V1_InventoryStatePc34 state;
    DM1_V1_ItemPc34 mainInput[DM1_PC34_HAND_SWAP_WITH_CHEST_SLOT_COUNT];
    DM1_V1_ItemPc34 otherInput[DM1_PC34_HAND_SWAP_WITH_CHEST_SLOT_COUNT];
    DM1_V1_ItemPc34 closed[DM1_PC34_HAND_SWAP_WITH_CHEST_SLOT_COUNT];
    int closedTypes[DM1_PC34_HAND_SWAP_WITH_CHEST_SLOT_COUNT];
    DM1_V1_InventoryHandSwapWithChestHandPc34 hand;
    DM1_V1_InventoryHandSwapWithChestSlotPc34 slot;
    DM1_V1_InventoryHandSwapWithChestResultPc34 result;
    DM1_V1_ItemPc34 item;
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
        DM1_V1_InventoryChestLoad_SourceEvidencePc34Compat() != NULL ? 1 : 0;
    out->stackingNotApplicable =
        dm1_v1_inventory_hand_swap_with_chest_pc34_spec.stackingNotApplicable;

    DM1_V1_Inventory_InitPc34Compat(&state, 3);
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
    out->mainOpenResult = DM1_V1_Inventory_OpenChestPc34Compat(
        &state, 0, DM1_PC34_HAND_SWAP_WITH_CHEST_MAIN_THING,
        mainInput, DM1_PC34_HAND_SWAP_WITH_CHEST_SLOT_COUNT);
    out->mainOpenThing = DM1_V1_Inventory_GetOpenChestThingPc34Compat(&state, 0);
    hand.inventory = &state;
    hand.championIndex = 0;
    hand.actionHandIndex = 0;
    hand.expectedOpenChestThing = DM1_PC34_HAND_SWAP_WITH_CHEST_MAIN_THING;

    /* ReDMCSB CHAMPION.C F0302 lines 694-710 allows empty leader-hand pickup
     * from a C30+ chest slot and leaves the selected G0425 entry empty. */
    slot.chestSlotIndex = DM1_PC34_HAND_SWAP_WITH_CHEST_EMPTY_INDEX;
    if (!DM1_V1_InventoryHandSwapWithChest_ApplyPc34Compat(
            &hand, &slot, &result)) {
        return 0;
    }
    out->emptySwapResult = result.accepted;
    out->emptyHandBefore = result.handBefore.itemType;
    out->emptyHandAfter = result.handAfter.itemType;
    out->emptyOriginalChestOccupant = result.originalChestOccupant.itemType;
    out->emptyChestSlotAfter = result.chestSlotAfter.itemType;
    out->emptyClosedCount = DM1_V1_Inventory_CloseChestPc34Compat(
        &state, 0, closed, DM1_PC34_HAND_SWAP_WITH_CHEST_SLOT_COUNT);
    copy_item_types(closed, out->emptyClosedCount, closedTypes);
    out->emptyReopenResult = DM1_V1_Inventory_OpenChestPc34Compat(
        &state, 0, DM1_PC34_HAND_SWAP_WITH_CHEST_MAIN_THING + 1,
        closed, out->emptyClosedCount);
    if (!DM1_V1_Inventory_GetItemInChestSlotPc34Compat(
            &state, 0, DM1_PC34_HAND_SWAP_WITH_CHEST_EMPTY_INDEX,
            &item)) {
        return 0;
    }
    out->emptyReopenedSlotContainsNextVisible =
        item.itemType == DM1_PC34_HAND_SWAP_WITH_CHEST_FIRST_ITEM +
            DM1_PC34_HAND_SWAP_WITH_CHEST_EMPTY_INDEX + 1 ? 1 : 0;
    if (!DM1_V1_Inventory_GetMouseItemPc34Compat(&state, 0, &item)) {
        return 0;
    }
    out->emptyReopenedOriginalStillInHand =
        item.itemType == out->emptyOriginalChestOccupant ? 1 : 0;

    /* ReDMCSB CHAMPION.C F0302 lines 688-710 swaps a full leader hand with
     * occupied C541/G0425, preserving charges through F0301 lines 606-660. */
    out->fullHandSetupResult = DM1_V1_Inventory_SetMouseItemPc34Compat(
        &state, 0, DM1_PC34_HAND_SWAP_WITH_CHEST_FULL_HAND_ITEM,
        17, DM1_PC34_HAND_SWAP_WITH_CHEST_STACK_CHARGES,
        DM1_PC34_ALLOWED_CONTAINER);
    hand.expectedOpenChestThing = DM1_PC34_HAND_SWAP_WITH_CHEST_MAIN_THING + 1;
    slot.chestSlotIndex = DM1_PC34_HAND_SWAP_WITH_CHEST_FULL_INDEX;
    if (!out->fullHandSetupResult ||
        !DM1_V1_InventoryHandSwapWithChest_ApplyPc34Compat(
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
    out->fullClosedCount = DM1_V1_Inventory_CloseChestPc34Compat(
        &state, 0, closed, DM1_PC34_HAND_SWAP_WITH_CHEST_SLOT_COUNT);
    out->fullReopenResult = DM1_V1_Inventory_OpenChestPc34Compat(
        &state, 0, DM1_PC34_HAND_SWAP_WITH_CHEST_MAIN_THING + 2,
        closed, out->fullClosedCount);
    if (!DM1_V1_Inventory_GetItemInChestSlotPc34Compat(
            &state, 0, DM1_PC34_HAND_SWAP_WITH_CHEST_FULL_INDEX,
            &item)) {
        return 0;
    }
    out->fullReopenedSlotContainsPreviousHand =
        item.itemType == DM1_PC34_HAND_SWAP_WITH_CHEST_FULL_HAND_ITEM ? 1 : 0;

    /* ReDMCSB CHAMPION.C F0302 lines 688-710 applies the same C541 swap in
     * reverse, restoring the original occupant before CHEST.C F0334 closes. */
    hand.expectedOpenChestThing = DM1_PC34_HAND_SWAP_WITH_CHEST_MAIN_THING + 2;
    if (!DM1_V1_InventoryHandSwapWithChest_ApplyPc34Compat(
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
    out->restoreClosedCount = DM1_V1_Inventory_CloseChestPc34Compat(
        &state, 0, closed, DM1_PC34_HAND_SWAP_WITH_CHEST_SLOT_COUNT);
    out->restoreReopenResult = DM1_V1_Inventory_OpenChestPc34Compat(
        &state, 0, DM1_PC34_HAND_SWAP_WITH_CHEST_MAIN_THING + 3,
        closed, out->restoreClosedCount);
    if (!DM1_V1_Inventory_GetItemInChestSlotPc34Compat(
            &state, 0, DM1_PC34_HAND_SWAP_WITH_CHEST_FULL_INDEX,
            &item)) {
        return 0;
    }
    out->restoreReopenedSlotOriginalOccupant =
        item.itemType == out->fullOriginalChestOccupant ? 1 : 0;

    /* ReDMCSB CHAMPION.C F0302 lines 697-699 rejects non-container masks for
     * C30+ chest slots before F0298/F0300/F0301 mutate hand or G0425 state. */
    hand.expectedOpenChestThing = DM1_PC34_HAND_SWAP_WITH_CHEST_MAIN_THING + 3;
    if (!DM1_V1_Inventory_SetMouseItemPc34Compat(
            &state, 0, DM1_PC34_TEST_STAFF_OF_CLAWS_OBJECT_INFO,
            1, 0, DM1_PC34_ALLOWED_QUIVER_LINE1) ||
        !DM1_V1_InventoryHandSwapWithChest_ApplyPc34Compat(
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
    out->otherChampionOpenResult = DM1_V1_Inventory_OpenChestPc34Compat(
        &state, 1, DM1_PC34_HAND_SWAP_WITH_CHEST_OTHER_THING,
        otherInput, DM1_PC34_HAND_SWAP_WITH_CHEST_SLOT_COUNT);
    if (!DM1_V1_Inventory_SetMouseItemPc34Compat(
            &state, 1, DM1_PC34_HAND_SWAP_WITH_CHEST_OTHER_HAND_ITEM,
            23, 0, DM1_PC34_ALLOWED_CONTAINER)) {
        return 0;
    }
    hand.championIndex = 1;
    hand.expectedOpenChestThing = DM1_PC34_HAND_SWAP_WITH_CHEST_OTHER_THING;
    slot.chestSlotIndex = 0;
    if (!DM1_V1_InventoryHandSwapWithChest_ApplyPc34Compat(
            &hand, &slot, &result)) {
        return 0;
    }
    out->otherChampionSwapResult = result.accepted;
    out->otherChampionHandAfter = result.handAfter.itemType;
    out->otherChampionSlotAfter = result.chestSlotAfter.itemType;
    if (!DM1_V1_Inventory_GetItemInChestSlotPc34Compat(&state, 0,
            DM1_PC34_HAND_SWAP_WITH_CHEST_FULL_INDEX, &item)) {
        return 0;
    }
    out->mainChampionUnaffectedSlot = item.itemType;

    hand.actionHandIndex = 1;
    out->nonActionHandRejected =
        DM1_V1_InventoryHandSwapWithChest_ApplyPc34Compat(
            &hand, &slot, &result) ? 0 : 1;
    hand.actionHandIndex = 0;
    hand.expectedOpenChestThing = DM1_PC34_HAND_SWAP_WITH_CHEST_OTHER_THING + 9;
    out->staleChestRejected =
        DM1_V1_InventoryHandSwapWithChest_ApplyPc34Compat(
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
