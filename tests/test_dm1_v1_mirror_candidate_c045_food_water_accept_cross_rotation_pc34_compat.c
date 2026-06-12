#include "firestaff/dm1/v1/mirror_candidate/c045_food_water_accept_cross_rotation_pc34_compat.h"

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
    const Dm1V1MirrorCandidateC045FoodWaterAcceptCrossRotationEvidencePc34 *e =
        dm1_v1_mirror_candidate_c045_food_water_accept_cross_rotation_evidence_pc34();
    const char *text =
        dm1_v1_mirror_candidate_c045_food_water_accept_cross_rotation_source_evidence_pc34();
    const char *siblings[] = {
        "c045_close_after_non_candidate_transition",
        "c045_food_water_close_no_candidate",
        "c040_chrome_inventory_owner_swap",
        "c040_close_non_leader_scroll_pickup",
        "c040_redraw_after_chest_close",
        "c040_panel_browse_pickup_rotate_race",
        "click_cancel_with_rotation",
        "rotation_during_resurrect_confirmation",
        "c159_click_rotation_combo",
        "c545_pickup_while_panel_live",
        "c545_drop_while_panel_live",
        "mirror_candidate_close_while_resurrect_pending_inventory_pickup",
        "chest_pickup_during_resurrect_pending_non_leader"
    };
    int i;

    check_true(e != NULL, "evidence accessor", "pass772");
    check_contains(e->revivePublishAnchor, "F0280:124-132", "F0280 anchor",
                   e->revivePublishAnchor);
    check_contains(e->reviveAcceptClearAnchor, "F0282:744-806",
                   "F0282 anchor", e->reviveAcceptClearAnchor);
    check_contains(e->championHandAnchor, "F0297/F0298:243-298",
                   "leader hand anchor", e->championHandAnchor);
    check_contains(e->championSlotAnchor, "F0301/F0302:606-714",
                   "slot anchor", e->championSlotAnchor);
    check_contains(e->panelAnchor, "F0344:1493-1561", "F0344 anchor",
                   e->panelAnchor);
    check_contains(e->panelAnchor, "F0345:1563-1617", "F0345 anchor",
                   e->panelAnchor);
    check_contains(e->panelAnchor, "F0354:2299-2352", "F0354 anchor",
                   e->panelAnchor);
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
    check_contains(e->defsAnchor, "C160..C162", "C160 defs",
                   e->defsAnchor);
    check_contains(e->defsAnchor, "C10_SLOT_NECK", "C10 slot defs",
                   e->defsAnchor);
    check_contains(e->defsAnchor, "C10_COLOR_FLESH", "C10 color defs",
                   e->defsAnchor);
    check_contains(e->defsAnchor, "C30", "C30 defs", e->defsAnchor);
    check_contains(e->defsAnchor, "C38", "C38 defs", e->defsAnchor);
    check_contains(e->defsAnchor, "C545", "C545 defs", e->defsAnchor);
    check_contains(e->defsAnchor, "G0299", "G0299 defs", e->defsAnchor);
    check_contains(e->nonOverlap, "C045 food/water ACCEPT",
                   "accept non-overlap marker", e->nonOverlap);
    check_contains(e->nonOverlap, "same-drain leader rotation",
                   "rotation non-overlap marker", e->nonOverlap);
    for (i = 0; i < (int)(sizeof(siblings) / sizeof(siblings[0])); ++i) {
        check_contains(e->nonOverlap, siblings[i],
                       "required non-overlap self-assert", e->nonOverlap);
    }
    check_contains(text, "REVIVE.C F0280:124-132", "source F0280", text);
    check_contains(text, "F0282:744-806", "source F0282", text);
    check_contains(text, "CHAMPION.C F0297/F0298:243-298",
                   "source F0297/F0298", text);
    check_contains(text, "F0301/F0302:606-714", "source F0301/F0302",
                   text);
    check_contains(text, "PANEL.C F0344:1493-1561", "source F0344", text);
    check_contains(text, "F0345:1563-1617", "source F0345", text);
    check_contains(text, "F0354:2299-2352", "source F0354", text);
    check_contains(text, "COMMAND.C F0359:1452-1662", "source F0359",
                   text);
    check_contains(text, "F0378:1956-1993", "source F0378", text);
    check_contains(text, "F0361:1709-1813", "source F0361", text);
    check_contains(text, "F0380:2045-2178", "source F0380", text);
    check_contains(text, "CLIKCHAM.C F0367/F0368:20-73",
                   "source CLIKCHAM", text);
    check_contains(text, "5694 G0299", "source G0299", text);
}

