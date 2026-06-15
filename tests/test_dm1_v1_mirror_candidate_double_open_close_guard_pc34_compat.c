#include "src/dm1/dm1_v1_mirror_candidate_double_open_close_guard_pc34_compat.h"

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

static void test_source_anchors(void)
{
    const Dm1V1MirrorCandidateDoubleOpenCloseGuardEvidencePc34Compat *e =
        dm1_v1_mirror_candidate_double_open_close_guard_evidence_pc34_compat();

    CHECK_REDMCSB(e != 0,
                  "double-open/close guard evidence is available",
                  "metadata");
    CHECK_REDMCSB(e->contractOnly == 1 &&
                      strstr(e->nonDuplicateScope, "double-open") != 0 &&
                      strstr(e->nonDuplicateScope, "pending chest-close") != 0,
                  "fixture is explicitly runtime-only and non-duplicative",
                  e->nonDuplicateScope);
    CHECK_REDMCSB(strstr(e->chamdrawPanelOpenAnchor, "F0291") != 0 &&
                      strstr(e->chamdrawPanelOpenAnchor, "551-552") != 0 &&
                      strstr(e->chamdrawPanelOpenAnchor, "G0425") != 0,
                  "CHAMDRAW.C F0291 C30/G0425 panel-open anchor is cited",
                  e->chamdrawPanelOpenAnchor);
    CHECK_REDMCSB(strstr(e->chamdrawPanelCloseAnchor, "F0296") != 0 &&
                      strstr(e->chamdrawPanelCloseAnchor, "1249-1252") != 0,
                  "CHAMDRAW.C F0296 chest close redraw anchor is cited",
                  e->chamdrawPanelCloseAnchor);
    CHECK_REDMCSB(strstr(e->championLeaderHandPutAnchor, "F0297") != 0 &&
                      strstr(e->championLeaderHandPutAnchor, "243-298") != 0,
                  "CHAMPION.C F0297 leader-hand put anchor is cited",
                  e->championLeaderHandPutAnchor);
    CHECK_REDMCSB(strstr(e->championOccupiedSlotClickAnchor, "F0302") != 0 &&
                      strstr(e->championOccupiedSlotClickAnchor, "662-710") != 0,
                  "CHAMPION.C F0302 occupied-slot dispatch anchor is cited",
                  e->championOccupiedSlotClickAnchor);
    CHECK_REDMCSB(strstr(e->commandPanelDispatchAnchor, "F0359") != 0 &&
                      strstr(e->commandPanelDispatchAnchor, "1985-1990") != 0 &&
                      strstr(e->commandPanelDispatchAnchor, "M568/C040") != 0,
                  "COMMAND.C F0359 M568/C040 dispatch anchor is cited",
                  e->commandPanelDispatchAnchor);
    CHECK_REDMCSB(strstr(e->reviveCandidateClearAnchor, "F0282") != 0 &&
                      strstr(e->reviveCandidateClearAnchor, "744-806") != 0,
                  "REVIVE.C F0282 candidate clear/finalize anchor is cited",
                  e->reviveCandidateClearAnchor);
    CHECK_REDMCSB(strstr(e->defsAnchor, "C30") != 0 &&
                      strstr(e->defsAnchor, "G0425") != 0 &&
                      strstr(e->defsAnchor, "G0426") != 0 &&
                      strstr(e->defsAnchor, "M070") != 0 &&
                      strstr(e->defsAnchor, "M516") != 0 &&
                      strstr(e->defsAnchor, "C040") != 0,
                  "DEFS.H constants and globals are cited",
                  e->defsAnchor);
}

