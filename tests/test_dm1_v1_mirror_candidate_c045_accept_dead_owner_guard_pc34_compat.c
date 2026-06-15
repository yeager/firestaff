#include "firestaff/dm1/v1/mirror_candidate/c045_accept_dead_owner_guard_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static void check_true(int condition, const char *message, const char *anchor)
{
    ++g_assertions;
    if (!condition) {
        ++g_failures;
        printf("FAIL %s [%s]\n", message, anchor ? anchor : "(null)");
    }
}

static void check_int_eq(int actual, int expected, const char *message,
                         const char *anchor)
{
    ++g_assertions;
    if (actual != expected) {
        ++g_failures;
        printf("FAIL %s actual=%d expected=%d [%s]\n", message, actual,
               expected, anchor ? anchor : "(null)");
    }
}

static void check_u16_eq(uint16_t actual, uint16_t expected,
                         const char *message, const char *anchor)
{
    ++g_assertions;
    if (actual != expected) {
        ++g_failures;
        printf("FAIL %s actual=0x%04x expected=0x%04x [%s]\n", message,
               actual, expected, anchor ? anchor : "(null)");
    }
}

static void check_contains(const char *haystack, const char *needle,
                           const char *message, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || !strstr(haystack, needle)) {
        ++g_failures;
        printf("FAIL %s missing=%s [%s]\n", message, needle ? needle : "(null)",
               anchor ? anchor : "(null)");
    }
}

