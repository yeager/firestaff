#include "dm1_v1_mirror_candidate_pending_hand_queue_pc34_compat.h"

#include <string.h>

/* ReDMCSB source-lock anchors:
 * CHEST.C F0333:30-32 is the already-open chest guard; this gate verifies that
 * an open-chest refresh cannot disturb pending hand-swap queue entries.
 * CHEST.C F0334:113-132 clears G0426/G0425 and relinks non-empty chest slots;
 * this gate closes a chest mid-queue and preserves each submitted slot index.
 * COMMAND.C F0359:1985-1990 dispatches C040 mirror-candidate panel commands;
 * this gate routes queued hand swaps while C040 remains pending.
 * REVIVE.C F0280:124-132 keeps candidate admission on the live-candidate path;
 * this gate requires the candidate ordinal to stay alive while the queue drains.
 */

enum {
    kCandidateOrdinal = 2,
    kPartyCountWithCandidate = 2,
    kFirstChestSlotCommand = 38
};

static const Dm1V1MirrorCandidatePendingHandQueueEvidencePc34Compat s_evidence = {
    "ReDMCSB CHEST.C F0333:30-32 already-open chest guard returns before "
    "visible G0425 chest slots are reorganized",
    "ReDMCSB CHEST.C F0334:113-132 close path clears G0426/G0425 and relinks "
    "non-empty chest contents after hand-swap-visible slots",
    "ReDMCSB COMMAND.C F0359:1985-1990 mirror candidate C040 panel dispatch "
    "routes pending hand-queue work in original click order",
    "ReDMCSB REVIVE.C F0280:124-132 alive candidate guard remains set while "
    "candidate hand commands are routed",
    "contract-only synthetic runtime regression; no real chest, dungeon, "
    "bitmap, savegame, or asset data is loaded",
    "non-overlap: covers five pending hand-swap queue entries, a same-chest "
    "open guard, and an F0334 close between submission and drain; distinct "
    "from occupied hand, candidate-chest-close-pending-panel, and walkpath gates"
};

static int valid_slot_index(int slotIndex)
{
    return slotIndex >= 0 &&
           slotIndex <
               DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_SLOT_COUNT_PC34_COMPAT;
}

static int queue_tail(
    const Dm1V1MirrorCandidatePendingHandQueueStatePc34Compat *state)
{
    return (state->pendingHead + state->pendingCount) %
           DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_CAPACITY_PC34_COMPAT;
}

static void fill_fixture_slots(int slots[
    DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_SLOT_COUNT_PC34_COMPAT])
{
    slots[0] = DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_SLOT0_THING_PC34_COMPAT;
    slots[1] = DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_SLOT1_THING_PC34_COMPAT;
    slots[2] = DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_SLOT2_THING_PC34_COMPAT;
    slots[3] = DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_SLOT3_THING_PC34_COMPAT;
    slots[4] = DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_SLOT4_THING_PC34_COMPAT;
    slots[5] = DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_NONE_PC34_COMPAT;
    slots[6] = DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_NONE_PC34_COMPAT;
    slots[7] = DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_NONE_PC34_COMPAT;
}

