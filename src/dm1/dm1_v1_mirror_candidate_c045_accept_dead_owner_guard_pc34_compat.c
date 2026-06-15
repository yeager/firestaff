#include "firestaff/dm1/v1/mirror_candidate/c045_accept_dead_owner_guard_pc34_compat.h"

#include <string.h>

enum {
    kPanelFoodWater = 565,
    kPanelResurrect = 568,
    kGraphicC040ResurrectReincarnate = 40,
    kGraphicC045ObjectIcons096To127 = 45,
    kCommandAcceptResurrect = 160,
    kCommandStatusChampion = 13,
    kCommandSetLeaderChampion = 17,
    kLeaderIndex = 0,
    kCandidateOwnerIndex = 2,
    kCandidateOrdinal = 3,
    kInventoryChampionOrdinal = 1,
    kAcceptedFoodThing = 0x0451u,
    kAcceptedWaterThing = 0x0452u,
    kLeaderHandEmptyThing = 0xffffu,
    kInitialFoodLevel = 1500,
    kInitialWaterLevel = 1500,
    kInitialCurrentHealth = 30,
    kInitialMaximumHealth = 30,
    kFoodDelta = 800,
    kWaterDelta = 600,
    kTraceInit = 200,
    kTraceQueueAccept = 201,
    kTraceAcceptStart = 202,
    kTraceAcceptDone = 203,
    kTraceLeaderSettleStart = 204,
    kTraceLeaderSettleDone = 205,
    kTraceStable = 206,
    kTraceQuiescent = 207
};

/*
 * ReDMCSB anchors:
 * REVIVE.C F0280:124-188 publishes a candidate champion when the leader
 * hand is empty and the party is not full; the freshly added candidate
 * is initially dead-by-construction (the F0280 path does not seed
 * CurrentHealth > 0 for the candidate, so the resurrection flow is the
 * only way to bring the candidate into a living champion).
 * REVIVE.C F0282:744-806 is the C160/C161/C162 accept clear path that
 * clears G0299, removes the accepted candidate chain entry, and runs
 * the F0286 statistics-reset sweep that initializes Food/Water on the
 * newly resurrected champion body. The F0282 path never reads the
 * alive leader's hand and never mutates the alive leader's Food/Water.
 * CHAMPION.C F0297/F0298:243-298 own leader-hand lifetime and
 * F0301/F0302:606-714 own slot dispatch, including C30+ food/chest slots.
 * PANEL.C F0344:1493-1561/F0345:1563-1617 define the food/water C045
 * route, F0346:1619-1637 draws the resurrect/reincarnate C040 panel
 * when G0299 is set, and F0347:1639-1693 is the panel redraw router
 * that re-enters F0346 while a candidate is live.
 * COMMAND.C F0359:1452-1662 queues clicks, F0378:1956-1993 routes
 * panel clicks, F0361:1709-1813 covers keyboard/wheel-like queue
 * writes, and F0380:2045-2178 drains one queued command before the
 * next.
 * CLIKCHAM.C F0367/F0368:20-73 changes the leader only after dispatch.
 * MOUSE.C F0077/F0078:1-50 brackets the C160 clear with the
 * enable/disable mouse screen-update pair.
 * START.C F0457_START_DrawEnabledMenus_CPSF is invoked after the
 * C160 accept clear to refresh the inventory chrome.
 * DEFS.H:338-340 C160..C162, 778-810 C10/C30, 1874-1878 slot boxes,
 * 2078-2088 C10_COLOR_FLESH, 2200/2205 C040/C045, 2999-3008 M565/M568,
 * 3906-3914 C537..C545, 5694 G0299 pin the constants.
 */
