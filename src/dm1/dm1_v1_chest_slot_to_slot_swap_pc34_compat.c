#include "dm1_v1_chest_slot_to_slot_swap_pc34_compat.h"

#include <string.h>

static const char s_source_evidence[] =
    "CHEST.C F0333:30-32 ignores an already-open G0426 chest without rebuilding G0425\n"
    "CHEST.C F0333:53-67 copies the first eight linked objects into G0425_aT_ChestSlots / C537-C544\n"
    "CHEST.C F0334:117-132 clears and rewrites the open container from non-empty G0425 slots\n"
    "INVENTORY.C (PC 3.4) AllowedSlots lookup uses DATA.C G0038 lines 1049-1087 slot masks\n"
    "DEFS.H C545/C546/C547 pouch/quiver/backpack mask contract: 0x0100/0x0040/0xFFFF\n"
    "OBJECT.C F0032/F0033:121-212 resolves object type and icon identity for slot redraw\n"
    "AMMO.C F0294:54-79 bow/sling ammunition compatibility is unrelated to container rearrangement\n"
    "BLITMASK.C F0133:30-33 uses transparent color C10 for masked icon blits\n"
    "CHAMDRAW.C F0291/F0296:551-552,1249-1252 reads G0425 for C30+ chest icon refresh";

const DM1_V1_ChestSlotToSlotSwapSpecPc34
    dm1_v1_chest_slot_to_slot_swap_pc34_spec = {
        "Source-locked contract gate only; not full real-asset chest runtime parity.",
        DM1_PC34_SLOT_CHEST_1,
        DM1_PC34_SLOT_CHEST_8,
        DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_SOURCE_PC34_SLOT,
        DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_DEST_PC34_SLOT,
        DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_SOURCE_INDEX,
        DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_DEST_INDEX,
        DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_EMPTY_INDEX_A,
        DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_EMPTY_INDEX_B,
        DM1_PC34_ALLOWED_CONTAINER,
        DM1_PC34_ALLOWED_POUCH,
        DM1_PC34_ALLOWED_QUIVER_LINE1,
        DM1_PC34_ALLOWED_ANY_SLOT,
        DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_C10_TRANSPARENT_COLOR,
        DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_CONTRACT_ASSERTION_BUDGET
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

static int copy_chest_view(const M11_InventoryState* state,
                           int* typesOut,
                           int* allowedOut)
{
    int i;

    if (!state || !typesOut || !allowedOut) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_SLOT_COUNT; ++i) {
        M11_Item item;

        if (!m11_inventory_get_item_in_chest_slot(state, 0, i, &item)) {
            return 0;
        }
        typesOut[i] = item.itemType;
        allowedOut[i] = item.allowedSlots;
    }
    return 1;
}

static void copy_closed_view(const M11_Item* items,
                             int count,
                             int* typesOut,
                             int* allowedOut)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_SLOT_COUNT; ++i) {
        if (items && i < count && items[i].itemType != 0) {
            typesOut[i] = items[i].itemType;
            allowedOut[i] = items[i].allowedSlots;
        } else {
            typesOut[i] = 0;
            allowedOut[i] = 0;
        }
    }
}

static int count_non_empty(const int* types)
{
    int count = 0;
    int i;

    if (!types) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_SLOT_COUNT; ++i) {
        if (types[i] != 0) {
            ++count;
        }
    }
    return count;
}

static int no_duplicate_types(const int* types)
{
    int i;

    if (!types) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_SLOT_COUNT; ++i) {
        int j;

        if (types[i] == 0) {
            continue;
        }
        for (j = i + 1; j < DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_SLOT_COUNT; ++j) {
            if (types[i] == types[j]) {
                return 0;
            }
        }
    }
    return 1;
}

static int views_match(const int* a, const int* b)
{
    int i;

    if (!a || !b) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_SLOT_COUNT; ++i) {
        if (a[i] != b[i]) {
            return 0;
        }
    }
    return 1;
}

static int begin_drag_from_chest_slot(M11_InventoryState* state,
                                      int slotIndex,
                                      M11_Item* selectedOut)
{
    M11_Item item;

    if (!state || !selectedOut || slotIndex < 0 ||
        slotIndex >= DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_SLOT_COUNT ||
        !m11_inventory_get_item_in_chest_slot(state, 0, slotIndex, &item) ||
        item.itemType == 0) {
        return 0;
    }

    /* ReDMCSB CHEST.C F0333 lines 53-67 has already materialized the visible
     * C537-C544 window into G0425.  This contract-only drag selection removes
     * the picked G0425 member from its original visible index without touching
     * the leader hand. */
    *selectedOut = item;
    return m11_inventory_set_item_in_chest_slot(state, 0, slotIndex, 0, 0, 0, 0);
}