static void capture_before(
    const Dm1V1MirrorCandidatePendingHandQueueStatePc34Compat *state,
    int slotIndex,
    Dm1V1MirrorCandidatePendingHandQueueResultPc34Compat *result)
{
    memset(result, 0, sizeof(*result));
    result->evidence = &s_evidence;
    result->slotIndex = slotIndex;
    result->drainedSlotIndex =
        DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_NONE_PC34_COMPAT;
    result->drainedSubmissionOrdinal =
        DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_NONE_PC34_COMPAT;
    if (!state) {
        result->ignored = 1;
        return;
    }
    result->command = valid_slot_index(slotIndex) ?
        kFirstChestSlotCommand + slotIndex :
        DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_NONE_PC34_COMPAT;
    result->pendingCountBefore = state->pendingCount;
    result->pendingCountAfter = state->pendingCount;
    result->queueSubmitCountBefore = state->queueSubmitCount;
    result->queueSubmitCountAfter = state->queueSubmitCount;
    result->handQueueDispatchCountBefore = state->handQueueDispatchCount;
    result->handQueueDispatchCountAfter = state->handQueueDispatchCount;
    result->candidateAliveGuardPassCountBefore =
        state->candidateAliveGuardPassCount;
    result->candidateAliveGuardPassCountAfter =
        state->candidateAliveGuardPassCount;
    result->candidateAliveGuardBlockCountBefore =
        state->candidateAliveGuardBlockCount;
    result->candidateAliveGuardBlockCountAfter =
        state->candidateAliveGuardBlockCount;
    result->alreadyOpenGuardCountBefore = state->alreadyOpenGuardCount;
    result->alreadyOpenGuardCountAfter = state->alreadyOpenGuardCount;
    result->f0334CloseCountBefore = state->f0334CloseCount;
    result->f0334CloseCountAfter = state->f0334CloseCount;
    result->closeDuringPendingQueueCountBefore =
        state->closeDuringPendingQueueCount;
    result->closeDuringPendingQueueCountAfter =
        state->closeDuringPendingQueueCount;
    result->chestSlotClearCountBefore = state->chestSlotClearCount;
    result->chestSlotClearCountAfter = state->chestSlotClearCount;
    result->chestFirstSlotWriteCountBefore = state->chestFirstSlotWriteCount;
    result->chestFirstSlotWriteCountAfter = state->chestFirstSlotWriteCount;
    result->chestRelinkCountBefore = state->chestRelinkCount;
    result->chestRelinkCountAfter = state->chestRelinkCount;
    result->slotIndexLostCountBefore = state->slotIndexLostCount;
    result->slotIndexLostCountAfter = state->slotIndexLostCount;
    result->orderViolationCountBefore = state->orderViolationCount;
    result->orderViolationCountAfter = state->orderViolationCount;
    result->openChestBefore = state->openChestThing;
    result->openChestAfter = state->openChestThing;
    result->containerHeadBefore = state->containerHeadThing;
    result->containerHeadAfter = state->containerHeadThing;
    result->candidateOrdinalBefore = (int)state->candidateChampionOrdinal;
    result->candidateOrdinalAfter = (int)state->candidateChampionOrdinal;
    result->partyCountBefore = (int)state->partyChampionCount;
    result->partyCountAfter = (int)state->partyChampionCount;
    result->panelContentBefore = state->panelContent;
    result->panelContentAfter = state->panelContent;
    result->c040OpenBefore = state->c040PanelOpen;
    result->c040OpenAfter = state->c040PanelOpen;
}

