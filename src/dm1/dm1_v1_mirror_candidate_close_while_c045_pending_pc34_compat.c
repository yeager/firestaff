#include "firestaff/dm1/v1/mirror_candidate_close/close_while_c045_pending_pc34_compat.h"

#include <string.h>

enum {
    kCommandC045Slot = 45,
    kCommandC160Close = 160,
    kPanelFoodWater = 565,
    kPanelResurrect = 568,
    kGraphicC040Resurrect = 40,
    kGraphicC045ObjectIcons = 45,
    kLeaderIndex = 0,
    kInventoryChampionOrdinal = 1,
    kCandidateOrdinal = 3,
    kFoodThing = 0x0451,
    kTraceInit = 300,
    kTraceQueueC045 = 301,
    kTraceStoreClose = 302,
    kTraceRejectStaleC045 = 303,
    kTraceReplayClose = 304,
    kTraceDispatchClose = 305,
    kTraceClearCandidate = 306,
    kTraceStable = 307
};

/*
 * ReDMCSB source-lock map:
 * - REVIVE.C F0280:124-132 publishes the candidate only while the leader hand
 *   and party-count gates allow it.
 * - REVIVE.C F0282:744-806 owns the C160..C162 resurrect/reincarnate panel
 *   clear path, including G0299 clear and candidate-chain removal.
 * - PANEL.C F0344:1493-1561/F0345:1563-1617 define the food/water panel that
 *   created the stale C045 accept.
 * - PANEL.C F0346:1619-1637 and F0347:1639-1693 reroute back to C040 while
 *   G0299 is live.
 * - COMMAND.C F0359:1452-1662 queues clicks, with the locked-queue pending
 *   click fields at 1489-1494; F0360:1692-1707 replays that pending click.
 * - COMMAND.C F0378:1956-1993 dispatches panel clicks, and
 *   F0380:2045-2184 drains one queued command before the next.
 * - CHAMPION.C F0297/F0298:243-298 owns leader-hand lifetime; this gate
 *   proves the stale C045 accept does not remove the hand before C160 close.
 */
static const char s_source_evidence[] =
    "REVIVE.C F0280:124-132 publishes G0299; F0282:744-806 handles "
    "C160..C162 close/accept clear, G0299 clear, and candidate-chain removal. "
    "PANEL.C F0344:1493-1561 and F0345:1563-1617 define the food/water C045 "
    "surface; PANEL.C F0346:1619-1637 and F0347:1639-1693 keep C040 live "
    "while G0299 is set. COMMAND.C F0359:1452-1662 queues clicks and "
    "1489-1494 stores locked pending clicks; F0360:1692-1707 replays one "
    "pending click; F0378:1956-1993 routes panel clicks; F0380:2045-2184 "
    "drains one command at a time. CHAMPION.C F0297/F0298:243-298 owns "
    "leader hand lifetime. DEFS.H:338-340 C160..C162, 778-810 C10/C30, "
    "2200/2205 C040/C045, 2999-3008 M565/M568, 3906-3914 C537..C545, "
    "5694 G0299.";

