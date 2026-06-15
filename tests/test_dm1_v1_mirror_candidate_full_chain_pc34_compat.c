/* DM1 V1 mirror candidate full-chain regression gate.
 *
 * Source-lock anchors:
 * - ReDMCSB CHAMDRAW.C F0293_CHAMPION_DrawAllChampionStates:1117-1143
 *   redraws every party champion state at the candidate icon click boundary.
 * - ReDMCSB CHAMPION.C F0284_CHAMPION_SetPartyDirection:93-131 rotates
 *   every champion Cell/Direction when the party direction changes.
 * - ReDMCSB CHAMPION.C F0297_CHAMPION_PutObjectInLeaderHand:243-268 fills
 *   the leader hand, refreshes object pointer/name, and redraws the leader.
 * - ReDMCSB COMMAND.C F0359:1985-1990 gates M568/C040 panel input on the
 *   empty leader hand; REVIVE.C F0282:744-806 clears the pending candidate.
 */
#include "dm1_v1_mirror_candidate_full_chain_pc34_compat.h"

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

static void test_source_lock_and_non_overlap_metadata(void)
{
    const Dm1V1MirrorCandidateFullChainEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateFullChain_EvidencePc34Compat();

    CHECK_REDMCSB(e != NULL,
                  "source-lock evidence is available",
                  "metadata");
    CHECK_REDMCSB(strstr(e->chamdrawAllStatesAnchor, "1117-1143") != NULL &&
                      strstr(e->chamdrawAllStatesAnchor, "F0293") != NULL,
                  "F0293 all-state draw range is cited",
                  e->chamdrawAllStatesAnchor);
    CHECK_REDMCSB(strstr(e->championPartyDirectionAnchor, "93-131") != NULL &&
                      strstr(e->championPartyDirectionAnchor, "F0284") != NULL,
                  "F0284 party-direction range is cited",
                  e->championPartyDirectionAnchor);
    CHECK_REDMCSB(strstr(e->championLeaderHandAnchor, "243-268") != NULL &&
                      strstr(e->championLeaderHandAnchor, "F0297") != NULL,
                  "F0297 leader-hand range is cited",
                  e->championLeaderHandAnchor);
    CHECK_REDMCSB(strstr(e->commandPanelAnchor, "1985-1990") != NULL &&
                      strstr(e->commandPanelAnchor, "M568") != NULL,
                  "M568/C040 empty-hand gate is cited",
                  e->commandPanelAnchor);
    CHECK_REDMCSB(strstr(e->revivePanelAnchor, "744-806") != NULL,
                  "REVIVE.C panel clear range is cited",
                  e->revivePanelAnchor);
    CHECK_REDMCSB(strstr(e->nonOverlapNote, "open -> candidate icon click") !=
                      NULL &&
                      strstr(e->nonOverlapNote, "occupied-hand-only") != NULL,
                  "metadata records non-overlap with isolated gates",
                  e->nonOverlapNote);
    CHECK_REDMCSB(strstr(e->contractScope, "contract-only") != NULL,
                  "contract scope is explicit",
                  e->contractScope);
}

