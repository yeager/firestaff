#include "dm1_v1_mirror_candidate_open_then_reselect_pc34_compat.h"

#include <string.h>

/* Source-lock anchors for this contract_only=1 open-then-reselect gate:
 * COMMAND.C:484-488 maps C159..C162 champion-name rows to C016..C019.
 * COMMAND.C F0359:1985-1990 dispatches the M568/C040 panel through
 * F0282 only when the leader hand is empty.
 * COMMAND.C:508-511 maps C160/C161/C162 panel buttons to the C040 panel.
 * PANEL.C F0354:2208-2240 draws the selected champion portrait box; this
 * contract treats that redraw cadence as the champion-switch observable.
 * CHEST.C F0333:30-32 leaves an already open panel/chest untouched.
 * CHEST.C F0334:113-132 closes and rewrites the visible slot chain before
 * the next open. REVIVE.C F0280:124-132 is the no-pending publish guard.
 * REVIVE.C F0282:744-758 clears G0299 on C162 cancel. COMMAND.C
 * F0380:2159-2181,2302-2311,2366-2370 gates status, inventory, spell,
 * action, and save dispatch on !G0299.
 */

enum {
    kChampionAIndex = 0,
    kChampionBIndex = 1,
    kChampionAOrdinal = 1,
    kChampionBOrdinal = 2,
    kPartyChampionCount = 2,
    kLeaderHandThing = 0x7a11,
    kChampionASlotFingerprint = 0xA159u,
    kChampionBSlotFingerprint = 0xB159u
};

static const Dm1V1MirrorCandidateOpenThenReselectEvidencePc34Compat
    s_evidence = {
        1,
        "COMMAND.C:484-488 G0455 maps C159/C160 champion-name rows to "
        "C016/C017 leader commands",
        "COMMAND.C F0359:1985-1990 M568/C040 dispatch calls F0282 only "
        "after the leader-empty guard",
        "COMMAND.C:508-511 C160/C161/C162 panel buttons use the C040 "
        "resurrect/reincarnate/cancel hit table",
        "PANEL.C F0354:2208-2240 draws the selected champion portrait box; "
        "used here as the champion-switch redraw cadence anchor",
        "CHEST.C F0333:30-32 same-open no-op preserves the already-open "
        "panel/slot state",
        "CHEST.C F0334:113-132 no-open return, G0426 clear, first-slot "
        "write, and relink rewrite before the next open",
        "REVIVE.C F0280:124-132 refuses publication with no valid pending "
        "resurrect path or occupied/full-party precondition",
        "REVIVE.C F0282:744-758 clears G0299 and decrements/cancels the "
        "candidate through C162",
        "COMMAND.C F0380:2159-2181,2302-2311,2366-2370 guards status, "
        "inventory, spell/action, and save input on !G0299",
        "non-overlap: covers C159 open then different champion reselect and "
        "re-open; existing select/click/cancel/deadzone/cancel-reselect "
        "gates cover adjacent single-step slices",
        "contract_only=1 deterministic runtime regression; no real assets, "
        "bitmaps, dungeon sensors, save files, or pixel parity are claimed"
    };

static int valid_champion_index(int championIndex)
{
    return championIndex >= 0 &&
           championIndex <
               DM1_V1_MIRROR_CANDIDATE_OPEN_THEN_RESELECT_CHAMPION_COUNT_PC34_COMPAT;
}

static unsigned int champion_ordinal(
    const Dm1V1MirrorCandidateOpenThenReselectStatePc34Compat *state,
    int championIndex)
{
    if (!state || !valid_champion_index(championIndex) ||
        !state->champions[championIndex].present) {
        return 0u;
    }
    return state->champions[championIndex].championOrdinal;
}

static int c159_maps_to_champion_command(int championIndex)
{
    return DM1_V1_MIRROR_CANDIDATE_OPEN_THEN_RESELECT_C016_PC34_COMPAT +
           championIndex;
}

