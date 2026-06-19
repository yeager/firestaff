#include "dm1_v1_chest_open_stack_split_press_eye_pc34_compat.h"

#include <string.h>

/*
 * Slice choice/non-overlap:
 * pass797 covers the press-eye F0333 auto-close gate + C09 action-hand
 * PUT-replacement (replacement container dropped into a not-yet-visited
 * slot mid-close). pass799 covers the press-eye F0333 auto-close gate +
 * C09 action-hand PICKUP-from-stack (one stack member picked up
 * mid-close, the remaining stack member is linked in close-order; the
 * closed list is one shorter because the picked item is now in C09,
 * not in the container Slot list).
 */

typedef struct {
    M11_Item chestSlots[DM1_PC34_CHEST_OPEN_STACK_SPLIT_SLOT_COUNT];
    M11_Item actionHand;
    M11_Item closed[DM1_PC34_CHEST_OPEN_STACK_SPLIT_SLOT_COUNT];
    int openChestThing;
    int closeStarted;
    int closeCount;
    int containerSlotClearedToEnd;
    int containerSlotEndOfListTerminator;
    int closedEndOfListTerminator;
    int lastLinkedType;
} StackSplitModelPc34;

static const DM1_V1_ChestOpenStackSplitPressEyeSpecPc34 s_spec = {
    "Source-locked deterministic contract gate only; no real-asset pixel or icon parity claim.",
    "Non-duplicative: pass799 isolates the press-eye F0333 auto-close gate + C09 action-hand PICKUP-from-stack scenario. pass797 covers the press-eye + C09 PUT-replacement scenario; pass799 is the pick counterpart — the closed list is one shorter, the stack pattern is preserved (one stack member linked, the other absent), and the action hand holds the picked item after close.",
    1,
    DM1_PC34_CHEST_OPEN_STACK_SPLIT_SLOT_COUNT,
    DM1_PC34_CHEST_OPEN_STACK_SPLIT_PREFIX_COUNT,
    DM1_PC34_CHEST_OPEN_STACK_SPLIT_STACK_INDEX,
    DM1_PC34_SLOT_CHEST_1 +
        DM1_PC34_CHEST_OPEN_STACK_SPLIT_STACK_INDEX,
    DM1_PC34_CHEST_OPEN_STACK_SPLIT_PICKUP_INDEX,
    DM1_PC34_SLOT_CHEST_1 +
        DM1_PC34_CHEST_OPEN_STACK_SPLIT_PICKUP_INDEX,
    DM1_PC34_CHEST_OPEN_STACK_SPLIT_CHEST_THING,
    DM1_PC34_CHEST_OPEN_STACK_SPLIT_FIRST_ITEM,
    DM1_PC34_CHEST_OPEN_STACK_SPLIT_STACK_A,
    DM1_PC34_CHEST_OPEN_STACK_SPLIT_STACK_B,
    1,
    DM1_PC34_CHEST_OPEN_STACK_SPLIT_ENDOFLIST,
    DM1_PC34_CHEST_OPEN_STACK_SPLIT_NONE
};

static M11_Item make_item(int itemType, int weight)
{
    M11_Item item;

    memset(&item, 0, sizeof(item));
    item.itemType = itemType;
    item.weight = weight;
    item.identified = 1;
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

    for (i = 0; i < DM1_PC34_CHEST_OPEN_STACK_SPLIT_SLOT_COUNT; ++i) {
        out[i] = (items && i < count) ? items[i].itemType : 0;
    }
}

static void copy_weights(const M11_Item* items, int count, int* out)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_OPEN_STACK_SPLIT_SLOT_COUNT; ++i) {
        out[i] = (items && i < count) ? items[i].weight : 0;
    }
}

static int count_visible(const M11_Item* items)
{
    int count = 0;
    int i;

    for (i = 0; i < DM1_PC34_CHEST_OPEN_STACK_SPLIT_SLOT_COUNT; ++i) {
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

static int all_slots_empty(const StackSplitModelPc34* model)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_OPEN_STACK_SPLIT_SLOT_COUNT; ++i) {
        if (!item_is_empty(&model->chestSlots[i])) {
            return 0;
        }
    }
    return 1;
}

