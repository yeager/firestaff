#include "firestaff/dm1/v1/mirror_candidate_close/close_while_c045_pending_pc34_compat.h"

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
    const Dm1V1MirrorCandidateCloseWhileC045PendingEvidencePc34 *e =
        dm1_v1_mirror_candidate_close_while_c045_pending_evidence_pc34();
    const char *text =
        dm1_v1_mirror_candidate_close_while_c045_pending_source_evidence_pc34();
    const char *disjoint[] = {
        "C545 accept during rotation",
        "C045 accept cross-rotation",
        "C045 close after non-candidate transition",
        "resurrect close-pending",
        "chest scroll-wheel drop",
        "mirror-candidate inventory-click rotations",
        "spell race"
    };
    int i;

    check_true(e != NULL, "evidence accessor", "source-lock");
    check_contains(e->revivePublishAnchor, "F0280:124-132", "F0280 anchor",
                   e->revivePublishAnchor);
    check_contains(e->reviveCloseAnchor, "F0282:744-806", "F0282 anchor",
                   e->reviveCloseAnchor);
    check_contains(e->panelFoodWaterAnchor, "F0344:1493-1561",
                   "F0344 anchor", e->panelFoodWaterAnchor);
    check_contains(e->panelFoodWaterAnchor, "F0345:1563-1617",
                   "F0345 anchor", e->panelFoodWaterAnchor);
    check_contains(e->panelResurrectAnchor, "F0346:1619-1637",
                   "F0346 anchor", e->panelResurrectAnchor);
    check_contains(e->panelResurrectAnchor, "F0347:1639-1693",
                   "F0347 anchor", e->panelResurrectAnchor);
    check_contains(e->commandQueueAnchor, "F0359:1452-1662",
                   "F0359 anchor", e->commandQueueAnchor);
    check_contains(e->commandPendingAnchor, "1489-1494",
                   "pending store anchor", e->commandPendingAnchor);
    check_contains(e->commandPendingAnchor, "F0360:1692-1707",
                   "pending replay anchor", e->commandPendingAnchor);
    check_contains(e->commandPanelRouteAnchor, "F0378:1956-1993",
                   "panel route anchor", e->commandPanelRouteAnchor);
    check_contains(e->commandDrainAnchor, "F0380:2045-2184",
                   "queue drain anchor", e->commandDrainAnchor);
    check_contains(e->championHandAnchor, "F0297/F0298:243-298",
                   "hand anchor", e->championHandAnchor);
    check_contains(e->defsAnchor, "C160..C162", "C160 defs",
                   e->defsAnchor);
    check_contains(e->defsAnchor, "C040/C045", "C040/C045 defs",
                   e->defsAnchor);
    check_contains(e->defsAnchor, "M565/M568", "panel defs",
                   e->defsAnchor);
    check_contains(e->defsAnchor, "C537..C545", "slot defs",
                   e->defsAnchor);
    check_contains(e->defsAnchor, "G0299", "G0299 defs",
                   e->defsAnchor);
    check_contains(e->nonOverlap, "stale C045 food/water accept pending",
                   "narrow non-overlap", e->nonOverlap);
    for (i = 0; i < (int)(sizeof(disjoint) / sizeof(disjoint[0])); ++i) {
        check_contains(e->nonOverlap, disjoint[i], "disjoint marker",
                       e->nonOverlap);
    }
    check_contains(text, "REVIVE.C F0280:124-132", "source F0280", text);
    check_contains(text, "F0282:744-806", "source F0282", text);
    check_contains(text, "PANEL.C F0344:1493-1561", "source F0344", text);
    check_contains(text, "F0345:1563-1617", "source F0345", text);
    check_contains(text, "F0346:1619-1637", "source F0346", text);
    check_contains(text, "F0347:1639-1693", "source F0347", text);
    check_contains(text, "COMMAND.C F0359:1452-1662", "source F0359",
                   text);
    check_contains(text, "1489-1494", "source pending store", text);
    check_contains(text, "F0360:1692-1707", "source F0360", text);
    check_contains(text, "F0378:1956-1993", "source F0378", text);
    check_contains(text, "F0380:2045-2184", "source F0380", text);
    check_contains(text, "CHAMPION.C F0297/F0298:243-298",
                   "source champion hand", text);
    check_contains(text, "5694 G0299", "source G0299", text);
}

