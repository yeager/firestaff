#include "dm1_v1_mirror_candidate_click_cancel_pc34_compat.h"

#include <string.h>

enum {
    kDungeonViewCommand = 80,
    kSourceC040PanelGraphic = 40,
    kSourceM568PanelResurrectReincarnate = 568,
    kSourceC127ChampionPortraitSensor = 127,
    kSourceD1CViewWall = 587,
    kPartyChampionCount = 1,
    kParty0Ordinal = 1,
    kParty0Health = 72,
    kParty0Portrait = 7,
    kLeaderIndex = 0,
    kLeaderHandThing = 0x1234,
    kVisibleMirrorChampionOrdinal = 4,
    kVisibleMirrorPortraitIndex = 13,
    kC040AbsencePixelHash = 0xC0400FF
};

static const char s_source_evidence[] =
    "ReDMCSB MOVESENS.C:1501-1503 reaches REVIVE.C F0280 only through a "
    "C127 champion portrait sensor; REVIVE.C F0280 lines 272-276 appends "
    "the candidate and sets G0299, while REVIVE.C F0282 lines 744-758 and "
    "785-806 clear G0299 only after a live C040 candidate panel command; "
    "COMMAND.C:2159-2181 gates status-box and inventory-toggle dispatch on "
    "!G0299, COMMAND.C:2302-2311 gates spell/action dispatch on !G0299; "
    "DUNVIEW.C lines 8488-8533 draw D1L/D1R before D1C and lines "
    "3913-3928 blit the D1C front champion portrait; CHAMPION.C F0297/"
    "F0298/F0302 lines 243-285 and 662-706 are the leader-hand put/remove/"
    "slot routes that this D1C cell no-op must not enter.";

const Dm1V1MirrorCandidateClickCancelSpecPc34Compat
    DM1_V1_MirrorCandidateClickCancelSpecPc34Compat = {
        "dm1_v1_mirror_candidate_click_cancel_pc34_compat",
        kDungeonViewCommand,
        kSourceC040PanelGraphic,
        kSourceM568PanelResurrectReincarnate,
        kSourceC127ChampionPortraitSensor,
        kSourceD1CViewWall,
        "D1C front mirror cell click with no pending candidate is a no-op",
        s_source_evidence
    };

static int valid_party_index(int index)
{
    return index >= 0 &&
           index <
               DM1_V1_MIRROR_CANDIDATE_CLICK_CANCEL_CHAMPION_COUNT_PC34_COMPAT;
}

void dm1_v1_mirror_candidate_click_cancel_init_pc34(
    Dm1V1MirrorCandidateClickCancelStatePc34Compat *state)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->active = 1;
    state->partyChampionCount = kPartyChampionCount;
    state->preC040PartyChampionCount = kPartyChampionCount;
    state->candidateAppendCount = 0;
    state->g0299CandidateChampionOrdinal = 0u;
    state->candidateChampionOrdinal = 0u;
    state->selectedChampionOrdinal = 0u;
    state->inventoryChampionOrdinal = 0u;
    state->inventoryPanelOpen = 0;
    state->leaderIndex = kLeaderIndex;
    state->leaderHandThing = kLeaderHandThing;
    state->frontD1cMirrorChampionOrdinal = kVisibleMirrorChampionOrdinal;
    state->frontD1cMirrorPortraitIndex = kVisibleMirrorPortraitIndex;
    state->mirrorRouteArmed = 1;
    state->frontD1cCellVisible = 1;
    state->c040PanelOpen = 0;
    state->c040PanelPixelsDrawn = 0;
    state->c040AbsencePixelHash = kC040AbsencePixelHash;

    state->party[0].championOrdinal = kParty0Ordinal;
    state->party[0].currentHealth = kParty0Health;
    state->party[0].portraitOrdinal = kParty0Portrait;
    state->party[0].present = 1;
}