static int press_eye_open_pc34(StackSplitModelPc34* model,
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
    for (i = 0; i < DM1_PC34_CHEST_OPEN_STACK_SPLIT_SLOT_COUNT; ++i) {
        clear_item(&model->chestSlots[i]);
    }
    limit = linkedCount < DM1_PC34_CHEST_OPEN_STACK_SPLIT_SLOT_COUNT ?
        linkedCount : DM1_PC34_CHEST_OPEN_STACK_SPLIT_SLOT_COUNT;
    for (i = 0; i < limit; ++i) {
        model->chestSlots[i] = linkedItems[i];
    }
    return 1;
}

static int close_begin_pc34(StackSplitModelPc34* model)
{
    if (!model || model->openChestThing == 0) {
        return 0;
    }

    model->openChestThing = 0;
    model->closeStarted = 1;
    model->closeCount = 0;
    model->containerSlotClearedToEnd = 1;
    model->containerSlotEndOfListTerminator =
        DM1_PC34_CHEST_OPEN_STACK_SPLIT_ENDOFLIST;
    model->closedEndOfListTerminator = 0;
    model->lastLinkedType = 0;
    memset(model->closed, 0, sizeof(model->closed));
    return 1;
}

static int close_step_pc34(StackSplitModelPc34* model, int chestSlotIndex)
{
    M11_Item thing;

    if (!model || !model->closeStarted || chestSlotIndex < 0 ||
        chestSlotIndex >= DM1_PC34_CHEST_OPEN_STACK_SPLIT_SLOT_COUNT) {
        return 0;
    }

    thing = model->chestSlots[chestSlotIndex];
    if (!item_is_empty(&thing)) {
        clear_item(&model->chestSlots[chestSlotIndex]);
        if (model->closeCount < DM1_PC34_CHEST_OPEN_STACK_SPLIT_SLOT_COUNT) {
            model->closed[model->closeCount] = thing;
        }
        ++model->closeCount;
        model->lastLinkedType = thing.itemType;
    }
    return 1;
}

/*
 * C09 action-hand click on a chest slot when the C09 hand is empty:
 * this is the pickup path. ReDMCSB CHAMPION.C F0302 lines 688-710 reads
 * leader hand + destination slot, rejects empty/empty + incompatible
 * destinations, then performs the F0300 (remove from slot) + F0297
 * (put in leader hand) sequence. The destination-slot put-back is
 * gated on the original leader hand being non-empty; for the pickup
 * path the leader hand starts empty, so the slot stays empty and the
 * slot contents move into the leader hand.
 */
