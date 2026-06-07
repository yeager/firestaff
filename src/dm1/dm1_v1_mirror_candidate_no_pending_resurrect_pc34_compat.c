#include "dm1_v1_mirror_candidate_no_pending_resurrect_pc34_compat.h"

#include <string.h>

enum {
    kG0299OpenRouteOrdinal = 1,
    kG0305NoPartyCandidate = 0,
    kInventoryChampionOrdinal = 1,
    kPanelPixelHash = 0xC040568u,
    kFrontD1cMirrorOrdinal = 1
};

static const Dm1V1MirrorCandidateNoPendingResurrectEvidencePc34Compat
    s_evidence = {
        "REVIVE.C F0280:272-276 publishes G0299 and increments G0305 for "
        "a real C127 mirror portrait candidate",
        "REVIVE.C F0282:744-758 C162 cancel starts from G0305 - 1 and clears "
        "G0299/G0305 only for a live appended candidate",
        "REVIVE.C F0282:785-806 clears G0299 before C160/C161 confirm, "
        "then disables the first mirror-square sensor",
        "COMMAND.C:2159-2181 gates status/inventory on !G0299; "
        "COMMAND.C:2302-2311 gates spell/action on !G0299",
        "COMMAND.C F0359:1985-1989 scans C040 panel commands only when the "
        "leader hand is empty",
        "contract-only no-pending mirror route: G0299 is non-zero but G0305 "
        "has no appended candidate, so C160/C161/C162 are no-op and no F0282 "
        "side effects are claimed"
    };

static int is_panel_command(int command)
{
    return command ==
            DM1_V1_MIRROR_CANDIDATE_NO_PENDING_RESURRECT_C160_PC34_COMPAT ||
        command ==
            DM1_V1_MIRROR_CANDIDATE_NO_PENDING_RESURRECT_C161_PC34_COMPAT ||
        command ==
            DM1_V1_MIRROR_CANDIDATE_NO_PENDING_RESURRECT_C162_PC34_COMPAT;
}

void DM1_V1_MirrorCandidateNoPendingResurrect_InitPc34Compat(
    Dm1V1MirrorCandidateNoPendingResurrectStatePc34Compat *state)
{
    if (!state) {
        return;
    }

    memset(state, 0, sizeof(*state));
    state->active = 1;
    state->g0299CandidateChampionOrdinal = kG0299OpenRouteOrdinal;
    state->g0305PartyChampionCount = kG0305NoPartyCandidate;
    state->inventoryChampionOrdinal = kInventoryChampionOrdinal;
    state->inventoryPanelOpen = 1;
    state->panelContent =
        DM1_V1_MIRROR_CANDIDATE_NO_PENDING_RESURRECT_PANEL_CONTENT_PC34_COMPAT;
    state->c040PanelOpen = 1;
    state->c040PanelPixelsDrawn = 1;
    state->c040PanelPixelHash = kPanelPixelHash;
    state->leaderIndex =
        DM1_V1_MIRROR_CANDIDATE_NO_PENDING_RESURRECT_NONE_PC34_COMPAT;
    state->leaderHandEmpty = 1;
    state->mirrorRouteOpen = 1;
    state->frontD1cMirrorChampionOrdinal = kFrontD1cMirrorOrdinal;
    state->magicCasterChampionIndex =
        DM1_V1_MIRROR_CANDIDATE_NO_PENDING_RESURRECT_NONE_PC34_COMPAT;
}

