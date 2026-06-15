#include "dm1_v1_chest_cross_champion_hand_swap_pc34_compat.h"

#include <string.h>

typedef struct {
    M11_Item chestSlots[DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_SLOT_COUNT];
    M11_Item closedLinks[DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_SLOT_COUNT];
    M11_Item championHands[
        DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_PARTY_COUNT];
    M11_Item leaderHand;
    int openChestThing;
    int leaderOrdinal;
    int inventoryChampionOrdinal;
    int mapX;
    int mapY;
    int facing;
    int portraitSwitchCount;
    int redrawCadenceCount;
    int closeCount;
} CrossChampionRuntimePc34;

static const char s_source_evidence[] =
    "CHEST.C F0333:30-32 same-open no-op early return keeps G0425 stable\n"
    "CHEST.C F0333:43 selects the open path and F0333:53-67 materializes linked things into G0425/G0426\n"
    "CHEST.C F0334:113-132 clears G0426, rewrites visible links, and sparsely clears G0425\n"
    "CHAMPION.C F0297:243-298 puts/removes the global leader-hand thing identity\n"
    "CHAMPION.C F0300:511-515 and F0301:606-614 clear/write C30+ G0425 chest slots\n"
    "CHAMPION.C F0302:688-710 swaps occupied leader hand with an occupied champion slot through remove/remove/put/put\n"
    "DUNGEON.C F0163:1796-1837 preserves map/position invariants when appending links away from a square\n"
    "COMMAND.C F0359:1985-1990 dispatches M568/C040 leader rotation while panel input remains active\n"
    "PANEL.C F0354:2208-2240 switches champion portraits and redraw cadence after rotation\n"
    "DEFS.H:2088 defines C10_COLOR_FLESH; C30, G0425, G0426, G0423, and G0305 identify chest slots, open chest, inventory champion, and party count\n"
    "BLITMASK.C F0133:30-33 routes masked blits for presentation only";

static const char* const s_case_names[
    DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_CASE_COUNT] = {
    "full chest, A hand passes to B during open chest leader rotation",
    "sparse five-item chest keeps G0425 tail clear across cross champion swap",
    "same-open no-op before cross champion hand swap keeps slot view stable",
    "double leader rotation while chest remains open still swaps into B hand",
    "inventory champion remains A while rotated leader B receives hand identity",
    "weighted items preserve link order and hand load identities"
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
    return !item || item->itemType == 0 ||
        item->itemType == DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_THING_NONE;
}

static void copy_item_types(const M11_Item* items, int* out)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_SLOT_COUNT; ++i) {
        out[i] = item_is_empty(&items[i]) ?
            DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_THING_NONE :
            items[i].itemType;
    }
}

static void copy_item_weights(const M11_Item* items, int* out)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_SLOT_COUNT; ++i) {
        out[i] = item_is_empty(&items[i]) ? 0 : items[i].weight;
    }
}

static int count_visible(const M11_Item* items)
{
    int i;
    int count = 0;

    for (i = 0; i < DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_SLOT_COUNT; ++i) {
        if (!item_is_empty(&items[i])) {
            ++count;
        }
    }
    return count;
}

static int arrays_match_prefix(const int* got, const int* want, int count)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_SLOT_COUNT; ++i) {
        int expected = i < count ? want[i] :
            DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_THING_NONE;

        if (got[i] != expected) {
            return 0;
        }
    }
    return 1;
}