int dm1_v1_mirror_candidate_click_cancel_select_champion_pc34(
    Dm1V1MirrorCandidateClickCancelStatePc34Compat *state,
    unsigned int championOrdinal)
{
    int i;

    if (!state || !state->active || championOrdinal == 0u) {
        return 0;
    }
    for (i = 0; i < state->partyChampionCount; ++i) {
        if (valid_party_index(i) &&
            state->party[i].present &&
            state->party[i].championOrdinal == championOrdinal) {
            state->selectedChampionOrdinal = championOrdinal;
            return 1;
        }
    }
    return 0;
}

static void result_init(
    const Dm1V1MirrorCandidateClickCancelStatePc34Compat *state,
    Dm1V1MirrorCandidateClickCancelResultPc34Compat *result)
{
    if (!result) {
        return;
    }
    memset(result, 0, sizeof(*result));
    result->command = kDungeonViewCommand;
    if (!state) {
        return;
    }
    result->noCandidateBefore =
        state->g0299CandidateChampionOrdinal == 0u &&
        state->candidateChampionOrdinal == 0u &&
        state->candidateAppendCount == 0;
    result->noC040Before =
        state->c040PanelOpen == 0 && state->c040PanelPixelsDrawn == 0;
    result->candidateAppendCountBefore = state->candidateAppendCount;
    result->candidateAppendCountAfter = state->candidateAppendCount;
    result->partyChampionCountBefore = state->partyChampionCount;
    result->partyChampionCountAfter = state->partyChampionCount;
    result->g0299Before = state->g0299CandidateChampionOrdinal;
    result->g0299After = state->g0299CandidateChampionOrdinal;
    result->candidateChampionOrdinalBefore = state->candidateChampionOrdinal;
    result->candidateChampionOrdinalAfter = state->candidateChampionOrdinal;
    result->selectedChampionOrdinalBefore = state->selectedChampionOrdinal;
    result->selectedChampionOrdinalAfter = state->selectedChampionOrdinal;
    result->inventoryChampionOrdinalBefore = state->inventoryChampionOrdinal;
    result->inventoryChampionOrdinalAfter = state->inventoryChampionOrdinal;
    result->inventoryPanelOpenBefore = state->inventoryPanelOpen;
    result->inventoryPanelOpenAfter = state->inventoryPanelOpen;
    result->leaderIndexBefore = state->leaderIndex;
    result->leaderIndexAfter = state->leaderIndex;
    result->leaderHandThingBefore = state->leaderHandThing;
    result->leaderHandThingAfter = state->leaderHandThing;
    result->frontD1cMirrorChampionOrdinalBefore =
        state->frontD1cMirrorChampionOrdinal;
    result->frontD1cMirrorChampionOrdinalAfter =
        state->frontD1cMirrorChampionOrdinal;
    result->frontD1cMirrorPortraitIndexBefore =
        state->frontD1cMirrorPortraitIndex;
    result->frontD1cMirrorPortraitIndexAfter =
        state->frontD1cMirrorPortraitIndex;
    result->mirrorRouteArmedBefore = state->mirrorRouteArmed;
    result->mirrorRouteArmedAfter = state->mirrorRouteArmed;
    result->c040PanelPixelsBefore = state->c040PanelPixelsDrawn;
    result->c040PanelPixelsAfter = state->c040PanelPixelsDrawn;
    result->c040AbsencePixelHashBefore = state->c040AbsencePixelHash;
    result->c040AbsencePixelHashAfter = state->c040AbsencePixelHash;
}