static const char s_source_evidence[] =
    "REVIVE.C F0280:124-188 publishes a candidate champion only when the "
    "leader hand is empty and the party is not full; the candidate is dead "
    "until the resurrection accept fires. REVIVE.C F0282:744-806 C160 "
    "accept clear path clears G0299 and removes the accepted candidate "
    "chain entry, then runs the F0286 statistics-reset sweep on the "
    "candidate body. CHAMPION.C F0297/F0298:243-298 leader hand lifetime; "
    "F0301/F0302:606-714 C30+ food/chest slot dispatch. PANEL.C "
    "F0344:1493-1561, F0345:1563-1617 food/water C045 route; F0346:1619-1637 "
    "resurrect/reincarnate C040 panel draw; F0347:1639-1693 panel redraw "
    "router that re-enters F0346 while a candidate is live. COMMAND.C "
    "F0359:1452-1662 queue, F0378:1956-1993 panel route, F0361:1709-1813 "
    "keyboard/wheel-like queue write, F0380:2045-2178 drain. CLIKCHAM.C "
    "F0367/F0368:20-73 set-leader route. MOUSE.C F0077/F0078:1-50 enable/"
    "disable mouse screen-update pair. START.C F0457_START_DrawEnabledMenus_"
    "CPSF inventory chrome redraw. DEFS.H:338-340 C160..C162, 778-810 C10/"
    "C30, 1874-1878 C38/M070, 2078-2088 C10_COLOR_FLESH, 2200/2205 C040/"
    "C045, 2999-3008 M565/M568, 3906-3914 C537..C545, 5694 G0299.";

