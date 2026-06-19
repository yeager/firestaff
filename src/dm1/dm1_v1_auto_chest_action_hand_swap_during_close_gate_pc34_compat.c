#include "dm1_v1_auto_chest_action_hand_swap_during_close_gate_pc34_compat.h"

#include <string.h>

/*
 * Slice choice/non-overlap:
 * tests/test_dm1_v1_chest_mid_close_hand_swap_pc34_compat.c covers a manual
 * F0333 open (C145 blit path) + mid-close hand swap driven from
 * DM1_PC34_SLOT_CHEST_* + DM1_PC34_SLOT_READY_HAND. pass797 isolates the
 * press-eye auto-close gate (F0333 with P0694_B_PressingEye = TRUE) plus
 * the leader's C09 action hand (DM1_PC34_SLOT_ACTION_HAND) as the swap
 * source. It also pins the F0334 already-visited-slot late-write contract
 * and the press-eye replay contract.
 */

typedef struct {
    M11_Item chestSlots[DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT];
    M11_Item actionHand;
    M11_Item closed[DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT];
    int openChestThing;
    int closeStarted;
    int closeCount;
    int containerSlotClearedToEnd;
    int containerSlotEndOfListTerminator;
    int closedEndOfListTerminator;
    int lastLinkedType;
} AutoCloseGateModelPc34;

