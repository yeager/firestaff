#include "dm1_v1_mirror_candidate_no_pending_resurrect_pc34_compat.h"

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

static Dm1V1MirrorCandidateNoPendingResurrectStatePc34Compat
base_no_pending_state(void)
{
    Dm1V1MirrorCandidateNoPendingResurrectStatePc34Compat state;

    DM1_V1_MirrorCandidateNoPendingResurrect_InitPc34Compat(&state);
    return state;
}

static void test_fixture_models_open_route_without_pending_party_candidate(void)
{
    Dm1V1MirrorCandidateNoPendingResurrectStatePc34Compat state =
        base_no_pending_state();

    CHECK_REDMCSB(state.g0299CandidateChampionOrdinal == 1u,
                  "fixture keeps G0299 set so the mirror panel route is open",
                  "COMMAND.C:2159-2181; COMMAND.C:2302-2311");
    CHECK_REDMCSB(state.g0305PartyChampionCount == 0u,
                  "fixture has no F0280-appended party candidate",
                  "REVIVE.C F0280:272-276");
    CHECK_REDMCSB(state.c040PanelOpen == 1 &&
                      state.c040PanelPixelsDrawn == 1 &&
                      state.panelContent ==
                          DM1_V1_MIRROR_CANDIDATE_NO_PENDING_RESURRECT_PANEL_CONTENT_PC34_COMPAT,
                  "C040 panel route starts visibly open",
                  "PANEL.C F0346:1619-1635; PANEL.C F0347");
    CHECK_REDMCSB(state.leaderHandEmpty == 1,
                  "empty leader hand allows C040 panel command scanning",
                  "COMMAND.C F0359:1985-1989");
    CHECK_REDMCSB(state.mirrorRouteOpen == 1 &&
                      state.frontD1cMirrorChampionOrdinal == 1,
                  "front mirror route is open before the no-op click",
                  "DUNVIEW.C:3913-3928");
}

static void test_c160_resurrect_no_pending_is_noop(void)
{
    Dm1V1MirrorCandidateNoPendingResurrectStatePc34Compat state =
        base_no_pending_state();
    Dm1V1MirrorCandidateNoPendingResurrectResultPc34Compat result;
    int consumed;

    consumed =
        DM1_V1_MirrorCandidateNoPendingResurrect_ProcessPanelCommandPc34Compat(
            &state,
            DM1_V1_MIRROR_CANDIDATE_NO_PENDING_RESURRECT_C160_PC34_COMPAT,
            &result);

    CHECK_REDMCSB(consumed == 0 && result.consumed == 0,
                  "C160 with G0299=1 and G0305=0 does not consume the click",
                  "REVIVE.C F0282:785-806");
    CHECK_REDMCSB(result.validPanelCommand == 1 &&
                      result.panelRouteOpen == 1 &&
                      result.ignoredNoPendingCandidate == 1,
                  "C160 reaches the open panel guard but finds no pending candidate",
                  "COMMAND.C F0359:1985-1989; REVIVE.C F0282:744");
    CHECK_REDMCSB(result.g0299Before == 1u &&
                      result.g0299After == 1u &&
                      result.g0299Preserved == 1,
                  "no-pending C160 does not close G0299",
                  "REVIVE.C F0282:785-806");
    CHECK_REDMCSB(result.g0305Before == 0u &&
                      result.g0305After == 0u &&
                      result.g0305Preserved == 1,
                  "no-pending C160 leaves G0305 unchanged",
                  "REVIVE.C F0280:272-276; REVIVE.C F0282:744");
    CHECK_REDMCSB(result.noF0282Called == 1 &&
                      result.resurrectCallPreserved == 1 &&
                      result.f0282ResurrectCallCountAfter == 0,
                  "no-pending C160 does not call F0282 resurrect",
                  "REVIVE.C F0282:785-806");
    CHECK_REDMCSB(result.noChampionRearmed == 1 &&
                      result.championRearmCountAfter == 0,
                  "no-pending C160 does not re-arm any champion",
                  "DUNVIEW.C:3913-3928; DUNVIEW.C:8488-8533");
    CHECK_REDMCSB(result.c040PanelPreserved == 1 &&
                      state.c040PanelOpen == 1 &&
                      state.c040PanelPixelsDrawn == 1,
                  "no-pending C160 leaves C040 panel state unchanged",
                  "PANEL.C F0346:1619-1635; PANEL.C F0347");
    CHECK_REDMCSB(result.inventoryPreserved == 1 &&
                      state.inventoryChampionOrdinal == 1u &&
                      state.inventoryPanelOpen == 1,
                  "no-pending C160 leaves inventory state unchanged",
                  "COMMAND.C:2159-2181");
    CHECK_REDMCSB(result.mirrorRoutePreserved == 1 &&
                      state.mirrorRouteOpen == 1,
                  "no-pending C160 keeps the mirror route open",
                  "DUNVIEW.C:3913-3928");
}

