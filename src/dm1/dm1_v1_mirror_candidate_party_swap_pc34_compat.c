#include "dm1_v1_mirror_candidate_party_swap_pc34_compat.h"

#include <string.h>

/* ReDMCSB: CHAMPION.C:243-340 F0297_CHAMPION_PutObjectInLeaderHand anchors
 * the leader-hand side effects that this party-order gate must not enter.
 * ReDMCSB: CHAMPION.C:587-700 F0301_CHAMPION_AddObjectInSlot and
 * F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox anchor slot click
 * handling and the G0299 candidate early return.
 * ReDMCSB: CHAMPION.C:2496-2499 F0293_CHAMPION_DrawAllChampionStates anchors
 * the single whole-party redraw after candidate state changes.
 * ReDMCSB: DEFS.H:780-807 defines CM1_SLOT_LEADER_HAND, C00_SLOT_READY_HAND,
 * C01_SLOT_ACTION_HAND, C28_SLOT_BACKPACK_LINE1_8, and C29_SLOT_BACKPACK_LINE1_9;
 * this gate exposes the requested C00_SLOT_LEADER_HAND/C28_PARTY_FIRST/
 * C29_PARTY_LAST compat names for the mirror-party swap command range.
 * ReDMCSB: CHAMDRAW.C:540-700 anchors mirror-candidate slot drawing and the
 * G0299 candidate condition that allows drawing the candidate's action hand.
 */

enum {
    kLeaderIndex = 0,
    kCandidateOrdinal = 4,
    kInventoryOrdinal = 4
};

static const Dm1V1MirrorCandidatePartySwapEvidencePc34Compat s_evidence = {
    "ReDMCSB: CHAMPION.C:243-340 F0297_CHAMPION_PutObjectInLeaderHand; "
    "leader hand object/weight redraw path is intentionally not entered",
    "ReDMCSB: CHAMPION.C:587-700 F0301_CHAMPION_AddObjectInSlot and "
    "F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox; G0299 slot-click "
    "early return keeps mirror candidate routing separate",
    "ReDMCSB: CHAMPION.C:2496-2499 F0293_CHAMPION_DrawAllChampionStates; "
    "party swap requests exactly one full state redraw",
    "ReDMCSB: DEFS.H:780-807 CM1_SLOT_LEADER_HAND/C00_SLOT_READY_HAND/"
    "C01_SLOT_ACTION_HAND plus C28/C29 slot constants; compat exposes "
    "C00_SLOT_LEADER_HAND, C28_PARTY_FIRST, C29_PARTY_LAST names",
    "ReDMCSB: CHAMDRAW.C:540-700 mirror-candidate slot rendering and "
    "G0299 candidate draw condition",
    "ReDMCSB: REVIVE.C:272-276 publishes G0299 and G0305 party count; "
    "CHAMPION.C:2333-2335 iterates party candidates while skipping G0299",
    "ReDMCSB: COMMAND.C:2158-2182 and CHAMPION.C:678-681 gate status/slot "
    "routes while G0299 owns the mirror candidate panel",
    "ReDMCSB: non-overlap with select/click/cancel/deadzone/cancel-reselect; "
    "this gate only swaps party order A<->B and checks redraw cadence"
};

static void copy_party(
    int dst[DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_MAX_PARTY_PC34_COMPAT],
    const int src[DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_MAX_PARTY_PC34_COMPAT])
{
    int i;

    for (i = 0;
         i < DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_MAX_PARTY_PC34_COMPAT;
         ++i) {
        dst[i] = src[i];
    }
}

static int party_count(
    const int party[DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_MAX_PARTY_PC34_COMPAT])
{
    int count = 0;
    int i;

    for (i = 0;
         i < DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_MAX_PARTY_PC34_COMPAT;
         ++i) {
        if (party[i] != DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_EMPTY_PC34_COMPAT) {
            ++count;
        }
    }
    return count;
}

