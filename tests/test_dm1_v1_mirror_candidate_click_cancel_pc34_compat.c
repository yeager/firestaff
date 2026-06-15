#include "dm1_v1_mirror_candidate_click_cancel_pc34_compat.h"

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

static void test_no_candidate_front_cell_click_is_noop(void)
{
    Dm1V1MirrorCandidateClickCancelStatePc34Compat state;
    Dm1V1MirrorCandidateClickCancelResultPc34Compat result;
    int changed;
    unsigned int leaderHandBefore;
    int partyCountBefore;
    int frontOrdinalBefore;
    int frontPortraitBefore;
    unsigned int absenceHashBefore;

    dm1_v1_mirror_candidate_click_cancel_init_pc34(&state);
    leaderHandBefore = state.leaderHandThing;
    partyCountBefore = state.partyChampionCount;
    frontOrdinalBefore = state.frontD1cMirrorChampionOrdinal;
    frontPortraitBefore = state.frontD1cMirrorPortraitIndex;
    absenceHashBefore = state.c040AbsencePixelHash;

    CHECK_REDMCSB(state.active == 1,
                  "fixture starts in an active Hall mirror runtime state",
                  "DUNVIEW.C 8488-8533");
    CHECK_REDMCSB(state.partyChampionCount == 1 &&
                      state.preC040PartyChampionCount == 1,
                  "fixture starts with exactly one real party champion",
                  "REVIVE.C F0280 lines 272-276");
    CHECK_REDMCSB(state.party[0].present == 1 &&
                      state.party[0].championOrdinal == 1u,
                  "party champion identity is present before the click",
                  "CHAMPION.C F0302 lines 662-706");
    CHECK_REDMCSB(state.g0299CandidateChampionOrdinal == 0u,
                  "G0299 starts clear",
                  "COMMAND.C 2159-2181; COMMAND.C 2302-2311");
    CHECK_REDMCSB(state.candidateChampionOrdinal == 0u &&
                      state.candidateAppendCount == 0,
                  "no mirror candidate identity is selected before the click",
                  "REVIVE.C F0280 lines 272-276");
    CHECK_REDMCSB(state.c040PanelOpen == 0 &&
                      state.c040PanelPixelsDrawn == 0,
                  "C040 resurrect/reincarnate panel starts closed",
                  "REVIVE.C F0282 lines 744-758");
    CHECK_REDMCSB(state.inventoryPanelOpen == 0 &&
                      state.inventoryChampionOrdinal == 0u,
                  "inventory is closed before the click",
                  "COMMAND.C 2159-2181");
    CHECK_REDMCSB(state.mirrorRouteArmed == 1 &&
                      state.frontD1cCellVisible == 1,
                  "visible D1C front mirror route is armed before the click",
                  "DUNVIEW.C 3913-3928");
    CHECK_REDMCSB(state.frontD1cMirrorChampionOrdinal == 4 &&
                      state.frontD1cMirrorPortraitIndex == 13,
                  "visible D1C mirror identity is distinct from the party",
                  "DUNVIEW.C 3913-3928");
    CHECK_REDMCSB(state.leaderIndex == 0 &&
                      state.leaderHandThing == leaderHandBefore,
                  "leader and leader hand are stable before the click",
                  "CHAMPION.C F0297/F0298 lines 243-285");

    changed = dm1_v1_mirror_candidate_click_cancel_front_cell_pc34(
        &state, &result);

    CHECK_REDMCSB(changed == 0,
                  "D1C front cell click with no candidate is not consumed",
                  "MOVESENS.C 1501-1503; REVIVE.C F0280");
    CHECK_REDMCSB(result.command ==
                      DM1_V1_MIRROR_CANDIDATE_CLICK_CANCEL_COMMAND_DUNGEON_VIEW_PC34_COMPAT,
                  "result records the source dungeon-view command",
                  "COMMAND.C C080 dungeon view dispatch");
    CHECK_REDMCSB(result.ignoredFrontCellOnly == 1 &&
                      result.consumed == 0,
                  "click is classified as front-cell-only no-op",
                  "MOVESENS.C 1501-1503");
    CHECK_REDMCSB(result.noCandidateBefore == 1 &&
                      result.noCandidateAfter == 1,
                  "no-candidate state is preserved",
                  "REVIVE.C F0280 lines 272-276; REVIVE.C F0282 lines 744-758");
    CHECK_REDMCSB(state.g0299CandidateChampionOrdinal == 0u &&
                      result.g0299Before == 0u &&
                      result.g0299After == 0u,
                  "G0299 remains clear after the cell click",
                  "COMMAND.C 2159-2181; COMMAND.C 2302-2311");
    CHECK_REDMCSB(state.candidateChampionOrdinal == 0u &&
                      result.candidateChampionOrdinalBefore == 0u &&
                      result.candidateChampionOrdinalAfter == 0u,
                  "candidate identity stays none",
                  "REVIVE.C F0280 lines 272-276");
    CHECK_REDMCSB(state.candidateAppendCount == 0 &&
                      result.candidateAppendCountBefore == 0 &&
                      result.candidateAppendCountAfter == 0,
                  "candidate append count stays zero",
                  "REVIVE.C F0280 lines 272-276");
    CHECK_REDMCSB(result.candidateCountStayedZero == 1,
                  "candidate-count no-op flag is set",
                  "REVIVE.C F0280 lines 272-276");
    CHECK_REDMCSB(result.candidateIdentityStayedNone == 1,
                  "candidate identity no-op flag is set",
                  "REVIVE.C F0282 lines 744-758");
    CHECK_REDMCSB(state.partyChampionCount == partyCountBefore &&
                      result.partyChampionCountBefore == partyCountBefore &&
                      result.partyChampionCountAfter == partyCountBefore,
                  "party count is not incremented by the cell click",
                  "REVIVE.C F0280 lines 272-276");
    CHECK_REDMCSB(result.partyCountUnchanged == 1,
                  "party-count no-op flag is set",
                  "REVIVE.C F0280 lines 272-276");
    CHECK_REDMCSB(state.party[0].present == 1 &&
                      state.party[0].championOrdinal == 1u &&
                      state.party[0].currentHealth == 72,
                  "real party champion is unchanged",
                  "CHAMPION.C F0302 lines 662-706");
    CHECK_REDMCSB(state.c040PanelOpen == 0 &&
                      result.noC040Before == 1 &&
                      result.noC040After == 1,
                  "C040 panel does not open",
                  "REVIVE.C F0282 lines 744-758");
    CHECK_REDMCSB(result.c040PanelStayedClosed == 1,
                  "C040 closed-state flag is preserved",
                  "REVIVE.C F0282 lines 744-758");
    CHECK_REDMCSB(state.c040PanelPixelsDrawn == 0 &&
                      result.c040PanelPixelsBefore == 0 &&
                      result.c040PanelPixelsAfter == 0,
                  "real C040 pixels remain absent",
                  "DUNVIEW.C 3913-3928; PANEL C040 absence contract");
    CHECK_REDMCSB(state.c040AbsencePixelHash == absenceHashBefore &&
                      result.c040AbsencePixelHashBefore == absenceHashBefore &&
                      result.c040AbsencePixelHashAfter == absenceHashBefore,
                  "C040 absence pixel hash is preserved",
                  "PANEL C040 absence contract");
    CHECK_REDMCSB(result.c040PixelsPreserved == 1,
                  "C040 pixel-preservation flag is set",
                  "PANEL C040 absence contract");
    CHECK_REDMCSB(state.inventoryPanelOpen == 0 &&
                      result.inventoryPanelOpenBefore == 0 &&
                      result.inventoryPanelOpenAfter == 0,
                  "inventory panel stays closed",
                  "COMMAND.C 2159-2181");
    CHECK_REDMCSB(state.inventoryChampionOrdinal == 0u &&
                      result.inventoryChampionOrdinalBefore == 0u &&
                      result.inventoryChampionOrdinalAfter == 0u,
                  "inventory champion stays none",
                  "COMMAND.C 2159-2181");
    CHECK_REDMCSB(result.inventoryStayedClosed == 1,
                  "inventory no-op flag is set",
                  "COMMAND.C 2159-2181");
    CHECK_REDMCSB(state.frontD1cMirrorChampionOrdinal == frontOrdinalBefore &&
                      result.frontD1cMirrorChampionOrdinalBefore ==
                          frontOrdinalBefore &&
                      result.frontD1cMirrorChampionOrdinalAfter ==
                          frontOrdinalBefore,
                  "mirror route ordinal stays armed and unchanged",
                  "DUNVIEW.C 3913-3928");
    CHECK_REDMCSB(state.frontD1cMirrorPortraitIndex == frontPortraitBefore &&
                      result.frontD1cMirrorPortraitIndexBefore ==
                          frontPortraitBefore &&
                      result.frontD1cMirrorPortraitIndexAfter ==
                          frontPortraitBefore,
                  "visible mirror portrait identity is unchanged",
                  "DUNVIEW.C 3913-3928");
    CHECK_REDMCSB(state.mirrorRouteArmed == 1 &&
                      result.mirrorRouteArmedBefore == 1 &&
                      result.mirrorRouteArmedAfter == 1,
                  "mirror route remains armed",
                  "DUNVIEW.C 8488-8533");
    CHECK_REDMCSB(result.mirrorRouteStayedArmed == 1,
                  "mirror route no-op flag is set",
                  "DUNVIEW.C 3913-3928");
    CHECK_REDMCSB(state.leaderHandThing == leaderHandBefore &&
                      result.leaderHandThingBefore == leaderHandBefore &&
                      result.leaderHandThingAfter == leaderHandBefore,
                  "leader hand is unchanged",
                  "CHAMPION.C F0297/F0298 lines 243-285");
    CHECK_REDMCSB(result.leaderHandUnchanged == 1,
                  "leader-hand no-op flag is set",
                  "CHAMPION.C F0297/F0298 lines 243-285");
    CHECK_REDMCSB(state.leaderIndex == 0 &&
                      result.leaderIndexBefore == 0 &&
                      result.leaderIndexAfter == 0,
                  "leader index is unchanged",
                  "CHAMPION.C F0302 lines 662-706");
    CHECK_REDMCSB(result.championIdentityUnchanged == 1,
                  "champion identity no-op flag is set",
                  "DUNVIEW.C 3913-3928");
}

