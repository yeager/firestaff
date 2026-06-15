#include "dm1/dm1_v1_chest_non_leader_hand_occupied_slot_swap_pc34_compat.h"

#include <string.h>

enum {
    READY_HAND = 0,
    ACTION_HAND = 1,
    CHAMPION_COUNT = 4,
    HAND_COUNT = 2,
    ALLOWED_CONTAINER = 0x0400
};

typedef struct {
    int thing;
    int weight;
    int allowedSlots;
} RuntimeItemPc34;

typedef struct {
    RuntimeItemPc34 chestSlots[DM1_PC34_CHEST_NON_LEADER_SWAP_SLOT_COUNT];
    RuntimeItemPc34 closedLinks[DM1_PC34_CHEST_NON_LEADER_SWAP_SLOT_COUNT];
    RuntimeItemPc34 championHands[CHAMPION_COUNT][HAND_COUNT];
    RuntimeItemPc34 leaderPointer;
    int openChestThing;
} RuntimePc34;

static const char s_source_evidence[] =
    "CHEST.C F0334:117-132 closes G0426, rewrites non-empty G0425 chest slots, and clears processed slots to C0xFFFF_NONE\n"
    "CHAMPION.C F0297:243-298 puts a thing into the global leader-hand pointer; F0298:270-298 removes only that pointer\n"
    "CHAMPION.C F0300:511-515 clears C30+ G0425 chest slots to C0xFFFF_NONE; F0301:606-614 writes C30+ slots back to G0425\n"
    "CHAMPION.C F0302:662-710 resolves non-inventory champion hand slot boxes and performs occupied-slot swaps through the leader hand\n"
    "OBJECT.C F0033:147-212 derives icon identity for the swapped thing; BLITMASK.C F0133:30-33 is presentation-only partial-mask dispatch\n"
    "DEFS.H:2088 plus C30/G0425/G0426/G0423/G0305/M070/M516 define C30 chest slots, open chest, inventory champion, party count, hand-slot mapping, and champion slots";

const DM1_V1_ChestNonLeaderHandSwapSpecPc34
    dm1_v1_chest_non_leader_hand_occupied_slot_swap_pc34_spec = {
        1,
        DM1_PC34_CHEST_NON_LEADER_SWAP_NONE,
        DM1_PC34_CHEST_NON_LEADER_SWAP_END_OF_LIST,
        DM1_PC34_CHEST_NON_LEADER_SWAP_C30_BASE,
        DM1_PC34_CHEST_NON_LEADER_SWAP_LEADER_ORDINAL,
        DM1_PC34_CHEST_NON_LEADER_SWAP_INVENTORY_ORDINAL,
        DM1_PC34_CHEST_NON_LEADER_SWAP_NON_LEADER_ORDINAL,
        DM1_PC34_CHEST_NON_LEADER_SWAP_NON_LEADER_SLOT_BOX,
        DM1_PC34_CHEST_NON_LEADER_SWAP_TARGET_INDEX,
        DM1_PC34_CHEST_NON_LEADER_SWAP_COMPACTED_INDEX,
        { DM1_PC34_CHEST_NON_LEADER_SWAP_NON_LEADER_ITEM, 7,
          ALLOWED_CONTAINER },
        { DM1_PC34_CHEST_NON_LEADER_SWAP_LEADER_ITEM, 11,
          ALLOWED_CONTAINER },
        "CHEST.C F0334 lines 117-132 close-rewrite",
        "CHAMPION.C F0297 lines 243-298 leader-hand put",
        "CHAMPION.C F0298 lines 270-298 leader-hand remove",
        "CHAMPION.C F0300 lines 511-515 C30+ slot clear",
        "CHAMPION.C F0301 lines 606-614 C30+ slot write",
        "CHAMPION.C F0302 lines 662-710 occupied-slot swap",
        "OBJECT.C F0033 lines 147-212 icon identity",
        "BLITMASK.C F0133 lines 30-33 partial-mask dispatch",
        "DEFS.H line 2088 C30/G0425/G0426/G0423/G0305/M070/M516"
    };

static RuntimeItemPc34 make_item(int thing, int weight)
{
    RuntimeItemPc34 item;

    item.thing = thing;
    item.weight = weight;
    item.allowedSlots = ALLOWED_CONTAINER;
    return item;
}

static RuntimeItemPc34 none_item(void)
{
    return make_item(DM1_PC34_CHEST_NON_LEADER_SWAP_NONE, 0);
}

static int is_none(RuntimeItemPc34 item)
{
    return item.thing == DM1_PC34_CHEST_NON_LEADER_SWAP_NONE;
}

