#include "dm1_v1_mirror_candidate_pickup_right_click_pc34_compat.h"

#include "dm1_v1_mirror_candidate_click_cancel_pc34_compat.h"
#include "dm1_v1_mirror_candidate_close_button_pc34_compat.h"
#include "dm1_v1_mirror_candidate_inventory_toggle_pc34_compat.h"
#include "dm1_v1_mirror_candidate_keyboard_browse_pc34_compat.h"
#include "dm1_v1_mirror_candidate_reincarnate_rearm_pc34_compat.h"
#include "dm1_v1_mirror_candidate_resurrect_rearm_pc34_compat.h"
#include "dm1_v1_mirror_candidate_runtime_spell_rune_pc34_compat.h"

#include <string.h>

enum {
    kBasePartyCount = 1,
    kCandidatePartyCount = 2,
    kLeaderIndex = 0,
    kCandidateOrdinal = 2,
    kCandidateThing = 0xC159u,
    kOtherLeaderHandThing = 0x0BEEu
};

static const Dm1V1MirrorCandidatePickupRightClickEvidencePc34Compat
    s_evidence = {
        "REVIVE.C F0280:272-276 publishes G0299 and increments "
        "G0305 after a C127 mirror portrait sensor",
        "REVIVE.C F0282:744-758 handles C162 cancel clearing G0299 "
        "and party count; REVIVE.C F0282:785-806 clears G0299 before "
        "resurrect/reincarnate resolution",
        "COMMAND.C:2159-2181 gates status/inventory on !G0299; "
        "COMMAND.C:2302-2311 gates spell/action on !G0299",
        "CHAMPION.C F0297/F0298/F0302:243-285,662-706 are the "
        "leader-hand put/remove/slot state-machine routes",
        "MOVESENS.C:1501-1503 reaches F0280 through a C127 mirror-cell "
        "champion portrait click",
        "COMMAND.C:484-488 maps C159..C162 champion name rows to "
        "C016..C019 with the left-button mask",
        "COMMAND.C F0359:1985-1989 processes the C040 panel only when "
        "the leader hand is empty before scanning panel commands",
        "contract-only right-click pickup hook over a deterministic C159 "
        "candidate row; no real-asset pixel parity is claimed"
    };

static void snapshot_begin(
    const Dm1V1MirrorCandidatePickupRightClickStatePc34Compat *state,
    unsigned int mouseButtons,
    Dm1V1MirrorCandidatePickupRightClickResultPc34Compat *result)
{
    if (!result) {
        return;
    }
    memset(result, 0, sizeof(*result));
    result->evidence = &s_evidence;
    result->resolvedRowIndex =
        DM1_V1_MIRROR_CANDIDATE_PICKUP_RIGHT_CLICK_NONE_PC34_COMPAT;
    result->resolvedZone =
        DM1_V1_MIRROR_CANDIDATE_PICKUP_RIGHT_CLICK_NONE_PC34_COMPAT;
    result->rightClickOnly =
        (mouseButtons &
         DM1_V1_MIRROR_CANDIDATE_PICKUP_RIGHT_CLICK_MOUSE_RIGHT_PC34_COMPAT) !=
            0u &&
        (mouseButtons &
         DM1_V1_MIRROR_CANDIDATE_PICKUP_RIGHT_CLICK_MOUSE_LEFT_PC34_COMPAT) ==
            0u;
    if (!state) {
        return;
    }
    result->partyChampionCountBefore = state->partyChampionCount;
    result->partyChampionCountAfter = state->partyChampionCount;
    result->g0299Before = state->g0299CandidateChampionOrdinal;
    result->g0299After = state->g0299CandidateChampionOrdinal;
    result->candidateChampionOrdinalBefore = state->candidateChampionOrdinal;
    result->candidateChampionOrdinalAfter = state->candidateChampionOrdinal;
    result->leaderHandThingBefore = state->leaderHandThing;
    result->leaderHandThingAfter = state->leaderHandThing;
    result->leaderHandEmptyBefore = state->leaderHandEmpty;
    result->leaderHandEmptyAfter = state->leaderHandEmpty;
    result->c040PanelOpenBefore = state->c040PanelOpen;
    result->c040PanelOpenAfter = state->c040PanelOpen;
    result->c040PanelPixelsBefore = state->c040PanelPixelsDrawn;
    result->c040PanelPixelsAfter = state->c040PanelPixelsDrawn;
    result->candidatePublishCountBefore = state->candidatePublishCount;
    result->candidatePublishCountAfter = state->candidatePublishCount;
    result->candidateClearCountBefore = state->candidateClearCount;
    result->candidateClearCountAfter = state->candidateClearCount;
    result->c040PanelPublishCountBefore = state->c040PanelPublishCount;
    result->c040PanelPublishCountAfter = state->c040PanelPublishCount;
    result->c040PanelClearCountBefore = state->c040PanelClearCount;
    result->c040PanelClearCountAfter = state->c040PanelClearCount;
    result->leaderHandPutCountBefore = state->leaderHandPutCount;
    result->leaderHandPutCountAfter = state->leaderHandPutCount;
    result->leaderHandRemoveCountBefore = state->leaderHandRemoveCount;
    result->leaderHandRemoveCountAfter = state->leaderHandRemoveCount;
    result->leftClickCommandCountBefore = state->leftClickCommandCount;
    result->leftClickCommandCountAfter = state->leftClickCommandCount;
    result->spellActionDispatchCountBefore = state->spellActionDispatchCount;
    result->spellActionDispatchCountAfter = state->spellActionDispatchCount;
}

