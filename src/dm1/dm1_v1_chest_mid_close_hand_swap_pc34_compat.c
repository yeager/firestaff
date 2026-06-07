#include "dm1_v1_chest_mid_close_hand_swap_pc34_compat.h"

#include <string.h>

/*
 * Slice choice/non-overlap:
 * Existing gates cover ordinary close rewire after a pre-close swap,
 * hidden-tail truncation, floor drop, and cross-champion pickup. This model
 * isolates the contract for a leader-hand slot swap interleaved after
 * CHEST.C F0334 has begun iterating G0425_aT_ChestSlots.
 */

typedef struct {
    M11_Item chestSlots[DM1_PC34_CHEST_MID_CLOSE_SLOT_COUNT];
    M11_Item leaderHand;
    M11_Item closed[DM1_PC34_CHEST_MID_CLOSE_SLOT_COUNT];
    int openChestThing;
    int closeStarted;
    int closeCount;
    int containerSlotClearedToEnd;
    int lastLinkedType;
} MidCloseModelPc34;

static const DM1_V1_ChestMidCloseHandSwapSpecPc34 s_spec = {
    "Source-locked deterministic contract gate only; no real-asset pixel or icon parity claim.",
    "Non-duplicative: existing close-rewire swaps before close; this gate mutates G0425 after F0334 has already cleared G0426 and processed part of the close loop.",
    1,
    DM1_PC34_CHEST_MID_CLOSE_SLOT_COUNT,
    DM1_PC34_CHEST_MID_CLOSE_PREFIX_COUNT,
    DM1_PC34_CHEST_MID_CLOSE_ALREADY_PROCESSED_INDEX,
    DM1_PC34_CHEST_MID_CLOSE_FUTURE_INDEX,
    DM1_PC34_SLOT_CHEST_1 + DM1_PC34_CHEST_MID_CLOSE_FUTURE_INDEX,
    DM1_PC34_SLOT_CHEST_1 +
        DM1_PC34_CHEST_MID_CLOSE_ALREADY_PROCESSED_INDEX,
    DM1_PC34_CHEST_MID_CLOSE_CHEST_THING,
    DM1_PC34_CHEST_MID_CLOSE_FIRST_ITEM,
    DM1_PC34_CHEST_MID_CLOSE_FIRST_ITEM +
        DM1_PC34_CHEST_MID_CLOSE_FUTURE_INDEX,
    DM1_PC34_CHEST_MID_CLOSE_REPLACEMENT_ITEM
};

static M11_Item make_item(int itemType, int weight)
{
    M11_Item item;

    memset(&item, 0, sizeof(item));
    item.itemType = itemType;
    item.weight = weight;
    item.identified = 1;
    item.allowedSlots = DM1_PC34_ALLOWED_CONTAINER;
    return item;
}

static void clear_item(M11_Item* item)
{
    memset(item, 0, sizeof(*item));
}

static int item_is_empty(const M11_Item* item)
{
    return !item || item->itemType == 0;
}

static void copy_types(const M11_Item* items, int count, int* out)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_MID_CLOSE_SLOT_COUNT; ++i) {
        out[i] = (items && i < count) ? items[i].itemType : 0;
    }
}

static void copy_weights(const M11_Item* items, int count, int* out)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_MID_CLOSE_SLOT_COUNT; ++i) {
        out[i] = (items && i < count) ? items[i].weight : 0;
    }
}

static int count_visible(const M11_Item* items)
{
    int count = 0;
    int i;

    for (i = 0; i < DM1_PC34_CHEST_MID_CLOSE_SLOT_COUNT; ++i) {
        if (!item_is_empty(&items[i])) {
            ++count;
        }
    }
    return count;
}

static int contains_type(const M11_Item* items, int count, int itemType)
{
    int i;

    if (!items || count < 0) {
        return 0;
    }
    for (i = 0; i < count; ++i) {
        if (items[i].itemType == itemType) {
            return 1;
        }
    }
    return 0;
}

static int order_matches_input(const M11_Item* items)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_MID_CLOSE_SLOT_COUNT; ++i) {
        if (items[i].itemType !=
            DM1_PC34_CHEST_MID_CLOSE_FIRST_ITEM + i) {
            return 0;
        }
    }
    return 1;
}

