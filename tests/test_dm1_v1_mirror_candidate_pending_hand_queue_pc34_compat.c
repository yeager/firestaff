/* ReDMCSB source-lock evidence:
 * CHEST.C F0333:30-32 guards an already-open chest.
 * CHEST.C F0334:113-132 closes and relinks chest slots.
 * COMMAND.C F0359:1985-1990 dispatches the mirror-candidate C040 route.
 * REVIVE.C F0280:124-132 keeps candidate handling on a live candidate.
 */
#include "dm1_v1_mirror_candidate_pending_hand_queue_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int gTests;
static int gPasses;

#define CHECK_REDMCSB(cond, msg, anchor) do { \
    ++gTests; \
    if (cond) { \
        ++gPasses; \
    } else { \
        printf("FAIL: %s [%s]\n", msg, anchor); \
    } \
} while (0)

static void test_source_lock_metadata(void)
{
    const Dm1V1MirrorCandidatePendingHandQueueEvidencePc34Compat *e =
        DM1_V1_MirrorCandidatePendingHandQueue_EvidencePc34Compat();

    CHECK_REDMCSB(e != NULL,
                  "evidence metadata is available",
                  "CHEST.C F0333:30-32");
    CHECK_REDMCSB(strstr(e->chestAlreadyOpenGuardAnchor,
                         "F0333:30-32") != NULL,
                  "evidence cites already-open chest guard",
                  e->chestAlreadyOpenGuardAnchor);
    CHECK_REDMCSB(strstr(e->chestCloseHandSwapAnchor,
                         "F0334:113-132") != NULL,
                  "evidence cites chest close hand-swap side effects",
                  e->chestCloseHandSwapAnchor);
    CHECK_REDMCSB(strstr(e->commandMirrorQueueDispatchAnchor,
                         "F0359:1985-1990") != NULL,
                  "evidence cites mirror candidate queue dispatch",
                  e->commandMirrorQueueDispatchAnchor);
    CHECK_REDMCSB(strstr(e->reviveCandidateAliveGuardAnchor,
                         "F0280:124-132") != NULL,
                  "evidence cites live-candidate guard",
                  e->reviveCandidateAliveGuardAnchor);
    CHECK_REDMCSB(strstr(e->contractScope, "contract-only") != NULL,
                  "evidence marks gate as contract-only",
                  e->contractScope);
    CHECK_REDMCSB(strstr(e->nonOverlapNote, "five pending hand-swap") != NULL &&
                      strstr(e->nonOverlapNote, "walkpath") != NULL,
                  "evidence records non-overlap with sibling gates",
                  e->nonOverlapNote);
}