static void snapshot_finish(
    const Dm1V1MirrorCandidatePickupRightClickStatePc34Compat *state,
    Dm1V1MirrorCandidatePickupRightClickResultPc34Compat *result)
{
    if (!state || !result) {
        return;
    }
    result->partyChampionCountAfter = state->partyChampionCount;
    result->g0299After = state->g0299CandidateChampionOrdinal;
    result->candidateChampionOrdinalAfter = state->candidateChampionOrdinal;
    result->leaderHandThingAfter = state->leaderHandThing;
    result->leaderHandEmptyAfter = state->leaderHandEmpty;
    result->c040PanelOpenAfter = state->c040PanelOpen;
    result->c040PanelPixelsAfter = state->c040PanelPixelsDrawn;
    result->candidatePublishCountAfter = state->candidatePublishCount;
    result->candidateClearCountAfter = state->candidateClearCount;
    result->c040PanelPublishCountAfter = state->c040PanelPublishCount;
    result->c040PanelClearCountAfter = state->c040PanelClearCount;
    result->leaderHandPutCountAfter = state->leaderHandPutCount;
    result->leaderHandRemoveCountAfter = state->leaderHandRemoveCount;
    result->leftClickCommandCountAfter = state->leftClickCommandCount;
    result->spellActionDispatchCountAfter = state->spellActionDispatchCount;
    result->noLeftClickCommand =
        result->leftClickCommandCountBefore == result->leftClickCommandCountAfter;
    result->c159ChampionIconGuardHeld =
        result->resolvedZone ==
            DM1_V1_MIRROR_CANDIDATE_PICKUP_RIGHT_CLICK_C159_ZONE_PC34_COMPAT &&
        result->spellActionDispatchCountBefore ==
            result->spellActionDispatchCountAfter &&
        result->noLeftClickCommand;
}

static int resolve_row(
    const Dm1V1MirrorCandidatePickupRightClickStatePc34Compat *state,
    int x,
    int y)
{
    int i;

    if (!state) {
        return DM1_V1_MIRROR_CANDIDATE_PICKUP_RIGHT_CLICK_NONE_PC34_COMPAT;
    }
    for (i = 0;
         i < DM1_V1_MIRROR_CANDIDATE_PICKUP_RIGHT_CLICK_ROW_COUNT_PC34_COMPAT;
         ++i) {
        const Dm1V1MirrorCandidatePickupRightClickRowPc34Compat *row =
            &state->rows[i];
        if (x >= row->left && x <= row->right &&
            y >= row->top && y <= row->bottom) {
            return i;
        }
    }
    return DM1_V1_MIRROR_CANDIDATE_PICKUP_RIGHT_CLICK_NONE_PC34_COMPAT;
}

