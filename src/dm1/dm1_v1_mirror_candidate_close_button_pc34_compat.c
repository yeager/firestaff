#include "dm1_v1_mirror_candidate_close_button_pc34_compat.h"

#include <string.h>

enum {
    /* ReDMCSB COMMAND.C:508-511 binds C040 panel buttons through G0457. */
    kSourceC040PanelGraphic = 40,
    kSourceM568PanelResurrectReincarnate = 568,
    kSourceC160Resurrect = 160,
    kSourceC161Reincarnate = 161,
    kSourceC162Cancel = 162,
    /* Contract-only Firestaff close button for the C040 panel chrome. */
    kC040CloseButtonCommand = 0x4000,
    kPreviousPartyChampionCount = 2,
    kCandidatePartyOrdinal = 3,
    kCandidateMirrorOrdinal = 4,
    kPreC040InventoryChampionOrdinal = 1,
    kLeaderHandChampionOrdinal = 1,
    kLeaderIndex = 0,
    kParty0Ordinal = 1,
    kParty1Ordinal = 2,
    kParty0Health = 72,
    kParty1Health = 54,
    kParty0Portrait = 7,
    kParty1Portrait = 11,
    kCandidateHealth = 0,
    kCandidatePortrait = 13
};

static const char s_source_evidence[] =
    "ReDMCSB COMMAND.C G0457 lines 508-511 maps C040 panel buttons to "
    "C160/C161/C162; COMMAND.C F0378 lines 1985-1991 dispatches only those "
    "panel commands while M568_PANEL_RESURRECT_REINCARNATE is active; "
    "REVIVE.C F0282 lines 744-758 is the cancel/close path that clears G0299 "
    "and removes the appended candidate before returning; REVIVE.C F0282 "
    "lines 785-806 is the separate resurrect/reincarnate confirm path that "
    "clears G0299 and disables the first mirror-square sensor; COMMAND.C "
    "lines 2302-2311 gates spell/action processing on !G0299 after close; "
    "DUNVIEW.C lines 3913-3928 redraw the D1C front champion portrait ordinal "
    "that M11_GameView_SelectFrontMirrorCandidate keeps live; PANEL.C F0346 "
    "lines 1619-1635 blits C040 to the C101 panel and PANEL.C F0347 "
    "lines 1654-1656 selects that panel while G0299 is set.";

const Dm1V1MirrorCandidateCloseButtonSpecPc34Compat
    DM1_V1_MirrorCandidateCloseButtonSpecPc34Compat = {
        "dm1_v1_mirror_candidate_close_button_pc34_compat",
        kSourceC040PanelGraphic,
        kSourceM568PanelResurrectReincarnate,
        kC040CloseButtonCommand,
        kSourceC162Cancel,
        "contract-only C040 close-button marker",
        s_source_evidence
    };

static int valid_party_index(int index)
{
    return index >= 0 &&
           index < DM1_V1_MIRROR_CANDIDATE_CLOSE_BUTTON_CHAMPION_COUNT_PC34_COMPAT;
}

static int portrait_index_for_front_d1c(
    const Dm1V1MirrorCandidateCloseButtonStatePc34Compat *state)
{
    int index;

    if (!state || state->frontD1cMirrorChampionOrdinal == 0) {
        return DM1_V1_MIRROR_CANDIDATE_CLOSE_BUTTON_NONE_PC34_COMPAT;
    }
    /* ReDMCSB DUNVIEW.C:3913-3928 decrements the one-based D1C champion
     * portrait ordinal before looking up the C026 champion portrait cell. */
    index = state->frontD1cMirrorChampionOrdinal - 1;
    if (!valid_party_index(index) || !state->party[index].present) {
        return DM1_V1_MIRROR_CANDIDATE_CLOSE_BUTTON_NONE_PC34_COMPAT;
    }
    return state->party[index].portraitOrdinal;
}

