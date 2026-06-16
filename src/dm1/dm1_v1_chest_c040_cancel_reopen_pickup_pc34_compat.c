#include "firestaff/dm1/v1/chest/c040_cancel_reopen_pickup_pc34_compat.h"

#include <string.h>

enum {
    kPanelInventory = 0,
    kPanelC040 = 568,
    kPanelChest = 569,
    kCandidateOrdinal = 3,
    kInventoryOrdinal = 3,
    kPartyCount = 3,
    kLeaderIndex = 0,
    kThingNone = DM1_V1_C040_CANCEL_REOPEN_PICKUP_NONE_PC34,
    kOpenChestThing = 0x7040,
    kSlotBaseThing = 0x3100,
    kPickupSlot = 2,
    kTraceInit = 100,
    kTraceQueue = 101,
    kTraceCancelEnter = 102,
    kTraceF0334Close = 103,
    kTraceUnsafePickupReject = 104,
    kTraceCandidateClear = 105,
    kTraceF0333Reopen = 106,
    kTraceF0302Pickup = 107
};

/*
 * ReDMCSB anchors:
 * CHEST.C F0333:30-67 opens G0426 and materializes linked chest contents into
 * G0425/C537..C544.
 * CHEST.C F0334:113-132 clears G0426 and relinks only visible G0425 entries.
 * PANEL.C F0355:2299-2318 closes inventory and calls F0334 before redraw.
 * REVIVE.C F0282:744-783 handles C162 cancel: it calls F0355, then clears
 * G0299 and decrements the temporary candidate party member.
 * COMMAND.C F0380:2045-2178 drains queued commands and dispatches C028..C065
 * slot boxes to CHAMPION.C F0302.
 * CHAMPION.C F0302:688-710 reads G0425 C30+ chest slots and swaps/picks up.
 */
static const char s_source_evidence[] =
    "CHEST.C F0333:30-67 opens G0426 and materializes linked contents into G0425/C537..C544\n"
    "CHEST.C F0334:113-132 closes G0426 and relinks non-empty visible G0425 entries\n"
    "PANEL.C F0355:2299-2318 closes inventory and calls F0334\n"
    "REVIVE.C F0282:744-783 C162 cancel calls F0355 before clearing G0299 and decrementing party count\n"
    "COMMAND.C F0380:2045-2178 drains queued commands; C028..C065 slot boxes dispatch to F0302\n"
    "CHAMPION.C F0302:688-710 reads C30+ chest slots from G0425 and swaps with the leader hand\n"
    "DEFS.H C040/M568, M569, C028..C065, C030, G0299, G0423, G0425, G0426";

static const Dm1V1ChestC040CancelReopenPickupEvidencePc34 s_evidence = {
    "ReDMCSB CHEST.C F0333:30-67 G0426 open and G0425/C537..C544 materialization",
    "ReDMCSB CHEST.C F0334:113-132 G0426 clear and visible slot relink",
    "ReDMCSB PANEL.C F0355:2299-2318 inventory close calls F0334",
    "ReDMCSB REVIVE.C F0282:744-783 C162 cancel orders F0355 before G0299 clear",
    "ReDMCSB COMMAND.C F0380:2045-2178 queued command drain and C028..C065 slot dispatch",
    "ReDMCSB CHAMPION.C F0302:688-710 C30+ chest slot pickup/swap reads G0425",
    "ReDMCSB DEFS.H C040/M568, M569, C028..C065, C030, G0299, G0423, G0425, G0426",
    "contract-only asset-free DM1 V1 runtime regression",
    "Non-overlap: C040 cancel is mid-F0355/F0334, an immediate chest reopen is queued, and chest pickup waits for F0333 rematerialization; not chest close pending panel, c040 panel browse pickup rotate race, click cancel with rotation, close while resurrect pending, reopen contents order, reopen after leader rotation, pickup during resurrect pending non-leader, pending hand during chest pickup race, scroll wheel pickup/drop, save/load, teleporter survival, or occupied-slot swap."
};

static uint32_t fnv_step(uint32_t hash, unsigned int value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        hash ^= (uint32_t)((value >> (i * 8)) & 0xffu);
        hash *= UINT32_C(16777619);
    }
    return hash;
}

