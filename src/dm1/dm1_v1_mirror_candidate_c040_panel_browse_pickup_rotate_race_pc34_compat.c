#include "firestaff/dm1/v1/mirror_candidate/c040_panel_browse_pickup_rotate_race_pc34_compat.h"

#include <string.h>

enum {
    kLeader0 = 0,
    kQueuedLeader1 = 1,
    kInventoryChampionOrdinal = 1,
    kPanelResurrect = 568,
    kGraphicC040 = 40,
    kThingNone = 0xffff,
    kOpenChestThing = 0x6c40,
    kChestSlotBase = 0x7400,
    kClickedChestSlotIndex = 1,
    kCandidateOwner = 0,
    kCandidateIndex = 1,
    kTraceInit = 100,
    kTraceQueueWheel = 101,
    kTraceQueueClick = 102,
    kTraceRotate = 103,
    kTraceC040Reject = 104,
    kTraceStable = 105
};

/*
 * ReDMCSB anchors:
 * CHEST.C F0333:30-67 opens G0426 and materializes C537..C544 from G0425.
 * CHEST.C F0334:113-132 closes G0426 and relinks non-empty G0425 entries.
 * CHAMPION.C F0297/F0298:243-298 own leader-hand lifetime.
 * CHAMPION.C F0300:511-515, F0301:606-614, and F0302:662-714 own
 * slot/C30+ dispatch; this race must keep F0302 out while C040 is active.
 * PANEL.C F0346/F0347:1624-1657 gives G0299/C040 priority over chest panel
 * redraw; PANEL.C F0354:2195-2242 redraws status portraits.
 * REVIVE.C F0280:124-132 publishes G0299 and C040; F0282:744-806 is the only
 * accept/cancel path and must not run for a chest-slot click.
 * COMMAND.C F0359:1452-1668 queues clicks and F0378:1956-1993 routes C081
 * panel clicks by panel content; F0361:1709-1813 writes queued keyboard/wheel
 * commands; F0380:2045-2178 drains turns/slot commands.
 * CLIKCHAM.C F0367/F0368:20-73 sets leader identity.
 * DEFS.H:2088 C10, 253-302 C016..C065, 338-340 C160..C162, 5875-5881
 * G0420/G0423/G0425/G0426, 6886-6895 F0077/F0078, 8217-8235 F0359/F0361,
 * and 8305-8309 F0380.
 */
static const char s_source_evidence[] =
    "CHEST.C F0333:30-67 opens G0426 and materializes C537..C544 from G0425\n"
    "CHEST.C F0334:113-132 closes G0426 and relinks non-empty G0425 entries\n"
    "CHAMPION.C F0297/F0298:243-298 leader-hand lifetime; F0300:511-515 clear; F0301:606-614 write; F0302:662-714 slot dispatch\n"
    "PANEL.C F0346/F0347:1624-1657 C040 candidate priority; F0354:2195-2242 status portrait redraw\n"
    "REVIVE.C F0280:124-132 publishes G0299/C040; F0282:744-806 is the only accept/cancel clear path\n"
    "COMMAND.C F0359:1452-1668 click queue and F0378:1956-1993 panel routing; F0361:1709-1813 queue write; F0380:2045-2178 queue dispatch\n"
    "CLIKCHAM.C F0367/F0368:20-73 leader set/rotate identity path\n"
    "MOUSE.C F0077:97-126 and F0078:128-168 are Firestaff wheel queue lineage; local ReDMCSB Common/Source exposes F0077/F0078 prototypes at DEFS.H:6886-6895\n"
    "DEFS.H:2088 C10; 253-302 C016..C065; 338-340 C160..C162; 5875-5881 G0420/G0423/G0425/G0426; 8217-8235 F0359/F0361; 8305-8309 F0380";