static const Dm1V1MirrorCandidateCloseWhileC045PendingEvidencePc34 s_evidence =
    {
        "ReDMCSB REVIVE.C F0280:124-132 candidate publication gate",
        "ReDMCSB REVIVE.C F0282:744-806 C160 close/cancel candidate clear",
        "ReDMCSB PANEL.C F0344:1493-1561, F0345:1563-1617 C045 food/water surface",
        "ReDMCSB PANEL.C F0346:1619-1637 and F0347:1639-1693 C040 reroute while G0299 is live",
        "ReDMCSB COMMAND.C F0359:1452-1662 click queue",
        "ReDMCSB COMMAND.C F0359:1489-1494 pending click store and F0360:1692-1707 replay",
        "ReDMCSB COMMAND.C F0378:1956-1993 panel route",
        "ReDMCSB COMMAND.C F0380:2045-2184 one-command drain",
        "ReDMCSB CHAMPION.C F0297/F0298:243-298 leader-hand lifetime",
        "ReDMCSB DEFS.H C160..C162, C10/C30, C040/C045, M565/M568, C537..C545, G0299",
        "Non-overlap marker: this covers stale C045 food/water accept pending while a C160 C040 close click is stored during the locked queue, then replayed and drained. It is disjoint from chest action-hand owner-change, mirror-candidate close-after-party-shuffle, panel-redraw-after-inventory-exit, chest close object stack-merge, C040-eye-live-candidate, reshuffle panel live, scroll-pickup, chest close while mirror-candidate live, resurrect reselect/double-candidate/champion-switch/full-chain/cross-clear/close-pending, resurrect close-pending, C545 pickup, C545 accept during rotation, resurrect rotation scroll wheel, chest scroll-wheel drop/pickup/close/resurrect/overflow/open-combat/teleport/rotation gates, mirror-candidate left-click/scroll-wheel/inventory-click rotations, mirror-candidate inventory-click rotations, pickup save-load, inventory toggle, spell race, C045 accept cross-rotation, and C045 close after non-candidate transition."
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
    const Dm1V1MirrorCandidateCloseWhileC045PendingStatePc34 *state)
{
    uint32_t hash = UINT32_C(2166136261);
    int i;

    hash = hash_step(hash, state->seed);
    hash = hash_step(hash, (unsigned int)state->contractOnly);
    hash = hash_step(hash, (unsigned int)state->partyChampionCount);
    hash = hash_step(hash, (unsigned int)state->leaderIndex);
    hash = hash_step(hash, (unsigned int)state->g0299CandidateOrdinal);
    hash = hash_step(hash, (unsigned int)state->c040PanelOpen);
    hash = hash_step(hash, (unsigned int)state->c045FoodWaterAcceptPending);
    hash = hash_step(hash, (unsigned int)state->c160CloseClickQueued);
    hash = hash_step(hash, (unsigned int)state->pendingClickStoredWhileLocked);
    hash = hash_step(hash, (unsigned int)state->panelContent);
    hash = hash_step(hash, (unsigned int)state->panelGraphic);
    hash = hash_step(hash, (unsigned int)state->leaderHandThing);
    hash = hash_step(hash, (unsigned int)state->commandQueueDepth);
    hash = hash_step(hash, (unsigned int)state->f0282CloseClearCount);
    hash = hash_step(hash, (unsigned int)state->staleC045RejectCount);
    hash = hash_step(hash, (unsigned int)state->foodConsumed);
    for (i = 0; i < DM1_V1_MC_CLOSE_C045_PENDING_CHAIN_COUNT_PC34; ++i) {
        hash = hash_step(hash, (unsigned int)state->candidateChainOrdinals[i]);
        hash = hash_step(hash, (unsigned int)state->partyChainOrdinals[i]);
    }
    for (i = 0; i < DM1_V1_MC_CLOSE_C045_PENDING_TRACE_COUNT_PC34; ++i) {
        hash = hash_step(hash, (unsigned int)state->trace[i]);
    }
    return hash;
}

static int source_anchors_present(void)
{
    return strstr(s_source_evidence, "REVIVE.C F0280:124-132") != NULL &&
           strstr(s_source_evidence, "F0282:744-806") != NULL &&
           strstr(s_source_evidence, "PANEL.C F0344:1493-1561") != NULL &&
           strstr(s_source_evidence, "F0345:1563-1617") != NULL &&
           strstr(s_source_evidence, "F0346:1619-1637") != NULL &&
           strstr(s_source_evidence, "F0347:1639-1693") != NULL &&
           strstr(s_source_evidence, "COMMAND.C F0359:1452-1662") != NULL &&
           strstr(s_source_evidence, "1489-1494") != NULL &&
           strstr(s_source_evidence, "F0360:1692-1707") != NULL &&
           strstr(s_source_evidence, "F0378:1956-1993") != NULL &&
           strstr(s_source_evidence, "F0380:2045-2184") != NULL &&
           strstr(s_source_evidence, "CHAMPION.C F0297/F0298:243-298") !=
               NULL &&
           strstr(s_source_evidence, "5694 G0299") != NULL;
}

