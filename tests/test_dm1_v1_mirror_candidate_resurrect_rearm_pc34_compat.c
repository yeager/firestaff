#include "dm1_v1_mirror_candidate_resurrect_rearm_pc34_compat.h"

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

static Dm1V1MirrorClickClosedStatePc34Compat base_candidate_state(void)
{
    Dm1V1MirrorClickClosedStatePc34Compat state;

    DM1_V1_MirrorClickClosed_InitPc34Compat(&state);
    state.partyChampionCount = 2;
    state.candidateChampionOrdinal = 2u;
    state.inventoryChampionOrdinal = 2u;
    state.leaderIndex = 0;
    state.frontD1cMirrorChampionOrdinal = 0;
    state.champions[0].currentHealth = 42;
    state.champions[0].portraitOrdinal = 7;
    state.champions[1].currentHealth = 0;
    state.champions[1].portraitOrdinal = 11;
    return state;
}

static void test_resurrect_clears_g0299_and_rearms_mirror(void)
{
    Dm1V1MirrorClickClosedStatePc34Compat state = base_candidate_state();
    Dm1V1MirrorCandidateResurrectRearmResultPc34Compat result;
    int changed;

    changed = DM1_V1_MirrorCandidateResurrectRearm_ProcessResurrectPc34Compat(
        &state, &result);

    CHECK_REDMCSB(changed == 1 && result.resurrected == 1,
                  "C160 resurrect finalizes a dead mirror candidate",
                  "REVIVE.C F0282");
    CHECK_REDMCSB(result.candidateChampionOrdinalBefore == 2u &&
                      state.candidateChampionOrdinal == 0u &&
                      result.candidateChampionOrdinalAfter == 0u,
                  "G0299 clears after the resurrect command",
                  "REVIVE.C F0282 line 785");
    CHECK_REDMCSB(result.candidateCleared == 1,
                  "candidate ownership is cleared with G0299",
                  "REVIVE.C F0282 lines 785-806");
    CHECK_REDMCSB(result.panelC040Cleared == 1,
                  "C040 resurrect/reincarnate panel is cleared after resurrect",
                  "REVIVE.C F0282 lines 785-806");
    CHECK_REDMCSB(state.frontD1cMirrorChampionOrdinal == 2 &&
                      result.newFrontD1cMirrorChampionOrdinal == 2,
                  "front D1C mirror ordinal is rearmed to the resurrected champion",
                  "DUNGEON.C 2608-2612; DUNVIEW.C 3913-3928");
    CHECK_REDMCSB(result.frontD1cPortraitIndex == 11,
                  "front D1C portrait draws the resurrected champion portrait index",
                  "DUNVIEW.C 3913-3928");
    CHECK_REDMCSB(result.mirrorRouteRearmed == 1,
                  "mirror route remains live after the resurrect panel clears",
                  "DUNGEON.C 2608-2612; DUNVIEW.C 3913-3928");
    CHECK_REDMCSB(state.champions[1].currentHealth == 1 &&
                      result.currentHealthBefore == 0 &&
                      result.currentHealthAfter == 1,
                  "resurrected dead champions restart at one hit point",
                  "REVIVE.C F0282");
    CHECK_REDMCSB(state.leaderIndex == 1 && result.newLeaderIndex == 1,
                  "resurrect rearm asserts the revived champion as leader",
                  "REVIVE.C F0282 lines 837-841; CLIKCHAM.C F0368");
}