static void test_initial_state(void)
{
    Dm1V1MirrorCandidateCloseWhileC045PendingStatePc34 state;
    int i;

    dm1_v1_mirror_candidate_close_while_c045_pending_init_pc34(&state,
                                                               0xc045160u);
    check_int_eq(state.contractOnly, 1, "contract-only", "asset-free");
    check_int_eq(state.noGameDataRequired, 1, "no game data required",
                 "fixture");
    check_int_eq((int)state.seed, 0xc045160, "deterministic seed", "seed");
    check_int_eq(state.partyChampionCount, 3, "party count",
                 "REVIVE.C F0280");
    check_int_eq(state.leaderIndex, 0, "leader index", "CHAMPION.C");
    check_int_eq(state.inventoryChampionOrdinal, 1, "inventory ordinal",
                 "G0423");
    check_int_eq(state.candidateChampionOrdinal, 3, "candidate ordinal",
                 "G0299");
    check_int_eq(state.g0299CandidateOrdinal, 3, "G0299 live",
                 "DEFS.H:5694");
    check_int_eq(state.c040PanelOpen, 1, "C040 panel open", "PANEL.C F0346");
    check_int_eq(state.c045FoodWaterAcceptPending, 1, "C045 pending",
                 "PANEL.C F0345");
    check_int_eq(state.c160CloseClickQueued, 0, "close not queued yet",
                 "COMMAND.C F0359");
    check_int_eq(state.pendingClickStoredWhileLocked, 0,
                 "no pending click stored yet", "COMMAND.C F0359");
    check_int_eq(state.panelContent, 568, "resurrect panel content",
                 "M568");
    check_int_eq(state.panelGraphic, 40, "C040 panel graphic", "C040");
    check_int_eq(state.queuedC045Command, 45, "C045 stale command", "C045");
    check_int_eq(state.queuedCloseCommand, 160, "C160 close command",
                 "C160");
    check_u16_eq(state.leaderHandThing, state.c045FoodThing,
                 "leader hand food fixture", "CHAMPION.C F0298");
    check_int_eq(state.candidateChainOrdinals[0], 3, "candidate chain head",
                 "REVIVE.C F0280");
    check_int_eq(state.candidateChainOrdinals[1], 4, "candidate chain tail",
                 "REVIVE.C F0282");
    check_int_eq(state.partyChainOrdinals[0], 1, "party leader ordinal",
                 "party chain");
    check_int_eq(state.partyChainOrdinals[1], 2, "party second ordinal",
                 "party chain");
    check_int_eq(state.partyChainOrdinals[2], 3, "party candidate ordinal",
                 "party chain");
    check_int_eq(state.commandQueueDepth, 0, "empty queue",
                 "COMMAND.C F0380");
    check_int_eq(state.f0280PublishCount, 1, "candidate published once",
                 "REVIVE.C F0280");
    check_int_eq(state.f0282CloseClearCount, 0, "no close clear yet",
                 "REVIVE.C F0282");
    check_int_eq(state.f0298RemoveLeaderHandCount, 0, "no hand removal",
                 "CHAMPION.C F0298");
    check_int_eq(state.f0344FoodWaterReadCount, 2, "food/water bars read",
                 "PANEL.C F0344");
    check_int_eq(state.f0345FoodWaterDrawCount, 1, "food/water drawn",
                 "PANEL.C F0345");
    check_int_eq(state.f0346ResurrectDrawCount, 1, "C040 drawn",
                 "PANEL.C F0346");
    check_int_eq(state.trace[0], 300, "trace initialized", "determinism");
    check_true(state.beforeHash != 0u, "initial hash nonzero",
               "determinism");
    for (i = 3; i < DM1_V1_MC_CLOSE_C045_PENDING_CHAIN_COUNT_PC34; ++i) {
        check_int_eq(state.partyChainOrdinals[i], 0, "unused party slot zero",
                     "fixture");
    }
}

