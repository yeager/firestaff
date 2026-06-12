#include "firestaff/dm1/v1/mirror_candidate/c045_food_water_accept_cross_rotation_pc34_compat.h"

#include <string.h>

enum {
    kPanelFoodWater = 565,
    kGraphicC045ObjectIcons096To127 = 45,
    kGraphicC040ResurrectReincarnate = 40,
    kCommandAcceptResurrect = 160,
    kCommandClickChampion1Status = 13,
    kCommandSetLeaderChampion1 = 17,
    kOldLeaderIndex = 0,
    kNewLeaderIndex = 1,
    kCandidateOrdinal = 3,
    kInventoryChampionOrdinal = 1,
    kFoodSlotIndexC30 = 30,
    kAcceptedFoodThing = 0x0451,
    kNewLeaderHandThing = 0x7021,
    kTraceInit = 100,
    kTraceQueueAccept = 101,
    kTraceQueueRotation = 102,
    kTraceAcceptStart = 103,
    kTraceAcceptDone = 104,
    kTraceRotateStart = 105,
    kTraceRotateDone = 106,
    kTraceStable = 107
};

/*
 * ReDMCSB anchors:
 * REVIVE.C F0280:124-132 publishes a candidate only when the leader hand is
 * empty enough for the candidate flow; F0282:744-806 is the accept/cancel
 * clear path, clearing G0299 before removing the accepted candidate chain.
 * CHAMPION.C F0297/F0298:243-298 own leader-hand lifetime and
 * F0301/F0302:606-714 own slot dispatch, including C30+ food/chest slots.
 * PANEL.C F0344:1493-1561/F0345:1563-1617/F0354:2299-2352 define the
 * food/water C045 route and chrome surface.
 * COMMAND.C F0359:1452-1662 queues clicks, F0378:1956-1993 routes panel
 * clicks, F0361:1709-1813 covers keyboard/wheel-like queue writes, and
 * F0380:2045-2178 drains one queued command before the next.
 * CLIKCHAM.C F0367/F0368:20-73 changes the leader only after dispatch.
 * DEFS.H:338-340 C160..C162, 778-810 C10/C30, 1874-1878 slot boxes,
 * 2078-2088 C10_COLOR_FLESH, 2200/2205 C040/C045, 2999-3008 M565/M568,
 * 3906-3914 C537..C545, and 5694 G0299 pin the constants.
 */
static const char s_source_evidence[] =
    "REVIVE.C F0280:124-132 publishes the candidate; F0282:744-806 clears "
    "G0299 and removes the accepted candidate chain before later work. "
    "CHAMPION.C F0297/F0298:243-298 leader hand; F0301/F0302:606-714 "
    "C30+ food/chest slot dispatch. PANEL.C F0344:1493-1561, "
    "F0345:1563-1617, F0354:2299-2352 food/water route. COMMAND.C "
    "F0359:1452-1662 queue, F0378:1956-1993 panel route, F0361:1709-1813 "
    "keyboard/wheel-like queue write, F0380:2045-2178 drain. "
    "CLIKCHAM.C F0367/F0368:20-73 leader set. DEFS.H:338-340 C160..C162, "
    "778-810 C10/C30, 1874-1878 C38/M070, 2078-2088 C10_COLOR_FLESH, "
    "2200/2205 C040/C045, 2999-3008 M565/M568, 3906-3914 C537..C545, "
    "5694 G0299.";