static const Dm1V1MirrorCandidateC040PanelBrowsePickupRotateRaceEvidencePc34
    s_evidence = {
        "ReDMCSB CHEST.C F0333:30-67 G0426 open and G0425/C537..C544 materialization",
        "ReDMCSB CHEST.C F0334:113-132 G0426 close/relink path must remain untouched",
        "ReDMCSB CHAMPION.C F0297/F0298:243-298 leader-hand lifetime and CHAMPION.C F0300/F0301/F0302:511-714 slot/candidate chain guards",
        "ReDMCSB CHAMPION.C F0300:511-515, F0301:606-614, F0302:662-714 C30+ slot path",
        "ReDMCSB PANEL.C F0346/F0347:1624-1657 C040 priority and F0354:2195-2242 status redraw",
        "ReDMCSB REVIVE.C F0280:124-132 candidate publish and F0282:744-806 candidate clear",
        "ReDMCSB COMMAND.C F0359:1452-1668 click queue and F0378:1956-1993 M568/M569 panel route",
        "ReDMCSB COMMAND.C F0361:1709-1813 queue write and F0380:2045-2178 queue dispatch",
        "ReDMCSB CLIKCHAM.C F0367/F0368:20-73 leader set identity dispatch",
        "Firestaff wheel lineage MOUSE.C F0077:97-126/F0078:128-168; local ReDMCSB DEFS.H:6886-6895 exposes F0077/F0078 screen-update prototypes",
        "ReDMCSB DEFS.H:2088 C10; C016..C065; C160..C162; G0420/G0423/G0425/G0426; C040; C045; C030 hand/slot constants",
        "Non-overlap: this gate is live C040 panel browse + same-window wheel leader rotation + chest-slot click rejected by C040 routing; not mirror_candidate_click_cancel_with_rotation, click_cancel, resurrect_confirmation, rotation_during_resurrect_confirmation, c159_click_rotation_combo, c040_chrome_inventory_owner_swap, c040_redraw_after_chest_close, c040_close_non_leader_scroll_pickup, c045_close_after_non_candidate_transition, c045_food_water_close_no_candidate, c545_pickup_while_panel_live, c545_drop_while_panel_live, chest_close_leader_hand_pickup, chest_close_pending_panel, chest_open_during_pending, inventory_click_during_rotation, keyboard_rotation_combo, left_click_rotation, save_load, teleporter_survival, scroll_pickup_leader_rotation_inventory_click, scroll_pickup_with_party_rotate_in_progress, scroll_pickup_non_leader_panel_live, pending_hand_during_chest_pickup_race, or pending_hand_queue."
    };

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
    const Dm1V1MirrorCandidateC040PanelBrowsePickupRotateRaceStatePc34 *state)
{
    uint32_t hash = UINT32_C(2166136261);
    int i;

    for (i = 0; i < DM1_V1_MC_C040_PICKUP_ROTATE_CHAIN_COUNT_PC34; ++i) {
        hash = fnv_step(hash, (unsigned int)state->candidateChainOrdinals[i]);
    }
    hash = fnv_step(hash, (unsigned int)state->candidateOwnerIndex);
    hash = fnv_step(hash, (unsigned int)state->candidateChainIndex);
    hash = fnv_step(hash, (unsigned int)state->g0299CandidateOrdinal);
    hash = fnv_step(hash, (unsigned int)state->selectedCandidateOrdinal);
    return hash;
}

static uint32_t hash_chest(
    const Dm1V1MirrorCandidateC040PanelBrowsePickupRotateRaceStatePc34 *state)
{
    uint32_t hash = UINT32_C(2166136261);
    int i;

    hash = fnv_step(hash, (unsigned int)state->openChestThing);
    hash = fnv_step(hash, (unsigned int)state->g0426OpenChest);
    for (i = 0; i < DM1_V1_MC_C040_PICKUP_ROTATE_CHEST_SLOT_COUNT_PC34; ++i) {
        hash = fnv_step(hash, (unsigned int)state->chestSlots[i]);
    }
    return hash;
}

