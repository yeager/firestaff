#include "dm1_v1_mirror_candidate_double_open_close_guard_pc34_compat.h"

#include <string.h>

/* ReDMCSB: CHAMDRAW.C F0291_CHAMPION_DrawSlot line ~551-552 reads C30+
 * chest slots through G0425, and F0296_CHAMPION_DrawChangedObjectIcons line
 * ~1249-1252 redraws open-chest slot boxes only from that close/relink order.
 * ReDMCSB: CHAMPION.C F0297_CHAMPION_PutObjectInLeaderHand line ~243-298
 * and F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox line ~662-710 are
 * the leader-hand and occupied-slot routes guarded out of these panel no-ops.
 * ReDMCSB: COMMAND.C F0359_COMMAND_ProcessClick_CPSC line ~1985-1990 routes
 * M568/C040 panel clicks to REVIVE.C F0282 only when the leader hand is empty.
 * ReDMCSB: REVIVE.C F0282 line ~744-806 clears G0299 on C162 and then runs
 * resurrect/reincarnate finalization; double open/close must not re-enter it.
 * ReDMCSB: DEFS.H C30/G0425/G0426/M070/M516/C040/C162/M568 bind the slot,
 * chest, champion, and resurrect/reincarnate panel constants modeled here.
 */

enum {
    kInitialCandidateOrdinal = 4,
    kInitialPartyCount = 2,
    kCandidatePartyCount = 3,
    kLeaderHandThing = 0x0444,
    kLeaderHandQueueThing = 0x0555,
    kOpenChestThing = 0x0700
};

static const Dm1V1MirrorCandidateDoubleOpenCloseGuardEvidencePc34Compat
    s_evidence = {
        1,
        "runtime-only non-duplicative guard for C040 double-open, double-close, "
        "pending chest-close hand queue, and same-tick inventory click order",
        "CHAMDRAW.C F0291_CHAMPION_DrawSlot:551-552 C30/G0425 chest slot read",
        "CHAMDRAW.C F0296_CHAMPION_DrawChangedObjectIcons:1249-1252 chest slot redraw order",
        "CHAMPION.C F0297_CHAMPION_PutObjectInLeaderHand:243-298 leader-hand put/remove",
        "CHAMPION.C F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox:662-710 occupied-slot click dispatch",
        "COMMAND.C F0359_COMMAND_ProcessClick_CPSC:1985-1990 M568/C040 panel dispatch",
        "REVIVE.C F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel:744-806 candidate clear/finalize",
        "DEFS.H C30/G0425/G0426/M070/M516/C040/C162/M568"
    };

static void set_slot_orders(
    Dm1V1MirrorCandidateDoubleOpenCloseGuardStatePc34Compat *state)
{
    state->closeSlotOrder[0] = 0;
    state->closeSlotOrder[1] = 2;
    state->closeSlotOrder[2] = 3;
    state->closeSlotOrder[3] =
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_NONE_PC34;
    state->clickSlotOrder[0] = 1;
    state->clickSlotOrder[1] = 0;
    state->clickSlotOrder[2] = 2;
    state->clickSlotOrder[3] =
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_NONE_PC34;
    memcpy(state->usedSlotOrder,
           state->clickSlotOrder,
           sizeof(state->usedSlotOrder));
}

static void fill_chest_slots(
    Dm1V1MirrorCandidateDoubleOpenCloseGuardStatePc34Compat *state)
{
    state->chestSlots[0] = 0x0101;
    state->chestSlots[1] =
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_NONE_PC34;
    state->chestSlots[2] = 0x0102;
    state->chestSlots[3] = 0x0103;
}

static void init_common(
    Dm1V1MirrorCandidateDoubleOpenCloseGuardStatePc34Compat *state)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->contractOnly = 1;
    state->partyChampionCount = kInitialPartyCount;
    state->leaderHandThing = kLeaderHandThing;
    state->leaderHandQueueThing = kLeaderHandQueueThing;
    state->openChestThing = kOpenChestThing;
    fill_chest_slots(state);
    set_slot_orders(state);
}