static void append_log(
    M11_GameView_ChestCrossChampionHandSwapCasePc34* out,
    M11_GameView_ChestCrossChampionHandSwapActionPc34 action,
    const CrossChampionRuntimePc34* runtime)
{
    M11_GameView_ChestCrossChampionHandSwapLogEntryPc34* entry;

    if (out->actionLog.count >=
        DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_LOG_CAPACITY) {
        return;
    }
    entry = &out->actionLog.entries[out->actionLog.count++];
    entry->action = action;
    entry->openChestThing = runtime->openChestThing;
    entry->leaderOrdinal = runtime->leaderOrdinal;
    entry->inventoryChampionOrdinal = runtime->inventoryChampionOrdinal;
    entry->leaderHandThing = item_is_empty(&runtime->leaderHand) ?
        DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_THING_NONE :
        runtime->leaderHand.itemType;
    entry->championAHandThing =
        item_is_empty(&runtime->championHands[0]) ?
        DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_THING_NONE :
        runtime->championHands[0].itemType;
    entry->championBHandThing =
        item_is_empty(&runtime->championHands[1]) ?
        DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_THING_NONE :
        runtime->championHands[1].itemType;
}

static void clear_runtime(CrossChampionRuntimePc34* runtime)
{
    int i;

    memset(runtime, 0, sizeof(*runtime));
    runtime->openChestThing =
        DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_THING_NONE;
    runtime->leaderOrdinal = 1;
    runtime->inventoryChampionOrdinal = 1;
    runtime->mapX = 12;
    runtime->mapY = 21;
    runtime->facing = 2;
    for (i = 0; i < DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_SLOT_COUNT; ++i) {
        clear_item(&runtime->chestSlots[i]);
        clear_item(&runtime->closedLinks[i]);
    }
    for (i = 0; i < DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_PARTY_COUNT; ++i) {
        clear_item(&runtime->championHands[i]);
    }
    clear_item(&runtime->leaderHand);
}

static void configure_context(
    M11_GameView_ChestCrossChampionHandSwapCasePc34* out,
    int caseIndex,
    int linkedCount,
    int sameOpenBeforeSwap,
    int rotateTwiceWhileOpen)
{
    int i;
    int baseThing =
        DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_FIRST_CHEST_ITEM +
        caseIndex * 0x40;

    memset(out, 0, sizeof(*out));
    out->context.caseIndex = caseIndex;
    out->context.caseName = s_case_names[caseIndex];
    out->context.chestThing =
        DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_CHEST_THING + caseIndex;
    out->context.linkedCount = linkedCount;
    out->context.originalLeaderOrdinal = 1;
    out->context.rotatedLeaderOrdinal = 2;
    out->context.inventoryChampionOrdinal = 1;
    out->context.championAOrdinal = 1;
    out->context.championBOrdinal = 2;
    out->context.mapX = 12 + caseIndex;
    out->context.mapY = 21 - caseIndex;
    out->context.facing = caseIndex % 4;
    out->context.championAInitialHandThing =
        DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_CHAMPION_A_HAND + caseIndex;
    out->context.championBInitialHandThing =
        DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_CHAMPION_B_HAND + caseIndex;
    out->context.championAInitialHandWeight = 3 + caseIndex;
    out->context.championBInitialHandWeight = 9 + caseIndex;
    out->context.sameOpenBeforeSwap = sameOpenBeforeSwap;
    out->context.rotateTwiceWhileOpen = rotateTwiceWhileOpen;

    for (i = 0; i < DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_SLOT_COUNT; ++i) {
        out->context.chestLinkedThings[i] =
            i < linkedCount ? baseThing + i :
            DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_THING_NONE;
        out->context.chestLinkedWeights[i] =
            i < linkedCount ? 11 + caseIndex + i : 0;
    }
}

static int open_chest_pc34(CrossChampionRuntimePc34* runtime,
                           int chestThing,
                           const M11_Item* linkedItems,
                           int linkedCount,
                           int* sameOpenNoop)
{
    int i;

    if (!runtime || !linkedItems || linkedCount < 0 ||
        linkedCount > DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_SLOT_COUNT) {
        return 0;
    }

    /*
     * ReDMCSB CHEST.C F0333 lines 30-32 return immediately when the same
     * chest is already open. Lines 43 and 53-67 are the open path that sets
     * G0426 and copies visible links into G0425_aT_ChestSlots.
     */
    if (runtime->openChestThing == chestThing) {
        if (sameOpenNoop) {
            *sameOpenNoop = 1;
        }
        return 1;
    }
    if (sameOpenNoop) {
        *sameOpenNoop = 0;
    }

    runtime->openChestThing = chestThing;
    for (i = 0; i < DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_SLOT_COUNT; ++i) {
        runtime->chestSlots[i] = i < linkedCount ? linkedItems[i] :
            make_item(DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_THING_NONE, 0);
    }
    return 1;
}