void DM1_V1_MirrorCandidateOpenThenReselect_InitPc34Compat(
    Dm1V1MirrorCandidateOpenThenReselectStatePc34Compat *state)
{
    if (!state) {
        return;
    }

    memset(state, 0, sizeof(*state));
    state->contractOnly = 1;
    state->partyChampionCount = kPartyChampionCount;
    state->selectedChampionIndex = kChampionAIndex;
    state->panelOwnerChampionIndex =
        DM1_V1_MIRROR_CANDIDATE_OPEN_THEN_RESELECT_NONE_PC34_COMPAT;
    state->leaderHandThing = kLeaderHandThing;
    state->leaderHandEmpty = 1;
    state->lastRedrawChampionIndex =
        DM1_V1_MIRROR_CANDIDATE_OPEN_THEN_RESELECT_NONE_PC34_COMPAT;

    state->champions[kChampionAIndex].championOrdinal = kChampionAOrdinal;
    state->champions[kChampionAIndex].slotFingerprint =
        kChampionASlotFingerprint;
    state->champions[kChampionAIndex].present = 1;

    state->champions[kChampionBIndex].championOrdinal = kChampionBOrdinal;
    state->champions[kChampionBIndex].slotFingerprint =
        kChampionBSlotFingerprint;
    state->champions[kChampionBIndex].present = 1;
}

static void block_guarded_inputs(
    Dm1V1MirrorCandidateOpenThenReselectStatePc34Compat *state)
{
    if (state->g0299CandidateChampionOrdinal == 0u) {
        return;
    }

    ++state->blockedStatusBoxCount;
    ++state->blockedInventoryToggleCount;
    ++state->blockedSpellRuneCount;
    ++state->blockedActionAreaCount;
    ++state->blockedSaveCount;
}

static void close_panel_for_reopen(
    Dm1V1MirrorCandidateOpenThenReselectStatePc34Compat *state)
{
    if (!state->c040PanelOpen) {
        return;
    }

    ++state->f0334CloseRewriteCount;
    if (valid_champion_index(state->panelOwnerChampionIndex)) {
        state->champions[state->panelOwnerChampionIndex].slotFingerprint =
            state->panelSlotFingerprint;
    }
    state->c040PanelOpen = 0;
    state->panelContent = 0;
    state->c040Graphic = 0;
    state->panelOwnerChampionIndex =
        DM1_V1_MIRROR_CANDIDATE_OPEN_THEN_RESELECT_NONE_PC34_COMPAT;
}

int DM1_V1_MirrorCandidateOpenThenReselect_OpenCandidatePc34Compat(
    Dm1V1MirrorCandidateOpenThenReselectStatePc34Compat *state,
    int championIndex)
{
    unsigned int ordinal;

    if (!state || !state->contractOnly || !state->leaderHandEmpty ||
        state->partyChampionCount >=
            DM1_V1_MIRROR_CANDIDATE_OPEN_THEN_RESELECT_CHAMPION_COUNT_PC34_COMPAT ||
        !valid_champion_index(championIndex) ||
        !state->champions[championIndex].present) {
        return 0;
    }

    ++state->c159RowClickCount;
    (void)c159_maps_to_champion_command(championIndex);

    if (state->c040PanelOpen &&
        state->panelOwnerChampionIndex == championIndex) {
        ++state->f0333SameOpenNoopCount;
        return 0;
    }

    if (state->c040PanelOpen) {
        close_panel_for_reopen(state);
    }

    ordinal = champion_ordinal(state, championIndex);
    state->selectedChampionIndex = championIndex;
    state->g0299CandidateChampionOrdinal = ordinal;
    state->c040PanelOpen = 1;
    state->panelContent =
        DM1_V1_MIRROR_CANDIDATE_OPEN_THEN_RESELECT_M568_PANEL_PC34_COMPAT;
    state->c040Graphic =
        DM1_V1_MIRROR_CANDIDATE_OPEN_THEN_RESELECT_C040_GRAPHIC_PC34_COMPAT;
    state->panelOwnerChampionIndex = championIndex;
    state->panelSlotFingerprint =
        state->champions[championIndex].slotFingerprint;
    ++state->champions[championIndex].candidateOpenCount;
    ++state->f0359PanelDispatchCount;
    block_guarded_inputs(state);
    return 1;
}