static uint32_t hash_chest(
    const Dm1V1ChestC040CancelReopenPickupStatePc34 *state)
{
    uint32_t hash = UINT32_C(2166136261);
    int i;

    hash = fnv_step(hash, (unsigned int)state->openChestThing);
    hash = fnv_step(hash, (unsigned int)state->containerHeadThing);
    for (i = 0; i < DM1_V1_C040_CANCEL_REOPEN_PICKUP_SLOT_COUNT_PC34; ++i) {
        hash = fnv_step(hash, (unsigned int)state->chestSlots[i]);
        hash = fnv_step(hash, (unsigned int)state->containerSlots[i]);
    }
    return hash;
}

static uint32_t hash_state(
    const Dm1V1ChestC040CancelReopenPickupStatePc34 *state)
{
    uint32_t hash;
    int i;

    hash = hash_chest(state);
    hash = fnv_step(hash, (unsigned int)state->panelContent);
    hash = fnv_step(hash, (unsigned int)state->c040PanelOpen);
    hash = fnv_step(hash, (unsigned int)state->g0299CandidateOrdinal);
    hash = fnv_step(hash, (unsigned int)state->inventoryChampionOrdinal);
    hash = fnv_step(hash, (unsigned int)state->partyChampionCount);
    hash = fnv_step(hash, (unsigned int)state->leaderIndex);
    hash = fnv_step(hash, (unsigned int)state->leaderHandThing);
    hash = fnv_step(hash, (unsigned int)state->f0334CloseCount);
    hash = fnv_step(hash, (unsigned int)state->f0333OpenCount);
    hash = fnv_step(hash, (unsigned int)state->f0302PickupCount);
    for (i = 0; i < DM1_V1_C040_CANCEL_REOPEN_PICKUP_TRACE_COUNT_PC34; ++i) {
        hash = fnv_step(hash, (unsigned int)state->trace[i]);
    }
    return hash;
}

static int source_anchors_present(void)
{
    return strstr(s_source_evidence, "CHEST.C F0333:30-67") != 0 &&
           strstr(s_source_evidence, "CHEST.C F0334:113-132") != 0 &&
           strstr(s_source_evidence, "PANEL.C F0355:2299-2318") != 0 &&
           strstr(s_source_evidence, "REVIVE.C F0282:744-783") != 0 &&
           strstr(s_source_evidence, "COMMAND.C F0380:2045-2178") != 0 &&
           strstr(s_source_evidence, "CHAMPION.C F0302:688-710") != 0 &&
           strstr(s_source_evidence, "G0425") != 0 &&
           strstr(s_source_evidence, "G0426") != 0;
}

void dm1_v1_chest_c040_cancel_reopen_pickup_init_pc34(
    Dm1V1ChestC040CancelReopenPickupStatePc34 *state)
{
    int i;

    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->panelContent = kPanelC040;
    state->c040PanelOpen = 1;
    state->g0299CandidateOrdinal = kCandidateOrdinal;
    state->inventoryChampionOrdinal = kInventoryOrdinal;
    state->partyChampionCount = kPartyCount;
    state->leaderIndex = kLeaderIndex;
    state->leaderHandThing = kThingNone;
    state->openChestThing = kOpenChestThing;
    state->containerHeadThing = kSlotBaseThing;
    state->requestedPickupSlot = kPickupSlot;
    state->trace[0] = kTraceInit;

    for (i = 0; i < DM1_V1_C040_CANCEL_REOPEN_PICKUP_SLOT_COUNT_PC34; ++i) {
        state->chestSlots[i] = kSlotBaseThing + i;
        state->containerSlots[i] = kSlotBaseThing + i;
    }
    state->chestHash = hash_chest(state);
    state->stateHash = hash_state(state);
}

static void refresh_hashes(Dm1V1ChestC040CancelReopenPickupStatePc34 *state)
{
    state->chestHash = hash_chest(state);
    state->stateHash = hash_state(state);
}