static int parties_equal(
    const int a[DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_MAX_PARTY_PC34_COMPAT],
    const int b[DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_MAX_PARTY_PC34_COMPAT])
{
    int i;

    for (i = 0;
         i < DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_MAX_PARTY_PC34_COMPAT;
         ++i) {
        if (a[i] != b[i]) {
            return 0;
        }
    }
    return 1;
}

static int only_pair_moved(
    const Dm1V1MirrorCandidatePartySwapResultPc34Compat *result)
{
    int i;

    for (i = 0;
         i < DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_MAX_PARTY_PC34_COMPAT;
         ++i) {
        if (i == result->requestedIndexA || i == result->requestedIndexB) {
            continue;
        }
        if (result->beforeParty[i] != result->afterParty[i]) {
            return 0;
        }
    }
    return result->beforeParty[result->requestedIndexA] ==
               result->afterParty[result->requestedIndexB] &&
           result->beforeParty[result->requestedIndexB] ==
               result->afterParty[result->requestedIndexA];
}

static void capture_before(
    const Dm1V1MirrorCandidatePartySwapStatePc34Compat *state,
    int indexA,
    int indexB,
    Dm1V1MirrorCandidatePartySwapResultPc34Compat *result)
{
    memset(result, 0, sizeof(*result));
    result->evidence = &s_evidence;
    result->requestedIndexA = indexA;
    result->requestedIndexB = indexB;
    copy_party(result->beforeParty, state->g0227_aT_Party);
    result->partyCountBefore = state->g0305_ui_PartyChampionCount;
    result->leaderIndexBefore = state->g0411_i_LeaderIndex;
    result->leaderIdBefore = state->g0227_aT_Party[kLeaderIndex];
    result->mirrorPanelOpenBefore = state->mirrorPanelOpen;
    result->candidateOrdinalBefore = state->g0299_ui_CandidateChampionOrdinal;
    result->inventoryOrdinalBefore = state->g0423_i_InventoryChampionOrdinal;
    result->panelDrawCountBefore = state->panelDrawCount;
    result->f0293DrawAllCountBefore =
        state->f0293DrawAllChampionStatesCount;
}

static void capture_after(
    const Dm1V1MirrorCandidatePartySwapStatePc34Compat *state,
    Dm1V1MirrorCandidatePartySwapResultPc34Compat *result)
{
    copy_party(result->afterParty, state->g0227_aT_Party);
    result->partyCountAfter = state->g0305_ui_PartyChampionCount;
    result->leaderIndexAfter = state->g0411_i_LeaderIndex;
    result->leaderIdAfter = state->g0227_aT_Party[kLeaderIndex];
    result->mirrorPanelOpenAfter = state->mirrorPanelOpen;
    result->candidateOrdinalAfter = state->g0299_ui_CandidateChampionOrdinal;
    result->inventoryOrdinalAfter = state->g0423_i_InventoryChampionOrdinal;
    result->panelDrawCountAfter = state->panelDrawCount;
    result->f0293DrawAllCountAfter =
        state->f0293DrawAllChampionStatesCount;
    result->partyReordered =
        !parties_equal(result->beforeParty, result->afterParty);
    result->partyOrderUnchanged =
        parties_equal(result->beforeParty, result->afterParty);
    result->leaderPreserved =
        result->leaderIndexBefore == kLeaderIndex &&
        result->leaderIndexAfter == kLeaderIndex &&
        result->leaderIdBefore ==
            DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_LEADER_ID_PC34_COMPAT &&
        result->leaderIdAfter ==
            DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_LEADER_ID_PC34_COMPAT;
    result->partyCountPreserved =
        result->partyCountBefore == result->partyCountAfter &&
        result->partyCountAfter == party_count(result->afterParty);
    result->mirrorOwnerPreserved =
        result->candidateOrdinalBefore == result->candidateOrdinalAfter &&
        result->inventoryOrdinalBefore == result->inventoryOrdinalAfter;
    result->panelDrawRefreshedOnce =
        result->panelDrawCountAfter - result->panelDrawCountBefore == 1;
    result->f0293CalledOnce =
        result->f0293DrawAllCountAfter -
            result->f0293DrawAllCountBefore == 1;
    result->f0293NotCalledTwice =
        result->f0293DrawAllCountAfter -
            result->f0293DrawAllCountBefore != 2;
    result->onlyRequestedPairMoved =
        result->accepted ? only_pair_moved(result) : result->partyOrderUnchanged;
    result->g0227PartyContract =
        result->leaderPreserved &&
        result->partyCountPreserved &&
        result->mirrorOwnerPreserved &&
        result->onlyRequestedPairMoved;
}