static void capture_after(
    const Dm1V1MirrorCandidatePendingHandQueueStatePc34Compat *state,
    Dm1V1MirrorCandidatePendingHandQueueResultPc34Compat *result)
{
    int i;
    int pendingOk = 1;
    int drainedOk = 1;

    if (!state || !result) {
        return;
    }
    result->pendingCountAfter = state->pendingCount;
    result->queueSubmitCountAfter = state->queueSubmitCount;
    result->handQueueDispatchCountAfter = state->handQueueDispatchCount;
    result->candidateAliveGuardPassCountAfter =
        state->candidateAliveGuardPassCount;
    result->candidateAliveGuardBlockCountAfter =
        state->candidateAliveGuardBlockCount;
    result->alreadyOpenGuardCountAfter = state->alreadyOpenGuardCount;
    result->f0334CloseCountAfter = state->f0334CloseCount;
    result->closeDuringPendingQueueCountAfter =
        state->closeDuringPendingQueueCount;
    result->chestSlotClearCountAfter = state->chestSlotClearCount;
    result->chestFirstSlotWriteCountAfter = state->chestFirstSlotWriteCount;
    result->chestRelinkCountAfter = state->chestRelinkCount;
    result->slotIndexLostCountAfter = state->slotIndexLostCount;
    result->orderViolationCountAfter = state->orderViolationCount;
    result->openChestAfter = state->openChestThing;
    result->containerHeadAfter = state->containerHeadThing;
    result->candidateOrdinalAfter = (int)state->candidateChampionOrdinal;
    result->partyCountAfter = (int)state->partyChampionCount;
    result->panelContentAfter = state->panelContent;
    result->c040OpenAfter = state->c040PanelOpen;
    result->candidateAlivePreserved =
        result->candidateOrdinalBefore == result->candidateOrdinalAfter &&
        result->partyCountBefore == result->partyCountAfter &&
        result->panelContentBefore == result->panelContentAfter &&
        result->c040OpenBefore == result->c040OpenAfter;
    for (i = 0; i < state->pendingCount; ++i) {
        const int pendingIndex =
            (state->pendingHead + i) %
            DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_CAPACITY_PC34_COMPAT;
        const Dm1V1MirrorCandidatePendingHandQueueEntryPc34Compat *entry =
            &state->pending[pendingIndex];
        if (entry->submissionOrdinal < 1 ||
            entry->submissionOrdinal >
                DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_CAPACITY_PC34_COMPAT ||
            state->submittedSlots[entry->submissionOrdinal - 1] !=
                entry->slotIndex) {
            pendingOk = 0;
        }
    }
    for (i = 0; i < state->handQueueDispatchCount; ++i) {
        if (state->drainedOrdinals[i] != i + 1 ||
            state->drainedSlots[i] != state->submittedSlots[i]) {
            drainedOk = 0;
        }
    }
    result->pendingSlotIndicesPreserved =
        pendingOk &&
        state->slotIndexLostCount == result->slotIndexLostCountBefore;
    result->queueOrderPreserved =
        drainedOk &&
        state->orderViolationCount == result->orderViolationCountBefore;
    result->chestCloseRepackedSlots =
        result->f0334CloseCountAfter == result->f0334CloseCountBefore + 1 &&
        result->openChestAfter ==
            DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_NONE_PC34_COMPAT &&
        result->containerHeadAfter ==
            DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_SLOT0_THING_PC34_COMPAT &&
        result->chestFirstSlotWriteCountAfter ==
            result->chestFirstSlotWriteCountBefore + 1 &&
        result->chestRelinkCountAfter == result->chestRelinkCountBefore + 4 &&
        result->chestSlotClearCountAfter ==
            result->chestSlotClearCountBefore + 5;
    result->alreadyOpenPreservedQueue =
        result->alreadyOpenGuardCountAfter ==
            result->alreadyOpenGuardCountBefore + 1 &&
        result->pendingCountAfter == result->pendingCountBefore &&
        result->openChestAfter == result->openChestBefore;
}

void DM1_V1_MirrorCandidatePendingHandQueue_InitPc34Compat(
    Dm1V1MirrorCandidatePendingHandQueueStatePc34Compat *state)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->panelContent =
        DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_C040_PANEL_PC34_COMPAT;
    state->c040PanelOpen = 1;
    state->candidateChampionOrdinal = kCandidateOrdinal;
    state->partyChampionCount = kPartyCountWithCandidate;
    state->leaderHandThing =
        DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_LEADER_HAND_PC34_COMPAT;
    state->openChestThing =
        DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_OPEN_CHEST_PC34_COMPAT;
    state->containerHeadThing =
        DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_NONE_PC34_COMPAT;
    state->nextSubmissionOrdinal = 1;
    fill_fixture_slots(state->chestSlots);
    memset(state->submittedSlots,
           DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_NONE_PC34_COMPAT,
           sizeof(state->submittedSlots));
    memset(state->drainedSlots,
           DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_NONE_PC34_COMPAT,
           sizeof(state->drainedSlots));
    memset(state->drainedOrdinals,
           DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_NONE_PC34_COMPAT,
           sizeof(state->drainedOrdinals));
}