static void init_runtime(RuntimePc34* runtime)
{
    int c;
    int h;
    int i;

    memset(runtime, 0, sizeof(*runtime));
    for (i = 0; i < DM1_PC34_CHEST_NON_LEADER_SWAP_SLOT_COUNT; ++i) {
        runtime->chestSlots[i] = none_item();
        runtime->closedLinks[i] = none_item();
    }
    for (c = 0; c < CHAMPION_COUNT; ++c) {
        for (h = 0; h < HAND_COUNT; ++h) {
            runtime->championHands[c][h] = none_item();
        }
    }
    runtime->leaderPointer = none_item();
    runtime->openChestThing = DM1_PC34_CHEST_NON_LEADER_SWAP_OPEN_CHEST;
    runtime->chestSlots[0] =
        make_item(DM1_PC34_CHEST_NON_LEADER_SWAP_SEED_A, 3);
    runtime->chestSlots[2] =
        make_item(DM1_PC34_CHEST_NON_LEADER_SWAP_SEED_B, 5);
    runtime->championHands
        [DM1_PC34_CHEST_NON_LEADER_SWAP_NON_LEADER_ORDINAL - 1]
        [READY_HAND] =
            make_item(DM1_PC34_CHEST_NON_LEADER_SWAP_NON_LEADER_ITEM, 7);
}

static void copy_chest_types(const RuntimeItemPc34* slots, int* outTypes)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_NON_LEADER_SWAP_SLOT_COUNT; ++i) {
        outTypes[i] = slots[i].thing;
    }
}

static int contains_thing(const int* types, int thing)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_NON_LEADER_SWAP_SLOT_COUNT; ++i) {
        if (types[i] == thing) {
            return 1;
        }
    }
    return 0;
}

static int find_thing(const int* types, int thing)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_NON_LEADER_SWAP_SLOT_COUNT; ++i) {
        if (types[i] == thing) {
            return i;
        }
    }
    return -1;
}

static int resolve_status_slot_box(int slotBoxIndex,
                                   int* championIndex,
                                   int* handSlot)
{
    if (!championIndex || !handSlot || slotBoxIndex < 0 ||
        slotBoxIndex >= 8) {
        return 0;
    }
    *championIndex = slotBoxIndex >> 1;
    *handSlot = (slotBoxIndex & 1) ? ACTION_HAND : READY_HAND;
    return 1;
}

static int click_non_leader_hand(RuntimePc34* runtime,
                                 int slotBoxIndex,
                                 int* outChampionIndex,
                                 int* outHandSlot)
{
    int championIndex;
    int handSlot;
    RuntimeItemPc34 leaderBefore;
    RuntimeItemPc34 slotBefore;

    if (!runtime ||
        !resolve_status_slot_box(slotBoxIndex, &championIndex, &handSlot) ||
        championIndex < 0 || championIndex >= CHAMPION_COUNT) {
        return 0;
    }
    if (outChampionIndex) {
        *outChampionIndex = championIndex;
    }
    if (outHandSlot) {
        *outHandSlot = handSlot;
    }
    leaderBefore = runtime->leaderPointer;
    slotBefore = runtime->championHands[championIndex][handSlot];
    if (is_none(leaderBefore) && is_none(slotBefore)) {
        return 0;
    }

    /*
     * ReDMCSB CHAMPION.C F0302 lines 662-687 resolve C0..C7 status
     * hand boxes through M070/M516, then lines 700-710 move the selected
     * slot thing through F0300/F0297 and the prior leader pointer through
     * F0298/F0301.
     */
    if (!is_none(leaderBefore)) {
        runtime->leaderPointer = none_item();
    }
    if (!is_none(slotBefore)) {
        runtime->championHands[championIndex][handSlot] = none_item();
        runtime->leaderPointer = slotBefore;
    }
    if (!is_none(leaderBefore)) {
        runtime->championHands[championIndex][handSlot] = leaderBefore;
    }
    return 1;
}

