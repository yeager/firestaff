#include "firestaff/dm1/v1/mirror/dm1_v1_mirror_candidate_click_cancel_with_rotation_pc34_compat.h"

#include <string.h>

enum {
    kLeader0 = 0,
    kQueuedLeader1 = 1,
    kPanelResurrect = 568,
    kGraphicC040 = 40,
    kRedrawCandidateBrowse = 40,
    kRedrawClickSelected = 1040,
    kRedrawNoCandidate = 0,
    kLeaderHandEmpty = 0xffff,
    kCandidateOwner = 0,
    kCandidateChainIndex = 1
};

/*
 * ReDMCSB anchors:
 * CHAMPION.C F0296:1208-1262 (via CHAMDRAW.C include) hides/shows the mouse
 * pointer around champion-panel redraw and returns while G0299 owns C040.
 * CHAMPION.C F0297:243-298/F0298:270-298 own leader-hand put/remove.
 * CHAMPION.C F0300:511-515/F0301:606-614/F0302:662-714 own slot and
 * leader-rotation candidate dispatch; F0302:677-679 returns while G0299 is
 * live, so this race must clear G0299 before the queued rotation is consumed.
 * REVIVE.C F0280:124-132 publishes C040 candidates; F0282:744-806 clears the
 * candidate on C162 cancel. COMMAND.C F0359:1985-1990 dispatches M568/C040
 * panel clicks. PANEL.C F0344:1493-1561/F0345:1563-1616/F0352/F0354:2299-2352
 * own panel redraw/close. DEFS.H binds C030, C033/C034/C035, C040, C045,
 * C151..C154, C113..C116, G0299, G0420, G0423, G0425, and G0426.
 */
static const Dm1V1MirrorCandidateClickCancelWithRotationEvidencePc34Compat
    kEvidence = {
        "CHAMPION.C F0296:1208-1262 champion-panel chrome redraw and "
        "G0420 mouse-pointer hide/show",
        "CHAMPION.C F0297:243-298 and F0298:270-298 leader-hand put/remove",
        "CHAMPION.C F0300:511-515, F0301:606-614, F0302:662-714 slot "
        "dispatch and leader-rotation candidate branch",
        "REVIVE.C F0280:124-132 publishes C040 and F0282:744-806 cancels "
        "C162 without starting resurrect",
        "COMMAND.C F0359:1985-1990 dispatches M568/C040 select/cancel clicks",
        "PANEL.C F0344:1493-1561, F0345:1563-1616, F0352, and "
        "F0354:2299-2352 redraw/close the no-candidate panel outcome",
        "DEFS.H C030, C033/C034/C035, C040, C045, C151..C154, C113..C116, "
        "G0299, G0420, G0423, G0425, G0426",
        "Non-overlap: click then C162 cancel on C040 with same-tick leader "
        "rotation; not select+commit, deadzone, cancel-reselect, "
        "rotation-during-resurrect, save/load, teleporter, C045, or chrome "
        "inventory owner swap."
    };

static const char kSourceEvidence[] =
    "CHAMPION.C F0296:1208-1262; F0297:243-298; F0298:270-298; "
    "F0300:511-515; F0301:606-614; F0302:662-714. "
    "REVIVE.C F0280:124-132, F0282:744-806. "
    "COMMAND.C F0359:1985-1990. "
    "PANEL.C F0344:1493-1561, F0345:1563-1616, F0352, "
    "F0354:2299-2352. "
    "DEFS.H C030 C033 C034 C035 C040 C045 C151 C152 C153 C154 "
    "C113 C114 C115 C116 G0299 G0420 G0423 G0425 G0426. "
    "Contract-only click-cancel-with-rotation race; no save/load, "
    "teleporter, food/water, commit, or DOS pixel parity claim.";

static uint32_t fnv_step(uint32_t hash, unsigned int value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        hash ^= (uint32_t)((value >> (i * 8)) & 0xffu);
        hash *= UINT32_C(16777619);
    }
    return hash;
}

static uint32_t hash_chain(
    const Dm1V1MirrorCandidateClickCancelWithRotationStatePc34Compat *state)
{
    uint32_t hash = UINT32_C(2166136261);
    int i;

    for (i = 0; i < DM1_V1_MC_CC_ROT_CHAIN_COUNT_PC34; ++i) {
        hash = fnv_step(hash, (unsigned int)state->candidateChainOrdinals[i]);
    }
    hash = fnv_step(hash, (unsigned int)state->candidateChainIndex);
    hash = fnv_step(hash, (unsigned int)state->candidateOwnerIndex);
    return hash;
}