static void test_resurrect_with_no_dead_champion_keeps_state(void)
{
    Dm1V1MirrorClickClosedStatePc34Compat state = base_candidate_state();
    Dm1V1MirrorCandidateResurrectRearmResultPc34Compat result;
    int changed;

    state.frontD1cMirrorChampionOrdinal = 2;
    state.champions[1].currentHealth = 17;
    changed = DM1_V1_MirrorCandidateResurrectRearm_ProcessResurrectPc34Compat(
        &state, &result);

    CHECK_REDMCSB(changed == 0,
                  "C160 does not finalize when the mirror candidate is not dead",
                  "REVIVE.C F0282");
    CHECK_REDMCSB(result.ignoredNoDeadChampion == 1,
                  "live candidate keeps the resurrect panel state",
                  "REVIVE.C F0282; CHAMPION.C death health state");
    CHECK_REDMCSB(state.candidateChampionOrdinal == 2u &&
                      result.candidateChampionOrdinalAfter == 2u,
                  "G0299 stays set when resurrect is rejected",
                  "REVIVE.C F0282 line 785");
    CHECK_REDMCSB(state.champions[1].currentHealth == 17,
                  "live champion health is preserved by the rejected resurrect",
                  "REVIVE.C F0282");
    CHECK_REDMCSB(state.frontD1cMirrorChampionOrdinal == 2 &&
                      result.newFrontD1cMirrorChampionOrdinal == 2,
                  "front D1C mirror route is unchanged on rejected resurrect",
                  "DUNGEON.C 2608-2612; DUNVIEW.C 3913-3928");
    CHECK_REDMCSB(result.panelC040Cleared == 0,
                  "C040 remains open when resurrect does not finalize",
                  "COMMAND.C 2302-2311; REVIVE.C F0282");
}

static void test_post_resurrect_c159_click_re_routes_to_new_leader(void)
{
    Dm1V1MirrorClickClosedStatePc34Compat state = base_candidate_state();
    Dm1V1MirrorCandidateResurrectRearmResultPc34Compat resurrect;
    Dm1V1MirrorCandidateStatusBoxResultPc34Compat status;
    int changed;

    (void)DM1_V1_MirrorCandidateResurrectRearm_ProcessResurrectPc34Compat(
        &state, &resurrect);
    changed = DM1_V1_MirrorCandidateResurrectRearm_ProcessStatusBoxClickPc34Compat(
        &state,
        DM1_V1_MIRROR_CLICK_CLOSED_STATUS_BOX_0_PC34_COMPAT,
        1,
        5,
        DM1_V1_MIRROR_CLICK_CLOSED_MOUSE_LEFT_PC34_COMPAT,
        &status);

    CHECK_REDMCSB(resurrect.resurrected == 1 &&
                      resurrect.candidateChampionOrdinalAfter == 0u,
                  "post-resurrect setup has C040 closed and G0299 clear",
                  "REVIVE.C F0282 line 785");
    CHECK_REDMCSB(status.statusBox.dispatchedStatusBoxClick == 1,
                  "C012 status-box dispatch is re-enabled after resurrect",
                  "COMMAND.C 2158-2162");
    CHECK_REDMCSB(status.statusBox.scannedChampionNameRows == 1,
                  "C159 name-row scan runs through G0455 after C040 closes",
                  "COMMAND.C 484-488");
    CHECK_REDMCSB(status.statusBox.nestedCommand ==
                      DM1_V1_MIRROR_CLICK_CLOSED_SET_LEADER_0_PC34_COMPAT,
                  "C159 maps the click to C016 set-leader champion 0",
                  "COMMAND.C 484-488");
    CHECK_REDMCSB(changed == 1 && status.statusBoxChangedLeader == 1,
                  "F0368 changes the leader after resurrect clears G0299",
                  "CLIKCHAM.C F0368 lines 51-72");
    CHECK_REDMCSB(state.leaderIndex == 0 &&
                      status.statusBox.newLeaderIndex == 0,
                  "new leader is champion 0 after the C159 click",
                  "CLIKCHAM.C F0368 lines 66-72");
    CHECK_REDMCSB(state.frontD1cMirrorChampionOrdinal == 1 &&
                      status.newFrontD1cMirrorChampionOrdinal == 1,
                  "front D1C mirror ordinal updates to the new leader",
                  "DUNGEON.C 2608-2612; DUNVIEW.C 3913-3928");
    CHECK_REDMCSB(status.frontD1cPortraitIndex == 7,
                  "front D1C portrait now draws the new leader portrait",
                  "DUNVIEW.C 3913-3928");
    CHECK_REDMCSB(status.mirrorRouteLive == 1 &&
                      state.candidateChampionOrdinal == 0u,
                  "mirror route remains live while G0299 stays clear",
                  "COMMAND.C 2158-2162; DUNGEON.C 2608-2612");
}