static void publish_candidate(
    Dm1V1MirrorCandidatePickupRightClickStatePc34Compat *state,
    const Dm1V1MirrorCandidatePickupRightClickRowPc34Compat *row,
    Dm1V1MirrorCandidatePickupRightClickResultPc34Compat *result)
{
    /* ReDMCSB: MOVESENS.C:1501-1503 reaches REVIVE.C F0280 for a C127
     * mirror portrait; REVIVE.C F0280:272-276 then publishes G0299 and
     * grows G0305.  This hook models the adjacent C159 right-click pickup
     * as a single publish into the already-pending C040 candidate panel. */
    state->candidateChampionOrdinal = row->championOrdinal;
    state->c040PanelOpen = 1;
    state->c040PanelPixelsDrawn = 1;
    ++state->candidatePublishCount;
    ++state->c040PanelPublishCount;

    /* ReDMCSB: CHAMPION.C F0297:250-266 makes the leader hand non-empty
     * and installs the picked object/icon, including load refresh for an
     * existing leader. */
    state->leaderHandThing = row->leaderHandThing;
    state->leaderHandEmpty = 0;
    ++state->leaderHandPutCount;

    result->publishedCandidate = 1;
    result->consumed = 1;
}

static void clear_candidate(
    Dm1V1MirrorCandidatePickupRightClickStatePc34Compat *state,
    const Dm1V1MirrorCandidatePickupRightClickRowPc34Compat *row,
    Dm1V1MirrorCandidatePickupRightClickResultPc34Compat *result)
{
    (void)row;
    /* ReDMCSB: REVIVE.C F0282:744-758 clears G0299 and decrements the
     * appended candidate on cancel; F0282:785-806 also clears G0299 before
     * confirm resolution.  The right-click pickup toggle uses that same
     * clear contract for the already-published C159 row. */
    state->g0299CandidateChampionOrdinal = 0u;
    state->candidateChampionOrdinal = 0u;
    state->partyChampionCount = state->preC040PartyChampionCount;
    state->c040PanelOpen = 0;
    state->c040PanelPixelsDrawn = 0;
    ++state->candidateClearCount;
    ++state->c040PanelClearCount;

    /* ReDMCSB: CHAMPION.C F0298:279-285 marks the leader hand empty and
     * clears the pointer/name when an object is removed from the hand. */
    state->leaderHandThing =
        DM1_V1_MIRROR_CANDIDATE_PICKUP_RIGHT_CLICK_THING_NONE_PC34_COMPAT;
    state->leaderHandEmpty = 1;
    ++state->leaderHandRemoveCount;

    result->clearedCandidate = 1;
    result->noDoublePublish =
        state->candidatePublishCount == result->candidatePublishCountBefore;
    result->consumed = 1;
}

void DM1_V1_MirrorCandidatePickupRightClick_InitPc34Compat(
    Dm1V1MirrorCandidatePickupRightClickStatePc34Compat *state)
{
    int i;

    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->active = 1;
    state->partyChampionCount = kCandidatePartyCount;
    state->preC040PartyChampionCount = kBasePartyCount;
    state->g0299CandidateChampionOrdinal = kCandidateOrdinal;
    state->candidateChampionOrdinal = 0u;
    state->leaderHandThing =
        DM1_V1_MIRROR_CANDIDATE_PICKUP_RIGHT_CLICK_THING_NONE_PC34_COMPAT;
    state->leaderHandEmpty = 1;
    state->leaderIndex = kLeaderIndex;
    state->panelContent =
        DM1_V1_MIRROR_CANDIDATE_PICKUP_RIGHT_CLICK_PANEL_CONTENT_PC34_COMPAT;

    for (i = 0;
         i < DM1_V1_MIRROR_CANDIDATE_PICKUP_RIGHT_CLICK_ROW_COUNT_PC34_COMPAT;
         ++i) {
        Dm1V1MirrorCandidatePickupRightClickRowPc34Compat *row =
            &state->rows[i];
        row->zone =
            DM1_V1_MIRROR_CANDIDATE_PICKUP_RIGHT_CLICK_C159_ZONE_PC34_COMPAT +
            i;
        row->left = i * 69;
        row->right = row->left + 42;
        row->top = 0;
        row->bottom = 6;
        row->championOrdinal = (unsigned int)(kCandidateOrdinal + i);
        row->leaderHandThing = kCandidateThing + (unsigned int)i;
        row->present = i == 0 ? 1 : 0;
    }
}