int DM1_V1_MirrorCandidateOpenThenReselect_SelectChampionPc34Compat(
    Dm1V1MirrorCandidateOpenThenReselectStatePc34Compat *state,
    int championIndex)
{
    unsigned int handBefore;

    if (!state || !state->contractOnly || !valid_champion_index(championIndex) ||
        !state->champions[championIndex].present) {
        return 0;
    }
    if (state->selectedChampionIndex == championIndex) {
        ++state->sameChampionNoopCount;
        return 0;
    }

    handBefore = state->leaderHandThing;
    close_panel_for_reopen(state);
    state->selectedChampionIndex = championIndex;
    ++state->f0354SwitchCount;
    ++state->f0354RedrawCount;
    ++state->champions[championIndex].redrawGeneration;
    state->lastRedrawChampionIndex = championIndex;
    state->lastRedrawGeneration =
        state->champions[championIndex].redrawGeneration;
    if (state->leaderHandThing != handBefore) {
        ++state->leaderHandPutCount;
    }
    return 1;
}

int DM1_V1_MirrorCandidateOpenThenReselect_CancelPc34Compat(
    Dm1V1MirrorCandidateOpenThenReselectStatePc34Compat *state)
{
    if (!state || !state->contractOnly || !state->c040PanelOpen ||
        state->g0299CandidateChampionOrdinal == 0u) {
        return 0;
    }

    ++state->f0282CancelCount;
    if (valid_champion_index(state->panelOwnerChampionIndex)) {
        ++state->champions[state->panelOwnerChampionIndex].cancelCount;
    }
    state->g0299CandidateChampionOrdinal = 0u;
    close_panel_for_reopen(state);
    return 1;
}

int DM1_V1_MirrorCandidateOpenThenReselect_ClickResurrectPc34Compat(
    Dm1V1MirrorCandidateOpenThenReselectStatePc34Compat *state)
{
    if (!state || !state->contractOnly || !state->c040PanelOpen ||
        state->g0299CandidateChampionOrdinal == 0u) {
        if (state) {
            ++state->noPendingResurrectRejectCount;
        }
        return 0;
    }
    return 1;
}

static void capture_before(
    const Dm1V1MirrorCandidateOpenThenReselectStatePc34Compat *state,
    Dm1V1MirrorCandidateOpenThenReselectResultPc34Compat *result)
{
    memset(result, 0, sizeof(*result));
    result->evidence = &s_evidence;
    result->championAIndex = kChampionAIndex;
    result->championBIndex = kChampionBIndex;
    result->leaderHandBefore = state->leaderHandThing;
    result->leaderHandAfter = state->leaderHandThing;
    result->aSlotBefore = state->champions[kChampionAIndex].slotFingerprint;
    result->aSlotAfterReopen = result->aSlotBefore;
    result->bSlotBefore = state->champions[kChampionBIndex].slotFingerprint;
    result->bSlotAfterReopen = result->bSlotBefore;
    result->f0354SwitchCountBefore = state->f0354SwitchCount;
    result->f0354RedrawCountBefore = state->f0354RedrawCount;
    result->f0334CloseRewriteCountBefore = state->f0334CloseRewriteCount;
    result->f0282CancelCountBefore = state->f0282CancelCount;
}