static void test_double_open_guard(void)
{
    Dm1V1MirrorCandidateDoubleOpenCloseGuardResultPc34Compat result;
    const Dm1V1MirrorCandidateDoubleOpenCloseGuardEvidencePc34Compat *e =
        dm1_v1_mirror_candidate_double_open_close_guard_evidence_pc34_compat();
    int ok =
        dm1_v1_mirror_candidate_double_open_close_guard_run_double_open_pc34_compat(
            &result);

    CHECK_REDMCSB(ok == 1,
                  "double-open sequence runs through the C040 guard",
                  e->commandPanelDispatchAnchor);
    CHECK_REDMCSB(result.eventsProcessed == 2,
                  "both open events are observed",
                  e->commandPanelDispatchAnchor);
    CHECK_REDMCSB(result.c040PanelOpenBefore == 1 &&
                      result.c040PanelOpenAfter == 1,
                  "C040 panel remains open after a second open",
                  e->chamdrawPanelOpenAnchor);
    CHECK_REDMCSB(result.panelContentBefore ==
                          DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_M568_C040_PC34 &&
                      result.panelContentAfter == result.panelContentBefore,
                  "second open does not rewrite panel content",
                  e->commandPanelDispatchAnchor);
    CHECK_REDMCSB(result.duplicateOpenNoopCount == 2 &&
                      result.doubleOpenWasNoop == 1,
                  "a second C040 open while already open is a no-op",
                  e->commandPanelDispatchAnchor);
    CHECK_REDMCSB(result.openDispatchCount == 0,
                  "already-open C040 does not append a candidate again",
                  e->reviveCandidateClearAnchor);
    CHECK_REDMCSB(result.candidateBefore == result.candidateAfter &&
                      result.inventoryBefore == result.inventoryAfter,
                  "candidate and inventory ordinals survive double open",
                  e->reviveCandidateClearAnchor);
    CHECK_REDMCSB(result.partyCountBefore == result.partyCountAfter,
                  "party count is unchanged by double open",
                  e->reviveCandidateClearAnchor);
    CHECK_REDMCSB(result.leaderHandBefore == result.leaderHandAfter &&
                      result.doubleOpenPreservedLeaderHand == 1,
                  "double open does not clear the leader hand",
                  e->championLeaderHandPutAnchor);
    CHECK_REDMCSB(result.f0297LeaderHandPutCount == 0 &&
                      result.f0302SlotDispatchCount == 0,
                  "double open does not run F0297/F0302 slot mutation",
                  e->championOccupiedSlotClickAnchor);
    CHECK_REDMCSB(result.f0282CandidateClearCount == 0 &&
                      result.doubleOpenDidNotClearCandidate == 1,
                  "double open does not re-trigger F0282 candidate clear",
                  e->reviveCandidateClearAnchor);
}

static void test_double_close_guard(void)
{
    Dm1V1MirrorCandidateDoubleOpenCloseGuardResultPc34Compat result;
    const Dm1V1MirrorCandidateDoubleOpenCloseGuardEvidencePc34Compat *e =
        dm1_v1_mirror_candidate_double_open_close_guard_evidence_pc34_compat();
    int ok =
        dm1_v1_mirror_candidate_double_open_close_guard_run_double_close_pc34_compat(
            &result);

    CHECK_REDMCSB(ok == 1,
                  "double-close sequence runs through the C040 guard",
                  e->commandPanelDispatchAnchor);
    CHECK_REDMCSB(result.eventsProcessed == 1,
                  "closed-panel close event is observed",
                  e->commandPanelDispatchAnchor);
    CHECK_REDMCSB(result.c040PanelOpenBefore == 0 &&
                      result.c040PanelOpenAfter == 0,
                  "already-closed C040 remains closed",
                  e->reviveCandidateClearAnchor);
    CHECK_REDMCSB(result.panelContentBefore ==
                          DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_M569_CHEST_PC34 &&
                      result.panelContentAfter == result.panelContentBefore,
                  "second close does not zero the existing panel state",
                  e->chamdrawPanelCloseAnchor);
    CHECK_REDMCSB(result.duplicateCloseNoopCount == 1 &&
                      result.doubleCloseWasNoop == 1,
                  "a C040 close while already closed is a no-op",
                  e->reviveCandidateClearAnchor);
    CHECK_REDMCSB(result.closeDispatchCount == 0 &&
                      result.f0355InventoryCloseCount == 0,
                  "closed-panel close does not call the inventory close route",
                  e->reviveCandidateClearAnchor);
    CHECK_REDMCSB(result.f0282CandidateClearCount == 0 &&
                      result.doubleCloseDidNotClearCandidateAgain == 1,
                  "closed-panel close does not re-emit F0282 candidate clear",
                  e->reviveCandidateClearAnchor);
    CHECK_REDMCSB(result.panelZeroCount == 0 &&
                      result.doubleClosePreservedClosedPanelState == 1,
                  "closed-panel close does not zero the panel state",
                  e->chamdrawPanelCloseAnchor);
    CHECK_REDMCSB(result.candidateBefore == 0u &&
                      result.candidateAfter == 0u &&
                      result.inventoryAfter == 0u,
                  "closed-panel close leaves candidate ownership absent",
                  e->reviveCandidateClearAnchor);
    CHECK_REDMCSB(result.leaderHandBefore == result.leaderHandAfter,
                  "closed-panel close leaves leader hand untouched",
                  e->championLeaderHandPutAnchor);
}