static void test_evidence(void)
{
    const Dm1V1MirrorCandidateC045AcceptDeadOwnerEvidencePc34 *e =
        dm1_v1_mirror_candidate_c045_accept_dead_owner_guard_evidence_pc34();
    const char *text =
        dm1_v1_mirror_candidate_c045_accept_dead_owner_guard_source_evidence_pc34();
    const char *siblings[] = {
        "c045_food_water_accept_cross_rotation",
        "c045_food_water_close_no_candidate",
        "c045_close_after_non_candidate_transition",
        "c160_close_while_rotation_pending",
        "c061_drop_resurrect_pending",
        "c545_accept_during_rotation",
        "close_while_c045_pending",
        "resurrect_chest_close_order",
        "resurrect_reselect_with_inventory_pickup",
        "resurrect_confirm_inventory_interrupt",
        "resurrect_cross_candidate_clear",
        "resurrect_full_c30_chain",
        "resurrect_reincarnate_skills",
        "resurrect_rearm",
        "panel_redraw_after_inventory_exit",
        "c040_panel_browse_pickup_rotate_race",
        "c040_chrome_inventory_owner_swap",
        "c040_close_non_leader_scroll_pickup",
        "c040_redraw_after_chest_close",
        "c040_eye_live_candidate",
        "rotation_during_resurrect_confirmation",
        "click_cancel_with_rotation",
        "click_cancel",
        "inventory_toggle",
        "teleporter_survival",
        "resurrect_reincarnate_rearm",
        "chest_open_mirror_rotation_three_way",
        "scroll_pickup_with_party_rotate_in_progress"
    };
    int i;

    check_true(e != NULL, "evidence accessor", "c045_accept_dead_owner_guard");
    check_contains(e->reviveAddCandidateAnchor, "F0280:124-188",
                   "F0280 anchor", e->reviveAddCandidateAnchor);
    check_contains(e->reviveAcceptClearAnchor, "F0282:744-806", "F0282 anchor",
                   e->reviveAcceptClearAnchor);
    check_contains(e->reviveStatsResetAnchor, "F0286", "F0286 anchor",
                   e->reviveStatsResetAnchor);
    check_contains(e->championHandAnchor, "F0297/F0298:243-298",
                   "leader hand anchor", e->championHandAnchor);
    check_contains(e->championSlotAnchor, "F0301/F0302:606-714",
                   "slot anchor", e->championSlotAnchor);
    check_contains(e->panelFoodWaterAnchor, "F0344:1493-1561",
                   "F0344 anchor", e->panelFoodWaterAnchor);
    check_contains(e->panelFoodWaterAnchor, "F0345:1563-1617",
                   "F0345 anchor", e->panelFoodWaterAnchor);
    check_contains(e->panelResurrectAnchor, "F0346:1619-1637",
                   "F0346 anchor", e->panelResurrectAnchor);
    check_contains(e->panelRedrawAnchor, "F0347:1639-1693",
                   "F0347 anchor", e->panelRedrawAnchor);
    check_contains(e->commandQueueAnchor, "F0359:1452-1662",
                   "F0359 anchor", e->commandQueueAnchor);
    check_contains(e->commandQueueAnchor, "F0361:1709-1813",
                   "F0361 anchor", e->commandQueueAnchor);
    check_contains(e->commandPanelRouteAnchor, "F0378:1956-1993",
                   "F0378 anchor", e->commandPanelRouteAnchor);
    check_contains(e->commandDrainAnchor, "F0380:2045-2178",
                   "F0380 anchor", e->commandDrainAnchor);
    check_contains(e->leaderSetAnchor, "CLIKCHAM.C F0367/F0368:20-73",
                   "leader set anchor", e->leaderSetAnchor);
    check_contains(e->defsAnchor, "C160..C162", "C160 defs", e->defsAnchor);
    check_contains(e->defsAnchor, "C10_SLOT_NECK", "C10 slot defs",
                   e->defsAnchor);
    check_contains(e->defsAnchor, "C10_COLOR_FLESH", "C10 color defs",
                   e->defsAnchor);
    check_contains(e->defsAnchor, "C30", "C30 defs", e->defsAnchor);
    check_contains(e->defsAnchor, "C40", "C40 defs", e->defsAnchor);
    check_contains(e->defsAnchor, "C45", "C45 defs", e->defsAnchor);
    check_contains(e->defsAnchor, "C545", "C545 defs", e->defsAnchor);
    check_contains(e->defsAnchor, "G0299", "G0299 defs", e->defsAnchor);
    check_contains(e->defsAnchor, "M565_PANEL_FOOD_WATER_POISONED",
                   "M565 defs", e->defsAnchor);
    check_contains(e->defsAnchor, "M568_PANEL_RESURRECT_REINCARNATE",
                   "M568 defs", e->defsAnchor);
    check_contains(e->nonOverlap, "DEAD", "dead owner non-overlap marker",
                   e->nonOverlap);
    check_contains(e->nonOverlap, "F0282/F0286", "F0282/F0286 non-overlap marker",
                   e->nonOverlap);
    for (i = 0; i < (int)(sizeof(siblings) / sizeof(siblings[0])); ++i) {
        check_contains(e->nonOverlap, siblings[i],
                       "required non-overlap self-assert", e->nonOverlap);
    }
    check_contains(text, "REVIVE.C F0280:124-188", "source F0280", text);
    check_contains(text, "F0282:744-806", "source F0282", text);
    check_contains(text, "F0286", "source F0286", text);
    check_contains(text, "CHAMPION.C F0297/F0298:243-298", "source F0297/F0298",
                   text);
    check_contains(text, "F0301/F0302:606-714", "source F0301/F0302", text);
    check_contains(text, "PANEL.C F0344:1493-1561", "source F0344", text);
    check_contains(text, "F0345:1563-1617", "source F0345", text);
    check_contains(text, "F0346:1619-1637", "source F0346", text);
    check_contains(text, "F0347:1639-1693", "source F0347", text);
    check_contains(text, "COMMAND.C F0359:1452-1662", "source F0359", text);
    check_contains(text, "F0378:1956-1993", "source F0378", text);
    check_contains(text, "F0361:1709-1813", "source F0361", text);
    check_contains(text, "F0380:2045-2178", "source F0380", text);
    check_contains(text, "CLIKCHAM.C F0367/F0368:20-73", "source CLIKCHAM",
                   text);
    check_contains(text, "MOUSE.C F0077/F0078:1-50", "source F0077/F0078", text);
    check_contains(text, "F0457_START_DrawEnabledMenus_CPSF",
                   "source F0457", text);
    check_contains(text, "5694 G0299", "source G0299", text);
}