static void finish_result(
    const Dm1V1MirrorCandidateOpenThenReselectStatePc34Compat *state,
    Dm1V1MirrorCandidateOpenThenReselectResultPc34Compat *result)
{
    result->leaderHandAfter = state->leaderHandThing;
    result->aSlotAfterReopen = state->panelSlotFingerprint;
    result->bSlotAfterReopen =
        state->champions[kChampionBIndex].slotFingerprint;
    result->handCarryPreserved =
        result->leaderHandBefore == result->leaderHandAfter &&
        state->leaderHandPutCount == 0 &&
        state->leaderHandRemoveCount == 0;
    result->redrawCadencePreserved =
        result->f0354SwitchCountAfterBSelect ==
            result->f0354SwitchCountBefore + 1 &&
        result->f0354RedrawCountAfterBSelect ==
            result->f0354RedrawCountBefore + 1 &&
        result->f0354SwitchCountAfterAReselect ==
            result->f0354SwitchCountBefore + 2 &&
        result->f0354RedrawCountAfterAReselect ==
            result->f0354RedrawCountBefore + 2;
    result->noBRedrawLeakedIntoA =
        state->bRedrawLeakIntoA == 0 &&
        state->lastRedrawChampionIndex == kChampionAIndex &&
        state->champions[kChampionBIndex].redrawGeneration == 1 &&
        state->champions[kChampionAIndex].redrawGeneration == 1;
    result->guardsBlockedWhileG0299 =
        result->blockedStatusBoxCountAfter >= 3 &&
        result->blockedInventoryToggleCountAfter >= 3 &&
        result->blockedSpellRuneCountAfter >= 3 &&
        result->blockedActionAreaCountAfter >= 3 &&
        result->blockedSaveCountAfter >= 3;
    result->noLeaderHandRoutes =
        state->leaderHandPutCount == 0 &&
        state->leaderHandRemoveCount == 0;
    result->noSlotRoutes = state->slotRouteCount == 0;
    result->contractOnly = state->contractOnly == 1 &&
                           s_evidence.contractOnly == 1;
    result->noAssetsOrPixelParity =
        state->assetLoadCount == 0 &&
        state->pixelParityClaimCount == 0;
    result->ok =
        result->c159MappedToC016A &&
        result->c159MappedToC017B &&
        result->aPanelOpened &&
        result->bSelectedViaF0354 &&
        result->bPanelReopened &&
        result->bPanelUsesBSlotState &&
        result->bCancelClearedPending &&
        result->reopenedAUsesPreviousAState &&
        result->noBRedrawLeakedIntoA &&
        result->handCarryPreserved &&
        result->redrawCadencePreserved &&
        result->sameChampionDeadzoneNoop &&
        result->sameOpenNoopPreserved &&
        result->noPendingResurrectRejected &&
        result->closeRewriteRanBeforeBOpen &&
        result->guardsBlockedWhileG0299 &&
        result->noLeaderHandRoutes &&
        result->noSlotRoutes &&
        result->contractOnly &&
        result->noAssetsOrPixelParity;
}