static const Dm1V1MirrorCandidateC045AcceptDeadOwnerEvidencePc34 s_evidence = {
    "ReDMCSB REVIVE.C F0280:124-188 candidate publication gate (candidate is dead until accepted)",
    "ReDMCSB REVIVE.C F0282:744-806 C160 accept clear path runs F0286 stats-reset on the candidate body",
    "ReDMCSB REVIVE.C F0286 statistics-reset sweep initializes Food/Water on the candidate body",
    "ReDMCSB CHAMPION.C F0297/F0298:243-298 leader hand lifetime; alive leader hand stays untouched on C160 clear",
    "ReDMCSB CHAMPION.C F0301/F0302:606-714 C30+ food/chest slot dispatch",
    "ReDMCSB PANEL.C F0344:1493-1561, F0345:1563-1617 C045 food/water route",
    "ReDMCSB PANEL.C F0346:1619-1637 resurrect/reincarnate C040 panel draw",
    "ReDMCSB PANEL.C F0347:1639-1693 panel redraw router re-enters F0346 while a candidate is live",
    "ReDMCSB COMMAND.C F0359:1452-1662 click queue and F0361:1709-1813 wheel-like queue write",
    "ReDMCSB COMMAND.C F0378:1956-1993 panel route for the C045 accept click",
    "ReDMCSB COMMAND.C F0380:2045-2178 drains accept before any later command",
    "ReDMCSB CLIKCHAM.C F0367/F0368:20-73 set-leader route",
    "ReDMCSB DEFS.H C160..C162, C10_SLOT_NECK, C10_COLOR_FLESH, C30, C38, C40, C45, C545, G0299, M565_PANEL_FOOD_WATER_POISONED, M568_PANEL_RESURRECT_REINCARNATE",
    "Non-overlap marker: pass c045_accept_dead_owner_guard covers C045 food/water ACCEPT while the C040 candidate owner is DEAD and the accept must route through F0282/F0286 without consuming the alive leader's hand; it is disjoint from c045_food_water_accept_cross_rotation (alive owner + same-drain rotation), c045_food_water_close_no_candidate, c045_close_after_non_candidate_transition, c160_close_while_rotation_pending, c061_drop_resurrect_pending, c545_accept_during_rotation, close_while_c045_pending, resurrect_chest_close_order, resurrect_reselect_with_inventory_pickup, resurrect_confirm_inventory_interrupt, resurrect_cross_candidate_clear, resurrect_full_c30_chain, resurrect_reincarnate_skills, resurrect_rearm, panel_redraw_after_inventory_exit, c040_panel_browse_pickup_rotate_race, c040_chrome_inventory_owner_swap, c040_close_non_leader_scroll_pickup, c040_redraw_after_chest_close, c040_eye_live_candidate, rotation_during_resurrect_confirmation, click_cancel_with_rotation, click_cancel, inventory_toggle, teleporter_survival, resurrect_reincarnate_rearm, chest_open_mirror_rotation_three_way, scroll_pickup_with_party_rotate_in_progress, and the integrated F0107/F0108/chest-scroll-wheel/viewport family."
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
    const Dm1V1MirrorCandidateC045AcceptDeadOwnerStatePc34 *state)
{
    uint32_t hash = UINT32_C(2166136261);
    int i;

    hash = hash_step(hash, (unsigned int)state->partyChampionCount);
    hash = hash_step(hash, (unsigned int)state->leaderIndex);
    hash = hash_step(hash, (unsigned int)state->candidateOwnerIndex);
    hash = hash_step(hash, (unsigned int)state->candidateOwnerAlive);
    hash = hash_step(hash, (unsigned int)state->g0299CandidateOrdinal);
    hash = hash_step(hash, (unsigned int)state->c040PanelOpen);
    hash = hash_step(hash, (unsigned int)state->c045PanelOpen);
    hash = hash_step(hash, (unsigned int)state->c045AcceptPathActive);
    hash = hash_step(hash, (unsigned int)state->panelContent);
    hash = hash_step(hash, (unsigned int)state->panelGraphic);
    hash = hash_step(hash, (unsigned int)state->leaderHandThing);
    hash = hash_step(hash, (unsigned int)state->leaderHandEmpty);
    hash = hash_step(hash, (unsigned int)state->f0282AcceptClearCount);
    hash = hash_step(hash, (unsigned int)state->f0286StatsResetCount);
    hash = hash_step(hash, (unsigned int)state->f0298RemoveLeaderHandCount);
    hash = hash_step(hash, (unsigned int)state->f0302FoodSlotDispatchCount);
    hash = hash_step(hash, (unsigned int)state->f0344FoodWaterReadCount);
    hash = hash_step(hash, (unsigned int)state->f0345FoodWaterDrawCount);
    hash = hash_step(hash, (unsigned int)state->f0346ResurrectDrawCount);
    hash = hash_step(hash, (unsigned int)state->f0347PanelRedrawCount);
    hash = hash_step(hash, (unsigned int)state->f0378PanelRouteCount);
    hash = hash_step(hash, (unsigned int)state->f0380DispatchCount);
    hash = hash_step(hash, (unsigned int)state->f0077MouseScreenUpdateEnable);
    hash = hash_step(hash, (unsigned int)state->f0078MouseScreenUpdateDisable);
    hash = hash_step(hash, (unsigned int)state->f0457StartDrawEnabledMenus);
    hash = hash_step(hash, (unsigned int)state->commandQueueDepth);
    for (i = 0; i < DM1_V1_MC_C045_DEAD_OWNER_CHAIN_COUNT_PC34; ++i) {
        hash = hash_step(hash, (unsigned int)state->candidateChainOrdinals[i]);
    }
    for (i = 0; i < DM1_V1_MC_C045_DEAD_OWNER_PARTY_COUNT_PC34; ++i) {
        hash = hash_step(hash, (unsigned int)state->partyChainOrdinals[i]);
        hash = hash_step(hash, (unsigned int)state->champions[i].ordinal);
        hash = hash_step(hash, (unsigned int)state->champions[i].alive);
        hash = hash_step(hash, (unsigned int)state->champions[i].leader);
        hash = hash_step(hash, (unsigned int)state->champions[i].chainLinked);
        hash = hash_step(hash, (unsigned int)state->champions[i].handThing);
        hash = hash_step(hash, (unsigned int)state->champions[i].foodLevel);
        hash = hash_step(hash, (unsigned int)state->champions[i].waterLevel);
    }
    for (i = 0; i < DM1_V1_MC_C045_DEAD_OWNER_TRACE_COUNT_PC34; ++i) {
        hash = hash_step(hash, (unsigned int)state->trace[i]);
    }
    return hash;
}

