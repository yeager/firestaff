#include "dm1_v1_mirror_candidate_occupied_hand_panel_pc34_compat.h"

#include <string.h>

enum {
    kCandidateOrdinal = 4,
    kCandidateIdentityToken = 0xC0404A11u,
    kPartyChampionCountWithCandidate = 4
};

static const Dm1V1MirrorCandidateOccupiedHandPanelEvidencePc34Compat
    s_evidence = {
        "ReDMCSB COMMAND.C F0359:1985-1990",
        "ReDMCSB COMMAND.C mouse table:508-512",
        "ReDMCSB REVIVE.C F0282:744-806",
        "ReDMCSB REVIVE.C F0280:124-132",
        "non-overlap: this gate covers occupied leader-hand rejection before "
        "the C040 hit scan; it does not cover no-pending resurrect, click "
        "cancel, right-click pickup, close-button hit testing, inventory "
        "portrait clicks, spell-rune guards, keyboard rotation, or party "
        "direction updates",
        "contract-only deterministic C040 panel route; no real-asset portrait "
        "or pixel parity is claimed"
    };

static void capture_before(
    const Dm1V1MirrorCandidateOccupiedHandPanelStatePc34Compat *state,
    int command,
    Dm1V1MirrorCandidateOccupiedHandPanelResultPc34Compat *result)
{
    memset(result, 0, sizeof(*result));
    result->evidence = &s_evidence;
    result->command = command;
    result->panelContentBefore = state->panelContent;
    result->panelContentAfter = state->panelContent;
    result->c040PanelOpenBefore = state->c040PanelOpen;
    result->c040PanelOpenAfter = state->c040PanelOpen;
    result->leaderHandEmptyBefore = state->leaderHandEmpty;
    result->leaderHandEmptyAfter = state->leaderHandEmpty;
    result->leaderHandObjectBefore = state->leaderHandObject;
    result->leaderHandObjectAfter = state->leaderHandObject;
    result->candidateChampionOrdinalBefore = state->candidateChampionOrdinal;
    result->candidateChampionOrdinalAfter = state->candidateChampionOrdinal;
    result->candidateIdentityTokenBefore = state->candidateIdentityToken;
    result->candidateIdentityTokenAfter = state->candidateIdentityToken;
    result->partyChampionCountBefore = state->partyChampionCount;
    result->partyChampionCountAfter = state->partyChampionCount;
    result->inventoryChampionOrdinalBefore = state->inventoryChampionOrdinal;
    result->inventoryChampionOrdinalAfter = state->inventoryChampionOrdinal;
    result->f0358HitScanCountBefore = state->f0358HitScanCount;
    result->f0358HitScanCountAfter = state->f0358HitScanCount;
    result->f0282DispatchCountBefore = state->f0282DispatchCount;
    result->f0282DispatchCountAfter = state->f0282DispatchCount;
    result->cancelClearCountBefore = state->cancelClearCount;
    result->cancelClearCountAfter = state->cancelClearCount;
    result->resurrectClearCountBefore = state->resurrectClearCount;
    result->resurrectClearCountAfter = state->resurrectClearCount;
    result->reincarnateClearCountBefore = state->reincarnateClearCount;
    result->reincarnateClearCountAfter = state->reincarnateClearCount;
    result->screenUpdateEnableCountBefore = state->screenUpdateEnableCount;
    result->screenUpdateEnableCountAfter = state->screenUpdateEnableCount;
    result->clickConsumedCountBefore = state->clickConsumedCount;
    result->clickConsumedCountAfter = state->clickConsumedCount;
}