static void test_repeat_front_cell_click_remains_noop(void)
{
    Dm1V1MirrorCandidateClickCancelStatePc34Compat state;
    Dm1V1MirrorCandidateClickCancelResultPc34Compat first;
    Dm1V1MirrorCandidateClickCancelResultPc34Compat second;
    int changedFirst;
    int changedSecond;

    dm1_v1_mirror_candidate_click_cancel_init_pc34(&state);
    changedFirst = dm1_v1_mirror_candidate_click_cancel_front_cell_pc34(
        &state, &first);
    changedSecond = dm1_v1_mirror_candidate_click_cancel_front_cell_pc34(
        &state, &second);

    CHECK_REDMCSB(changedFirst == 0 && changedSecond == 0,
                  "repeat D1C cell clicks remain unconsumed",
                  "MOVESENS.C 1501-1503");
    CHECK_REDMCSB(first.noCandidateAfter == 1 &&
                      second.noCandidateBefore == 1 &&
                      second.noCandidateAfter == 1,
                  "repeat click starts and ends with no candidate",
                  "REVIVE.C F0280 lines 272-276");
    CHECK_REDMCSB(second.c040PanelStayedClosed == 1 &&
                      second.c040PixelsPreserved == 1,
                  "repeat click preserves C040 absence",
                  "REVIVE.C F0282 lines 744-758");
    CHECK_REDMCSB(second.candidateCountStayedZero == 1 &&
                      state.candidateAppendCount == 0,
                  "repeat click does not append a candidate",
                  "REVIVE.C F0280 lines 272-276");
    CHECK_REDMCSB(second.inventoryStayedClosed == 1 &&
                      state.inventoryPanelOpen == 0,
                  "repeat click does not enable inventory",
                  "COMMAND.C 2159-2181");
    CHECK_REDMCSB(second.mirrorRouteStayedArmed == 1 &&
                      state.mirrorRouteArmed == 1,
                  "repeat click keeps the mirror route armed",
                  "DUNVIEW.C 8488-8533");
    CHECK_REDMCSB(second.leaderHandUnchanged == 1 &&
                      first.leaderHandThingAfter == second.leaderHandThingAfter,
                  "repeat click keeps the leader hand unchanged",
                  "CHAMPION.C F0297/F0298 lines 243-285");
    CHECK_REDMCSB(first.frontD1cMirrorChampionOrdinalAfter ==
                      second.frontD1cMirrorChampionOrdinalAfter,
                  "repeat click keeps the same D1C mirror ordinal",
                  "DUNVIEW.C 3913-3928");
    CHECK_REDMCSB(first.c040AbsencePixelHashAfter ==
                      second.c040AbsencePixelHashAfter,
                  "repeat click keeps the same C040 absence pixels",
                  "PANEL C040 absence contract");
}