static int drop_dragged_item_on_chest_slot(M11_InventoryState* state,
                                           int slotIndex,
                                           M11_Item* selected)
{
    M11_Item destination;

    if (!state || !selected || selected->itemType == 0 || slotIndex < 0 ||
        slotIndex >= DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_SLOT_COUNT ||
        !m11_inventory_get_item_in_chest_slot(state, 0, slotIndex,
                                              &destination)) {
        return 0;
    }

    /* ReDMCSB CHEST.C F0334 lines 117-132 later serializes the G0425 order.
     * Preserve each object's AllowedSlots payload while exchanging the two
     * visible C30+ entries so the close/reopen chain remains source-faithful. */
    if (!m11_inventory_set_item_in_chest_slot(
            state, 0, slotIndex, selected->itemType, selected->weight,
            selected->charges, selected->allowedSlots)) {
        return 0;
    }
    *selected = destination;
    return 1;
}

const char* dm1_v1_chest_slot_to_slot_swap_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const DM1_V1_ChestSlotToSlotSwapSpecPc34*
dm1_v1_chest_slot_to_slot_swap_spec_pc34(void)
{
    return &dm1_v1_chest_slot_to_slot_swap_pc34_spec;
}

int dm1_v1_chest_slot_to_slot_swap_run_pc34(
    DM1_V1_ChestSlotToSlotSwapProbePc34* out)
{
    M11_InventoryState state;
    M11_Item linked[DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_SLOT_COUNT];
    M11_Item closed[DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_SLOT_COUNT];
    M11_Item selected;
    M11_Item hand;
    M11_Item item;
    int scratchAllowed[DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_SLOT_COUNT];

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    memset(linked, 0, sizeof(linked));
    memset(closed, 0, sizeof(closed));
    memset(&selected, 0, sizeof(selected));

    out->sourceLockedContractOnly = 1;
    out->noAssetParityClaim = 1;
    out->chestThing = DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_CHEST_THING;
    out->sourcePc34Slot = DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_SOURCE_PC34_SLOT;
    out->destinationPc34Slot = DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_DEST_PC34_SLOT;
    out->sourceIndex = DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_SOURCE_INDEX;
    out->destinationIndex = DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_DEST_INDEX;
    out->emptyIndexA = DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_EMPTY_INDEX_A;
    out->emptyIndexB = DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_EMPTY_INDEX_B;
    out->chestSlotMaskSource = m11_inventory_pc34_slot_mask(out->sourcePc34Slot);
    out->chestSlotMaskDestination =
        m11_inventory_pc34_slot_mask(out->destinationPc34Slot);
    out->pouchMask = DM1_PC34_ALLOWED_POUCH;
    out->quiverLine1Mask = DM1_PC34_ALLOWED_QUIVER_LINE1;
    out->backpackMask = DM1_PC34_ALLOWED_ANY_SLOT;

    linked[0] = make_item(DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_ITEM_A, 5,
                          DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_ALLOWED_A);
    linked[1] = make_item(DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_FILLER_1, 6,
                          DM1_PC34_ALLOWED_CONTAINER);
    linked[2] = make_item(DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_FILLER_2, 7,
                          DM1_PC34_ALLOWED_CONTAINER);
    linked[3] = make_item(DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_FILLER_3, 8,
                          DM1_PC34_ALLOWED_CONTAINER);
    linked[4] = make_item(DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_ITEM_B, 9,
                          DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_ALLOWED_B);

    m11_inventory_init(&state, 1);
    if (!m11_inventory_set_mouse_item(
            &state, 0, DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_LEADER_SENTINEL,
            3, 0, DM1_PC34_ALLOWED_ANY_SLOT) ||
        !m11_inventory_get_mouse_item(&state, 0, &hand)) {
        return 0;
    }
    out->leaderHandBefore = hand.itemType;

    /* ReDMCSB CHEST.C F0333 lines 53-67 opens C537-C544 from the linked
     * container chain; slots 6-8 are intentionally empty for the no-op lane. */
    out->openResult = m11_inventory_open_chest(
        &state, 0, out->chestThing, linked,
        DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_SLOT_COUNT);
    out->openThing = m11_inventory_get_open_chest_thing(&state, 0);
    if (!out->openResult ||
        !copy_chest_view(&state, out->beforeTypes, out->beforeAllowed)) {
        return 0;
    }
    out->sameOpenNoopResult = m11_inventory_open_chest(
        &state, 0, out->chestThing, linked,
        DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_SLOT_COUNT);
    out->sameOpenThing = m11_inventory_get_open_chest_thing(&state, 0);
    out->nonEmptyCountBefore = count_non_empty(out->beforeTypes);
    out->itemAMaskBefore = out->beforeAllowed[out->sourceIndex];
    out->itemBMaskBefore = out->beforeAllowed[out->destinationIndex];
    out->itemAContainerOverlap =
        out->itemAMaskBefore & out->chestSlotMaskSource;
    out->itemBContainerOverlap =
        out->itemBMaskBefore & out->chestSlotMaskDestination;
    out->loadBeforeSwap = m11_inventory_get_load(&state, 0);

    out->selectedSourceResult = begin_drag_from_chest_slot(
        &state, out->sourceIndex, &selected);
    out->selectedSourceIndex = out->sourceIndex;
    out->selectedThing = selected.itemType;
    if (!copy_chest_view(&state, out->duringDragTypes, scratchAllowed)) {
        return 0;
    }
    out->sourceEmptyDuringDrag =
        out->duringDragTypes[out->sourceIndex] == 0 ? 1 : 0;

    out->selectedDestinationResult = drop_dragged_item_on_chest_slot(
        &state, out->destinationIndex, &selected);
    if (!out->selectedDestinationResult ||
        !m11_inventory_set_item_in_chest_slot(
            &state, 0, out->sourceIndex, selected.itemType, selected.weight,
            selected.charges, selected.allowedSlots) ||
        !copy_chest_view(&state, out->afterSwapTypes, out->afterSwapAllowed) ||
        !m11_inventory_get_mouse_item(&state, 0, &hand)) {
        return 0;
    }
    out->leaderHandAfterSwap = hand.itemType;
    out->loadAfterSwap = m11_inventory_get_load(&state, 0);
    out->swapCompleted = 1;
    out->sourceReceivesB =
        out->afterSwapTypes[out->sourceIndex] ==
        DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_ITEM_B ? 1 : 0;
    out->destinationReceivesA =
        out->afterSwapTypes[out->destinationIndex] ==
        DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_ITEM_A ? 1 : 0;
    out->itemAAllowedMaskPreserved =
        out->afterSwapAllowed[out->destinationIndex] ==
        out->itemAMaskBefore ? 1 : 0;
    out->itemBAllowedMaskPreserved =
        out->afterSwapAllowed[out->sourceIndex] ==
        out->itemBMaskBefore ? 1 : 0;
    out->leaderHandUntouchedBySwap =
        out->leaderHandAfterSwap == out->leaderHandBefore ? 1 : 0;
    out->loadUnchangedBySwap =
        out->loadAfterSwap == out->loadBeforeSwap ? 1 : 0;
    out->viewPreservedAfterSwap =
        out->afterSwapTypes[1] == DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_FILLER_1 &&
        out->afterSwapTypes[2] == DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_FILLER_2 &&
        out->afterSwapTypes[3] == DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_FILLER_3 &&
        out->afterSwapTypes[5] == 0 ? 1 : 0;
    out->iconRefreshAcknowledged = out->sourceReceivesB &&
        out->destinationReceivesA ? 1 : 0;
    out->maskedBlitTransparentC10 =
        DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_C10_TRANSPARENT_COLOR == 10 ? 1 : 0;
    out->objectTypeIconLookupAcknowledged =
        out->selectedThing == DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_ITEM_A ? 1 : 0;
    out->ammoCompatibilityUnaffected =
        out->pouchMask == DM1_PC34_ALLOWED_POUCH &&
        out->quiverLine1Mask == DM1_PC34_ALLOWED_QUIVER_LINE1 ? 1 : 0;
    out->nonEmptyCountAfterSwap = count_non_empty(out->afterSwapTypes);

    memset(&selected, 0, sizeof(selected));
    out->emptySourceClickResult = begin_drag_from_chest_slot(
        &state, out->emptyIndexA, &selected);
    out->emptyDestinationClickResult = selected.itemType == 0 ?
        begin_drag_from_chest_slot(&state, out->emptyIndexB, &selected) : 0;
    if (!copy_chest_view(&state, out->afterEmptyNoopTypes, scratchAllowed) ||
        !m11_inventory_get_mouse_item(&state, 0, &hand)) {
        return 0;
    }
    out->leaderHandAfterEmptyNoop = hand.itemType;
    out->loadAfterEmptyNoop = m11_inventory_get_load(&state, 0);
    out->emptySlotsNoop =
        out->emptySourceClickResult == 0 &&
        out->emptyDestinationClickResult == 0 &&
        views_match(out->afterSwapTypes, out->afterEmptyNoopTypes) ? 1 : 0;
    out->loadUnchangedByNoop =
        out->loadAfterEmptyNoop == out->loadAfterSwap ? 1 : 0;

    if (!m11_inventory_set_mouse_item(
            &state, 0, DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_INVALID_HAND_ITEM,
            4, 0, DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_INVALID_ALLOWED) ||
        !m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    out->invalidCanEquip = m11_inventory_can_equip(
        &item, out->destinationPc34Slot);
    out->invalidClickResult = m11_inventory_click_pc34_source_slot(
        &state, 0, out->destinationPc34Slot);
    if (!copy_chest_view(&state, out->afterInvalidTypes, scratchAllowed) ||
        !m11_inventory_get_mouse_item(&state, 0, &hand)) {
        return 0;
    }
    out->leaderHandAfterInvalidClick = hand.itemType;
    out->loadAfterInvalidClick = m11_inventory_get_load(&state, 0);
    out->invalidSlotUnchanged =
        out->invalidClickResult == 0 &&
        out->afterInvalidTypes[out->destinationIndex] ==
        out->afterSwapTypes[out->destinationIndex] ? 1 : 0;
    out->loadUnchangedByInvalidClick =
        out->loadAfterInvalidClick == out->loadAfterEmptyNoop ? 1 : 0;
    if (!m11_inventory_set_mouse_item(
            &state, 0, DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_LEADER_SENTINEL,
            3, 0, DM1_PC34_ALLOWED_ANY_SLOT)) {
        return 0;
    }

    /* ReDMCSB CHEST.C F0334 lines 117-132 rewrites only non-empty G0425
     * entries, so the filler entries keep C535-C541 dense and allow C541
     * (chest_5) to remain item A after a close/reopen round trip. */
    out->closeCount = m11_inventory_close_chest(
        &state, 0, closed, DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_SLOT_COUNT);
    copy_closed_view(closed, out->closeCount, out->closedTypes,
                     out->closedAllowed);
    out->openChestClearedAfterClose =
        m11_inventory_get_open_chest_thing(&state, 0) == 0 ? 1 : 0;
    if (!m11_inventory_get_mouse_item(&state, 0, &hand)) {
        return 0;
    }
    out->leaderHandAfterClose = hand.itemType;
    out->loadAfterClose = m11_inventory_get_load(&state, 0);
    out->nonEmptyCountAfterClose = count_non_empty(out->closedTypes);

    out->reopenResult = m11_inventory_open_chest(
        &state, 0, out->chestThing, closed, out->closeCount);
    out->reopenedOpenThing = m11_inventory_get_open_chest_thing(&state, 0);
    if (!out->reopenResult ||
        !copy_chest_view(&state, out->reopenedTypes, out->reopenedAllowed) ||
        !m11_inventory_get_mouse_item(&state, 0, &hand)) {
        return 0;
    }
    out->leaderHandAfterReopen = hand.itemType;
    out->loadAfterReopen = m11_inventory_get_load(&state, 0);
    out->reopenedLoadMatchesOpenLoad =
        out->loadAfterReopen == out->loadBeforeSwap ? 1 : 0;
    out->closeRewritePreservedVisibleOrder =
        views_match(out->closedTypes, out->afterSwapTypes) ? 1 : 0;
    out->reopenPreservedSlotToSlotSwap =
        out->reopenedTypes[out->sourceIndex] ==
        DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_ITEM_B &&
        out->reopenedTypes[out->destinationIndex] ==
        DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_ITEM_A ? 1 : 0;
    out->noDuplicateObjectIds = no_duplicate_types(out->reopenedTypes);
    out->nonEmptyCountAfterReopen = count_non_empty(out->reopenedTypes);
    out->noLeaderHandLeak =
        out->leaderHandAfterSwap == out->leaderHandBefore &&
        out->leaderHandAfterClose == out->leaderHandBefore &&
        out->leaderHandAfterReopen == out->leaderHandBefore ? 1 : 0;

    return 1;
}
