#include "dm1_v1_mirror_candidate_party_swap_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/* ReDMCSB source-lock anchors:
 * CHAMPION.C:243-340 F0297_CHAMPION_PutObjectInLeaderHand.
 * CHAMPION.C:587-700 F0301_CHAMPION_AddObjectInSlot and
 * F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox.
 * CHAMPION.C:2496-2499 F0293_CHAMPION_DrawAllChampionStates.
 * DEFS.H:780-807 slot constants including CM1/C00/C01/C28/C29.
 * CHAMDRAW.C:540-700 mirror-candidate slot rendering.
 */

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

static void check_party(
    const int party[DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_MAX_PARTY_PC34_COMPAT],
    int a,
    int b,
    int c,
    int d,
    const char *anchor)
{
    CHECK_REDMCSB(party[0] == a, "party index 0 matches expected order",
                  anchor);
    CHECK_REDMCSB(party[1] == b, "party index 1 matches expected order",
                  anchor);
    CHECK_REDMCSB(party[2] == c, "party index 2 matches expected order",
                  anchor);
    CHECK_REDMCSB(party[3] == d, "party index 3 matches expected order",
                  anchor);
}

static void test_evidence_and_fixture(void)
{
    Dm1V1MirrorCandidatePartySwapStatePc34Compat state;
    const Dm1V1MirrorCandidatePartySwapEvidencePc34Compat *e =
        DM1_V1_MirrorCandidatePartySwap_EvidencePc34Compat();

    DM1_V1_MirrorCandidatePartySwap_InitPc34Compat(&state, 1);

    CHECK_REDMCSB(e != NULL, "evidence metadata is available",
                  e->defsAnchor);
    CHECK_REDMCSB(strstr(e->leaderHandAnchor, "CHAMPION.C:243-340") != NULL,
                  "leader-hand evidence cites F0297",
                  e->leaderHandAnchor);
    CHECK_REDMCSB(strstr(e->slotClickAnchor, "CHAMPION.C:587-700") != NULL,
                  "slot-click evidence cites F0301/F0302",
                  e->slotClickAnchor);
    CHECK_REDMCSB(strstr(e->drawAllAnchor, "CHAMPION.C:2496-2499") != NULL,
                  "draw-all evidence cites F0293",
                  e->drawAllAnchor);
    CHECK_REDMCSB(strstr(e->defsAnchor, "C01_SLOT_ACTION_HAND") != NULL &&
                      strstr(e->defsAnchor, "C28_PARTY_FIRST") != NULL &&
                      strstr(e->defsAnchor, "C29_PARTY_LAST") != NULL,
                  "defs evidence cites requested slot/range names",
                  e->defsAnchor);
    CHECK_REDMCSB(strstr(e->chamdrawAnchor, "CHAMDRAW.C:540-700") != NULL,
                  "rendering evidence cites CHAMDRAW mirror slots",
                  e->chamdrawAnchor);
    CHECK_REDMCSB(strstr(e->partyAnchor, "REVIVE.C:272-276") != NULL,
                  "party evidence cites G0299/G0305 publication",
                  e->partyAnchor);
    CHECK_REDMCSB(strstr(e->mirrorClosedAnchor, "COMMAND.C:2158-2182") != NULL,
                  "closed/gated evidence cites command guard",
                  e->mirrorClosedAnchor);
    CHECK_REDMCSB(strstr(e->nonOverlapNote, "non-overlap") != NULL,
                  "non-overlap evidence is explicit",
                  e->nonOverlapNote);
    CHECK_REDMCSB(state.mirrorPanelOpen == 1,
                  "fixture starts with mirror panel open",
                  e->chamdrawAnchor);
    CHECK_REDMCSB(state.g0299_ui_CandidateChampionOrdinal == 4,
                  "fixture starts with G0299 candidate owner",
                  e->partyAnchor);
    CHECK_REDMCSB(state.g0423_i_InventoryChampionOrdinal == 4,
                  "fixture starts with matching inventory ordinal",
                  e->partyAnchor);
    CHECK_REDMCSB(state.g0411_i_LeaderIndex == 0,
                  "fixture pins leader to index 0",
                  e->defsAnchor);
    CHECK_REDMCSB(state.g0305_ui_PartyChampionCount == 3,
                  "fixture counts only non-empty party slots",
                  e->partyAnchor);
    check_party(state.g0227_aT_Party,
                DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_LEADER_ID_PC34_COMPAT,
                DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_A_ID_PC34_COMPAT,
                DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_EMPTY_PC34_COMPAT,
                DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_B_ID_PC34_COMPAT,
                e->partyAnchor);
}