static int closed_order_uses_current_future_slot(const M11_Item* items)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_MID_CLOSE_SLOT_COUNT; ++i) {
        int expected = DM1_PC34_CHEST_MID_CLOSE_FIRST_ITEM + i;

        if (i == DM1_PC34_CHEST_MID_CLOSE_FUTURE_INDEX) {
            expected = DM1_PC34_CHEST_MID_CLOSE_REPLACEMENT_ITEM;
        }
        if (items[i].itemType != expected) {
            return 0;
        }
    }
    return 1;
}

static int all_slots_empty(const MidCloseModelPc34* model)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_MID_CLOSE_SLOT_COUNT; ++i) {
        if (!item_is_empty(&model->chestSlots[i])) {
            return 0;
        }
    }
    return 1;
}

static int open_chest_pc34(MidCloseModelPc34* model,
                           int chestThing,
                           const M11_Item* linkedItems,
                           int linkedCount)
{
    int limit;
    int i;

    if (!model || chestThing == 0 || linkedCount < 0 ||
        (linkedCount > 0 && !linkedItems)) {
        return 0;
    }

    /* ReDMCSB CHEST.C F0333 lines 43 and 53-67 set G0426, then copy only the
     * first eight linked things into G0425_aT_ChestSlots in list order. */
    model->openChestThing = chestThing;
    for (i = 0; i < DM1_PC34_CHEST_MID_CLOSE_SLOT_COUNT; ++i) {
        clear_item(&model->chestSlots[i]);
    }
    limit = linkedCount < DM1_PC34_CHEST_MID_CLOSE_SLOT_COUNT ?
        linkedCount : DM1_PC34_CHEST_MID_CLOSE_SLOT_COUNT;
    for (i = 0; i < limit; ++i) {
        model->chestSlots[i] = linkedItems[i];
    }
    return 1;
}

static int close_begin_pc34(MidCloseModelPc34* model)
{
    if (!model || model->openChestThing == 0) {
        return 0;
    }

    /* ReDMCSB CHEST.C F0334 lines 113-117 exits if no chest is open, gets the
     * container, clears G0426, and resets the container Slot to ENDOFLIST
     * before the G0425 loop starts. */
    model->openChestThing = 0;
    model->closeStarted = 1;
    model->closeCount = 0;
    model->containerSlotClearedToEnd = 1;
    model->lastLinkedType = 0;
    memset(model->closed, 0, sizeof(model->closed));
    return 1;
}

static int close_step_pc34(MidCloseModelPc34* model, int chestSlotIndex)
{
    M11_Item thing;

    if (!model || !model->closeStarted || chestSlotIndex < 0 ||
        chestSlotIndex >= DM1_PC34_CHEST_MID_CLOSE_SLOT_COUNT) {
        return 0;
    }

    /* ReDMCSB CHEST.C F0334 lines 118-132 reads the current G0425 slot value,
     * clears that slot in newer PC34 media at lines 120-121, then either starts
     * the container list or links after the previous visible thing. */
    thing = model->chestSlots[chestSlotIndex];
    if (!item_is_empty(&thing)) {
        clear_item(&model->chestSlots[chestSlotIndex]);
        if (model->closeCount < DM1_PC34_CHEST_MID_CLOSE_SLOT_COUNT) {
            model->closed[model->closeCount] = thing;
        }
        ++model->closeCount;
        model->lastLinkedType = thing.itemType;
    }
    return 1;
}