static void test_close_after_real_close_no_second_clear(void)
{
    Dm1V1MirrorCandidateDoubleOpenCloseGuardStatePc34Compat state;
    Dm1V1MirrorCandidateDoubleOpenCloseGuardEventPc34Compat events[2];
    Dm1V1MirrorCandidateDoubleOpenCloseGuardResultPc34Compat result;
    const Dm1V1MirrorCandidateDoubleOpenCloseGuardEvidencePc34Compat *e =
        dm1_v1_mirror_candidate_double_open_close_guard_evidence_pc34_compat();

    dm1_v1_mirror_candidate_double_open_close_guard_init_open_pc34_compat(
        &state);
    memset(events, 0, sizeof(events));
    events[0].kind =
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_EVENT_C040_CLOSE_PC34;
    events[0].tick = 50;
    events[1].kind =
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_EVENT_C040_CLOSE_PC34;
    events[1].tick = 51;

    CHECK_REDMCSB(
        dm1_v1_mirror_candidate_double_open_close_guard_run_events_pc34_compat(
            &state, events, 2u, &result) == 1,
        "open close close sequence dispatches",
        e->commandPanelDispatchAnchor);
    CHECK_REDMCSB(result.closeDispatchCount == 1 &&
                      result.duplicateCloseNoopCount == 1,
                  "real close is followed by one suppressed double-close",
                  e->reviveCandidateClearAnchor);
    CHECK_REDMCSB(result.f0282CandidateClearCount == 1 &&
                      result.doubleCloseDidNotClearCandidateAgain == 1,
                  "second close does not re-emit F0282 candidate clear",
                  e->reviveCandidateClearAnchor);
    CHECK_REDMCSB(result.panelZeroCount == 1 &&
                      result.panelContentAfter == 0,
                  "only the real close zeroes panel state",
                  e->chamdrawPanelCloseAnchor);
    CHECK_REDMCSB(result.partyCountAfter == 2u &&
                      result.candidateAfter == 0u,
                  "real close restores party count once",
                  e->reviveCandidateClearAnchor);
}

