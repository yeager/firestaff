#include "dm1_v1_mirror_candidate_pickup_right_click_pc34_compat.h"

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

static void test_right_click_c159_publishes_candidate_and_c040_panel(void)
{
    Dm1V1MirrorCandidatePickupRightClickStatePc34Compat state;
    Dm1V1MirrorCandidatePickupRightClickResultPc34Compat result;
    int changed;

    DM1_V1_MirrorCandidatePickupRightClick_InitPc34Compat(&state);

    CHECK_REDMCSB(state.g0299CandidateChampionOrdinal == 2u,
                  "fixture starts with a pending G0299 mirror candidate",
                  "REVIVE.C F0280:272-276");
    CHECK_REDMCSB(state.partyChampionCount == 2 &&
                      state.preC040PartyChampionCount == 1,
                  "fixture models the F0280 candidate append over one base champion",
                  "REVIVE.C F0280:272-276");
    CHECK_REDMCSB(state.c040PanelOpen == 0 &&
                      state.c040PanelPixelsDrawn == 0,
                  "C040 has not been published before the right-click hook",
                  "REVIVE.C F0282:744-758,785-806");
    CHECK_REDMCSB(state.rows[0].present == 1 &&
                      state.rows[0].zone == 159,
                  "row zero is the C159 champion-name candidate row",
                  "COMMAND.C:484-488");
    CHECK_REDMCSB(state.rows[0].left == 0 &&
                      state.rows[0].right == 42 &&
                      state.rows[0].top == 0 &&
                      state.rows[0].bottom == 6,
                  "C159 row uses inclusive name-row hit bounds",
                  "COMMAND.C:484-488");
    CHECK_REDMCSB(state.leaderHandEmpty == 1 &&
                      state.leaderHandThing ==
                          DM1_V1_MIRROR_CANDIDATE_PICKUP_RIGHT_CLICK_THING_NONE_PC34_COMPAT,
                  "leader hand starts empty for the panel pickup path",
                  "COMMAND.C F0359:1985-1989; CHAMPION.C F0297:243-268");

    changed = DM1_V1_MirrorCandidatePickupRightClick_ApplyPc34Compat(
        &state,
        1,
        5,
        DM1_V1_MIRROR_CANDIDATE_PICKUP_RIGHT_CLICK_MOUSE_RIGHT_PC34_COMPAT,
        &result);

    CHECK_REDMCSB(changed == 1 && result.consumed == 1,
                  "right-click on C159 is consumed by the pickup hook",
                  "MOVESENS.C:1501-1503; REVIVE.C F0280:272-276");
    CHECK_REDMCSB(result.resolvedRowIndex == 0 &&
                      result.resolvedZone == 159,
                  "per-row resolver selects the C159 candidate row",
                  "COMMAND.C:484-488");
    CHECK_REDMCSB(result.rightClickOnly == 1,
                  "the publish path is driven by right-click without left-click",
                  "COMMAND.C:484-488");
    CHECK_REDMCSB(result.noLeftClickCommand == 1 &&
                      state.leftClickCommandCount == 0,
                  "right-only pickup does not synthesize the G0455 left-click command",
                  "COMMAND.C:484-488");
    CHECK_REDMCSB(result.publishedCandidate == 1 &&
                      state.candidateChampionOrdinal == 2u,
                  "right-click publishes the C159 candidate identity",
                  "REVIVE.C F0280:272-276");
    CHECK_REDMCSB(result.g0299Before == 2u &&
                      result.g0299After == 2u,
                  "publish preserves the pending G0299 owner until a clear",
                  "COMMAND.C:2159-2181; COMMAND.C:2302-2311");
    CHECK_REDMCSB(result.c040PanelOpenBefore == 0 &&
                      result.c040PanelOpenAfter == 1,
                  "publish opens the C040 candidate panel",
                  "REVIVE.C F0282:744-758");
    CHECK_REDMCSB(result.c040PanelPixelsBefore == 0 &&
                      result.c040PanelPixelsAfter == 1,
                  "publish records the C040 panel-pixel side effect",
                  "REVIVE.C F0282:744-758");
    CHECK_REDMCSB(result.candidatePublishCountAfter ==
                      result.candidatePublishCountBefore + 1,
                  "candidate publish count increments once",
                  "REVIVE.C F0280:272-276");
    CHECK_REDMCSB(result.c040PanelPublishCountAfter ==
                      result.c040PanelPublishCountBefore + 1,
                  "C040 panel publish count increments once",
                  "REVIVE.C F0282:744-758");
    CHECK_REDMCSB(result.leaderHandPutCountAfter ==
                      result.leaderHandPutCountBefore + 1,
                  "F0297-style leader-hand put side effect is recorded",
                  "CHAMPION.C F0297:243-268");
    CHECK_REDMCSB(result.leaderHandThingAfter == state.rows[0].leaderHandThing &&
                      result.leaderHandEmptyAfter == 0,
                  "published C159 token moves into the leader hand",
                  "CHAMPION.C F0297:250-266");
    CHECK_REDMCSB(result.partyChampionCountBefore == 2 &&
                      result.partyChampionCountAfter == 2,
                  "right-click publish does not append a duplicate party candidate",
                  "REVIVE.C F0280:272-276");
    CHECK_REDMCSB(result.c159ChampionIconGuardHeld == 1,
                  "C159 right-click stays outside spell/action dispatch while G0299 is live",
                  "COMMAND.C:2302-2311; COMMAND.C:484-488");
}