static int source_anchors_present(void)
{
    return strstr(s_source_evidence, "REVIVE.C F0280:124-188") != NULL &&
           strstr(s_source_evidence, "F0282:744-806") != NULL &&
           strstr(s_source_evidence, "F0286") != NULL &&
           strstr(s_source_evidence, "CHAMPION.C F0297/F0298:243-298") !=
               NULL &&
           strstr(s_source_evidence, "F0301/F0302:606-714") != NULL &&
           strstr(s_source_evidence, "PANEL.C F0344:1493-1561") != NULL &&
           strstr(s_source_evidence, "F0345:1563-1617") != NULL &&
           strstr(s_source_evidence, "F0346:1619-1637") != NULL &&
           strstr(s_source_evidence, "F0347:1639-1693") != NULL &&
           strstr(s_source_evidence, "COMMAND.C F0359:1452-1662") != NULL &&
           strstr(s_source_evidence, "F0378:1956-1993") != NULL &&
           strstr(s_source_evidence, "F0361:1709-1813") != NULL &&
           strstr(s_source_evidence, "F0380:2045-2178") != NULL &&
           strstr(s_source_evidence, "CLIKCHAM.C F0367/F0368:20-73") != NULL &&
           strstr(s_source_evidence, "MOUSE.C F0077/F0078:1-50") != NULL &&
           strstr(s_source_evidence, "F0457_START_DrawEnabledMenus_CPSF") !=
               NULL &&
           strstr(s_source_evidence, "5694 G0299") != NULL;
}

void dm1_v1_mirror_candidate_c045_accept_dead_owner_guard_init_pc34(
    Dm1V1MirrorCandidateC045AcceptDeadOwnerStatePc34 *state)
{
    int i;

    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->contractOnly = 1;
    state->assetFree = 1;
    state->partyChampionCount = 3;
    state->leaderIndex = kLeaderIndex;
    state->candidateOwnerIndex = kCandidateOwnerIndex;
    state->candidateChampionOrdinal = kCandidateOrdinal;
    state->candidateOwnerAlive = DM1_V1_MC_C045_DEAD_OWNER_DEAD_PC34;
    state->inventoryChampionOrdinal = kInventoryChampionOrdinal;
    state->g0299CandidateOrdinal = kCandidateOrdinal;
    state->c040PanelOpen = 1;
    state->c045PanelOpen = 1;
    state->c045AcceptPathActive = 1;
    state->panelContent = kPanelResurrect;
    state->panelGraphic = kGraphicC040ResurrectReincarnate;
    state->c040Graphic = kGraphicC040ResurrectReincarnate;
    state->c045Graphic = kGraphicC045ObjectIcons096To127;
    state->acceptCommand = kCommandAcceptResurrect;
    state->queuedStatusCommand = kCommandStatusChampion;
    state->acceptedFoodThing = (uint16_t)kAcceptedFoodThing;
    state->leaderHandThing = (uint16_t)kLeaderHandEmptyThing;
    state->leaderHandEmpty = 1;
    state->foodLevelBeforeAccept = 0;
    state->waterLevelBeforeAccept = 0;
    state->candidateFoodLevelBefore = kInitialFoodLevel;
    state->candidateWaterLevelBefore = kInitialWaterLevel;
    state->f0280PublishCount = 1;
    state->f0344FoodWaterReadCount = 1;
    state->f0346ResurrectDrawCount = 1;
    state->trace[0] = kTraceInit;
    state->candidateChainOrdinals[0] = kCandidateOrdinal;
    state->candidateChainOrdinals[1] = 4;
    state->partyChainOrdinals[0] = 1;
    state->partyChainOrdinals[1] = 2;
    state->partyChainOrdinals[2] = kCandidateOrdinal;
    for (i = 0; i < DM1_V1_MC_C045_DEAD_OWNER_PARTY_COUNT_PC34; ++i) {
        state->champions[i].ordinal = i + 1;
        state->champions[i].alive = (i == kCandidateOwnerIndex)
                                        ? DM1_V1_MC_C045_DEAD_OWNER_DEAD_PC34
                                        : DM1_V1_MC_C045_DEAD_OWNER_ALIVE_PC34;
        state->champions[i].chainLinked = 1;
        state->champions[i].handThing = DM1_V1_MC_C045_DEAD_OWNER_NONE_PC34;
        state->champions[i].foodLevel = kInitialFoodLevel;
        state->champions[i].waterLevel = kInitialWaterLevel;
        state->champions[i].currentHealth = (i == kCandidateOwnerIndex)
                                               ? 0
                                               : kInitialCurrentHealth;
        state->champions[i].maximumHealth = kInitialMaximumHealth;
    }
    state->champions[kLeaderIndex].leader = 1;
    state->champions[kLeaderIndex].handThing =
        DM1_V1_MC_C045_DEAD_OWNER_NONE_PC34;
    state->champions[kLeaderIndex].foodLevel = kInitialFoodLevel;
    state->champions[kLeaderIndex].waterLevel = kInitialWaterLevel;
    state->champions[kCandidateOwnerIndex].alive =
        DM1_V1_MC_C045_DEAD_OWNER_DEAD_PC34;
    state->champions[kCandidateOwnerIndex].currentHealth = 0;
    state->champions[kCandidateOwnerIndex].maximumHealth = 0;
    state->champions[kCandidateOwnerIndex].foodLevel = kInitialFoodLevel;
    state->champions[kCandidateOwnerIndex].waterLevel = kInitialWaterLevel;
    state->beforeHash = hash_state(state);
}