static uint32_t test_run(void)
{
    Dm1V1MirrorCandidateCloseWhileC045PendingStatePc34 state;
    Dm1V1MirrorCandidateCloseWhileC045PendingResultPc34 result;
    int ok;
    int i;

    dm1_v1_mirror_candidate_close_while_c045_pending_init_pc34(&state,
                                                               0xc045160u);
    ok = dm1_v1_mirror_candidate_close_while_c045_pending_run_pc34(&state,
                                                                    &result);
    check_int_eq(ok, 1, "run accepted", "runtime gate");
    check_int_eq(result.accepted, 1, "result accepted", "contract");
    check_int_eq(result.c045PendingAtStart, 1, "C045 pending at start",
                 "PANEL.C F0345");
    check_int_eq(result.c160CloseArrivedWhileLocked, 1,
                 "C160 arrived while locked", "COMMAND.C F0359:1489-1494");
    check_int_eq(result.staleC045RejectedBeforeClose, 1,
                 "stale C045 rejected before close", "COMMAND.C F0378");
    check_int_eq(result.candidatePreservedUntilClose, 1,
                 "candidate preserved until C160", "G0299");
    check_int_eq(result.pendingClickReplayed, 1, "pending click replayed",
                 "COMMAND.C F0360");
    check_int_eq(result.closeDispatchedThroughC040Panel, 1,
                 "C160 dispatched through C040 panel", "COMMAND.C F0378");
    check_int_eq(result.g0299ClearedByClose, 1, "G0299 cleared by close",
                 "REVIVE.C F0282");
    check_int_eq(result.c045PendingClearedByClose, 1,
                 "C045 pending cleared by close", "REVIVE.C F0282");
    check_int_eq(result.candidateRemovedFromChain, 1,
                 "candidate removed from chain", "REVIVE.C F0282:757");
    check_int_eq(result.foodNotConsumed, 1, "food not consumed",
                 "CHAMPION.C F0298");
    check_int_eq(result.leaderHandStable, 1, "leader hand stable",
                 "CHAMPION.C F0297/F0298");
    check_int_eq(result.noLeaderHandRemoval, 1, "no F0298 removal",
                 "CHAMPION.C F0298");
    check_int_eq(result.queueDrained, 1, "queue drained",
                 "COMMAND.C F0380");
    check_int_eq(result.panelClosedAfterC160, 1, "panel closed after C160",
                 "PANEL.C F0347");
    check_int_eq(result.sourceAnchorsPresent, 1, "source anchors present",
                 "source-lock");
    check_int_eq(result.guardRejectsNullState, 1, "null state guard",
                 "guard");
    check_int_eq(result.guardRejectsNullResult, 1, "null result guard",
                 "guard");
    check_int_eq(result.g0299Before, 3, "G0299 before", "G0299");
    check_int_eq(result.g0299AfterStaleC045, 3, "G0299 after stale C045",
                 "COMMAND.C F0378");
    check_int_eq(result.g0299AfterClose, 0, "G0299 after close",
                 "REVIVE.C F0282");
    check_u16_eq(result.leaderHandBefore, result.leaderHandAfter,
                 "leader hand unchanged", "CHAMPION.C F0297/F0298");
    check_int_eq(result.candidateChainBefore[0], 3, "chain before head",
                 "REVIVE.C F0280");
    check_int_eq(result.candidateChainAfterStaleC045[0], 3,
                 "chain after stale C045 still candidate",
                 "COMMAND.C F0378");
    check_int_eq(result.candidateChainAfterClose[0], 4,
                 "chain after close shifted", "REVIVE.C F0282");
    check_true(result.beforeHash != 0u, "before hash nonzero", "hash");
    check_true(result.afterStaleC045Hash != 0u, "stale hash nonzero",
               "hash");
    check_true(result.afterPendingReplayHash != 0u, "replay hash nonzero",
               "hash");
    check_true(result.afterCloseHash != 0u, "close hash nonzero", "hash");
    check_true(result.beforeHash != result.afterStaleC045Hash,
               "stale phase changes hash", "hash");
    check_true(result.afterStaleC045Hash != result.afterPendingReplayHash,
               "replay phase changes hash", "hash");
    check_true(result.afterPendingReplayHash != result.afterCloseHash,
               "close phase changes hash", "hash");
    check_true(result.hash == result.afterCloseHash, "final hash matches",
               "hash");
    check_int_eq(state.f0359QueueWriteCount, 2, "two queue writes",
                 "COMMAND.C F0359/F0360");
    check_int_eq(state.f0360PendingReplayCount, 1, "one replay",
                 "COMMAND.C F0360");
    check_int_eq(state.f0378PanelRouteCount, 2, "two panel routes",
                 "COMMAND.C F0378");
    check_int_eq(state.f0380DrainCount, 2, "two drains", "COMMAND.C F0380");
    check_int_eq(state.staleC045RejectCount, 1, "one stale reject",
                 "COMMAND.C F0378");
    check_int_eq(state.f0282CloseClearCount, 1, "one close clear",
                 "REVIVE.C F0282");
    check_int_eq(state.closeClearedPendingC045, 1, "close cleared pending",
                 "REVIVE.C F0282");
    check_int_eq(state.candidateSensorDisabled, 1, "candidate disabled",
                 "REVIVE.C F0282");
    check_int_eq(state.foodConsumed, 0, "food not consumed count",
                 "PANEL.C F0349");
    check_int_eq(state.commandQueueDepth, 0, "queue depth zero",
                 "COMMAND.C F0380");
    check_int_eq(state.pendingClickStoredWhileLocked, 0,
                 "pending click cleared", "COMMAND.C F0360");
    for (i = 0; i < DM1_V1_MC_CLOSE_C045_PENDING_TRACE_COUNT_PC34; ++i) {
        if (i < 8) {
            check_true(result.trace[i] >= 300 && result.trace[i] <= 307,
                       "trace value in expected range", "determinism");
        }
    }
    return result.hash;
}