static void test_c162_cancel_no_pending_does_not_close_g0299(void)
{
    Dm1V1MirrorCandidateNoPendingResurrectStatePc34Compat state =
        base_no_pending_state();
    Dm1V1MirrorCandidateNoPendingResurrectResultPc34Compat result;
    int consumed;

    consumed =
        DM1_V1_MirrorCandidateNoPendingResurrect_ProcessPanelCommandPc34Compat(
            &state,
            DM1_V1_MIRROR_CANDIDATE_NO_PENDING_RESURRECT_C162_PC34_COMPAT,
            &result);

    CHECK_REDMCSB(consumed == 0 && result.ignoredNoPendingCandidate == 1,
                  "C162 cancel is also a no-op without the G0305 append",
                  "REVIVE.C F0282:744-758");
    CHECK_REDMCSB(result.g0299After == 1u &&
                      result.g0305After == 0u,
                  "no-pending C162 neither closes G0299 nor changes G0305",
                  "REVIVE.C F0282:744-758");
    CHECK_REDMCSB(result.noF0282Called == 1 &&
                      result.cancelCallPreserved == 1 &&
                      result.f0282CancelCallCountAfter == 0,
                  "no-pending C162 does not call the F0282 cancel cleanup",
                  "REVIVE.C F0282:744-758");
    CHECK_REDMCSB(result.c040PanelPreserved == 1 &&
                      result.inventoryPreserved == 1,
                  "no-pending C162 leaves C040 and inventory state unchanged",
                  "REVIVE.C F0282:744-758; COMMAND.C:2159-2181");
}

static void test_g0299_gates_remain_closed_after_noop(void)
{
    Dm1V1MirrorCandidateNoPendingResurrectStatePc34Compat state =
        base_no_pending_state();
    Dm1V1MirrorCandidateNoPendingResurrectResultPc34Compat commandResult;
    Dm1V1MirrorCandidateNoPendingResurrectGateResultPc34Compat statusGate;
    Dm1V1MirrorCandidateNoPendingResurrectGateResultPc34Compat inventoryGate;
    Dm1V1MirrorCandidateNoPendingResurrectGateResultPc34Compat actionGate;

    (void)DM1_V1_MirrorCandidateNoPendingResurrect_ProcessPanelCommandPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_NO_PENDING_RESURRECT_C160_PC34_COMPAT,
        &commandResult);
    (void)DM1_V1_MirrorCandidateNoPendingResurrect_CanDispatchCommandPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_NO_PENDING_RESURRECT_STATUS_BOX_0_PC34_COMPAT,
        &statusGate);
    (void)DM1_V1_MirrorCandidateNoPendingResurrect_CanDispatchCommandPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_NO_PENDING_RESURRECT_CLOSE_INVENTORY_PC34_COMPAT,
        &inventoryGate);
    (void)DM1_V1_MirrorCandidateNoPendingResurrect_CanDispatchCommandPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_NO_PENDING_RESURRECT_ACTION_AREA_PC34_COMPAT,
        &actionGate);

    CHECK_REDMCSB(commandResult.g0299After == 1u,
                  "no-op keeps G0299 set for the sibling command gates",
                  "REVIVE.C F0282:744-806");
    CHECK_REDMCSB(statusGate.blockedByG0299 == 1 &&
                      statusGate.statusBoxAllowed == 0,
                  "status-box dispatch remains blocked while G0299 is set",
                  "COMMAND.C:2159-2181");
    CHECK_REDMCSB(inventoryGate.blockedByG0299 == 1 &&
                      inventoryGate.inventoryAllowed == 0,
                  "inventory dispatch remains blocked while G0299 is set",
                  "COMMAND.C:2159-2181");
    CHECK_REDMCSB(actionGate.blockedByG0299 == 1 &&
                      actionGate.actionAreaAllowed == 0,
                  "action-area dispatch remains blocked while G0299 is set",
                  "COMMAND.C:2302-2311");
}

static void test_source_lock_evidence(void)
{
    const Dm1V1MirrorCandidateNoPendingResurrectEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateNoPendingResurrect_EvidencePc34Compat();

    CHECK_REDMCSB(e != NULL,
                  "evidence accessor returns source-lock metadata",
                  "REVIVE.C F0282:744-806");
    CHECK_REDMCSB(strstr(e->candidatePublishAnchor, "F0280:272-276") != NULL,
                  "evidence cites F0280 candidate publish and G0305 append",
                  "REVIVE.C F0280:272-276");
    CHECK_REDMCSB(strstr(e->f0282CancelAnchor, "744-758") != NULL,
                  "evidence cites C162 cancel cleanup range",
                  "REVIVE.C F0282:744-758");
    CHECK_REDMCSB(strstr(e->f0282ConfirmAnchor, "785-806") != NULL,
                  "evidence cites C160/C161 confirm cleanup range",
                  "REVIVE.C F0282:785-806");
    CHECK_REDMCSB(strstr(e->commandGateAnchor, "2159-2181") != NULL &&
                      strstr(e->commandGateAnchor, "2302-2311") != NULL,
                  "evidence cites both !G0299 command gates",
                  "COMMAND.C:2159-2181; COMMAND.C:2302-2311");
    CHECK_REDMCSB(strstr(e->panelEmptyHandAnchor, "1985-1989") != NULL,
                  "evidence cites the empty-hand C040 scan gate",
                  "COMMAND.C F0359:1985-1989");
    CHECK_REDMCSB(strstr(e->contractScope, "contract-only") != NULL &&
                      strstr(e->contractScope, "G0299") != NULL &&
                      strstr(e->contractScope, "G0305") != NULL,
                  "evidence marks this as a no-pending contract slice",
                  "REVIVE.C F0282:744-806");
}

int main(void)
{
    test_fixture_models_open_route_without_pending_party_candidate();
    test_c160_resurrect_no_pending_is_noop();
    test_c162_cancel_no_pending_does_not_close_g0299();
    test_g0299_gates_remain_closed_after_noop();
    test_source_lock_evidence();

    printf("PASS dm1_v1_mirror_candidate_no_pending_resurrect_pc34_compat "
           "%d/%d assertions\n",
           gPasses, gTests);
    return gPasses == gTests ? 0 : 1;
}