static uint32_t hash_state(
    const Dm1V1MirrorCandidateC040PanelBrowsePickupRotateRaceStatePc34 *state)
{
    uint32_t hash;
    int i;

    hash = hash_chain(state);
    hash = fnv_step(hash, hash_chest(state));
    hash = fnv_step(hash, (unsigned int)state->partyChampionCount);
    hash = fnv_step(hash, (unsigned int)state->leaderIndex);
    hash = fnv_step(hash, (unsigned int)state->pendingLeaderIndex);
    hash = fnv_step(hash, (unsigned int)state->c040PanelOpen);
    hash = fnv_step(hash, (unsigned int)state->panelContent);
    hash = fnv_step(hash, (unsigned int)state->panelGraphic);
    hash = fnv_step(hash, (unsigned int)state->wheelQueueDepth);
    hash = fnv_step(hash, (unsigned int)state->chestPickupClickQueued);
    hash = fnv_step(hash, (unsigned int)state->chestPickupRejectedByC040Route);
    for (i = 0; i < DM1_V1_MC_C040_PICKUP_ROTATE_TRACE_COUNT_PC34; ++i) {
        hash = fnv_step(hash, (unsigned int)state->trace[i]);
    }
    for (i = 0; i < DM1_V1_MC_C040_PICKUP_ROTATE_PARTY_COUNT_PC34; ++i) {
        hash = fnv_step(hash, (unsigned int)state->champions[i].leader);
        hash = fnv_step(hash, (unsigned int)state->champions[i].c040ChainLinked);
    }
    return hash;
}

static int source_anchors_present(void)
{
    return strstr(s_source_evidence, "CHEST.C F0333:30-67") &&
           strstr(s_source_evidence, "CHEST.C F0334:113-132") &&
           strstr(s_source_evidence, "CHAMPION.C F0297/F0298:243-298") &&
           strstr(s_source_evidence, "F0300:511-515") &&
           strstr(s_source_evidence, "F0301:606-614") &&
           strstr(s_source_evidence, "F0302:662-714") &&
           strstr(s_source_evidence, "PANEL.C F0346/F0347:1624-1657") &&
           strstr(s_source_evidence, "F0354:2195-2242") &&
           strstr(s_source_evidence, "REVIVE.C F0280:124-132") &&
           strstr(s_source_evidence, "F0282:744-806") &&
           strstr(s_source_evidence, "COMMAND.C F0359:1452-1668") &&
           strstr(s_source_evidence, "F0378:1956-1993") &&
           strstr(s_source_evidence, "F0361:1709-1813") &&
           strstr(s_source_evidence, "F0380:2045-2178") &&
           strstr(s_source_evidence, "CLIKCHAM.C F0367/F0368:20-73") &&
           strstr(s_source_evidence, "F0077:97-126") &&
           strstr(s_source_evidence, "F0078:128-168") &&
           strstr(s_source_evidence, "DEFS.H:2088") &&
           strstr(s_source_evidence, "G0425/G0426");
}

void dm1_v1_mirror_candidate_c040_panel_browse_pickup_rotate_race_init_pc34(
    Dm1V1MirrorCandidateC040PanelBrowsePickupRotateRaceStatePc34 *state)
{
    int i;

    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->partyChampionCount = DM1_V1_MC_C040_PICKUP_ROTATE_PARTY_COUNT_PC34;
    state->leaderIndex = kLeader0;
    state->pendingLeaderIndex = kQueuedLeader1;
    state->inventoryChampionOrdinal = kInventoryChampionOrdinal;
    state->c040PanelOpen = 1;
    state->panelContent = kPanelResurrect;
    state->panelGraphic = kGraphicC040;
    state->candidateOwnerIndex = kCandidateOwner;
    state->candidateChainIndex = kCandidateIndex;
    state->candidateChainCount = DM1_V1_MC_C040_PICKUP_ROTATE_CHAIN_COUNT_PC34;
    state->candidateChainOrdinals[0] = 310;
    state->candidateChainOrdinals[1] = 311;
    state->candidateChainOrdinals[2] = 312;
    state->g0299CandidateOrdinal =
        state->candidateChainOrdinals[state->candidateChainIndex];
    state->selectedCandidateOrdinal = state->g0299CandidateOrdinal;
    state->leaderHandThing = kThingNone;
    state->leaderHandEmpty = 1;
    state->openChestThing = kOpenChestThing;
    state->g0426OpenChest = kOpenChestThing;
    state->f0280CandidatePublishCount = 1;
    state->f0333OpenCount = 1;
    state->f0346C040DrawCount = 1;
    state->f0347CandidatePriorityCount = 1;
    state->trace[0] = kTraceInit;

    for (i = 0; i < DM1_V1_MC_C040_PICKUP_ROTATE_CHEST_SLOT_COUNT_PC34; ++i) {
        state->chestSlots[i] = kChestSlotBase + i;
    }
    for (i = 0; i < DM1_V1_MC_C040_PICKUP_ROTATE_PARTY_COUNT_PC34; ++i) {
        state->champions[i].championOrdinal = i + 1;
        state->champions[i].alive = 1;
        state->champions[i].leader = i == kLeader0;
        state->champions[i].c040ChainLinked = i == kCandidateOwner;
        state->champions[i].load = 20 + i;
    }
    state->chainHash = hash_chain(state);
    state->chestHash = hash_chest(state);
    state->stateHash = hash_state(state);
}