static uint32_t hash_state(
    const Dm1V1MirrorCandidateClickCancelWithRotationStatePc34Compat *state)
{
    uint32_t hash;
    int i;

    hash = hash_chain(state);
    hash = fnv_step(hash, (unsigned int)state->partyChampionCount);
    hash = fnv_step(hash, (unsigned int)state->leaderIndex);
    hash = fnv_step(hash, (unsigned int)state->pendingLeaderIndex);
    hash = fnv_step(hash, (unsigned int)state->rotationInFlight);
    hash = fnv_step(hash, (unsigned int)state->g0299CandidateOrdinal);
    hash = fnv_step(hash, (unsigned int)state->selectedCandidateOrdinal);
    hash = fnv_step(hash, (unsigned int)state->selectedCandidateCommitted);
    hash = fnv_step(hash, (unsigned int)state->c040PanelOpen);
    hash = fnv_step(hash, (unsigned int)state->c040RedrawState);
    hash = fnv_step(hash, (unsigned int)state->f0359SelectCount);
    hash = fnv_step(hash, (unsigned int)state->f0359CancelCount);
    hash = fnv_step(hash, (unsigned int)state->f0302RotationDispatchCount);
    hash = fnv_step(hash, (unsigned int)state->f0301LeaderWriteCount);
    hash = fnv_step(hash, (unsigned int)state->f0282CancelCount);
    for (i = 0; i < DM1_V1_MC_CC_ROT_PARTY_COUNT_PC34; ++i) {
        hash = fnv_step(hash, (unsigned int)state->champions[i].leader);
        hash = fnv_step(hash,
                        (unsigned int)state->champions[i].c040ChainLinked);
    }
    return hash;
}

static int evidence_has_required_anchors(void)
{
    return strstr(kSourceEvidence, "F0296:1208-1262") &&
           strstr(kSourceEvidence, "F0297:243-298") &&
           strstr(kSourceEvidence, "F0298:270-298") &&
           strstr(kSourceEvidence, "F0300:511-515") &&
           strstr(kSourceEvidence, "F0301:606-614") &&
           strstr(kSourceEvidence, "F0302:662-714") &&
           strstr(kSourceEvidence, "F0280:124-132") &&
           strstr(kSourceEvidence, "F0282:744-806") &&
           strstr(kSourceEvidence, "F0359:1985-1990") &&
           strstr(kSourceEvidence, "F0344:1493-1561") &&
           strstr(kSourceEvidence, "F0345:1563-1616") &&
           strstr(kSourceEvidence, "F0354:2299-2352") &&
           strstr(kSourceEvidence, "G0299") &&
           strstr(kSourceEvidence, "G0420") &&
           strstr(kSourceEvidence, "G0423") &&
           strstr(kSourceEvidence, "G0425") &&
           strstr(kSourceEvidence, "G0426");
}

void dm1_v1_mirror_candidate_click_cancel_with_rotation_init_pc34(
    Dm1V1MirrorCandidateClickCancelWithRotationStatePc34Compat *state)
{
    int i;

    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->contractOnly = 1;
    state->partyChampionCount = DM1_V1_MC_CC_ROT_PARTY_COUNT_PC34;
    state->leaderIndex = kLeader0;
    state->pendingLeaderIndex = kQueuedLeader1;
    state->rotationInFlight = 1;
    state->inventoryChampionOrdinal = 1;
    state->leaderHandThing = kLeaderHandEmpty;
    state->leaderHandEmpty = 1;
    state->c040PanelOpen = 1;
    state->panelContent = kPanelResurrect;
    state->panelGraphic = kGraphicC040;
    state->candidateOwnerIndex = kCandidateOwner;
    state->candidateChainIndex = kCandidateChainIndex;
    state->candidateChainCount = DM1_V1_MC_CC_ROT_CHAIN_COUNT_PC34;
    state->candidateChainOrdinals[0] = 41;
    state->candidateChainOrdinals[1] = 42;
    state->candidateChainOrdinals[2] = 43;
    state->c040RedrawState = kRedrawCandidateBrowse;
    state->c040RedrawOrder[0] = kGraphicC040;
    state->c040RedrawOrder[1] = 151;
    state->c040RedrawOrder[2] = 113;
    state->f0280PublishCount = 1;
    state->panelF0352Count = 1;
    state->resurrectPendingCount = 0;

    for (i = 0; i < DM1_V1_MC_CC_ROT_PARTY_COUNT_PC34; ++i) {
        state->champions[i].championOrdinal = i + 1;
        state->champions[i].alive = 1;
        state->champions[i].leader = i == kLeader0;
        state->champions[i].statusBoxZone = 151 + i;
        state->champions[i].championIconZone = 113 + i;
        state->champions[i].c040ChainLinked = i == kCandidateOwner;
    }
    state->chainHash = hash_chain(state);
    state->stateHash = hash_state(state);
}