static int click_chest_slot_pc34(MidCloseModelPc34* model,
                                 int chestSlotIndex,
                                 int* removedLeader,
                                 int* removedSlot,
                                 int* putSlotInLeader,
                                 int* putLeaderInSlot)
{
    M11_Item leaderHandObject;
    M11_Item slotObject;

    if (removedLeader) {
        *removedLeader = 0;
    }
    if (removedSlot) {
        *removedSlot = 0;
    }
    if (putSlotInLeader) {
        *putSlotInLeader = 0;
    }
    if (putLeaderInSlot) {
        *putLeaderInSlot = 0;
    }
    if (!model || chestSlotIndex < 0 ||
        chestSlotIndex >= DM1_PC34_CHEST_MID_CLOSE_SLOT_COUNT) {
        return 0;
    }

    /*
     * ReDMCSB CHAMPION.C F0302 lines 688-710 reads leader hand plus G0425,
     * rejects empty/empty and incompatible destinations, then calls F0298,
     * F0300, F0297, and F0301 in that order. This intentionally operates on
     * the current G0425 value; F0334's for-loop decides later whether that
     * slot will still be visited.
     */
    leaderHandObject = model->leaderHand;
    slotObject = model->chestSlots[chestSlotIndex];
    if (item_is_empty(&leaderHandObject) && item_is_empty(&slotObject)) {
        return 0;
    }
    if (!item_is_empty(&leaderHandObject) &&
        (leaderHandObject.allowedSlots & DM1_PC34_ALLOWED_CONTAINER) == 0) {
        return 0;
    }
    if (!item_is_empty(&leaderHandObject)) {
        clear_item(&model->leaderHand);
        if (removedLeader) {
            *removedLeader = 1;
        }
    }
    if (!item_is_empty(&slotObject)) {
        clear_item(&model->chestSlots[chestSlotIndex]);
        model->leaderHand = slotObject;
        if (removedSlot) {
            *removedSlot = 1;
        }
        if (putSlotInLeader) {
            *putSlotInLeader = 1;
        }
    }
    if (!item_is_empty(&leaderHandObject)) {
        model->chestSlots[chestSlotIndex] = leaderHandObject;
        if (putLeaderInSlot) {
            *putLeaderInSlot = 1;
        }
    }
    return 1;
}

static int close_rest_pc34(MidCloseModelPc34* model, int startIndex)
{
    int i;

    if (!model || startIndex < 0 ||
        startIndex > DM1_PC34_CHEST_MID_CLOSE_SLOT_COUNT) {
        return -1;
    }
    for (i = startIndex; i < DM1_PC34_CHEST_MID_CLOSE_SLOT_COUNT; ++i) {
        if (!close_step_pc34(model, i)) {
            return -1;
        }
    }
    model->closeStarted = 0;
    return model->closeCount;
}

const char* dm1_v1_chest_mid_close_hand_swap_source_evidence_pc34(void)
{
    return
        "CHEST.C F0333:43,53-67 selects the open chest and materializes the first eight linked objects into G0425\n"
        "CHEST.C F0334:113-117 exits when no chest is open, clears G0426, and resets the container Slot before relinking\n"
        "CHEST.C F0334:118-132 reads each current G0425 slot in ascending order, clears visited non-empty slots, and relinks visible things\n"
        "CHAMPION.C F0297:243-268/F0298:270-298 put/remove leader-hand identity and update leader load\n"
        "CHAMPION.C F0300:511-515 removes G0425 chest slots; F0301:606-614 writes G0425 chest slots and load\n"
        "CHAMPION.C F0302:688-710 reads leader hand plus destination slot, rejects invalid swaps, then performs remove/remove/put/put\n"
        "OBJECT.C F0033:147-212 supplies icon identity for F0333/F0297 drawing; this gate uses itemType sentinels, not pixel parity\n"
        "BLITMASK.C F0133:30-33 describes masked bitmap presentation; this gate is contract-only and does not assert real-asset pixels";
}

const DM1_V1_ChestMidCloseHandSwapSpecPc34*
dm1_v1_chest_mid_close_hand_swap_spec_pc34(void)
{
    return &s_spec;
}