static void test_fixture_sets_party_and_fresh_rotated_candidate(void)
{
    Dm1V1MirrorCandidateFullChainStatePc34Compat state;

    DM1_V1_MirrorCandidateFullChain_InitPc34Compat(&state);

    CHECK_REDMCSB(state.partyChampionCount == 4,
                  "fixture contains a four-entry party/candidate row set",
                  "REVIVE.C F0280:272-276");
    CHECK_REDMCSB(state.leaderIndex == 0 &&
                      state.champions[0].ordinal == 1u,
                  "leader champion starts in row zero",
                  "CHAMPION.C F0297:263-267");
    CHECK_REDMCSB(state.nonLeaderChampionCount == 1 &&
                      state.champions[1].ordinal == 2u,
                  "fixture includes a non-leader champion",
                  "CHAMPION.C F0284:123-129");
    CHECK_REDMCSB(state.candidateChampionOrdinal == 3u &&
                      state.activeCandidateRowIndex == 2,
                  "first mirror candidate is pending before open",
                  "REVIVE.C F0280:272-276");
    CHECK_REDMCSB(state.champions[3].ordinal == 4u &&
                      state.champions[3].wounded == 1 &&
                      state.champions[3].poisoned == 1,
                  "rotated candidate is wounded and poisoned for F0293 draw",
                  "CHAMDRAW.C F0293:1117-1143");
    CHECK_REDMCSB(state.leaderHandEmpty == 1 &&
                      state.leaderHandThing ==
                          DM1_V1_MIRROR_CANDIDATE_FULL_CHAIN_THING_NONE_PC34_COMPAT,
                  "leader hand starts empty for the normal pickup chain",
                  "COMMAND.C F0359:1985-1990");
}

static void test_full_chain_picks_rotated_candidate_into_leader_hand(void)
{
    Dm1V1MirrorCandidateFullChainStatePc34Compat state;
    Dm1V1MirrorCandidateFullChainResultPc34Compat result;
    int ok;

    DM1_V1_MirrorCandidateFullChain_InitPc34Compat(&state);
    ok = DM1_V1_MirrorCandidateFullChain_RunPc34Compat(&state, &result);

    CHECK_REDMCSB(ok == 1 && result.sourceLockedFullChain == 1,
                  "open/click/rotate/click/pickup chain returns success",
                  result.evidence->contractScope);
    CHECK_REDMCSB(result.openedPanel == 1 &&
                      result.panelOpenAfterOpen == 1 &&
                      result.candidateOrdinalAfterOpen == 3,
                  "open step publishes the M568/C040 candidate panel",
                  result.evidence->revivePanelAnchor);
    CHECK_REDMCSB(result.firstIconClicked == 1 &&
                      result.panelOpenAfterFirstClick == 1 &&
                      result.candidateOrdinalAfterFirstClick == 3,
                  "first candidate icon click keeps panel ownership",
                  result.evidence->chamdrawAllStatesAnchor);
    CHECK_REDMCSB(result.allStateDrawCountAfterFirstClick == 1,
                  "first icon click performs one F0293-style all-state draw",
                  result.evidence->chamdrawAllStatesAnchor);
    CHECK_REDMCSB(result.rotatedCandidate == 1 &&
                      result.activeCandidateBeforeRotation == 3 &&
                      result.activeCandidateAfterRotation == 4,
                  "rotation selects a fresh candidate identity",
                  result.evidence->championPartyDirectionAnchor);
    CHECK_REDMCSB(result.partyDirectionBeforeRotation == 0 &&
                      result.partyDirectionAfterRotation == 1,
                  "F0284-style party direction advances once",
                  result.evidence->championPartyDirectionAnchor);
    CHECK_REDMCSB(result.rotatedCandidateDirectionBefore == 0 &&
                      result.rotatedCandidateDirectionAfter == 1,
                  "rotated candidate direction follows the party delta",
                  result.evidence->championPartyDirectionAnchor);
    CHECK_REDMCSB(result.rotatedIconClicked == 1 &&
                      result.panelOpenAfterRotatedClick == 1 &&
                      result.candidateOrdinalAfterRotatedClick == 4,
                  "rotated candidate icon click keeps C040 live",
                  result.evidence->chamdrawAllStatesAnchor);
    CHECK_REDMCSB(result.allStateDrawCountAfterRotatedClick == 2 &&
                      result.woundedPoisonedDrawCountAfterRotatedClick >= 2,
                  "rotated wounded/poisoned candidate is included in all-state draw",
                  result.evidence->chamdrawAllStatesAnchor);
    CHECK_REDMCSB(result.pickupAttempted == 1 &&
                      result.pickupSucceeded == 1 &&
                      result.pickupRejectedOccupiedHand == 0,
                  "pickup step enters the F0297 leader-hand path",
                  result.evidence->championLeaderHandAnchor);
    CHECK_REDMCSB(result.leaderHandEmptyBeforePickup == 1 &&
                      result.leaderHandEmptyAfterPickup == 0,
                  "leader hand changes from empty to occupied",
                  result.evidence->championLeaderHandAnchor);
    CHECK_REDMCSB(result.leaderHandThingAfterPickup == state.champions[3].handThing,
                  "rotated candidate object lands in leader hand",
                  result.evidence->championLeaderHandAnchor);
    CHECK_REDMCSB(result.candidateOrdinalAfterPickup == 0 &&
                      result.panelOpenAfterPickup == 0 &&
                      result.candidateClearCountAfterPickup == 1,
                  "pickup clears the pending candidate and closes C040",
                  result.evidence->revivePanelAnchor);
    CHECK_REDMCSB(result.nonLeaderChampionPresent == 1,
                  "chain retained the non-leader champion fixture",
                  result.evidence->championPartyDirectionAnchor);
}