void dm1_v1_mirror_candidate_double_open_close_guard_init_open_pc34_compat(
    Dm1V1MirrorCandidateDoubleOpenCloseGuardStatePc34Compat *state)
{
    init_common(state);
    if (!state) {
        return;
    }
    state->panelContent =
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_M568_C040_PC34;
    state->c040PanelOpen = 1;
    state->candidateChampionOrdinal = kInitialCandidateOrdinal;
    state->inventoryChampionOrdinal = kInitialCandidateOrdinal;
    state->partyChampionCount = kCandidatePartyCount;
}

void dm1_v1_mirror_candidate_double_open_close_guard_init_closed_pc34_compat(
    Dm1V1MirrorCandidateDoubleOpenCloseGuardStatePc34Compat *state)
{
    init_common(state);
    if (!state) {
        return;
    }
    state->panelContent =
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_M569_CHEST_PC34;
    state->c040PanelOpen = 0;
}

static unsigned int event_candidate(
    const Dm1V1MirrorCandidateDoubleOpenCloseGuardEventPc34Compat *event)
{
    if (event && event->candidateChampionOrdinal != 0u) {
        return event->candidateChampionOrdinal;
    }
    return kInitialCandidateOrdinal;
}

static void open_c040_panel(
    Dm1V1MirrorCandidateDoubleOpenCloseGuardStatePc34Compat *state,
    const Dm1V1MirrorCandidateDoubleOpenCloseGuardEventPc34Compat *event)
{
    if (state->c040PanelOpen) {
        ++state->duplicateOpenNoopCount;
        return;
    }
    state->panelContent =
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_M568_C040_PC34;
    state->c040PanelOpen = 1;
    state->candidateChampionOrdinal = event_candidate(event);
    state->inventoryChampionOrdinal = state->candidateChampionOrdinal;
    state->partyChampionCount = kCandidatePartyCount;
    ++state->openDispatchCount;
}

static void close_chest_in_close_order(
    Dm1V1MirrorCandidateDoubleOpenCloseGuardStatePc34Compat *state)
{
    if (state->openChestThing ==
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_NONE_PC34) {
        return;
    }
    ++state->f0334ChestCloseCount;
    memcpy(state->usedSlotOrder,
           state->closeSlotOrder,
           sizeof(state->usedSlotOrder));
    state->openChestThing =
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_NONE_PC34;
}

static void close_c040_panel(
    Dm1V1MirrorCandidateDoubleOpenCloseGuardStatePc34Compat *state)
{
    if (!state->c040PanelOpen) {
        ++state->duplicateCloseNoopCount;
        return;
    }
    ++state->f0355InventoryCloseCount;
    close_chest_in_close_order(state);
    state->candidateChampionOrdinal = 0u;
    state->inventoryChampionOrdinal = 0u;
    state->partyChampionCount = kInitialPartyCount;
    state->c040PanelOpen = 0;
    state->panelContent = 0;
    ++state->f0282CandidateClearCount;
    ++state->panelZeroCount;
    ++state->closeDispatchCount;
}

static void arm_pending_open_from_chest_close(
    Dm1V1MirrorCandidateDoubleOpenCloseGuardStatePc34Compat *state,
    const Dm1V1MirrorCandidateDoubleOpenCloseGuardEventPc34Compat *event)
{
    ++state->f0334ChestCloseCount;
    state->openChestThing =
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_NONE_PC34;
    state->pendingOpenArmed = 1;
    state->pendingCandidateChampionOrdinal = event_candidate(event);
}

static void flush_pending_open(
    Dm1V1MirrorCandidateDoubleOpenCloseGuardStatePc34Compat *state)
{
    Dm1V1MirrorCandidateDoubleOpenCloseGuardEventPc34Compat event;

    if (!state->pendingOpenArmed) {
        return;
    }
    memset(&event, 0, sizeof(event));
    event.kind =
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_EVENT_C040_OPEN_PC34;
    event.candidateChampionOrdinal = state->pendingCandidateChampionOrdinal;
    state->pendingOpenArmed = 0;
    state->pendingCandidateChampionOrdinal = 0u;
    open_c040_panel(state, &event);
    ++state->pendingOpenFlushCount;
}

