#include "firestaff/dm1/v1/mirror_candidate/c160_close_while_rotation_pending_pc34_compat.h"

#include <string.h>

enum {
    kLeader0 = 0,
    kQueuedLeader1 = 1,
    kInventoryChampionOrdinal = 1,
    kCandidateOrdinal = 314,
    kCandidateOwnerIndex = 0,
    kCandidateChainIndex = 1,
    kPanelResurrect = 568,
    kGraphicC040Resurrect = 40,
    kC160CloseCommand = 160,
    kThingNone = 0xffff,
    kOpenChestThing = 0x6c60,
    kChestSlotBase = 0x7500,
    kTraceInit = 460,
    kTraceF0302Pending = 461,
    kTraceFreshC160Click = 462,
    kTraceF0378PanelRoute = 463,
    kTraceC160Clear = 464,
    kTraceDungeonFloorCeiling = 465,
    kTraceMouseRefresh = 466,
    kTraceLeaderCommit = 467,
    kTraceStable = 468
};

/*
 * ReDMCSB source-lock map for this race:
 * - PANEL.C F0344:1493-1561 and F0345:1563-1617 are the sibling food/water
 *   panel chain.  They must not be redrawn by this C160 close path.
 * - PANEL.C F0346:1619-1637 and F0347:1639-1693 give G0299/C040 priority,
 *   but the fresh close click clears G0299 instead of re-running F0346.
 * - PANEL.C F0355:2318-2322 skips status redraw while G0299 is live.  This
 *   gate is not allowed to suppress the C160 panel close.
 * - CHEST.C F0333/F0334 opens/closes G0426; neither side fires here.
 * - CHAMPION.C F0297/F0298:243-298 and F0300/F0301/F0302:511-714 own the
 *   in-flight leader/slot path; the pending leader change commits only after
 *   C160 close handling.
 * - COMMAND.C F0359:1452-1662, F0360:1692-1707, F0378:1956-1993, and
 *   F0380:2045-2184 route a fresh panel click through M568 to F0282.
 * - CLIKCHAM.C F0367/F0368:1-73 is the leader set/order path.
 * - DEFS.H:277/282/338-340/2200/2999-3008/3906-3914/5694 provide
 *   C040/C045/C160..C162/C030/M565/M568/C537..C545/G0299.
 */
static const char s_source_evidence[] =
    "PANEL.C F0344:1493-1561 and F0345:1563-1617 food/water chain; "
    "PANEL.C F0346:1619-1637 and F0347:1639-1693 C040 resurrect reroute; "
    "PANEL.C F0355:2318-2322 !G0299 candidate gate must not suppress C160. "
    "CHEST.C F0333/F0334 G0426 open/close must stay idle. "
    "CHAMPION.C F0297/F0298:243-298 leader-hand lifetime; "
    "CHAMPION.C F0300/F0301:511-614 slot writes; "
    "CHAMPION.C F0302:677-712 in-flight leader/slot path. "
    "COMMAND.C F0359:1452-1662 fresh click queue; "
    "COMMAND.C F0360:1692-1707 pending replay stays idle for PC fresh click; "
    "COMMAND.C F0378:1956-1993 M568 panel dispatch; "
    "COMMAND.C F0380:2045-2184 queue drain. "
    "CLIKCHAM.C F0367/F0368:1-73 set leader/order path. "
    "DEFS.H C160..C162/C040/C045/C030/M565/M568/C537..C545/G0299. "
    "Close landing: F0098_DUNGEONVIEW_DrawFloorAndCeiling and "
    "F0326_B_RefreshMousePointerInMainLoop.";