static void test_selected_champion_cell_click_does_not_enable_inventory(void)
{
    Dm1V1MirrorCandidateClickCancelStatePc34Compat state;
    Dm1V1MirrorCandidateClickCancelResultPc34Compat result;
    int selected;
    int changed;

    dm1_v1_mirror_candidate_click_cancel_init_pc34(&state);
    selected = dm1_v1_mirror_candidate_click_cancel_select_champion_pc34(
        &state, 1u);

    CHECK_REDMCSB(selected == 1 && state.selectedChampionOrdinal == 1u,
                  "fixture can select the existing champion while C040 is off",
                  "COMMAND.C 2159-2181");
    CHECK_REDMCSB(state.c040PanelOpen == 0 &&
                      state.g0299CandidateChampionOrdinal == 0u,
                  "selected champion starts outside the C040 candidate panel",
                  "REVIVE.C F0282 lines 744-758");
    CHECK_REDMCSB(state.inventoryPanelOpen == 0 &&
                      state.inventoryChampionOrdinal == 0u,
                  "selected champion does not imply open inventory",
                  "COMMAND.C 2159-2181");

    changed = dm1_v1_mirror_candidate_click_cancel_front_cell_pc34(
        &state, &result);

    CHECK_REDMCSB(changed == 0,
                  "selected champion D1C cell click is still a no-op",
                  "MOVESENS.C 1501-1503");
    CHECK_REDMCSB(result.selectedChampionOrdinalBefore == 1u &&
                      result.selectedChampionOrdinalAfter == 1u,
                  "selected champion identity is preserved",
                  "CHAMPION.C F0302 lines 662-706");
    CHECK_REDMCSB(result.inventoryStayedClosed == 1 &&
                      state.inventoryPanelOpen == 0,
                  "selected champion cell click does not silently enable inventory",
                  "COMMAND.C 2159-2181");
    CHECK_REDMCSB(result.candidateIdentityStayedNone == 1 &&
                      state.candidateChampionOrdinal == 0u,
                  "selected champion cell click does not synthesize a candidate",
                  "REVIVE.C F0280 lines 272-276");
    CHECK_REDMCSB(result.c040PanelStayedClosed == 1 &&
                      state.c040PanelOpen == 0,
                  "selected champion cell click does not open C040",
                  "REVIVE.C F0282 lines 744-758");
    CHECK_REDMCSB(result.mirrorRouteStayedArmed == 1 &&
                      state.frontD1cMirrorChampionOrdinal == 4,
                  "selected champion cell click preserves the mirror route",
                  "DUNVIEW.C 3913-3928");
    CHECK_REDMCSB(result.leaderHandUnchanged == 1,
                  "selected champion cell click does not enter leader-hand routes",
                  "CHAMPION.C F0297/F0298/F0302");
}