static void test_guards(void)
{
    Dm1V1MirrorCandidateCloseWhileC045PendingStatePc34 state;
    Dm1V1MirrorCandidateCloseWhileC045PendingResultPc34 result;

    dm1_v1_mirror_candidate_close_while_c045_pending_init_pc34(&state,
                                                               0xc045160u);
    state.g0299CandidateOrdinal = 0;
    check_int_eq(dm1_v1_mirror_candidate_close_while_c045_pending_run_pc34(
                     &state, &result),
                 0, "guard rejects no candidate", "G0299");

    dm1_v1_mirror_candidate_close_while_c045_pending_init_pc34(&state,
                                                               0xc045160u);
    state.panelContent = 565;
    check_int_eq(dm1_v1_mirror_candidate_close_while_c045_pending_run_pc34(
                     &state, &result),
                 0, "guard rejects wrong panel", "M568");

    dm1_v1_mirror_candidate_close_while_c045_pending_init_pc34(&state,
                                                               0xc045160u);
    state.c045FoodWaterAcceptPending = 0;
    check_int_eq(dm1_v1_mirror_candidate_close_while_c045_pending_run_pc34(
                     &state, &result),
                 0, "guard rejects no C045 pending", "C045");

    dm1_v1_mirror_candidate_close_while_c045_pending_init_pc34(&state,
                                                               0xc045160u);
    state.queuedCloseCommand = 162;
    check_int_eq(dm1_v1_mirror_candidate_close_while_c045_pending_run_pc34(
                     &state, &result),
                 0, "guard rejects non-C160 close click", "C160");
}

int main(void)
{
    uint32_t hash;

    test_evidence();
    test_initial_state();
    hash = test_run();
    test_guards();

    if (g_failures || g_assertions < 60) {
        printf("FAIL dm1_v1_mirror_candidate_close_while_c045_pending_pc34_compat "
               "assertions=%d failures=%d hash=0x%08x\n",
               g_assertions, g_failures, (unsigned int)hash);
        return 1;
    }
    printf("PASS dm1_v1_mirror_candidate_close_while_c045_pending_pc34_compat "
           "assertions=%d hash=0x%08x\n",
           g_assertions, (unsigned int)hash);
    return 0;
}