static void snapshot_begin(
    const Dm1V1MirrorCandidateNoPendingResurrectStatePc34Compat *state,
    int command,
    Dm1V1MirrorCandidateNoPendingResurrectResultPc34Compat *result)
{
    if (!result) {
        return;
    }

    memset(result, 0, sizeof(*result));
    result->evidence = &s_evidence;
    result->command = command;
    result->validPanelCommand = is_panel_command(command);
    if (!state) {
        return;
    }

    result->panelRouteOpen =
        state->active &&
        state->panelContent ==
            DM1_V1_MIRROR_CANDIDATE_NO_PENDING_RESURRECT_PANEL_CONTENT_PC34_COMPAT &&
        state->c040PanelOpen &&
        state->leaderHandEmpty;
    result->wouldReachF0282 =
        result->panelRouteOpen && result->validPanelCommand &&
        state->g0299CandidateChampionOrdinal != 0u &&
        state->g0305PartyChampionCount != 0u;
    result->g0299Before = state->g0299CandidateChampionOrdinal;
    result->g0299After = state->g0299CandidateChampionOrdinal;
    result->g0305Before = state->g0305PartyChampionCount;
    result->g0305After = state->g0305PartyChampionCount;
    result->inventoryChampionOrdinalBefore = state->inventoryChampionOrdinal;
    result->inventoryChampionOrdinalAfter = state->inventoryChampionOrdinal;
    result->inventoryPanelOpenBefore = state->inventoryPanelOpen;
    result->inventoryPanelOpenAfter = state->inventoryPanelOpen;
    result->panelContentBefore = state->panelContent;
    result->panelContentAfter = state->panelContent;
    result->c040PanelOpenBefore = state->c040PanelOpen;
    result->c040PanelOpenAfter = state->c040PanelOpen;
    result->c040PanelPixelsBefore = state->c040PanelPixelsDrawn;
    result->c040PanelPixelsAfter = state->c040PanelPixelsDrawn;
    result->c040PanelPixelHashBefore = state->c040PanelPixelHash;
    result->c040PanelPixelHashAfter = state->c040PanelPixelHash;
    result->leaderIndexBefore = state->leaderIndex;
    result->leaderIndexAfter = state->leaderIndex;
    result->mirrorRouteOpenBefore = state->mirrorRouteOpen;
    result->mirrorRouteOpenAfter = state->mirrorRouteOpen;
    result->frontD1cMirrorChampionOrdinalBefore =
        state->frontD1cMirrorChampionOrdinal;
    result->frontD1cMirrorChampionOrdinalAfter =
        state->frontD1cMirrorChampionOrdinal;
    result->championRearmCountBefore = state->championRearmCount;
    result->championRearmCountAfter = state->championRearmCount;
    result->f0282ResurrectCallCountBefore = state->f0282ResurrectCallCount;
    result->f0282ResurrectCallCountAfter = state->f0282ResurrectCallCount;
    result->f0282ReincarnateCallCountBefore = state->f0282ReincarnateCallCount;
    result->f0282ReincarnateCallCountAfter = state->f0282ReincarnateCallCount;
    result->f0282CancelCallCountBefore = state->f0282CancelCallCount;
    result->f0282CancelCallCountAfter = state->f0282CancelCallCount;
}

static void snapshot_finish(
    const Dm1V1MirrorCandidateNoPendingResurrectStatePc34Compat *state,
    Dm1V1MirrorCandidateNoPendingResurrectResultPc34Compat *result)
{
    if (!state || !result) {
        return;
    }

    result->g0299After = state->g0299CandidateChampionOrdinal;
    result->g0305After = state->g0305PartyChampionCount;
    result->inventoryChampionOrdinalAfter = state->inventoryChampionOrdinal;
    result->inventoryPanelOpenAfter = state->inventoryPanelOpen;
    result->panelContentAfter = state->panelContent;
    result->c040PanelOpenAfter = state->c040PanelOpen;
    result->c040PanelPixelsAfter = state->c040PanelPixelsDrawn;
    result->c040PanelPixelHashAfter = state->c040PanelPixelHash;
    result->leaderIndexAfter = state->leaderIndex;
    result->mirrorRouteOpenAfter = state->mirrorRouteOpen;
    result->frontD1cMirrorChampionOrdinalAfter =
        state->frontD1cMirrorChampionOrdinal;
    result->championRearmCountAfter = state->championRearmCount;
    result->f0282ResurrectCallCountAfter = state->f0282ResurrectCallCount;
    result->f0282ReincarnateCallCountAfter = state->f0282ReincarnateCallCount;
    result->f0282CancelCallCountAfter = state->f0282CancelCallCount;
    result->g0299Preserved = result->g0299Before == result->g0299After;
    result->g0305Preserved = result->g0305Before == result->g0305After;
    result->c040PanelPreserved =
        result->panelContentBefore == result->panelContentAfter &&
        result->c040PanelOpenBefore == result->c040PanelOpenAfter &&
        result->c040PanelPixelsBefore == result->c040PanelPixelsAfter &&
        result->c040PanelPixelHashBefore == result->c040PanelPixelHashAfter;
    result->inventoryPreserved =
        result->inventoryChampionOrdinalBefore ==
            result->inventoryChampionOrdinalAfter &&
        result->inventoryPanelOpenBefore == result->inventoryPanelOpenAfter;
    result->mirrorRoutePreserved =
        result->mirrorRouteOpenBefore == result->mirrorRouteOpenAfter &&
        result->frontD1cMirrorChampionOrdinalBefore ==
            result->frontD1cMirrorChampionOrdinalAfter;
    result->noChampionRearmed =
        result->championRearmCountBefore == result->championRearmCountAfter &&
        result->leaderIndexBefore == result->leaderIndexAfter;
    result->resurrectCallPreserved =
        result->f0282ResurrectCallCountBefore ==
        result->f0282ResurrectCallCountAfter;
    result->cancelCallPreserved =
        result->f0282CancelCallCountBefore ==
        result->f0282CancelCallCountAfter;
    result->noF0282Called =
        result->resurrectCallPreserved &&
        result->f0282ReincarnateCallCountBefore ==
            result->f0282ReincarnateCallCountAfter &&
        result->cancelCallPreserved;
}

