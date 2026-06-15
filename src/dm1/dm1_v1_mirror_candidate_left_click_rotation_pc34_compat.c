#include "dm1_v1_mirror_candidate_left_click_rotation_pc34_compat.h"

#include <string.h>

/* Contract-only left-click rotation gate.
 *
 * ReDMCSB COMMAND.C F0359:1985-1990 dispatches the live M568/C040 mirror
 * panel through the click table before F0282 receives panel commands.
 * COMMAND.C:484-488 is the separate C159 name-row path; this slice proves
 * the left-click rotation hand-off does not use it.
 * CHAMPION.C F0284:93-131 is the party-direction rotation model, while
 * F0297:243-298, F0298:270-298, and F0300/F0301:511-515,606-614 are the
 * leader-hand/slot mutations this view-only rotation must not enter.
 * PANEL.C F0354:2208-2240 redraws champion portraits after a champion switch.
 * DEFS.H:2088 C10_COLOR_FLESH, M070_HAND_SLOT_INDEX at 1878,
 * M516_CHAMPIONS at 873-876, and G0299/G0305/G0423/G0425/G0426 globals at
 * 5694/5700/5876/5878/5881 anchor the roster, hand, inventory, and pending
 * candidate surfaces guarded here. !G0299 guarded status/spell/save routes
 * are COMMAND.C:2158-2182,2302-2311,2366-2370.
 */

enum {
    kInitialLeaderIndex = 0,
    kInitialCandidateIndex = 1,
    kInitialLeaderHandEmpty = 0,
    kVisibleOrdinal0 = 11,
    kVisibleOrdinal1 = 12,
    kVisibleOrdinal2 = 13,
    kVisibleOrdinal3 = 14
};

static const Dm1V1MirrorCandidateLeftClickRotationEvidencePc34Compat
    s_evidence = {
        1,
        "COMMAND.C F0359:1985-1990 M568/C040 mirror dispatch to F0282",
        "COMMAND.C F0282 click-to-C040 routing; COMMAND.C:508-511 "
        "left-click panel button table is intentionally not the C159 row",
        "CHAMPION.C F0284:93-131 party-direction rotation state is separate "
        "from candidate view rotation",
        "CHAMPION.C F0297:243-298 leader-hand put must not run",
        "CHAMPION.C F0298:270-298 leader-hand remove must not run",
        "CHAMPION.C F0300/F0301:511-515,606-614 C30 slot clear/write "
        "must not run",
        "PANEL.C F0354:2208-2240 champion-switch portrait/redraw contract",
        "DEFS.H:2088 C10_COLOR_FLESH",
        "DEFS.H:M070_HAND_SLOT_INDEX line 1878 chest/inventory hand swap",
        "DEFS.H:M516_CHAMPIONS lines 873-876 visible candidate roster; "
        "M516 rotation mask remains a view-selection mask in this contract",
        "DEFS.H:G0425/G0426/G0423/G0305 lines 5876-5881 and 5700",
        "!G0299 guards COMMAND.C:2158-2182 status/inventory, 2302-2311 "
        "spell/action, and 2366-2370 save routes",
        "non-overlap: left-click candidate rotation only; no C159 click "
        "combo, keyboard rotation, open-then-reselect, right-click pickup, "
        "close button, chest close, or leader-hand pickup path is modeled",
        "contract_only=1 deterministic runtime regression gate; no real "
        "asset, framebuffer, DOSBox transcript, or full hit-test parity "
        "is claimed"
    };

static unsigned int candidate_ordinal_at(
    const Dm1V1MirrorCandidateLeftClickRotationStatePc34Compat *state,
    int index)
{
    if (!state || index < 0 || index >= state->visibleCandidateCount) {
        return 0u;
    }
    return state->visibleCandidateOrdinals[index];
}

