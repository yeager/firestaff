#include "dm1_v1_chest_pickup_encumbrance_partial_stack_runtime_pc34_compat.h"

#include <string.h>

/*
 * DM1 V1 chest partial-stack pickup encumbrance gate.
 *
 * ReDMCSB source lock:
 * - CHEST.C F0333 lines 53-76 materializes linked CONTENTS into the open
 *   G0425 chest slots that this probe treats as the visible stack.
 * - CHEST.C F0334 lines 117-132 rewires only non-empty G0425 slots on close;
 *   the probe uses the same compacted order after the bottom item is removed.
 * - CHAMPION.C F0284 lines 93-130 is the requested local state helper anchor
 *   for champion state redraw context; the local source's weight helpers are
 *   F0297/F0300/F0301 lines 263-265 and 582-615, with F0310 lines 1198-1205
 *   comparing Load against F0309 maximum load.
 * - No WEIGHT.C exists in the local ReDMCSB source root; DUNGEON.C F0140
 *   lines 1082-1133 is the object-weight helper.
 */

static DM1_V1_ItemPc34 make_item(int itemType, int weight, int allowedSlots)
{
    DM1_V1_ItemPc34 item;

    memset(&item, 0, sizeof(item));
    item.itemType = itemType;
    item.weight = weight;
    item.allowedSlots = allowedSlots;
    item.identified = 1;
    return item;
}

static int stack_count(const DM1_V1_InventoryStatePc34* state, int champ)
{
    int count = 0;
    int i;

    for (i = 0; i < DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_SLOT_COUNT; ++i) {
        DM1_V1_ItemPc34 item;

        if (DM1_V1_Inventory_GetItemInChestSlotPc34Compat(state, champ, i, &item) &&
            item.itemType != 0) {
            ++count;
        }
    }
    return count;
}

static int stack_visible_weight(const DM1_V1_InventoryStatePc34* state, int champ)
{
    return m11_inventory_pc34_open_chest_visible_contents_weight(state, champ);
}

static DM1_V1_ItemPc34 stack_bottom(const DM1_V1_InventoryStatePc34* state, int champ)
{
    DM1_V1_ItemPc34 empty;
    int i;

    memset(&empty, 0, sizeof(empty));
    for (i = 0; i < DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_SLOT_COUNT; ++i) {
        DM1_V1_ItemPc34 item;

        if (DM1_V1_Inventory_GetItemInChestSlotPc34Compat(state, champ, i, &item) &&
            item.itemType != 0) {
            return item;
        }
    }
    return empty;
}

static DM1_V1_ItemPc34 stack_top(const DM1_V1_InventoryStatePc34* state, int champ)
{
    DM1_V1_ItemPc34 top;
    int i;

    memset(&top, 0, sizeof(top));
    for (i = 0; i < DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_SLOT_COUNT; ++i) {
        DM1_V1_ItemPc34 item;

        if (DM1_V1_Inventory_GetItemInChestSlotPc34Compat(state, champ, i, &item) &&
            item.itemType != 0) {
            top = item;
        }
    }
    return top;
}

static void copy_slots(const DM1_V1_InventoryStatePc34* state,
                       int champ,
                       int* typesOut,
                       int* weightsOut)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_SLOT_COUNT; ++i) {
        DM1_V1_ItemPc34 item;

        memset(&item, 0, sizeof(item));
        (void)DM1_V1_Inventory_GetItemInChestSlotPc34Compat(state, champ, i, &item);
        typesOut[i] = item.itemType;
        weightsOut[i] = item.weight;
    }
}

static int compact_visible_stack(DM1_V1_InventoryStatePc34* state, int champ)
{
    DM1_V1_ItemPc34 compacted[DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_SLOT_COUNT];
    int count = 0;
    int i;

    memset(compacted, 0, sizeof(compacted));
    for (i = 0; i < DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_SLOT_COUNT; ++i) {
        DM1_V1_ItemPc34 item;

        if (!DM1_V1_Inventory_GetItemInChestSlotPc34Compat(state, champ, i, &item)) {
            return 0;
        }
        if (item.itemType != 0) {
            compacted[count++] = item;
        }
    }

    /* ReDMCSB CHEST.C F0334 lines 117-132 relinks non-empty G0425 entries
     * in visible order.  Keep the synthetic stack compact for the next live
     * pickup so the K-1 remaining items are the next click source. */
    for (i = 0; i < DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_SLOT_COUNT; ++i) {
        if (!DM1_V1_Inventory_SetItemInChestSlotPc34Compat(
                state, champ, i, compacted[i].itemType, compacted[i].weight,
                compacted[i].charges, compacted[i].allowedSlots)) {
            return 0;
        }
    }
    return 1;
}