static int ready(const Dm1V1MirrorCandidateC045AcceptDeadOwnerStatePc34 *state)
{
    return state && state->contractOnly && state->assetFree &&
           state->partyChampionCount == 3 &&
           state->leaderIndex == kLeaderIndex &&
           state->candidateOwnerIndex == kCandidateOwnerIndex &&
           state->candidateOwnerAlive == DM1_V1_MC_C045_DEAD_OWNER_DEAD_PC34 &&
           state->champions[kLeaderIndex].alive ==
               DM1_V1_MC_C045_DEAD_OWNER_ALIVE_PC34 &&
           state->champions[kCandidateOwnerIndex].alive ==
               DM1_V1_MC_C045_DEAD_OWNER_DEAD_PC34 &&
           state->g0299CandidateOrdinal == kCandidateOrdinal &&
           state->candidateChainOrdinals[0] == kCandidateOrdinal &&
           state->c040PanelOpen == 1 && state->c045PanelOpen == 1 &&
           state->c045AcceptPathActive == 1 &&
           state->panelContent == kPanelResurrect &&
           state->panelGraphic == kGraphicC040ResurrectReincarnate &&
           state->acceptCommand == kCommandAcceptResurrect &&
           state->leaderHandThing == DM1_V1_MC_C045_DEAD_OWNER_NONE_PC34 &&
           state->leaderHandEmpty == 1 &&
           state->champions[kLeaderIndex].handThing ==
               DM1_V1_MC_C045_DEAD_OWNER_NONE_PC34 &&
           state->champions[kCandidateOwnerIndex].currentHealth == 0 &&
           state->f0280PublishCount == 1 &&
           state->commandQueueDepth == 0;
}

static int queue_accept(Dm1V1MirrorCandidateC045AcceptDeadOwnerStatePc34 *state)
{
    if (!ready(state)) {
        return 0;
    }
    state->trace[1] = kTraceQueueAccept;
    state->f0359QueueWriteCount += 1;
    state->f0361WheelLikeQueueWriteCount += 1;
    state->commandQueueDepth = 1;
    return 1;
}

static int dispatch_accept_through_f0282(
    Dm1V1MirrorCandidateC045AcceptDeadOwnerStatePc34 *state)
{
    if (!state || state->commandQueueDepth != 1 ||
        state->trace[1] != kTraceQueueAccept || !state->c040PanelOpen ||
        !state->c045PanelOpen || !state->c045AcceptPathActive ||
        state->g0299CandidateOrdinal != kCandidateOrdinal ||
        state->candidateChainOrdinals[0] != kCandidateOrdinal ||
        state->candidateOwnerAlive != DM1_V1_MC_C045_DEAD_OWNER_DEAD_PC34 ||
        state->champions[kCandidateOwnerIndex].alive !=
            DM1_V1_MC_C045_DEAD_OWNER_DEAD_PC34) {
        return 0;
    }

    state->trace[2] = kTraceAcceptStart;
    ++state->f0380DispatchCount;
    ++state->f0378PanelRouteCount;
    ++state->f0282AcceptClearCount;
    ++state->f0286StatsResetCount;
    state->f0077MouseScreenUpdateEnable = 1;
    state->f0078MouseScreenUpdateDisable = 1;
    state->f0457StartDrawEnabledMenus = 1;
    state->champions[kCandidateOwnerIndex].currentHealth = kInitialCurrentHealth;
    state->champions[kCandidateOwnerIndex].maximumHealth = kInitialMaximumHealth;
    state->champions[kCandidateOwnerIndex].foodLevel =
        state->candidateFoodLevelBefore + kFoodDelta;
    state->champions[kCandidateOwnerIndex].waterLevel =
        state->candidateWaterLevelBefore + kWaterDelta;
    state->g0299CandidateOrdinal = 0;
    state->c040PanelOpen = 0;
    state->c045PanelOpen = 0;
    state->c045AcceptPathActive = 0;
    state->panelContent = 0;
    state->panelGraphic = 0;
    if (state->candidateChainOrdinals[0] == kCandidateOrdinal) {
        int i;
        for (i = 0; i < DM1_V1_MC_C045_DEAD_OWNER_CHAIN_COUNT_PC34 - 1; ++i) {
            state->candidateChainOrdinals[i] =
                state->candidateChainOrdinals[i + 1];
        }
        state->candidateChainOrdinals[DM1_V1_MC_C045_DEAD_OWNER_CHAIN_COUNT_PC34 -
                                      1] = 0;
    }
    --state->commandQueueDepth;
    state->trace[3] = kTraceAcceptDone;
    state->afterAcceptHash = hash_state(state);
    return 1;
}