static int click_action_hand_on_chest_slot_pickup_pc34(
    StackSplitModelPc34* model,
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
        chestSlotIndex >= DM1_PC34_CHEST_OPEN_STACK_SPLIT_SLOT_COUNT) {
        return 0;
    }

    actionHandObject = model->actionHand;
    slotObject = model->chestSlots[chestSlotIndex];
    if (item_is_empty(&actionHandObject) && item_is_empty(&slotObject)) {
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

static int close_rest_pc34(StackSplitModelPc34* model, int startIndex)
{
    int i;

    if (!model || startIndex < 0 ||
        startIndex > DM1_PC34_CHEST_OPEN_STACK_SPLIT_SLOT_COUNT) {
        return -1;
    }
    for (i = startIndex; i < DM1_PC34_CHEST_OPEN_STACK_SPLIT_SLOT_COUNT; ++i) {
        if (!close_step_pc34(model, i)) {
            return -1;
        }
    }
    model->closeStarted = 0;
    model->closedEndOfListTerminator =
        DM1_PC34_CHEST_OPEN_STACK_SPLIT_ENDOFLIST;
    return model->closeCount;
}

const char*
dm1_v1_chest_open_stack_split_press_eye_source_evidence_pc34(void)
{
    return
        "CHEST.C F0333:32-42 P0694_B_PressingEye controls the C145_ICON_CONTAINER_CHEST_OPEN blit (line 44) and selects the open chest (line 38-39)\n"
        "CHEST.C F0333:53-67 copies the first eight linked things into G0425_aT_ChestSlots in list order\n"
        "CHEST.C F0333:70-74 fills remaining G0425 slots with C0xFFFF_THING_NONE\n"
        "CHEST.C F0334:113-117 exits when no chest is open, clears G0426, and resets the container Slot to C0xFFFE_THING_ENDOFLIST\n"
        "CHEST.C F0334:118-132 reads each current G0425 slot in ascending order, clears visited non-empty slots, and relinks visible things via L1026_B_ProcessFirstChestSlot + F0163_DUNGEON_LinkThingToList\n"
        "CHAMPION.C F0297:243-268/F0298:270-298 put/remove leader-hand identity and update leader load\n"
        "CHAMPION.C F0300:511-515 removes G0425 chest slots; F0301:606-614 writes G0425 chest slots and load\n"
        "CHAMPION.C F0302:688-710 reads leader hand (DM1_PC34_SLOT_ACTION_HAND) plus destination slot, performs remove/remove/put/put; for the pickup path the leader hand starts empty so the slot stays empty and the slot contents move into the leader hand\n"
        "PANEL.C F0347:1639-1691 routes C09_SLOT_BOX_INVENTORY_ACTION_HAND click to F0302 for the action-hand container swap/pickup\n"
        "OBJECT.C F0033:147-212 supplies icon identity for F0333/F0297 drawing; this gate uses itemType sentinels, not pixel parity\n"
        "BLITMASK.C F0133:30-33 describes masked bitmap presentation; this gate is contract-only and does not assert real-asset pixels";
}

const DM1_V1_ChestOpenStackSplitPressEyeSpecPc34*
dm1_v1_chest_open_stack_split_press_eye_spec_pc34(void)
{
    return &s_spec;
}

int dm1_v1_chest_open_stack_split_press_eye_run_pc34(
    DM1_V1_ChestOpenStackSplitPressEyeProbePc34* out)
{
    StackSplitModelPc34 model;
    M11_Item linked[DM1_PC34_CHEST_OPEN_STACK_SPLIT_SLOT_COUNT];
    int i;

    if (!out) {
        return 0;
    }

    memset(out, 0, sizeof(*out));
    memset(&model, 0, sizeof(model));

    out->contractOnly = 1;
    out->pressEyePathEnabled = 1;

    /* Build G0425: distinct items 0..2, stack pair at 3..4, distinct
     * items 5..7. The stack pair is the contract target. */
    for (i = 0; i < DM1_PC34_CHEST_OPEN_STACK_SPLIT_SLOT_COUNT; ++i) {
        int type = DM1_PC34_CHEST_OPEN_STACK_SPLIT_FIRST_ITEM + i;
        int weight = 4 + i;

        if (i == DM1_PC34_CHEST_OPEN_STACK_SPLIT_PICKUP_INDEX) {
            type = DM1_PC34_CHEST_OPEN_STACK_SPLIT_STACK_A;
            weight = 11;
        } else if (i == DM1_PC34_CHEST_OPEN_STACK_SPLIT_STACK_INDEX) {
            type = DM1_PC34_CHEST_OPEN_STACK_SPLIT_STACK_B;
            weight = 12;
        }
        linked[i] = make_item(type, weight);
    }

    out->openResult = press_eye_open_pc34(
        &model, DM1_PC34_CHEST_OPEN_STACK_SPLIT_CHEST_THING, linked,
        DM1_PC34_CHEST_OPEN_STACK_SPLIT_SLOT_COUNT);
    out->openThing = model.openChestThing;
    out->openVisibleCount = count_visible(model.chestSlots);
    copy_types(model.chestSlots,
               DM1_PC34_CHEST_OPEN_STACK_SPLIT_SLOT_COUNT,
               out->openedTypes);
    copy_weights(model.chestSlots,
                 DM1_PC34_CHEST_OPEN_STACK_SPLIT_SLOT_COUNT,
                 out->openedWeights);
    {
        int matches = 1;
        for (i = 0; i < DM1_PC34_CHEST_OPEN_STACK_SPLIT_SLOT_COUNT; ++i) {
            if (model.chestSlots[i].itemType != linked[i].itemType) {
                matches = 0;
                break;
            }
        }
        out->openedOrderMatchesInput = matches;
    }
    out->pressEyeIconBlitSkipped = 1;
    out->stackPairAdjacent =
        (out->openedTypes
             [DM1_PC34_CHEST_OPEN_STACK_SPLIT_PICKUP_INDEX] ==
         DM1_PC34_CHEST_OPEN_STACK_SPLIT_STACK_A) &&
        (out->openedTypes
             [DM1_PC34_CHEST_OPEN_STACK_SPLIT_STACK_INDEX] ==
         DM1_PC34_CHEST_OPEN_STACK_SPLIT_STACK_B);

    /* C09 action-hand before close: empty. */
    out->actionHandBeforeCloseType = model.actionHand.itemType;
    out->actionHandBeforeCloseEmpty = item_is_empty(&model.actionHand);
    out->pickupAllowedByActionHand = 1;

    /* F0334 begin. */
    out->openThingBeforeCloseBegin = model.openChestThing;
    out->closeBeginResult = close_begin_pc34(&model);
    out->openThingAfterCloseBegin = model.openChestThing;
    out->containerSlotClearedToEnd = model.containerSlotClearedToEnd;
    out->containerSlotEndOfListTerminator =
        model.containerSlotEndOfListTerminator;

    /* F0334 close prefix (slots 0..1). */
    for (i = 0; i < DM1_PC34_CHEST_OPEN_STACK_SPLIT_PREFIX_COUNT; ++i) {
        if (!close_step_pc34(&model, i)) {
            return 0;
        }
        out->prefixSlotsCleared[i] = item_is_empty(&model.chestSlots[i]);
    }
    out->prefixProcessedCount = DM1_PC34_CHEST_OPEN_STACK_SPLIT_PREFIX_COUNT;
    out->prefixClosedCount = model.closeCount;
    copy_types(model.closed,
               DM1_PC34_CHEST_OPEN_STACK_SPLIT_SLOT_COUNT,
               out->prefixClosedTypes);
    out->prefixLastLinkedType = model.lastLinkedType;
    out->prefixEndOfListTerminator = 0;

    /* C09 action-hand click on the pickup slot (the lower stack
     * member). The action hand is empty so this is a pure pickup:
     * F0300 removes the slot, F0297 puts the slot contents into the
     * leader hand, no put-back into the slot. */
    out->midPickupPc34Slot = DM1_PC34_SLOT_ACTION_HAND;
    out->midPickupSourcePc34Slot =
        DM1_PC34_SLOT_CHEST_1 +
        DM1_PC34_CHEST_OPEN_STACK_SPLIT_PICKUP_INDEX;
    out->midPickupLeaderBeforeType = model.actionHand.itemType;
    out->midPickupSlotBeforeType =
        model.chestSlots[DM1_PC34_CHEST_OPEN_STACK_SPLIT_PICKUP_INDEX]
            .itemType;
    out->midPickupResult = click_action_hand_on_chest_slot_pickup_pc34(
        &model, DM1_PC34_CHEST_OPEN_STACK_SPLIT_PICKUP_INDEX,
        &out->midPickupLeaderRemovedCall, &out->midPickupSlotRemovedCall,
        &out->midPickupPutSlotInLeaderCall,
        &out->midPickupPutLeaderInSlotCall);
    out->midPickupLeaderAfterType = model.actionHand.itemType;
    out->midPickupSlotAfterType =
        model.chestSlots[DM1_PC34_CHEST_OPEN_STACK_SPLIT_PICKUP_INDEX]
            .itemType;
    out->midPickupLeaderEmptyAfterPickup = item_is_empty(&model.actionHand);
    out->pickupSourceAbsentFromSlotAfterPickup =
        out->midPickupSlotAfterType == 0;

    /* F0334 close remainder. F0334 visits slot 2 (FIRST_ITEM+2),
     * then slot 3 (now empty due to pickup, skipped), then slot 4
     * (STACK_B, linked), then slots 5..7. The closed list is one
     * shorter than the opened list because slot 3 was picked up. */
    out->closeCount = close_rest_pc34(
        &model, DM1_PC34_CHEST_OPEN_STACK_SPLIT_PREFIX_COUNT);
    copy_types(model.closed,
               DM1_PC34_CHEST_OPEN_STACK_SPLIT_SLOT_COUNT,
               out->closedTypes);
    copy_weights(model.closed,
                 DM1_PC34_CHEST_OPEN_STACK_SPLIT_SLOT_COUNT,
                 out->closedWeights);
    out->stackBLinkedAtStackPosition =
        (out->closedTypes
             [DM1_PC34_CHEST_OPEN_STACK_SPLIT_PREFIX_COUNT + 1] ==
         DM1_PC34_CHEST_OPEN_STACK_SPLIT_STACK_B);
    out->stackAAbsentFromClosedLinks =
        !contains_type(model.closed, model.closeCount,
                       DM1_PC34_CHEST_OPEN_STACK_SPLIT_STACK_A);
    out->pickupAbsentFromClosedLinks =
        !contains_type(model.closed, model.closeCount,
                       DM1_PC34_CHEST_OPEN_STACK_SPLIT_FIRST_ITEM +
                       DM1_PC34_CHEST_OPEN_STACK_SPLIT_PICKUP_INDEX);
    {
        int ordered = 1;
        int expectedSlot = 0;
        for (i = 0; i < DM1_PC34_CHEST_OPEN_STACK_SPLIT_SLOT_COUNT; ++i) {
            int expected = DM1_PC34_CHEST_OPEN_STACK_SPLIT_FIRST_ITEM + i;

            if (i == DM1_PC34_CHEST_OPEN_STACK_SPLIT_PICKUP_INDEX) {
                continue; /* picked up; absent from closed list */
            }
            if (i == DM1_PC34_CHEST_OPEN_STACK_SPLIT_STACK_INDEX) {
                expected = DM1_PC34_CHEST_OPEN_STACK_SPLIT_STACK_B;
            }
            if (model.closed[expectedSlot].itemType != expected) {
                ordered = 0;
                break;
            }
            ++expectedSlot;
        }
        out->closedOrderPreservesStackPattern = ordered;
    }
    out->pickupSlotClearedByClose =
        item_is_empty(
            &model.chestSlots
                [DM1_PC34_CHEST_OPEN_STACK_SPLIT_PICKUP_INDEX]);
    out->stackSlotClearedByClose =
        item_is_empty(
            &model.chestSlots
                [DM1_PC34_CHEST_OPEN_STACK_SPLIT_STACK_INDEX]);
    out->allSlotsEmptyAfterClose = all_slots_empty(&model);
    out->actionHandAfterCloseType = model.actionHand.itemType;
    out->actionHandAfterCloseWeight = model.actionHand.weight;
    out->actionHandAfterCloseEmpty = item_is_empty(&model.actionHand);
    out->closedEndOfListTerminator = model.closedEndOfListTerminator;

    /* Reopen via F0333 press-eye again. */
    out->reopenResult = press_eye_open_pc34(
        &model, DM1_PC34_CHEST_OPEN_STACK_SPLIT_REOPEN_THING,
        model.closed, model.closeCount);
    out->reopenThing = model.openChestThing;
    out->reopenedVisibleCount = count_visible(model.chestSlots);
    copy_types(model.chestSlots,
               DM1_PC34_CHEST_OPEN_STACK_SPLIT_SLOT_COUNT,
               out->reopenedTypes);
    out->reopenedOrderMatchesClosed =
        memcmp(model.chestSlots, model.closed, sizeof(model.closed)) == 0;
    out->stackBVisibleAfterReopen =
        contains_type(model.chestSlots,
                      DM1_PC34_CHEST_OPEN_STACK_SPLIT_SLOT_COUNT,
                      DM1_PC34_CHEST_OPEN_STACK_SPLIT_STACK_B);
    out->stackAAbsentAfterReopen =
        !contains_type(model.chestSlots,
                       DM1_PC34_CHEST_OPEN_STACK_SPLIT_SLOT_COUNT,
                       DM1_PC34_CHEST_OPEN_STACK_SPLIT_STACK_A);
    out->pickupAbsentAfterReopen =
        !contains_type(model.chestSlots,
                       DM1_PC34_CHEST_OPEN_STACK_SPLIT_SLOT_COUNT,
                       DM1_PC34_CHEST_OPEN_STACK_SPLIT_FIRST_ITEM +
                       DM1_PC34_CHEST_OPEN_STACK_SPLIT_PICKUP_INDEX);

    return 1;
}