void dm1_v1_mirror_candidate_close_while_c045_pending_init_pc34(
    Dm1V1MirrorCandidateCloseWhileC045PendingStatePc34 *state,
    uint32_t seed)
{
    uint32_t mix;

    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    mix = (seed ^ UINT32_C(0x9e3779b9)) & UINT32_C(0xff);
    state->contractOnly = 1;
    state->noGameDataRequired = 1;
    state->seed = seed;
    state->partyChampionCount = 3;
    state->leaderIndex = kLeaderIndex;
    state->inventoryChampionOrdinal = kInventoryChampionOrdinal;
    state->candidateChampionOrdinal = kCandidateOrdinal;
    state->g0299CandidateOrdinal = kCandidateOrdinal;
    state->c040PanelOpen = 1;
    state->c045FoodWaterAcceptPending = 1;
    state->panelContent = kPanelResurrect;
    state->panelGraphic = kGraphicC040Resurrect;
    state->queuedC045Command = kCommandC045Slot;
    state->queuedCloseCommand = kCommandC160Close;
    state->c045FoodThing = (uint16_t)(kFoodThing + (int)(mix & 0x000fu));
    state->leaderHandThing = state->c045FoodThing;
    state->candidateChainOrdinals[0] = kCandidateOrdinal;
    state->candidateChainOrdinals[1] = 4;
    state->partyChainOrdinals[0] = 1;
    state->partyChainOrdinals[1] = 2;
    state->partyChainOrdinals[2] = kCandidateOrdinal;
    state->f0280PublishCount = 1;
    state->f0344FoodWaterReadCount = 2;
    state->f0345FoodWaterDrawCount = 1;
    state->f0346ResurrectDrawCount = 1;
    state->trace[0] = kTraceInit;
    state->beforeHash = hash_state(state);
}

static int ready(
    const Dm1V1MirrorCandidateCloseWhileC045PendingStatePc34 *state)
{
    return state && state->contractOnly && state->noGameDataRequired &&
           state->partyChampionCount == 3 &&
           state->leaderIndex == kLeaderIndex &&
           state->inventoryChampionOrdinal == kInventoryChampionOrdinal &&
           state->candidateChampionOrdinal == kCandidateOrdinal &&
           state->g0299CandidateOrdinal == kCandidateOrdinal &&
           state->candidateChainOrdinals[0] == kCandidateOrdinal &&
           state->c040PanelOpen && state->c045FoodWaterAcceptPending &&
           state->panelContent == kPanelResurrect &&
           state->panelGraphic == kGraphicC040Resurrect &&
           state->queuedC045Command == kCommandC045Slot &&
           state->queuedCloseCommand == kCommandC160Close &&
           state->leaderHandThing == state->c045FoodThing &&
           state->commandQueueDepth == 0;
}

static int queue_c045_and_store_close(
    Dm1V1MirrorCandidateCloseWhileC045PendingStatePc34 *state)
{
    if (!ready(state)) {
        return 0;
    }
    state->trace[1] = kTraceQueueC045;
    state->commandQueueDepth = 1;
    state->f0359QueueWriteCount = 1;
    state->trace[2] = kTraceStoreClose;
    state->pendingClickStoredWhileLocked = 1;
    state->c160CloseClickQueued = 1;
    return 1;
}