static void test_open_non_adjacent_swap_refreshes_once(void)
{
    Dm1V1MirrorCandidatePartySwapStatePc34Compat state;
    Dm1V1MirrorCandidatePartySwapResultPc34Compat result;
    const Dm1V1MirrorCandidatePartySwapEvidencePc34Compat *e =
        DM1_V1_MirrorCandidatePartySwap_EvidencePc34Compat();
    int returned;

    DM1_V1_MirrorCandidatePartySwap_InitPc34Compat(&state, 1);
    returned = DM1_V1_MirrorCandidatePartySwap_RunPc34Compat(
        &state, 1, 3, &result);

    CHECK_REDMCSB(returned == 1, "open non-adjacent swap is accepted",
                  e->slotClickAnchor);
    CHECK_REDMCSB(result.accepted == 1, "result records accepted swap",
                  e->slotClickAnchor);
    CHECK_REDMCSB(result.rejectedLeaderSwap == 0,
                  "accepted swap is not rejected as leader move",
                  e->defsAnchor);
    CHECK_REDMCSB(result.rejectedEmptySlot == 0,
                  "accepted swap is not rejected as empty-slot move",
                  e->slotClickAnchor);
    CHECK_REDMCSB(result.rejectedMirrorClosed == 0,
                  "accepted swap is not rejected as closed mirror",
                  e->mirrorClosedAnchor);
    CHECK_REDMCSB(result.beforeParty[1] ==
                      DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_A_ID_PC34_COMPAT,
                  "before order has candidate A at index 1",
                  e->partyAnchor);
    CHECK_REDMCSB(result.beforeParty[3] ==
                      DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_B_ID_PC34_COMPAT,
                  "before order has candidate B at index 3",
                  e->partyAnchor);
    CHECK_REDMCSB(result.afterParty[1] ==
                      DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_B_ID_PC34_COMPAT,
                  "after order has candidate B at index 1",
                  e->partyAnchor);
    CHECK_REDMCSB(result.afterParty[3] ==
                      DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_A_ID_PC34_COMPAT,
                  "after order has candidate A at index 3",
                  e->partyAnchor);
    CHECK_REDMCSB(result.partyReordered == 1,
                  "G0227_aT_Party is reordered by A/B swap",
                  e->partyAnchor);
    CHECK_REDMCSB(result.partyOrderUnchanged == 0,
                  "G0227_aT_Party does not remain in old order",
                  e->partyAnchor);
    CHECK_REDMCSB(result.onlyRequestedPairMoved == 1,
                  "only the requested non-adjacent pair moves",
                  e->partyAnchor);
    CHECK_REDMCSB(result.leaderPreserved == 1,
                  "leader remains fixed at index 0",
                  e->defsAnchor);
    CHECK_REDMCSB(result.leaderIdAfter ==
                      DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_LEADER_ID_PC34_COMPAT,
                  "leader id is not lost during swap",
                  e->defsAnchor);
    CHECK_REDMCSB(result.partyCountPreserved == 1,
                  "party count is preserved after swap",
                  e->partyAnchor);
    CHECK_REDMCSB(result.mirrorOwnerPreserved == 1,
                  "G0299/G0423 mirror ownership is preserved",
                  e->chamdrawAnchor);
    CHECK_REDMCSB(result.panelDrawRefreshedOnce == 1,
                  "panel draw refreshes once for accepted swap",
                  e->chamdrawAnchor);
    CHECK_REDMCSB(result.f0293CalledOnce == 1,
                  "F0293 draw-all is called once",
                  e->drawAllAnchor);
    CHECK_REDMCSB(result.f0293NotCalledTwice == 1,
                  "F0293 draw-all is not called twice",
                  e->drawAllAnchor);
    CHECK_REDMCSB(result.g0227PartyContract == 1,
                  "G0227 party contract passes for accepted swap",
                  e->partyAnchor);
    CHECK_REDMCSB(state.panelDrawCount == 1,
                  "state panel draw counter increments once",
                  e->chamdrawAnchor);
    CHECK_REDMCSB(state.f0293DrawAllChampionStatesCount == 1,
                  "state F0293 counter increments once",
                  e->drawAllAnchor);
    CHECK_REDMCSB(state.swapAttemptCount == 1,
                  "state records one swap attempt",
                  e->slotClickAnchor);
    CHECK_REDMCSB(state.swapAcceptedCount == 1,
                  "state records one accepted swap",
                  e->slotClickAnchor);
    check_party(state.g0227_aT_Party,
                DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_LEADER_ID_PC34_COMPAT,
                DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_B_ID_PC34_COMPAT,
                DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_EMPTY_PC34_COMPAT,
                DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_A_ID_PC34_COMPAT,
                e->partyAnchor);
}