static void test_full_chain_occupied_hand_rejects_pickup_after_rotated_click(void)
{
    Dm1V1MirrorCandidateFullChainStatePc34Compat state;
    Dm1V1MirrorCandidateFullChainResultPc34Compat result;
    int ok;

    DM1_V1_MirrorCandidateFullChain_InitPc34Compat(&state);
    DM1_V1_MirrorCandidateFullChain_SetLeaderHandFullBeforePickupPc34Compat(
        &state, 1);
    ok = DM1_V1_MirrorCandidateFullChain_RunPc34Compat(&state, &result);

    CHECK_REDMCSB(ok == 1 && result.sourceLockedFullChain == 1,
                  "occupied-hand variant still runs the full chain to pickup",
                  result.evidence->contractScope);
    CHECK_REDMCSB(result.openedPanel == 1 &&
                      result.firstIconClicked == 1 &&
                      result.rotatedCandidate == 1 &&
                      result.rotatedIconClicked == 1,
                  "occupied-hand case reaches the rotated candidate click",
                  result.evidence->nonOverlapNote);
    CHECK_REDMCSB(result.leaderHandEmptyBeforePickup == 0 &&
                      result.leaderHandThingBeforePickup == 0x0BEEu,
                  "leader hand is full going into the pickup step",
                  result.evidence->commandPanelAnchor);
    CHECK_REDMCSB(result.pickupAttempted == 1 &&
                      result.pickupRejectedOccupiedHand == 1 &&
                      result.pickupSucceeded == 0,
                  "occupied hand blocks the F0297 pickup",
                  result.evidence->commandPanelAnchor);
    CHECK_REDMCSB(result.leaderHandThingAfterPickup == 0x0BEEu &&
                      result.leaderHandEmptyAfterPickup == 0,
                  "occupied-hand rejection preserves the held object",
                  result.evidence->commandPanelAnchor);
    CHECK_REDMCSB(result.candidateOrdinalAfterPickup == 4 &&
                      result.panelOpenAfterPickup == 1 &&
                      result.candidateClearCountAfterPickup == 0,
                  "occupied-hand rejection leaves C040 and G0299 pending",
                  result.evidence->revivePanelAnchor);
    CHECK_REDMCSB(result.leaderHandPutCountAfterPickup == 0 &&
                      result.occupiedHandRejectCountAfterPickup == 1,
                  "occupied-hand path records no F0297 put",
                  result.evidence->championLeaderHandAnchor);
}

int main(void)
{
    test_source_lock_and_non_overlap_metadata();
    test_fixture_sets_party_and_fresh_rotated_candidate();
    test_full_chain_picks_rotated_candidate_into_leader_hand();
    test_full_chain_occupied_hand_rejects_pickup_after_rotated_click();

    printf("PASS dm1_v1_mirror_candidate_full_chain_pc34_compat "
           "%d/%d assertions\n",
           gPasses, gTests);
    return gPasses == gTests ? 0 : 1;
}
