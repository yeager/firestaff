#include "firestaff/dm1/v1/mirror_candidate/c545_accept_during_rotation_pc34_compat.h"

#include <string.h>

enum {
    kCommandAcceptResurrect = 160,
    kCommandCancel = 162,
    kCommandSetLeaderChampion1 = 17,
    kPanelFoodWater = 565,
    kPanelResurrectReincarnate = 568,
    kGraphicC040ResurrectReincarnate = 40,
    kGraphicC045ObjectIcons096To127 = 45,
    kZoneC545Mouth = 545,
    kOldLeaderIndex = 0,
    kNewLeaderIndex = 1,
    kCandidateOrdinal = 3,
    kInventoryChampionOrdinal = 1,
    kC040CandidateIndex = 2,
    kTraceInit = 200,
    kTraceQueueAccept = 201,
    kTraceQueueClose = 202,
    kTraceQueueRotation = 203,
    kTraceRejectC040 = 204,
    kTraceCloseC040 = 205,
    kTraceRejectRotation = 206,
    kTraceDrainRotation = 207,
    kTraceAccept = 208,
    kTraceStable = 209
};

/*
 * ReDMCSB anchors:
 * REVIVE.C F0280:124-132 publishes G0299 only after the leader-hand and party
 * count gate; REVIVE.C F0282:744-806 clears G0299 and removes the accepted
 * candidate chain. PANEL.C F0344:1493-1561/F0345:1563-1617/F0354:2299-2352
 * are food/water context only, while PANEL.C F0351:1965-2109,
 * F0352:2111-2160, and F0353:2162-2193 pin the C545-adjacent panel press
 * redraw/restore surface. CHAMPION.C F0297/F0298:243-298 own leader-hand
 * lifetime and F0301/F0302:606-714 own C30+ slot dispatch. COMMAND.C
 * F0359:1452-1662 queues clicks, F0378:1956-1993 routes panel clicks,
 * F0361:1709-1813 writes keyboard/wheel-like commands, and F0380:2045-2178
 * drains exactly one command before the next.
 */
static const char s_source_evidence[] =
    "REVIVE.C F0280:124-132 publishes the candidate; F0282:744-806 clears "
    "G0299 and removes the accepted candidate chain. PANEL.C "
    "F0344:1493-1561, F0345:1563-1617, F0354:2299-2352 food/water context; "
    "PANEL.C F0351:1965-2109, F0352:2111-2160, F0353:2162-2193 C545 panel "
    "press/redraw/restore anchors. CHAMPION.C F0297/F0298:243-298 leader "
    "hand; F0301/F0302:606-714 C30+ slot dispatch. COMMAND.C "
    "F0359:1452-1662 queue, F0378:1956-1993 panel route, F0361:1709-1813 "
    "keyboard/wheel-like queue write, COMMAND.C F0380:2045-2178 drain. "
    "DEFS.H:338-340 "
    "C160..C162, 778-810 C10/C30, 1874-1878 C38/M070, 2078-2088 "
    "C10_COLOR_FLESH, 2200/2205 C040/C045, 2999-3008 M565/M568, "
    "3906-3914 C537..C545, 5694 G0299.";

static const Dm1V1MirrorCandidateC545AcceptDuringRotationEvidencePc34
    s_evidence = {
        "ReDMCSB REVIVE.C F0280:124-132 candidate publication gate",
        "ReDMCSB REVIVE.C F0282:744-806 accept clear and candidate-chain removal",
        "ReDMCSB PANEL.C F0344:1493-1561, F0345:1563-1617, F0354:2299-2352 food/water context only",
        "ReDMCSB PANEL.C F0351:1965-2109, F0352:2111-2160, F0353:2162-2193 C545 panel press/redraw/restore anchors",
        "ReDMCSB CHAMPION.C F0297/F0298:243-298 leader hand lifetime",
        "ReDMCSB CHAMPION.C F0301/F0302:606-714 C30+ slot dispatch",
        "ReDMCSB COMMAND.C F0359:1452-1662 click queue",
        "ReDMCSB COMMAND.C F0378:1956-1993 panel route",
        "ReDMCSB COMMAND.C F0361:1709-1813 keyboard/wheel-like queue write",
        "ReDMCSB COMMAND.C F0380:2045-2178 drain",
        "ReDMCSB DEFS.H C160..C162, C10_SLOT_NECK, C30, C38/M070, C10_COLOR_FLESH, C040/C045, M565/M568, C537..C545, G0299",
        "Non-overlap marker: pass776 covers C545 resurrect-accept while a C040 close and leader rotation are both pending. It is not pass768 C040 panel browse pickup-rotate, pass772 C045 food/water accept cross-rotation, C045 close after non-candidate transition, C045 food/water close no-candidate, C040 chrome inventory-owner swap, or mirror-candidate click-cancel with rotation."
    };

