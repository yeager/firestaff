#include "firestaff/dm1/v1/mirror/dm1_v1_mirror_candidate_scroll_pickup_with_party_rotate_in_progress_pc34_compat.h"

#include <string.h>

/* ReDMCSB source-lock anchors for this contract-only runtime regression:
 * PANEL.C F0344:1895-1944 + F0345:1946-1999 route the panel cell/highlight
 * path used here for the scroll-pickup attempt while C040 remains live.
 * CHAMPION.C F0297:243-268 and F0298:270-298 own leader-hand identity;
 * F0302:662-713 owns slot-box pickup, including C30/C537..C544 chest slots.
 * COMMAND.C F0359:1985-1990 gates M568/C040 clicks; F0361:1709-1813 writes
 * turn commands to the queue; F0380:2045-2156 unlocks pending clicks before
 * turn dispatch and routes C001/C002 to party rotation.
 * MOUSE.C F0077:97-126 + F0078:128-168 anchor the original wheel/click queue
 * bracket (local ReDMCSB source names this area differently for some ports).
 * REVIVE.C F0280:124-132 publishes G0299/C040 only with empty hand and room;
 * F0282:744-806 clears the live candidate after resurrect/reincarnate/cancel.
 * CHAMDRAW.C F0291:498-560, F0292:703-735, and F0296:1185-1252 own slot,
 * state, and changed-object redraws while the candidate panel is live.
 * DEFS.H:277 C040; 810 C30; 3906-3913 C537..C544; 5694 G0299;
 * 5700 G0305; 873/876 M516_CHAMPIONS.
 */

enum {
    kLeaderIndex = 0,
    kInitialPartyDirection = 0,
    kRotatedPartyDirection = 1,
    kPartyChampionCount = 3,
    kCandidateChampionIndex = 3,
    kCandidateOrdinal = 4,
    kCandidateIndex = 2,
    kScrollChestSlotIndex = 2
};

static const Dm1V1MirrorCandidateScrollPickupWithPartyRotateEvidencePc34Compat
    s_evidence = {
        1,
        "PANEL.C F0344:1895-1944 + F0345:1946-1999 panel click/cell highlight route",
        "CHAMPION.C F0297:243-268 put object in leader hand",
        "CHAMPION.C F0298:270-298 remove object from leader hand",
        "CHAMPION.C F0302:662-713 occupied slot and C30/C537..C544 dispatch",
        "COMMAND.C F0359:1985-1990 M568/C040 pending panel click dispatch",
        "COMMAND.C F0361:1709-1813 queue write for C001/C002 turns",
        "COMMAND.C F0380:2045-2156 queue dispatch, pending-click unlock, turn route",
        "MOUSE.C F0077:97-126 + F0078:128-168 wheel/click queue bracket",
        "REVIVE.C F0280:124-132 C040 candidate publish gate",
        "REVIVE.C F0282:744-806 C040 candidate clear/decision path",
        "CHAMDRAW.C F0291:498-560 slot draw and C30 slot source",
        "CHAMDRAW.C F0292:703-735 champion state redraw",
        "CHAMDRAW.C F0296:1185-1252 changed icon redraw and C38 chest sweep",
        "DEFS.H:277 C040; 810 C30; 3906-3913 C537..C544; 5694 G0299; 5700 G0305; 873/876 M516_CHAMPIONS",
        "pass764 fresh contract: scroll pickup during F0380 party rotation, not candidate-internal rotation; excludes pass715/pass718/pass722/pass723/pass727/pass728/pass731/pass732/pass735/pass736/pass737/pass740/pass752/pass753/pass759/pass760"
    };

static unsigned int hash_mix(unsigned int hash, unsigned int value)
{
    hash ^= value;
    hash *= 16777619u;
    return hash;
}

static unsigned int hash_chain(const int chain[], int length)
{
    int i;
    unsigned int hash = 2166136261u;

    hash = hash_mix(hash, (unsigned int)length);
    for (i = 0; i < DM1_V1_MCSPPR_CANDIDATE_CHAIN_CAPACITY_PC34; ++i) {
        hash = hash_mix(hash, (unsigned int)chain[i]);
    }
    return hash;
}