static void capture_before(
    const Dm1V1MirrorCandidateLeftClickRotationStatePc34Compat *state,
    Dm1V1MirrorCandidateLeftClickRotationResultPc34Compat *result)
{
    memset(result, 0, sizeof(*result));
    result->evidence = &s_evidence;
    result->visibleCandidateCountBefore = state->visibleCandidateCount;
    result->visibleCandidateCountAfter = state->visibleCandidateCount;
    result->visibleCandidateIndexBefore = state->visibleCandidateIndex;
    result->visibleCandidateIndexAfter = state->visibleCandidateIndex;
    result->candidateOrdinalBefore =
        candidate_ordinal_at(state, state->visibleCandidateIndex);
    result->candidateOrdinalAfter = result->candidateOrdinalBefore;
    result->g0299Before = state->g0299CandidateChampionOrdinal;
    result->g0299After = state->g0299CandidateChampionOrdinal;
    result->leaderHandThingBefore = state->leaderHandThingOrdinal;
    result->leaderHandThingAfter = state->leaderHandThingOrdinal;
    result->leaderHandEmptyBefore = state->leaderHandEmpty;
    result->leaderHandEmptyAfter = state->leaderHandEmpty;
    result->leaderIndexBefore = state->leaderIndex;
    result->leaderIndexAfter = state->leaderIndex;
    result->leftClickDispatchCountBefore = state->leftClickDispatchCount;
    result->leftClickDispatchCountAfter = state->leftClickDispatchCount;
    result->mirrorCandidateHandlerCountBefore =
        state->mirrorCandidateHandlerCount;
    result->mirrorCandidateHandlerCountAfter =
        state->mirrorCandidateHandlerCount;
    result->rotationDispatchCountBefore = state->rotationDispatchCount;
    result->rotationDispatchCountAfter = state->rotationDispatchCount;
    result->c159NameRowDispatchCountBefore =
        state->c159NameRowDispatchCount;
    result->c159NameRowDispatchCountAfter = state->c159NameRowDispatchCount;
    result->c159SetLeaderCountBefore = state->c159SetLeaderCount;
    result->c159SetLeaderCountAfter = state->c159SetLeaderCount;
    result->statusBoxDispatchCountBefore = state->statusBoxDispatchCount;
    result->statusBoxDispatchCountAfter = state->statusBoxDispatchCount;
    result->spellRuneDispatchCountBefore = state->spellRuneDispatchCount;
    result->spellRuneDispatchCountAfter = state->spellRuneDispatchCount;
    result->saveDispatchCountBefore = state->saveDispatchCount;
    result->saveDispatchCountAfter = state->saveDispatchCount;
    result->leaderHandPutCountBefore = state->leaderHandPutCount;
    result->leaderHandPutCountAfter = state->leaderHandPutCount;
    result->leaderHandRemoveCountBefore = state->leaderHandRemoveCount;
    result->leaderHandRemoveCountAfter = state->leaderHandRemoveCount;
    result->slotWriteCountBefore = state->slotWriteCount;
    result->slotWriteCountAfter = state->slotWriteCount;
    result->slotClearCountBefore = state->slotClearCount;
    result->slotClearCountAfter = state->slotClearCount;
    result->portraitRedrawCountBefore = state->portraitRedrawCount;
    result->portraitRedrawCountAfter = state->portraitRedrawCount;
}

static void capture_after(
    const Dm1V1MirrorCandidateLeftClickRotationStatePc34Compat *state,
    Dm1V1MirrorCandidateLeftClickRotationResultPc34Compat *result)
{
    int expectedIndex =
        (result->visibleCandidateIndexBefore + 1) %
        result->visibleCandidateCountBefore;

    result->visibleCandidateCountAfter = state->visibleCandidateCount;
    result->visibleCandidateIndexAfter = state->visibleCandidateIndex;
    result->candidateOrdinalAfter =
        candidate_ordinal_at(state, state->visibleCandidateIndex);
    result->g0299After = state->g0299CandidateChampionOrdinal;
    result->leaderHandThingAfter = state->leaderHandThingOrdinal;
    result->leaderHandEmptyAfter = state->leaderHandEmpty;
    result->leaderIndexAfter = state->leaderIndex;
    result->leftClickDispatchCountAfter = state->leftClickDispatchCount;
    result->mirrorCandidateHandlerCountAfter =
        state->mirrorCandidateHandlerCount;
    result->rotationDispatchCountAfter = state->rotationDispatchCount;
    result->c159NameRowDispatchCountAfter = state->c159NameRowDispatchCount;
    result->c159SetLeaderCountAfter = state->c159SetLeaderCount;
    result->statusBoxDispatchCountAfter = state->statusBoxDispatchCount;
    result->spellRuneDispatchCountAfter = state->spellRuneDispatchCount;
    result->saveDispatchCountAfter = state->saveDispatchCount;
    result->leaderHandPutCountAfter = state->leaderHandPutCount;
    result->leaderHandRemoveCountAfter = state->leaderHandRemoveCount;
    result->slotWriteCountAfter = state->slotWriteCount;
    result->slotClearCountAfter = state->slotClearCount;
    result->portraitRedrawCountAfter = state->portraitRedrawCount;

    result->eventDispatchedToMirrorCandidateHandler =
        result->leftClickDispatchCountAfter ==
            result->leftClickDispatchCountBefore + 1 &&
        result->mirrorCandidateHandlerCountAfter ==
            result->mirrorCandidateHandlerCountBefore + 1 &&
        result->rotationDispatchCountAfter ==
            result->rotationDispatchCountBefore + 1;
    result->candidateAdvancedByOne =
        result->visibleCandidateIndexAfter == expectedIndex &&
        result->candidateOrdinalAfter != result->candidateOrdinalBefore;
    result->candidateStayedWithinVisibleSet =
        result->visibleCandidateCountAfter ==
            result->visibleCandidateCountBefore &&
        result->visibleCandidateIndexAfter >= 0 &&
        result->visibleCandidateIndexAfter < result->visibleCandidateCountAfter &&
        result->g0299After == result->candidateOrdinalAfter;
    result->noLeaderHandSwap =
        result->leaderHandThingAfter == result->leaderHandThingBefore &&
        result->leaderHandEmptyAfter == result->leaderHandEmptyBefore &&
        result->leaderHandPutCountAfter == result->leaderHandPutCountBefore &&
        result->leaderHandRemoveCountAfter ==
            result->leaderHandRemoveCountBefore;
    result->noSlotWriteOrClear =
        result->slotWriteCountAfter == result->slotWriteCountBefore &&
        result->slotClearCountAfter == result->slotClearCountBefore;
    result->noC159NameRowSideEffect =
        result->c159NameRowDispatchCountAfter ==
            result->c159NameRowDispatchCountBefore &&
        result->c159SetLeaderCountAfter == result->c159SetLeaderCountBefore &&
        result->leaderIndexAfter == result->leaderIndexBefore;
    result->noG0299GuardedSideEffect =
        result->statusBoxDispatchCountAfter ==
            result->statusBoxDispatchCountBefore &&
        result->spellRuneDispatchCountAfter ==
            result->spellRuneDispatchCountBefore &&
        result->saveDispatchCountAfter == result->saveDispatchCountBefore;
    result->c040PanelStillOpen =
        state->c040PanelOpen &&
        state->panelKind ==
            DM1_V1_MIRROR_CANDIDATE_LEFT_CLICK_ROTATION_M568_PANEL_PC34_COMPAT;
    result->rotationViewOnly =
        result->eventDispatchedToMirrorCandidateHandler &&
        result->candidateAdvancedByOne &&
        result->candidateStayedWithinVisibleSet &&
        result->noLeaderHandSwap &&
        result->noSlotWriteOrClear &&
        result->noC159NameRowSideEffect &&
        result->noG0299GuardedSideEffect &&
        result->c040PanelStillOpen;
}