static uint32_t hash_step(uint32_t hash, unsigned int value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        hash ^= (uint32_t)((value >> (i * 8)) & 0xffu);
        hash *= UINT32_C(16777619);
    }
    return hash;
}

static void copy_ints(int dst[], const int src[], int count)
{
    int i;

    for (i = 0; i < count; ++i) {
        dst[i] = src[i];
    }
}

static int int_arrays_equal(const int a[], const int b[], int count)
{
    int i;

    for (i = 0; i < count; ++i) {
        if (a[i] != b[i]) {
            return 0;
        }
    }
    return 1;
}

static uint32_t hash_state(
    const Dm1V1MirrorCandidateC545AcceptDuringRotationStatePc34 *state)
{
    uint32_t hash = UINT32_C(2166136261);
    int i;

    hash = hash_step(hash, (unsigned int)state->contractOnly);
    hash = hash_step(hash, (unsigned int)state->leaderIndex);
    hash = hash_step(hash, (unsigned int)state->queuedLeaderIndex);
    hash = hash_step(hash, (unsigned int)state->g0299CandidateOrdinal);
    hash = hash_step(hash, (unsigned int)state->c040CandidateIndex);
    hash = hash_step(hash, (unsigned int)state->c045CandidateOpen);
    hash = hash_step(hash, (unsigned int)state->c040PanelOpen);
    hash = hash_step(hash, (unsigned int)state->c040CloseCompleted);
    hash = hash_step(hash, (unsigned int)state->leaderRotationQueued);
    hash = hash_step(hash, (unsigned int)state->leaderRotationDrained);
    hash = hash_step(hash, (unsigned int)state->panelContent);
    hash = hash_step(hash, (unsigned int)state->panelGraphic);
    hash = hash_step(hash, (unsigned int)state->c545Zone);
    hash = hash_step(hash, state->c545PanelPixel);
    hash = hash_step(hash, state->c040RedrawState);
    hash = hash_step(hash, (unsigned int)state->commandQueueDepth);
    hash = hash_step(hash, (unsigned int)state->f0282AcceptClearCount);
    hash = hash_step(hash, (unsigned int)state->f0368SetLeaderCount);
    for (i = 0; i < DM1_V1_MC_C545_ACCEPT_ROTATE_CHAIN_COUNT_PC34; ++i) {
        hash = hash_step(hash, (unsigned int)state->candidateChainOrdinals[i]);
    }
    for (i = 0; i < DM1_V1_MC_C545_ACCEPT_ROTATE_PARTY_COUNT_PC34; ++i) {
        hash = hash_step(hash, (unsigned int)state->championChainOrdinals[i]);
        hash = hash_step(hash, (unsigned int)state->champions[i].ordinal);
        hash = hash_step(hash, (unsigned int)state->champions[i].leader);
        hash = hash_step(hash, (unsigned int)state->champions[i].chainLinked);
        hash = hash_step(hash, (unsigned int)state->champions[i].handThing);
    }
    for (i = 0; i < DM1_V1_MC_C545_ACCEPT_ROTATE_TRACE_COUNT_PC34; ++i) {
        hash = hash_step(hash, (unsigned int)state->trace[i]);
    }
    return hash;
}

static int source_anchors_present(void)
{
    return strstr(s_source_evidence, "REVIVE.C F0280:124-132") != NULL &&
           strstr(s_source_evidence, "F0282:744-806") != NULL &&
           strstr(s_source_evidence, "PANEL.C F0351:1965-2109") != NULL &&
           strstr(s_source_evidence, "F0352:2111-2160") != NULL &&
           strstr(s_source_evidence, "F0353:2162-2193") != NULL &&
           strstr(s_source_evidence, "CHAMPION.C F0297/F0298:243-298") !=
               NULL &&
           strstr(s_source_evidence, "F0301/F0302:606-714") != NULL &&
           strstr(s_source_evidence, "COMMAND.C F0359:1452-1662") != NULL &&
           strstr(s_source_evidence, "F0378:1956-1993") != NULL &&
           strstr(s_source_evidence, "F0361:1709-1813") != NULL &&
           strstr(s_source_evidence, "F0380:2045-2178") != NULL &&
           strstr(s_source_evidence, "3906-3914 C537..C545") != NULL &&
           strstr(s_source_evidence, "5694 G0299") != NULL;
}