int DM1_V1_MirrorCandidatePendingHandQueue_SubmitHandSwapPc34Compat(
    Dm1V1MirrorCandidatePendingHandQueueStatePc34Compat *state,
    int slotIndex,
    Dm1V1MirrorCandidatePendingHandQueueResultPc34Compat *outResult)
{
    Dm1V1MirrorCandidatePendingHandQueueResultPc34Compat localResult;
    Dm1V1MirrorCandidatePendingHandQueueResultPc34Compat *result =
        outResult ? outResult : &localResult;
    Dm1V1MirrorCandidatePendingHandQueueEntryPc34Compat *entry;
    int tail;

    capture_before(state, slotIndex, result);
    if (!state || !valid_slot_index(slotIndex)) {
        result->ignored = 1;
        return 0;
    }
    if (state->candidateChampionOrdinal == 0u) {
        ++state->candidateAliveGuardBlockCount;
        result->blockedByCandidateAliveGuard = 1;
        capture_after(state, result);
        return 0;
    }
    if (state->pendingCount >=
        DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_CAPACITY_PC34_COMPAT) {
        result->queueFull = 1;
        capture_after(state, result);
        return 0;
    }

    tail = queue_tail(state);
    entry = &state->pending[tail];
    entry->submissionOrdinal = state->nextSubmissionOrdinal++;
    entry->slotIndex = slotIndex;
    entry->command = kFirstChestSlotCommand + slotIndex;
    entry->handThingAtSubmission = state->leaderHandThing;
    state->submittedSlots[entry->submissionOrdinal - 1] = slotIndex;
    ++state->pendingCount;
    ++state->queueSubmitCount;
    result->accepted = 1;
    result->submissionOrdinal = entry->submissionOrdinal;
    result->command = entry->command;
    capture_after(state, result);
    return 1;
}

int DM1_V1_MirrorCandidatePendingHandQueue_OpenAlreadyOpenChestPc34Compat(
    Dm1V1MirrorCandidatePendingHandQueueStatePc34Compat *state,
    Dm1V1MirrorCandidatePendingHandQueueResultPc34Compat *outResult)
{
    Dm1V1MirrorCandidatePendingHandQueueResultPc34Compat localResult;
    Dm1V1MirrorCandidatePendingHandQueueResultPc34Compat *result =
        outResult ? outResult : &localResult;

    capture_before(
        state,
        DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_NONE_PC34_COMPAT,
        result);
    if (!state) {
        return 0;
    }
    if (state->openChestThing ==
        DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_OPEN_CHEST_PC34_COMPAT) {
        ++state->alreadyOpenGuardCount;
        result->ignored = 1;
        capture_after(state, result);
        return 0;
    }
    result->accepted = 1;
    capture_after(state, result);
    return 1;
}

int DM1_V1_MirrorCandidatePendingHandQueue_CloseChestPc34Compat(
    Dm1V1MirrorCandidatePendingHandQueueStatePc34Compat *state,
    Dm1V1MirrorCandidatePendingHandQueueResultPc34Compat *outResult)
{
    Dm1V1MirrorCandidatePendingHandQueueResultPc34Compat localResult;
    Dm1V1MirrorCandidatePendingHandQueueResultPc34Compat *result =
        outResult ? outResult : &localResult;
    int i;
    int processFirst = 1;

    capture_before(
        state,
        DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_NONE_PC34_COMPAT,
        result);
    if (!state) {
        return 0;
    }
    if (state->openChestThing ==
        DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_NONE_PC34_COMPAT) {
        result->ignored = 1;
        capture_after(state, result);
        return 0;
    }

    ++state->f0334CloseCount;
    if (state->pendingCount > 0) {
        ++state->closeDuringPendingQueueCount;
    }
    state->openChestThing =
        DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_NONE_PC34_COMPAT;
    state->containerHeadThing =
        DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_NONE_PC34_COMPAT;

    for (i = 0;
         i < DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        const int thing = state->chestSlots[i];
        if (thing != DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_NONE_PC34_COMPAT) {
            if (processFirst) {
                processFirst = 0;
                state->containerHeadThing = thing;
                ++state->chestFirstSlotWriteCount;
            } else {
                ++state->chestRelinkCount;
            }
            state->chestSlots[i] =
                DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_NONE_PC34_COMPAT;
            ++state->chestSlotClearCount;
        }
    }

    result->accepted = 1;
    capture_after(state, result);
    return 1;
}