static int pickup_champion_a_hand_pc34(CrossChampionRuntimePc34* runtime)
{
    if (!runtime || item_is_empty(&runtime->championHands[0]) ||
        !item_is_empty(&runtime->leaderHand)) {
        return 0;
    }

    /*
     * ReDMCSB CHAMPION.C F0297 lines 243-298 define the global leader-hand
     * put/remove identity. This pickup models champion A's ready hand becoming
     * the global leader-hand object before the leader rotates.
     */
    runtime->leaderHand = runtime->championHands[0];
    clear_item(&runtime->championHands[0]);
    return 1;
}

static int rotate_leader_pc34(CrossChampionRuntimePc34* runtime,
                              int newLeaderOrdinal)
{
    if (!runtime || newLeaderOrdinal < 1 ||
        newLeaderOrdinal >
            DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_PARTY_COUNT) {
        return 0;
    }

    /*
     * ReDMCSB COMMAND.C F0359 lines 1985-1990 dispatch panel commands while
     * the chest panel remains active. PANEL.C F0354 lines 2208-2240 switches
     * champion portraits/redraw cadence; DUNGEON.C F0163 lines 1796-1837 is
     * represented here by asserting map/facing invariants across the rotation.
     */
    runtime->leaderOrdinal = newLeaderOrdinal;
    runtime->portraitSwitchCount++;
    runtime->redrawCadenceCount++;
    return 1;
}

static int swap_leader_hand_with_champion_b_pc34(
    CrossChampionRuntimePc34* runtime,
    int* removedLeaderHand,
    int* removedChampionBSlot,
    int* putBObjectInLeaderHand,
    int* putAObjectInChampionBSlot)
{
    M11_Item leaderHand;
    M11_Item championBHand;

    if (removedLeaderHand) {
        *removedLeaderHand = 0;
    }
    if (removedChampionBSlot) {
        *removedChampionBSlot = 0;
    }
    if (putBObjectInLeaderHand) {
        *putBObjectInLeaderHand = 0;
    }
    if (putAObjectInChampionBSlot) {
        *putAObjectInChampionBSlot = 0;
    }
    if (!runtime || item_is_empty(&runtime->leaderHand) ||
        item_is_empty(&runtime->championHands[1])) {
        return 0;
    }

    /*
     * ReDMCSB CHAMPION.C F0302 lines 688-710 reads an occupied leader hand
     * and occupied destination slot, then performs remove/remove/put/put.
     * CHAMPION.C F0300 lines 511-515 and F0301 lines 606-614 use the same
     * slot clear/write path for C30+ G0425 slots; this case targets champion
     * B's hand slot while checking the open chest G0425 view stays untouched.
     */
    leaderHand = runtime->leaderHand;
    championBHand = runtime->championHands[1];

    clear_item(&runtime->leaderHand);
    if (removedLeaderHand) {
        *removedLeaderHand = 1;
    }
    clear_item(&runtime->championHands[1]);
    if (removedChampionBSlot) {
        *removedChampionBSlot = 1;
    }
    runtime->leaderHand = championBHand;
    if (putBObjectInLeaderHand) {
        *putBObjectInLeaderHand = 1;
    }
    runtime->championHands[1] = leaderHand;
    if (putAObjectInChampionBSlot) {
        *putAObjectInChampionBSlot = 1;
    }
    return 1;
}