static void test_leader_swap_rejected_and_order_unchanged(void)
{
    Dm1V1MirrorCandidatePartySwapStatePc34Compat state;
    Dm1V1MirrorCandidatePartySwapResultPc34Compat result;
    const Dm1V1MirrorCandidatePartySwapEvidencePc34Compat *e =
        DM1_V1_MirrorCandidatePartySwap_EvidencePc34Compat();
    int returned;

    DM1_V1_MirrorCandidatePartySwap_InitPc34Compat(&state, 1);
    returned = DM1_V1_MirrorCandidatePartySwap_RunPc34Compat(
        &state, 0, 1, &result);

    CHECK_REDMCSB(returned == 0, "leader/non-leader swap is rejected",
                  e->defsAnchor);
    CHECK_REDMCSB(result.accepted == 0,
                  "leader swap result is not accepted",
                  e->defsAnchor);
    CHECK_REDMCSB(result.rejectedLeaderSwap == 1,
                  "leader swap records fixed-index rejection",
                  e->defsAnchor);
    CHECK_REDMCSB(result.partyOrderUnchanged == 1,
                  "leader rejection preserves party order",
                  e->partyAnchor);
    CHECK_REDMCSB(result.partyReordered == 0,
                  "leader rejection does not reorder party",
                  e->partyAnchor);
    CHECK_REDMCSB(result.leaderPreserved == 1,
                  "leader stays at index 0 after rejected swap",
                  e->defsAnchor);
    CHECK_REDMCSB(result.partyCountPreserved == 1,
                  "party count is preserved after leader rejection",
                  e->partyAnchor);
    CHECK_REDMCSB(result.mirrorOwnerPreserved == 1,
                  "mirror candidate ownership survives leader rejection",
                  e->chamdrawAnchor);
    CHECK_REDMCSB(result.panelDrawRefreshedOnce == 0,
                  "leader rejection does not refresh panel",
                  e->chamdrawAnchor);
    CHECK_REDMCSB(result.f0293CalledOnce == 0,
                  "leader rejection does not call F0293",
                  e->drawAllAnchor);
    CHECK_REDMCSB(result.f0293NotCalledTwice == 1,
                  "leader rejection still avoids double F0293",
                  e->drawAllAnchor);
    CHECK_REDMCSB(state.rejectedLeaderSwapCount == 1,
                  "state counts one leader rejection",
                  e->defsAnchor);
    check_party(state.g0227_aT_Party,
                DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_LEADER_ID_PC34_COMPAT,
                DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_A_ID_PC34_COMPAT,
                DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_EMPTY_PC34_COMPAT,
                DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_B_ID_PC34_COMPAT,
                e->partyAnchor);
}

static void test_empty_party_slot_swap_rejected(void)
{
    Dm1V1MirrorCandidatePartySwapStatePc34Compat state;
    Dm1V1MirrorCandidatePartySwapResultPc34Compat result;
    const Dm1V1MirrorCandidatePartySwapEvidencePc34Compat *e =
        DM1_V1_MirrorCandidatePartySwap_EvidencePc34Compat();
    int returned;

    DM1_V1_MirrorCandidatePartySwap_InitPc34Compat(&state, 1);
    returned = DM1_V1_MirrorCandidatePartySwap_RunPc34Compat(
        &state, 1, 2, &result);

    CHECK_REDMCSB(returned == 0, "swap with empty party slot is rejected",
                  e->slotClickAnchor);
    CHECK_REDMCSB(result.accepted == 0,
                  "empty-slot result is not accepted",
                  e->slotClickAnchor);
    CHECK_REDMCSB(result.rejectedEmptySlot == 1,
                  "empty-slot rejection is recorded",
                  e->slotClickAnchor);
    CHECK_REDMCSB(result.rejectedLeaderSwap == 0,
                  "empty-slot rejection is not leader rejection",
                  e->defsAnchor);
    CHECK_REDMCSB(result.partyOrderUnchanged == 1,
                  "empty-slot rejection preserves party order",
                  e->partyAnchor);
    CHECK_REDMCSB(result.leaderPreserved == 1,
                  "leader stays fixed after empty-slot rejection",
                  e->defsAnchor);
    CHECK_REDMCSB(result.partyCountPreserved == 1,
                  "party count is preserved after empty-slot rejection",
                  e->partyAnchor);
    CHECK_REDMCSB(result.mirrorOwnerPreserved == 1,
                  "mirror owner survives empty-slot rejection",
                  e->chamdrawAnchor);
    CHECK_REDMCSB(result.panelDrawRefreshedOnce == 0,
                  "empty-slot rejection does not redraw panel",
                  e->chamdrawAnchor);
    CHECK_REDMCSB(result.f0293CalledOnce == 0,
                  "empty-slot rejection does not call F0293",
                  e->drawAllAnchor);
    CHECK_REDMCSB(result.f0293NotCalledTwice == 1,
                  "empty-slot rejection avoids double F0293",
                  e->drawAllAnchor);
    CHECK_REDMCSB(state.rejectedEmptySlotCount == 1,
                  "state counts one empty-slot rejection",
                  e->slotClickAnchor);
    check_party(state.g0227_aT_Party,
                DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_LEADER_ID_PC34_COMPAT,
                DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_A_ID_PC34_COMPAT,
                DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_EMPTY_PC34_COMPAT,
                DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_B_ID_PC34_COMPAT,
                e->partyAnchor);
}