int dm1_v1_chest_mid_close_hand_swap_run_pc34(
    DM1_V1_ChestMidCloseHandSwapProbePc34* out)
{
    MidCloseModelPc34 model;
    MidCloseModelPc34 alreadyProcessedModel;
    M11_Item linked[DM1_PC34_CHEST_MID_CLOSE_SLOT_COUNT];
    M11_Item item;
    int i;

    if (!out) {
        return 0;
    }

    memset(out, 0, sizeof(*out));
    memset(&model, 0, sizeof(model));
    memset(&alreadyProcessedModel, 0, sizeof(alreadyProcessedModel));

    out->contractOnly = 1;
    for (i = 0; i < DM1_PC34_CHEST_MID_CLOSE_SLOT_COUNT; ++i) {
        linked[i] = make_item(DM1_PC34_CHEST_MID_CLOSE_FIRST_ITEM + i,
                              4 + i);
    }

    out->openResult = open_chest_pc34(
        &model, DM1_PC34_CHEST_MID_CLOSE_CHEST_THING, linked,
        DM1_PC34_CHEST_MID_CLOSE_SLOT_COUNT);
    out->openThing = model.openChestThing;
    out->openVisibleCount = count_visible(model.chestSlots);
    copy_types(model.chestSlots, DM1_PC34_CHEST_MID_CLOSE_SLOT_COUNT,
               out->openedTypes);
    copy_weights(model.chestSlots, DM1_PC34_CHEST_MID_CLOSE_SLOT_COUNT,
                 out->openedWeights);
    out->openedOrderMatchesInput = order_matches_input(model.chestSlots);
    out->objectIconPathCited = 1;
    out->blitMaskIsPresentationOnly = 1;

    model.leaderHand = make_item(DM1_PC34_CHEST_MID_CLOSE_REPLACEMENT_ITEM,
                                 29);
    out->leaderHandBeforeCloseType = model.leaderHand.itemType;
    out->leaderHandBeforeCloseWeight = model.leaderHand.weight;
    out->leaderHandBeforeCloseAllowedSlots = model.leaderHand.allowedSlots;
    out->replacementCanEnterChest =
        (model.leaderHand.allowedSlots & DM1_PC34_ALLOWED_CONTAINER) != 0;

    out->openThingBeforeCloseBegin = model.openChestThing;
    out->closeBeginResult = close_begin_pc34(&model);
    out->openThingAfterCloseBegin = model.openChestThing;
    out->containerSlotClearedToEnd = model.containerSlotClearedToEnd;
    for (i = 0; i < DM1_PC34_CHEST_MID_CLOSE_PREFIX_COUNT; ++i) {
        if (!close_step_pc34(&model, i)) {
            return 0;
        }
        out->prefixSlotsCleared[i] = item_is_empty(&model.chestSlots[i]);
    }
    out->prefixProcessedCount = DM1_PC34_CHEST_MID_CLOSE_PREFIX_COUNT;
    out->prefixClosedCount = model.closeCount;
    copy_types(model.closed, DM1_PC34_CHEST_MID_CLOSE_SLOT_COUNT,
               out->prefixClosedTypes);
    out->prefixLastLinkedType = model.lastLinkedType;
    out->futureSlotBeforeSwapType =
        model.chestSlots[DM1_PC34_CHEST_MID_CLOSE_FUTURE_INDEX].itemType;
    out->futureSlotBeforeSwapWeight =
        model.chestSlots[DM1_PC34_CHEST_MID_CLOSE_FUTURE_INDEX].weight;

    out->midSwapPc34Slot =
        DM1_PC34_SLOT_CHEST_1 + DM1_PC34_CHEST_MID_CLOSE_FUTURE_INDEX;
    out->midSwapLeaderBeforeType = model.leaderHand.itemType;
    out->midSwapSlotBeforeType =
        model.chestSlots[DM1_PC34_CHEST_MID_CLOSE_FUTURE_INDEX].itemType;
    out->midSwapResult = click_chest_slot_pc34(
        &model, DM1_PC34_CHEST_MID_CLOSE_FUTURE_INDEX,
        &out->midSwapLeaderRemovedCall, &out->midSwapSlotRemovedCall,
        &out->midSwapPutDisplacedInLeaderCall,
        &out->midSwapPutReplacementInSlotCall);
    out->midSwapLeaderAfterType = model.leaderHand.itemType;
    out->midSwapSlotAfterType =
        model.chestSlots[DM1_PC34_CHEST_MID_CLOSE_FUTURE_INDEX].itemType;
    out->futureOriginalAbsentFromSlotAfterSwap =
        out->midSwapSlotAfterType != out->futureSlotBeforeSwapType;

    out->closeCount = close_rest_pc34(
        &model, DM1_PC34_CHEST_MID_CLOSE_PREFIX_COUNT);
    copy_types(model.closed, DM1_PC34_CHEST_MID_CLOSE_SLOT_COUNT,
               out->closedTypes);
    copy_weights(model.closed, DM1_PC34_CHEST_MID_CLOSE_SLOT_COUNT,
                 out->closedWeights);
    out->replacementLinkedAtFuturePosition =
        model.closed[DM1_PC34_CHEST_MID_CLOSE_FUTURE_INDEX].itemType ==
        DM1_PC34_CHEST_MID_CLOSE_REPLACEMENT_ITEM;
    out->displacedAbsentFromClosedLinks =
        !contains_type(model.closed, model.closeCount,
                       DM1_PC34_CHEST_MID_CLOSE_FIRST_ITEM +
                       DM1_PC34_CHEST_MID_CLOSE_FUTURE_INDEX);
    out->closedOrderUsesCurrentFutureSlot =
        closed_order_uses_current_future_slot(model.closed);
    out->futureSlotClearedByClose =
        item_is_empty(&model.chestSlots[DM1_PC34_CHEST_MID_CLOSE_FUTURE_INDEX]);
    out->allSlotsEmptyAfterClose = all_slots_empty(&model);
    out->leaderHandAfterCloseType = model.leaderHand.itemType;
    out->leaderHandAfterCloseWeight = model.leaderHand.weight;

    out->reopenResult = open_chest_pc34(
        &model, DM1_PC34_CHEST_MID_CLOSE_REOPEN_THING, model.closed,
        model.closeCount);
    out->reopenThing = model.openChestThing;
    out->reopenedVisibleCount = count_visible(model.chestSlots);
    copy_types(model.chestSlots, DM1_PC34_CHEST_MID_CLOSE_SLOT_COUNT,
               out->reopenedTypes);
    out->reopenedOrderMatchesClosed =
        memcmp(model.chestSlots, model.closed, sizeof(model.closed)) == 0;
    out->replacementVisibleAfterReopen =
        contains_type(model.chestSlots, DM1_PC34_CHEST_MID_CLOSE_SLOT_COUNT,
                      DM1_PC34_CHEST_MID_CLOSE_REPLACEMENT_ITEM);
    out->displacedAbsentAfterReopen =
        !contains_type(model.chestSlots, DM1_PC34_CHEST_MID_CLOSE_SLOT_COUNT,
                       DM1_PC34_CHEST_MID_CLOSE_FIRST_ITEM +
                       DM1_PC34_CHEST_MID_CLOSE_FUTURE_INDEX);

    if (!open_chest_pc34(&alreadyProcessedModel,
                         DM1_PC34_CHEST_MID_CLOSE_CHEST_THING, linked,
                         DM1_PC34_CHEST_MID_CLOSE_SLOT_COUNT)) {
        return 0;
    }
    alreadyProcessedModel.leaderHand =
        make_item(DM1_PC34_CHEST_MID_CLOSE_REPLACEMENT_ITEM, 29);
    if (!close_begin_pc34(&alreadyProcessedModel)) {
        return 0;
    }
    for (i = 0; i <= DM1_PC34_CHEST_MID_CLOSE_ALREADY_PROCESSED_INDEX; ++i) {
        if (!close_step_pc34(&alreadyProcessedModel, i)) {
            return 0;
        }
    }
    out->alreadyProcessedPc34Slot =
        DM1_PC34_SLOT_CHEST_1 +
        DM1_PC34_CHEST_MID_CLOSE_ALREADY_PROCESSED_INDEX;
    out->alreadyProcessedLeaderBeforeType =
        alreadyProcessedModel.leaderHand.itemType;
    out->alreadyProcessedSlotBeforeLateWriteType =
        alreadyProcessedModel
            .chestSlots[DM1_PC34_CHEST_MID_CLOSE_ALREADY_PROCESSED_INDEX]
            .itemType;
    out->alreadyProcessedSwapResult = click_chest_slot_pc34(
        &alreadyProcessedModel,
        DM1_PC34_CHEST_MID_CLOSE_ALREADY_PROCESSED_INDEX,
        0, 0, 0, 0);
    out->alreadyProcessedSlotAfterLateWriteType =
        alreadyProcessedModel
            .chestSlots[DM1_PC34_CHEST_MID_CLOSE_ALREADY_PROCESSED_INDEX]
            .itemType;
    out->alreadyProcessedCloseCount = close_rest_pc34(
        &alreadyProcessedModel,
        DM1_PC34_CHEST_MID_CLOSE_ALREADY_PROCESSED_INDEX + 1);
    out->alreadyProcessedReplacementLinked =
        contains_type(alreadyProcessedModel.closed,
                      alreadyProcessedModel.closeCount,
                      DM1_PC34_CHEST_MID_CLOSE_REPLACEMENT_ITEM);
    out->alreadyProcessedLateWriteNotRevisited =
        !out->alreadyProcessedReplacementLinked;
    out->alreadyProcessedStaleSlotAfterCloseType =
        alreadyProcessedModel
            .chestSlots[DM1_PC34_CHEST_MID_CLOSE_ALREADY_PROCESSED_INDEX]
            .itemType;
    item = alreadyProcessedModel.leaderHand;
    out->alreadyProcessedLeaderAfterCloseType = item.itemType;

    return 1;
}