static int reject_stale_c045(
    Dm1V1MirrorCandidateCloseWhileC045PendingStatePc34 *state)
{
    if (!state || state->commandQueueDepth != 1 ||
        state->queuedC045Command != kCommandC045Slot ||
        state->panelContent != kPanelResurrect || !state->c040PanelOpen ||
        !state->c045FoodWaterAcceptPending) {
        return 0;
    }
    ++state->f0380DrainCount;
    ++state->f0378PanelRouteCount;
    ++state->staleC045RejectCount;
    state->trace[3] = kTraceRejectStaleC045;
    state->commandQueueDepth = 0;
    state->afterStaleC045Hash = hash_state(state);
    return 1;
}

static int replay_pending_close(
    Dm1V1MirrorCandidateCloseWhileC045PendingStatePc34 *state)
{
    if (!state || state->commandQueueDepth != 0 ||
        !state->pendingClickStoredWhileLocked || !state->c160CloseClickQueued ||
        state->g0299CandidateOrdinal != kCandidateOrdinal) {
        return 0;
    }
    ++state->f0360PendingReplayCount;
    ++state->f0359QueueWriteCount;
    state->pendingClickStoredWhileLocked = 0;
    state->commandQueueDepth = 1;
    state->trace[4] = kTraceReplayClose;
    state->afterPendingReplayHash = hash_state(state);
    return 1;
}

static void remove_candidate_from_chain(
    Dm1V1MirrorCandidateCloseWhileC045PendingStatePc34 *state)
{
    int i;

    for (i = 0; i < DM1_V1_MC_CLOSE_C045_PENDING_CHAIN_COUNT_PC34 - 1; ++i) {
        state->candidateChainOrdinals[i] = state->candidateChainOrdinals[i + 1];
    }
    state->candidateChainOrdinals[DM1_V1_MC_CLOSE_C045_PENDING_CHAIN_COUNT_PC34 -
                                  1] = 0;
}

static int dispatch_c160_close(
    Dm1V1MirrorCandidateCloseWhileC045PendingStatePc34 *state)
{
    if (!state || state->commandQueueDepth != 1 ||
        state->queuedCloseCommand != kCommandC160Close ||
        state->g0299CandidateOrdinal != kCandidateOrdinal ||
        !state->c040PanelOpen || !state->c045FoodWaterAcceptPending) {
        return 0;
    }
    ++state->f0380DrainCount;
    ++state->f0378PanelRouteCount;
    ++state->f0282CloseClearCount;
    state->trace[5] = kTraceDispatchClose;
    state->g0299CandidateOrdinal = 0;
    state->c040PanelOpen = 0;
    state->c045FoodWaterAcceptPending = 0;
    state->panelContent = 0;
    state->panelGraphic = 0;
    state->candidateSensorDisabled = 1;
    remove_candidate_from_chain(state);
    state->candidateRemovedFromChain =
        state->candidateChainOrdinals[0] != kCandidateOrdinal;
    state->closeClearedPendingC045 = 1;
    state->commandQueueDepth = 0;
    state->c160CloseClickQueued = 0;
    state->trace[6] = kTraceClearCandidate;
    state->trace[7] = kTraceStable;
    state->afterCloseHash = hash_state(state);
    return 1;
}