void dm1_v1_mirror_candidate_c545_accept_during_rotation_init_pc34(
    Dm1V1MirrorCandidateC545AcceptDuringRotationStatePc34 *state)
{
    int i;

    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->contractOnly = 1;
    state->noDosPixelParityClaim = 1;
    state->partyChampionCount = 3;
    state->leaderIndex = kOldLeaderIndex;
    state->queuedLeaderIndex = kNewLeaderIndex;
    state->inventoryChampionOrdinal = kInventoryChampionOrdinal;
    state->candidateChampionOrdinal = kCandidateOrdinal;
    state->c040CandidateIndex = kC040CandidateIndex;
    state->candidateChainOrdinals[0] = kCandidateOrdinal;
    state->candidateChainOrdinals[1] = 4;
    state->championChainOrdinals[0] = 1;
    state->championChainOrdinals[1] = 2;
    state->championChainOrdinals[2] = kCandidateOrdinal;
    state->g0299CandidateOrdinal = kCandidateOrdinal;
    state->c045CandidateOpen = 1;
    state->c040PanelOpen = 1;
    state->c040CloseQueued = 1;
    state->leaderRotationQueued = 1;
    state->panelContent = kPanelResurrectReincarnate;
    state->panelGraphic = kGraphicC040ResurrectReincarnate;
    state->c545Zone = kZoneC545Mouth;
    state->c545PanelPixel = UINT32_C(0x00c545a5);
    state->c040RedrawState = UINT32_C(0x040c5450);
    state->c545AcceptCommand = kCommandAcceptResurrect;
    state->f0280PublishCount = 1;
    state->f0344FoodWaterReadCount = 2;
    state->f0345FoodWaterDrawCount = 1;
    state->f0351PanelDrawCount = 1;
    state->f0352PanelPressCount = 1;
    state->f0353PanelRestoreCount = 1;
    state->f0359QueueWriteCount = 3;
    state->f0361WheelQueueWriteCount = 1;
    state->commandQueueDepth = 3;
    state->trace[0] = kTraceInit;
    state->trace[1] = kTraceQueueAccept;
    state->trace[2] = kTraceQueueClose;
    state->trace[3] = kTraceQueueRotation;
    for (i = 0; i < DM1_V1_MC_C545_ACCEPT_ROTATE_PARTY_COUNT_PC34; ++i) {
        state->champions[i].ordinal = i + 1;
        state->champions[i].alive = i < state->partyChampionCount;
        state->champions[i].chainLinked = i < state->partyChampionCount;
        state->champions[i].handThing =
            DM1_V1_MC_C545_ACCEPT_ROTATE_NONE_PC34;
    }
    state->champions[kOldLeaderIndex].leader = 1;
    state->champions[kOldLeaderIndex].handThing = 0x6001u;
    state->champions[kNewLeaderIndex].handThing = 0x7002u;
    state->beforeHash = hash_state(state);
}

static int ready(
    const Dm1V1MirrorCandidateC545AcceptDuringRotationStatePc34 *state)
{
    return state && state->contractOnly && state->noDosPixelParityClaim &&
           state->partyChampionCount == 3 &&
           state->leaderIndex == kOldLeaderIndex &&
           state->queuedLeaderIndex == kNewLeaderIndex &&
           state->candidateChampionOrdinal == kCandidateOrdinal &&
           state->c040CandidateIndex == kC040CandidateIndex &&
           state->g0299CandidateOrdinal == kCandidateOrdinal &&
           state->candidateChainOrdinals[0] == kCandidateOrdinal &&
           state->c045CandidateOpen && state->c040PanelOpen &&
           state->c040CloseQueued && state->leaderRotationQueued &&
           state->panelContent == kPanelResurrectReincarnate &&
           state->panelGraphic == kGraphicC040ResurrectReincarnate &&
           state->c545Zone == kZoneC545Mouth &&
           state->c545AcceptCommand == kCommandAcceptResurrect &&
           state->commandQueueDepth == 3;
}