static void test_initial_state(void)
{
    Dm1V1MirrorCandidateC045AcceptDeadOwnerStatePc34 state;
    int i;

    dm1_v1_mirror_candidate_c045_accept_dead_owner_guard_init_pc34(&state);
    check_int_eq(state.contractOnly, 1, "contract-only", "asset-free");
    check_int_eq(state.assetFree, 1, "asset-free", "no game data");
    check_int_eq(state.partyChampionCount, 3, "party champion count",
                 "REVIVE.C F0280");
    check_int_eq(state.leaderIndex, 0, "leader index",
                 "CLIKCHAM.C F0368");
    check_int_eq(state.candidateOwnerIndex, 2, "candidate owner index",
                 "REVIVE.C F0280");
    check_int_eq(state.candidateChampionOrdinal, 3, "candidate ordinal",
                 "G0299");
    check_int_eq(state.candidateOwnerAlive, 0, "candidate owner dead at start",
                 "REVIVE.C F0280");
    check_int_eq(state.inventoryChampionOrdinal, 1, "inventory ordinal",
                 "G0423");
    check_int_eq(state.g0299CandidateOrdinal, 3, "G0299 candidate",
                 "DEFS.H:5694");
    check_int_eq(state.c040PanelOpen, 1, "C040 panel live", "C040");
    check_int_eq(state.c045PanelOpen, 1, "C045 panel live", "C045");
    check_int_eq(state.c045AcceptPathActive, 1, "C045 accept path active",
                 "PANEL.C F0347");
    check_int_eq(state.panelContent, 568, "resurrect panel content",
                 "M568_PANEL_RESURRECT_REINCARNATE");
    check_int_eq(state.panelGraphic, 40, "C040 graphic", "C040");
    check_int_eq(state.c040Graphic, 40, "C040 graphic field", "C040");
    check_int_eq(state.c045Graphic, 45, "C045 graphic field", "C045");
    check_int_eq(state.acceptCommand, 160, "accept command", "C160");
    check_int_eq(state.queuedStatusCommand, 13, "status command C013",
                 "COMMAND.C F0380");
    check_u16_eq(state.acceptedFoodThing, 0x0451u, "accepted food thing",
                 "C045");
    check_u16_eq(state.leaderHandThing, 0xffffu, "leader hand empty", "F0298");
    check_int_eq(state.leaderHandEmpty, 1, "leader hand empty flag",
                 "CHAMPION.C F0297");
    check_int_eq(state.candidateFoodLevelBefore, 1500, "candidate food level",
                 "REVIVE.C F0280");
    check_int_eq(state.candidateWaterLevelBefore, 1500, "candidate water level",
                 "REVIVE.C F0280");
    check_int_eq(state.f0280PublishCount, 1, "candidate published",
                 "REVIVE.C F0280");
    check_int_eq(state.f0282AcceptClearCount, 0, "no accept clear yet",
                 "REVIVE.C F0282");
    check_int_eq(state.f0286StatsResetCount, 0, "no stats reset yet",
                 "REVIVE.C F0286");
    check_int_eq(state.f0077MouseScreenUpdateEnable, 0, "F0077 baseline",
                 "MOUSE.C F0077");
    check_int_eq(state.f0078MouseScreenUpdateDisable, 0, "F0078 baseline",
                 "MOUSE.C F0078");
    check_int_eq(state.f0457StartDrawEnabledMenus, 0, "F0457 baseline",
                 "START.C F0457");
    check_int_eq(state.commandQueueDepth, 0, "empty queue", "COMMAND.C F0380");
    check_int_eq(state.trace[0], 200, "trace init", "determinism");
    check_int_eq(state.candidateChainOrdinals[0], 3, "candidate in chain",
                 "REVIVE.C F0280");
    check_int_eq(state.candidateChainOrdinals[1], 4, "candidate chain tail",
                 "REVIVE.C F0282");
    check_int_eq(state.partyChainOrdinals[0], 1, "party chain leader",
                 "champion chain");
    check_int_eq(state.partyChainOrdinals[1], 2, "party chain second",
                 "champion chain");
    check_int_eq(state.partyChainOrdinals[2], 3, "party chain candidate",
                 "champion chain");
    check_true(state.beforeHash != 0u, "initial hash nonzero", "determinism");
    check_int_eq(state.champions[0].leader, 1, "leader marked",
                 "CLIKCHAM.C F0368");
    check_int_eq(state.champions[0].alive, 1, "alive leader", "F0280");
    check_int_eq(state.champions[2].alive, 0, "dead candidate owner",
                 "REVIVE.C F0280");
    check_int_eq(state.champions[2].currentHealth, 0,
                 "dead candidate owner current health", "REVIVE.C F0280");
    check_int_eq(state.champions[2].maximumHealth, 0,
                 "dead candidate owner maximum health", "REVIVE.C F0280");
    check_u16_eq(state.champions[0].handThing, 0xffffu,
                 "alive leader starts with empty hand", "CHAMPION.C F0297");
    for (i = 0; i < 3; ++i) {
        check_int_eq(state.champions[i].chainLinked, 1,
                     "party champion chain linked", "champion chain");
    }
}