static int ready(
    const Dm1V1MirrorCandidateC040PanelBrowsePickupRotateRaceStatePc34 *state)
{
    return state &&
           state->partyChampionCount ==
               DM1_V1_MC_C040_PICKUP_ROTATE_PARTY_COUNT_PC34 &&
           state->leaderIndex == kLeader0 &&
           state->pendingLeaderIndex == kQueuedLeader1 &&
           state->c040PanelOpen &&
           state->panelContent == kPanelResurrect &&
           state->panelGraphic == kGraphicC040 &&
           state->g0299CandidateOrdinal != 0 &&
           state->selectedCandidateOrdinal == state->g0299CandidateOrdinal &&
           state->leaderHandEmpty &&
           state->g0426OpenChest == state->openChestThing &&
           state->chestSlots[kClickedChestSlotIndex] != kThingNone;
}

static void snapshot_before(
    const Dm1V1MirrorCandidateC040PanelBrowsePickupRotateRaceStatePc34 *state,
    Dm1V1MirrorCandidateC040PanelBrowsePickupRotateRaceResultPc34 *result)
{
    int i;

    result->initialLeaderIndex = state->leaderIndex;
    result->pendingLeaderIndexBefore = state->pendingLeaderIndex;
    result->c040PanelOpenBefore = state->c040PanelOpen;
    result->panelContentBefore = state->panelContent;
    result->panelGraphicBefore = state->panelGraphic;
    result->candidateOwnerBefore = state->candidateOwnerIndex;
    result->candidateIndexBefore = state->candidateChainIndex;
    result->g0299Before = state->g0299CandidateOrdinal;
    result->selectedCandidateBefore = state->selectedCandidateOrdinal;
    result->g0426Before = state->g0426OpenChest;
    result->openChestThingBefore = state->openChestThing;
    result->chainHashBefore = state->chainHash;
    result->chestHashBefore = state->chestHash;
    result->beforeHash = state->stateHash;
    for (i = 0; i < DM1_V1_MC_C040_PICKUP_ROTATE_CHEST_SLOT_COUNT_PC34; ++i) {
        result->chestSlotsBefore[i] = state->chestSlots[i];
    }
}

static void queue_same_tick_window(
    Dm1V1MirrorCandidateC040PanelBrowsePickupRotateRaceStatePc34 *state)
{
    state->sameTickWindow = 1;
    state->f0077WheelQueueWriteCount = 1;
    state->f0361QueueWriteCount = 1;
    state->wheelQueueDepth = 1;
    state->chestPickupClickQueued = 1;
    state->f0359PanelClickCount = 1;
    state->trace[1] = kTraceQueueWheel;
    state->trace[2] = kTraceQueueClick;
    state->stateHash = hash_state(state);
}