static int click_chest_slot(RuntimePc34* runtime,
                            int chestIndex,
                            int* clearSlotIndex,
                            int* valueAfterClear,
                            int* valueAfterWrite)
{
    RuntimeItemPc34 leaderBefore;
    RuntimeItemPc34 slotBefore;

    if (!runtime || chestIndex < 0 ||
        chestIndex >= DM1_PC34_CHEST_NON_LEADER_SWAP_SLOT_COUNT) {
        return 0;
    }
    leaderBefore = runtime->leaderPointer;
    slotBefore = runtime->chestSlots[chestIndex];
    if (is_none(leaderBefore) && is_none(slotBefore)) {
        return 0;
    }
    if (clearSlotIndex) {
        *clearSlotIndex = -1;
    }
    if (valueAfterClear) {
        *valueAfterClear = DM1_PC34_CHEST_NON_LEADER_SWAP_NONE;
    }
    if (valueAfterWrite) {
        *valueAfterWrite = runtime->chestSlots[chestIndex].thing;
    }

    /*
     * ReDMCSB CHAMPION.C F0298 lines 270-298 removes the global leader
     * pointer first. F0300 lines 511-515 clears the occupied C30+ G0425
     * slot, F0297 lines 243-298 moves the swapped-out thing into the leader
     * pointer, and F0301 lines 606-614 writes the previous leader thing back
     * to the C30+ target.
     */
    if (!is_none(leaderBefore)) {
        runtime->leaderPointer = none_item();
    }
    if (!is_none(slotBefore)) {
        runtime->chestSlots[chestIndex] = none_item();
        if (clearSlotIndex) {
            *clearSlotIndex = chestIndex;
        }
        if (valueAfterClear) {
            *valueAfterClear = runtime->chestSlots[chestIndex].thing;
        }
        runtime->leaderPointer = slotBefore;
    }
    if (!is_none(leaderBefore)) {
        runtime->chestSlots[chestIndex] = leaderBefore;
    }
    if (valueAfterWrite) {
        *valueAfterWrite = runtime->chestSlots[chestIndex].thing;
    }
    return 1;
}

static int close_chest(RuntimePc34* runtime, int* outTypes, int* outHead)
{
    int i;
    int count = 0;

    if (!runtime || !outTypes) {
        return -1;
    }
    for (i = 0; i < DM1_PC34_CHEST_NON_LEADER_SWAP_SLOT_COUNT; ++i) {
        runtime->closedLinks[i] = none_item();
        outTypes[i] = DM1_PC34_CHEST_NON_LEADER_SWAP_NONE;
    }
    if (outHead) {
        *outHead = DM1_PC34_CHEST_NON_LEADER_SWAP_END_OF_LIST;
    }

    /*
     * ReDMCSB CHEST.C F0334 lines 117-132 writes the container Slot to
     * C0xFFFE, skips C0xFFFF entries, relinks non-empty G0425 slots in order,
     * and clears each processed G0425 entry to C0xFFFF_NONE.
     */
    for (i = 0; i < DM1_PC34_CHEST_NON_LEADER_SWAP_SLOT_COUNT; ++i) {
        if (!is_none(runtime->chestSlots[i])) {
            runtime->closedLinks[count] = runtime->chestSlots[i];
            outTypes[count] = runtime->chestSlots[i].thing;
            if (outHead && *outHead ==
                DM1_PC34_CHEST_NON_LEADER_SWAP_END_OF_LIST) {
                *outHead = runtime->chestSlots[i].thing;
            }
            runtime->chestSlots[i] = none_item();
            ++count;
        }
    }
    runtime->openChestThing = DM1_PC34_CHEST_NON_LEADER_SWAP_NONE;
    return count;
}

static int reopen_from_closed(RuntimePc34* runtime)
{
    int i;

    if (!runtime) {
        return 0;
    }
    runtime->openChestThing = DM1_PC34_CHEST_NON_LEADER_SWAP_REOPEN_CHEST;
    for (i = 0; i < DM1_PC34_CHEST_NON_LEADER_SWAP_SLOT_COUNT; ++i) {
        runtime->chestSlots[i] = runtime->closedLinks[i];
    }
    return 1;
}

const char*
dm1_v1_chest_non_leader_hand_occupied_slot_swap_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const DM1_V1_ChestNonLeaderHandSwapSpecPc34*
dm1_v1_chest_non_leader_hand_occupied_slot_swap_spec_pc34(void)
{
    return &dm1_v1_chest_non_leader_hand_occupied_slot_swap_pc34_spec;
}