int DM1_V1_MirrorCandidateOpenThenReselect_RunPc34Compat(
    Dm1V1MirrorCandidateOpenThenReselectStatePc34Compat *state,
    Dm1V1MirrorCandidateOpenThenReselectResultPc34Compat *outResult)
{
    unsigned int aSlot;
    unsigned int bSlot;
    int openedA;
    int sameOpen;
    int sameSelect;
    int selectedB;
    int openedB;
    int rejectedNoPending;
    int cancelledB;
    int selectedA;
    int reopenedA;

    if (!state || !outResult || !state->contractOnly) {
        return 0;
    }

    capture_before(state, outResult);
    aSlot = state->champions[kChampionAIndex].slotFingerprint;
    bSlot = state->champions[kChampionBIndex].slotFingerprint;

    openedA =
        DM1_V1_MirrorCandidateOpenThenReselect_OpenCandidatePc34Compat(
            state, kChampionAIndex);
    outResult->aPanelOpened =
        openedA &&
        state->c040PanelOpen &&
        state->panelOwnerChampionIndex == kChampionAIndex &&
        state->panelSlotFingerprint == aSlot;
    outResult->g0299AfterAOpen = state->g0299CandidateChampionOrdinal;
    outResult->c159MappedToC016A =
        c159_maps_to_champion_command(kChampionAIndex) ==
        DM1_V1_MIRROR_CANDIDATE_OPEN_THEN_RESELECT_C016_PC34_COMPAT;

    sameOpen =
        DM1_V1_MirrorCandidateOpenThenReselect_OpenCandidatePc34Compat(
            state, kChampionAIndex);
    outResult->sameOpenNoopPreserved =
        sameOpen == 0 &&
        state->f0333SameOpenNoopCount == 1 &&
        state->panelOwnerChampionIndex == kChampionAIndex &&
        state->panelSlotFingerprint == aSlot;

    sameSelect =
        DM1_V1_MirrorCandidateOpenThenReselect_SelectChampionPc34Compat(
            state, kChampionAIndex);
    outResult->sameChampionDeadzoneNoop =
        sameSelect == 0 &&
        state->sameChampionNoopCount == 1 &&
        state->selectedChampionIndex == kChampionAIndex &&
        state->panelOwnerChampionIndex == kChampionAIndex;

    selectedB =
        DM1_V1_MirrorCandidateOpenThenReselect_SelectChampionPc34Compat(
            state, kChampionBIndex);
    outResult->bSelectedViaF0354 =
        selectedB &&
        state->selectedChampionIndex == kChampionBIndex &&
        state->lastRedrawChampionIndex == kChampionBIndex &&
        state->leaderHandThing == kLeaderHandThing;
    outResult->f0354SwitchCountAfterBSelect = state->f0354SwitchCount;
    outResult->f0354RedrawCountAfterBSelect = state->f0354RedrawCount;

    openedB =
        DM1_V1_MirrorCandidateOpenThenReselect_OpenCandidatePc34Compat(
            state, kChampionBIndex);
    outResult->g0299AfterBOpen = state->g0299CandidateChampionOrdinal;
    outResult->f0334CloseRewriteCountAfterBOpen =
        state->f0334CloseRewriteCount;
    outResult->bPanelReopened =
        openedB &&
        state->c040PanelOpen &&
        state->panelOwnerChampionIndex == kChampionBIndex;
    outResult->bPanelUsesBSlotState =
        state->panelSlotFingerprint == bSlot &&
        outResult->g0299AfterBOpen == kChampionBOrdinal;
    outResult->c159MappedToC017B =
        c159_maps_to_champion_command(kChampionBIndex) ==
        DM1_V1_MIRROR_CANDIDATE_OPEN_THEN_RESELECT_C016_PC34_COMPAT + 1;
    outResult->closeRewriteRanBeforeBOpen =
        outResult->f0334CloseRewriteCountAfterBOpen ==
        outResult->f0334CloseRewriteCountBefore + 1;

    cancelledB = DM1_V1_MirrorCandidateOpenThenReselect_CancelPc34Compat(
        state);
    outResult->g0299AfterBCancel = state->g0299CandidateChampionOrdinal;
    outResult->f0282CancelCountAfterB = state->f0282CancelCount;
    outResult->bCancelClearedPending =
        cancelledB &&
        outResult->g0299AfterBCancel == 0u &&
        outResult->f0282CancelCountAfterB ==
            outResult->f0282CancelCountBefore + 1 &&
        state->champions[kChampionBIndex].cancelCount == 1;

    rejectedNoPending =
        DM1_V1_MirrorCandidateOpenThenReselect_ClickResurrectPc34Compat(
            state);
    outResult->noPendingResurrectRejected =
        rejectedNoPending == 0 &&
        state->noPendingResurrectRejectCount == 1 &&
        state->g0299CandidateChampionOrdinal == 0u;

    selectedA =
        DM1_V1_MirrorCandidateOpenThenReselect_SelectChampionPc34Compat(
            state, kChampionAIndex);
    outResult->f0354SwitchCountAfterAReselect = state->f0354SwitchCount;
    outResult->f0354RedrawCountAfterAReselect = state->f0354RedrawCount;
    reopenedA =
        DM1_V1_MirrorCandidateOpenThenReselect_OpenCandidatePc34Compat(
            state, kChampionAIndex);
    outResult->g0299AfterAReopen = state->g0299CandidateChampionOrdinal;
    outResult->reopenedAUsesPreviousAState =
        selectedA &&
        reopenedA &&
        state->panelOwnerChampionIndex == kChampionAIndex &&
        state->panelSlotFingerprint == aSlot &&
        state->panelSlotFingerprint != bSlot &&
        state->g0299CandidateChampionOrdinal == kChampionAOrdinal;

    outResult->blockedStatusBoxCountAfter = state->blockedStatusBoxCount;
    outResult->blockedInventoryToggleCountAfter =
        state->blockedInventoryToggleCount;
    outResult->blockedSpellRuneCountAfter = state->blockedSpellRuneCount;
    outResult->blockedActionAreaCountAfter = state->blockedActionAreaCount;
    outResult->blockedSaveCountAfter = state->blockedSaveCount;
    finish_result(state, outResult);
    return outResult->ok;
}

const Dm1V1MirrorCandidateOpenThenReselectEvidencePc34Compat *
DM1_V1_MirrorCandidateOpenThenReselect_EvidencePc34Compat(void)
{
    return &s_evidence;
}