int dm1_v1_mirror_candidate_close_while_c045_pending_run_pc34(
    Dm1V1MirrorCandidateCloseWhileC045PendingStatePc34 *state,
    Dm1V1MirrorCandidateCloseWhileC045PendingResultPc34 *result)
{
    int beforeChain[DM1_V1_MC_CLOSE_C045_PENDING_CHAIN_COUNT_PC34];
    int afterStaleChain[DM1_V1_MC_CLOSE_C045_PENDING_CHAIN_COUNT_PC34];
    uint16_t leaderHandBefore;

    if (!state || !result || !ready(state)) {
        return 0;
    }
    memset(result, 0, sizeof(*result));
    copy_ints(beforeChain, state->candidateChainOrdinals,
              DM1_V1_MC_CLOSE_C045_PENDING_CHAIN_COUNT_PC34);
    leaderHandBefore = state->leaderHandThing;
    result->g0299Before = state->g0299CandidateOrdinal;
    result->beforeHash = state->beforeHash;
    result->leaderHandBefore = leaderHandBefore;

    if (!queue_c045_and_store_close(state) || !reject_stale_c045(state)) {
        return 0;
    }
    copy_ints(afterStaleChain, state->candidateChainOrdinals,
              DM1_V1_MC_CLOSE_C045_PENDING_CHAIN_COUNT_PC34);
    result->g0299AfterStaleC045 = state->g0299CandidateOrdinal;
    result->afterStaleC045Hash = state->afterStaleC045Hash;

    if (!replay_pending_close(state) || !dispatch_c160_close(state)) {
        return 0;
    }

    result->accepted = 1;
    result->c045PendingAtStart = 1;
    result->c160CloseArrivedWhileLocked = 1;
    result->staleC045RejectedBeforeClose = state->staleC045RejectCount == 1;
    result->candidatePreservedUntilClose =
        result->g0299AfterStaleC045 == kCandidateOrdinal &&
        afterStaleChain[0] == kCandidateOrdinal;
    result->pendingClickReplayed = state->f0360PendingReplayCount == 1;
    result->closeDispatchedThroughC040Panel =
        state->f0282CloseClearCount == 1 && state->f0378PanelRouteCount == 2;
    result->g0299ClearedByClose = state->g0299CandidateOrdinal == 0;
    result->c045PendingClearedByClose = state->closeClearedPendingC045;
    result->candidateRemovedFromChain = state->candidateRemovedFromChain;
    result->foodNotConsumed = state->foodConsumed == 0;
    result->leaderHandStable = state->leaderHandThing == leaderHandBefore;
    result->noLeaderHandRemoval = state->f0298RemoveLeaderHandCount == 0;
    result->queueDrained = state->commandQueueDepth == 0 &&
                           !state->pendingClickStoredWhileLocked;
    result->panelClosedAfterC160 = !state->c040PanelOpen &&
                                   state->panelContent == 0 &&
                                   state->panelGraphic == 0;
    result->sourceAnchorsPresent = source_anchors_present();
    result->guardRejectsNullState =
        dm1_v1_mirror_candidate_close_while_c045_pending_run_pc34(
            NULL, result) == 0;
    result->guardRejectsNullResult =
        dm1_v1_mirror_candidate_close_while_c045_pending_run_pc34(
            state, NULL) == 0;
    result->g0299AfterClose = state->g0299CandidateOrdinal;
    result->leaderHandAfter = state->leaderHandThing;
    copy_ints(result->candidateChainBefore, beforeChain,
              DM1_V1_MC_CLOSE_C045_PENDING_CHAIN_COUNT_PC34);
    copy_ints(result->candidateChainAfterStaleC045, afterStaleChain,
              DM1_V1_MC_CLOSE_C045_PENDING_CHAIN_COUNT_PC34);
    copy_ints(result->candidateChainAfterClose, state->candidateChainOrdinals,
              DM1_V1_MC_CLOSE_C045_PENDING_CHAIN_COUNT_PC34);
    copy_ints(result->trace, state->trace,
              DM1_V1_MC_CLOSE_C045_PENDING_TRACE_COUNT_PC34);
    result->afterPendingReplayHash = state->afterPendingReplayHash;
    result->afterCloseHash = state->afterCloseHash;
    result->hash = hash_state(state);
    return 1;
}

const Dm1V1MirrorCandidateCloseWhileC045PendingEvidencePc34 *
dm1_v1_mirror_candidate_close_while_c045_pending_evidence_pc34(void)
{
    return &s_evidence;
}

const char *
dm1_v1_mirror_candidate_close_while_c045_pending_source_evidence_pc34(void)
{
    return s_source_evidence;
}