static void test_close_during_pending_guard(void)
{
    Dm1V1MirrorCandidateDoubleOpenCloseGuardResultPc34Compat result;
    const Dm1V1MirrorCandidateDoubleOpenCloseGuardEvidencePc34Compat *e =
        dm1_v1_mirror_candidate_double_open_close_guard_evidence_pc34_compat();
    int ok =
        dm1_v1_mirror_candidate_double_open_close_guard_run_close_during_pending_pc34_compat(
            &result);

    CHECK_REDMCSB(ok == 1,
                  "close-during-pending sequence runs through the guard",
                  e->commandPanelDispatchAnchor);
    CHECK_REDMCSB(result.eventsProcessed == 2,
                  "chest close and C040 close land on the same tick",
                  e->commandPanelDispatchAnchor);
    CHECK_REDMCSB(result.openChestBefore !=
                          DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_NONE_PC34 &&
                      result.openChestAfter ==
                          DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_NONE_PC34,
                  "chest close is modeled before pending C040 open",
                  e->chamdrawPanelCloseAnchor);
    CHECK_REDMCSB(result.f0334ChestCloseCount == 1,
                  "chest close route runs once",
                  e->chamdrawPanelCloseAnchor);
    CHECK_REDMCSB(result.duplicateCloseNoopCount == 1 &&
                      result.closeDispatchCount == 0,
                  "close on a pending candidate is treated as closed-panel no-op",
                  e->commandPanelDispatchAnchor);
    CHECK_REDMCSB(result.leaderHandQueueBefore == result.leaderHandQueueAfter &&
                      result.closeDuringPendingPreservedLeaderHandQueue == 1,
                  "close-during-pending does not clear the leader-hand queue",
                  e->championLeaderHandPutAnchor);
    CHECK_REDMCSB(result.leaderHandQueueClearCount == 0,
                  "leader-hand queued object is not erased",
                  e->championLeaderHandPutAnchor);
    CHECK_REDMCSB(result.f0282CandidateClearCount == 0 &&
                      result.closeDuringPendingDidNotClearCandidate == 1,
                  "close-during-pending does not run F0282 candidate clear",
                  e->reviveCandidateClearAnchor);
    CHECK_REDMCSB(result.pendingOpenFlushCount == 1 &&
                      result.closeDuringPendingOpenedCandidate == 1,
                  "pending chest close still opens the mirror candidate panel",
                  e->commandPanelDispatchAnchor);
    CHECK_REDMCSB(result.c040PanelOpenAfter == 1 &&
                      result.panelContentAfter ==
                          DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_M568_C040_PC34,
                  "C040 panel is open after pending flush",
                  e->chamdrawPanelOpenAnchor);
    CHECK_REDMCSB(result.candidateAfter == 4u &&
                      result.partyCountAfter == 3u,
                  "pending candidate identity and party count are installed",
                  e->reviveCandidateClearAnchor);
}

static void test_inventory_click_during_close_guard(void)
{
    Dm1V1MirrorCandidateDoubleOpenCloseGuardResultPc34Compat result;
    const Dm1V1MirrorCandidateDoubleOpenCloseGuardEvidencePc34Compat *e =
        dm1_v1_mirror_candidate_double_open_close_guard_evidence_pc34_compat();
    int ok =
        dm1_v1_mirror_candidate_double_open_close_guard_run_inventory_click_during_close_pc34_compat(
            &result);

    CHECK_REDMCSB(ok == 1,
                  "inventory-click during close sequence runs through guard",
                  e->commandPanelDispatchAnchor);
    CHECK_REDMCSB(result.eventsProcessed == 2,
                  "close and inventory portrait click share one tick",
                  e->commandPanelDispatchAnchor);
    CHECK_REDMCSB(result.closeDispatchCount == 1 &&
                      result.f0282CandidateClearCount == 1,
                  "close dispatch runs once before same-tick inventory click",
                  e->reviveCandidateClearAnchor);
    CHECK_REDMCSB(result.f0355InventoryCloseCount == 1 &&
                      result.f0334ChestCloseCount == 1,
                  "inventory/chest close routes provide the slot order",
                  e->chamdrawPanelCloseAnchor);
    CHECK_REDMCSB(result.inventoryPortraitClickCount == 1,
                  "same-tick inventory portrait click is observed",
                  e->championOccupiedSlotClickAnchor);
    CHECK_REDMCSB(result.sameTickCloseSlotOrderCount == 1 &&
                      result.inventoryClickUsedCloseSlotOrder == 1,
                  "inventory portrait click uses the close's slot order",
                  e->chamdrawPanelCloseAnchor);
    CHECK_REDMCSB(result.usedSlotOrder[0] == 0 &&
                      result.usedSlotOrder[1] == 2 &&
                      result.usedSlotOrder[2] == 3 &&
                      result.usedSlotOrder[3] ==
                          DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_NONE_PC34,
                  "used slot order matches close order 0,2,3",
                  e->defsAnchor);
    CHECK_REDMCSB(result.clickSlotOrderCount == 0 &&
                      result.inventoryClickDidNotUseClickSlotOrder == 1,
                  "same-tick inventory click does not use its own click order",
                  e->championOccupiedSlotClickAnchor);
    CHECK_REDMCSB(result.f0302SlotDispatchCount == 0 &&
                      result.inventoryClickDidNotDispatchF0302 == 1,
                  "same-tick inventory click does not dispatch F0302",
                  e->championOccupiedSlotClickAnchor);
    CHECK_REDMCSB(result.f0297LeaderHandPutCount == 0,
                  "same-tick inventory click does not run F0297 leader-hand put",
                  e->championLeaderHandPutAnchor);
    CHECK_REDMCSB(result.c040PanelOpenAfter == 0 &&
                      result.panelContentAfter == 0,
                  "close leaves the C040 panel closed",
                  e->reviveCandidateClearAnchor);
    CHECK_REDMCSB(result.candidateAfter == 0u &&
                      result.inventoryAfter == 0u &&
                      result.partyCountAfter == 2u,
                  "close clears candidate ownership once",
                  e->reviveCandidateClearAnchor);
}