static void test_mirror_closed_swap_rejected(void)
{
    Dm1V1MirrorCandidatePartySwapStatePc34Compat state;
    Dm1V1MirrorCandidatePartySwapResultPc34Compat result;
    const Dm1V1MirrorCandidatePartySwapEvidencePc34Compat *e =
        DM1_V1_MirrorCandidatePartySwap_EvidencePc34Compat();
    int returned;

    DM1_V1_MirrorCandidatePartySwap_InitPc34Compat(&state, 0);
    returned = DM1_V1_MirrorCandidatePartySwap_RunPc34Compat(
        &state, 1, 3, &result);

    CHECK_REDMCSB(returned == 0, "closed mirror swap is rejected",
                  e->mirrorClosedAnchor);
    CHECK_REDMCSB(result.accepted == 0,
                  "closed mirror result is not accepted",
                  e->mirrorClosedAnchor);
    CHECK_REDMCSB(result.rejectedMirrorClosed == 1,
                  "closed mirror rejection is recorded",
                  e->mirrorClosedAnchor);
    CHECK_REDMCSB(result.mirrorPanelOpenBefore == 0 &&
                      result.mirrorPanelOpenAfter == 0,
                  "mirror remains closed before and after attempt",
                  e->mirrorClosedAnchor);
    CHECK_REDMCSB(result.candidateOrdinalBefore == 0 &&
                      result.candidateOrdinalAfter == 0,
                  "closed mirror has no G0299 owner",
                  e->mirrorClosedAnchor);
    CHECK_REDMCSB(result.inventoryOrdinalBefore == 0 &&
                      result.inventoryOrdinalAfter == 0,
                  "closed mirror has no inventory candidate owner",
                  e->mirrorClosedAnchor);
    CHECK_REDMCSB(result.partyOrderUnchanged == 1,
                  "closed mirror rejection preserves party order",
                  e->partyAnchor);
    CHECK_REDMCSB(result.partyReordered == 0,
                  "closed mirror rejection does not reorder party",
                  e->partyAnchor);
    CHECK_REDMCSB(result.leaderPreserved == 1,
                  "closed mirror rejection preserves leader",
                  e->defsAnchor);
    CHECK_REDMCSB(result.partyCountPreserved == 1,
                  "closed mirror rejection preserves party count",
                  e->partyAnchor);
    CHECK_REDMCSB(result.panelDrawRefreshedOnce == 0,
                  "closed mirror rejection does not refresh panel",
                  e->chamdrawAnchor);
    CHECK_REDMCSB(result.f0293CalledOnce == 0,
                  "closed mirror rejection does not call F0293",
                  e->drawAllAnchor);
    CHECK_REDMCSB(result.f0293NotCalledTwice == 1,
                  "closed mirror rejection avoids double F0293",
                  e->drawAllAnchor);
    CHECK_REDMCSB(state.rejectedMirrorClosedCount == 1,
                  "state counts one closed mirror rejection",
                  e->mirrorClosedAnchor);
    check_party(state.g0227_aT_Party,
                DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_LEADER_ID_PC34_COMPAT,
                DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_A_ID_PC34_COMPAT,
                DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_EMPTY_PC34_COMPAT,
                DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_B_ID_PC34_COMPAT,
                e->partyAnchor);
}

