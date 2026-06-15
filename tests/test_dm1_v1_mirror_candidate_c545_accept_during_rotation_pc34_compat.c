#include "firestaff/dm1/v1/mirror_candidate/c545_accept_during_rotation_pc34_compat.h"

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

static void check_u32_eq(uint32_t actual, uint32_t expected,
                         const char *message, const char *anchor)
{
    ++g_assertions;
    if (actual != expected) {
        ++g_failures;
        printf("FAIL %s actual=0x%08x expected=0x%08x [%s]\n", message,
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
    const Dm1V1MirrorCandidateC545AcceptDuringRotationEvidencePc34 *e =
        dm1_v1_mirror_candidate_c545_accept_during_rotation_evidence_pc34();
    const char *text =
        dm1_v1_mirror_candidate_c545_accept_during_rotation_source_evidence_pc34();

    check_true(e != NULL, "evidence accessor", "pass776");
    check_contains(e->revivePublishAnchor, "F0280:124-132", "F0280 anchor",
                   e->revivePublishAnchor);
    check_contains(e->reviveAcceptAnchor, "F0282:744-806", "F0282 anchor",
                   e->reviveAcceptAnchor);
    check_contains(e->panelFoodWaterAnchor, "F0344:1493-1561",
                   "F0344 context anchor", e->panelFoodWaterAnchor);
    check_contains(e->panelFoodWaterAnchor, "F0345:1563-1617",
                   "F0345 context anchor", e->panelFoodWaterAnchor);
    check_contains(e->panelFoodWaterAnchor, "F0354:2299-2352",
                   "F0354 context anchor", e->panelFoodWaterAnchor);
    check_contains(e->panelC545Anchor, "F0351:1965-2109",
                   "F0351 C545 anchor", e->panelC545Anchor);
    check_contains(e->panelC545Anchor, "F0352:2111-2160",
                   "F0352 C545 anchor", e->panelC545Anchor);
    check_contains(e->panelC545Anchor, "F0353:2162-2193",
                   "F0353 C545 anchor", e->panelC545Anchor);
    check_contains(e->championHandAnchor, "F0297/F0298:243-298",
                   "leader hand anchor", e->championHandAnchor);
    check_contains(e->championSlotAnchor, "F0301/F0302:606-714",
                   "slot dispatch anchor", e->championSlotAnchor);
    check_contains(e->commandQueueAnchor, "F0359:1452-1662",
                   "queue anchor", e->commandQueueAnchor);
    check_contains(e->commandPanelRouteAnchor, "F0378:1956-1993",
                   "panel route anchor", e->commandPanelRouteAnchor);
    check_contains(e->commandWheelQueueAnchor, "F0361:1709-1813",
                   "wheel queue anchor", e->commandWheelQueueAnchor);
    check_contains(e->commandDrainAnchor, "F0380:2045-2178",
                   "drain anchor", e->commandDrainAnchor);
    check_contains(e->defsAnchor, "C160..C162", "C160 defs",
                   e->defsAnchor);
    check_contains(e->defsAnchor, "C10_SLOT_NECK", "C10 defs",
                   e->defsAnchor);
    check_contains(e->defsAnchor, "C30", "C30 defs", e->defsAnchor);
    check_contains(e->defsAnchor, "C38/M070", "C38 defs", e->defsAnchor);
    check_contains(e->defsAnchor, "C10_COLOR_FLESH", "C10 color defs",
                   e->defsAnchor);
    check_contains(e->defsAnchor, "C040/C045", "C040/C045 defs",
                   e->defsAnchor);
    check_contains(e->defsAnchor, "M565/M568", "M565/M568 defs",
                   e->defsAnchor);
    check_contains(e->defsAnchor, "C537..C545", "C545 defs",
                   e->defsAnchor);
    check_contains(e->defsAnchor, "G0299", "G0299 defs", e->defsAnchor);
    check_contains(e->nonOverlap, "C545 resurrect-accept",
                   "C545 non-overlap", e->nonOverlap);
    check_contains(e->nonOverlap, "C040 close and leader rotation",
                   "gate non-overlap", e->nonOverlap);
    check_contains(e->nonOverlap, "not pass768", "pass768 non-overlap",
                   e->nonOverlap);
    check_contains(e->nonOverlap, "pass772", "pass772 non-overlap",
                   e->nonOverlap);
    check_contains(e->nonOverlap, "click-cancel with rotation",
                   "click-cancel non-overlap", e->nonOverlap);
    check_contains(text, "REVIVE.C F0280:124-132", "source F0280", text);
    check_contains(text, "F0282:744-806", "source F0282", text);
    check_contains(text, "PANEL.C F0351:1965-2109", "source F0351", text);
    check_contains(text, "F0352:2111-2160", "source F0352", text);
    check_contains(text, "F0353:2162-2193", "source F0353", text);
    check_contains(text, "COMMAND.C F0380:2045-2178", "source F0380",
                   text);
    check_contains(text, "3906-3914 C537..C545", "source C545", text);
    check_contains(text, "5694 G0299", "source G0299", text);
}

static void test_initial_state(void)
{
    Dm1V1MirrorCandidateC545AcceptDuringRotationStatePc34 state;
    int i;

    dm1_v1_mirror_candidate_c545_accept_during_rotation_init_pc34(&state);
    check_int_eq(state.contractOnly, 1, "contract-only", "asset-free");
    check_int_eq(state.noDosPixelParityClaim, 1, "no DOS pixel claim",
                 "test contract");
    check_int_eq(state.partyChampionCount, 3, "party count",
                 "REVIVE.C F0280");
    check_int_eq(state.leaderIndex, 0, "old leader", "COMMAND.C F0380");
    check_int_eq(state.queuedLeaderIndex, 1, "queued leader",
                 "COMMAND.C F0361");
    check_int_eq(state.inventoryChampionOrdinal, 1, "inventory ordinal",
                 "G0423");
    check_int_eq(state.candidateChampionOrdinal, 3, "candidate ordinal",
                 "REVIVE.C F0280");
    check_int_eq(state.c040CandidateIndex, 2, "C040 candidate index",
                 "C040");
    check_int_eq(state.candidateChainOrdinals[0], 3, "candidate chain head",
                 "REVIVE.C F0280");
    check_int_eq(state.candidateChainOrdinals[1], 4, "candidate chain tail",
                 "REVIVE.C F0282");
    check_int_eq(state.championChainOrdinals[0], 1, "champion chain 1",
                 "champion chain");
    check_int_eq(state.championChainOrdinals[1], 2, "champion chain 2",
                 "champion chain");
    check_int_eq(state.championChainOrdinals[2], 3, "champion chain 3",
                 "champion chain");
    check_int_eq(state.g0299CandidateOrdinal, 3, "G0299 candidate",
                 "DEFS.H:5694");
    check_int_eq(state.c045CandidateOpen, 1, "C045 candidate open", "C045");
    check_int_eq(state.c040PanelOpen, 1, "C040 panel open", "C040");
    check_int_eq(state.c040CloseQueued, 1, "C040 close queued",
                 "COMMAND.C F0359");
    check_int_eq(state.leaderRotationQueued, 1, "rotation queued",
                 "COMMAND.C F0361");
    check_int_eq(state.panelContent, 568, "M568 panel",
                 "M568_PANEL_RESURRECT_REINCARNATE");
    check_int_eq(state.panelGraphic, 40, "C040 panel graphic", "DEFS.H:2200");
    check_int_eq(state.c545Zone, 545, "C545 panel zone", "DEFS.H:3914");
    check_u32_eq(state.c545PanelPixel, 0x00c545a5u, "C545 panel pixel",
                 "C545");
    check_u32_eq(state.c040RedrawState, 0x040c5450u, "C040 redraw state",
                 "PANEL.C F0353");
    check_int_eq(state.c545AcceptCommand, 160, "C160 accept command",
                 "DEFS.H:338");
    check_int_eq(state.f0280PublishCount, 1, "F0280 publish count",
                 "REVIVE.C F0280");
    check_int_eq(state.f0282AcceptClearCount, 0, "F0282 not run yet",
                 "REVIVE.C F0282");
    check_int_eq(state.f0359QueueWriteCount, 3, "three queued commands",
                 "COMMAND.C F0359");
    check_int_eq(state.f0361WheelQueueWriteCount, 1, "wheel queue marker",
                 "COMMAND.C F0361");
    check_int_eq(state.commandQueueDepth, 3, "initial queue depth",
                 "COMMAND.C F0380");
    for (i = 0; i < 4; ++i) {
        check_int_eq(state.trace[i], 200 + i, "initial trace order",
                     "determinism");
    }
    check_true(state.beforeHash != 0u, "before hash nonzero",
               "determinism");
}

static uint32_t test_run(void)
{
    Dm1V1MirrorCandidateC545AcceptDuringRotationStatePc34 state;
    Dm1V1MirrorCandidateC545AcceptDuringRotationResultPc34 result;
    int ok;
    int i;

    dm1_v1_mirror_candidate_c545_accept_during_rotation_init_pc34(&state);
    ok = dm1_v1_mirror_candidate_c545_accept_during_rotation_run_pc34(
        &state, &result);
    check_int_eq(ok, 1, "run accepted", "pass776");
    check_int_eq(result.accepted, 1, "result accepted", "contract");
    check_int_eq(result.c545AcceptRoute, 1, "C545 accept route",
                 "COMMAND.C F0378");
    check_int_eq(result.c040GateRequired, 1, "C040 close gate required",
                 "C040");
    check_int_eq(result.rotationGateRequired, 1, "rotation gate required",
                 "COMMAND.C F0380");
    check_int_eq(result.rejectedBeforeC040Close, 1,
                 "reject before C040 close", "C040");
    check_int_eq(result.rejectedBeforeRotationDrain, 1,
                 "reject before rotation drain", "COMMAND.C F0380");
    check_int_eq(result.stableThroughRejectedC545, 1,
                 "stable through rejected C545 event", "C545");
    check_int_eq(result.stableUntilBothGates, 1,
                 "stable until both gates", "C040/F0380");
    check_int_eq(result.c040CloseCompletedBeforeAccept, 1,
                 "C040 close before accept", "C040");
    check_int_eq(result.rotationDrainedBeforeAccept, 1,
                 "rotation drained before accept", "COMMAND.C F0380");
    check_int_eq(result.acceptAfterGatesSucceeded, 1,
                 "accept after gates succeeds", "REVIVE.C F0282");
    check_int_eq(result.g0299PreservedBeforeGates, 1,
                 "G0299 preserved before gates", "DEFS.H:5694");
    check_int_eq(result.g0299ClearedAfterAccept, 1,
                 "G0299 cleared after accept", "REVIVE.C F0282:785");
    check_int_eq(result.c040CandidateIndexPreserved, 1,
                 "C040 candidate index preserved", "C040");
    check_int_eq(result.c040RedrawStatePreserved, 1,
                 "C040 redraw state preserved", "PANEL.C F0353");
    check_int_eq(result.championChainPreservedBeforeGates, 1,
                 "champion chain preserved before gates", "REVIVE.C F0282");
    check_int_eq(result.c545PanelPixelPreservedBeforeGates, 1,
                 "C545 panel pixel preserved before gates", "C545");
    check_int_eq(result.candidateRemovedFromChain, 1,
                 "candidate removed after accept", "REVIVE.C F0282");
    check_int_eq(result.leaderRotationCompleted, 1, "leader rotation done",
                 "COMMAND.C F0380");
    check_int_eq(result.leaderHandCoherentAfterRotation, 1,
                 "leader hand coherent", "CHAMPION.C F0297/F0298");
    check_int_eq(result.sourceAnchorsPresent, 1, "source anchors present",
                 "ReDMCSB");
    check_int_eq(result.guardRejectsNullState, 1, "null state rejected",
                 "guard");
    check_int_eq(result.guardRejectsNullResult, 1, "null result rejected",
                 "guard");
    check_int_eq(result.guardRejectsNonContract, 1, "non-contract rejected",
                 "guard");
    check_int_eq(result.guardRejectsNoCandidate, 1, "no candidate rejected",
                 "G0299");
    check_int_eq(result.guardRejectsWrongPanel, 1, "wrong panel rejected",
                 "M568");
    check_int_eq(result.guardRejectsNoRotation, 1, "no rotation rejected",
                 "COMMAND.C F0361");
    check_int_eq(result.guardRejectsNoCloseQueued, 1,
                 "no C040 close rejected", "C040");
    check_int_eq(result.leaderBefore, 0, "leader before", "F0380");
    check_int_eq(result.leaderAfter, 1, "leader after", "F0380");
    check_int_eq(result.g0299Before, 3, "G0299 before", "G0299");
    check_int_eq(result.g0299AfterRejectedC040, 3,
                 "G0299 after C040-gated reject", "G0299");
    check_int_eq(result.g0299AfterRejectedRotation, 3,
                 "G0299 after rotation-gated reject", "G0299");
    check_int_eq(result.g0299AfterAccept, 0, "G0299 after accept",
                 "REVIVE.C F0282");
    check_int_eq(result.c040CandidateIndexBefore, 2,
                 "C040 candidate index before", "C040");
    check_int_eq(result.c040CandidateIndexAfterRejectedC040, 2,
                 "C040 candidate index after C040 reject", "C040");
    check_int_eq(result.c040CandidateIndexAfterRejectedRotation, 2,
                 "C040 candidate index after rotation reject", "C040");
    check_u32_eq(result.c040RedrawStateBefore, 0x040c5450u,
                 "redraw state before", "PANEL.C F0353");
    check_u32_eq(result.c040RedrawStateAfterRejectedC040, 0x040c5450u,
                 "redraw state after C040 reject", "PANEL.C F0353");
    check_u32_eq(result.c040RedrawStateAfterRejectedRotation, 0x040c5450u,
                 "redraw state after rotation reject", "PANEL.C F0353");
    check_u32_eq(result.c545PanelPixelBefore, 0x00c545a5u,
                 "C545 pixel before", "C545");
    check_u32_eq(result.c545PanelPixelAfterRejectedC040, 0x00c545a5u,
                 "C545 pixel after C040 reject", "C545");
    check_u32_eq(result.c545PanelPixelAfterRejectedRotation, 0x00c545a5u,
                 "C545 pixel after rotation reject", "C545");
    check_int_eq(state.c040GateRejectCount, 1, "one C040 gate reject",
                 "C040");
    check_int_eq(state.rotationGateRejectCount, 1, "one rotation gate reject",
                 "COMMAND.C F0380");
    check_int_eq(state.f0282AcceptClearCount, 1, "one F0282 accept clear",
                 "REVIVE.C F0282");
    check_int_eq(state.f0378PanelRouteCount, 3, "three panel route attempts",
                 "COMMAND.C F0378");
    check_int_eq(state.f0380DrainCount, 3, "three drained commands",
                 "COMMAND.C F0380");
    check_int_eq(state.commandQueueDepth, 0, "queue drained", "F0380");
    check_int_eq(state.candidateSensorDisabled, 1, "sensor disabled",
                 "REVIVE.C F0282:796-799");
    check_int_eq(state.candidateRemovedFromChain, 1,
                 "candidate chain removed", "REVIVE.C F0282");
    for (i = 0; i < DM1_V1_MC_C545_ACCEPT_ROTATE_TRACE_COUNT_PC34; ++i) {
        check_int_eq(result.trace[i], 200 + i, "trace order",
                     "C040 close then rotation then accept");
    }
    for (i = 0; i < DM1_V1_MC_C545_ACCEPT_ROTATE_CHAIN_COUNT_PC34; ++i) {
        check_int_eq(result.candidateChainAfterRejectedC040[i],
                     result.candidateChainBefore[i],
                     "chain stable after C040 reject", "REVIVE.C F0282");
        check_int_eq(result.candidateChainAfterRejectedRotation[i],
                     result.candidateChainBefore[i],
                     "chain stable after rotation reject", "REVIVE.C F0282");
    }
    check_int_eq(result.candidateChainAfterAccept[0], 4,
                 "accepted candidate removed from chain", "REVIVE.C F0282");
    check_true(result.beforeHash != 0u, "before hash", "determinism");
    check_true(result.afterRejectedC040Hash != 0u, "C040 reject hash",
               "determinism");
    check_true(result.afterCloseRejectedRotationHash != 0u,
               "rotation reject hash", "determinism");
    check_true(result.afterAcceptHash != 0u, "accept hash", "determinism");
    check_true(result.hash != 0u, "result hash", "determinism");
    check_true(result.afterAcceptHash != result.afterCloseRejectedRotationHash,
               "accept changes deterministic hash", "determinism");
    return result.hash;
}

static void test_rejects(void)
{
    Dm1V1MirrorCandidateC545AcceptDuringRotationStatePc34 state;
    Dm1V1MirrorCandidateC545AcceptDuringRotationResultPc34 result;

    check_int_eq(dm1_v1_mirror_candidate_c545_accept_during_rotation_run_pc34(
                     NULL, &result),
                 0, "null state rejected", "guard");
    dm1_v1_mirror_candidate_c545_accept_during_rotation_init_pc34(&state);
    check_int_eq(dm1_v1_mirror_candidate_c545_accept_during_rotation_run_pc34(
                     &state, NULL),
                 0, "null result rejected", "guard");
    dm1_v1_mirror_candidate_c545_accept_during_rotation_init_pc34(&state);
    state.contractOnly = 0;
    check_int_eq(dm1_v1_mirror_candidate_c545_accept_during_rotation_run_pc34(
                     &state, &result),
                 0, "non-contract rejected", "asset-free");
    dm1_v1_mirror_candidate_c545_accept_during_rotation_init_pc34(&state);
    state.g0299CandidateOrdinal = 0;
    check_int_eq(dm1_v1_mirror_candidate_c545_accept_during_rotation_run_pc34(
                     &state, &result),
                 0, "missing candidate rejected", "G0299");
    dm1_v1_mirror_candidate_c545_accept_during_rotation_init_pc34(&state);
    state.panelContent = 565;
    check_int_eq(dm1_v1_mirror_candidate_c545_accept_during_rotation_run_pc34(
                     &state, &result),
                 0, "wrong panel rejected", "M568");
    dm1_v1_mirror_candidate_c545_accept_during_rotation_init_pc34(&state);
    state.leaderRotationQueued = 0;
    check_int_eq(dm1_v1_mirror_candidate_c545_accept_during_rotation_run_pc34(
                     &state, &result),
                 0, "missing rotation rejected", "COMMAND.C F0361");
    dm1_v1_mirror_candidate_c545_accept_during_rotation_init_pc34(&state);
    state.c040CloseQueued = 0;
    check_int_eq(dm1_v1_mirror_candidate_c545_accept_during_rotation_run_pc34(
                     &state, &result),
                 0, "missing C040 close rejected", "C040");
}

int main(void)
{
    uint32_t hash;

    printf("probe=dm1_v1_mirror_candidate_c545_accept_during_rotation_pc34_compat\n");
    printf("%s\n",
           dm1_v1_mirror_candidate_c545_accept_during_rotation_source_evidence_pc34());
    test_evidence();
    test_initial_state();
    hash = test_run();
    test_rejects();
    if (g_failures || g_assertions < 145) {
        printf("FAIL assertions=%d failures=%d hash=0x%08X\n", g_assertions,
               g_failures, hash);
        return 1;
    }
    printf("PASS test_dm1_v1_mirror_candidate_c545_accept_during_rotation_pc34_compat assertions=%d failures=0 hash=0x%08X\n",
           g_assertions, hash);
    return 0;
}