static void inventory_portrait_click(
    Dm1V1MirrorCandidateDoubleOpenCloseGuardStatePc34Compat *state,
    int closeProcessedThisTick)
{
    ++state->inventoryPortraitClickCount;
    if (closeProcessedThisTick) {
        ++state->sameTickCloseSlotOrderCount;
        return;
    }
    ++state->clickSlotOrderCount;
    memcpy(state->usedSlotOrder,
           state->clickSlotOrder,
           sizeof(state->usedSlotOrder));
    ++state->f0302SlotDispatchCount;
    ++state->f0297LeaderHandPutCount;
}

static void capture_before(
    const Dm1V1MirrorCandidateDoubleOpenCloseGuardStatePc34Compat *state,
    Dm1V1MirrorCandidateDoubleOpenCloseGuardResultPc34Compat *result)
{
    memset(result, 0, sizeof(*result));
    result->evidence = &s_evidence;
    if (!state) {
        return;
    }
    result->panelContentBefore = state->panelContent;
    result->c040PanelOpenBefore = state->c040PanelOpen;
    result->candidateBefore = state->candidateChampionOrdinal;
    result->inventoryBefore = state->inventoryChampionOrdinal;
    result->partyCountBefore = state->partyChampionCount;
    result->leaderHandBefore = state->leaderHandThing;
    result->leaderHandQueueBefore = state->leaderHandQueueThing;
    result->openChestBefore = state->openChestThing;
}

static int same_order(const int *a, const int *b)
{
    int i;

    for (i = 0;
         i <
         DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_SLOT_COUNT_PC34;
         ++i) {
        if (a[i] != b[i]) {
            return 0;
        }
    }
    return 1;
}

static void capture_after(
    const Dm1V1MirrorCandidateDoubleOpenCloseGuardStatePc34Compat *state,
    Dm1V1MirrorCandidateDoubleOpenCloseGuardResultPc34Compat *result)
{
    if (!state || !result) {
        return;
    }
    result->panelContentAfter = state->panelContent;
    result->c040PanelOpenAfter = state->c040PanelOpen;
    result->candidateAfter = state->candidateChampionOrdinal;
    result->inventoryAfter = state->inventoryChampionOrdinal;
    result->partyCountAfter = state->partyChampionCount;
    result->leaderHandAfter = state->leaderHandThing;
    result->leaderHandQueueAfter = state->leaderHandQueueThing;
    result->openChestAfter = state->openChestThing;
    result->openDispatchCount = state->openDispatchCount;
    result->closeDispatchCount = state->closeDispatchCount;
    result->duplicateOpenNoopCount = state->duplicateOpenNoopCount;
    result->duplicateCloseNoopCount = state->duplicateCloseNoopCount;
    result->f0282CandidateClearCount = state->f0282CandidateClearCount;
    result->f0297LeaderHandPutCount = state->f0297LeaderHandPutCount;
    result->f0302SlotDispatchCount = state->f0302SlotDispatchCount;
    result->f0334ChestCloseCount = state->f0334ChestCloseCount;
    result->f0355InventoryCloseCount = state->f0355InventoryCloseCount;
    result->panelZeroCount = state->panelZeroCount;
    result->leaderHandQueueClearCount = state->leaderHandQueueClearCount;
    result->pendingOpenFlushCount = state->pendingOpenFlushCount;
    result->inventoryPortraitClickCount = state->inventoryPortraitClickCount;
    result->sameTickCloseSlotOrderCount = state->sameTickCloseSlotOrderCount;
    result->clickSlotOrderCount = state->clickSlotOrderCount;
    memcpy(result->usedSlotOrder,
           state->usedSlotOrder,
           sizeof(result->usedSlotOrder));
    result->doubleOpenWasNoop =
        state->duplicateOpenNoopCount > 0 &&
        state->openDispatchCount == 0 &&
        state->c040PanelOpen;
    result->doubleOpenPreservedLeaderHand =
        result->leaderHandBefore == result->leaderHandAfter &&
        state->f0297LeaderHandPutCount == 0;
    result->doubleOpenDidNotClearCandidate =
        result->candidateBefore == result->candidateAfter &&
        state->f0282CandidateClearCount == 0;
    result->doubleCloseWasNoop = state->duplicateCloseNoopCount > 0;
    result->doubleCloseDidNotClearCandidateAgain =
        state->f0282CandidateClearCount <= 1;
    result->doubleClosePreservedClosedPanelState =
        !result->c040PanelOpenBefore &&
        !result->c040PanelOpenAfter &&
        result->panelContentBefore == result->panelContentAfter &&
        state->panelZeroCount == 0;
    result->closeDuringPendingPreservedLeaderHandQueue =
        result->leaderHandQueueBefore == result->leaderHandQueueAfter &&
        state->leaderHandQueueClearCount == 0;
    result->closeDuringPendingDidNotClearCandidate =
        state->f0282CandidateClearCount == 0;
    result->closeDuringPendingOpenedCandidate =
        state->pendingOpenFlushCount == 1 &&
        state->c040PanelOpen &&
        state->candidateChampionOrdinal != 0u;
    result->inventoryClickUsedCloseSlotOrder =
        state->sameTickCloseSlotOrderCount == 1 &&
        same_order(state->usedSlotOrder, state->closeSlotOrder);
    result->inventoryClickDidNotUseClickSlotOrder =
        state->clickSlotOrderCount == 0 &&
        !same_order(state->usedSlotOrder, state->clickSlotOrder);
    result->inventoryClickDidNotDispatchF0302 =
        state->f0302SlotDispatchCount == 0 &&
        state->f0297LeaderHandPutCount == 0;
}