static void test_second_right_click_clears_published_row_without_double_publish(void)
{
    Dm1V1MirrorCandidatePickupRightClickStatePc34Compat state;
    Dm1V1MirrorCandidatePickupRightClickResultPc34Compat first;
    Dm1V1MirrorCandidatePickupRightClickResultPc34Compat second;

    DM1_V1_MirrorCandidatePickupRightClick_InitPc34Compat(&state);
    (void)DM1_V1_MirrorCandidatePickupRightClick_ApplyPc34Compat(
        &state,
        1,
        5,
        DM1_V1_MIRROR_CANDIDATE_PICKUP_RIGHT_CLICK_MOUSE_RIGHT_PC34_COMPAT,
        &first);
    (void)DM1_V1_MirrorCandidatePickupRightClick_ApplyPc34Compat(
        &state,
        1,
        5,
        DM1_V1_MIRROR_CANDIDATE_PICKUP_RIGHT_CLICK_MOUSE_RIGHT_PC34_COMPAT,
        &second);

    CHECK_REDMCSB(first.publishedCandidate == 1,
                  "first right-click publishes before the toggle clear",
                  "REVIVE.C F0280:272-276");
    CHECK_REDMCSB(second.consumed == 1 &&
                      second.clearedCandidate == 1,
                  "second right-click on the published row toggles to clear",
                  "REVIVE.C F0282:744-758,785-806");
    CHECK_REDMCSB(second.noDoublePublish == 1,
                  "second right-click does not publish a duplicate candidate",
                  "REVIVE.C F0280:272-276");
    CHECK_REDMCSB(second.candidatePublishCountBefore == 1 &&
                      second.candidatePublishCountAfter == 1,
                  "candidate publish count remains at one on toggle clear",
                  "REVIVE.C F0280:272-276");
    CHECK_REDMCSB(second.g0299Before == 2u &&
                      second.g0299After == 0u,
                  "toggle clear releases the G0299 pending candidate",
                  "REVIVE.C F0282:744-758,785-806");
    CHECK_REDMCSB(second.candidateChampionOrdinalBefore == 2u &&
                      second.candidateChampionOrdinalAfter == 0u,
                  "toggle clear removes the published candidate identity",
                  "REVIVE.C F0282:744-758,785-806");
    CHECK_REDMCSB(second.partyChampionCountBefore == 2 &&
                      second.partyChampionCountAfter == 1,
                  "toggle clear restores the pre-C040 party count",
                  "REVIVE.C F0282:744-758");
    CHECK_REDMCSB(second.c040PanelOpenBefore == 1 &&
                      second.c040PanelOpenAfter == 0,
                  "toggle clear closes C040",
                  "REVIVE.C F0282:744-758");
    CHECK_REDMCSB(second.c040PanelPixelsBefore == 1 &&
                      second.c040PanelPixelsAfter == 0,
                  "toggle clear removes C040 panel pixels",
                  "REVIVE.C F0282:744-758");
    CHECK_REDMCSB(second.candidateClearCountAfter ==
                      second.candidateClearCountBefore + 1,
                  "candidate clear count increments once",
                  "REVIVE.C F0282:744-758,785-806");
    CHECK_REDMCSB(second.c040PanelClearCountAfter ==
                      second.c040PanelClearCountBefore + 1,
                  "C040 clear count increments once",
                  "REVIVE.C F0282:744-758,785-806");
    CHECK_REDMCSB(second.leaderHandRemoveCountAfter ==
                      second.leaderHandRemoveCountBefore + 1,
                  "F0298-style leader-hand remove side effect is recorded",
                  "CHAMPION.C F0298:270-285");
    CHECK_REDMCSB(second.leaderHandThingAfter ==
                      DM1_V1_MIRROR_CANDIDATE_PICKUP_RIGHT_CLICK_THING_NONE_PC34_COMPAT &&
                      second.leaderHandEmptyAfter == 1,
                  "toggle clear empties the leader hand",
                  "CHAMPION.C F0298:279-285");
    CHECK_REDMCSB(second.leaderHandPutCountBefore == 1 &&
                      second.leaderHandPutCountAfter == 1,
                  "toggle clear does not enter F0297 put again",
                  "CHAMPION.C F0297/F0298:243-285");
}