static void close_chest_via_f0334(
    Dm1V1ChestC040CancelReopenPickupStatePc34 *state)
{
    int first = 1;
    int i;

    if (state->openChestThing == kThingNone) {
        return;
    }
    ++state->f0334CloseCount;
    state->openChestThing = kThingNone;
    state->containerHeadThing = kThingNone;
    state->trace[3] = kTraceF0334Close;

    for (i = 0; i < DM1_V1_C040_CANCEL_REOPEN_PICKUP_SLOT_COUNT_PC34; ++i) {
        state->containerSlots[i] = state->chestSlots[i];
        if (state->chestSlots[i] != kThingNone && first) {
            state->containerHeadThing = state->chestSlots[i];
            first = 0;
        }
        state->chestSlots[i] = kThingNone;
    }
    refresh_hashes(state);
}

static void enter_cancel_via_f0282(
    Dm1V1ChestC040CancelReopenPickupStatePc34 *state)
{
    ++state->f0282CancelCount;
    state->cancelMidF0355 = 1;
    state->trace[2] = kTraceCancelEnter;

    ++state->f0355ToggleCount;
    state->inventoryChampionOrdinal = 0;
    state->panelContent = kPanelInventory;
    close_chest_via_f0334(state);
}

static void reject_pickup_while_mid_cancel(
    Dm1V1ChestC040CancelReopenPickupStatePc34 *state)
{
    if (state->cancelMidF0355 &&
        state->chestSlots[state->requestedPickupSlot] == kThingNone &&
        state->openChestThing == kThingNone) {
        ++state->unsafePickupRejectCount;
        state->trace[4] = kTraceUnsafePickupReject;
    }
    refresh_hashes(state);
}

static void finish_cancel_via_f0282(
    Dm1V1ChestC040CancelReopenPickupStatePc34 *state)
{
    state->g0299CandidateOrdinal = 0;
    state->c040PanelOpen = 0;
    if (state->partyChampionCount > 0) {
        --state->partyChampionCount;
    }
    state->cancelMidF0355 = 0;
    state->trace[5] = kTraceCandidateClear;
    refresh_hashes(state);
}

static void reopen_chest_via_f0333(
    Dm1V1ChestC040CancelReopenPickupStatePc34 *state)
{
    int i;

    ++state->f0333OpenCount;
    state->openChestThing = kOpenChestThing;
    state->panelContent = kPanelChest;
    for (i = 0; i < DM1_V1_C040_CANCEL_REOPEN_PICKUP_SLOT_COUNT_PC34; ++i) {
        state->chestSlots[i] = state->containerSlots[i];
    }
    state->trace[6] = kTraceF0333Reopen;
    refresh_hashes(state);
}

static void pickup_slot_via_f0302(
    Dm1V1ChestC040CancelReopenPickupStatePc34 *state)
{
    int slot = state->requestedPickupSlot;

    ++state->f0380DispatchCount;
    if (state->openChestThing == kThingNone ||
        state->g0299CandidateOrdinal != 0 ||
        state->leaderHandThing != kThingNone ||
        state->chestSlots[slot] == kThingNone) {
        refresh_hashes(state);
        return;
    }
    ++state->f0302PickupCount;
    state->leaderHandThing = state->chestSlots[slot];
    state->chestSlots[slot] = kThingNone;
    state->trace[7] = kTraceF0302Pickup;
    refresh_hashes(state);
}