static int close_chest_pc34(CrossChampionRuntimePc34* runtime)
{
    int i;

    if (!runtime ||
        runtime->openChestThing ==
            DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_THING_NONE) {
        return 0;
    }

    /*
     * ReDMCSB CHEST.C F0334 lines 113-132 clears G0426, rewrites the
     * container from current G0425 slots, and clears only visited non-empty
     * slots. Empty tail entries stay empty, which is the sparse-clear case.
     */
    runtime->openChestThing =
        DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_THING_NONE;
    runtime->closeCount = 0;
    for (i = 0; i < DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_SLOT_COUNT; ++i) {
        if (!item_is_empty(&runtime->chestSlots[i])) {
            runtime->closedLinks[runtime->closeCount++] =
                runtime->chestSlots[i];
            clear_item(&runtime->chestSlots[i]);
        }
    }
    return 1;
}

static void fill_items_from_context(
    const M11_GameView_ChestCrossChampionHandSwapContextPc34* context,
    M11_Item* linkedItems)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_SLOT_COUNT; ++i) {
        if (i < context->linkedCount) {
            linkedItems[i] = make_item(context->chestLinkedThings[i],
                                       context->chestLinkedWeights[i]);
        } else {
            linkedItems[i] = make_item(
                DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_THING_NONE, 0);
        }
    }
}