int DM1_V1_MirrorCandidateNoPendingResurrect_ProcessPanelCommandPc34Compat(
    Dm1V1MirrorCandidateNoPendingResurrectStatePc34Compat *state,
    int command,
    Dm1V1MirrorCandidateNoPendingResurrectResultPc34Compat *outResult)
{
    snapshot_begin(state, command, outResult);
    if (!state || !outResult || !state->active) {
        snapshot_finish(state, outResult);
        return 0;
    }
    if (!is_panel_command(command)) {
        outResult->ignoredWrongCommand = 1;
        snapshot_finish(state, outResult);
        return 0;
    }
    if (!state->leaderHandEmpty) {
        /* ReDMCSB: COMMAND.C F0359:1985-1989 breaks before C040 scanning when
         * G0415_ui_LeaderEmptyHanded is false. */
        outResult->ignoredLeaderHandFull = 1;
        snapshot_finish(state, outResult);
        return 0;
    }
    if (state->g0299CandidateChampionOrdinal != 0u &&
        state->g0305PartyChampionCount == 0u) {
        /* ReDMCSB: REVIVE.C F0282:744 indexes G0305 - 1 before the C162 and
         * C160/C161 branches.  A non-zero G0299 without the F0280 G0305 append
         * is therefore a Firestaff no-pending guard, not a F0282 dispatch. */
        outResult->ignoredNoPendingCandidate = 1;
        snapshot_finish(state, outResult);
        return 0;
    }

    snapshot_finish(state, outResult);
    return 0;
}

int DM1_V1_MirrorCandidateNoPendingResurrect_CanDispatchCommandPc34Compat(
    const Dm1V1MirrorCandidateNoPendingResurrectStatePc34Compat *state,
    int command,
    Dm1V1MirrorCandidateNoPendingResurrectGateResultPc34Compat *outResult)
{
    int championIndex;

    if (outResult) {
        memset(outResult, 0, sizeof(*outResult));
        outResult->command = command;
    }
    if (!state || !outResult) {
        return 0;
    }

    outResult->blockedByG0299 = state->g0299CandidateChampionOrdinal != 0u;
    championIndex = command -
        DM1_V1_MIRROR_CANDIDATE_NO_PENDING_RESURRECT_STATUS_BOX_0_PC34_COMPAT;
    outResult->statusBoxAllowed =
        !outResult->blockedByG0299 && championIndex >= 0 &&
        command <
            DM1_V1_MIRROR_CANDIDATE_NO_PENDING_RESURRECT_STATUS_BOX_0_PC34_COMPAT +
            DM1_V1_MIRROR_CANDIDATE_NO_PENDING_RESURRECT_CHAMPION_COUNT_PC34_COMPAT &&
        (unsigned int)championIndex < state->g0305PartyChampionCount;

    championIndex = command -
        DM1_V1_MIRROR_CANDIDATE_NO_PENDING_RESURRECT_INVENTORY_0_PC34_COMPAT;
    outResult->inventoryAllowed =
        !outResult->blockedByG0299 &&
        command >=
            DM1_V1_MIRROR_CANDIDATE_NO_PENDING_RESURRECT_INVENTORY_0_PC34_COMPAT &&
        command <=
            DM1_V1_MIRROR_CANDIDATE_NO_PENDING_RESURRECT_CLOSE_INVENTORY_PC34_COMPAT &&
        (command ==
             DM1_V1_MIRROR_CANDIDATE_NO_PENDING_RESURRECT_CLOSE_INVENTORY_PC34_COMPAT ||
         (championIndex >= 0 &&
          (unsigned int)championIndex < state->g0305PartyChampionCount));
    outResult->spellAreaAllowed =
        !outResult->blockedByG0299 &&
        command ==
            DM1_V1_MIRROR_CANDIDATE_NO_PENDING_RESURRECT_SPELL_AREA_PC34_COMPAT &&
        state->magicCasterChampionIndex !=
            DM1_V1_MIRROR_CANDIDATE_NO_PENDING_RESURRECT_NONE_PC34_COMPAT;
    outResult->actionAreaAllowed =
        !outResult->blockedByG0299 &&
        command ==
            DM1_V1_MIRROR_CANDIDATE_NO_PENDING_RESURRECT_ACTION_AREA_PC34_COMPAT;
    return outResult->statusBoxAllowed || outResult->inventoryAllowed ||
        outResult->spellAreaAllowed || outResult->actionAreaAllowed;
}

const Dm1V1MirrorCandidateNoPendingResurrectEvidencePc34Compat *
DM1_V1_MirrorCandidateNoPendingResurrect_EvidencePc34Compat(void)
{
    return &s_evidence;
}