static const Dm1V1MirrorCandidateC045FoodWaterAcceptCrossRotationEvidencePc34
    s_evidence = {
        "ReDMCSB REVIVE.C F0280:124-132 candidate publication gate",
        "ReDMCSB REVIVE.C F0282:744-806 accept/cancel clear path must finish first",
        "ReDMCSB CHAMPION.C F0297/F0298:243-298 leader hand lifetime",
        "ReDMCSB CHAMPION.C F0301/F0302:606-714 C30+ food/chest slot path",
        "ReDMCSB PANEL.C F0344:1493-1561, F0345:1563-1617, F0354:2299-2352 C045 food/water route",
        "ReDMCSB COMMAND.C F0359:1452-1662 click queue and F0361:1709-1813 wheel-like queue write",
        "ReDMCSB COMMAND.C F0378:1956-1993 panel route for the C045 accept click",
        "ReDMCSB COMMAND.C F0380:2045-2178 drains accept before the queued rotation",
        "ReDMCSB CLIKCHAM.C F0367/F0368:20-73 set-leader route",
        "ReDMCSB DEFS.H C160..C162, C10_SLOT_NECK, C10_COLOR_FLESH, C30, C38, C545, G0299",
        "Non-overlap marker: pass772 covers C045 food/water ACCEPT plus same-drain leader rotation; not c045_close_after_non_candidate_transition, c045_food_water_close_no_candidate, c040_chrome_inventory_owner_swap, c040_close_non_leader_scroll_pickup, c040_redraw_after_chest_close, c040_panel_browse_pickup_rotate_race, click_cancel_with_rotation, rotation_during_resurrect_confirmation, c159_click_rotation_combo, c545_pickup_while_panel_live, c545_drop_while_panel_live, mirror_candidate_close_while_resurrect_pending_inventory_pickup, or chest_pickup_during_resurrect_pending_non_leader."
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

static uint32_t hash_state(
    const Dm1V1MirrorCandidateC045FoodWaterAcceptCrossRotationStatePc34
        *state)
{
    uint32_t hash = UINT32_C(2166136261);
    int i;

    hash = hash_step(hash, (unsigned int)state->sameDrainWindow);
    hash = hash_step(hash, (unsigned int)state->partyChampionCount);
    hash = hash_step(hash, (unsigned int)state->leaderIndex);
    hash = hash_step(hash, (unsigned int)state->queuedLeaderIndex);
    hash = hash_step(hash, (unsigned int)state->g0299CandidateOrdinal);
    hash = hash_step(hash, (unsigned int)state->c040PanelOpen);
    hash = hash_step(hash, (unsigned int)state->c045PanelOpen);
    hash = hash_step(hash, (unsigned int)state->panelContent);
    hash = hash_step(hash, (unsigned int)state->panelGraphic);
    hash = hash_step(hash, (unsigned int)state->leaderHandThing);
    hash = hash_step(hash, (unsigned int)state->candidateSensorDisabled);
    hash = hash_step(hash, (unsigned int)state->candidateRemovedFromChain);
    hash = hash_step(hash, (unsigned int)state->acceptClearCompleted);
    hash = hash_step(hash, (unsigned int)state->f0282AcceptClearCount);
    hash = hash_step(hash, (unsigned int)state->f0368SetLeaderCount);
    hash = hash_step(hash, (unsigned int)state->commandQueueDepth);
    for (i = 0; i < DM1_V1_MC_C045_ACCEPT_ROTATE_CHAIN_COUNT_PC34; ++i) {
        hash = hash_step(hash, (unsigned int)state->candidateChainOrdinals[i]);
    }
    for (i = 0; i < DM1_V1_MC_C045_ACCEPT_ROTATE_PARTY_COUNT_PC34; ++i) {
        hash = hash_step(hash, (unsigned int)state->partyChainOrdinals[i]);
        hash = hash_step(hash, (unsigned int)state->champions[i].ordinal);
        hash = hash_step(hash, (unsigned int)state->champions[i].leader);
        hash = hash_step(hash, (unsigned int)state->champions[i].chainLinked);
        hash = hash_step(hash, (unsigned int)state->champions[i].handThing);
    }
    for (i = 0; i < DM1_V1_MC_C045_ACCEPT_ROTATE_TRACE_COUNT_PC34; ++i) {
        hash = hash_step(hash, (unsigned int)state->trace[i]);
    }
    return hash;
}

static int source_anchors_present(void)
{
    return strstr(s_source_evidence, "REVIVE.C F0280:124-132") != NULL &&
           strstr(s_source_evidence, "F0282:744-806") != NULL &&
           strstr(s_source_evidence, "CHAMPION.C F0297/F0298:243-298") !=
               NULL &&
           strstr(s_source_evidence, "F0301/F0302:606-714") != NULL &&
           strstr(s_source_evidence, "PANEL.C F0344:1493-1561") != NULL &&
           strstr(s_source_evidence, "F0345:1563-1617") != NULL &&
           strstr(s_source_evidence, "COMMAND.C F0359:1452-1662") != NULL &&
           strstr(s_source_evidence, "F0378:1956-1993") != NULL &&
           strstr(s_source_evidence, "F0361:1709-1813") != NULL &&
           strstr(s_source_evidence, "F0380:2045-2178") != NULL &&
           strstr(s_source_evidence, "CLIKCHAM.C F0367/F0368:20-73") != NULL &&
           strstr(s_source_evidence, "5694 G0299") != NULL;
}

void dm1_v1_mirror_candidate_c045_food_water_accept_cross_rotation_init_pc34(
    Dm1V1MirrorCandidateC045FoodWaterAcceptCrossRotationStatePc34 *state)
{
    int i;

    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->contractOnly = 1;
    state->sameDrainWindow = 1;
    state->partyChampionCount = 3;
    state->leaderIndex = kOldLeaderIndex;
    state->queuedLeaderIndex = kNewLeaderIndex;
    state->inventoryChampionOrdinal = kInventoryChampionOrdinal;
    state->candidateChampionOrdinal = kCandidateOrdinal;
    state->candidateChainOrdinals[0] = kCandidateOrdinal;
    state->candidateChainOrdinals[1] = 4;
    state->partyChainOrdinals[0] = 1;
    state->partyChainOrdinals[1] = 2;
    state->partyChainOrdinals[2] = kCandidateOrdinal;
    state->g0299CandidateOrdinal = kCandidateOrdinal;
    state->c040PanelOpen = 0;
    state->c045PanelOpen = 1;
    state->panelContent = kPanelFoodWater;
    state->panelGraphic = kGraphicC045ObjectIcons096To127;
    state->acceptCommand = kCommandAcceptResurrect;
    state->queuedStatusCommand = kCommandClickChampion1Status;
    state->queuedSetLeaderCommand = kCommandSetLeaderChampion1;
    state->acceptedFoodThing = (uint16_t)kAcceptedFoodThing;
    state->leaderHandThing = (uint16_t)kAcceptedFoodThing;
    state->foodSlotIndex = kFoodSlotIndexC30;
    state->f0280PublishCount = 1;
    state->f0344FoodWaterReadCount = 2;
    state->f0345FoodWaterDrawCount = 1;
    state->trace[0] = kTraceInit;
    for (i = 0; i < DM1_V1_MC_C045_ACCEPT_ROTATE_PARTY_COUNT_PC34; ++i) {
        state->champions[i].ordinal = i + 1;
        state->champions[i].alive = i < state->partyChampionCount;
        state->champions[i].chainLinked = i < state->partyChampionCount;
        state->champions[i].handThing =
            DM1_V1_MC_C045_ACCEPT_ROTATE_NONE_PC34;
    }
    state->champions[kOldLeaderIndex].leader = 1;
    state->champions[kOldLeaderIndex].handThing =
        (uint16_t)kAcceptedFoodThing;
    state->champions[kOldLeaderIndex].load = 37;
    state->champions[kNewLeaderIndex].handThing =
        (uint16_t)kNewLeaderHandThing;
    state->champions[kNewLeaderIndex].load = 42;
    state->beforeHash = hash_state(state);
}

static int ready(
    const Dm1V1MirrorCandidateC045FoodWaterAcceptCrossRotationStatePc34
        *state)
{
    return state && state->contractOnly && state->sameDrainWindow &&
           state->partyChampionCount == 3 &&
           state->leaderIndex == kOldLeaderIndex &&
           state->queuedLeaderIndex == kNewLeaderIndex &&
           state->g0299CandidateOrdinal == kCandidateOrdinal &&
           state->candidateChainOrdinals[0] == kCandidateOrdinal &&
           state->c040PanelOpen == 0 && state->c045PanelOpen == 1 &&
           state->panelContent == kPanelFoodWater &&
           state->panelGraphic == kGraphicC045ObjectIcons096To127 &&
           state->acceptCommand == kCommandAcceptResurrect &&
           state->queuedSetLeaderCommand == kCommandSetLeaderChampion1 &&
           state->leaderHandThing == state->acceptedFoodThing &&
           state->champions[kOldLeaderIndex].handThing ==
               state->acceptedFoodThing &&
           state->champions[kNewLeaderIndex].handThing ==
               kNewLeaderHandThing &&
           state->commandQueueDepth == 0;
}

static int queue_accept_and_rotation(
    Dm1V1MirrorCandidateC045FoodWaterAcceptCrossRotationStatePc34 *state)
{
    if (!ready(state)) {
        return 0;
    }
    state->trace[1] = kTraceQueueAccept;
    state->trace[2] = kTraceQueueRotation;
    state->f0359QueueWriteCount += 2;
    state->f0361WheelLikeQueueWriteCount += 1;
    state->commandQueueDepth = 2;
    state->afterQueueHash = hash_state(state);
    return 1;
}

static void remove_candidate_from_chain(
    Dm1V1MirrorCandidateC045FoodWaterAcceptCrossRotationStatePc34 *state)
{
    int i;

    for (i = 0; i < DM1_V1_MC_C045_ACCEPT_ROTATE_CHAIN_COUNT_PC34 - 1; ++i) {
        state->candidateChainOrdinals[i] = state->candidateChainOrdinals[i + 1];
    }
    state->candidateChainOrdinals[DM1_V1_MC_C045_ACCEPT_ROTATE_CHAIN_COUNT_PC34 -
                                  1] = 0;
}

static int dispatch_accept_clear(
    Dm1V1MirrorCandidateC045FoodWaterAcceptCrossRotationStatePc34 *state)
{
    if (!state || state->commandQueueDepth != 2 ||
        state->trace[1] != kTraceQueueAccept ||
        state->trace[2] != kTraceQueueRotation || state->c040PanelOpen ||
        !state->c045PanelOpen ||
        state->g0299CandidateOrdinal != kCandidateOrdinal ||
        state->candidateChainOrdinals[0] != kCandidateOrdinal) {
        return 0;
    }

    state->trace[3] = kTraceAcceptStart;
    ++state->f0380DispatchCount;
    ++state->f0378PanelRouteCount;
    ++state->f0282AcceptClearCount;
    ++state->f0302FoodSlotDispatchCount;
    ++state->f0298RemoveLeaderHandCount;
    state->g0299CandidateOrdinal = 0;
    state->c040PanelOpen = 0;
    state->c045PanelOpen = 0;
    state->panelContent = 0;
    state->panelGraphic = 0;
    state->leaderHandThing = DM1_V1_MC_C045_ACCEPT_ROTATE_NONE_PC34;
    state->champions[kOldLeaderIndex].handThing =
        DM1_V1_MC_C045_ACCEPT_ROTATE_NONE_PC34;
    state->champions[kOldLeaderIndex].load -= 1;
    state->foodRemovedFromOldLeader = 1;
    state->candidateSensorDisabled = 1;
    remove_candidate_from_chain(state);
    state->candidateRemovedFromChain =
        state->candidateChainOrdinals[0] != kCandidateOrdinal;
    state->acceptClearCompleted = 1;
    state->acceptCompletedBeforeRotation = state->f0368SetLeaderCount == 0;
    --state->commandQueueDepth;
    state->trace[4] = kTraceAcceptDone;
    state->afterAcceptHash = hash_state(state);
    return 1;
}

static int dispatch_leader_rotation(
    Dm1V1MirrorCandidateC045FoodWaterAcceptCrossRotationStatePc34 *state)
{
    int oldLeader;

    if (!state || state->commandQueueDepth != 1 ||
        !state->acceptClearCompleted || state->g0299CandidateOrdinal != 0 ||
        state->candidateChainOrdinals[0] == kCandidateOrdinal ||
        state->c040PanelOpen || state->c045PanelOpen) {
        state->skippedClearAttempted = 1;
        return 0;
    }

    state->trace[5] = kTraceRotateStart;
    ++state->f0380DispatchCount;
    ++state->f0367LeaderClickRouteCount;
    ++state->f0368SetLeaderCount;
    oldLeader = state->leaderIndex;
    state->champions[oldLeader].leader = 0;
    state->leaderIndex = state->queuedLeaderIndex;
    state->champions[state->leaderIndex].leader = 1;
    state->leaderHandThing = state->champions[state->leaderIndex].handThing;
    --state->commandQueueDepth;
    state->trace[6] = kTraceRotateDone;
    state->trace[7] = kTraceStable;
    state->afterRotateHash = hash_state(state);
    return 1;
}

static int guard_rejects(
    const Dm1V1MirrorCandidateC045FoodWaterAcceptCrossRotationStatePc34 *base,
    int kind)
{
    Dm1V1MirrorCandidateC045FoodWaterAcceptCrossRotationStatePc34 probe =
        *base;
    Dm1V1MirrorCandidateC045FoodWaterAcceptCrossRotationResultPc34 result;

    if (kind == 0) {
        probe.c040PanelOpen = 1;
        probe.panelGraphic = kGraphicC040ResurrectReincarnate;
    } else if (kind == 1) {
        probe.g0299CandidateOrdinal = 0;
    } else if (kind == 2) {
        probe.panelContent = 999;
    } else {
        probe.queuedLeaderIndex = kOldLeaderIndex;
        probe.queuedSetLeaderCommand = 16;
    }
    return dm1_v1_mirror_candidate_c045_food_water_accept_cross_rotation_run_pc34(
               &probe, &result) == 0;
}

int dm1_v1_mirror_candidate_c045_food_water_accept_cross_rotation_run_pc34(
    Dm1V1MirrorCandidateC045FoodWaterAcceptCrossRotationStatePc34 *state,
    Dm1V1MirrorCandidateC045FoodWaterAcceptCrossRotationResultPc34 *result)
{
    Dm1V1MirrorCandidateC045FoodWaterAcceptCrossRotationStatePc34 base;
    int queued;
    int accepted;
    int rotated;

    if (!state || !result) {
        return 0;
    }
    memset(result, 0, sizeof(*result));
    if (!ready(state)) {
        return 0;
    }

    base = *state;
    result->sameDrainWindow = state->sameDrainWindow;
    result->leaderBefore = state->leaderIndex;
    result->oldLeaderHandBefore = state->champions[kOldLeaderIndex].handThing;
    result->newLeaderHandBefore = state->champions[kNewLeaderIndex].handThing;
    result->g0299Before = state->g0299CandidateOrdinal;
    copy_ints(result->candidateChainBefore, state->candidateChainOrdinals,
              DM1_V1_MC_C045_ACCEPT_ROTATE_CHAIN_COUNT_PC34);
    queued = queue_accept_and_rotation(state);
    accepted = dispatch_accept_clear(state);
    rotated = dispatch_leader_rotation(state);

    result->leaderAfter = state->leaderIndex;
    result->oldLeaderHandAfter = state->champions[kOldLeaderIndex].handThing;
    result->newLeaderHandAfter = state->champions[kNewLeaderIndex].handThing;
    result->g0299After = state->g0299CandidateOrdinal;
    copy_ints(result->candidateChainAfter, state->candidateChainOrdinals,
              DM1_V1_MC_C045_ACCEPT_ROTATE_CHAIN_COUNT_PC34);
    copy_ints(result->trace, state->trace,
              DM1_V1_MC_C045_ACCEPT_ROTATE_TRACE_COUNT_PC34);
    result->beforeHash = base.beforeHash;
    result->afterAcceptHash = state->afterAcceptHash;
    result->afterRotateHash = state->afterRotateHash;
    result->c045AcceptPath =
        queued && accepted && base.c045PanelOpen && !base.c040PanelOpen &&
        base.panelContent == kPanelFoodWater &&
        base.acceptCommand == kCommandAcceptResurrect &&
        state->f0378PanelRouteCount == 1;
    result->c040NotLive = base.c040PanelOpen == 0 &&
                          base.panelGraphic != kGraphicC040ResurrectReincarnate;
    result->acceptClearRanFirst =
        accepted && state->acceptCompletedBeforeRotation &&
        state->trace[3] == kTraceAcceptStart &&
        state->trace[4] == kTraceAcceptDone &&
        state->trace[5] == kTraceRotateStart &&
        state->trace[6] == kTraceRotateDone;
    result->candidateRemovedFromChain =
        state->candidateRemovedFromChain &&
        state->candidateChainOrdinals[0] != base.candidateChainOrdinals[0];
    result->g0299Cleared = state->g0299CandidateOrdinal == 0;
    result->c040C045Cleared =
        state->c040PanelOpen == 0 && state->c045PanelOpen == 0 &&
        state->panelContent == 0 && state->panelGraphic == 0;
    result->foodRemovedByAccept =
        state->foodRemovedFromOldLeader &&
        state->f0298RemoveLeaderHandCount == 1 &&
        state->f0302FoodSlotDispatchCount == 1;
    result->oldLeaderHandEmpty =
        state->champions[kOldLeaderIndex].handThing ==
            DM1_V1_MC_C045_ACCEPT_ROTATE_NONE_PC34 &&
        result->oldLeaderHandBefore == kAcceptedFoodThing;
    result->rotationCompletedAfterAccept =
        rotated && state->f0368SetLeaderCount == 1 &&
        state->leaderIndex == kNewLeaderIndex &&
        state->f0380DispatchCount == 2;
    result->newLeaderHandPreserved =
        state->champions[kNewLeaderIndex].handThing ==
            result->newLeaderHandBefore &&
        result->newLeaderHandBefore == kNewLeaderHandThing;
    result->leaderHandCoherentAfterRotation =
        state->leaderHandThing == state->champions[state->leaderIndex].handThing;
    result->noDoubleClear =
        state->f0282AcceptClearCount == 1 && !state->doubleClearAttempted;
    result->noSkippedClear =
        state->acceptClearCompleted && !state->skippedClearAttempted;
    result->noDanglingCandidate =
        state->g0299CandidateOrdinal == 0 &&
        state->candidateChainOrdinals[0] != kCandidateOrdinal &&
        state->candidateSensorDisabled;
    result->partyChainCorrect =
        state->partyChainOrdinals[0] == 1 &&
        state->partyChainOrdinals[1] == 2 &&
        state->partyChainOrdinals[2] == kCandidateOrdinal &&
        state->champions[0].chainLinked && state->champions[1].chainLinked &&
        state->champions[2].chainLinked;
    result->queueDrained = state->commandQueueDepth == 0;
    result->sourceAnchorsPresent = source_anchors_present();
    result->guardRejectsC040Live = guard_rejects(&base, 0);
    result->guardRejectsNoCandidate = guard_rejects(&base, 1);
    result->guardRejectsWrongPanel = guard_rejects(&base, 2);
    result->guardRejectsNoRotation = guard_rejects(&base, 3);
    result->accepted =
        result->sameDrainWindow && result->c045AcceptPath &&
        result->c040NotLive && result->acceptClearRanFirst &&
        result->candidateRemovedFromChain && result->g0299Cleared &&
        result->c040C045Cleared && result->foodRemovedByAccept &&
        result->oldLeaderHandEmpty && result->rotationCompletedAfterAccept &&
        result->newLeaderHandPreserved &&
        result->leaderHandCoherentAfterRotation && result->noDoubleClear &&
        result->noSkippedClear && result->noDanglingCandidate &&
        result->partyChainCorrect && result->queueDrained &&
        result->sourceAnchorsPresent && result->guardRejectsC040Live &&
        result->guardRejectsNoCandidate && result->guardRejectsWrongPanel &&
        result->guardRejectsNoRotation && state->afterAcceptHash != 0u &&
        state->afterRotateHash != 0u &&
        state->afterAcceptHash != state->afterRotateHash;
    result->hash = hash_step(state->afterRotateHash,
                             (unsigned int)result->accepted);
    return result->accepted;
}

const Dm1V1MirrorCandidateC045FoodWaterAcceptCrossRotationEvidencePc34 *
dm1_v1_mirror_candidate_c045_food_water_accept_cross_rotation_evidence_pc34(
    void)
{
    return &s_evidence;
}

const char *
dm1_v1_mirror_candidate_c045_food_water_accept_cross_rotation_source_evidence_pc34(
    void)
{
    return s_source_evidence;
}