static void capture_after(
    const Dm1V1MirrorCandidateOccupiedHandPanelStatePc34Compat *state,
    Dm1V1MirrorCandidateOccupiedHandPanelResultPc34Compat *result)
{
    result->panelContentAfter = state->panelContent;
    result->c040PanelOpenAfter = state->c040PanelOpen;
    result->leaderHandEmptyAfter = state->leaderHandEmpty;
    result->leaderHandObjectAfter = state->leaderHandObject;
    result->candidateChampionOrdinalAfter = state->candidateChampionOrdinal;
    result->candidateIdentityTokenAfter = state->candidateIdentityToken;
    result->partyChampionCountAfter = state->partyChampionCount;
    result->inventoryChampionOrdinalAfter = state->inventoryChampionOrdinal;
    result->f0358HitScanCountAfter = state->f0358HitScanCount;
    result->f0282DispatchCountAfter = state->f0282DispatchCount;
    result->cancelClearCountAfter = state->cancelClearCount;
    result->resurrectClearCountAfter = state->resurrectClearCount;
    result->reincarnateClearCountAfter = state->reincarnateClearCount;
    result->screenUpdateEnableCountAfter = state->screenUpdateEnableCount;
    result->clickConsumedCountAfter = state->clickConsumedCount;
    result->candidateStatePreserved =
        result->candidateChampionOrdinalBefore ==
            result->candidateChampionOrdinalAfter &&
        result->candidateIdentityTokenBefore ==
            result->candidateIdentityTokenAfter &&
        result->partyChampionCountBefore == result->partyChampionCountAfter &&
        result->inventoryChampionOrdinalBefore ==
            result->inventoryChampionOrdinalAfter;
    result->panelStatePreserved =
        result->panelContentBefore == result->panelContentAfter &&
        result->c040PanelOpenBefore == result->c040PanelOpenAfter;
    result->leaderHandPreserved =
        result->leaderHandEmptyBefore == result->leaderHandEmptyAfter &&
        result->leaderHandObjectBefore == result->leaderHandObjectAfter;
    result->noReviveSideEffects =
        result->f0282DispatchCountBefore == result->f0282DispatchCountAfter &&
        result->cancelClearCountBefore == result->cancelClearCountAfter &&
        result->resurrectClearCountBefore ==
            result->resurrectClearCountAfter &&
        result->reincarnateClearCountBefore ==
            result->reincarnateClearCountAfter &&
        result->screenUpdateEnableCountBefore ==
            result->screenUpdateEnableCountAfter;
}

void DM1_V1_MirrorCandidateOccupiedHandPanel_InitPc34Compat(
    Dm1V1MirrorCandidateOccupiedHandPanelStatePc34Compat *state)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->panelContent =
        DM1_V1_MIRROR_CANDIDATE_OCCUPIED_HAND_PANEL_M568_PC34_COMPAT;
    state->c040PanelOpen = 1;
    state->leaderHandEmpty = 0;
    state->leaderHandObject =
        DM1_V1_MIRROR_CANDIDATE_OCCUPIED_HAND_PANEL_OBJECT_PC34_COMPAT;
    state->candidateChampionOrdinal = kCandidateOrdinal;
    state->candidateIdentityToken = kCandidateIdentityToken;
    state->partyChampionCount = kPartyChampionCountWithCandidate;
    state->inventoryChampionOrdinal = kCandidateOrdinal;
}

int DM1_V1_MirrorCandidateOccupiedHandPanel_ClickPc34Compat(
    Dm1V1MirrorCandidateOccupiedHandPanelStatePc34Compat *state,
    int command,
    Dm1V1MirrorCandidateOccupiedHandPanelResultPc34Compat *outResult)
{
    Dm1V1MirrorCandidateOccupiedHandPanelResultPc34Compat localResult;
    Dm1V1MirrorCandidateOccupiedHandPanelResultPc34Compat *result =
        outResult ? outResult : &localResult;

    if (!state) {
        memset(result, 0, sizeof(*result));
        result->evidence = &s_evidence;
        result->command = command;
        result->commandIgnored = 1;
        return 0;
    }

    capture_before(state, command, result);
    if (state->panelContent !=
            DM1_V1_MIRROR_CANDIDATE_OCCUPIED_HAND_PANEL_M568_PC34_COMPAT ||
        !state->c040PanelOpen) {
        result->commandIgnored = 1;
        capture_after(state, result);
        return 0;
    }

    if (!state->leaderHandEmpty) {
        result->blockedByOccupiedLeaderHand = 1;
        result->f0358HitScanSkipped = 1;
        result->f0282NotInvoked = 1;
        result->commandIgnored = 1;
        capture_after(state, result);
        return 0;
    }

    ++state->f0358HitScanCount;
    ++state->f0282DispatchCount;
    ++state->clickConsumedCount;
    result->accepted = 1;
    if (command ==
        DM1_V1_MIRROR_CANDIDATE_OCCUPIED_HAND_PANEL_C162_PC34_COMPAT) {
        ++state->cancelClearCount;
    } else if (
        command ==
        DM1_V1_MIRROR_CANDIDATE_OCCUPIED_HAND_PANEL_C160_PC34_COMPAT) {
        ++state->resurrectClearCount;
    } else if (
        command ==
        DM1_V1_MIRROR_CANDIDATE_OCCUPIED_HAND_PANEL_C161_PC34_COMPAT) {
        ++state->reincarnateClearCount;
    }
    capture_after(state, result);
    return 1;
}

const Dm1V1MirrorCandidateOccupiedHandPanelEvidencePc34Compat *
DM1_V1_MirrorCandidateOccupiedHandPanel_EvidencePc34Compat(void)
{
    return &s_evidence;
}