static const DM1_V1_AutoChestActionHandSwapDuringCloseGateSpecPc34 s_spec = {
    "Source-locked deterministic contract gate only; no real-asset pixel or icon parity claim.",
    "Non-duplicative: pass797 isolates the press-eye F0333 auto-close gate (skips the C145 blit) plus the C09 action hand as the swap source. The mid-close hand swap test in chest_mid_close_hand_swap_pc34_compat uses the manual F0333 open + ready-hand slot-box swap; pass797 covers the press-eye path with action-hand slot-box swap, F0334 visited-slot late-write, and end-of-list terminator placement.",
    1,
    DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT,
    DM1_PC34_AUTO_CHEST_ACTION_HAND_PREFIX_COUNT,
    DM1_PC34_AUTO_CHEST_ACTION_HAND_FUTURE_INDEX,
    DM1_PC34_SLOT_CHEST_1 +
        DM1_PC34_AUTO_CHEST_ACTION_HAND_FUTURE_INDEX,
    DM1_PC34_AUTO_CHEST_ACTION_HAND_ALREADY_PROCESSED_INDEX,
    DM1_PC34_SLOT_CHEST_1 +
        DM1_PC34_AUTO_CHEST_ACTION_HAND_ALREADY_PROCESSED_INDEX,
    DM1_PC34_AUTO_CHEST_ACTION_HAND_CHEST_THING,
    DM1_PC34_AUTO_CHEST_ACTION_HAND_FIRST_ITEM,
    DM1_PC34_AUTO_CHEST_ACTION_HAND_FIRST_ITEM +
        DM1_PC34_AUTO_CHEST_ACTION_HAND_FUTURE_INDEX,
    DM1_PC34_AUTO_CHEST_ACTION_HAND_REPLACEMENT_CONTAINER,
    1,
    DM1_PC34_AUTO_CHEST_ACTION_HAND_ENDOFLIST,
    DM1_PC34_AUTO_CHEST_ACTION_HAND_NONE
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

static void clear_item(M11_Item* item)
{
    if (item) {
        memset(item, 0, sizeof(*item));
    }
}

static int item_is_empty(const M11_Item* item)
{
    return !item || item->itemType == 0;
}

static void copy_types(const M11_Item* items, int count, int* out)
{
    int i;

    for (i = 0; i < DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT; ++i) {
        out[i] = (items && i < count) ? items[i].itemType : 0;
    }
}

static void copy_weights(const M11_Item* items, int count, int* out)
{
    int i;

    for (i = 0; i < DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT; ++i) {
        out[i] = (items && i < count) ? items[i].weight : 0;
    }
}

static int count_visible(const M11_Item* items)
{
    int count = 0;
    int i;

    for (i = 0; i < DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT; ++i) {
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

    for (i = 0; i < DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT; ++i) {
        if (items[i].itemType !=
            DM1_PC34_AUTO_CHEST_ACTION_HAND_FIRST_ITEM + i) {
            return 0;
        }
    }
    return 1;
}

static int closed_order_uses_current_future_slot(const M11_Item* items)
{
    int i;

    for (i = 0; i < DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT; ++i) {
        int expected = DM1_PC34_AUTO_CHEST_ACTION_HAND_FIRST_ITEM + i;

        if (i == DM1_PC34_AUTO_CHEST_ACTION_HAND_FUTURE_INDEX) {
            expected = DM1_PC34_AUTO_CHEST_ACTION_HAND_REPLACEMENT_CONTAINER;
        }
        if (items[i].itemType != expected) {
            return 0;
        }
    }
    return 1;
}

static int all_slots_empty(const AutoCloseGateModelPc34* model)
{
    int i;

    for (i = 0; i < DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT; ++i) {
        if (!item_is_empty(&model->chestSlots[i])) {
            return 0;
        }
    }
    return 1;
}

/*
 * Press-eye open path. ReDMCSB CHEST.C F0333 lines 32-42 set G0426 and
 * (only when P0694_B_PressingEye is FALSE) draw the C145_ICON_CONTAINER
 * _CHEST_OPEN icon. Lines 53-67 then copy the first eight linked things
 * into G0425_aT_ChestSlots in list order. Lines 70-74 fill remaining
 * slots with C0xFFFF_THING_NONE.
 */
static int press_eye_open_pc34(AutoCloseGateModelPc34* model,
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

    model->openChestThing = chestThing;
    for (i = 0; i < DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT; ++i) {
        clear_item(&model->chestSlots[i]);
    }
    limit = linkedCount < DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT ?
        linkedCount : DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT;
    for (i = 0; i < limit; ++i) {
        model->chestSlots[i] = linkedItems[i];
    }
    return 1;
}

/*
 * F0334_INVENTORY_CloseChest begin. ReDMCSB CHEST.C F0334 lines 113-117
 * exit if no chest is open, clear G0426, and reset the container Slot
 * to C0xFFFE_THING_ENDOFLIST. We additionally assert that
 * containerSlotEndOfListTerminator is set to C0xFFFE so the close-loop
 * list-head is well-formed for F0163_DUNGEON_LinkThingToList on the
 * first non-empty slot.
 */
static int close_begin_pc34(AutoCloseGateModelPc34* model)
{
    if (!model || model->openChestThing == 0) {
        return 0;
    }

    model->openChestThing = 0;
    model->closeStarted = 1;
    model->closeCount = 0;
    model->containerSlotClearedToEnd = 1;
    model->containerSlotEndOfListTerminator =
        DM1_PC34_AUTO_CHEST_ACTION_HAND_ENDOFLIST;
    model->closedEndOfListTerminator = 0;
    model->lastLinkedType = 0;
    memset(model->closed, 0, sizeof(model->closed));
    return 1;
}

/*
 * F0334 close-step for one G0425 slot. ReDMCSB CHEST.C F0334 lines 118-132
 * read the current G0425 slot value, clear that slot in PC34 media at
 * 120-121, then either start the container list (line 124-128, the
 * L1026_B_ProcessFirstChestSlot branch) or link after the previous
 * visible thing via F0163_DUNGEON_LinkThingToList (line 130-131).
 */
static int close_step_pc34(AutoCloseGateModelPc34* model, int chestSlotIndex)
{
    M11_Item thing;

    if (!model || !model->closeStarted || chestSlotIndex < 0 ||
        chestSlotIndex >= DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT) {
        return 0;
    }

    thing = model->chestSlots[chestSlotIndex];
    if (!item_is_empty(&thing)) {
        clear_item(&model->chestSlots[chestSlotIndex]);
        if (model->closeCount < DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT) {
            model->closed[model->closeCount] = thing;
        }
        ++model->closeCount;
        model->lastLinkedType = thing.itemType;
    }
    return 1;
}

/*
 * C09 action-hand click on a chest slot. ReDMCSB CHAMPION.C F0302 lines
 * 688-710 reads the leader action hand (DM1_PC34_SLOT_ACTION_HAND) plus
 * the destination G0425 slot, rejects empty/empty and incompatible
 * destinations, then performs the F0298/F0300/F0297/F0301
 * remove/remove/put/put sequence.
 */
static int click_action_hand_on_chest_slot_pc34(
    AutoCloseGateModelPc34* model,
    int chestSlotIndex,
    int* removedLeader,
    int* removedSlot,
    int* putSlotInLeader,
    int* putLeaderInSlot)
{
    M11_Item actionHandObject;
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
        chestSlotIndex >= DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT) {
        return 0;
    }

    actionHandObject = model->actionHand;
    slotObject = model->chestSlots[chestSlotIndex];
    if (item_is_empty(&actionHandObject) && item_is_empty(&slotObject)) {
        return 0;
    }
    if (!item_is_empty(&actionHandObject) &&
        (actionHandObject.allowedSlots & DM1_PC34_ALLOWED_CONTAINER) == 0) {
        return 0;
    }
    if (!item_is_empty(&actionHandObject)) {
        clear_item(&model->actionHand);
        if (removedLeader) {
            *removedLeader = 1;
        }
    }
    if (!item_is_empty(&slotObject)) {
        clear_item(&model->chestSlots[chestSlotIndex]);
        model->actionHand = slotObject;
        if (removedSlot) {
            *removedSlot = 1;
        }
        if (putSlotInLeader) {
            *putSlotInLeader = 1;
        }
    }
    if (!item_is_empty(&actionHandObject)) {
        model->chestSlots[chestSlotIndex] = actionHandObject;
        if (putLeaderInSlot) {
            *putLeaderInSlot = 1;
        }
    }
    return 1;
}

static int close_rest_pc34(AutoCloseGateModelPc34* model, int startIndex)
{
    int i;

    if (!model || startIndex < 0 ||
        startIndex > DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT) {
        return -1;
    }
    for (i = startIndex; i < DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT; ++i) {
        if (!close_step_pc34(model, i)) {
            return -1;
        }
    }
    model->closeStarted = 0;
    /*
     * ReDMCSB CHEST.C F0334 line 128 writes C0xFFFE_THING_ENDOFLIST into
     * L1028_ps_Generic->Next for the first linked thing; line 130-131
     * uses F0163_DUNGEON_LinkThingToList which itself writes ENDOFLIST
     * on the tail. Once the for-loop has visited all eight slots, the
     * container list's tail is always ENDOFLIST.
     */
    model->closedEndOfListTerminator =
        DM1_PC34_AUTO_CHEST_ACTION_HAND_ENDOFLIST;
    return model->closeCount;
}

const char*
dm1_v1_auto_chest_action_hand_swap_during_close_gate_source_evidence_pc34(void)
{
    return
        "CHEST.C F0333:32-42 P0694_B_PressingEye controls the C145_ICON_CONTAINER_CHEST_OPEN blit (line 44) and selects the open chest (line 38-39)\n"
        "CHEST.C F0333:53-67 copies the first eight linked things into G0425_aT_ChestSlots in list order\n"
        "CHEST.C F0333:70-74 fills remaining G0425 slots with C0xFFFF_THING_NONE\n"
        "CHEST.C F0334:113-117 exits when no chest is open, clears G0426, and resets the container Slot to C0xFFFE_THING_ENDOFLIST\n"
        "CHEST.C F0334:118-132 reads each current G0425 slot in ascending order, clears visited non-empty slots, and relinks visible things via L1026_B_ProcessFirstChestSlot + F0163_DUNGEON_LinkThingToList\n"
        "CHAMPION.C F0297:243-268/F0298:270-298 put/remove leader-hand identity and update leader load\n"
        "CHAMPION.C F0300:511-515 removes G0425 chest slots; F0301:606-614 writes G0425 chest slots and load\n"
        "CHAMPION.C F0302:688-710 reads leader hand (DM1_PC34_SLOT_ACTION_HAND) plus destination slot, rejects invalid swaps, then performs remove/remove/put/put\n"
        "PANEL.C F0347:1639-1691 routes C09_SLOT_BOX_INVENTORY_ACTION_HAND click to F0302 for the action-hand container swap\n"
        "OBJECT.C F0033:147-212 supplies icon identity for F0333/F0297 drawing; this gate uses itemType sentinels, not pixel parity\n"
        "BLITMASK.C F0133:30-33 describes masked bitmap presentation; this gate is contract-only and does not assert real-asset pixels";
}

const DM1_V1_AutoChestActionHandSwapDuringCloseGateSpecPc34*
dm1_v1_auto_chest_action_hand_swap_during_close_gate_spec_pc34(void)
{
    return &s_spec;
}

int dm1_v1_auto_chest_action_hand_swap_during_close_gate_run_pc34(
    DM1_V1_AutoChestActionHandSwapDuringCloseGateProbePc34* out)
{
    AutoCloseGateModelPc34 model;
    AutoCloseGateModelPc34 alreadyProcessedModel;
    AutoCloseGateModelPc34 replayModel;
    M11_Item linked[DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT];
    M11_Item item;
    int i;

    if (!out) {
        return 0;
    }

    memset(out, 0, sizeof(*out));
    memset(&model, 0, sizeof(model));
    memset(&alreadyProcessedModel, 0, sizeof(alreadyProcessedModel));
    memset(&replayModel, 0, sizeof(replayModel));

    out->contractOnly = 1;
    out->pressEyePathEnabled = 1;

    for (i = 0; i < DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT; ++i) {
        linked[i] = make_item(
            DM1_PC34_AUTO_CHEST_ACTION_HAND_FIRST_ITEM + i,
            4 + i,
            DM1_PC34_ALLOWED_CONTAINER);
    }

    /* F0333 press-eye open: sets G0426, skips C145 blit, populates G0425. */
    out->openResult = press_eye_open_pc34(
        &model, DM1_PC34_AUTO_CHEST_ACTION_HAND_CHEST_THING, linked,
        DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT);
    out->openThing = model.openChestThing;
    out->openVisibleCount = count_visible(model.chestSlots);
    copy_types(model.chestSlots, DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT,
               out->openedTypes);
    copy_weights(model.chestSlots, DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT,
                 out->openedWeights);
    out->openedOrderMatchesInput = order_matches_input(model.chestSlots);
    out->pressEyeIconBlitSkipped = 1;

    /* Leader C09 (DM1_PC34_SLOT_ACTION_HAND) already holds the
     * replacement container before F0334 begins. */
    model.actionHand = make_item(
        DM1_PC34_AUTO_CHEST_ACTION_HAND_REPLACEMENT_CONTAINER, 29,
        DM1_PC34_ALLOWED_CONTAINER);
    out->actionHandBeforeCloseType = model.actionHand.itemType;
    out->actionHandBeforeCloseWeight = model.actionHand.weight;
    out->actionHandBeforeCloseAllowedSlots = model.actionHand.allowedSlots;
    out->replacementCanEnterChest =
        (model.actionHand.allowedSlots & DM1_PC34_ALLOWED_CONTAINER) != 0;

    /* F0334 begin: clears G0426, resets container Slot to ENDOFLIST. */
    out->openThingBeforeCloseBegin = model.openChestThing;
    out->closeBeginResult = close_begin_pc34(&model);
    out->openThingAfterCloseBegin = model.openChestThing;
    out->containerSlotClearedToEnd = model.containerSlotClearedToEnd;
    out->containerSlotEndOfListTerminator =
        model.containerSlotEndOfListTerminator;

    /* F0334 close prefix. */
    for (i = 0; i < DM1_PC34_AUTO_CHEST_ACTION_HAND_PREFIX_COUNT; ++i) {
        if (!close_step_pc34(&model, i)) {
            return 0;
        }
        out->prefixSlotsCleared[i] = item_is_empty(&model.chestSlots[i]);
    }
    out->prefixProcessedCount = DM1_PC34_AUTO_CHEST_ACTION_HAND_PREFIX_COUNT;
    out->prefixClosedCount = model.closeCount;
    copy_types(model.closed, DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT,
               out->prefixClosedTypes);
    out->prefixLastLinkedType = model.lastLinkedType;
    out->prefixEndOfListTerminator = 0; /* not yet terminated */

    /* C09 action-hand click on the future G0425 slot mid-close. */
    out->midSwapPc34Slot = DM1_PC34_SLOT_ACTION_HAND;
    out->midSwapDestinationPc34Slot =
        DM1_PC34_SLOT_CHEST_1 +
        DM1_PC34_AUTO_CHEST_ACTION_HAND_FUTURE_INDEX;
    out->midSwapLeaderBeforeType = model.actionHand.itemType;
    out->midSwapSlotBeforeType =
        model.chestSlots[DM1_PC34_AUTO_CHEST_ACTION_HAND_FUTURE_INDEX]
            .itemType;
    out->midSwapResult = click_action_hand_on_chest_slot_pc34(
        &model, DM1_PC34_AUTO_CHEST_ACTION_HAND_FUTURE_INDEX,
        &out->midSwapLeaderRemovedCall, &out->midSwapSlotRemovedCall,
        &out->midSwapPutDisplacedInLeaderCall,
        &out->midSwapPutReplacementInSlotCall);
    out->midSwapLeaderAfterType = model.actionHand.itemType;
    out->midSwapSlotAfterType =
        model.chestSlots[DM1_PC34_AUTO_CHEST_ACTION_HAND_FUTURE_INDEX]
            .itemType;
    out->futureOriginalAbsentFromSlotAfterSwap =
        out->midSwapSlotAfterType !=
        (DM1_PC34_AUTO_CHEST_ACTION_HAND_FIRST_ITEM +
         DM1_PC34_AUTO_CHEST_ACTION_HAND_FUTURE_INDEX);
    out->actionHandBecomesEmptyAfterSwap =
        item_is_empty(&model.actionHand);

    /* F0334 close remainder. */
    out->closeCount = close_rest_pc34(
        &model, DM1_PC34_AUTO_CHEST_ACTION_HAND_PREFIX_COUNT);
    copy_types(model.closed, DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT,
               out->closedTypes);
    copy_weights(model.closed, DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT,
                 out->closedWeights);
    out->replacementLinkedAtFuturePosition =
        model.closed[DM1_PC34_AUTO_CHEST_ACTION_HAND_FUTURE_INDEX]
            .itemType ==
        DM1_PC34_AUTO_CHEST_ACTION_HAND_REPLACEMENT_CONTAINER;
    out->displacedAbsentFromClosedLinks =
        !contains_type(model.closed, model.closeCount,
                       DM1_PC34_AUTO_CHEST_ACTION_HAND_FIRST_ITEM +
                       DM1_PC34_AUTO_CHEST_ACTION_HAND_FUTURE_INDEX);
    out->closedOrderUsesCurrentFutureSlot =
        closed_order_uses_current_future_slot(model.closed);
    out->futureSlotClearedByClose =
        item_is_empty(
            &model.chestSlots[DM1_PC34_AUTO_CHEST_ACTION_HAND_FUTURE_INDEX]);
    out->allSlotsEmptyAfterClose = all_slots_empty(&model);
    out->actionHandAfterCloseType = model.actionHand.itemType;
    out->actionHandAfterCloseWeight = model.actionHand.weight;
    out->closedEndOfListTerminator = model.closedEndOfListTerminator;

    /* Reopen via F0333 press-eye again. */
    out->reopenResult = press_eye_open_pc34(
        &model, DM1_PC34_AUTO_CHEST_ACTION_HAND_REOPEN_THING,
        model.closed, model.closeCount);
    out->reopenThing = model.openChestThing;
    out->reopenedVisibleCount = count_visible(model.chestSlots);
    copy_types(model.chestSlots, DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT,
               out->reopenedTypes);
    out->reopenedOrderMatchesClosed =
        memcmp(model.chestSlots, model.closed, sizeof(model.closed)) == 0;
    out->replacementVisibleAfterReopen =
        contains_type(model.chestSlots,
                      DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT,
                      DM1_PC34_AUTO_CHEST_ACTION_HAND_REPLACEMENT_CONTAINER);
    out->displacedAbsentAfterReopen =
        !contains_type(model.chestSlots,
                       DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT,
                       DM1_PC34_AUTO_CHEST_ACTION_HAND_FIRST_ITEM +
                       DM1_PC34_AUTO_CHEST_ACTION_HAND_FUTURE_INDEX);

    /* F0334 already-processed slot late-write contract. */
    if (!press_eye_open_pc34(&alreadyProcessedModel,
                             DM1_PC34_AUTO_CHEST_ACTION_HAND_CHEST_THING,
                             linked,
                             DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT)) {
        return 0;
    }
    alreadyProcessedModel.actionHand = make_item(
        DM1_PC34_AUTO_CHEST_ACTION_HAND_REPLACEMENT_CONTAINER, 29,
        DM1_PC34_ALLOWED_CONTAINER);
    if (!close_begin_pc34(&alreadyProcessedModel)) {
        return 0;
    }
    for (i = 0; i <=
         DM1_PC34_AUTO_CHEST_ACTION_HAND_ALREADY_PROCESSED_INDEX; ++i) {
        if (!close_step_pc34(&alreadyProcessedModel, i)) {
            return 0;
        }
    }
    out->alreadyProcessedPc34Slot =
        DM1_PC34_SLOT_CHEST_1 +
        DM1_PC34_AUTO_CHEST_ACTION_HAND_ALREADY_PROCESSED_INDEX;
    out->alreadyProcessedLeaderBeforeType =
        alreadyProcessedModel.actionHand.itemType;
    out->alreadyProcessedSlotBeforeLateWriteType =
        alreadyProcessedModel
            .chestSlots
                [DM1_PC34_AUTO_CHEST_ACTION_HAND_ALREADY_PROCESSED_INDEX]
            .itemType;
    out->alreadyProcessedSwapResult = click_action_hand_on_chest_slot_pc34(
        &alreadyProcessedModel,
        DM1_PC34_AUTO_CHEST_ACTION_HAND_ALREADY_PROCESSED_INDEX,
        0, 0, 0, 0);
    out->alreadyProcessedSlotAfterLateWriteType =
        alreadyProcessedModel
            .chestSlots
                [DM1_PC34_AUTO_CHEST_ACTION_HAND_ALREADY_PROCESSED_INDEX]
            .itemType;
    out->alreadyProcessedCloseCount = close_rest_pc34(
        &alreadyProcessedModel,
        DM1_PC34_AUTO_CHEST_ACTION_HAND_ALREADY_PROCESSED_INDEX + 1);
    out->alreadyProcessedReplacementLinked =
        contains_type(alreadyProcessedModel.closed,
                      alreadyProcessedModel.closeCount,
                      DM1_PC34_AUTO_CHEST_ACTION_HAND_REPLACEMENT_CONTAINER);
    out->alreadyProcessedLateWriteNotRevisited =
        !out->alreadyProcessedReplacementLinked;
    out->alreadyProcessedStaleSlotAfterCloseType =
        alreadyProcessedModel
            .chestSlots
                [DM1_PC34_AUTO_CHEST_ACTION_HAND_ALREADY_PROCESSED_INDEX]
            .itemType;
    item = alreadyProcessedModel.actionHand;
    out->alreadyProcessedLeaderAfterCloseType = item.itemType;

    /* Press-eye replay: a second press-eye open + close, with the action
     * hand already holding a container, must complete without F0334
     * short-circuiting on the existing open chest (G0426 is already
     * cleared at this point because the previous close has run). */
    if (!press_eye_open_pc34(&replayModel,
                             DM1_PC34_AUTO_CHEST_ACTION_HAND_REOPEN_THING,
                             model.closed, model.closeCount)) {
        return 0;
    }
    replayModel.actionHand = make_item(
        DM1_PC34_AUTO_CHEST_ACTION_HAND_REPLACEMENT_CONTAINER, 29,
        DM1_PC34_ALLOWED_CONTAINER);
    out->secondPressEyeOpenResult = 1;
    out->secondPressEyeOpenThing = replayModel.openChestThing;
    out->secondActionHandBeforeCloseType = replayModel.actionHand.itemType;
    out->secondCloseBeginResult = close_begin_pc34(&replayModel);
    out->secondContainerSlotClearedToEnd =
        replayModel.containerSlotClearedToEnd;
    out->secondCloseCount = close_rest_pc34(&replayModel, 0);
    out->secondClosedEndOfListTerminator =
        replayModel.closedEndOfListTerminator;

    return 1;
}