int dm1_v1_mirror_candidate_double_open_close_guard_run_events_pc34_compat(
    const Dm1V1MirrorCandidateDoubleOpenCloseGuardStatePc34Compat *initial,
    const Dm1V1MirrorCandidateDoubleOpenCloseGuardEventPc34Compat *events,
    unsigned int eventCount,
    Dm1V1MirrorCandidateDoubleOpenCloseGuardResultPc34Compat *outResult)
{
    Dm1V1MirrorCandidateDoubleOpenCloseGuardStatePc34Compat state;
    Dm1V1MirrorCandidateDoubleOpenCloseGuardResultPc34Compat localResult;
    Dm1V1MirrorCandidateDoubleOpenCloseGuardResultPc34Compat *result =
        outResult ? outResult : &localResult;
    unsigned int index;

    if (!initial || !events || !initial->contractOnly) {
        return 0;
    }
    state = *initial;
    capture_before(initial, result);
    for (index = 0u; index < eventCount;) {
        int tick = events[index].tick;
        unsigned int start = index;
        unsigned int end = index;
        int closeProcessedThisTick = 0;

        while (end < eventCount && events[end].tick == tick) {
            ++end;
        }
        for (index = start; index < end; ++index) {
            if (events[index].kind ==
                DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_EVENT_CHEST_CLOSE_PENDING_PC34) {
                arm_pending_open_from_chest_close(&state, &events[index]);
            }
        }
        for (index = start; index < end; ++index) {
            if (events[index].kind ==
                DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_EVENT_C040_OPEN_PC34) {
                open_c040_panel(&state, &events[index]);
            }
        }
        for (index = start; index < end; ++index) {
            if (events[index].kind ==
                DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_EVENT_C040_CLOSE_PC34) {
                int closeCountBefore = state.closeDispatchCount;
                close_c040_panel(&state);
                if (state.closeDispatchCount != closeCountBefore) {
                    closeProcessedThisTick = 1;
                }
            }
        }
        for (index = start; index < end; ++index) {
            if (events[index].kind ==
                DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_EVENT_INVENTORY_PORTRAIT_CLICK_PC34) {
                inventory_portrait_click(&state, closeProcessedThisTick);
            }
        }
        flush_pending_open(&state);
        index = end;
    }
    result->eventsProcessed = (int)eventCount;
    capture_after(&state, result);
    return 1;
}