void DM1_V1_MirrorCandidatePartySwap_InitPc34Compat(
    Dm1V1MirrorCandidatePartySwapStatePc34Compat *state,
    int mirrorPanelOpen)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->g0227_aT_Party[0] =
        DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_LEADER_ID_PC34_COMPAT;
    state->g0227_aT_Party[1] =
        DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_A_ID_PC34_COMPAT;
    state->g0227_aT_Party[2] =
        DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_EMPTY_PC34_COMPAT;
    state->g0227_aT_Party[3] =
        DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_B_ID_PC34_COMPAT;
    state->g0305_ui_PartyChampionCount = party_count(state->g0227_aT_Party);
    state->g0411_i_LeaderIndex = kLeaderIndex;
    state->g0299_ui_CandidateChampionOrdinal =
        mirrorPanelOpen ? kCandidateOrdinal : 0;
    state->g0423_i_InventoryChampionOrdinal =
        mirrorPanelOpen ? kInventoryOrdinal : 0;
    state->mirrorPanelOpen = mirrorPanelOpen ? 1 : 0;
}

int DM1_V1_MirrorCandidatePartySwap_RunPc34Compat(
    Dm1V1MirrorCandidatePartySwapStatePc34Compat *state,
    int indexA,
    int indexB,
    Dm1V1MirrorCandidatePartySwapResultPc34Compat *outResult)
{
    int temp;

    if (!state || !outResult) {
        return 0;
    }
    capture_before(state, indexA, indexB, outResult);
    ++state->swapAttemptCount;

    if (!state->mirrorPanelOpen) {
        ++state->rejectedMirrorClosedCount;
        outResult->rejectedMirrorClosed = 1;
        capture_after(state, outResult);
        return 0;
    }
    if (indexA <= kLeaderIndex || indexB <= kLeaderIndex ||
        indexA >= DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_MAX_PARTY_PC34_COMPAT ||
        indexB >= DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_MAX_PARTY_PC34_COMPAT) {
        ++state->rejectedLeaderSwapCount;
        outResult->rejectedLeaderSwap = 1;
        capture_after(state, outResult);
        return 0;
    }
    if (state->g0227_aT_Party[indexA] ==
            DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_EMPTY_PC34_COMPAT ||
        state->g0227_aT_Party[indexB] ==
            DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_EMPTY_PC34_COMPAT) {
        ++state->rejectedEmptySlotCount;
        outResult->rejectedEmptySlot = 1;
        capture_after(state, outResult);
        return 0;
    }

    temp = state->g0227_aT_Party[indexA];
    state->g0227_aT_Party[indexA] = state->g0227_aT_Party[indexB];
    state->g0227_aT_Party[indexB] = temp;
    ++state->swapAcceptedCount;
    ++state->panelDrawCount;
    ++state->f0293DrawAllChampionStatesCount;
    outResult->accepted = 1;

    capture_after(state, outResult);
    return outResult->accepted;
}

const Dm1V1MirrorCandidatePartySwapEvidencePc34Compat *
DM1_V1_MirrorCandidatePartySwap_EvidencePc34Compat(void)
{
    return &s_evidence;
}