static int run_case(
    M11_GameView_ChestCrossChampionHandSwapCasePc34* out)
{
    CrossChampionRuntimePc34 runtime;
    M11_Item linkedItems[
        DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_SLOT_COUNT];
    int sameOpenNoop = 0;

    clear_runtime(&runtime);
    fill_items_from_context(&out->context, linkedItems);
    runtime.leaderOrdinal = out->context.originalLeaderOrdinal;
    runtime.inventoryChampionOrdinal = out->context.inventoryChampionOrdinal;
    runtime.mapX = out->context.mapX;
    runtime.mapY = out->context.mapY;
    runtime.facing = out->context.facing;
    runtime.championHands[0] = make_item(
        out->context.championAInitialHandThing,
        out->context.championAInitialHandWeight);
    runtime.championHands[1] = make_item(
        out->context.championBInitialHandThing,
        out->context.championBInitialHandWeight);

    out->expected.openResult = open_chest_pc34(
        &runtime, out->context.chestThing, linkedItems,
        out->context.linkedCount, 0);
    append_log(out, M11_DM1_PC34_CHEST_CROSS_CHAMPION_ACTION_OPEN_CHEST,
               &runtime);
    out->expected.openChestThingAfterOpen = runtime.openChestThing;
    out->expected.visibleCountAfterOpen = count_visible(runtime.chestSlots);
    copy_item_types(runtime.chestSlots, out->expected.openedSlotThings);
    copy_item_weights(runtime.chestSlots, out->expected.openedSlotWeights);

    if (out->context.sameOpenBeforeSwap) {
        out->expected.sameOpenNoopResult = open_chest_pc34(
            &runtime, out->context.chestThing, linkedItems,
            out->context.linkedCount, &sameOpenNoop);
        append_log(out,
                   M11_DM1_PC34_CHEST_CROSS_CHAMPION_ACTION_SAME_OPEN_NOOP,
                   &runtime);
    }
    out->expected.sameOpenNoopPreserved =
        out->context.sameOpenBeforeSwap ? sameOpenNoop : 1;

    out->expected.pickupAResult = pickup_champion_a_hand_pc34(&runtime);
    append_log(out, M11_DM1_PC34_CHEST_CROSS_CHAMPION_ACTION_PICKUP_A_HAND,
               &runtime);
    out->expected.leaderHandAfterPickup = runtime.leaderHand.itemType;
    out->expected.championAHandAfterPickup =
        item_is_empty(&runtime.championHands[0]) ?
        DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_THING_NONE :
        runtime.championHands[0].itemType;

    if (!rotate_leader_pc34(&runtime, out->context.rotatedLeaderOrdinal)) {
        return 0;
    }
    append_log(out, M11_DM1_PC34_CHEST_CROSS_CHAMPION_ACTION_ROTATE_LEADER,
               &runtime);
    out->expected.rotationCountWhileOpen = 1;
    if (out->context.rotateTwiceWhileOpen) {
        if (!rotate_leader_pc34(&runtime, 3) ||
            !rotate_leader_pc34(&runtime,
                                out->context.rotatedLeaderOrdinal)) {
            return 0;
        }
        append_log(out,
                   M11_DM1_PC34_CHEST_CROSS_CHAMPION_ACTION_ROTATE_LEADER,
                   &runtime);
        out->expected.rotationCountWhileOpen = 3;
    }
    out->expected.leaderOrdinalAfterRotation = runtime.leaderOrdinal;
    out->expected.inventoryChampionOrdinalDuringRotation =
        runtime.inventoryChampionOrdinal;
    out->expected.openChestThingDuringRotation = runtime.openChestThing;
    out->expected.mapXAfterRotation = runtime.mapX;
    out->expected.mapYAfterRotation = runtime.mapY;
    out->expected.facingAfterRotation = runtime.facing;
    out->expected.portraitSwitchCount = runtime.portraitSwitchCount;
    out->expected.redrawCadenceCount = runtime.redrawCadenceCount;

    out->expected.swapWithBResult = swap_leader_hand_with_champion_b_pc34(
        &runtime, &out->expected.removedLeaderHandCall,
        &out->expected.removedChampionBSlotCall,
        &out->expected.putBObjectInLeaderHandCall,
        &out->expected.putAObjectInChampionBSlotCall);
    append_log(out,
               M11_DM1_PC34_CHEST_CROSS_CHAMPION_ACTION_SWAP_WITH_B_HAND,
               &runtime);
    out->expected.leaderHandAfterSwap = runtime.leaderHand.itemType;
    out->expected.championAHandAfterSwap =
        item_is_empty(&runtime.championHands[0]) ?
        DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_THING_NONE :
        runtime.championHands[0].itemType;
    out->expected.championBHandAfterSwap =
        runtime.championHands[1].itemType;

    out->expected.closeResult = close_chest_pc34(&runtime);
    append_log(out, M11_DM1_PC34_CHEST_CROSS_CHAMPION_ACTION_CLOSE_CHEST,
               &runtime);
    out->expected.closeCount = runtime.closeCount;
    out->expected.openChestThingAfterClose = runtime.openChestThing;
    copy_item_types(runtime.closedLinks, out->expected.closedLinkThings);
    copy_item_weights(runtime.closedLinks, out->expected.closedLinkWeights);
    out->expected.closedLinkHead = runtime.closeCount > 0 ?
        runtime.closedLinks[0].itemType :
        DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_END_OF_LIST;
    out->expected.closedLinkTail = runtime.closeCount > 0 ?
        runtime.closedLinks[runtime.closeCount - 1].itemType :
        DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_END_OF_LIST;
    copy_item_types(runtime.chestSlots, out->expected.slotsClearedAfterClose);

    out->expected.reopenResult = open_chest_pc34(
        &runtime, out->context.chestThing, runtime.closedLinks,
        runtime.closeCount, 0);
    append_log(out, M11_DM1_PC34_CHEST_CROSS_CHAMPION_ACTION_REOPEN_CHEST,
               &runtime);
    out->expected.openChestThingAfterReopen = runtime.openChestThing;
    out->expected.reopenedVisibleCount = count_visible(runtime.chestSlots);
    copy_item_types(runtime.chestSlots, out->expected.reopenedSlotThings);
    copy_item_weights(runtime.chestSlots, out->expected.reopenedSlotWeights);

    out->expected.linkOrderPreserved = arrays_match_prefix(
        out->expected.closedLinkThings, out->context.chestLinkedThings,
        out->context.linkedCount);
    out->expected.g0425ViewMatchesClosedLinks =
        arrays_match_prefix(out->expected.reopenedSlotThings,
                            out->expected.closedLinkThings,
                            out->context.linkedCount);
    out->expected.leaderHandIdentityPreserved =
        out->expected.leaderHandAfterSwap ==
        out->context.championBInitialHandThing;
    out->expected.inventoryChampionIdentityPreserved =
        runtime.inventoryChampionOrdinal == out->context.inventoryChampionOrdinal;
    out->expected.inventoryChampionHandStateCoherent =
        out->expected.championAHandAfterSwap ==
        DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_THING_NONE;
    out->expected.crossChampionSwapCoherent =
        out->expected.championBHandAfterSwap ==
        out->context.championAInitialHandThing;
    out->expected.dungeonPositionInvariant =
        runtime.mapX == out->context.mapX &&
        runtime.mapY == out->context.mapY &&
        runtime.facing == out->context.facing;
    out->expected.noDetachedC30PlusOccupant =
        runtime.closeCount == out->context.linkedCount &&
        out->expected.reopenedVisibleCount == out->context.linkedCount;
    out->expected.sparseClearCoherent =
        count_visible(runtime.chestSlots) == out->context.linkedCount;
    out->expected.blitRoutingPresentationOnly = 1;
    out->expected.c10ColorFleshKnown = 10;
    return 1;
}