static Dm1V1MirrorCandidateDoubleOpenCloseGuardEventPc34Compat event_of(
    int kind,
    int tick)
{
    Dm1V1MirrorCandidateDoubleOpenCloseGuardEventPc34Compat event;

    memset(&event, 0, sizeof(event));
    event.kind = kind;
    event.tick = tick;
    event.candidateChampionOrdinal = kInitialCandidateOrdinal;
    event.requestedSlotIndex =
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_C040_SLOT_PC34;
    return event;
}

int dm1_v1_mirror_candidate_double_open_close_guard_run_double_open_pc34_compat(
    Dm1V1MirrorCandidateDoubleOpenCloseGuardResultPc34Compat *outResult)
{
    Dm1V1MirrorCandidateDoubleOpenCloseGuardStatePc34Compat state;
    Dm1V1MirrorCandidateDoubleOpenCloseGuardEventPc34Compat events[2];

    dm1_v1_mirror_candidate_double_open_close_guard_init_open_pc34_compat(
        &state);
    events[0] = event_of(
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_EVENT_C040_OPEN_PC34,
        10);
    events[1] = event_of(
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_EVENT_C040_OPEN_PC34,
        11);
    return dm1_v1_mirror_candidate_double_open_close_guard_run_events_pc34_compat(
        &state, events, 2u, outResult);
}

int dm1_v1_mirror_candidate_double_open_close_guard_run_double_close_pc34_compat(
    Dm1V1MirrorCandidateDoubleOpenCloseGuardResultPc34Compat *outResult)
{
    Dm1V1MirrorCandidateDoubleOpenCloseGuardStatePc34Compat state;
    Dm1V1MirrorCandidateDoubleOpenCloseGuardEventPc34Compat events[1];

    dm1_v1_mirror_candidate_double_open_close_guard_init_closed_pc34_compat(
        &state);
    events[0] = event_of(
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_EVENT_C040_CLOSE_PC34,
        20);
    return dm1_v1_mirror_candidate_double_open_close_guard_run_events_pc34_compat(
        &state, events, 1u, outResult);
}

int dm1_v1_mirror_candidate_double_open_close_guard_run_close_during_pending_pc34_compat(
    Dm1V1MirrorCandidateDoubleOpenCloseGuardResultPc34Compat *outResult)
{
    Dm1V1MirrorCandidateDoubleOpenCloseGuardStatePc34Compat state;
    Dm1V1MirrorCandidateDoubleOpenCloseGuardEventPc34Compat events[2];

    dm1_v1_mirror_candidate_double_open_close_guard_init_closed_pc34_compat(
        &state);
    events[0] = event_of(
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_EVENT_CHEST_CLOSE_PENDING_PC34,
        30);
    events[1] = event_of(
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_EVENT_C040_CLOSE_PC34,
        30);
    return dm1_v1_mirror_candidate_double_open_close_guard_run_events_pc34_compat(
        &state, events, 2u, outResult);
}

int dm1_v1_mirror_candidate_double_open_close_guard_run_inventory_click_during_close_pc34_compat(
    Dm1V1MirrorCandidateDoubleOpenCloseGuardResultPc34Compat *outResult)
{
    Dm1V1MirrorCandidateDoubleOpenCloseGuardStatePc34Compat state;
    Dm1V1MirrorCandidateDoubleOpenCloseGuardEventPc34Compat events[2];

    dm1_v1_mirror_candidate_double_open_close_guard_init_open_pc34_compat(
        &state);
    events[0] = event_of(
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_EVENT_C040_CLOSE_PC34,
        40);
    events[1] = event_of(
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_EVENT_INVENTORY_PORTRAIT_CLICK_PC34,
        40);
    return dm1_v1_mirror_candidate_double_open_close_guard_run_events_pc34_compat(
        &state, events, 2u, outResult);
}

const Dm1V1MirrorCandidateDoubleOpenCloseGuardEvidencePc34Compat *
dm1_v1_mirror_candidate_double_open_close_guard_evidence_pc34_compat(void)
{
    return &s_evidence;
}