static void test_not_pending_gate_rejects_right_click(void)
{
    Dm1V1MirrorCandidatePickupRightClickStatePc34Compat state;
    Dm1V1MirrorCandidatePickupRightClickResultPc34Compat result;
    int changed;

    DM1_V1_MirrorCandidatePickupRightClick_InitPc34Compat(&state);
    state.g0299CandidateChampionOrdinal = 0u;
    state.partyChampionCount = state.preC040PartyChampionCount;

    changed = DM1_V1_MirrorCandidatePickupRightClick_ApplyPc34Compat(
        &state,
        1,
        5,
        DM1_V1_MIRROR_CANDIDATE_PICKUP_RIGHT_CLICK_MOUSE_RIGHT_PC34_COMPAT,
        &result);

    CHECK_REDMCSB(changed == 0 && result.consumed == 0,
                  "!G0299 rejects right-click pickup when C040 is not pending",
                  "COMMAND.C:2159-2181; COMMAND.C:2302-2311");
    CHECK_REDMCSB(result.rejectedPanelNotPending == 1,
                  "result records the not-pending gate",
                  "COMMAND.C:2159-2181; COMMAND.C:2302-2311");
    CHECK_REDMCSB(result.resolvedZone == 159,
                  "C159 still resolves before the pending-panel gate rejects it",
                  "COMMAND.C:484-488");
    CHECK_REDMCSB(result.candidateChampionOrdinalAfter == 0u &&
                      state.candidatePublishCount == 0,
                  "not-pending right-click does not publish a candidate",
                  "REVIVE.C F0280:272-276");
    CHECK_REDMCSB(result.c040PanelOpenAfter == 0 &&
                      result.c040PanelPixelsAfter == 0,
                  "not-pending right-click does not publish C040",
                  "REVIVE.C F0282:744-758");
    CHECK_REDMCSB(result.leaderHandThingAfter ==
                      DM1_V1_MIRROR_CANDIDATE_PICKUP_RIGHT_CLICK_THING_NONE_PC34_COMPAT,
                  "not-pending right-click leaves the leader hand empty",
                  "CHAMPION.C F0297/F0298:243-285");
}