static int slots_match(const int* a, const int* b)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_SLOT_COUNT; ++i) {
        if (a[i] != b[i]) {
            return 0;
        }
    }
    return 1;
}

static int pickup_bottom_with_weight_gate(
    DM1_V1_InventoryStatePc34* state,
    int champ,
    int storagePc34Slot,
    int capacityN,
    int* carriedLoad,
    DM1_V1_ChestPickupEncumbrancePartialStackEventPc34* out)
{
    DM1_V1_ItemPc34 bottom;
    DM1_V1_ItemPc34 top;
    DM1_V1_ItemPc34 handBefore;
    DM1_V1_ItemPc34 handAfter;
    DM1_V1_ItemPc34 stored;

    if (!state || !carriedLoad || !out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    copy_slots(state, champ, out->slotTypesBefore, out->slotWeightsBefore);
    bottom = stack_bottom(state, champ);
    top = stack_top(state, champ);
    memset(&handBefore, 0, sizeof(handBefore));
    (void)DM1_V1_Inventory_GetMouseItemPc34Compat(
        state, DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_LEADER, &handBefore);

    out->championIndex = champ;
    out->storagePc34Slot = storagePc34Slot;
    out->capacityN = capacityN;
    out->loadBefore = *carriedLoad;
    out->itemWeight = bottom.weight;
    out->candidateLoad = *carriedLoad + bottom.weight;
    out->stackCountBefore = stack_count(state, champ);
    out->topTypeBefore = top.itemType;
    out->bottomTypeBefore = bottom.itemType;
    out->pickupType = bottom.itemType;
    out->pickupWasBottom =
        bottom.itemType == out->slotTypesBefore[0] ? 1 : 0;
    out->visibleWeightBefore = stack_visible_weight(state, champ);
    out->leaderHandTypeBefore = handBefore.itemType;

    /* ReDMCSB CHAMPION.C F0310 lines 1198-1205 compares current Load against
     * F0309 maximum load.  This live pickup gate rejects a candidate item
     * before mutating G0425 when it would push the carrier beyond capacity. */
    if (bottom.itemType == 0 || out->candidateLoad > capacityN) {
        int beforeTypes[DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_SLOT_COUNT];

        memcpy(beforeTypes, out->slotTypesBefore, sizeof(beforeTypes));
        out->blocked = 1;
        out->result = 0;
        copy_slots(state, champ, out->slotTypesAfter, out->slotWeightsAfter);
        out->stackUnchangedAfterBlock =
            slots_match(beforeTypes, out->slotTypesAfter);
        out->loadAfter = *carriedLoad;
        out->stackCountAfter = stack_count(state, champ);
        out->topTypeAfter = stack_top(state, champ).itemType;
        out->bottomTypeAfter = stack_bottom(state, champ).itemType;
        out->nextPickupSeesCount = out->stackCountAfter;
        out->nextPickupSeesBottomType = out->bottomTypeAfter;
        out->nextPickupSeesTopType = out->topTypeAfter;
        out->visibleWeightAfter = stack_visible_weight(state, champ);
        memset(&stored, 0, sizeof(stored));
        (void)DM1_V1_Inventory_GetItemInPc34SourceSlotCompat(
            state, champ, storagePc34Slot, &stored);
        out->storageTypeAfter = stored.itemType;
        out->storageWeightAfter = stored.weight;
        memset(&handAfter, 0, sizeof(handAfter));
        (void)DM1_V1_Inventory_GetMouseItemPc34Compat(
            state, DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_LEADER,
            &handAfter);
        out->leaderHandTypeAfter = handAfter.itemType;
        return 1;
    }

    if (!DM1_V1_Inventory_SetItemInPc34SourceSlotCompat(
            state, champ, storagePc34Slot, bottom.itemType, bottom.weight,
            bottom.charges, bottom.allowedSlots) ||
        !DM1_V1_Inventory_SetItemInChestSlotPc34Compat(state, champ, 0, 0, 0, 0, 0) ||
        !compact_visible_stack(state, champ)) {
        return 0;
    }
    *carriedLoad = out->candidateLoad;

    out->blocked = 0;
    out->result = 1;
    out->loadAfter = *carriedLoad;
    copy_slots(state, champ, out->slotTypesAfter, out->slotWeightsAfter);
    out->stackCountAfter = stack_count(state, champ);
    out->topTypeAfter = stack_top(state, champ).itemType;
    out->bottomTypeAfter = stack_bottom(state, champ).itemType;
    out->nextPickupSeesCount = out->stackCountAfter;
    out->nextPickupSeesBottomType = out->bottomTypeAfter;
    out->nextPickupSeesTopType = out->topTypeAfter;
    out->visibleWeightAfter = stack_visible_weight(state, champ);
    memset(&stored, 0, sizeof(stored));
    (void)DM1_V1_Inventory_GetItemInPc34SourceSlotCompat(
        state, champ, storagePc34Slot, &stored);
    out->storageTypeAfter = stored.itemType;
    out->storageWeightAfter = stored.weight;
    memset(&handAfter, 0, sizeof(handAfter));
    (void)DM1_V1_Inventory_GetMouseItemPc34Compat(
        state, DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_LEADER, &handAfter);
    out->leaderHandTypeAfter = handAfter.itemType;

    return 1;
}

const char*
dm1_v1_chest_pickup_encumbrance_partial_stack_runtime_source_evidence_pc34(
    void)
{
    return
        "CHEST.C F0333 lines 53-76 opens linked CONTENTS into G0425 slots; "
        "CHEST.C F0334 lines 117-132 closes/rewires non-empty visible slots; "
        "CHAMPION.C F0284 lines 93-130 is the requested local state helper "
        "anchor, while F0297/F0300/F0301 lines 263-265 and 582-615 update "
        "Load and F0310 lines 1198-1205 compares Load to F0309 max load; "
        "WEIGHT.C is absent in this local ReDMCSB tree, DUNGEON.C F0140 "
        "lines 1082-1133 supplies object weight.";
}

int dm1_v1_chest_pickup_encumbrance_partial_stack_runtime_run_pc34(
    DM1_V1_ChestPickupEncumbrancePartialStackProbePc34* out)
{
    DM1_V1_InventoryStatePc34 state;
    DM1_V1_ItemPc34 linked[DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_K];
    DM1_V1_ItemPc34 hand;
    DM1_V1_ItemPc34 targetHand;
    DM1_V1_ItemPc34 storedFirst;
    DM1_V1_ItemPc34 storedSecond;
    DM1_V1_ItemPc34 closed[DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_SLOT_COUNT];
    int carriedLoad = DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_BASE_LOAD;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    memset(closed, 0, sizeof(closed));

    linked[0] = make_item(DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_BOTTOM,
                          45, DM1_PC34_ALLOWED_ANY_SLOT);
    linked[1] = make_item(DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_MIDDLE,
                          20, DM1_PC34_ALLOWED_ANY_SLOT);
    linked[2] = make_item(DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_TOP,
                          7, DM1_PC34_ALLOWED_ANY_SLOT);

    DM1_V1_Inventory_InitPc34Compat(
        &state, DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_CHAMPION_COUNT);
    out->setupResult = DM1_V1_Inventory_SetMouseItemPc34Compat(
        &state, DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_LEADER,
        DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_LEADER_HAND, 6, 0,
        DM1_PC34_ALLOWED_ANY_SLOT);
    if (!out->setupResult) {
        return 0;
    }

    out->championCount = state.championCount;
    out->leaderIndex = DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_LEADER;
    out->targetChampionIndex = DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_TARGET;
    out->targetIsNonLeader =
        out->targetChampionIndex != out->leaderIndex ? 1 : 0;
    out->capacityN = DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_CAPACITY_N;
    out->stackK = DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_K;
    out->initialCarriedLoad = carriedLoad;

    out->openResult = DM1_V1_Inventory_OpenChestPc34Compat(
        &state, out->targetChampionIndex,
        DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_CHEST_THING,
        linked, DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_K);
    out->openChestThing =
        DM1_V1_Inventory_GetOpenChestThingPc34Compat(&state, out->targetChampionIndex);
    if (!out->openResult) {
        return 0;
    }
    out->initialStackCount = stack_count(&state, out->targetChampionIndex);
    out->initialBottomType = stack_bottom(&state, out->targetChampionIndex).itemType;
    out->initialTopType = stack_top(&state, out->targetChampionIndex).itemType;
    out->initialVisibleWeight =
        stack_visible_weight(&state, out->targetChampionIndex);
    memset(&hand, 0, sizeof(hand));
    (void)DM1_V1_Inventory_GetMouseItemPc34Compat(
        &state, DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_LEADER, &hand);
    out->initialLeaderHandType = hand.itemType;
    memset(&targetHand, 0, sizeof(targetHand));
    (void)DM1_V1_Inventory_GetMouseItemPc34Compat(
        &state, out->targetChampionIndex, &targetHand);
    out->initialTargetHandType = targetHand.itemType;

    if (!pickup_bottom_with_weight_gate(
            &state, out->targetChampionIndex,
            DM1_PC34_SLOT_BACKPACK_LINE1_1, out->capacityN, &carriedLoad,
            &out->firstPickup) ||
        !pickup_bottom_with_weight_gate(
            &state, out->targetChampionIndex,
            DM1_PC34_SLOT_BACKPACK_LINE1_2, out->capacityN, &carriedLoad,
            &out->blockedPickup)) {
        return 0;
    }

    memset(&storedFirst, 0, sizeof(storedFirst));
    memset(&storedSecond, 0, sizeof(storedSecond));
    (void)DM1_V1_Inventory_GetItemInPc34SourceSlotCompat(
        &state, out->targetChampionIndex, DM1_PC34_SLOT_BACKPACK_LINE1_1,
        &storedFirst);
    (void)DM1_V1_Inventory_GetItemInPc34SourceSlotCompat(
        &state, out->targetChampionIndex, DM1_PC34_SLOT_BACKPACK_LINE1_2,
        &storedSecond);
    out->firstStoragePc34SlotTypeAfter = storedFirst.itemType;
    out->secondStoragePc34SlotTypeAfter = storedSecond.itemType;

    out->cancelClosedCount = DM1_V1_Inventory_CloseChestPc34Compat(
        &state, out->targetChampionIndex, closed,
        DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_SLOT_COUNT);
    out->cancelResult = out->cancelClosedCount >= 0 ? 1 : 0;
    if (!out->cancelResult) {
        return 0;
    }
    out->cancelRemainingStackCount = out->cancelClosedCount;
    for (i = 0; i < DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_SLOT_COUNT; ++i) {
        out->cancelClosedTypes[i] = closed[i].itemType;
        out->cancelClosedWeights[i] = closed[i].weight;
        out->cancelRemainingVisibleWeight += closed[i].weight;
    }
    out->cancelRemainingBottomType =
        out->cancelClosedCount > 0 ? closed[0].itemType : 0;
    out->cancelRemainingTopType =
        out->cancelClosedCount > 0 ?
        closed[out->cancelClosedCount - 1].itemType : 0;

    memset(&hand, 0, sizeof(hand));
    (void)DM1_V1_Inventory_GetMouseItemPc34Compat(
        &state, DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_LEADER, &hand);
    out->cancelLeaderHandType = hand.itemType;
    out->cancelLeaderHandWeight = hand.weight;
    out->cancelLeaderHandPreserved =
        hand.itemType == out->initialLeaderHandType ? 1 : 0;
    memset(&targetHand, 0, sizeof(targetHand));
    (void)DM1_V1_Inventory_GetMouseItemPc34Compat(
        &state, out->targetChampionIndex, &targetHand);
    out->cancelTargetHandType = targetHand.itemType;
    out->cancelTargetHandPreserved =
        targetHand.itemType == out->initialTargetHandType ? 1 : 0;
    out->cancelRemainingStackMatches =
        out->cancelClosedCount == 2 &&
        out->cancelClosedTypes[0] ==
            DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_MIDDLE &&
        out->cancelClosedTypes[1] ==
            DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_TOP ? 1 : 0;
    out->cancelPickedItemStillStored =
        storedFirst.itemType ==
            DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_BOTTOM ? 1 : 0;
    out->cancelBlockedSlotStillEmpty =
        storedSecond.itemType == 0 ? 1 : 0;

    return 1;
}