static const Dm1V1MirrorCandidateC160CloseWhileRotationPendingEvidencePc34
    s_evidence = {
        "ReDMCSB PANEL.C F0344:1493-1561 and F0345:1563-1617 food/water panel chain",
        "ReDMCSB PANEL.C F0346:1619-1637 and F0347:1639-1693 C040 resurrect reroute",
        "ReDMCSB PANEL.C F0355:2318-2322 !G0299 candidate gate; C160 close bypasses this suppression",
        "ReDMCSB CHEST.C F0333/F0334 G0426 open/close path remains idle",
        "ReDMCSB CHAMPION.C F0297/F0298:243-298 plus F0300/F0301/F0302:511-714 in-flight leader/slot path",
        "ReDMCSB COMMAND.C F0359:1452-1662 fresh click queue and C160 click capture",
        "ReDMCSB COMMAND.C F0378:1956-1993 M568 panel route to C160..C162 handler",
        "ReDMCSB COMMAND.C F0360:1692-1707 pending replay stays idle; F0380:2045-2184 drains the C160 panel click",
        "ReDMCSB CLIKCHAM.C F0367/F0368:1-73 set leader/order path commits after close",
        "ReDMCSB DEFS.H C160..C162/C040/C045/C030/M565/M568/C537..C545/G0299 constants",
        "Firestaff close landing contract: F0098_DUNGEONVIEW_DrawFloorAndCeiling plus F0326_B_RefreshMousePointerInMainLoop",
        "Non-overlap marker: live C040 mirror-candidate with a fresh C160 close click while F0302 leader rotation is in-flight; not C045 pending, not C160 close replay, not resurrect confirmation, not chest open/close, not chest pickup/drop, not inventory toggle, not save/load, not teleporter, and not party turn/party-rotate side effects."
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

static uint32_t hash_chest(
    const Dm1V1MirrorCandidateC160CloseWhileRotationPendingStatePc34 *state)
{
    uint32_t hash = UINT32_C(2166136261);
    int i;

    hash = hash_step(hash, (unsigned int)state->openChestThing);
    hash = hash_step(hash, (unsigned int)state->g0426OpenChest);
    for (i = 0; i < DM1_V1_MC_C160_ROTATION_CHEST_SLOT_COUNT_PC34; ++i) {
        hash = hash_step(hash, (unsigned int)state->chestSlots[i]);
    }
    return hash;
}

static uint32_t hash_state(
    const Dm1V1MirrorCandidateC160CloseWhileRotationPendingStatePc34 *state)
{
    uint32_t hash = UINT32_C(2166136261);
    int i;

    hash = hash_step(hash, state->seed);
    hash = hash_step(hash, (unsigned int)state->partyChampionCount);
    hash = hash_step(hash, (unsigned int)state->leaderIndex);
    hash = hash_step(hash, (unsigned int)state->pendingLeaderIndex);
    hash = hash_step(hash, (unsigned int)state->inventoryChampionOrdinal);
    hash = hash_step(hash, (unsigned int)state->g0299CandidateOrdinal);
    hash = hash_step(hash, (unsigned int)state->candidateOwnerIndex);
    hash = hash_step(hash, (unsigned int)state->candidateChainIndex);
    hash = hash_step(hash, (unsigned int)state->c040PanelLive);
    hash = hash_step(hash, (unsigned int)state->panelContent);
    hash = hash_step(hash, (unsigned int)state->panelGraphic);
    hash = hash_step(hash, (unsigned int)state->leaderHandEmpty);
    hash = hash_step(hash, (unsigned int)state->f0302RotationInFlight);
    hash = hash_step(hash, (unsigned int)state->f0302RotationCommitted);
    hash = hash_step(hash, (unsigned int)state->c160CloseClickDispatched);
    hash = hash_step(hash, (unsigned int)state->commandQueueDepth);
    hash = hash_step(hash, hash_chest(state));
    hash = hash_step(hash, state->c040PanelPixelHashBefore);
    hash = hash_step(hash, state->c040PanelPixelHashAfterClose);
    for (i = 0; i < DM1_V1_MC_C160_ROTATION_TRACE_COUNT_PC34; ++i) {
        hash = hash_step(hash, (unsigned int)state->trace[i]);
    }
    for (i = 0; i < DM1_V1_MC_C160_ROTATION_PARTY_COUNT_PC34; ++i) {
        hash = hash_step(hash, (unsigned int)state->champions[i].leader);
        hash = hash_step(hash, (unsigned int)state->champions[i].rotationPending);
        hash = hash_step(hash, (unsigned int)state->champions[i].c040CandidateOwner);
    }
    return hash;
}

static int source_anchors_present(void)
{
    return strstr(s_source_evidence, "PANEL.C F0344:1493-1561") != NULL &&
           strstr(s_source_evidence, "F0345:1563-1617") != NULL &&
           strstr(s_source_evidence, "F0346:1619-1637") != NULL &&
           strstr(s_source_evidence, "F0347:1639-1693") != NULL &&
           strstr(s_source_evidence, "F0355:2318-2322") != NULL &&
           strstr(s_source_evidence, "CHEST.C F0333/F0334") != NULL &&
           strstr(s_source_evidence, "CHAMPION.C F0297/F0298:243-298") != NULL &&
           strstr(s_source_evidence, "F0300/F0301:511-614") != NULL &&
           strstr(s_source_evidence, "F0302:677-712") != NULL &&
           strstr(s_source_evidence, "COMMAND.C F0359:1452-1662") != NULL &&
           strstr(s_source_evidence, "F0360:1692-1707") != NULL &&
           strstr(s_source_evidence, "F0378:1956-1993") != NULL &&
           strstr(s_source_evidence, "F0380:2045-2184") != NULL &&
           strstr(s_source_evidence, "CLIKCHAM.C F0367/F0368:1-73") != NULL &&
           strstr(s_source_evidence, "C160..C162/C040/C045/C030") != NULL &&
           strstr(s_source_evidence, "F0098_DUNGEONVIEW_DrawFloorAndCeiling") != NULL &&
           strstr(s_source_evidence, "F0326_B_RefreshMousePointerInMainLoop") != NULL;
}

void dm1_v1_mirror_candidate_c160_close_while_rotation_pending_init_pc34(
    Dm1V1MirrorCandidateC160CloseWhileRotationPendingStatePc34 *state,
    uint32_t seed)
{
    int i;

    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->contractOnly = 1;
    state->noGameDataRequired = 1;
    state->seed = seed;
    state->partyChampionCount = DM1_V1_MC_C160_ROTATION_PARTY_COUNT_PC34;
    state->leaderIndex = kLeader0;
    state->pendingLeaderIndex = kQueuedLeader1;
    state->inventoryChampionOrdinal = kInventoryChampionOrdinal;
    state->g0299CandidateOrdinal = kCandidateOrdinal;
    state->candidateOwnerIndex = kCandidateOwnerIndex;
    state->candidateChainIndex = kCandidateChainIndex;
    state->c040PanelLive = 1;
    state->panelContent = kPanelResurrect;
    state->panelGraphic = kGraphicC040Resurrect;
    state->leaderHandEmpty = 1;
    state->openChestThing = kOpenChestThing;
    state->g0426OpenChest = kOpenChestThing;
    state->f0346C040DrawCount = 1;
    state->f0347PanelDrawCount = 1;
    state->c040PanelPixelHashBefore =
        UINT32_C(0xc1600400) ^ (seed & UINT32_C(0x0000ffff));
    state->c040PanelPixelHashAfterClose = state->c040PanelPixelHashBefore;
    state->trace[0] = kTraceInit;

    for (i = 0; i < DM1_V1_MC_C160_ROTATION_CHEST_SLOT_COUNT_PC34; ++i) {
        state->chestSlots[i] = kChestSlotBase + i;
    }
    for (i = 0; i < DM1_V1_MC_C160_ROTATION_PARTY_COUNT_PC34; ++i) {
        state->champions[i].championOrdinal = i + 1;
        state->champions[i].alive = 1;
        state->champions[i].leader = i == kLeader0;
        state->champions[i].rotationPending = i == kQueuedLeader1;
        state->champions[i].c040CandidateOwner = i == kCandidateOwnerIndex;
    }
    state->chestHashBefore = hash_chest(state);
    state->chestHashAfter = state->chestHashBefore;
    state->beforeHash = hash_state(state);
}

static int ready(
    const Dm1V1MirrorCandidateC160CloseWhileRotationPendingStatePc34 *state)
{
    return state && state->contractOnly && state->noGameDataRequired &&
           state->partyChampionCount == DM1_V1_MC_C160_ROTATION_PARTY_COUNT_PC34 &&
           state->leaderIndex == kLeader0 &&
           state->pendingLeaderIndex == kQueuedLeader1 &&
           state->inventoryChampionOrdinal == kInventoryChampionOrdinal &&
           state->g0299CandidateOrdinal == kCandidateOrdinal &&
           state->candidateOwnerIndex == kCandidateOwnerIndex &&
           state->c040PanelLive &&
           state->panelContent == kPanelResurrect &&
           state->panelGraphic == kGraphicC040Resurrect &&
           state->leaderHandEmpty &&
           state->g0426OpenChest == kOpenChestThing &&
           state->commandQueueDepth == 0;
}

static int enter_f0302_rotation_pending(
    Dm1V1MirrorCandidateC160CloseWhileRotationPendingStatePc34 *state)
{
    if (!ready(state)) {
        return 0;
    }
    state->f0302EnterCount = 1;
    state->f0302RotationInFlight = 1;
    state->commandQueueDepth = 1;
    state->trace[1] = kTraceF0302Pending;
    state->afterF0302PendingHash = hash_state(state);
    return 1;
}

static int dispatch_fresh_c160_close(
    Dm1V1MirrorCandidateC160CloseWhileRotationPendingStatePc34 *state)
{
    if (!state || !state->f0302RotationInFlight ||
        state->f0302RotationCommitted || state->g0299CandidateOrdinal == 0 ||
        state->panelContent != kPanelResurrect ||
        state->panelGraphic != kGraphicC040Resurrect) {
        return 0;
    }
    state->f0359FreshClickCount = 1;
    state->c160CloseClickDispatched = kC160CloseCommand;
    state->trace[2] = kTraceFreshC160Click;
    state->f0380DrainCount = 1;
    state->f0378PanelRouteCount = 1;
    state->trace[3] = kTraceF0378PanelRoute;
    state->f0282C160ClearCount = 1;
    state->g0299CandidateOrdinal = 0;
    state->c040PanelLive = 0;
    state->panelContent = 0;
    state->panelGraphic = 0;
    state->commandQueueDepth = 0;
    state->trace[4] = kTraceC160Clear;
    state->f0098FloorCeilingDrawCount = 1;
    state->trace[5] = kTraceDungeonFloorCeiling;
    state->f0326MousePointerRefreshCount = 1;
    state->trace[6] = kTraceMouseRefresh;
    state->c040PanelPixelHashAfterClose = state->c040PanelPixelHashBefore;
    state->chestHashAfter = hash_chest(state);
    state->afterCloseHash = hash_state(state);
    return 1;
}

static int commit_leader_rotation_after_close(
    Dm1V1MirrorCandidateC160CloseWhileRotationPendingStatePc34 *state)
{
    int oldLeader;
    int newLeader;

    if (!state || !state->f0302RotationInFlight ||
        state->g0299CandidateOrdinal != 0 || state->commandQueueDepth != 0) {
        return 0;
    }
    oldLeader = state->leaderIndex;
    newLeader = state->pendingLeaderIndex;
    state->f0367StatusBoxClickCount = 1;
    state->f0368SetLeaderCount = 1;
    state->f0302LeaderCommitCount = 1;
    state->f0302RotationCommitted = 1;
    state->f0302RotationInFlight = 0;
    state->leaderIndex = newLeader;
    state->pendingLeaderIndex = -1;
    state->champions[oldLeader].leader = 0;
    state->champions[oldLeader].rotationPending = 0;
    state->champions[newLeader].leader = 1;
    state->champions[newLeader].rotationPending = 0;
    state->trace[7] = kTraceLeaderCommit;
    state->trace[8] = kTraceStable;
    state->afterRotationCommitHash = hash_state(state);
    return 1;
}

static void snapshot_before_close(
    const Dm1V1MirrorCandidateC160CloseWhileRotationPendingStatePc34 *state,
    Dm1V1MirrorCandidateC160CloseWhileRotationPendingResultPc34 *result)
{
    int i;

    result->contractOnly = state->contractOnly;
    result->noGameDataRequired = state->noGameDataRequired;
    result->initialLeaderIndex = state->leaderIndex;
    result->pendingLeaderIndexBeforeClose = state->pendingLeaderIndex;
    result->g0299BeforeClose = state->g0299CandidateOrdinal;
    result->candidateOwnerBeforeClose = state->candidateOwnerIndex;
    result->c040PanelLiveBeforeClose = state->c040PanelLive;
    result->panelContentBeforeClose = state->panelContent;
    result->panelGraphicBeforeClose = state->panelGraphic;
    result->f0302RotationInFlightBeforeClose = state->f0302RotationInFlight;
    result->beforeHash = state->beforeHash;
    result->afterF0302PendingHash = state->afterF0302PendingHash;
    result->c040PanelPixelHashBefore = state->c040PanelPixelHashBefore;
    result->chestHashBefore = state->chestHashBefore;
    for (i = 0; i < DM1_V1_MC_C160_ROTATION_CHEST_SLOT_COUNT_PC34; ++i) {
        result->chestSlotsBefore[i] = state->chestSlots[i];
    }
}

static void snapshot_after_close(
    const Dm1V1MirrorCandidateC160CloseWhileRotationPendingStatePc34 *state,
    Dm1V1MirrorCandidateC160CloseWhileRotationPendingResultPc34 *result)
{
    result->leaderIndexDuringClose = state->leaderIndex;
    result->pendingLeaderIndexAfterClose = state->pendingLeaderIndex;
    result->g0299AfterClose = state->g0299CandidateOrdinal;
    result->candidateOwnerAfterClose = state->candidateOwnerIndex;
    result->c040PanelLiveAfterClose = state->c040PanelLive;
    result->panelContentAfterClose = state->panelContent;
    result->panelGraphicAfterClose = state->panelGraphic;
    result->commandQueueDepthAfterClose = state->commandQueueDepth;
    result->f0302RotationInFlightAfterClose = state->f0302RotationInFlight;
    result->f0302RotationCommittedAfterClose = state->f0302RotationCommitted;
    result->f0282C160ClearCount = state->f0282C160ClearCount;
    result->f0282NonC160ClearCount = state->f0282NonC160ClearCount;
    result->f0302EnterCount = state->f0302EnterCount;
    result->f0333OpenCount = state->f0333OpenCount;
    result->f0334CloseCount = state->f0334CloseCount;
    result->f0345FoodWaterDrawCount = state->f0345FoodWaterDrawCount;
    result->f0346C040DrawCount = state->f0346C040DrawCount;
    result->f0347PanelDrawCount = state->f0347PanelDrawCount;
    result->f0355ToggleSuppressedByCandidateCount =
        state->f0355ToggleSuppressedByCandidateCount;
    result->f0359FreshClickCount = state->f0359FreshClickCount;
    result->f0360PendingReplayCount = state->f0360PendingReplayCount;
    result->f0378PanelRouteCount = state->f0378PanelRouteCount;
    result->f0380DrainCount = state->f0380DrainCount;
    result->f0098FloorCeilingDrawCount = state->f0098FloorCeilingDrawCount;
    result->f0326MousePointerRefreshCount =
        state->f0326MousePointerRefreshCount;
    result->saveLoadCount = state->saveLoadCount;
    result->teleporterCount = state->teleporterCount;
    result->partyRotateCount = state->partyRotateCount;
    result->c040PanelRerenderDuringCloseCount =
        state->c040PanelRerenderDuringCloseCount;
    result->c040PanelPixelHashAfterClose =
        state->c040PanelPixelHashAfterClose;
    result->chestHashAfter = state->chestHashAfter;
    result->afterCloseHash = state->afterCloseHash;
}

static void snapshot_after_commit(
    const Dm1V1MirrorCandidateC160CloseWhileRotationPendingStatePc34 *state,
    Dm1V1MirrorCandidateC160CloseWhileRotationPendingResultPc34 *result)
{
    int i;

    result->finalLeaderIndex = state->leaderIndex;
    result->pendingLeaderIndexAfterCommit = state->pendingLeaderIndex;
    result->commandQueueDepthAfterCommit = state->commandQueueDepth;
    result->f0302RotationCommittedAfterCommit =
        state->f0302RotationCommitted;
    result->f0302LeaderCommitCount = state->f0302LeaderCommitCount;
    result->f0367StatusBoxClickCount = state->f0367StatusBoxClickCount;
    result->f0368SetLeaderCount = state->f0368SetLeaderCount;
    result->afterRotationCommitHash = state->afterRotationCommitHash;
    for (i = 0; i < DM1_V1_MC_C160_ROTATION_CHEST_SLOT_COUNT_PC34; ++i) {
        result->chestSlotsAfter[i] = state->chestSlots[i];
    }
    for (i = 0; i < DM1_V1_MC_C160_ROTATION_TRACE_COUNT_PC34; ++i) {
        result->trace[i] = state->trace[i];
    }
}

int dm1_v1_mirror_candidate_c160_close_while_rotation_pending_run_pc34(
    Dm1V1MirrorCandidateC160CloseWhileRotationPendingStatePc34 *state,
    Dm1V1MirrorCandidateC160CloseWhileRotationPendingResultPc34 *result)
{
    int i;

    if (!state || !result) {
        return 0;
    }
    memset(result, 0, sizeof(*result));
    if (!enter_f0302_rotation_pending(state)) {
        return 0;
    }
    snapshot_before_close(state, result);
    if (!dispatch_fresh_c160_close(state)) {
        return 0;
    }
    snapshot_after_close(state, result);
    if (!commit_leader_rotation_after_close(state)) {
        return 0;
    }
    snapshot_after_commit(state, result);

    result->c160ClearsG0299DespiteRotationPending =
        result->f0302RotationInFlightBeforeClose == 1 &&
        result->f0302RotationCommittedAfterClose == 0 &&
        result->g0299BeforeClose == kCandidateOrdinal &&
        result->g0299AfterClose == 0 &&
        result->f0282C160ClearCount == 1;
    result->closeBypassesF0355CandidateGate =
        result->f0355ToggleSuppressedByCandidateCount == 0 &&
        result->f0378PanelRouteCount == 1 &&
        result->f0380DrainCount == 1;
    result->noC040RerenderDuringClose =
        result->f0346C040DrawCount == 1 &&
        result->f0347PanelDrawCount == 1 &&
        result->c040PanelRerenderDuringCloseCount == 0;
    result->rotationCommitsAfterClose =
        result->leaderIndexDuringClose == kLeader0 &&
        result->pendingLeaderIndexAfterClose == kQueuedLeader1 &&
        result->f0302RotationCommittedAfterClose == 0 &&
        result->finalLeaderIndex == kQueuedLeader1 &&
        result->pendingLeaderIndexAfterCommit == -1 &&
        result->f0302RotationCommittedAfterCommit == 1 &&
        result->f0302LeaderCommitCount == 1 &&
        result->f0368SetLeaderCount == 1;
    result->noChestOpenOrClose =
        result->f0333OpenCount == 0 && result->f0334CloseCount == 0;
    result->noExtraCandidateClear =
        result->f0282C160ClearCount == 1 &&
        result->f0282NonC160ClearCount == 0;
    result->noSaveLoadTeleporterPartyRotate =
        result->saveLoadCount == 0 &&
        result->teleporterCount == 0 &&
        result->partyRotateCount == 0;
    result->closeLandsInDungeonRefresh =
        result->f0098FloorCeilingDrawCount == 1 &&
        result->f0326MousePointerRefreshCount == 1;
    result->c040PanelPixelsStable =
        result->c040PanelPixelHashBefore ==
        result->c040PanelPixelHashAfterClose;
    result->chestStatePreserved =
        result->chestHashBefore == result->chestHashAfter;
    for (i = 0; i < DM1_V1_MC_C160_ROTATION_CHEST_SLOT_COUNT_PC34; ++i) {
        if (result->chestSlotsBefore[i] != result->chestSlotsAfter[i]) {
            result->chestStatePreserved = 0;
        }
    }
    result->sourceLockAnchorsPresent = source_anchors_present();
    result->guardRejectsNullState =
        dm1_v1_mirror_candidate_c160_close_while_rotation_pending_run_pc34(
            NULL, result) == 0;
    result->guardRejectsNullResult =
        dm1_v1_mirror_candidate_c160_close_while_rotation_pending_run_pc34(
            state, NULL) == 0;
    result->accepted =
        result->contractOnly &&
        result->noGameDataRequired &&
        result->c160ClearsG0299DespiteRotationPending &&
        result->closeBypassesF0355CandidateGate &&
        result->noC040RerenderDuringClose &&
        result->rotationCommitsAfterClose &&
        result->noChestOpenOrClose &&
        result->noExtraCandidateClear &&
        result->noSaveLoadTeleporterPartyRotate &&
        result->closeLandsInDungeonRefresh &&
        result->c040PanelPixelsStable &&
        result->chestStatePreserved &&
        result->sourceLockAnchorsPresent;
    result->deterministicHash =
        hash_step(result->afterRotationCommitHash,
                  (unsigned int)result->accepted);
    result->deterministicHash =
        hash_step(result->deterministicHash, result->afterCloseHash);
    result->deterministicHash =
        hash_step(result->deterministicHash, result->chestHashAfter);
    return result->accepted;
}

const Dm1V1MirrorCandidateC160CloseWhileRotationPendingEvidencePc34 *
dm1_v1_mirror_candidate_c160_close_while_rotation_pending_evidence_pc34(void)
{
    return &s_evidence;
}

const char *
dm1_v1_mirror_candidate_c160_close_while_rotation_pending_source_evidence_pc34(
    void)
{
    return s_source_evidence;
}