int DM1_V1_MirrorCandidatePickupRightClick_ApplyPc34Compat(
    Dm1V1MirrorCandidatePickupRightClickStatePc34Compat *state,
    int x,
    int y,
    unsigned int mouseButtons,
    Dm1V1MirrorCandidatePickupRightClickResultPc34Compat *outResult)
{
    int rowIndex;
    Dm1V1MirrorCandidatePickupRightClickRowPc34Compat *row;

    snapshot_begin(state, mouseButtons, outResult);
    if (!state || !outResult || !state->active) {
        snapshot_finish(state, outResult);
        return 0;
    }
    if ((mouseButtons &
         DM1_V1_MIRROR_CANDIDATE_PICKUP_RIGHT_CLICK_MOUSE_RIGHT_PC34_COMPAT) ==
        0u) {
        snapshot_finish(state, outResult);
        return 0;
    }

    rowIndex = resolve_row(state, x, y);
    if (rowIndex ==
        DM1_V1_MIRROR_CANDIDATE_PICKUP_RIGHT_CLICK_NONE_PC34_COMPAT) {
        outResult->deadzoneSkipped = 1;
        snapshot_finish(state, outResult);
        return 0;
    }
    row = &state->rows[rowIndex];
    outResult->resolvedRowIndex = rowIndex;
    outResult->resolvedZone = row->zone;

    /* ReDMCSB: COMMAND.C:484-488 binds C159..C162 only for the left-button
     * G0455 scan; a right-only pickup hook must not synthesize that C016
     * leader-change path or the !G0299 spell/action routes at 2302-2311. */
    if ((mouseButtons &
         DM1_V1_MIRROR_CANDIDATE_PICKUP_RIGHT_CLICK_MOUSE_LEFT_PC34_COMPAT) !=
        0u) {
        ++state->leftClickCommandCount;
    }

    if (state->g0299CandidateChampionOrdinal == 0u ||
        state->panelContent !=
            DM1_V1_MIRROR_CANDIDATE_PICKUP_RIGHT_CLICK_PANEL_CONTENT_PC34_COMPAT) {
        outResult->rejectedPanelNotPending = 1;
        snapshot_finish(state, outResult);
        return 0;
    }
    if (!row->present || row->championOrdinal == 0u) {
        outResult->emptyRowNoop = 1;
        snapshot_finish(state, outResult);
        return 0;
    }

    if (state->candidateChampionOrdinal == row->championOrdinal) {
        clear_candidate(state, row, outResult);
        snapshot_finish(state, outResult);
        return outResult->consumed;
    }

    if (!state->leaderHandEmpty &&
        state->leaderHandThing !=
            DM1_V1_MIRROR_CANDIDATE_PICKUP_RIGHT_CLICK_THING_NONE_PC34_COMPAT) {
        outResult->rejectedLeaderHandFull = 1;
        snapshot_finish(state, outResult);
        return 0;
    }

    publish_candidate(state, row, outResult);
    snapshot_finish(state, outResult);
    return outResult->consumed;
}

const Dm1V1MirrorCandidatePickupRightClickEvidencePc34Compat *
DM1_V1_MirrorCandidatePickupRightClick_EvidencePc34Compat(void)
{
    return &s_evidence;
}