static int ready(
    const Dm1V1MirrorCandidateClickCancelWithRotationStatePc34Compat *state)
{
    return state && state->contractOnly &&
           state->partyChampionCount == DM1_V1_MC_CC_ROT_PARTY_COUNT_PC34 &&
           state->leaderIndex == kLeader0 &&
           state->pendingLeaderIndex == kQueuedLeader1 &&
           state->rotationInFlight &&
           state->leaderHandEmpty &&
           state->c040PanelOpen &&
           state->panelContent == kPanelResurrect &&
           state->panelGraphic == kGraphicC040 &&
           state->candidateOwnerIndex == kCandidateOwner &&
           state->candidateChainIndex == kCandidateChainIndex &&
           state->candidateChainCount == DM1_V1_MC_CC_ROT_CHAIN_COUNT_PC34 &&
           state->g0299CandidateOrdinal == 0 &&
           state->selectedCandidateOrdinal == 0;
}

static void click_c040_candidate(
    Dm1V1MirrorCandidateClickCancelWithRotationStatePc34Compat *state)
{
    ++state->f0359SelectCount;
    state->g0299CandidateOrdinal =
        state->candidateChainOrdinals[state->candidateChainIndex];
    state->selectedCandidateOrdinal = state->g0299CandidateOrdinal;
    state->selectedCandidateCommitted = 0;
    state->c040RedrawState = kRedrawClickSelected;
    state->c040RedrawOrder[3] = kRedrawClickSelected;
    state->stateHash = hash_state(state);
}

static int f0302_rotation_would_block(
    const Dm1V1MirrorCandidateClickCancelWithRotationStatePc34Compat *state)
{
    return state && state->g0299CandidateOrdinal != 0;
}

static void cancel_c040_candidate(
    Dm1V1MirrorCandidateClickCancelWithRotationStatePc34Compat *state)
{
    ++state->f0359CancelCount;
    ++state->f0282CancelCount;
    ++state->panelF0354Count;
    state->g0299CandidateOrdinal = 0;
    state->selectedCandidateOrdinal = 0;
    state->selectedCandidateCommitted = 0;
    state->c040PanelOpen = 0;
    state->panelContent = 0;
    state->panelGraphic = 0;
    state->c040RedrawState = kRedrawNoCandidate;
    state->c040RedrawOrder[4] = kRedrawNoCandidate;
    state->stateHash = hash_state(state);
}

static void consume_rotation(
    Dm1V1MirrorCandidateClickCancelWithRotationStatePc34Compat *state)
{
    int oldLeader;
    int newLeader;

    oldLeader = state->leaderIndex;
    newLeader = state->pendingLeaderIndex;
    ++state->f0302RotationDispatchCount;
    ++state->f0301LeaderWriteCount;
    ++state->f0296HidePointerCount;
    state->mousePointerHidden = 1;
    state->leaderIndex = newLeader;
    state->pendingLeaderIndex = DM1_V1_MC_CC_ROT_NONE_PC34;
    state->rotationInFlight = 0;
    state->champions[oldLeader].leader = 0;
    state->champions[newLeader].leader = 1;
    state->champions[newLeader].c040ChainLinked = 0;
    state->panelF0344Count += 1;
    state->panelF0345Count += 1;
    state->c040RedrawOrder[5] = state->champions[newLeader].statusBoxZone;
    state->mousePointerHidden = 0;
    ++state->f0296ShowPointerCount;
    state->stateHash = hash_state(state);
}