static int attempt_c545_accept(
    Dm1V1MirrorCandidateC545AcceptDuringRotationStatePc34 *state)
{
    if (!state || state->g0299CandidateOrdinal != kCandidateOrdinal ||
        !state->c045CandidateOpen ||
        state->c545AcceptCommand != kCommandAcceptResurrect ||
        state->c545Zone != kZoneC545Mouth) {
        return 0;
    }

    ++state->f0378PanelRouteCount;
    if (state->c040PanelOpen || !state->c040CloseCompleted) {
        ++state->c040GateRejectCount;
        state->trace[4] = kTraceRejectC040;
        return 0;
    }
    if (state->leaderRotationQueued || !state->leaderRotationDrained) {
        ++state->rotationGateRejectCount;
        state->trace[6] = kTraceRejectRotation;
        return 0;
    }

    ++state->f0380DrainCount;
    ++state->f0282AcceptClearCount;
    ++state->f0302SlotDispatchCount;
    ++state->f0298LeaderHandCount;
    state->g0299CandidateOrdinal = 0;
    state->candidateSensorDisabled = 1;
    state->candidateChainOrdinals[0] = state->candidateChainOrdinals[1];
    state->candidateChainOrdinals[1] = 0;
    state->candidateRemovedFromChain =
        state->candidateChainOrdinals[0] != kCandidateOrdinal;
    state->c045CandidateOpen = 0;
    state->panelContent = 0;
    state->panelGraphic = 0;
    state->acceptedAfterBothGates = 1;
    --state->commandQueueDepth;
    state->trace[8] = kTraceAccept;
    state->trace[9] = kTraceStable;
    return 1;
}

static int close_c040(
    Dm1V1MirrorCandidateC545AcceptDuringRotationStatePc34 *state)
{
    if (!state || !state->c040PanelOpen || !state->c040CloseQueued) {
        return 0;
    }
    ++state->f0380DrainCount;
    state->c040PanelOpen = 0;
    state->c040CloseCompleted = 1;
    state->c040CloseQueued = 0;
    --state->commandQueueDepth;
    state->trace[5] = kTraceCloseC040;
    return 1;
}

static int drain_rotation(
    Dm1V1MirrorCandidateC545AcceptDuringRotationStatePc34 *state)
{
    int oldLeader;

    if (!state || !state->leaderRotationQueued || !state->c040CloseCompleted) {
        return 0;
    }
    ++state->f0380DrainCount;
    ++state->f0368SetLeaderCount;
    oldLeader = state->leaderIndex;
    state->champions[oldLeader].leader = 0;
    state->leaderIndex = state->queuedLeaderIndex;
    state->champions[state->leaderIndex].leader = 1;
    state->leaderRotationQueued = 0;
    state->leaderRotationDrained = 1;
    --state->commandQueueDepth;
    state->trace[7] = kTraceDrainRotation;
    return 1;
}

static int guard_rejects(
    const Dm1V1MirrorCandidateC545AcceptDuringRotationStatePc34 *base,
    int kind)
{
    Dm1V1MirrorCandidateC545AcceptDuringRotationStatePc34 probe = *base;
    Dm1V1MirrorCandidateC545AcceptDuringRotationResultPc34 result;

    if (kind == 0) {
        probe.contractOnly = 0;
    } else if (kind == 1) {
        probe.g0299CandidateOrdinal = 0;
    } else if (kind == 2) {
        probe.panelContent = kPanelFoodWater;
    } else if (kind == 3) {
        probe.leaderRotationQueued = 0;
        probe.queuedLeaderIndex = kOldLeaderIndex;
    } else {
        probe.c040CloseQueued = 0;
    }
    return dm1_v1_mirror_candidate_c545_accept_during_rotation_run_pc34(
               &probe, &result) == 0;
}