static void test_inventory_click_without_close_negative_control(void)
{
    Dm1V1MirrorCandidateDoubleOpenCloseGuardStatePc34Compat state;
    Dm1V1MirrorCandidateDoubleOpenCloseGuardEventPc34Compat event;
    Dm1V1MirrorCandidateDoubleOpenCloseGuardResultPc34Compat result;
    const Dm1V1MirrorCandidateDoubleOpenCloseGuardEvidencePc34Compat *e =
        dm1_v1_mirror_candidate_double_open_close_guard_evidence_pc34_compat();

    dm1_v1_mirror_candidate_double_open_close_guard_init_open_pc34_compat(
        &state);
    memset(&event, 0, sizeof(event));
    event.kind =
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_EVENT_INVENTORY_PORTRAIT_CLICK_PC34;
    event.tick = 60;

    CHECK_REDMCSB(
        dm1_v1_mirror_candidate_double_open_close_guard_run_events_pc34_compat(
            &state, &event, 1u, &result) == 1,
        "inventory click without close dispatches as a negative control",
        e->championOccupiedSlotClickAnchor);
    CHECK_REDMCSB(result.inventoryPortraitClickCount == 1 &&
                      result.sameTickCloseSlotOrderCount == 0,
                  "negative-control click has no close slot order",
                  e->championOccupiedSlotClickAnchor);
    CHECK_REDMCSB(result.clickSlotOrderCount == 1 &&
                      result.usedSlotOrder[0] == 1,
                  "negative-control click uses the click slot order",
                  e->championOccupiedSlotClickAnchor);
    CHECK_REDMCSB(result.f0302SlotDispatchCount == 1 &&
                      result.f0297LeaderHandPutCount == 1,
                  "negative-control click reaches F0302/F0297",
                  e->championLeaderHandPutAnchor);
    CHECK_REDMCSB(result.inventoryClickUsedCloseSlotOrder == 0,
                  "negative-control click is not mislabeled as close order",
                  e->chamdrawPanelCloseAnchor);
}

int main(void)
{
    test_source_anchors();
    test_double_open_guard();
    test_double_close_guard();
    test_close_after_real_close_no_second_clear();
    test_close_during_pending_guard();
    test_inventory_click_during_close_guard();
    test_inventory_click_without_close_negative_control();

    printf("PASS dm1_v1_mirror_candidate_double_open_close_guard_pc34_compat "
           "%d/%d assertions\n",
           gPasses, gTests);
    return gPasses == gTests ? 0 : 1;
}
