#include "dm1_v1_mirror_candidate_close_button_pc34_compat.h"

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

static void test_close_button_clears_panel_without_resurrecting(void)
{
    Dm1V1MirrorCandidateCloseButtonStatePc34Compat state;
    Dm1V1MirrorCandidateCloseButtonResultPc34Compat result;
    int changed;
    int party0OrdinalBefore;
    int party1OrdinalBefore;
    int party0HealthBefore;
    int party1HealthBefore;
    int leaderIndexBefore;
    int frontOrdinalBefore;
    unsigned int candidateIdentityBefore;
    unsigned int preInventoryOrdinal;

    dm1_v1_mirror_candidate_close_button_init_pc34(&state);
    party0OrdinalBefore = (int)state.party[0].championOrdinal;
    party1OrdinalBefore = (int)state.party[1].championOrdinal;
    party0HealthBefore = state.party[0].currentHealth;
    party1HealthBefore = state.party[1].currentHealth;
    leaderIndexBefore = state.leaderIndex;
    frontOrdinalBefore = state.frontD1cMirrorChampionOrdinal;
    candidateIdentityBefore = state.candidateChampionOrdinal;
    preInventoryOrdinal = state.preC040InventoryChampionOrdinal;

    CHECK_REDMCSB(state.active == 1,
                  "fixture starts in an active runtime mirror route",
                  "DUNVIEW.C 3913-3928; M11_GameView_SelectFrontMirrorCandidate");
    CHECK_REDMCSB(state.c040PanelOpen == 1 &&
                      state.c040PanelPixelsDrawn == 1,
                  "C040 resurrect/reincarnate panel starts open",
                  "PANEL.C F0346 lines 1619-1635; PANEL.C F0347 lines 1654-1656");
    CHECK_REDMCSB(state.panelContent ==
                      DM1_V1_MIRROR_CANDIDATE_CLOSE_BUTTON_SOURCE_PANEL_CONTENT_PC34_COMPAT,
                  "M568 panel content owns the C040 panel",
                  "COMMAND.C F0378 lines 1985-1991");
    CHECK_REDMCSB(state.g0299CandidateChampionOrdinal != 0u,
                  "G0299 is live before close",
                  "REVIVE.C F0282 lines 744-758");
    CHECK_REDMCSB(state.g0299CandidateChampionOrdinal == 3u,
                  "G0299 points at the appended candidate party ordinal",
                  "REVIVE.C F0280 lines 272-276");
    CHECK_REDMCSB(state.candidateChampionOrdinal == 4u,
                  "candidate source identity is selected before close",
                  "DUNVIEW.C 3913-3928");
    CHECK_REDMCSB(state.inventoryChampionOrdinal ==
                      state.g0299CandidateChampionOrdinal,
                  "C040 open state has the candidate in the inventory panel",
                  "REVIVE.C F0280 lines 272-276; PANEL.C F0347 lines 1654-1656");
    CHECK_REDMCSB(state.inventoryChampionOrdinal !=
                      state.preC040InventoryChampionOrdinal,
                  "pre-C040 inventory owner differs from the pending candidate",
                  "REVIVE.C F0282 lines 744-758");
    CHECK_REDMCSB(state.frontD1cMirrorChampionOrdinal != 0,
                  "front D1C mirror route is armed before close",
                  "DUNVIEW.C 3913-3928");
    CHECK_REDMCSB(state.frontD1cMirrorChampionOrdinal == 4,
                  "front mirror ordinal matches the selected mirror identity",
                  "DUNVIEW.C 3913-3928");
    CHECK_REDMCSB(state.partyChampionCount == 3 &&
                      state.preC040PartyChampionCount == 2,
                  "candidate is appended while the C040 panel is open",
                  "REVIVE.C F0280 lines 272-276");
    CHECK_REDMCSB(state.party[2].present == 1 &&
                      state.party[2].currentHealth == 0,
                  "pending candidate slot is present but not resurrected",
                  "REVIVE.C F0282 lines 785-806");
    CHECK_REDMCSB(state.leaderHandChampionOrdinal !=
                      state.candidateChampionOrdinal,
                  "candidate is not in the leader hand before close",
                  "COMMAND.C 2302-2311");

    changed = dm1_v1_mirror_candidate_close_button_pc34(
        &state,
        DM1_V1_MIRROR_CANDIDATE_CLOSE_BUTTON_COMMAND_PC34_COMPAT,
        &result);

    CHECK_REDMCSB(changed == 1,
                  "C040 close-button command is consumed",
                  "COMMAND.C F0378 lines 1985-1991; panel close-button anchor");
    CHECK_REDMCSB(result.command ==
                      DM1_V1_MIRROR_CANDIDATE_CLOSE_BUTTON_COMMAND_PC34_COMPAT,
                  "result records the explicit C040 close-button command",
                  "panel close-button anchor");
    CHECK_REDMCSB(result.validCloseButtonCommand == 1 &&
                      result.ignoredNotCloseButton == 0,
                  "only the C040 close-button route ran",
                  "COMMAND.C F0378 lines 1985-1991");
    CHECK_REDMCSB(result.resurrectCommandReached == 0 &&
                      result.reincarnateCommandReached == 0,
                  "close button does not run C160 resurrect or C161 reincarnate",
                  "REVIVE.C F0282 lines 785-806");
    CHECK_REDMCSB(state.g0299CandidateChampionOrdinal == 0u &&
                      result.g0299Before == 3u &&
                      result.g0299After == 0u,
                  "C040 close clears G0299 and closes the panel gate",
                  "REVIVE.C F0282 lines 744-758; COMMAND.C 2302-2311");
    CHECK_REDMCSB(state.c040PanelOpen == 0 &&
                      result.closedPanel == 1,
                  "C040 panel is closed after the close-button command",
                  "PANEL.C F0347 lines 1654-1656");
    CHECK_REDMCSB(result.c040PanelCleared == 1 &&
                      state.c040PanelPixelsDrawn == 0,
                  "C040 panel pixels are cleared",
                  "PANEL.C F0346 lines 1619-1635");
    CHECK_REDMCSB(result.c040PanelPixelsBefore == 1 &&
                      result.c040PanelPixelsAfter == 0,
                  "pixel marker transitions from drawn to cleared",
                  "PANEL.C F0346 lines 1619-1635");
    CHECK_REDMCSB(result.candidateChampionOrdinalBefore ==
                      (int)candidateIdentityBefore &&
                      result.candidateChampionOrdinalAfter ==
                          (int)candidateIdentityBefore,
                  "candidate identity is preserved across close",
                  "REVIVE.C F0282 lines 744-758");
    CHECK_REDMCSB(result.candidateIdentityPreserved == 1,
                  "candidate identity preservation flag is set",
                  "REVIVE.C F0282 lines 744-758");
    CHECK_REDMCSB(state.candidateChampionOrdinal == candidateIdentityBefore,
                  "state keeps the selected mirror identity after close",
                  "DUNVIEW.C 3913-3928");
    CHECK_REDMCSB(result.candidatePromotedToLeaderHand == 0 &&
                      state.leaderHandChampionOrdinal !=
                          state.candidateChampionOrdinal,
                  "close button does not promote candidate to leader hand",
                  "REVIVE.C F0282 lines 785-806");
    CHECK_REDMCSB(result.leaderHandChampionOrdinalBefore ==
                      result.leaderHandChampionOrdinalAfter,
                  "leader hand occupant is unchanged by close",
                  "COMMAND.C 2302-2311");
    CHECK_REDMCSB(state.inventoryChampionOrdinal == preInventoryOrdinal &&
                      result.inventoryChampionOrdinalAfter ==
                          result.inventoryChampionOrdinalPreC040,
                  "inventory champion reverts to the pre-C040 owner",
                  "REVIVE.C F0282 lines 744-758");
    CHECK_REDMCSB(result.inventoryChampionOrdinalBefore == 3u &&
                      result.inventoryChampionOrdinalAfter == 1u,
                  "candidate inventory panel owner is canceled, not promoted",
                  "REVIVE.C F0282 lines 744-758");
    CHECK_REDMCSB(state.partyChampionCount == state.preC040PartyChampionCount &&
                      result.partyChampionCountBefore == 3 &&
                      result.partyChampionCountAfter == 2,
                  "close removes the appended candidate party slot",
                  "REVIVE.C F0282 lines 744-758");
    CHECK_REDMCSB(result.candidateSlotPresentBefore == 1 &&
                      result.candidateSlotPresentAfter == 0,
                  "candidate slot is cleared on the close-button cancel path",
                  "REVIVE.C F0282 lines 744-758");
    CHECK_REDMCSB(state.party[2].present == 0 &&
                      state.party[2].currentHealth == 0,
                  "candidate is not resurrected into the party",
                  "REVIVE.C F0282 lines 785-806");
    CHECK_REDMCSB(state.party[0].championOrdinal ==
                      (unsigned int)party0OrdinalBefore &&
                      result.party0OrdinalAfter == party0OrdinalBefore,
                  "party[0] champion ordinal is unchanged",
                  "REVIVE.C F0282 lines 744-758");
    CHECK_REDMCSB(state.party[1].championOrdinal ==
                      (unsigned int)party1OrdinalBefore &&
                      result.party1OrdinalAfter == party1OrdinalBefore,
                  "party[1] champion ordinal is unchanged",
                  "REVIVE.C F0282 lines 744-758");
    CHECK_REDMCSB(state.party[0].currentHealth == party0HealthBefore &&
                      result.party0HealthAfter == party0HealthBefore,
                  "party[0] health is unchanged; no resurrection happened",
                  "REVIVE.C F0282 lines 785-806");
    CHECK_REDMCSB(state.party[1].currentHealth == party1HealthBefore &&
                      result.party1HealthAfter == party1HealthBefore,
                  "party[1] health is unchanged; no resurrection happened",
                  "REVIVE.C F0282 lines 785-806");
    CHECK_REDMCSB(result.party0OrdinalBefore == result.party0OrdinalAfter &&
                      result.party1OrdinalBefore == result.party1OrdinalAfter,
                  "result records stable party[0]/party[1] ordinals",
                  "REVIVE.C F0282 lines 744-758");
    CHECK_REDMCSB(result.party0HealthBefore == result.party0HealthAfter &&
                      result.party1HealthBefore == result.party1HealthAfter,
                  "result records stable party[0]/party[1] health",
                  "REVIVE.C F0282 lines 785-806");
    CHECK_REDMCSB(state.leaderIndex == leaderIndexBefore &&
                      result.leaderIndexBefore == result.leaderIndexAfter,
                  "leader index is unchanged by panel close",
                  "COMMAND.C 2302-2311");
    CHECK_REDMCSB(state.frontD1cMirrorChampionOrdinal == frontOrdinalBefore &&
                      result.newFrontD1cMirrorChampionOrdinal ==
                          result.previousFrontD1cMirrorChampionOrdinal,
                  "close preserves the mirror route ordinal from open state",
                  "DUNVIEW.C 3913-3928");
    CHECK_REDMCSB(result.mirrorRoutePreservedFromOpen == 1,
                  "mirror route is preserved rather than newly rearmed",
                  "DUNVIEW.C 3913-3928");
    CHECK_REDMCSB(result.mirrorRouteRearmedByResurrect == 0,
                  "close path is distinguished from resurrect rearm",
                  "REVIVE.C F0282 lines 785-806");
    CHECK_REDMCSB(result.frontD1cPortraitIndex ==
                      DM1_V1_MIRROR_CANDIDATE_CLOSE_BUTTON_NONE_PC34_COMPAT,
                  "out-of-party mirror identity remains route state, not a party portrait",
                  "DUNVIEW.C 3913-3928");
    CHECK_REDMCSB(result.actionAreaGateOpenAfterClose == 1,
                  "COMMAND.C action/spell gate reopens after G0299 clears",
                  "COMMAND.C 2302-2311");
}