static void test_leader_hand_full_rejects_publish(void)
{
    Dm1V1MirrorCandidatePickupRightClickStatePc34Compat state;
    Dm1V1MirrorCandidatePickupRightClickResultPc34Compat result;
    int changed;

    DM1_V1_MirrorCandidatePickupRightClick_InitPc34Compat(&state);
    state.leaderHandEmpty = 0;
    state.leaderHandThing = 0x0BEEu;

    changed = DM1_V1_MirrorCandidatePickupRightClick_ApplyPc34Compat(
        &state,
        1,
        5,
        DM1_V1_MIRROR_CANDIDATE_PICKUP_RIGHT_CLICK_MOUSE_RIGHT_PC34_COMPAT,
        &result);

    CHECK_REDMCSB(changed == 0 && result.rejectedLeaderHandFull == 1,
                  "full leader hand blocks C040 right-click publish",
                  "COMMAND.C F0359:1985-1989");
    CHECK_REDMCSB(result.candidatePublishCountAfter == 0 &&
                      result.c040PanelPublishCountAfter == 0,
                  "full-hand guard prevents both candidate and C040 publish",
                  "REVIVE.C F0280:272-276; REVIVE.C F0282:744-758");
    CHECK_REDMCSB(result.leaderHandThingBefore == 0x0BEEu &&
                      result.leaderHandThingAfter == 0x0BEEu,
                  "full-hand guard preserves the existing leader-hand object",
                  "CHAMPION.C F0297/F0298:243-285");
    CHECK_REDMCSB(result.leaderHandPutCountAfter == 0 &&
                      result.leaderHandRemoveCountAfter == 0,
                  "full-hand guard enters neither F0297 nor F0298",
                  "CHAMPION.C F0297/F0298:243-285");
    CHECK_REDMCSB(result.g0299After == 2u &&
                      result.partyChampionCountAfter == 2,
                  "full-hand guard keeps the pending candidate append intact",
                  "REVIVE.C F0280:272-276");
}

static void test_deadzone_right_click_is_noop(void)
{
    Dm1V1MirrorCandidatePickupRightClickStatePc34Compat state;
    Dm1V1MirrorCandidatePickupRightClickResultPc34Compat result;
    int changed;

    DM1_V1_MirrorCandidatePickupRightClick_InitPc34Compat(&state);
    changed = DM1_V1_MirrorCandidatePickupRightClick_ApplyPc34Compat(
        &state,
        60,
        5,
        DM1_V1_MIRROR_CANDIDATE_PICKUP_RIGHT_CLICK_MOUSE_RIGHT_PC34_COMPAT,
        &result);

    CHECK_REDMCSB(changed == 0 && result.deadzoneSkipped == 1,
                  "right-click between C159 and C160 is a deadzone no-op",
                  "COMMAND.C:484-488");
    CHECK_REDMCSB(result.resolvedRowIndex ==
                      DM1_V1_MIRROR_CANDIDATE_PICKUP_RIGHT_CLICK_NONE_PC34_COMPAT,
                  "deadzone does not resolve a candidate row",
                  "COMMAND.C:484-488");
    CHECK_REDMCSB(result.candidateChampionOrdinalAfter == 0u &&
                      result.g0299After == 2u,
                  "deadzone leaves pending candidate identity unchanged",
                  "REVIVE.C F0280:272-276");
    CHECK_REDMCSB(result.c040PanelPublishCountAfter == 0 &&
                      result.c040PanelOpenAfter == 0,
                  "deadzone does not publish C040",
                  "REVIVE.C F0282:744-758");
    CHECK_REDMCSB(result.leaderHandPutCountAfter == 0 &&
                      result.leaderHandThingAfter ==
                          DM1_V1_MIRROR_CANDIDATE_PICKUP_RIGHT_CLICK_THING_NONE_PC34_COMPAT,
                  "deadzone does not enter leader-hand pickup",
                  "CHAMPION.C F0297/F0298:243-285");
}