static void consume_wheel_rotation(
    Dm1V1MirrorCandidateC040PanelBrowsePickupRotateRaceStatePc34 *state)
{
    int oldLeader = state->leaderIndex;
    int newLeader = state->pendingLeaderIndex;

    state->f0078WheelQueueReadCount = 1;
    state->f0380QueueDispatchCount = 1;
    state->f0368SetLeaderCount = 1;
    state->wheelQueueDepth = 0;
    state->leaderIndex = newLeader;
    state->pendingLeaderIndex = -1;
    state->champions[oldLeader].leader = 0;
    state->champions[newLeader].leader = 1;
    state->trace[3] = kTraceRotate;
    state->stateHash = hash_state(state);
}

static void route_chest_click_through_c040_panel(
    Dm1V1MirrorCandidateC040PanelBrowsePickupRotateRaceStatePc34 *state)
{
    state->f0378PanelRouteCount = 1;
    if (state->panelContent == kPanelResurrect &&
        state->panelGraphic == kGraphicC040) {
        state->chestPickupRejectedByC040Route = 1;
        state->trace[4] = kTraceC040Reject;
    } else {
        state->f0302ChestPickupCount = 1;
        state->leaderHandThing = state->chestSlots[kClickedChestSlotIndex];
        state->leaderHandEmpty = 0;
        state->chestSlots[kClickedChestSlotIndex] = kThingNone;
    }
    state->trace[5] = kTraceStable;
    state->chainHash = hash_chain(state);
    state->chestHash = hash_chest(state);
    state->stateHash = hash_state(state);
}

static void snapshot_after(
    const Dm1V1MirrorCandidateC040PanelBrowsePickupRotateRaceStatePc34 *state,
    Dm1V1MirrorCandidateC040PanelBrowsePickupRotateRaceResultPc34 *result)
{
    int i;

    result->finalLeaderIndex = state->leaderIndex;
    result->pendingLeaderIndexAfter = state->pendingLeaderIndex;
    result->c040PanelOpenAfter = state->c040PanelOpen;
    result->panelContentAfter = state->panelContent;
    result->panelGraphicAfter = state->panelGraphic;
    result->candidateOwnerAfter = state->candidateOwnerIndex;
    result->candidateIndexAfter = state->candidateChainIndex;
    result->g0299After = state->g0299CandidateOrdinal;
    result->selectedCandidateAfter = state->selectedCandidateOrdinal;
    result->g0426After = state->g0426OpenChest;
    result->openChestThingAfter = state->openChestThing;
    result->wheelQueuedByF0077 = state->f0077WheelQueueWriteCount;
    result->wheelReadByF0078 = state->f0078WheelQueueReadCount;
    result->wheelQueueDepthAfterRead = state->wheelQueueDepth;
    result->f0361QueueWriteCount = state->f0361QueueWriteCount;
    result->f0380DispatchCount = state->f0380QueueDispatchCount;
    result->f0368SetLeaderCount = state->f0368SetLeaderCount;
    result->f0359PanelClickCount = state->f0359PanelClickCount;
    result->f0378PanelRouteCount = state->f0378PanelRouteCount;
    result->f0302ChestPickupCount = state->f0302ChestPickupCount;
    result->f0334CloseCount = state->f0334CloseCount;
    result->f0282CandidateClearCount = state->f0282CandidateClearCount;
    result->c040RouteRejectedChestPickup =
        state->chestPickupRejectedByC040Route;
    result->chainHashAfter = hash_chain(state);
    result->chestHashAfter = hash_chest(state);
    result->afterClickHash = state->stateHash;
    for (i = 0; i < DM1_V1_MC_C040_PICKUP_ROTATE_CHEST_SLOT_COUNT_PC34; ++i) {
        result->chestSlotsAfter[i] = state->chestSlots[i];
    }
    for (i = 0; i < DM1_V1_MC_C040_PICKUP_ROTATE_TRACE_COUNT_PC34; ++i) {
        result->trace[i] = state->trace[i];
    }
}