static void test_initial_state(void)
{
    Dm1V1MirrorCandidateC045FoodWaterAcceptCrossRotationStatePc34 state;
    int i;

    dm1_v1_mirror_candidate_c045_food_water_accept_cross_rotation_init_pc34(
        &state);
    check_int_eq(state.contractOnly, 1, "contract-only", "asset-free");
    check_int_eq(state.sameDrainWindow, 1, "same drain window",
                 "COMMAND.C F0380");
    check_int_eq(state.partyChampionCount, 3, "party champion count",
                 "REVIVE.C F0280");
    check_int_eq(state.leaderIndex, 0, "old leader index",
                 "CLIKCHAM.C F0368");
    check_int_eq(state.queuedLeaderIndex, 1, "queued new leader",
                 "COMMAND.C F0359");
    check_int_eq(state.inventoryChampionOrdinal, 1, "inventory ordinal",
                 "G0423");
    check_int_eq(state.candidateChampionOrdinal, 3, "candidate ordinal",
                 "G0299");
    check_int_eq(state.candidateChainOrdinals[0], 3,
                 "candidate starts in chain", "REVIVE.C F0280");
    check_int_eq(state.candidateChainOrdinals[1], 4, "candidate chain tail",
                 "REVIVE.C F0282");
    check_int_eq(state.partyChainOrdinals[0], 1, "party chain leader",
                 "champion chain");
    check_int_eq(state.partyChainOrdinals[1], 2, "party chain second",
                 "champion chain");
    check_int_eq(state.partyChainOrdinals[2], 3, "party chain candidate",
                 "champion chain");
    check_int_eq(state.g0299CandidateOrdinal, 3, "G0299 candidate",
                 "DEFS.H:5694");
    check_int_eq(state.c040PanelOpen, 0, "C040 not live", "C040");
    check_int_eq(state.c045PanelOpen, 1, "C045 panel live", "C045");
    check_int_eq(state.panelContent, 565, "food/water panel",
                 "M565_PANEL_FOOD_WATER_POISONED");
    check_int_eq(state.panelGraphic, 45, "C045 graphic", "C045");
    check_int_eq(state.acceptCommand, 160, "accept command", "C160");
    check_int_eq(state.queuedStatusCommand, 13, "status command C013",
                 "COMMAND.C F0380");
    check_int_eq(state.queuedSetLeaderCommand, 17, "set leader C017",
                 "CLIKCHAM.C F0367");
    check_u16_eq(state.acceptedFoodThing, 0x0451u, "accepted food thing",
                 "C045");
    check_u16_eq(state.leaderHandThing, 0x0451u, "global leader hand food",
                 "CHAMPION.C F0297/F0298");
    check_int_eq(state.foodSlotIndex, 30, "C30+ food slot",
                 "CHAMPION.C F0301/F0302");
    check_int_eq(state.f0280PublishCount, 1, "candidate published",
                 "REVIVE.C F0280");
    check_int_eq(state.f0282AcceptClearCount, 0, "no clear yet",
                 "REVIVE.C F0282");
    check_int_eq(state.f0344FoodWaterReadCount, 2, "food/water read count",
                 "PANEL.C F0344");
    check_int_eq(state.f0345FoodWaterDrawCount, 1, "food/water draw count",
                 "PANEL.C F0345");
    check_int_eq(state.commandQueueDepth, 0, "empty queue", "COMMAND.C F0380");
    check_int_eq(state.trace[0], 100, "trace init", "determinism");
    check_true(state.beforeHash != 0u, "initial hash nonzero",
               "determinism");
    check_int_eq(state.champions[0].leader, 1, "old leader marked",
                 "CLIKCHAM.C F0368");
    check_int_eq(state.champions[1].leader, 0, "new leader not yet leader",
                 "CLIKCHAM.C F0368");
    check_u16_eq(state.champions[0].handThing, 0x0451u,
                 "old leader starts with food", "CHAMPION.C F0298");
    check_u16_eq(state.champions[1].handThing, 0x7021u,
                 "new leader starts with own hand", "CHAMPION.C F0297");
    for (i = 0; i < 3; ++i) {
        check_int_eq(state.champions[i].chainLinked, 1,
                     "party champion chain linked", "champion chain");
    }
}