static void test_empty_row_right_click_is_noop(void)
{
    Dm1V1MirrorCandidatePickupRightClickStatePc34Compat state;
    Dm1V1MirrorCandidatePickupRightClickResultPc34Compat result;
    int changed;

    DM1_V1_MirrorCandidatePickupRightClick_InitPc34Compat(&state);
    changed = DM1_V1_MirrorCandidatePickupRightClick_ApplyPc34Compat(
        &state,
        70,
        5,
        DM1_V1_MIRROR_CANDIDATE_PICKUP_RIGHT_CLICK_MOUSE_RIGHT_PC34_COMPAT,
        &result);

    CHECK_REDMCSB(changed == 0 && result.emptyRowNoop == 1,
                  "right-click on empty C160 row is a no-op",
                  "COMMAND.C:484-488");
    CHECK_REDMCSB(result.resolvedRowIndex == 1 &&
                      result.resolvedZone == 160,
                  "per-row resolver reaches the empty C160 row",
                  "COMMAND.C:484-488");
    CHECK_REDMCSB(result.candidateChampionOrdinalAfter == 0u &&
                      result.candidatePublishCountAfter == 0,
                  "empty row does not publish a candidate",
                  "REVIVE.C F0280:272-276");
    CHECK_REDMCSB(result.c040PanelOpenAfter == 0 &&
                      result.c040PanelPublishCountAfter == 0,
                  "empty row does not publish C040",
                  "REVIVE.C F0282:744-758");
    CHECK_REDMCSB(result.leaderHandPutCountAfter == 0 &&
                      result.leaderHandRemoveCountAfter == 0,
                  "empty row enters no leader-hand route",
                  "CHAMPION.C F0297/F0298/F0302:243-285,662-706");
}

static void test_source_lock_evidence(void)
{
    const Dm1V1MirrorCandidatePickupRightClickEvidencePc34Compat *e =
        DM1_V1_MirrorCandidatePickupRightClick_EvidencePc34Compat();

    CHECK_REDMCSB(e != NULL,
                  "evidence accessor returns source-lock metadata",
                  "REVIVE.C F0280:272-276");
    CHECK_REDMCSB(strstr(e->candidatePublishAnchor, "F0280") != NULL &&
                      strstr(e->candidatePublishAnchor, "272-276") != NULL,
                  "evidence cites candidate publish lines",
                  "REVIVE.C F0280:272-276");
    CHECK_REDMCSB(strstr(e->candidateClearAnchor, "744-758") != NULL &&
                      strstr(e->candidateClearAnchor, "785-806") != NULL,
                  "evidence cites both candidate clear ranges",
                  "REVIVE.C F0282:744-758,785-806");
    CHECK_REDMCSB(strstr(e->commandGateAnchor, "2159-2181") != NULL &&
                      strstr(e->commandGateAnchor, "2302-2311") != NULL,
                  "evidence cites the !G0299 command gates",
                  "COMMAND.C:2159-2181; COMMAND.C:2302-2311");
    CHECK_REDMCSB(strstr(e->championLeaderHandAnchor, "F0297") != NULL &&
                      strstr(e->championLeaderHandAnchor, "F0298") != NULL &&
                      strstr(e->championLeaderHandAnchor, "F0302") != NULL,
                  "evidence cites leader-hand put/remove/slot routes",
                  "CHAMPION.C F0297/F0298/F0302:243-285,662-706");
    CHECK_REDMCSB(strstr(e->mirrorCellAnchor, "MOVESENS.C:1501-1503") != NULL,
                  "evidence cites mirror-cell C127 click route",
                  "MOVESENS.C:1501-1503");
    CHECK_REDMCSB(strstr(e->c159NameRowAnchor, "C159") != NULL &&
                      strstr(e->c159NameRowAnchor, "484-488") != NULL,
                  "evidence cites C159 name-row mapping",
                  "COMMAND.C:484-488");
    CHECK_REDMCSB(strstr(e->panelEmptyHandAnchor, "1985-1989") != NULL,
                  "evidence cites the C040 empty-hand panel gate",
                  "COMMAND.C F0359:1985-1989");
    CHECK_REDMCSB(strstr(e->contractScope, "contract-only") != NULL,
                  "evidence marks this as a deterministic contract gate",
                  "REVIVE.C F0280:272-276");
}

int main(void)
{
    test_right_click_c159_publishes_candidate_and_c040_panel();
    test_second_right_click_clears_published_row_without_double_publish();
    test_not_pending_gate_rejects_right_click();
    test_leader_hand_full_rejects_publish();
    test_deadzone_right_click_is_noop();
    test_empty_row_right_click_is_noop();
    test_source_lock_evidence();

    printf("PASS dm1_v1_mirror_candidate_pickup_right_click_pc34_compat "
           "%d/%d assertions\n",
           gPasses, gTests);
    return gPasses == gTests ? 0 : 1;
}