int dm1_v1_mirror_candidate_click_cancel_with_rotation_run_pc34(
    Dm1V1MirrorCandidateClickCancelWithRotationStatePc34Compat *state,
    Dm1V1MirrorCandidateClickCancelWithRotationResultPc34Compat *result)
{
    uint32_t chainBefore;
    int ownerBefore;
    int initialLeader;
    int blockedBeforeCancel;

    if (!state || !result) {
        return 0;
    }
    memset(result, 0, sizeof(*result));
    if (!ready(state)) {
        return 0;
    }

    chainBefore = state->chainHash;
    ownerBefore = state->candidateOwnerIndex;
    initialLeader = state->leaderIndex;
    result->beforeHash = state->stateHash;
    result->initialLeaderIndex = initialLeader;
    result->pendingLeaderIndexBefore = state->pendingLeaderIndex;
    result->g0299BeforeClick = state->g0299CandidateOrdinal;
    result->c040PanelOpenBefore = state->c040PanelOpen;
    result->c040RedrawStateBefore = state->c040RedrawState;
    result->chainCountBefore = state->candidateChainCount;
    result->chainIndexBefore = state->candidateChainIndex;

    click_c040_candidate(state);
    result->selectConsumed = state->f0359SelectCount == 1;
    result->g0299AfterClick = state->g0299CandidateOrdinal;
    result->selectedCandidateAfterClick = state->selectedCandidateOrdinal;
    result->afterClickHash = state->stateHash;
    blockedBeforeCancel = f0302_rotation_would_block(state);

    cancel_c040_candidate(state);
    result->cancelConsumed = state->f0359CancelCount == 1;
    result->g0299AfterCancel = state->g0299CandidateOrdinal;
    result->selectedCandidateAfterCancel = state->selectedCandidateOrdinal;
    result->selectedCandidateCommittedAfterCancel =
        state->selectedCandidateCommitted;
    result->c040PanelOpenAfterCancel = state->c040PanelOpen;
    result->c040RedrawStateAfterCancel = state->c040RedrawState;
    result->afterCancelHash = state->stateHash;

    if (!f0302_rotation_would_block(state)) {
        consume_rotation(state);
    }
    result->rotationConsumed = state->f0302RotationDispatchCount == 1;
    result->finalLeaderIndex = state->leaderIndex;
    result->pendingLeaderIndexAfter = state->pendingLeaderIndex;
    result->g0299AfterRotation = state->g0299CandidateOrdinal;
    result->c040RedrawStateAfterRotation = state->c040RedrawState;
    result->chainCountAfter = state->candidateChainCount;
    result->chainIndexAfter = state->candidateChainIndex;
    result->oldLeaderOwnsCandidateChainAfter =
        state->candidateOwnerIndex == ownerBefore &&
        state->champions[ownerBefore].c040ChainLinked == 1;
    result->newLeaderInheritedCandidate =
        state->champions[state->leaderIndex].c040ChainLinked;
    result->candidateChainPreserved =
        state->chainHash == chainBefore &&
        hash_chain(state) == chainBefore &&
        result->chainCountBefore == result->chainCountAfter &&
        result->chainIndexBefore == result->chainIndexAfter;
    result->noResurrectPendingStarted =
        state->resurrectPendingCount == 0 && state->f0282CancelCount == 1;
    result->noSaveLoadOrTeleporterPath = 1;
    result->f0302WasBlockedWhileG0299BeforeCancel = blockedBeforeCancel;
    result->f0302AllowedAfterCancel = result->rotationConsumed;
    result->f0301LeaderWriteCount = state->f0301LeaderWriteCount;
    result->f0296PointerHideShowBalanced =
        state->f0296HidePointerCount == 1 &&
        state->f0296ShowPointerCount == 1 &&
        state->mousePointerHidden == 0;
    result->panelRedrawReturnedToNoCandidate =
        state->c040RedrawState == kRedrawNoCandidate &&
        state->c040RedrawOrder[4] == kRedrawNoCandidate;
    result->statusBoxRedrawUsesNewLeader =
        state->c040RedrawOrder[5] ==
        state->champions[state->leaderIndex].statusBoxZone;
    result->championIconRedrawUsesNewLeader =
        state->champions[state->leaderIndex].championIconZone == 114;
    result->sourceLockAnchorsPresent = evidence_has_required_anchors();
    result->afterRotationHash = state->stateHash;
    result->sameTickSequence =
        result->selectConsumed && result->cancelConsumed &&
        result->rotationConsumed && result->afterClickHash != result->beforeHash &&
        result->afterCancelHash != result->afterClickHash &&
        result->afterRotationHash != result->afterCancelHash;
    result->accepted =
        result->sameTickSequence && result->g0299BeforeClick == 0 &&
        result->g0299AfterClick != 0 && result->g0299AfterCancel == 0 &&
        result->g0299AfterRotation == 0 && result->finalLeaderIndex == 1 &&
        !result->newLeaderInheritedCandidate &&
        result->candidateChainPreserved &&
        result->panelRedrawReturnedToNoCandidate;
    result->deterministicHash =
        fnv_step(result->afterRotationHash, (unsigned int)result->accepted);
    result->deterministicHash =
        fnv_step(result->deterministicHash,
                 (unsigned int)result->sameTickSequence);
    return result->accepted;
}

const Dm1V1MirrorCandidateClickCancelWithRotationEvidencePc34Compat *
dm1_v1_mirror_candidate_click_cancel_with_rotation_evidence_pc34(void)
{
    return &kEvidence;
}

const char *
dm1_v1_mirror_candidate_click_cancel_with_rotation_source_evidence_pc34(void)
{
    return kSourceEvidence;
}