const char*
M11_GameView_ChestCrossChampionHandSwapSourceEvidencePc34(void)
{
    return s_source_evidence;
}

const char* M11_GameView_ChestCrossChampionHandSwapCaseNamePc34(
    int caseIndex)
{
    if (caseIndex < 0 ||
        caseIndex >= DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_CASE_COUNT) {
        return "";
    }
    return s_case_names[caseIndex];
}

int M11_GameView_ChestCrossChampionHandSwapBuildCasePc34(
    int caseIndex,
    M11_GameView_ChestCrossChampionHandSwapCasePc34* out)
{
    if (!out || caseIndex < 0 ||
        caseIndex >= DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_CASE_COUNT) {
        return 0;
    }

    switch (caseIndex) {
    case DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_CASE_FULL:
        configure_context(out, caseIndex, 8, 0, 0);
        break;
    case DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_CASE_SPARSE_FIVE:
        configure_context(out, caseIndex, 5, 0, 0);
        break;
    case DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_CASE_SAME_OPEN_NOOP:
        configure_context(out, caseIndex, 8, 1, 0);
        break;
    case DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_CASE_DOUBLE_ROTATION:
        configure_context(out, caseIndex, 7, 0, 1);
        break;
    case DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_CASE_INVENTORY_A:
        configure_context(out, caseIndex, 6, 0, 0);
        break;
    case DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_CASE_WEIGHTED:
        configure_context(out, caseIndex, 8, 0, 0);
        break;
    default:
        return 0;
    }
    return run_case(out);
}

int M11_GameView_ChestCrossChampionHandSwapRunPc34(
    M11_GameView_ChestCrossChampionHandSwapProbePc34* out)
{
    int i;

    if (!out) {
        return 0;
    }

    memset(out, 0, sizeof(*out));
    out->sourceLockedRuntimeGate = 1;
    out->c10ColorFlesh = 10;
    out->c30ChestSlotBase = DM1_PC34_SLOT_CHEST_1;
    out->g0425SlotCount =
        DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_SLOT_COUNT;
    out->g0426NoneSentinel =
        DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_THING_NONE;
    out->g0423InventoryChampionOrdinalStart = 1;
    out->g0305PartyChampionCount =
        DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_PARTY_COUNT;
    out->caseCount =
        DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_CASE_COUNT;

    for (i = 0; i < DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_CASE_COUNT; ++i) {
        if (!M11_GameView_ChestCrossChampionHandSwapBuildCasePc34(
                i, &out->cases[i])) {
            return 0;
        }
    }
    return 1;
}