static void test_fixture_starts_with_open_chest_and_alive_candidate(void)
{
    Dm1V1MirrorCandidatePendingHandQueueStatePc34Compat state;

    DM1_V1_MirrorCandidatePendingHandQueue_InitPc34Compat(&state);

    CHECK_REDMCSB(state.panelContent ==
                      DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_C040_PANEL_PC34_COMPAT,
                  "fixture starts in C040 panel",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(state.c040PanelOpen == 1,
                  "fixture has C040 open",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(state.candidateChampionOrdinal == 2u,
                  "fixture has live candidate ordinal",
                  "REVIVE.C F0280:124-132");
    CHECK_REDMCSB(state.partyChampionCount == 2u,
                  "fixture includes appended candidate in party count",
                  "REVIVE.C F0280:124-132");
    CHECK_REDMCSB(state.leaderHandThing ==
                      DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_LEADER_HAND_PC34_COMPAT,
                  "fixture starts with leader hand content",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(state.openChestThing ==
                      DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_OPEN_CHEST_PC34_COMPAT,
                  "fixture has G0426 open chest",
                  "CHEST.C F0334:113-116");
    CHECK_REDMCSB(state.containerHeadThing ==
                      DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_NONE_PC34_COMPAT,
                  "fixture starts with no relinked container head",
                  "CHEST.C F0334:117-132");
    CHECK_REDMCSB(state.pendingCount == 0 && state.pendingHead == 0,
                  "fixture starts with empty hand queue",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(state.nextSubmissionOrdinal == 1,
                  "fixture submission ordinals start at one",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(state.chestSlots[0] ==
                      DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_SLOT0_THING_PC34_COMPAT,
                  "fixture chest slot 0 is populated",
                  "CHEST.C F0334:117-132");
    CHECK_REDMCSB(state.chestSlots[1] ==
                      DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_SLOT1_THING_PC34_COMPAT,
                  "fixture chest slot 1 is populated",
                  "CHEST.C F0334:117-132");
    CHECK_REDMCSB(state.chestSlots[2] ==
                      DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_SLOT2_THING_PC34_COMPAT,
                  "fixture chest slot 2 is populated",
                  "CHEST.C F0334:117-132");
    CHECK_REDMCSB(state.chestSlots[3] ==
                      DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_SLOT3_THING_PC34_COMPAT,
                  "fixture chest slot 3 is populated",
                  "CHEST.C F0334:117-132");
    CHECK_REDMCSB(state.chestSlots[4] ==
                      DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_SLOT4_THING_PC34_COMPAT,
                  "fixture chest slot 4 is populated",
                  "CHEST.C F0334:117-132");
    CHECK_REDMCSB(state.chestSlots[5] ==
                      DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_NONE_PC34_COMPAT,
                  "fixture leaves later chest slots empty",
                  "CHEST.C F0334:117-132");
}

static void submit_five_entries(
    Dm1V1MirrorCandidatePendingHandQueueStatePc34Compat *state)
{
    Dm1V1MirrorCandidatePendingHandQueueResultPc34Compat result;
    int i;

    for (i = 0; i < 5; ++i) {
        const int accepted =
            DM1_V1_MirrorCandidatePendingHandQueue_SubmitHandSwapPc34Compat(
                state, i, &result);
        CHECK_REDMCSB(accepted == 1 && result.accepted == 1,
                      "hand swap submission is accepted",
                      "COMMAND.C F0359:1985-1990");
        CHECK_REDMCSB(result.submissionOrdinal == i + 1,
                      "submission ordinal increments in click order",
                      "COMMAND.C F0359:1985-1990");
        CHECK_REDMCSB(result.slotIndex == i,
                      "submission records original slot index",
                      "COMMAND.C F0359:1985-1990");
        CHECK_REDMCSB(result.command == 38 + i,
                      "submission records source slot command",
                      "COMMAND.C F0359:1985-1990");
        CHECK_REDMCSB(result.pendingCountBefore == i &&
                          result.pendingCountAfter == i + 1,
                      "pending queue length grows by one",
                      "COMMAND.C F0359:1985-1990");
        CHECK_REDMCSB(result.queueSubmitCountAfter ==
                          result.queueSubmitCountBefore + 1,
                      "queue submit count increments once",
                      "COMMAND.C F0359:1985-1990");
        CHECK_REDMCSB(result.pendingSlotIndicesPreserved == 1,
                      "submitted slot indices remain visible in queue",
                      "CHEST.C F0334:113-132");
        CHECK_REDMCSB(result.candidateAlivePreserved == 1,
                      "candidate stays alive during queue submission",
                      "REVIVE.C F0280:124-132");
    }
}

static void test_already_open_guard_preserves_pending_queue(void)
{
    Dm1V1MirrorCandidatePendingHandQueueStatePc34Compat state;
    Dm1V1MirrorCandidatePendingHandQueueResultPc34Compat result;
    int accepted;

    DM1_V1_MirrorCandidatePendingHandQueue_InitPc34Compat(&state);
    submit_five_entries(&state);

    accepted =
        DM1_V1_MirrorCandidatePendingHandQueue_OpenAlreadyOpenChestPc34Compat(
            &state, &result);

    CHECK_REDMCSB(accepted == 0 && result.ignored == 1,
                  "already-open chest refresh is ignored",
                  "CHEST.C F0333:30-32");
    CHECK_REDMCSB(result.alreadyOpenGuardCountAfter ==
                      result.alreadyOpenGuardCountBefore + 1,
                  "already-open guard count increments once",
                  "CHEST.C F0333:30-32");
    CHECK_REDMCSB(result.alreadyOpenPreservedQueue == 1,
                  "already-open guard preserves pending queue",
                  "CHEST.C F0333:30-32");
    CHECK_REDMCSB(result.pendingSlotIndicesPreserved == 1,
                  "already-open guard keeps queued slot indices",
                  "CHEST.C F0333:30-32");
    CHECK_REDMCSB(result.f0334CloseCountAfter ==
                      result.f0334CloseCountBefore,
                  "already-open guard does not close chest",
                  "CHEST.C F0333:30-32; CHEST.C F0334:113-132");
    CHECK_REDMCSB(result.openChestAfter == result.openChestBefore,
                  "already-open guard leaves G0426 untouched",
                  "CHEST.C F0333:30-32");
    CHECK_REDMCSB(result.candidateAlivePreserved == 1,
                  "candidate remains alive after already-open guard",
                  "REVIVE.C F0280:124-132");
    CHECK_REDMCSB(state.pendingCount == 5,
                  "queue still shows five pending hand swaps",
                  "COMMAND.C F0359:1985-1990");
}

static void test_chest_close_preserves_pending_slot_indices(void)
{
    Dm1V1MirrorCandidatePendingHandQueueStatePc34Compat state;
    Dm1V1MirrorCandidatePendingHandQueueResultPc34Compat result;
    int accepted;
    int i;

    DM1_V1_MirrorCandidatePendingHandQueue_InitPc34Compat(&state);
    submit_five_entries(&state);

    accepted = DM1_V1_MirrorCandidatePendingHandQueue_CloseChestPc34Compat(
        &state, &result);

    CHECK_REDMCSB(accepted == 1 && result.accepted == 1,
                  "F0334 chest close is accepted",
                  "CHEST.C F0334:113-132");
    CHECK_REDMCSB(result.f0334CloseCountAfter ==
                      result.f0334CloseCountBefore + 1,
                  "F0334 close count increments once",
                  "CHEST.C F0334:113-132");
    CHECK_REDMCSB(result.closeDuringPendingQueueCountAfter ==
                      result.closeDuringPendingQueueCountBefore + 1,
                  "close happens while queue has pending entries",
                  "CHEST.C F0334:113-132; COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(result.openChestAfter ==
                      DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_NONE_PC34_COMPAT,
                  "F0334 clears G0426 open chest",
                  "CHEST.C F0334:113-116");
    CHECK_REDMCSB(result.containerHeadAfter ==
                      DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_SLOT0_THING_PC34_COMPAT,
                  "F0334 writes first non-empty slot as container head",
                  "CHEST.C F0334:117-124");
    CHECK_REDMCSB(result.chestSlotClearCountAfter ==
                      result.chestSlotClearCountBefore + 5,
                  "F0334 clears five visible chest slots",
                  "CHEST.C F0334:117-132");
    CHECK_REDMCSB(result.chestFirstSlotWriteCountAfter ==
                      result.chestFirstSlotWriteCountBefore + 1,
                  "F0334 writes one first-slot link",
                  "CHEST.C F0334:122-126");
    CHECK_REDMCSB(result.chestRelinkCountAfter ==
                      result.chestRelinkCountBefore + 4,
                  "F0334 relinks four remaining slots",
                  "CHEST.C F0334:127-132");
    CHECK_REDMCSB(result.chestCloseRepackedSlots == 1,
                  "close result reports repacked non-empty chest slots",
                  "CHEST.C F0334:113-132");
    CHECK_REDMCSB(result.pendingCountAfter == 5,
                  "close leaves all five pending queue entries visible",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(result.pendingSlotIndicesPreserved == 1,
                  "close does not drop any pending slot index",
                  "CHEST.C F0334:113-132");
    CHECK_REDMCSB(result.slotIndexLostCountAfter ==
                      result.slotIndexLostCountBefore,
                  "slot-index-loss counter stays unchanged",
                  "CHEST.C F0334:113-132");
    CHECK_REDMCSB(result.orderViolationCountAfter ==
                      result.orderViolationCountBefore,
                  "close does not reorder the pending queue",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(result.candidateAlivePreserved == 1,
                  "candidate remains alive after chest close",
                  "REVIVE.C F0280:124-132");
    CHECK_REDMCSB(state.c040PanelOpen == 1 && state.panelContent ==
                      DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_C040_PANEL_PC34_COMPAT,
                  "C040 panel remains routed after close",
                  "COMMAND.C F0359:1985-1990");
    for (i = 0; i < 5; ++i) {
        CHECK_REDMCSB(state.pending[i].slotIndex == i,
                      "pending entry keeps its submitted slot index",
                      "CHEST.C F0334:113-132");
    }
    for (i = 0; i < 5; ++i) {
        CHECK_REDMCSB(state.submittedSlots[i] == i,
                      "submitted slot ledger keeps original order",
                      "COMMAND.C F0359:1985-1990");
    }
    for (i = 0; i < 5; ++i) {
        CHECK_REDMCSB(state.chestSlots[i] ==
                          DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_NONE_PC34_COMPAT,
                      "visible chest slot is cleared after close",
                      "CHEST.C F0334:117-132");
    }
}

static void test_queue_drains_after_close_in_submission_order(void)
{
    Dm1V1MirrorCandidatePendingHandQueueStatePc34Compat state;
    Dm1V1MirrorCandidatePendingHandQueueResultPc34Compat result;
    int i;

    DM1_V1_MirrorCandidatePendingHandQueue_InitPc34Compat(&state);
    submit_five_entries(&state);
    (void)DM1_V1_MirrorCandidatePendingHandQueue_CloseChestPc34Compat(
        &state, &result);

    for (i = 0; i < 5; ++i) {
        const int accepted =
            DM1_V1_MirrorCandidatePendingHandQueue_DrainNextPc34Compat(
                &state, &result);
        CHECK_REDMCSB(accepted == 1 && result.accepted == 1,
                      "pending hand swap dispatch is accepted",
                      "COMMAND.C F0359:1985-1990");
        CHECK_REDMCSB(result.drainedSubmissionOrdinal == i + 1,
                      "drain uses original submission ordinal",
                      "COMMAND.C F0359:1985-1990");
        CHECK_REDMCSB(result.drainedSlotIndex == i,
                      "drain uses original submitted slot index",
                      "COMMAND.C F0359:1985-1990; CHEST.C F0334:113-132");
        CHECK_REDMCSB(result.command == 38 + i,
                      "drain preserves original slot command",
                      "COMMAND.C F0359:1985-1990");
        CHECK_REDMCSB(result.pendingCountBefore == 5 - i &&
                          result.pendingCountAfter == 4 - i,
                      "pending count drains one entry at a time",
                      "COMMAND.C F0359:1985-1990");
        CHECK_REDMCSB(result.handQueueDispatchCountAfter ==
                          result.handQueueDispatchCountBefore + 1,
                      "dispatch count increments once per drain",
                      "COMMAND.C F0359:1985-1990");
        CHECK_REDMCSB(result.candidateAliveGuardPassCountAfter ==
                          result.candidateAliveGuardPassCountBefore + 1,
                      "alive guard passes before draining",
                      "REVIVE.C F0280:124-132");
        CHECK_REDMCSB(result.queueOrderPreserved == 1,
                      "queue order is preserved through this drain",
                      "COMMAND.C F0359:1985-1990");
        CHECK_REDMCSB(result.pendingSlotIndicesPreserved == 1,
                      "remaining queue retains submitted slot indices",
                      "CHEST.C F0334:113-132");
        CHECK_REDMCSB(result.slotIndexLostCountAfter ==
                          result.slotIndexLostCountBefore,
                      "drain does not record a lost slot index",
                      "CHEST.C F0334:113-132");
        CHECK_REDMCSB(result.orderViolationCountAfter ==
                          result.orderViolationCountBefore,
                      "drain does not record an order violation",
                      "COMMAND.C F0359:1985-1990");
        CHECK_REDMCSB(result.candidateAlivePreserved == 1,
                      "candidate remains alive after drain",
                      "REVIVE.C F0280:124-132");
    }

    CHECK_REDMCSB(state.pendingCount == 0,
                  "queue is empty after five drains",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(state.handQueueDispatchCount == 5,
                  "all five pending entries were dispatched",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(state.slotIndexLostCount == 0,
                  "no pending slot index was lost",
                  "CHEST.C F0334:113-132");
    CHECK_REDMCSB(state.orderViolationCount == 0,
                  "no pending entry was reordered",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(state.candidateChampionOrdinal == 2u,
                  "candidate ordinal remains live after queue drain",
                  "REVIVE.C F0280:124-132");
    for (i = 0; i < 5; ++i) {
        CHECK_REDMCSB(state.drainedOrdinals[i] == i + 1,
                      "drained ordinal ledger matches submission order",
                      "COMMAND.C F0359:1985-1990");
    }
    for (i = 0; i < 5; ++i) {
        CHECK_REDMCSB(state.drainedSlots[i] == i,
                      "drained slot ledger matches original slots",
                      "CHEST.C F0334:113-132");
    }
}

static void test_dead_candidate_guard_blocks_drain_without_mutating_queue(void)
{
    Dm1V1MirrorCandidatePendingHandQueueStatePc34Compat state;
    Dm1V1MirrorCandidatePendingHandQueueResultPc34Compat result;
    int accepted;

    DM1_V1_MirrorCandidatePendingHandQueue_InitPc34Compat(&state);
    submit_five_entries(&state);
    state.candidateChampionOrdinal = 0u;

    accepted = DM1_V1_MirrorCandidatePendingHandQueue_DrainNextPc34Compat(
        &state, &result);

    CHECK_REDMCSB(accepted == 0 && result.blockedByCandidateAliveGuard == 1,
                  "dead candidate guard blocks queue drain",
                  "REVIVE.C F0280:124-132");
    CHECK_REDMCSB(result.candidateAliveGuardBlockCountAfter ==
                      result.candidateAliveGuardBlockCountBefore + 1,
                  "dead candidate guard increments block count",
                  "REVIVE.C F0280:124-132");
    CHECK_REDMCSB(result.pendingCountAfter == result.pendingCountBefore,
                  "blocked drain leaves queue length unchanged",
                  "REVIVE.C F0280:124-132; COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(result.handQueueDispatchCountAfter ==
                      result.handQueueDispatchCountBefore,
                  "blocked drain dispatches no hand swap",
                  "REVIVE.C F0280:124-132");
    CHECK_REDMCSB(result.pendingSlotIndicesPreserved == 1,
                  "blocked drain preserves pending slot indices",
                  "REVIVE.C F0280:124-132");
    CHECK_REDMCSB(result.slotIndexLostCountAfter ==
                      result.slotIndexLostCountBefore,
                  "blocked drain records no lost slot index",
                  "REVIVE.C F0280:124-132");
    CHECK_REDMCSB(state.pending[0].slotIndex == 0 &&
                      state.pending[1].slotIndex == 1,
                  "blocked drain keeps the front queue entries in place",
                  "COMMAND.C F0359:1985-1990");
}

static void test_embedded_self_check(void)
{
    int passed = 0;
    int failed = 0;
    int ok = dm1_v1_mirror_candidate_pending_hand_queue_run(&passed, &failed);

    CHECK_REDMCSB(ok == 1,
                  "embedded self-check passes",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(failed == 0,
                  "embedded self-check reports no failures",
                  "CHEST.C F0334:113-132");
    CHECK_REDMCSB(passed >= 23,
                  "embedded self-check exercises queue and close assertions",
                  "COMMAND.C F0359:1985-1990");
}

int main(void)
{
    test_source_lock_metadata();
    test_fixture_starts_with_open_chest_and_alive_candidate();
    test_already_open_guard_preserves_pending_queue();
    test_chest_close_preserves_pending_slot_indices();
    test_queue_drains_after_close_in_submission_order();
    test_dead_candidate_guard_blocks_drain_without_mutating_queue();
    test_embedded_self_check();

    if (gTests != gPasses) {
        printf("%d/%d assertions passed\n", gPasses, gTests);
        return 1;
    }
    printf("%d assertions passed\n", gPasses);
    return 0;
}