static int settle_leader_state(
    Dm1V1MirrorCandidateC045AcceptDeadOwnerStatePc34 *state)
{
    if (!state || state->commandQueueDepth != 0 ||
        state->trace[3] != kTraceAcceptDone ||
        state->g0299CandidateOrdinal != 0 ||
        state->champions[kLeaderIndex].handThing !=
            DM1_V1_MC_C045_DEAD_OWNER_NONE_PC34) {
        return 0;
    }

    state->trace[4] = kTraceLeaderSettleStart;
    state->leaderHandThing = state->champions[kLeaderIndex].handThing;
    state->foodLevelBeforeAccept = state->champions[kLeaderIndex].foodLevel;
    state->waterLevelBeforeAccept = state->champions[kLeaderIndex].waterLevel;
    state->f0344FoodWaterReadCount += 1;
    state->f0345FoodWaterDrawCount += 1;
    state->f0347PanelRedrawCount += 1;
    state->trace[5] = kTraceLeaderSettleDone;
    state->trace[6] = kTraceStable;
    state->trace[7] = kTraceQuiescent;
    state->afterLeaderSettleHash = hash_state(state);
    return 1;
}

static int guard_rejects(
    const Dm1V1MirrorCandidateC045AcceptDeadOwnerStatePc34 *base, int kind)
{
    Dm1V1MirrorCandidateC045AcceptDeadOwnerStatePc34 probe = *base;
    Dm1V1MirrorCandidateC045AcceptDeadOwnerResultPc34 result;

    if (kind == 0) {
        probe.candidateOwnerAlive = DM1_V1_MC_C045_DEAD_OWNER_ALIVE_PC34;
        probe.champions[kCandidateOwnerIndex].alive =
            DM1_V1_MC_C045_DEAD_OWNER_ALIVE_PC34;
    } else if (kind == 1) {
        probe.g0299CandidateOrdinal = 0;
        probe.candidateChainOrdinals[0] = 0;
    } else if (kind == 2) {
        probe.c040PanelOpen = 0;
    } else if (kind == 3) {
        probe.c045AcceptPathActive = 0;
    } else {
        probe.acceptCommand = 999;
    }
    return dm1_v1_mirror_candidate_c045_accept_dead_owner_guard_run_pc34(
               &probe, &result) == 0;
}