void dm1_v1_mirror_candidate_close_button_init_pc34(
    Dm1V1MirrorCandidateCloseButtonStatePc34Compat *state)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->active = 1;
    state->preC040PartyChampionCount = kPreviousPartyChampionCount;
    state->partyChampionCount = kPreviousPartyChampionCount + 1;
    state->g0299CandidateChampionOrdinal = kCandidatePartyOrdinal;
    state->candidateChampionOrdinal = kCandidateMirrorOrdinal;
    state->preC040InventoryChampionOrdinal = kPreC040InventoryChampionOrdinal;
    state->inventoryChampionOrdinal = kCandidatePartyOrdinal;
    state->leaderHandChampionOrdinal = kLeaderHandChampionOrdinal;
    state->leaderIndex = kLeaderIndex;
    state->frontD1cMirrorChampionOrdinal = kCandidateMirrorOrdinal;
    state->panelContent = kSourceM568PanelResurrectReincarnate;
    state->c040PanelOpen = 1;
    state->c040PanelPixelsDrawn = 1;

    state->party[0].championOrdinal = kParty0Ordinal;
    state->party[0].currentHealth = kParty0Health;
    state->party[0].portraitOrdinal = kParty0Portrait;
    state->party[0].present = 1;

    state->party[1].championOrdinal = kParty1Ordinal;
    state->party[1].currentHealth = kParty1Health;
    state->party[1].portraitOrdinal = kParty1Portrait;
    state->party[1].present = 1;

    state->party[2].championOrdinal = kCandidateMirrorOrdinal;
    state->party[2].currentHealth = kCandidateHealth;
    state->party[2].portraitOrdinal = kCandidatePortrait;
    state->party[2].present = 1;
}

static void result_init(
    const Dm1V1MirrorCandidateCloseButtonStatePc34Compat *state,
    int command,
    Dm1V1MirrorCandidateCloseButtonResultPc34Compat *result)
{
    if (!result) {
        return;
    }
    memset(result, 0, sizeof(*result));
    result->command = command;
    result->candidateChampionOrdinalBefore = state ?
        (int)state->candidateChampionOrdinal : 0;
    result->candidateChampionOrdinalAfter =
        result->candidateChampionOrdinalBefore;
    result->g0299Before = state ? state->g0299CandidateChampionOrdinal : 0u;
    result->g0299After = result->g0299Before;
    result->inventoryChampionOrdinalBefore =
        state ? state->inventoryChampionOrdinal : 0u;
    result->inventoryChampionOrdinalAfter =
        result->inventoryChampionOrdinalBefore;
    result->inventoryChampionOrdinalPreC040 =
        state ? state->preC040InventoryChampionOrdinal : 0u;
    result->leaderHandChampionOrdinalBefore =
        state ? state->leaderHandChampionOrdinal : 0u;
    result->leaderHandChampionOrdinalAfter =
        result->leaderHandChampionOrdinalBefore;
    result->leaderIndexBefore = state ? state->leaderIndex :
        DM1_V1_MIRROR_CANDIDATE_CLOSE_BUTTON_NONE_PC34_COMPAT;
    result->leaderIndexAfter = result->leaderIndexBefore;
    result->partyChampionCountBefore = state ? state->partyChampionCount : 0;
    result->partyChampionCountAfter = result->partyChampionCountBefore;
    result->party0OrdinalBefore = state ? (int)state->party[0].championOrdinal : 0;
    result->party0OrdinalAfter = result->party0OrdinalBefore;
    result->party1OrdinalBefore = state ? (int)state->party[1].championOrdinal : 0;
    result->party1OrdinalAfter = result->party1OrdinalBefore;
    result->party0HealthBefore = state ? state->party[0].currentHealth : 0;
    result->party0HealthAfter = result->party0HealthBefore;
    result->party1HealthBefore = state ? state->party[1].currentHealth : 0;
    result->party1HealthAfter = result->party1HealthBefore;
    result->candidateSlotPresentBefore =
        state && valid_party_index(kCandidatePartyOrdinal - 1) ?
        state->party[kCandidatePartyOrdinal - 1].present : 0;
    result->candidateSlotPresentAfter = result->candidateSlotPresentBefore;
    result->previousFrontD1cMirrorChampionOrdinal =
        state ? state->frontD1cMirrorChampionOrdinal : 0;
    result->newFrontD1cMirrorChampionOrdinal =
        result->previousFrontD1cMirrorChampionOrdinal;
    result->frontD1cPortraitIndex = portrait_index_for_front_d1c(state);
    result->c040PanelPixelsBefore = state ? state->c040PanelPixelsDrawn : 0;
    result->c040PanelPixelsAfter = result->c040PanelPixelsBefore;
}