int dm1_v1_mirror_candidate_c545_accept_during_rotation_run_pc34(
    Dm1V1MirrorCandidateC545AcceptDuringRotationStatePc34 *state,
    Dm1V1MirrorCandidateC545AcceptDuringRotationResultPc34 *result)
{
    Dm1V1MirrorCandidateC545AcceptDuringRotationStatePc34 base;
    int rejectedC040;
    int closed;
    int rejectedRotation;
    int rotated;
    int accepted;

    if (!state) {
        return 0;
    }
    if (!result) {
        return 0;
    }
    memset(result, 0, sizeof(*result));
    if (!ready(state)) {
        return 0;
    }

    base = *state;
    result->leaderBefore = state->leaderIndex;
    result->g0299Before = state->g0299CandidateOrdinal;
    result->c040CandidateIndexBefore = state->c040CandidateIndex;
    result->c040RedrawStateBefore = state->c040RedrawState;
    result->c545PanelPixelBefore = state->c545PanelPixel;
    copy_ints(result->candidateChainBefore, state->candidateChainOrdinals,
              DM1_V1_MC_C545_ACCEPT_ROTATE_CHAIN_COUNT_PC34);

    rejectedC040 = !attempt_c545_accept(state);
    result->g0299AfterRejectedC040 = state->g0299CandidateOrdinal;
    result->c040CandidateIndexAfterRejectedC040 = state->c040CandidateIndex;
    result->c040RedrawStateAfterRejectedC040 = state->c040RedrawState;
    result->c545PanelPixelAfterRejectedC040 = state->c545PanelPixel;
    copy_ints(result->candidateChainAfterRejectedC040,
              state->candidateChainOrdinals,
              DM1_V1_MC_C545_ACCEPT_ROTATE_CHAIN_COUNT_PC34);
    state->afterRejectedC040Hash = hash_state(state);

    closed = close_c040(state);
    rejectedRotation = !attempt_c545_accept(state);
    result->g0299AfterRejectedRotation = state->g0299CandidateOrdinal;
    result->c040CandidateIndexAfterRejectedRotation = state->c040CandidateIndex;
    result->c040RedrawStateAfterRejectedRotation = state->c040RedrawState;
    result->c545PanelPixelAfterRejectedRotation = state->c545PanelPixel;
    copy_ints(result->candidateChainAfterRejectedRotation,
              state->candidateChainOrdinals,
              DM1_V1_MC_C545_ACCEPT_ROTATE_CHAIN_COUNT_PC34);
    state->afterCloseRejectedRotationHash = hash_state(state);

    rotated = drain_rotation(state);
    accepted = attempt_c545_accept(state);
    state->afterAcceptHash = hash_state(state);

    result->leaderAfter = state->leaderIndex;
    result->g0299AfterAccept = state->g0299CandidateOrdinal;
    copy_ints(result->candidateChainAfterAccept, state->candidateChainOrdinals,
              DM1_V1_MC_C545_ACCEPT_ROTATE_CHAIN_COUNT_PC34);
    copy_ints(result->trace, state->trace,
              DM1_V1_MC_C545_ACCEPT_ROTATE_TRACE_COUNT_PC34);
    result->beforeHash = base.beforeHash;
    result->afterRejectedC040Hash = state->afterRejectedC040Hash;
    result->afterCloseRejectedRotationHash =
        state->afterCloseRejectedRotationHash;
    result->afterAcceptHash = state->afterAcceptHash;

    result->c545AcceptRoute =
        base.c545Zone == kZoneC545Mouth &&
        base.c545AcceptCommand == kCommandAcceptResurrect &&
        base.panelContent == kPanelResurrectReincarnate;
    result->c040GateRequired =
        rejectedC040 && state->c040GateRejectCount == 1 &&
        base.c040PanelOpen && base.c040CloseQueued;
    result->rotationGateRequired =
        rejectedRotation && state->rotationGateRejectCount == 1 &&
        closed && state->c040CloseCompleted;
    result->rejectedBeforeC040Close =
        rejectedC040 && base.c040PanelOpen && state->f0282AcceptClearCount == 1;
    result->rejectedBeforeRotationDrain =
        rejectedRotation && result->g0299AfterRejectedRotation ==
                                kCandidateOrdinal;
    result->g0299PreservedBeforeGates =
        result->g0299AfterRejectedC040 == kCandidateOrdinal &&
        result->g0299AfterRejectedRotation == kCandidateOrdinal;
    result->c040CandidateIndexPreserved =
        result->c040CandidateIndexAfterRejectedC040 == kC040CandidateIndex &&
        result->c040CandidateIndexAfterRejectedRotation == kC040CandidateIndex;
    result->c040RedrawStatePreserved =
        result->c040RedrawStateAfterRejectedC040 == base.c040RedrawState &&
        result->c040RedrawStateAfterRejectedRotation == base.c040RedrawState;
    result->championChainPreservedBeforeGates =
        int_arrays_equal(result->candidateChainBefore,
                         result->candidateChainAfterRejectedC040,
                         DM1_V1_MC_C545_ACCEPT_ROTATE_CHAIN_COUNT_PC34) &&
        int_arrays_equal(result->candidateChainBefore,
                         result->candidateChainAfterRejectedRotation,
                         DM1_V1_MC_C545_ACCEPT_ROTATE_CHAIN_COUNT_PC34);
    result->c545PanelPixelPreservedBeforeGates =
        result->c545PanelPixelAfterRejectedC040 == base.c545PanelPixel &&
        result->c545PanelPixelAfterRejectedRotation == base.c545PanelPixel;
    result->stableThroughRejectedC545 =
        result->g0299PreservedBeforeGates &&
        result->c040CandidateIndexPreserved &&
        result->c040RedrawStatePreserved &&
        int_arrays_equal(result->candidateChainBefore,
                         result->candidateChainAfterRejectedC040,
                         DM1_V1_MC_C545_ACCEPT_ROTATE_CHAIN_COUNT_PC34) &&
        result->c545PanelPixelAfterRejectedC040 == base.c545PanelPixel;
    result->stableUntilBothGates =
        result->stableThroughRejectedC545 &&
        result->championChainPreservedBeforeGates &&
        result->c545PanelPixelPreservedBeforeGates;
    result->c040CloseCompletedBeforeAccept =
        closed && state->c040CloseCompleted && result->trace[5] == kTraceCloseC040 &&
        result->trace[8] == kTraceAccept;
    result->rotationDrainedBeforeAccept =
        rotated && state->leaderRotationDrained &&
        result->trace[7] == kTraceDrainRotation &&
        result->trace[8] == kTraceAccept;
    result->acceptAfterGatesSucceeded =
        accepted && state->acceptedAfterBothGates &&
        state->f0282AcceptClearCount == 1;
    result->g0299ClearedAfterAccept = state->g0299CandidateOrdinal == 0;
    result->candidateRemovedFromChain =
        state->candidateRemovedFromChain &&
        result->candidateChainAfterAccept[0] != kCandidateOrdinal;
    result->leaderRotationCompleted =
        state->leaderIndex == kNewLeaderIndex && state->f0368SetLeaderCount == 1;
    result->leaderHandCoherentAfterRotation =
        state->champions[state->leaderIndex].leader &&
        state->champions[state->leaderIndex].handThing == 0x7002u;
    result->sourceAnchorsPresent = source_anchors_present();
    result->guardRejectsNullState =
        dm1_v1_mirror_candidate_c545_accept_during_rotation_run_pc34(
            NULL, result) == 0;
    result->guardRejectsNullResult =
        dm1_v1_mirror_candidate_c545_accept_during_rotation_run_pc34(
            &base, NULL) == 0;
    result->guardRejectsNonContract = guard_rejects(&base, 0);
    result->guardRejectsNoCandidate = guard_rejects(&base, 1);
    result->guardRejectsWrongPanel = guard_rejects(&base, 2);
    result->guardRejectsNoRotation = guard_rejects(&base, 3);
    result->guardRejectsNoCloseQueued = guard_rejects(&base, 4);
    result->accepted =
        result->c545AcceptRoute && result->c040GateRequired &&
        result->rotationGateRequired && result->rejectedBeforeC040Close &&
        result->rejectedBeforeRotationDrain &&
        result->stableThroughRejectedC545 &&
        result->stableUntilBothGates &&
        result->c040CloseCompletedBeforeAccept &&
        result->rotationDrainedBeforeAccept &&
        result->acceptAfterGatesSucceeded &&
        result->g0299PreservedBeforeGates &&
        result->g0299ClearedAfterAccept &&
        result->candidateRemovedFromChain && result->leaderRotationCompleted &&
        result->leaderHandCoherentAfterRotation &&
        result->sourceAnchorsPresent && result->guardRejectsNullState &&
        result->guardRejectsNullResult && result->guardRejectsNonContract &&
        result->guardRejectsNoCandidate && result->guardRejectsWrongPanel &&
        result->guardRejectsNoRotation && result->guardRejectsNoCloseQueued &&
        state->commandQueueDepth == 0 && state->afterAcceptHash != 0u &&
        state->afterAcceptHash != state->afterCloseRejectedRotationHash;
    result->hash = hash_step(state->afterAcceptHash,
                             (unsigned int)result->accepted);
    return result->accepted;
}

const Dm1V1MirrorCandidateC545AcceptDuringRotationEvidencePc34 *
dm1_v1_mirror_candidate_c545_accept_during_rotation_evidence_pc34(void)
{
    return &s_evidence;
}

const char *
dm1_v1_mirror_candidate_c545_accept_during_rotation_source_evidence_pc34(void)
{
    return s_source_evidence;
}