static void test_non_close_button_is_ignored(void)
{
    Dm1V1MirrorCandidateCloseButtonStatePc34Compat state;
    Dm1V1MirrorCandidateCloseButtonResultPc34Compat result;
    int changed;

    dm1_v1_mirror_candidate_close_button_init_pc34(&state);
    changed = dm1_v1_mirror_candidate_close_button_pc34(
        &state,
        DM1_V1_MIRROR_CANDIDATE_CLOSE_BUTTON_SOURCE_CANCEL_COMMAND_PC34_COMPAT,
        &result);

    CHECK_REDMCSB(changed == 0 && result.ignoredNotCloseButton == 1,
                  "C162 source cancel is not the explicit close-button command",
                  "COMMAND.C G0457 lines 508-511; panel close-button anchor");
    CHECK_REDMCSB(state.c040PanelOpen == 1 &&
                      state.g0299CandidateChampionOrdinal != 0u,
                  "non-close command leaves C040 open in this gate",
                  "COMMAND.C F0378 lines 1985-1991");
}

static void test_spec_and_source_evidence(void)
{
    const Dm1V1MirrorCandidateCloseButtonSpecPc34Compat *spec =
        dm1_v1_mirror_candidate_close_button_spec_pc34();
    const char *evidence =
        dm1_v1_mirror_candidate_close_button_source_evidence_pc34();

    CHECK_REDMCSB(spec == &DM1_V1_MirrorCandidateCloseButtonSpecPc34Compat,
                  "spec accessor returns the const probe spec",
                  "contract-only marker");
    CHECK_REDMCSB(spec->c040PanelGraphic == 40 &&
                      spec->c040PanelContent == 568,
                  "spec records C040 graphic and M568 panel",
                  "PANEL.C F0346 lines 1619-1635; COMMAND.C F0378 lines 1985-1991");
    CHECK_REDMCSB(spec->closeButtonCommand == 0x4000 &&
                      spec->sourceCancelCommand == 162,
                  "spec records close-button and source cancel commands",
                  "COMMAND.C G0457 lines 508-511; panel close-button anchor");
    CHECK_REDMCSB(spec->contractMarker != NULL &&
                      strstr(spec->contractMarker, "contract-only") != NULL,
                  "spec carries the contract-only marker",
                  "panel close-button anchor");
    CHECK_REDMCSB(evidence == spec->sourceEvidence,
                  "source evidence accessor matches the spec evidence",
                  "source evidence");
    CHECK_REDMCSB(strstr(evidence, "COMMAND.C") != NULL &&
                      strstr(evidence, "2302-2311") != NULL,
                  "evidence cites COMMAND.C 2302-2311",
                  "COMMAND.C 2302-2311");
    CHECK_REDMCSB(strstr(evidence, "REVIVE.C F0282") != NULL &&
                      strstr(evidence, "785-806") != NULL,
                  "evidence cites REVIVE.C F0282 785-806",
                  "REVIVE.C F0282 lines 785-806");
    CHECK_REDMCSB(strstr(evidence, "DUNVIEW.C") != NULL &&
                      strstr(evidence, "3913-3928") != NULL,
                  "evidence cites DUNVIEW.C 3913-3928",
                  "DUNVIEW.C 3913-3928");
    CHECK_REDMCSB(strstr(evidence, "PANEL.C F0346") != NULL &&
                      strstr(evidence, "PANEL.C F0347") != NULL,
                  "evidence cites the panel C040 close-button anchor",
                  "PANEL.C F0346/F0347");
    CHECK_REDMCSB(strstr(evidence, "M11_GameView_SelectFrontMirrorCandidate") != NULL,
                  "evidence names the M11 mirror candidate route",
                  "M11_GameView_SelectFrontMirrorCandidate");
}

int main(void)
{
    test_close_button_clears_panel_without_resurrecting();
    test_non_close_button_is_ignored();
    test_spec_and_source_evidence();

    printf("PASS dm1_v1_mirror_candidate_close_button_pc34_compat %d/%d assertions\n",
           gPasses, gTests);
    return gPasses == gTests ? 0 : 1;
}