static uint32_t test_run(void)
{
    Dm1V1MirrorCandidateC045FoodWaterAcceptCrossRotationStatePc34 state;
    Dm1V1MirrorCandidateC045FoodWaterAcceptCrossRotationResultPc34 result;
    int ok;
    int i;

    dm1_v1_mirror_candidate_c045_food_water_accept_cross_rotation_init_pc34(
        &state);
    ok = dm1_v1_mirror_candidate_c045_food_water_accept_cross_rotation_run_pc34(
        &state, &result);
    check_int_eq(ok, 1, "run accepted", "pass772");
    check_int_eq(result.accepted, 1, "result accepted", "contract");
    check_int_eq(result.sameDrainWindow, 1, "same drain", "COMMAND.C F0380");
    check_int_eq(result.c045AcceptPath, 1, "C045 accept path",
                 "PANEL.C F0345/F0378");
    check_int_eq(result.c040NotLive, 1, "C040 not live", "C040");
    check_int_eq(result.acceptClearRanFirst, 1, "accept clear first",
                 "REVIVE.C F0282/F0380");
    check_int_eq(result.candidateRemovedFromChain, 1,
                 "candidate removed from chain", "REVIVE.C F0282:789-805");
    check_int_eq(result.g0299Cleared, 1, "G0299 cleared",
                 "REVIVE.C F0282:785");
    check_int_eq(result.c040C045Cleared, 1, "C040/C045 cleared",
                 "PANEL.C F0345");
    check_int_eq(result.foodRemovedByAccept, 1, "food removed by accept",
                 "CHAMPION.C F0298");
    check_int_eq(result.oldLeaderHandEmpty, 1, "old leader hand empty",
                 "CHAMPION.C F0298");
    check_int_eq(result.rotationCompletedAfterAccept, 1,
                 "rotation completed after accept", "CLIKCHAM.C F0368");
    check_int_eq(result.newLeaderHandPreserved, 1,
                 "new leader hand preserved", "CHAMPION.C F0297");
    check_int_eq(result.leaderHandCoherentAfterRotation, 1,
                 "leader hand coherent", "CLIKCHAM.C F0368");
    check_int_eq(result.noDoubleClear, 1, "no double clear",
                 "REVIVE.C F0282");
    check_int_eq(result.noSkippedClear, 1, "no skipped clear",
                 "REVIVE.C F0282");
    check_int_eq(result.noDanglingCandidate, 1, "no dangling candidate",
                 "G0299");
    check_int_eq(result.partyChainCorrect, 1, "party chain correct",
                 "champion chain");
    check_int_eq(result.queueDrained, 1, "queue drained", "COMMAND.C F0380");
    check_int_eq(result.sourceAnchorsPresent, 1, "source anchors present",
                 "ReDMCSB");
    check_int_eq(result.guardRejectsC040Live, 1, "guard rejects C040 live",
                 "C040");
    check_int_eq(result.guardRejectsNoCandidate, 1,
                 "guard rejects no candidate", "G0299");
    check_int_eq(result.guardRejectsWrongPanel, 1,
                 "guard rejects wrong panel", "M565");
    check_int_eq(result.guardRejectsNoRotation, 1,
                 "guard rejects missing rotation", "CLIKCHAM.C F0368");
    check_int_eq(result.leaderBefore, 0, "leader before", "F0368");
    check_int_eq(result.leaderAfter, 1, "leader after", "F0368");
    check_u16_eq(result.oldLeaderHandBefore, 0x0451u,
                 "old leader food before", "CHAMPION.C F0298");
    check_u16_eq(result.oldLeaderHandAfter,
                 DM1_V1_MC_C045_ACCEPT_ROTATE_NONE_PC34,
                 "old leader empty after", "CHAMPION.C F0298");
    check_u16_eq(result.newLeaderHandBefore, 0x7021u,
                 "new leader hand before", "CHAMPION.C F0297");
    check_u16_eq(result.newLeaderHandAfter, 0x7021u,
                 "new leader hand after", "CHAMPION.C F0297");
    check_int_eq(result.g0299Before, 3, "G0299 before", "G0299");
    check_int_eq(result.g0299After, 0, "G0299 after", "G0299");
    check_int_eq(result.candidateChainBefore[0], 3, "candidate before chain",
                 "REVIVE.C F0280");
    check_int_eq(result.candidateChainAfter[0], 4, "candidate chain compacted",
                 "REVIVE.C F0282");
    check_int_eq(state.f0359QueueWriteCount, 2, "two queued commands",
                 "COMMAND.C F0359");
    check_int_eq(state.f0361WheelLikeQueueWriteCount, 1,
                 "wheel-like queue marker", "COMMAND.C F0361");
    check_int_eq(state.f0378PanelRouteCount, 1, "panel route count",
                 "COMMAND.C F0378");
    check_int_eq(state.f0380DispatchCount, 2, "two dispatches",
                 "COMMAND.C F0380");
    check_int_eq(state.f0282AcceptClearCount, 1, "F0282 clear count",
                 "REVIVE.C F0282");
    check_int_eq(state.f0298RemoveLeaderHandCount, 1,
                 "leader hand remove count", "CHAMPION.C F0298");
    check_int_eq(state.f0302FoodSlotDispatchCount, 1,
                 "food slot dispatch count", "CHAMPION.C F0302");
    check_int_eq(state.f0367LeaderClickRouteCount, 1,
                 "leader click route count", "CLIKCHAM.C F0367");
    check_int_eq(state.f0368SetLeaderCount, 1, "set leader count",
                 "CLIKCHAM.C F0368");
    check_int_eq(state.commandQueueDepth, 0, "queue depth final",
                 "COMMAND.C F0380");
    check_int_eq(state.candidateSensorDisabled, 1, "candidate sensor disabled",
                 "REVIVE.C F0282:796-799");
    check_int_eq(state.acceptCompletedBeforeRotation, 1,
                 "accept complete before rotation", "COMMAND.C F0380");
    check_int_eq(state.doubleClearAttempted, 0, "no double clear attempted",
                 "REVIVE.C F0282");
    check_int_eq(state.skippedClearAttempted, 0, "no skipped clear attempted",
                 "REVIVE.C F0282");
    check_u16_eq(state.leaderHandThing, 0x7021u, "global hand new leader",
                 "CLIKCHAM.C F0368");
    check_int_eq(state.panelContent, 0, "panel content cleared",
                 "PANEL.C F0354");
    check_int_eq(state.panelGraphic, 0, "panel graphic cleared",
                 "PANEL.C F0354");
    check_int_eq(state.c040PanelOpen, 0, "C040 remains closed", "C040");
    check_int_eq(state.c045PanelOpen, 0, "C045 cleared", "C045");
    check_true(result.beforeHash != 0u, "before hash", "determinism");
    check_true(result.afterAcceptHash != 0u, "accept hash", "determinism");
    check_true(result.afterRotateHash != 0u, "rotate hash", "determinism");
    check_true(result.hash != 0u, "result hash", "determinism");
    check_true(result.beforeHash != result.afterAcceptHash,
               "accept changes hash", "determinism");
    check_true(result.afterAcceptHash != result.afterRotateHash,
               "rotation changes hash", "determinism");
    for (i = 0; i < DM1_V1_MC_C045_ACCEPT_ROTATE_TRACE_COUNT_PC34; ++i) {
        check_int_eq(result.trace[i], 100 + i, "trace order",
                     "accept before rotation");
    }
    return result.hash;
}