static unsigned int hash_candidate_redraw(
    int candidateOrdinal,
    int candidateIndex,
    unsigned int candidateChainHash,
    int c040Graphic,
    int c040RedrawSerial)
{
    unsigned int hash = 2166136261u;

    hash = hash_mix(hash, (unsigned int)candidateOrdinal);
    hash = hash_mix(hash, (unsigned int)candidateIndex);
    hash = hash_mix(hash, candidateChainHash);
    hash = hash_mix(hash, (unsigned int)c040Graphic);
    hash = hash_mix(hash, (unsigned int)c040RedrawSerial);
    return hash;
}

static int snapshots_match(
    const Dm1V1MirrorCandidateScrollPickupWithPartyRotateSnapshotPc34Compat *a,
    const Dm1V1MirrorCandidateScrollPickupWithPartyRotateSnapshotPc34Compat *b)
{
    if (!a || !b) {
        return 0;
    }
    return a->candidateOrdinal == b->candidateOrdinal &&
           a->candidateIndex == b->candidateIndex &&
           a->candidateChainLength == b->candidateChainLength &&
           a->candidateChainHash == b->candidateChainHash &&
           a->c040PanelOpen == b->c040PanelOpen &&
           a->c040Graphic == b->c040Graphic &&
           a->c040RedrawSerial == b->c040RedrawSerial &&
           a->c040RedrawHash == b->c040RedrawHash &&
           a->f0282DecisionCount == b->f0282DecisionCount &&
           a->candidateInternalRotationCount == b->candidateInternalRotationCount &&
           memcmp(a->candidateChain, b->candidateChain,
                  sizeof(a->candidateChain)) == 0;
}

static int normalize_direction(int direction)
{
    while (direction < 0) {
        direction += 4;
    }
    return direction & 3;
}

void DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_InitPc34Compat(
    Dm1V1MirrorCandidateScrollPickupWithPartyRotateStatePc34Compat *state)
{
    int i;

    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->contractOnly = 1;
    state->partyChampionCount = kPartyChampionCount;
    state->leaderIndex = kLeaderIndex;
    state->partyDirection = kInitialPartyDirection;
    state->candidateChampionIndex = kCandidateChampionIndex;
    state->candidateOrdinal = kCandidateOrdinal;
    state->candidateIndex = kCandidateIndex;
    state->candidateChainLength = 3;
    state->candidateChain[0] = 0x4100;
    state->candidateChain[1] = 0x4101;
    state->candidateChain[2] = 0x4102;
    state->candidateChain[3] = DM1_V1_MCSPPR_THING_NONE_PC34;
    state->candidateChainHash =
        hash_chain(state->candidateChain, state->candidateChainLength);
    state->leaderHandThing = DM1_V1_MCSPPR_THING_NONE_PC34;
    state->leaderHandEmpty = 1;
    for (i = 0; i < DM1_V1_MCSPPR_CHEST_SLOT_COUNT_PC34; ++i) {
        state->chestSlots[i] = (unsigned int)(0x7000 + i);
    }
    state->scrollChestSlotIndex = kScrollChestSlotIndex;
    state->scrollPc34Slot =
        DM1_V1_MCSPPR_C30_CHEST_SLOT_1_PC34 + kScrollChestSlotIndex;
    state->scrollZone =
        DM1_V1_MCSPPR_C537_ZONE_CHEST_SLOT_1_PC34 + kScrollChestSlotIndex;
    state->chestSlots[kScrollChestSlotIndex] =
        DM1_V1_MCSPPR_SCROLL_THING_PC34;
}

void DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_SnapshotPc34Compat(
    const Dm1V1MirrorCandidateScrollPickupWithPartyRotateStatePc34Compat *state,
    Dm1V1MirrorCandidateScrollPickupWithPartyRotateSnapshotPc34Compat *snapshot)
{
    if (!state || !snapshot) {
        return;
    }
    memset(snapshot, 0, sizeof(*snapshot));
    snapshot->candidateOrdinal = state->candidateOrdinal;
    snapshot->candidateIndex = state->candidateIndex;
    memcpy(snapshot->candidateChain, state->candidateChain,
           sizeof(snapshot->candidateChain));
    snapshot->candidateChainLength = state->candidateChainLength;
    snapshot->candidateChainHash = state->candidateChainHash;
    snapshot->c040PanelOpen = state->c040PanelOpen;
    snapshot->c040Graphic = state->c040Graphic;
    snapshot->c040RedrawSerial = state->c040RedrawSerial;
    snapshot->c040RedrawHash = state->c040RedrawHash;
    snapshot->f0282DecisionCount = state->f0282DecisionCount;
    snapshot->candidateInternalRotationCount =
        state->candidateInternalRotationCount;
}

int DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_OpenCandidatePc34Compat(
    Dm1V1MirrorCandidateScrollPickupWithPartyRotateStatePc34Compat *state)
{
    if (!state || !state->contractOnly || !state->leaderHandEmpty ||
        state->partyChampionCount >= DM1_V1_MCSPPR_CHAMPION_COUNT_PC34 ||
        state->c040PanelOpen) {
        return 0;
    }
    state->c040PanelOpen = 1;
    state->c040Graphic = DM1_V1_MCSPPR_C040_GRAPHIC_PC34;
    ++state->c040RedrawSerial;
    state->c040RedrawHash = hash_candidate_redraw(
        state->candidateOrdinal,
        state->candidateIndex,
        state->candidateChainHash,
        state->c040Graphic,
        state->c040RedrawSerial);
    ++state->f0280OpenCount;
    ++state->f0291SlotDrawCount;
    ++state->f0292StateDrawCount;
    ++state->f0296ChangedObjectIconCount;
    return 1;
}

int DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_QueueTurnPc34Compat(
    Dm1V1MirrorCandidateScrollPickupWithPartyRotateStatePc34Compat *state,
    int turnCommand)
{
    if (!state || !state->contractOnly ||
        (turnCommand != DM1_V1_MCSPPR_C001_TURN_LEFT_PC34 &&
         turnCommand != DM1_V1_MCSPPR_C002_TURN_RIGHT_PC34) ||
        state->queuedTurnCommand) {
        return 0;
    }
    state->commandQueueLocked = 1;
    state->queuedTurnCommand = turnCommand;
    ++state->f0361QueueWriteCount;
    state->commandQueueLocked = 0;
    return 1;
}

int DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_BeginF0380PartyRotationPc34Compat(
    Dm1V1MirrorCandidateScrollPickupWithPartyRotateStatePc34Compat *state)
{
    if (!state || !state->contractOnly || !state->queuedTurnCommand ||
        state->f0380DispatchInProgress || state->partyRotationInProgress) {
        return 0;
    }
    state->commandQueueLocked = 1;
    state->f0380DispatchInProgress = 1;
    state->partyRotationInProgress = 1;
    ++state->f0380DispatchCount;
    return 1;
}

int DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_ClickScrollPickupPc34Compat(
    Dm1V1MirrorCandidateScrollPickupWithPartyRotateStatePc34Compat *state)
{
    if (!state || !state->contractOnly || !state->c040PanelOpen) {
        return 0;
    }
    ++state->pickupAttemptCount;
    ++state->f0359PanelClickCount;
    ++state->f0344PanelCellRouteCount;
    ++state->f0345PanelHighlightRouteCount;

    if (state->f0380DispatchInProgress || state->partyRotationInProgress) {
        ++state->ignoredPickupDuringPartyRotationCount;
        return 0;
    }

    if (!state->rotationCompletedBeforePickup ||
        !state->leaderHandEmpty ||
        state->chestSlots[state->scrollChestSlotIndex] !=
            DM1_V1_MCSPPR_SCROLL_THING_PC34) {
        return 0;
    }

    ++state->f0302SlotClickCount;
    ++state->f0297PutCount;
    state->leaderHandThing = state->chestSlots[state->scrollChestSlotIndex];
    state->leaderHandEmpty = 0;
    state->chestSlots[state->scrollChestSlotIndex] =
        DM1_V1_MCSPPR_THING_NONE_PC34;
    ++state->honoredPickupAfterRotationCount;
    ++state->f0291SlotDrawCount;
    ++state->f0292StateDrawCount;
    ++state->f0296ChangedObjectIconCount;
    return 1;
}

int DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_CompletePartyRotationPc34Compat(
    Dm1V1MirrorCandidateScrollPickupWithPartyRotateStatePc34Compat *state)
{
    if (!state || !state->contractOnly || !state->f0380DispatchInProgress ||
        !state->partyRotationInProgress || !state->queuedTurnCommand) {
        return 0;
    }
    if (state->queuedTurnCommand == DM1_V1_MCSPPR_C002_TURN_RIGHT_PC34) {
        state->partyDirection =
            normalize_direction(state->partyDirection + 1);
    } else {
        state->partyDirection =
            normalize_direction(state->partyDirection - 1);
    }
    state->queuedTurnCommand = 0;
    state->commandQueueLocked = 0;
    state->f0380DispatchInProgress = 0;
    state->partyRotationInProgress = 0;
    state->rotationCompletedBeforePickup = 1;
    ++state->f0380RotationCompleteCount;
    return 1;
}

int DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_RunPc34Compat(
    Dm1V1MirrorCandidateScrollPickupWithPartyRotateStatePc34Compat *state,
    Dm1V1MirrorCandidateScrollPickupWithPartyRotateResultPc34Compat *result)
{
    Dm1V1MirrorCandidateScrollPickupWithPartyRotateSnapshotPc34Compat before;
    Dm1V1MirrorCandidateScrollPickupWithPartyRotateSnapshotPc34Compat afterIgnore;
    int opened;
    int queued;
    int began;
    int ignored;
    int completed;
    int honored;

    if (!state || !result) {
        return 0;
    }
    memset(result, 0, sizeof(*result));
    DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_InitPc34Compat(state);
    result->initialized = state->contractOnly;

    opened =
        DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_OpenCandidatePc34Compat(
            state);
    queued =
        DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_QueueTurnPc34Compat(
            state,
            DM1_V1_MCSPPR_C002_TURN_RIGHT_PC34);
    began =
        DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_BeginF0380PartyRotationPc34Compat(
            state);
    DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_SnapshotPc34Compat(
        state,
        &before);
    ignored =
        !DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_ClickScrollPickupPc34Compat(
            state);
    DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_SnapshotPc34Compat(
        state,
        &afterIgnore);
    result->ignoredSnapshotHash =
        DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_HashPc34Compat(
            state,
            NULL);
    completed =
        DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_CompletePartyRotationPc34Compat(
            state);
    honored =
        DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_ClickScrollPickupPc34Compat(
            state);

    result->evidence =
        DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_EvidencePc34Compat();
    result->openedCandidate = opened;
    result->queuedPartyTurn = queued;
    result->beganF0380PartyRotation = began;
    result->ignoredPickupDuringPartyRotation =
        ignored && state->ignoredPickupDuringPartyRotationCount == 1;
    result->candidateIndexPreservedDuringIgnore =
        before.candidateIndex == afterIgnore.candidateIndex;
    result->candidateChainPreservedDuringIgnore =
        snapshots_match(&before, &afterIgnore);
    result->c040RedrawPreservedDuringIgnore =
        before.c040RedrawSerial == afterIgnore.c040RedrawSerial &&
        before.c040RedrawHash == afterIgnore.c040RedrawHash;
    result->candidateNotInternallyRotated =
        state->candidateInternalRotationCount == 0;
    result->noLeaderHandPickupDuringRotation =
        before.f0282DecisionCount == afterIgnore.f0282DecisionCount &&
        state->pickupAttemptCount >= 1 &&
        state->f0297PutCount == (honored ? 1 : 0);
    result->noSlotMutationDuringRotation =
        state->ignoredPickupDuringPartyRotationCount == 1 &&
        state->honoredPickupAfterRotationCount == (honored ? 1 : 0);
    result->rotationCompleted = completed &&
        state->partyDirection == kRotatedPartyDirection;
    result->rotationCompletedBeforePickup =
        state->rotationCompletedBeforePickup &&
        state->honoredPickupAfterRotationCount == 1;
    result->pickupHonoredAfterRotation = honored;
    result->leaderHandReceivedScroll =
        state->leaderHandThing == DM1_V1_MCSPPR_SCROLL_THING_PC34 &&
        !state->leaderHandEmpty;
    result->scrollSlotClearedAfterPickup =
        state->chestSlots[state->scrollChestSlotIndex] ==
        DM1_V1_MCSPPR_THING_NONE_PC34;
    result->candidateStillLiveAfterPickup =
        state->c040PanelOpen &&
        state->candidateOrdinal == kCandidateOrdinal &&
        state->candidateChainHash == before.candidateChainHash;
    result->f0302OnlyAfterRotation =
        completed && state->f0302SlotClickCount == 1;
    result->f0297OnlyAfterRotation =
        completed && state->f0297PutCount == 1;
    result->f0282NeverFired = state->f0282DecisionCount == 0;
    result->contractOnly = state->contractOnly;
    result->noAssetsOrPixelParity = 1;
    result->finalHash =
        DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_HashPc34Compat(
            state,
            result);
    result->ok = result->initialized && result->openedCandidate &&
                 result->queuedPartyTurn && result->beganF0380PartyRotation &&
                 result->ignoredPickupDuringPartyRotation &&
                 result->candidateIndexPreservedDuringIgnore &&
                 result->candidateChainPreservedDuringIgnore &&
                 result->c040RedrawPreservedDuringIgnore &&
                 result->candidateNotInternallyRotated &&
                 result->noLeaderHandPickupDuringRotation &&
                 result->noSlotMutationDuringRotation &&
                 result->rotationCompleted &&
                 result->rotationCompletedBeforePickup &&
                 result->pickupHonoredAfterRotation &&
                 result->leaderHandReceivedScroll &&
                 result->scrollSlotClearedAfterPickup &&
                 result->candidateStillLiveAfterPickup &&
                 result->f0302OnlyAfterRotation &&
                 result->f0297OnlyAfterRotation &&
                 result->f0282NeverFired;
    return result->ok;
}