static uint32_t test_run(void)
{
    Dm1V1MirrorCandidateC045AcceptDeadOwnerStatePc34 state;
    Dm1V1MirrorCandidateC045AcceptDeadOwnerResultPc34 result;
    int ok;
    int i;

    dm1_v1_mirror_candidate_c045_accept_dead_owner_guard_init_pc34(&state);
    ok = dm1_v1_mirror_candidate_c045_accept_dead_owner_guard_run_pc34(&state,
                                                                       &result);
    check_int_eq(ok, 1, "run accepted",
                 "c045_accept_dead_owner_guard");
    check_int_eq(result.accepted, 1, "result accepted", "contract");
    check_int_eq(result.deadOwnerAtStart, 1, "dead owner at start",
                 "REVIVE.C F0280");
    check_int_eq(result.aliveLeaderConfirmed, 1, "alive leader confirmed",
                 "CLIKCHAM.C F0368");
    check_int_eq(result.c045PanelOpenForDeadOwner, 1,
                 "C045 panel open for dead owner", "PANEL.C F0346");
    check_int_eq(result.c040PanelNotLive, 1, "C040 panel live at start",
                 "C040");
    check_int_eq(result.acceptRoutedThroughF0282, 1,
                 "accept routed through F0282", "REVIVE.C F0282");
    check_int_eq(result.statsResetByF0286Ran, 1, "F0286 stats reset ran",
                 "REVIVE.C F0286");
    check_int_eq(result.candidateFoodLevelIncreased, 1,
                 "candidate food level increased", "REVIVE.C F0282/F0286");
    check_int_eq(result.candidateWaterLevelIncreased, 1,
                 "candidate water level increased", "REVIVE.C F0282/F0286");
    check_int_eq(result.leaderHandStable, 1, "leader hand stable",
                 "CHAMPION.C F0297/F0298");
    check_int_eq(result.leaderHandNotConsumed, 1, "leader hand not consumed",
                 "CHAMPION.C F0297/F0298");
    check_int_eq(result.leaderHandNotMismatched, 1, "leader hand not mismatched",
                 "CHAMPION.C F0297/F0298");
    check_int_eq(result.leaderHandEmptyBefore, 1, "leader hand empty before",
                 "CHAMPION.C F0297");
    check_int_eq(result.aliveLeaderChainLinked, 1, "alive leader chain linked",
                 "champion chain");
    check_int_eq(result.candidateRemovedFromChain, 1,
                 "candidate removed from chain", "REVIVE.C F0282:789-805");
    check_int_eq(result.g0299Cleared, 1, "G0299 cleared",
                 "REVIVE.C F0282:785");
    check_int_eq(result.c040Cleared, 1, "C040 cleared", "C040");
    check_int_eq(result.c045Cleared, 1, "C045 cleared", "C045");
    check_int_eq(result.panelContentCleared, 1, "panel content cleared",
                 "PANEL.C F0346");
    check_int_eq(result.panelRedrawStable, 1, "panel redraw stable",
                 "PANEL.C F0347");
    check_int_eq(result.mouseScreenUpdateBracketed, 1,
                 "mouse screen update bracketed", "MOUSE.C F0077/F0078");
    check_int_eq(result.drawEnabledMenusInvoked, 1,
                 "draw enabled menus invoked", "START.C F0457");
    check_int_eq(result.queueDrained, 1, "queue drained", "COMMAND.C F0380");
    check_int_eq(result.noAcceptForAliveCandidate, 1,
                 "no accept for alive candidate", "REVIVE.C F0280");
    check_int_eq(result.noAcceptForNullCandidate, 1,
                 "no accept for null candidate", "G0299");
    check_int_eq(result.sourceAnchorsPresent, 1, "source anchors present",
                 "ReDMCSB");
    check_int_eq(result.guardRejectsAliveOwner, 1, "guard rejects alive owner",
                 "REVIVE.C F0280");
    check_int_eq(result.guardRejectsNullCandidate, 1,
                 "guard rejects null candidate", "G0299");
    check_int_eq(result.guardRejectsNoC040Panel, 1, "guard rejects no C040",
                 "C040");
    check_int_eq(result.guardRejectsNoC045Path, 1, "guard rejects no C045 path",
                 "PANEL.C F0347");
    check_int_eq(result.guardRejectsNoAcceptCommand, 1,
                 "guard rejects no accept command", "C160");
    check_int_eq(result.leaderBefore, 0, "leader before", "F0368");
    check_int_eq(result.leaderAfter, 0, "leader after", "F0368");
    check_u16_eq(result.leaderHandThingBefore, 0xffffu,
                 "leader hand empty before", "CHAMPION.C F0297");
    check_u16_eq(result.leaderHandThingAfter, 0xffffu,
                 "leader hand empty after", "CHAMPION.C F0297");
    check_int_eq(result.candidateFoodLevelBefore, 1500,
                 "candidate food level before", "REVIVE.C F0280");
    check_int_eq(result.candidateFoodLevelAfter, 2300,
                 "candidate food level after", "REVIVE.C F0282");
    check_int_eq(result.candidateWaterLevelBefore, 1500,
                 "candidate water level before", "REVIVE.C F0280");
    check_int_eq(result.candidateWaterLevelAfter, 2100,
                 "candidate water level after", "REVIVE.C F0282");
    check_int_eq(result.g0299Before, 0, "G0299 cleared at observation",
                 "G0299");
    check_int_eq(result.g0299After, 0, "G0299 cleared at observation",
                 "G0299");
    check_int_eq(result.candidateChainBefore[0], 4, "candidate chain head",
                 "REVIVE.C F0282");
    check_int_eq(result.candidateChainAfter[0], 4, "candidate chain head",
                 "REVIVE.C F0282");
    check_int_eq(result.candidateIndexBefore, 2, "candidate index before",
                 "REVIVE.C F0280");
    check_int_eq(result.candidateIndexAfter, 2, "candidate index after",
                 "REVIVE.C F0280");
    check_int_eq(state.f0359QueueWriteCount, 1, "one queued accept",
                 "COMMAND.C F0359");
    check_int_eq(state.f0361WheelLikeQueueWriteCount, 1,
                 "wheel-like queue marker", "COMMAND.C F0361");
    check_int_eq(state.f0378PanelRouteCount, 1, "panel route count",
                 "COMMAND.C F0378");
    check_int_eq(state.f0380DispatchCount, 1, "one dispatch",
                 "COMMAND.C F0380");
    check_int_eq(state.f0282AcceptClearCount, 1, "F0282 clear count",
                 "REVIVE.C F0282");
    check_int_eq(state.f0286StatsResetCount, 1, "F0286 stats reset count",
                 "REVIVE.C F0286");
    check_int_eq(state.f0297PutLeaderHandCount, 0, "leader hand not touched",
                 "CHAMPION.C F0297");
    check_int_eq(state.f0298RemoveLeaderHandCount, 0,
                 "leader hand not removed", "CHAMPION.C F0298");
    check_int_eq(state.f0301FoodSlotAddCount, 0, "no food slot add",
                 "CHAMPION.C F0301");
    check_int_eq(state.f0302FoodSlotDispatchCount, 0, "no food slot dispatch",
                 "CHAMPION.C F0302");
    check_int_eq(state.f0077MouseScreenUpdateEnable, 1, "F0077 balance",
                 "MOUSE.C F0077");
    check_int_eq(state.f0078MouseScreenUpdateDisable, 1, "F0078 balance",
                 "MOUSE.C F0078");
    check_int_eq(state.f0457StartDrawEnabledMenus, 1, "F0457 fired",
                 "START.C F0457");
    check_int_eq(state.f0344FoodWaterReadCount, 2, "F0344 read count",
                 "PANEL.C F0344");
    check_int_eq(state.f0345FoodWaterDrawCount, 1, "F0345 draw count",
                 "PANEL.C F0345");
    check_int_eq(state.f0346ResurrectDrawCount, 1, "F0346 resurrect draw count",
                 "PANEL.C F0346");
    check_int_eq(state.f0347PanelRedrawCount, 1, "F0347 panel redraw count",
                 "PANEL.C F0347");
    check_int_eq(state.commandQueueDepth, 0, "queue depth final",
                 "COMMAND.C F0380");
    check_int_eq(state.champions[0].foodLevel, 1500, "alive leader food stable",
                 "CHAMPION.C F0297");
    check_int_eq(state.champions[0].waterLevel, 1500,
                 "alive leader water stable", "CHAMPION.C F0297");
    check_int_eq(state.champions[2].currentHealth, 30,
                 "candidate resurrected to current health", "REVIVE.C F0286");
    check_int_eq(state.champions[2].maximumHealth, 30,
                 "candidate resurrected to maximum health", "REVIVE.C F0286");
    check_int_eq(state.champions[2].foodLevel, 2300,
                 "candidate food after F0286", "REVIVE.C F0286");
    check_int_eq(state.champions[2].waterLevel, 2100,
                 "candidate water after F0286", "REVIVE.C F0286");
    check_u16_eq(state.leaderHandThing, 0xffffu, "global hand empty",
                 "CLIKCHAM.C F0368");
    check_int_eq(state.panelContent, 0, "panel content cleared",
                 "PANEL.C F0346");
    check_int_eq(state.panelGraphic, 0, "panel graphic cleared",
                 "PANEL.C F0346");
    check_int_eq(state.c040PanelOpen, 0, "C040 remains closed after accept",
                 "C040");
    check_int_eq(state.c045PanelOpen, 0, "C045 cleared after accept", "C045");
    check_int_eq(state.c045AcceptPathActive, 0, "C045 accept path cleared",
                 "PANEL.C F0347");
    check_true(result.beforeHash != 0u, "before hash", "determinism");
    check_true(result.afterAcceptHash != 0u, "accept hash", "determinism");
    check_true(result.afterLeaderSettleHash != 0u, "settle hash",
               "determinism");
    check_true(result.hash != 0u, "result hash", "determinism");
    check_true(result.beforeHash != result.afterAcceptHash,
               "accept changes hash", "determinism");
    check_true(result.afterAcceptHash != result.afterLeaderSettleHash,
               "settle changes hash", "determinism");
    for (i = 0; i < DM1_V1_MC_C045_DEAD_OWNER_TRACE_COUNT_PC34; ++i) {
        check_int_eq(result.trace[i], 200 + i, "trace order",
                     "queue-accept-settle-stable");
    }
    return result.hash;
}