static void test_spec_and_source_evidence(void)
{
    const Dm1V1MirrorCandidateClickCancelSpecPc34Compat *spec =
        dm1_v1_mirror_candidate_click_cancel_spec_pc34();
    const char *evidence =
        dm1_v1_mirror_candidate_click_cancel_source_evidence_pc34();

    CHECK_REDMCSB(spec == &DM1_V1_MirrorCandidateClickCancelSpecPc34Compat,
                  "spec accessor returns the const click-cancel spec",
                  "contract marker");
    CHECK_REDMCSB(spec->dungeonViewCommand == 80 &&
                      spec->championPortraitSensor == 127,
                  "spec records C080 dungeon-view and C127 portrait anchors",
                  "MOVESENS.C 1501-1503");
    CHECK_REDMCSB(spec->c040PanelGraphic == 40 &&
                      spec->c040PanelContent == 568,
                  "spec records C040 and M568 panel anchors",
                  "REVIVE.C F0282");
    CHECK_REDMCSB(spec->d1cViewWall == 587,
                  "spec records the D1C front wall anchor",
                  "DUNVIEW.C M587_VIEW_WALL_D1C_FRONT");
    CHECK_REDMCSB(spec->contractMarker != NULL &&
                      strstr(spec->contractMarker, "no-op") != NULL,
                  "spec carries the no-op contract marker",
                  "contract marker");
    CHECK_REDMCSB(evidence == spec->sourceEvidence,
                  "source evidence accessor matches the spec evidence",
                  "source evidence");
    CHECK_REDMCSB(strstr(evidence, "REVIVE.C F0280") != NULL &&
                      strstr(evidence, "REVIVE.C F0282") != NULL,
                  "evidence cites REVIVE.C append and clear paths",
                  "REVIVE.C F0280/F0282");
    CHECK_REDMCSB(strstr(evidence, "2159-2181") != NULL,
                  "evidence cites COMMAND.C status/inventory gating",
                  "COMMAND.C 2159-2181");
    CHECK_REDMCSB(strstr(evidence, "2302-2311") != NULL,
                  "evidence cites COMMAND.C spell/action gating",
                  "COMMAND.C 2302-2311");
    CHECK_REDMCSB(strstr(evidence, "DUNVIEW.C") != NULL &&
                      strstr(evidence, "3913-3928") != NULL &&
                      strstr(evidence, "8488-8533") != NULL,
                  "evidence cites DUNVIEW.C D1C draw order and portrait blit",
                  "DUNVIEW.C 8488-8533; DUNVIEW.C 3913-3928");
    CHECK_REDMCSB(strstr(evidence, "CHAMPION.C F0297") != NULL &&
                      strstr(evidence, "F0298") != NULL &&
                      strstr(evidence, "F0302") != NULL,
                  "evidence cites CHAMPION.C leader-hand routes",
                  "CHAMPION.C F0297/F0298/F0302");
}

int main(void)
{
    test_no_candidate_front_cell_click_is_noop();
    test_repeat_front_cell_click_remains_noop();
    test_selected_champion_cell_click_does_not_enable_inventory();
    test_spec_and_source_evidence();

    printf("PASS dm1_v1_mirror_candidate_click_cancel_pc34_compat %d/%d assertions\n",
           gPasses, gTests);
    return gPasses == gTests ? 0 : 1;
}