const Dm1V1MirrorCandidateScrollPickupWithPartyRotateEvidencePc34Compat *
DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_EvidencePc34Compat(void)
{
    return &s_evidence;
}

unsigned int DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_HashPc34Compat(
    const Dm1V1MirrorCandidateScrollPickupWithPartyRotateStatePc34Compat *state,
    const Dm1V1MirrorCandidateScrollPickupWithPartyRotateResultPc34Compat *result)
{
    int i;
    unsigned int hash = 2166136261u;

    if (state) {
        hash = hash_mix(hash, (unsigned int)state->contractOnly);
        hash = hash_mix(hash, (unsigned int)state->partyChampionCount);
        hash = hash_mix(hash, (unsigned int)state->leaderIndex);
        hash = hash_mix(hash, (unsigned int)state->partyDirection);
        hash = hash_mix(hash, (unsigned int)state->candidateOrdinal);
        hash = hash_mix(hash, (unsigned int)state->candidateIndex);
        hash = hash_mix(hash, state->candidateChainHash);
        hash = hash_mix(hash, (unsigned int)state->c040PanelOpen);
        hash = hash_mix(hash, (unsigned int)state->c040Graphic);
        hash = hash_mix(hash, (unsigned int)state->c040RedrawSerial);
        hash = hash_mix(hash, state->c040RedrawHash);
        hash = hash_mix(hash, state->leaderHandThing);
        hash = hash_mix(hash, (unsigned int)state->leaderHandEmpty);
        for (i = 0; i < DM1_V1_MCSPPR_CHEST_SLOT_COUNT_PC34; ++i) {
            hash = hash_mix(hash, state->chestSlots[i]);
        }
        hash = hash_mix(hash, (unsigned int)state->scrollPc34Slot);
        hash = hash_mix(hash, (unsigned int)state->scrollZone);
        hash = hash_mix(hash, (unsigned int)state->f0380DispatchInProgress);
        hash = hash_mix(hash, (unsigned int)state->partyRotationInProgress);
        hash = hash_mix(hash, (unsigned int)state->candidateInternalRotationCount);
        hash = hash_mix(hash, (unsigned int)state->f0297PutCount);
        hash = hash_mix(hash, (unsigned int)state->f0302SlotClickCount);
        hash = hash_mix(hash, (unsigned int)state->f0359PanelClickCount);
        hash = hash_mix(hash, (unsigned int)state->f0361QueueWriteCount);
        hash = hash_mix(hash, (unsigned int)state->f0380DispatchCount);
        hash = hash_mix(hash, (unsigned int)state->f0380RotationCompleteCount);
        hash = hash_mix(hash, (unsigned int)state->f0344PanelCellRouteCount);
        hash = hash_mix(hash, (unsigned int)state->f0345PanelHighlightRouteCount);
        hash = hash_mix(hash, (unsigned int)state->f0280OpenCount);
        hash = hash_mix(hash, (unsigned int)state->f0282DecisionCount);
        hash = hash_mix(hash, (unsigned int)state->ignoredPickupDuringPartyRotationCount);
        hash = hash_mix(hash, (unsigned int)state->honoredPickupAfterRotationCount);
    }
    if (result) {
        hash = hash_mix(hash, (unsigned int)result->ignoredPickupDuringPartyRotation);
        hash = hash_mix(hash, (unsigned int)result->candidateChainPreservedDuringIgnore);
        hash = hash_mix(hash, (unsigned int)result->rotationCompletedBeforePickup);
        hash = hash_mix(hash, (unsigned int)result->pickupHonoredAfterRotation);
        hash = hash_mix(hash, (unsigned int)result->f0282NeverFired);
        hash = hash_mix(hash, (unsigned int)result->ok);
    }
    return hash;
}