static void test_rejects(void)
{
    Dm1V1MirrorCandidateC045FoodWaterAcceptCrossRotationStatePc34 state;
    Dm1V1MirrorCandidateC045FoodWaterAcceptCrossRotationResultPc34 result;

    check_int_eq(
        dm1_v1_mirror_candidate_c045_food_water_accept_cross_rotation_run_pc34(
            NULL, &result),
        0, "null state rejected", "guard");
    dm1_v1_mirror_candidate_c045_food_water_accept_cross_rotation_init_pc34(
        &state);
    check_int_eq(
        dm1_v1_mirror_candidate_c045_food_water_accept_cross_rotation_run_pc34(
            &state, NULL),
        0, "null result rejected", "guard");
    dm1_v1_mirror_candidate_c045_food_water_accept_cross_rotation_init_pc34(
        &state);
    state.contractOnly = 0;
    check_int_eq(
        dm1_v1_mirror_candidate_c045_food_water_accept_cross_rotation_run_pc34(
            &state, &result),
        0, "non-contract rejected", "asset-free");
    dm1_v1_mirror_candidate_c045_food_water_accept_cross_rotation_init_pc34(
        &state);
    state.c040PanelOpen = 1;
    check_int_eq(
        dm1_v1_mirror_candidate_c045_food_water_accept_cross_rotation_run_pc34(
            &state, &result),
        0, "C040 live rejected", "C040");
    dm1_v1_mirror_candidate_c045_food_water_accept_cross_rotation_init_pc34(
        &state);
    state.g0299CandidateOrdinal = 0;
    check_int_eq(
        dm1_v1_mirror_candidate_c045_food_water_accept_cross_rotation_run_pc34(
            &state, &result),
        0, "missing candidate rejected", "G0299");
    dm1_v1_mirror_candidate_c045_food_water_accept_cross_rotation_init_pc34(
        &state);
    state.panelContent = 999;
    check_int_eq(
        dm1_v1_mirror_candidate_c045_food_water_accept_cross_rotation_run_pc34(
            &state, &result),
        0, "wrong panel rejected", "M565");
    dm1_v1_mirror_candidate_c045_food_water_accept_cross_rotation_init_pc34(
        &state);
    state.queuedLeaderIndex = 0;
    state.queuedSetLeaderCommand = 16;
    check_int_eq(
        dm1_v1_mirror_candidate_c045_food_water_accept_cross_rotation_run_pc34(
            &state, &result),
        0, "missing rotation rejected", "CLIKCHAM.C F0368");
}

int main(void)
{
    uint32_t hash;

    printf("probe=dm1_v1_mirror_candidate_c045_food_water_accept_cross_rotation_pc34_compat\n");
    printf("%s\n",
           dm1_v1_mirror_candidate_c045_food_water_accept_cross_rotation_source_evidence_pc34());
    test_evidence();
    test_initial_state();
    hash = test_run();
    test_rejects();
    if (g_failures || g_assertions < 130) {
        printf("FAIL assertions=%d failures=%d hash=0x%08X\n", g_assertions,
               g_failures, hash);
        return 1;
    }
    printf("PASS test_dm1_v1_mirror_candidate_c045_food_water_accept_cross_rotation_pc34_compat assertions=%d failures=0 hash=0x%08X\n",
           g_assertions, hash);
    return 0;
}