static void test_post_resurrect_inventory_toggle_re_enabled(void)
{
    Dm1V1MirrorClickClosedStatePc34Compat state = base_candidate_state();
    Dm1V1MirrorCandidateCommandGateResultPc34Compat beforeGate;
    Dm1V1MirrorCandidateCommandGateResultPc34Compat afterGate;
    Dm1V1MirrorCandidateResurrectRearmResultPc34Compat resurrect;
    int allowedBefore;
    int allowedAfter;

    allowedBefore = DM1_V1_MirrorCandidateResurrectRearm_CanProcessCommandPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_ACTION_AREA_COMMAND_PC34_COMPAT,
        &beforeGate);
    (void)DM1_V1_MirrorCandidateResurrectRearm_ProcessResurrectPc34Compat(
        &state, &resurrect);
    allowedAfter = DM1_V1_MirrorCandidateResurrectRearm_CanProcessCommandPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_ACTION_AREA_COMMAND_PC34_COMPAT,
        &afterGate);

    CHECK_REDMCSB(allowedBefore == 0 &&
                      beforeGate.blockedByCandidatePanel == 1,
                  "C040-owned G0299 blocks action/inventory processing",
                  "COMMAND.C 2302-2311");
    CHECK_REDMCSB(resurrect.panelC040Cleared == 1 &&
                      state.candidateChampionOrdinal == 0u,
                  "resurrect closes C040 and clears G0299 before retesting input",
                  "REVIVE.C F0282 line 785");
    CHECK_REDMCSB(allowedAfter == 1 && afterGate.commandAllowed == 1,
                  "action/inventory processing is re-enabled after G0299 clears",
                  "COMMAND.C 2302-2311");
    CHECK_REDMCSB(afterGate.panelC040Closed == 1 &&
                      afterGate.blockedByCandidatePanel == 0,
                  "closed C040 no longer owns the command gate",
                  "COMMAND.C 2302-2311");
}

static void test_source_evidence_mentions_all_anchors(void)
{
    const char *evidence =
        DM1_V1_MirrorCandidateResurrectRearm_SourceEvidencePc34Compat();

    CHECK_REDMCSB(evidence != NULL,
                  "source evidence string is available",
                  "REVIVE.C F0282; DUNVIEW.C 3913-3928");
    CHECK_REDMCSB(strstr(evidence, "REVIVE.C F0282") != NULL,
                  "source evidence mentions REVIVE.C F0282",
                  "REVIVE.C F0282");
    CHECK_REDMCSB(strstr(evidence, "DUNVIEW.C 3913-3928") != NULL,
                  "source evidence mentions DUNVIEW.C mirror redraw lines",
                  "DUNVIEW.C 3913-3928");
    CHECK_REDMCSB(strstr(evidence, "DUNGEON.C 2608-2612") != NULL,
                  "source evidence mentions DUNGEON.C mirror ordinal lines",
                  "DUNGEON.C 2608-2612");
    CHECK_REDMCSB(strstr(evidence, "COMMAND.C 2302-2311") != NULL,
                  "source evidence mentions COMMAND.C action gate lines",
                  "COMMAND.C 2302-2311");
}

int main(void)
{
    test_resurrect_clears_g0299_and_rearms_mirror();
    test_resurrect_with_no_dead_champion_keeps_state();
    test_post_resurrect_c159_click_re_routes_to_new_leader();
    test_post_resurrect_inventory_toggle_re_enabled();
    test_source_evidence_mentions_all_anchors();

    printf("PASS dm1_v1_mirror_candidate_resurrect_rearm_pc34_compat %d/%d assertions\n",
           gPasses, gTests);
    return gPasses == gTests ? 0 : 1;
}