int DM1_V1_MirrorCandidatePendingHandQueue_DrainNextPc34Compat(
    Dm1V1MirrorCandidatePendingHandQueueStatePc34Compat *state,
    Dm1V1MirrorCandidatePendingHandQueueResultPc34Compat *outResult)
{
    Dm1V1MirrorCandidatePendingHandQueueResultPc34Compat localResult;
    Dm1V1MirrorCandidatePendingHandQueueResultPc34Compat *result =
        outResult ? outResult : &localResult;
    Dm1V1MirrorCandidatePendingHandQueueEntryPc34Compat entry;
    int expectedSlot;
    int drainIndex;

    capture_before(
        state,
        DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_NONE_PC34_COMPAT,
        result);
    if (!state || state->pendingCount == 0) {
        result->ignored = 1;
        return 0;
    }
    if (state->candidateChampionOrdinal == 0u) {
        ++state->candidateAliveGuardBlockCount;
        result->blockedByCandidateAliveGuard = 1;
        capture_after(state, result);
        return 0;
    }

    ++state->candidateAliveGuardPassCount;
    entry = state->pending[state->pendingHead];
    state->pendingHead =
        (state->pendingHead + 1) %
        DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_CAPACITY_PC34_COMPAT;
    --state->pendingCount;

    drainIndex = state->handQueueDispatchCount;
    expectedSlot = state->submittedSlots[drainIndex];
    if (entry.submissionOrdinal != drainIndex + 1) {
        ++state->orderViolationCount;
    }
    if (expectedSlot != entry.slotIndex || !valid_slot_index(entry.slotIndex)) {
        ++state->slotIndexLostCount;
    }
    state->drainedOrdinals[drainIndex] = entry.submissionOrdinal;
    state->drainedSlots[drainIndex] = entry.slotIndex;
    ++state->handQueueDispatchCount;

    result->accepted = 1;
    result->drainedSlotIndex = entry.slotIndex;
    result->drainedSubmissionOrdinal = entry.submissionOrdinal;
    result->submissionOrdinal = entry.submissionOrdinal;
    result->slotIndex = entry.slotIndex;
    result->command = entry.command;
    capture_after(state, result);
    return 1;
}

const Dm1V1MirrorCandidatePendingHandQueueEvidencePc34Compat *
DM1_V1_MirrorCandidatePendingHandQueue_EvidencePc34Compat(void)
{
    return &s_evidence;
}

static void self_check(int condition, int *passed, int *failed)
{
    if (condition) {
        ++*passed;
    } else {
        ++*failed;
    }
}

int dm1_v1_mirror_candidate_pending_hand_queue_run(
    int *passed,
    int *failed)
{
    Dm1V1MirrorCandidatePendingHandQueueStatePc34Compat state;
    Dm1V1MirrorCandidatePendingHandQueueResultPc34Compat result;
    int i;

    if (!passed || !failed) {
        return 0;
    }
    *passed = 0;
    *failed = 0;

    DM1_V1_MirrorCandidatePendingHandQueue_InitPc34Compat(&state);
    for (i = 0; i < 5; ++i) {
        (void)DM1_V1_MirrorCandidatePendingHandQueue_SubmitHandSwapPc34Compat(
            &state, i, &result);
        self_check(result.accepted == 1, passed, failed);
        self_check(result.pendingSlotIndicesPreserved == 1, passed, failed);
    }
    (void)DM1_V1_MirrorCandidatePendingHandQueue_CloseChestPc34Compat(
        &state, &result);
    self_check(result.chestCloseRepackedSlots == 1, passed, failed);
    self_check(result.pendingSlotIndicesPreserved == 1, passed, failed);
    for (i = 0; i < 5; ++i) {
        (void)DM1_V1_MirrorCandidatePendingHandQueue_DrainNextPc34Compat(
            &state, &result);
        self_check(result.drainedSlotIndex == i, passed, failed);
        self_check(result.queueOrderPreserved == 1, passed, failed);
        self_check(result.candidateAlivePreserved == 1, passed, failed);
    }
    self_check(state.pendingCount == 0, passed, failed);
    self_check(state.slotIndexLostCount == 0, passed, failed);
    self_check(state.orderViolationCount == 0, passed, failed);
    return *failed == 0;
}