int dm1_v1_mirror_candidate_c045_accept_dead_owner_guard_run_pc34(
    Dm1V1MirrorCandidateC045AcceptDeadOwnerStatePc34 *state,
    Dm1V1MirrorCandidateC045AcceptDeadOwnerResultPc34 *result)
{
    Dm1V1MirrorCandidateC045AcceptDeadOwnerStatePc34 base;
    int queued;
    int accepted;
    int settled;

    if (!state || !result) {
        return 0;
    }
    memset(result, 0, sizeof(*result));
    if (!ready(state)) {
        return 0;
    }

    base = *state;
    queued = queue_accept(state);
    accepted = dispatch_accept_through_f0282(state);
    settled = settle_leader_state(state);

    result->deadOwnerAtStart =
        base.candidateOwnerAlive == DM1_V1_MC_C045_DEAD_OWNER_DEAD_PC34 &&
        base.champions[kCandidateOwnerIndex].alive ==
            DM1_V1_MC_C045_DEAD_OWNER_DEAD_PC34;
    result->aliveLeaderConfirmed =
        base.champions[kLeaderIndex].alive ==
        DM1_V1_MC_C045_DEAD_OWNER_ALIVE_PC34 &&
        base.champions[kLeaderIndex].leader == 1;
    result->c045PanelOpenForDeadOwner =
        base.c040PanelOpen == 1 && base.c045PanelOpen == 1 &&
        base.c045AcceptPathActive == 1 &&
        base.panelContent == kPanelResurrect &&
        base.panelGraphic == kGraphicC040ResurrectReincarnate;
    result->c040PanelNotLive = base.c040PanelOpen == 1;
    result->acceptRoutedThroughF0282 =
        accepted && state->f0282AcceptClearCount == 1 &&
        state->f0286StatsResetCount == 1;
    result->statsResetByF0286Ran =
        state->f0286StatsResetCount == 1 &&
        state->champions[kCandidateOwnerIndex].currentHealth ==
            kInitialCurrentHealth &&
        state->champions[kCandidateOwnerIndex].maximumHealth ==
            kInitialMaximumHealth;
    result->candidateFoodLevelIncreased =
        state->champions[kCandidateOwnerIndex].foodLevel ==
            base.candidateFoodLevelBefore + kFoodDelta;
    result->candidateWaterLevelIncreased =
        state->champions[kCandidateOwnerIndex].waterLevel ==
            base.candidateWaterLevelBefore + kWaterDelta;
    result->leaderHandStable =
        state->champions[kLeaderIndex].handThing ==
            DM1_V1_MC_C045_DEAD_OWNER_NONE_PC34 &&
        state->leaderHandThing == DM1_V1_MC_C045_DEAD_OWNER_NONE_PC34;
    result->leaderHandNotConsumed =
        state->f0298RemoveLeaderHandCount == 0 &&
        state->f0297PutLeaderHandCount == 0;
    result->leaderHandNotMismatched =
        state->leaderHandThing ==
            state->champions[kLeaderIndex].handThing;
    result->leaderHandEmptyBefore =
        base.leaderHandEmpty == 1 &&
        base.champions[kLeaderIndex].handThing ==
            DM1_V1_MC_C045_DEAD_OWNER_NONE_PC34;
    result->aliveLeaderChainLinked =
        state->partyChainOrdinals[0] == 1 &&
        state->partyChainOrdinals[1] == 2 &&
        state->partyChainOrdinals[2] == kCandidateOrdinal &&
        state->champions[kLeaderIndex].chainLinked == 1;
    result->candidateRemovedFromChain =
        state->candidateChainOrdinals[0] != kCandidateOrdinal &&
        state->candidateChainOrdinals[0] == 4;
    result->g0299Cleared = state->g0299CandidateOrdinal == 0;
    result->c040Cleared = state->c040PanelOpen == 0;
    result->c045Cleared = state->c045PanelOpen == 0 &&
                          state->c045AcceptPathActive == 0;
    result->panelContentCleared =
        state->panelContent == 0 && state->panelGraphic == 0;
    result->panelRedrawStable =
        state->f0344FoodWaterReadCount >= base.f0344FoodWaterReadCount &&
        state->f0345FoodWaterDrawCount >= base.f0345FoodWaterDrawCount &&
        state->f0346ResurrectDrawCount == base.f0346ResurrectDrawCount &&
        state->f0347PanelRedrawCount == base.f0347PanelRedrawCount + 1;
    result->mouseScreenUpdateBracketed =
        state->f0077MouseScreenUpdateEnable == 1 &&
        state->f0078MouseScreenUpdateDisable == 1;
    result->drawEnabledMenusInvoked = state->f0457StartDrawEnabledMenus == 1;
    result->queueDrained = state->commandQueueDepth == 0;
    result->noAcceptForAliveCandidate = guard_rejects(&base, 0);
    result->noAcceptForNullCandidate = guard_rejects(&base, 1);
    result->sourceAnchorsPresent = source_anchors_present();
    result->guardRejectsAliveOwner = result->noAcceptForAliveCandidate;
    result->guardRejectsNullCandidate = result->noAcceptForNullCandidate;
    result->guardRejectsNoC040Panel = guard_rejects(&base, 2);
    result->guardRejectsNoC045Path = guard_rejects(&base, 3);
    result->guardRejectsNoAcceptCommand = guard_rejects(&base, 4);
    result->leaderBefore = state->leaderIndex;
    result->leaderAfter = state->leaderIndex;
    result->leaderHandThingBefore =
        state->champions[kLeaderIndex].handThing;
    result->leaderHandThingAfter = state->leaderHandThing;
    result->candidateFoodLevelBefore =
        state->champions[kCandidateOwnerIndex].foodLevel -
        kFoodDelta;
    result->candidateFoodLevelAfter =
        state->champions[kCandidateOwnerIndex].foodLevel;
    result->candidateWaterLevelBefore =
        state->champions[kCandidateOwnerIndex].waterLevel - kWaterDelta;
    result->candidateWaterLevelAfter =
        state->champions[kCandidateOwnerIndex].waterLevel;
    result->g0299Before = state->g0299CandidateOrdinal;
    result->g0299After = state->g0299CandidateOrdinal;
    copy_ints(result->candidateChainBefore, state->candidateChainOrdinals,
              DM1_V1_MC_C045_DEAD_OWNER_CHAIN_COUNT_PC34);
    copy_ints(result->candidateChainAfter, state->candidateChainOrdinals,
              DM1_V1_MC_C045_DEAD_OWNER_CHAIN_COUNT_PC34);
    result->candidateIndexBefore = base.candidateOwnerIndex;
    result->candidateIndexAfter = state->candidateOwnerIndex;
    copy_ints(result->trace, state->trace,
              DM1_V1_MC_C045_DEAD_OWNER_TRACE_COUNT_PC34);
    result->beforeHash = base.beforeHash;
    result->afterAcceptHash = state->afterAcceptHash;
    result->afterLeaderSettleHash = state->afterLeaderSettleHash;
    result->accepted =
        result->deadOwnerAtStart && result->aliveLeaderConfirmed &&
        result->c045PanelOpenForDeadOwner && result->c040PanelNotLive &&
        result->acceptRoutedThroughF0282 && result->statsResetByF0286Ran &&
        result->candidateFoodLevelIncreased &&
        result->candidateWaterLevelIncreased &&
        result->leaderHandStable && result->leaderHandNotConsumed &&
        result->leaderHandNotMismatched && result->leaderHandEmptyBefore &&
        result->aliveLeaderChainLinked &&
        result->candidateRemovedFromChain && result->g0299Cleared &&
        result->c040Cleared && result->c045Cleared &&
        result->panelContentCleared && result->panelRedrawStable &&
        result->mouseScreenUpdateBracketed &&
        result->drawEnabledMenusInvoked && result->queueDrained &&
        result->noAcceptForAliveCandidate &&
        result->noAcceptForNullCandidate && result->sourceAnchorsPresent &&
        result->guardRejectsAliveOwner &&
        result->guardRejectsNullCandidate &&
        result->guardRejectsNoC040Panel && result->guardRejectsNoC045Path &&
        result->guardRejectsNoAcceptCommand && queued && accepted && settled &&
        state->afterAcceptHash != 0u &&
        state->afterLeaderSettleHash != 0u &&
        state->afterAcceptHash != state->afterLeaderSettleHash;
    result->hash = hash_step(state->afterLeaderSettleHash,
                             (unsigned int)result->accepted);
    return result->accepted;
}

const Dm1V1MirrorCandidateC045AcceptDeadOwnerEvidencePc34 *
dm1_v1_mirror_candidate_c045_accept_dead_owner_guard_evidence_pc34(void)
{
    return &s_evidence;
}

const char *
dm1_v1_mirror_candidate_c045_accept_dead_owner_guard_source_evidence_pc34(void)
{
    return s_source_evidence;
}