int dm1_v1_chest_non_leader_hand_occupied_slot_swap_pc34(
    DM1_V1_ChestNonLeaderHandSwapProbePc34* out)
{
    RuntimePc34 runtime;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    init_runtime(&runtime);

    out->sourceSlotBox =
        DM1_PC34_CHEST_NON_LEADER_SWAP_NON_LEADER_SLOT_BOX;
    out->initialLeaderHand = runtime.leaderPointer.thing;
    out->initialNonLeaderHand =
        runtime.championHands
            [DM1_PC34_CHEST_NON_LEADER_SWAP_NON_LEADER_ORDINAL - 1]
            [READY_HAND].thing;
    copy_chest_types(runtime.chestSlots, out->initialChestTypes);

    out->pickupClickResult = click_non_leader_hand(
        &runtime, out->sourceSlotBox, &out->championIndexResolved,
        &out->sourceHandSlotResolved);
    out->leaderHandAfterNonLeaderPickup = runtime.leaderPointer.thing;
    out->nonLeaderHandAfterPickup =
        runtime.championHands
            [DM1_PC34_CHEST_NON_LEADER_SWAP_NON_LEADER_ORDINAL - 1]
            [READY_HAND].thing;
    out->c0ffffClearAppliedToSourceHand =
        out->nonLeaderHandAfterPickup ==
            DM1_PC34_CHEST_NON_LEADER_SWAP_NONE;

    out->emptyTargetClickResult = click_chest_slot(
        &runtime, DM1_PC34_CHEST_NON_LEADER_SWAP_TARGET_INDEX, 0, 0, 0);
    out->targetChestSlotAfterNonLeaderDrop =
        runtime.chestSlots
            [DM1_PC34_CHEST_NON_LEADER_SWAP_TARGET_INDEX].thing;
    out->leaderHandAfterNonLeaderDrop = runtime.leaderPointer.thing;
    out->nonLeaderHandAfterNonLeaderDrop =
        runtime.championHands
            [DM1_PC34_CHEST_NON_LEADER_SWAP_NON_LEADER_ORDINAL - 1]
            [READY_HAND].thing;
    out->targetStillOccupiedBeforeClose =
        out->targetChestSlotAfterNonLeaderDrop ==
            DM1_PC34_CHEST_NON_LEADER_SWAP_NON_LEADER_ITEM;

    out->leaderHandBeforeClose = runtime.leaderPointer.thing;
    out->closeCount = close_chest(
        &runtime, out->closedTypes, &out->closeContainerHead);
    out->closeEndSentinel = DM1_PC34_CHEST_NON_LEADER_SWAP_END_OF_LIST;
    out->closedContainsNonLeaderItem = contains_thing(
        out->closedTypes, DM1_PC34_CHEST_NON_LEADER_SWAP_NON_LEADER_ITEM);
    out->closedNonLeaderItemIndex = find_thing(
        out->closedTypes, DM1_PC34_CHEST_NON_LEADER_SWAP_NON_LEADER_ITEM);
    out->closePreservedLeaderHandPointer =
        runtime.leaderPointer.thing == out->leaderHandBeforeClose;
    out->targetClearedOnlyByCloseRewrite =
        runtime.chestSlots
            [DM1_PC34_CHEST_NON_LEADER_SWAP_TARGET_INDEX].thing ==
            DM1_PC34_CHEST_NON_LEADER_SWAP_NONE;

    out->reopenResult = reopen_from_closed(&runtime);
    copy_chest_types(runtime.chestSlots, out->reopenedTypes);
    out->reopenedNonLeaderItemIndex = find_thing(
        out->reopenedTypes, DM1_PC34_CHEST_NON_LEADER_SWAP_NON_LEADER_ITEM);

    runtime.leaderPointer =
        make_item(DM1_PC34_CHEST_NON_LEADER_SWAP_LEADER_ITEM, 11);
    out->leaderHandBeforeOccupiedSwap = runtime.leaderPointer.thing;
    out->nonLeaderHandBeforeOccupiedSwap =
        runtime.championHands
            [DM1_PC34_CHEST_NON_LEADER_SWAP_NON_LEADER_ORDINAL - 1]
            [READY_HAND].thing;
    out->occupiedSwapClickResult = click_chest_slot(
        &runtime, out->reopenedNonLeaderItemIndex, &out->c30ClearSlotIndex,
        &out->c30ValueAfterClear, &out->c30ValueAfterWrite);
    out->leaderHandAfterOccupiedSwap = runtime.leaderPointer.thing;
    out->nonLeaderHandAfterOccupiedSwap =
        runtime.championHands
            [DM1_PC34_CHEST_NON_LEADER_SWAP_NON_LEADER_ORDINAL - 1]
            [READY_HAND].thing;
    out->swapUsedLeaderPointer =
        out->leaderHandAfterOccupiedSwap ==
            DM1_PC34_CHEST_NON_LEADER_SWAP_NON_LEADER_ITEM;
    out->swapDidNotRestoreNonLeaderHand =
        out->nonLeaderHandAfterOccupiedSwap ==
            DM1_PC34_CHEST_NON_LEADER_SWAP_NONE;
    out->c0ffffClearDidNotSurviveTarget =
        out->c30ValueAfterWrite ==
            DM1_PC34_CHEST_NON_LEADER_SWAP_LEADER_ITEM;
    out->adjacentSlot0AfterSwap = runtime.chestSlots[0].thing;
    out->adjacentSlot1AfterSwap = runtime.chestSlots[1].thing;
    out->adjacentSlot3AfterSwap = runtime.chestSlots[3].thing;

    out->finalCloseCount = close_chest(
        &runtime, out->finalClosedTypes, 0);
    out->finalClosedContainsLeaderItem = contains_thing(
        out->finalClosedTypes, DM1_PC34_CHEST_NON_LEADER_SWAP_LEADER_ITEM);
    out->finalClosedContainsNonLeaderItem = contains_thing(
        out->finalClosedTypes,
        DM1_PC34_CHEST_NON_LEADER_SWAP_NON_LEADER_ITEM);

    return 1;
}