static void result_finish(
    const Dm1V1MirrorCandidateClickCancelStatePc34Compat *state,
    Dm1V1MirrorCandidateClickCancelResultPc34Compat *result)
{
    if (!state || !result) {
        return;
    }
    result->noCandidateAfter =
        state->g0299CandidateChampionOrdinal == 0u &&
        state->candidateChampionOrdinal == 0u &&
        state->candidateAppendCount == 0;
    result->noC040After =
        state->c040PanelOpen == 0 && state->c040PanelPixelsDrawn == 0;
    result->candidateAppendCountAfter = state->candidateAppendCount;
    result->partyChampionCountAfter = state->partyChampionCount;
    result->g0299After = state->g0299CandidateChampionOrdinal;
    result->candidateChampionOrdinalAfter = state->candidateChampionOrdinal;
    result->selectedChampionOrdinalAfter = state->selectedChampionOrdinal;
    result->inventoryChampionOrdinalAfter = state->inventoryChampionOrdinal;
    result->inventoryPanelOpenAfter = state->inventoryPanelOpen;
    result->leaderIndexAfter = state->leaderIndex;
    result->leaderHandThingAfter = state->leaderHandThing;
    result->frontD1cMirrorChampionOrdinalAfter =
        state->frontD1cMirrorChampionOrdinal;
    result->frontD1cMirrorPortraitIndexAfter =
        state->frontD1cMirrorPortraitIndex;
    result->mirrorRouteArmedAfter = state->mirrorRouteArmed;
    result->c040PanelPixelsAfter = state->c040PanelPixelsDrawn;
    result->c040AbsencePixelHashAfter = state->c040AbsencePixelHash;
    result->c040PanelStayedClosed =
        result->noC040Before && result->noC040After;
    result->c040PixelsPreserved =
        result->c040PanelPixelsBefore == result->c040PanelPixelsAfter &&
        result->c040AbsencePixelHashBefore ==
            result->c040AbsencePixelHashAfter;
    result->candidateCountStayedZero =
        result->candidateAppendCountBefore == 0 &&
        result->candidateAppendCountAfter == 0;
    result->candidateIdentityStayedNone =
        result->candidateChampionOrdinalBefore == 0u &&
        result->candidateChampionOrdinalAfter == 0u &&
        result->g0299Before == 0u &&
        result->g0299After == 0u;
    result->inventoryStayedClosed =
        result->inventoryPanelOpenBefore == 0 &&
        result->inventoryPanelOpenAfter == 0 &&
        result->inventoryChampionOrdinalBefore == 0u &&
        result->inventoryChampionOrdinalAfter == 0u;
    result->mirrorRouteStayedArmed =
        result->mirrorRouteArmedBefore == 1 &&
        result->mirrorRouteArmedAfter == 1 &&
        result->frontD1cMirrorChampionOrdinalBefore ==
            result->frontD1cMirrorChampionOrdinalAfter;
    result->leaderHandUnchanged =
        result->leaderHandThingBefore == result->leaderHandThingAfter;
    result->championIdentityUnchanged =
        result->selectedChampionOrdinalBefore ==
            result->selectedChampionOrdinalAfter &&
        result->frontD1cMirrorPortraitIndexBefore ==
            result->frontD1cMirrorPortraitIndexAfter;
    result->partyCountUnchanged =
        result->partyChampionCountBefore == result->partyChampionCountAfter;
}

int dm1_v1_mirror_candidate_click_cancel_front_cell_pc34(
    Dm1V1MirrorCandidateClickCancelStatePc34Compat *state,
    Dm1V1MirrorCandidateClickCancelResultPc34Compat *outResult)
{
    result_init(state, outResult);
    if (!state || !outResult || !state->active || !state->frontD1cCellVisible) {
        return 0;
    }

    /* ReDMCSB: MOVESENS.C:1501-1503 only calls REVIVE.C F0280 for a C127
     * champion portrait sensor.  A generic visible D1C cell click has no
     * pending G0299/C040 state, so it must not synthesize the F0280 append
     * or enter REVIVE.C F0282's C040 clear/confirm paths. */
    outResult->ignoredFrontCellOnly = 1;
    result_finish(state, outResult);
    return 0;
}

const Dm1V1MirrorCandidateClickCancelSpecPc34Compat *
dm1_v1_mirror_candidate_click_cancel_spec_pc34(void)
{
    return &DM1_V1_MirrorCandidateClickCancelSpecPc34Compat;
}

const char *dm1_v1_mirror_candidate_click_cancel_source_evidence_pc34(void)
{
    return s_source_evidence;
}