int dm1_v1_mirror_candidate_close_button_pc34(
    Dm1V1MirrorCandidateCloseButtonStatePc34Compat *state,
    int command,
    Dm1V1MirrorCandidateCloseButtonResultPc34Compat *outResult)
{
    result_init(state, command, outResult);
    if (!state || !outResult || !state->active) {
        return 0;
    }
    if (command != kC040CloseButtonCommand) {
        outResult->ignoredNotCloseButton = 1;
        return 0;
    }

    outResult->validCloseButtonCommand = 1;
    /* Contract marker: this is a Firestaff panel-chrome close command that
     * maps to the ReDMCSB C162 cancel/close cleanup, not REVIVE.C F0282's
     * C160/C161 resurrect or reincarnate confirm branch at lines 785-806. */
    state->g0299CandidateChampionOrdinal = 0u;
    state->inventoryChampionOrdinal = state->preC040InventoryChampionOrdinal;
    state->partyChampionCount = state->preC040PartyChampionCount;
    state->panelContent = 0;
    state->c040PanelOpen = 0;
    state->c040PanelPixelsDrawn = 0;
    if (valid_party_index(kCandidatePartyOrdinal - 1)) {
        memset(&state->party[kCandidatePartyOrdinal - 1],
               0,
               sizeof(state->party[kCandidatePartyOrdinal - 1]));
    }

    outResult->closedPanel = 1;
    outResult->candidateIdentityPreserved =
        outResult->candidateChampionOrdinalBefore ==
        (int)state->candidateChampionOrdinal;
    outResult->candidateChampionOrdinalAfter =
        (int)state->candidateChampionOrdinal;
    outResult->g0299After = state->g0299CandidateChampionOrdinal;
    outResult->inventoryChampionOrdinalAfter = state->inventoryChampionOrdinal;
    outResult->leaderHandChampionOrdinalAfter = state->leaderHandChampionOrdinal;
    outResult->leaderIndexAfter = state->leaderIndex;
    outResult->partyChampionCountAfter = state->partyChampionCount;
    outResult->party0OrdinalAfter = (int)state->party[0].championOrdinal;
    outResult->party1OrdinalAfter = (int)state->party[1].championOrdinal;
    outResult->party0HealthAfter = state->party[0].currentHealth;
    outResult->party1HealthAfter = state->party[1].currentHealth;
    outResult->candidateSlotPresentAfter =
        valid_party_index(kCandidatePartyOrdinal - 1) ?
        state->party[kCandidatePartyOrdinal - 1].present : 0;
    outResult->newFrontD1cMirrorChampionOrdinal =
        state->frontD1cMirrorChampionOrdinal;
    outResult->mirrorRoutePreservedFromOpen =
        outResult->newFrontD1cMirrorChampionOrdinal ==
        outResult->previousFrontD1cMirrorChampionOrdinal;
    outResult->mirrorRouteRearmedByResurrect = 0;
    outResult->frontD1cPortraitIndex = portrait_index_for_front_d1c(state);
    outResult->c040PanelCleared = state->c040PanelOpen == 0 &&
        state->c040PanelPixelsDrawn == 0;
    outResult->c040PanelPixelsAfter = state->c040PanelPixelsDrawn;
    outResult->actionAreaGateOpenAfterClose =
        state->g0299CandidateChampionOrdinal == 0u;
    outResult->candidatePromotedToLeaderHand =
        state->leaderHandChampionOrdinal == state->candidateChampionOrdinal;
    return 1;
}

const Dm1V1MirrorCandidateCloseButtonSpecPc34Compat *
dm1_v1_mirror_candidate_close_button_spec_pc34(void)
{
    return &DM1_V1_MirrorCandidateCloseButtonSpecPc34Compat;
}

const char *dm1_v1_mirror_candidate_close_button_source_evidence_pc34(void)
{
    return s_source_evidence;
}