static void test_rejects(void)
{
    Dm1V1MirrorCandidateC045AcceptDeadOwnerStatePc34 state;
    Dm1V1MirrorCandidateC045AcceptDeadOwnerResultPc34 result;

    check_int_eq(
        dm1_v1_mirror_candidate_c045_accept_dead_owner_guard_run_pc34(
            NULL, &result),
        0, "null state rejected", "guard");
    dm1_v1_mirror_candidate_c045_accept_dead_owner_guard_init_pc34(&state);
    check_int_eq(
        dm1_v1_mirror_candidate_c045_accept_dead_owner_guard_run_pc34(
            &state, NULL),
        0, "null result rejected", "guard");
    dm1_v1_mirror_candidate_c045_accept_dead_owner_guard_init_pc34(&state);
    state.contractOnly = 0;
    check_int_eq(
        dm1_v1_mirror_candidate_c045_accept_dead_owner_guard_run_pc34(
            &state, &result),
        0, "non-contract rejected", "asset-free");
    dm1_v1_mirror_candidate_c045_accept_dead_owner_guard_init_pc34(&state);
    state.candidateOwnerAlive = 1;
    state.champions[2].alive = 1;
    check_int_eq(
        dm1_v1_mirror_candidate_c045_accept_dead_owner_guard_run_pc34(
            &state, &result),
        0, "alive owner rejected", "REVIVE.C F0280");
    dm1_v1_mirror_candidate_c045_accept_dead_owner_guard_init_pc34(&state);
    state.g0299CandidateOrdinal = 0;
    state.candidateChainOrdinals[0] = 0;
    check_int_eq(
        dm1_v1_mirror_candidate_c045_accept_dead_owner_guard_run_pc34(
            &state, &result),
        0, "null candidate rejected", "G0299");
    dm1_v1_mirror_candidate_c045_accept_dead_owner_guard_init_pc34(&state);
    state.c040PanelOpen = 0;
    check_int_eq(
        dm1_v1_mirror_candidate_c045_accept_dead_owner_guard_run_pc34(
            &state, &result),
        0, "no C040 panel rejected", "C040");
    dm1_v1_mirror_candidate_c045_accept_dead_owner_guard_init_pc34(&state);
    state.c045AcceptPathActive = 0;
    check_int_eq(
        dm1_v1_mirror_candidate_c045_accept_dead_owner_guard_run_pc34(
            &state, &result),
        0, "no C045 accept path rejected", "PANEL.C F0347");
    dm1_v1_mirror_candidate_c045_accept_dead_owner_guard_init_pc34(&state);
    state.acceptCommand = 999;
    check_int_eq(
        dm1_v1_mirror_candidate_c045_accept_dead_owner_guard_run_pc34(
            &state, &result),
        0, "non-accept command rejected", "C160");
    dm1_v1_mirror_candidate_c045_accept_dead_owner_guard_init_pc34(&state);
    state.leaderHandThing = 0x0001u;
    state.leaderHandEmpty = 0;
    state.champions[0].handThing = 0x0001u;
    check_int_eq(
        dm1_v1_mirror_candidate_c045_accept_dead_owner_guard_run_pc34(
            &state, &result),
        0, "non-empty leader hand rejected", "CHAMPION.C F0297");
}

int main(void)
{
    uint32_t hash;

    printf("probe=dm1_v1_mirror_candidate_c045_accept_dead_owner_guard_pc34_compat\n");
    printf("%s\n",
           dm1_v1_mirror_candidate_c045_accept_dead_owner_guard_source_evidence_pc34());
    test_evidence();
    test_initial_state();
    hash = test_run();
    test_rejects();
    if (g_failures || g_assertions < 130) {
        printf("FAIL assertions=%d failures=%d hash=0x%08X\n", g_assertions,
               g_failures, hash);
        return 1;
    }
    printf("PASS test_dm1_v1_mirror_candidate_c045_accept_dead_owner_guard_pc34_compat assertions=%d failures=0 hash=0x%08X\n",
           g_assertions, hash);
    return 0;
}