static void test_order_after_each_sequential_swap(void)
{
    Dm1V1MirrorCandidatePartySwapStatePc34Compat state;
    Dm1V1MirrorCandidatePartySwapResultPc34Compat first;
    Dm1V1MirrorCandidatePartySwapResultPc34Compat second;
    const Dm1V1MirrorCandidatePartySwapEvidencePc34Compat *e =
        DM1_V1_MirrorCandidatePartySwap_EvidencePc34Compat();

    DM1_V1_MirrorCandidatePartySwap_InitPc34Compat(&state, 1);
    state.g0227_aT_Party[2] =
        DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_C_ID_PC34_COMPAT;
    state.g0305_ui_PartyChampionCount = 4;

    CHECK_REDMCSB(DM1_V1_MirrorCandidatePartySwap_RunPc34Compat(
                      &state, 1, 3, &first) == 1,
                  "first open swap succeeds in full party",
                  e->slotClickAnchor);
    check_party(state.g0227_aT_Party,
                DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_LEADER_ID_PC34_COMPAT,
                DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_B_ID_PC34_COMPAT,
                DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_C_ID_PC34_COMPAT,
                DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_A_ID_PC34_COMPAT,
                e->partyAnchor);
    CHECK_REDMCSB(first.panelDrawRefreshedOnce == 1,
                  "first swap refreshes panel once",
                  e->chamdrawAnchor);
    CHECK_REDMCSB(first.f0293CalledOnce == 1,
                  "first swap calls F0293 once",
                  e->drawAllAnchor);
    CHECK_REDMCSB(first.f0293NotCalledTwice == 1,
                  "first swap does not call F0293 twice",
                  e->drawAllAnchor);
    CHECK_REDMCSB(first.g0227PartyContract == 1,
                  "first swap satisfies G0227 party contract",
                  e->partyAnchor);

    CHECK_REDMCSB(DM1_V1_MirrorCandidatePartySwap_RunPc34Compat(
                      &state, 2, 3, &second) == 1,
                  "second open swap succeeds after prior reorder",
                  e->slotClickAnchor);
    check_party(state.g0227_aT_Party,
                DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_LEADER_ID_PC34_COMPAT,
                DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_B_ID_PC34_COMPAT,
                DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_A_ID_PC34_COMPAT,
                DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_C_ID_PC34_COMPAT,
                e->partyAnchor);
    CHECK_REDMCSB(second.beforeParty[1] ==
                      DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_B_ID_PC34_COMPAT,
                  "second swap starts from first swap order",
                  e->partyAnchor);
    CHECK_REDMCSB(second.afterParty[2] ==
                      DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_A_ID_PC34_COMPAT,
                  "second swap places A at index 2",
                  e->partyAnchor);
    CHECK_REDMCSB(second.afterParty[3] ==
                      DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_C_ID_PC34_COMPAT,
                  "second swap places C at index 3",
                  e->partyAnchor);
    CHECK_REDMCSB(second.leaderPreserved == 1,
                  "second swap preserves leader",
                  e->defsAnchor);
    CHECK_REDMCSB(second.partyCountPreserved == 1,
                  "second swap preserves party count",
                  e->partyAnchor);
    CHECK_REDMCSB(second.panelDrawRefreshedOnce == 1,
                  "second swap refreshes panel once",
                  e->chamdrawAnchor);
    CHECK_REDMCSB(second.f0293CalledOnce == 1,
                  "second swap calls F0293 once",
                  e->drawAllAnchor);
    CHECK_REDMCSB(second.f0293NotCalledTwice == 1,
                  "second swap does not call F0293 twice",
                  e->drawAllAnchor);
    CHECK_REDMCSB(state.panelDrawCount == 2,
                  "two accepted swaps yield two panel draws",
                  e->chamdrawAnchor);
    CHECK_REDMCSB(state.f0293DrawAllChampionStatesCount == 2,
                  "two accepted swaps yield two F0293 calls total",
                  e->drawAllAnchor);
    CHECK_REDMCSB(state.swapAcceptedCount == 2,
                  "state counts two accepted swaps",
                  e->slotClickAnchor);
}

int main(void)
{
    test_evidence_and_fixture();
    test_open_non_adjacent_swap_refreshes_once();
    test_leader_swap_rejected_and_order_unchanged();
    test_empty_party_slot_swap_rejected();
    test_mirror_closed_swap_rejected();
    test_order_after_each_sequential_swap();

    printf("PASS dm1_v1_mirror_candidate_party_swap_pc34_compat "
           "%d/%d assertions\n",
           gPasses, gTests);
    return gPasses == gTests ? 0 : 1;
}