void DM1_V1_MirrorCandidateLeftClickRotation_InitPc34Compat(
    Dm1V1MirrorCandidateLeftClickRotationStatePc34Compat *state)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->contractOnly = 1;
    state->c040PanelOpen = 1;
    state->panelKind =
        DM1_V1_MIRROR_CANDIDATE_LEFT_CLICK_ROTATION_M568_PANEL_PC34_COMPAT;
    state->leaderIndex = kInitialLeaderIndex;
    state->leaderHandEmpty = kInitialLeaderHandEmpty;
    state->leaderHandThingOrdinal =
        DM1_V1_MIRROR_CANDIDATE_LEFT_CLICK_ROTATION_LEADER_HAND_THING_PC34_COMPAT;
    state->visibleCandidateCount =
        DM1_V1_MIRROR_CANDIDATE_LEFT_CLICK_ROTATION_VISIBLE_COUNT_PC34_COMPAT;
    state->visibleCandidateIndex = kInitialCandidateIndex;
    state->visibleCandidateOrdinals[0] = kVisibleOrdinal0;
    state->visibleCandidateOrdinals[1] = kVisibleOrdinal1;
    state->visibleCandidateOrdinals[2] = kVisibleOrdinal2;
    state->visibleCandidateOrdinals[3] = kVisibleOrdinal3;
    state->g0299CandidateChampionOrdinal =
        state->visibleCandidateOrdinals[state->visibleCandidateIndex];
}

int DM1_V1_MirrorCandidateLeftClickRotation_ApplyPc34Compat(
    Dm1V1MirrorCandidateLeftClickRotationStatePc34Compat *state,
    unsigned int mouseButtons,
    Dm1V1MirrorCandidateLeftClickRotationResultPc34Compat *outResult)
{
    Dm1V1MirrorCandidateLeftClickRotationResultPc34Compat localResult;
    Dm1V1MirrorCandidateLeftClickRotationResultPc34Compat *result =
        outResult ? outResult : &localResult;

    if (!state || !state->contractOnly || state->visibleCandidateCount <= 0) {
        if (outResult) {
            memset(outResult, 0, sizeof(*outResult));
            outResult->evidence = &s_evidence;
        }
        return 0;
    }

    capture_before(state, result);
    if ((mouseButtons &
         DM1_V1_MIRROR_CANDIDATE_LEFT_CLICK_ROTATION_MOUSE_LEFT_PC34_COMPAT) &&
        state->c040PanelOpen &&
        state->panelKind ==
            DM1_V1_MIRROR_CANDIDATE_LEFT_CLICK_ROTATION_M568_PANEL_PC34_COMPAT &&
        state->g0299CandidateChampionOrdinal != 0u) {
        ++state->leftClickDispatchCount;
        ++state->mirrorCandidateHandlerCount;
        ++state->rotationDispatchCount;
        state->visibleCandidateIndex =
            (state->visibleCandidateIndex + 1) % state->visibleCandidateCount;
        state->g0299CandidateChampionOrdinal =
            state->visibleCandidateOrdinals[state->visibleCandidateIndex];
        ++state->portraitRedrawCount;
    }
    capture_after(state, result);
    return result->rotationViewOnly;
}

const Dm1V1MirrorCandidateLeftClickRotationEvidencePc34Compat *
DM1_V1_MirrorCandidateLeftClickRotation_EvidencePc34Compat(void)
{
    return &s_evidence;
}