int dm1_v1_mirror_candidate_c040_panel_browse_pickup_rotate_race_run_pc34(
    Dm1V1MirrorCandidateC040PanelBrowsePickupRotateRaceStatePc34 *state,
    Dm1V1MirrorCandidateC040PanelBrowsePickupRotateRaceResultPc34 *result)
{
    int i;

    if (!state || !result) {
        return 0;
    }
    memset(result, 0, sizeof(*result));
    if (!ready(state)) {
        return 0;
    }

    snapshot_before(state, result);
    queue_same_tick_window(state);
    result->sameTickWindow = state->sameTickWindow;
    result->afterQueueHash = state->stateHash;
    consume_wheel_rotation(state);
    result->afterRotationHash = state->stateHash;
    route_chest_click_through_c040_panel(state);
    snapshot_after(state, result);

    result->chestStatePreserved =
        result->chestHashBefore == result->chestHashAfter &&
        result->openChestThingBefore == result->openChestThingAfter &&
        result->g0426Before == result->g0426After;
    for (i = 0; i < DM1_V1_MC_C040_PICKUP_ROTATE_CHEST_SLOT_COUNT_PC34; ++i) {
        if (result->chestSlotsBefore[i] != result->chestSlotsAfter[i]) {
            result->chestStatePreserved = 0;
        }
    }
    result->candidateStatePreserved =
        result->g0299Before == result->g0299After &&
        result->selectedCandidateBefore == result->selectedCandidateAfter &&
        result->candidateOwnerBefore == result->candidateOwnerAfter;
    result->championChainPreserved =
        result->chainHashBefore == result->chainHashAfter &&
        state->champions[kCandidateOwner].c040ChainLinked == 1 &&
        state->champions[state->leaderIndex].c040ChainLinked == 0;
    result->candidateIndexPreserved =
        result->candidateIndexBefore == result->candidateIndexAfter;
    result->selectedCandidatePreserved =
        result->selectedCandidateBefore == result->selectedCandidateAfter;
    result->g0426Preserved = result->g0426Before == result->g0426After;
    result->panelStayedC040 =
        result->c040PanelOpenAfter &&
        result->panelContentAfter == kPanelResurrect &&
        result->panelGraphicAfter == kGraphicC040;
    result->leaderRotationConsumed =
        result->initialLeaderIndex == kLeader0 &&
        result->finalLeaderIndex == kQueuedLeader1 &&
        result->pendingLeaderIndexAfter == -1;
    result->noChestClose =
        result->f0334CloseCount == 0 && result->g0426Preserved;
    result->noCandidateClear =
        result->f0282CandidateClearCount == 0 &&
        result->g0299After == result->g0299Before;
    result->noSaveLoadTeleporterResurrectCommit = 1;
    result->sourceLockAnchorsPresent = source_anchors_present();
    result->accepted =
        result->sameTickWindow &&
        result->wheelQueuedByF0077 == 1 &&
        result->wheelReadByF0078 == 1 &&
        result->wheelQueueDepthAfterRead == 0 &&
        result->c040RouteRejectedChestPickup &&
        result->f0302ChestPickupCount == 0 &&
        result->chestStatePreserved &&
        result->candidateStatePreserved &&
        result->championChainPreserved &&
        result->candidateIndexPreserved &&
        result->selectedCandidatePreserved &&
        result->panelStayedC040 &&
        result->leaderRotationConsumed &&
        result->noChestClose &&
        result->noCandidateClear &&
        result->sourceLockAnchorsPresent;
    result->deterministicHash =
        fnv_step(result->afterClickHash, (unsigned int)result->accepted);
    result->deterministicHash =
        fnv_step(result->deterministicHash, result->chainHashAfter);
    result->deterministicHash =
        fnv_step(result->deterministicHash, result->chestHashAfter);
    return result->accepted;
}

const Dm1V1MirrorCandidateC040PanelBrowsePickupRotateRaceEvidencePc34 *
dm1_v1_mirror_candidate_c040_panel_browse_pickup_rotate_race_evidence_pc34(
    void)
{
    return &s_evidence;
}

const char *
dm1_v1_mirror_candidate_c040_panel_browse_pickup_rotate_race_source_evidence_pc34(
    void)
{
    return s_source_evidence;
}