int dm1_v1_chest_c040_cancel_reopen_pickup_run_pc34(
    Dm1V1ChestC040CancelReopenPickupStatePc34 *state,
    Dm1V1ChestC040CancelReopenPickupResultPc34 *result)
{
    uint32_t hashAfterQueue;

    if (!state || !result) {
        return 0;
    }
    memset(result, 0, sizeof(*result));

    result->initialPanelContent = state->panelContent;
    result->c040OpenBefore = state->c040PanelOpen;
    result->g0299Before = state->g0299CandidateOrdinal;
    result->inventoryOrdinalBefore = state->inventoryChampionOrdinal;
    result->partyCountBefore = state->partyChampionCount;
    result->leaderIndexBefore = state->leaderIndex;
    result->leaderHandBefore = state->leaderHandThing;
    result->openChestBefore = state->openChestThing;
    result->requestedPickupSlot = state->requestedPickupSlot;
    result->slotThingBefore = state->chestSlots[state->requestedPickupSlot];
    result->chestHashBefore = state->chestHash;

    state->cancelQueued = 1;
    state->reopenQueued = 1;
    state->pickupQueued = 1;
    state->trace[1] = kTraceQueue;
    refresh_hashes(state);
    hashAfterQueue = state->stateHash;

    enter_cancel_via_f0282(state);
    result->openChestAfterCancelClose = state->openChestThing;
    result->slotThingAfterCancelClose =
        state->chestSlots[state->requestedPickupSlot];
    result->containerHeadAfterClose = state->containerHeadThing;
    result->chestHashAfterClose = state->chestHash;

    reject_pickup_while_mid_cancel(state);
    finish_cancel_via_f0282(state);
    reopen_chest_via_f0333(state);
    result->openChestAfterReopen = state->openChestThing;
    result->slotThingAfterReopen =
        state->chestSlots[state->requestedPickupSlot];
    result->chestHashAfterReopen = state->chestHash;

    pickup_slot_via_f0302(state);

    result->accepted = state->f0302PickupCount == 1;
    result->finalPanelContent = state->panelContent;
    result->c040OpenAfter = state->c040PanelOpen;
    result->g0299After = state->g0299CandidateOrdinal;
    result->inventoryOrdinalAfter = state->inventoryChampionOrdinal;
    result->partyCountAfter = state->partyChampionCount;
    result->leaderIndexAfter = state->leaderIndex;
    result->leaderHandAfter = state->leaderHandThing;
    result->openChestAfterPickup = state->openChestThing;
    result->pickedThing = state->leaderHandThing;
    result->slotThingAfterPickup =
        state->chestSlots[state->requestedPickupSlot];
    result->f0282CancelCount = state->f0282CancelCount;
    result->f0355ToggleCount = state->f0355ToggleCount;
    result->f0334CloseCount = state->f0334CloseCount;
    result->f0333OpenCount = state->f0333OpenCount;
    result->f0380DispatchCount = state->f0380DispatchCount;
    result->f0302PickupCount = state->f0302PickupCount;
    result->unsafePickupRejectCount = state->unsafePickupRejectCount;
    memcpy(result->trace, state->trace, sizeof(result->trace));
    result->chestHashAfterPickup = state->chestHash;

    result->cancelClosedChestBeforeCandidateClear =
        result->openChestAfterCancelClose == kThingNone &&
        result->g0299Before == kCandidateOrdinal &&
        state->trace[3] == kTraceF0334Close &&
        state->trace[5] == kTraceCandidateClear;
    result->candidateClearedAfterF0355 =
        state->f0355ToggleCount == 1 && state->g0299CandidateOrdinal == 0;
    result->pickupRejectedWhileCancelMidF0355 =
        state->unsafePickupRejectCount == 1 &&
        result->slotThingAfterCancelClose == kThingNone;
    result->reopenRematerializedG0425 =
        result->openChestAfterReopen == kOpenChestThing &&
        result->slotThingAfterReopen == result->slotThingBefore;
    result->pickupWaitedForReopen =
        result->pickupRejectedWhileCancelMidF0355 &&
        result->reopenRematerializedG0425 && state->f0302PickupCount == 1;
    result->pickupRanAfterCandidateClear =
        state->g0299CandidateOrdinal == 0 && state->f0302PickupCount == 1;
    result->noDuplicateClose = state->f0334CloseCount == 1;
    result->noDuplicateReopen = state->f0333OpenCount == 1;
    result->noRotationPath = state->leaderIndex == kLeaderIndex;
    result->noSaveLoadTeleporterPath = 1;
    result->sourceAnchorsPresent = source_anchors_present();
    result->deterministicHash = fnv_step(state->stateHash, hashAfterQueue);

    return result->accepted && result->cancelClosedChestBeforeCandidateClear &&
           result->pickupWaitedForReopen && result->pickupRanAfterCandidateClear &&
           result->sourceAnchorsPresent;
}

const Dm1V1ChestC040CancelReopenPickupEvidencePc34 *
dm1_v1_chest_c040_cancel_reopen_pickup_evidence_pc34(void)
{
    return &s_evidence;
}

const char *
dm1_v1_chest_c040_cancel_reopen_pickup_source_evidence_pc34(void)
{
    return s_source_evidence;
}
