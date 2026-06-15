#include "firestaff/dm1/v1/mirror/dm1_v1_mirror_candidate_click_cancel_with_rotation_pc34_compat.h"

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

static void check_u32_nonzero(uint32_t actual, const char *message,
                              const char *anchor)
{
    ++g_assertions;
    if (actual == 0u) {
        ++g_failures;
        printf("FAIL %s actual=0x%08x [%s]\n", message, actual,
               anchor ? anchor : "(null)");
    }
}

static void check_contains(const char *haystack, const char *needle,
                           const char *message, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || !strstr(haystack, needle)) {
        ++g_failures;
        printf("FAIL %s missing=%s [%s]\n", message,
               needle ? needle : "(null)", anchor ? anchor : "(null)");
    }
}

static void test_evidence(void)
{
    const Dm1V1MirrorCandidateClickCancelWithRotationEvidencePc34Compat *e =
        dm1_v1_mirror_candidate_click_cancel_with_rotation_evidence_pc34();
    const char *text =
        dm1_v1_mirror_candidate_click_cancel_with_rotation_source_evidence_pc34();

    check_true(e != NULL, "evidence accessor", "source-lock");
    check_contains(e->championPanelRedrawAnchor, "F0296:1208-1262",
                   "F0296 anchor", e->championPanelRedrawAnchor);
    check_contains(e->championPanelRedrawAnchor, "G0420",
                   "G0420 anchor", e->championPanelRedrawAnchor);
    check_contains(e->leaderHandAnchor, "F0297:243-298", "F0297 anchor",
                   e->leaderHandAnchor);
    check_contains(e->leaderHandAnchor, "F0298:270-298", "F0298 anchor",
                   e->leaderHandAnchor);
    check_contains(e->championSlotAnchor, "F0300:511-515",
                   "F0300 anchor", e->championSlotAnchor);
    check_contains(e->championSlotAnchor, "F0301:606-614",
                   "F0301 anchor", e->championSlotAnchor);
    check_contains(e->championSlotAnchor, "F0302:662-714",
                   "F0302 anchor", e->championSlotAnchor);
    check_contains(e->reviveAnchor, "F0280:124-132", "F0280 anchor",
                   e->reviveAnchor);
    check_contains(e->reviveAnchor, "F0282:744-806", "F0282 anchor",
                   e->reviveAnchor);
    check_contains(e->commandAnchor, "F0359:1985-1990", "F0359 anchor",
                   e->commandAnchor);
    check_contains(e->panelAnchor, "F0344:1493-1561", "F0344 anchor",
                   e->panelAnchor);
    check_contains(e->panelAnchor, "F0345:1563-1616", "F0345 anchor",
                   e->panelAnchor);
    check_contains(e->panelAnchor, "F0352", "F0352 anchor",
                   e->panelAnchor);
    check_contains(e->panelAnchor, "F0354:2299-2352", "F0354 anchor",
                   e->panelAnchor);
    check_contains(e->defsAnchor, "C030", "C030 defs", e->defsAnchor);
    check_contains(e->defsAnchor, "C033/C034/C035", "slot graphic defs",
                   e->defsAnchor);
    check_contains(e->defsAnchor, "C040", "C040 defs", e->defsAnchor);
    check_contains(e->defsAnchor, "C045", "C045 defs", e->defsAnchor);
    check_contains(e->defsAnchor, "C151..C154", "status zones",
                   e->defsAnchor);
    check_contains(e->defsAnchor, "C113..C116", "icon zones",
                   e->defsAnchor);
    check_contains(e->defsAnchor, "G0299", "G0299 defs", e->defsAnchor);
    check_contains(e->defsAnchor, "G0423", "G0423 defs", e->defsAnchor);
    check_contains(e->defsAnchor, "G0425", "G0425 defs", e->defsAnchor);
    check_contains(e->defsAnchor, "G0426", "G0426 defs", e->defsAnchor);
    check_contains(e->nonOverlap, "click then C162 cancel", "race scope",
                   e->nonOverlap);
    check_contains(e->nonOverlap, "not select+commit", "non-overlap commit",
                   e->nonOverlap);
    check_contains(e->nonOverlap, "rotation-during-resurrect",
                   "non-overlap rotation during resurrect", e->nonOverlap);
    check_contains(e->nonOverlap, "save/load", "non-overlap save load",
                   e->nonOverlap);
    check_contains(e->nonOverlap, "teleporter", "non-overlap teleporter",
                   e->nonOverlap);
    check_contains(e->nonOverlap, "C045", "non-overlap C045",
                   e->nonOverlap);
    check_contains(text, "COMMAND.C F0359:1985-1990", "source F0359", text);
    check_contains(text, "PANEL.C F0344:1493-1561", "source F0344", text);
    check_contains(text, "G0420", "source G0420", text);
    check_contains(text, "no save/load", "source non-overlap", text);
}

static void test_initial_state(void)
{
    Dm1V1MirrorCandidateClickCancelWithRotationStatePc34Compat state;
    int i;

    dm1_v1_mirror_candidate_click_cancel_with_rotation_init_pc34(&state);
    check_int_eq(state.contractOnly, 1, "contract only", "runtime gate");
    check_int_eq(state.partyChampionCount, 4, "four champion party",
                 "REVIVE.C F0280:124-132");
    check_int_eq(state.leaderIndex, 0, "leader 0 starts active",
                 "CHAMPION.C F0302:662-714");
    check_int_eq(state.pendingLeaderIndex, 1, "leader 1 queued",
                 "CHAMPION.C F0301:606-614");
    check_int_eq(state.rotationInFlight, 1, "rotation in flight",
                 "CHAMPION.C F0302:662-714");
    check_int_eq(state.leaderHandThing, 0xffff, "leader hand empty",
                 "COMMAND.C F0359:1985-1990");
    check_int_eq(state.leaderHandEmpty, 1, "empty-hand guard satisfied",
                 "REVIVE.C F0280:124-132");
    check_int_eq(state.c040PanelOpen, 1, "C040 panel live", "C040");
    check_int_eq(state.panelContent, 568, "M568 panel content", "M568");
    check_int_eq(state.panelGraphic, 40, "C040 panel graphic", "C040");
    check_int_eq(state.candidateOwnerIndex, 0, "leader owns chain",
                 "REVIVE.C F0280");
    check_int_eq(state.candidateChainIndex, 1, "chain index 1 of 3",
                 "REVIVE.C F0280");
    check_int_eq(state.candidateChainCount, 3, "candidate chain count",
                 "REVIVE.C F0280");
    check_int_eq(state.g0299CandidateOrdinal, 0, "G0299 starts clear",
                 "G0299");
    check_int_eq(state.selectedCandidateOrdinal, 0, "no selected candidate",
                 "G0299");
    check_int_eq(state.selectedCandidateCommitted, 0, "no committed select",
                 "REVIVE.C F0282");
    check_int_eq(state.c040RedrawState, 40, "browse redraw state",
                 "CHAMPION.C F0296:1208-1262");
    check_int_eq(state.f0280PublishCount, 1, "F0280 candidate publish seed",
                 "REVIVE.C F0280:124-132");
    check_int_eq(state.f0282CancelCount, 0, "no cancel yet",
                 "REVIVE.C F0282:744-806");
    check_int_eq(state.resurrectPendingCount, 0, "no resurrect pending",
                 "REVIVE.C F0282:744-806");
    check_u32_nonzero(state.chainHash, "chain hash seeded", "determinism");
    check_u32_nonzero(state.stateHash, "state hash seeded", "determinism");

    for (i = 0; i < DM1_V1_MC_CC_ROT_PARTY_COUNT_PC34; ++i) {
        check_int_eq(state.champions[i].championOrdinal, i + 1,
                     "champion ordinal", "M516");
        check_int_eq(state.champions[i].alive, 1, "champion alive", "M516");
        check_int_eq(state.champions[i].statusBoxZone, 151 + i,
                     "status box zone", "C151..C154");
        check_int_eq(state.champions[i].championIconZone, 113 + i,
                     "champion icon zone", "C113..C116");
    }
    check_int_eq(state.champions[0].leader, 1, "champion 0 leader",
                 "CHAMPION.C F0302");
    check_int_eq(state.champions[1].leader, 0, "champion 1 not leader yet",
                 "CHAMPION.C F0301");
    check_int_eq(state.champions[0].c040ChainLinked, 1,
                 "old leader owns candidate chain", "REVIVE.C F0280");
    check_int_eq(state.champions[1].c040ChainLinked, 0,
                 "queued leader has no candidate chain", "F0301/F0302");
}

static uint32_t test_run(void)
{
    Dm1V1MirrorCandidateClickCancelWithRotationStatePc34Compat state;
    Dm1V1MirrorCandidateClickCancelWithRotationResultPc34Compat result;
    uint32_t initialChainHash;
    int ok;

    dm1_v1_mirror_candidate_click_cancel_with_rotation_init_pc34(&state);
    initialChainHash = state.chainHash;
    ok = dm1_v1_mirror_candidate_click_cancel_with_rotation_run_pc34(
        &state, &result);

    check_int_eq(ok, 1, "run accepted", "contract");
    check_int_eq(result.accepted, 1, "result accepted", "contract");
    check_int_eq(result.sameTickSequence, 1, "same tick sequence",
                 "COMMAND.C F0359:1985-1990");
    check_int_eq(result.selectConsumed, 1, "select consumed",
                 "COMMAND.C F0359:1985-1990");
    check_int_eq(result.cancelConsumed, 1, "cancel consumed",
                 "REVIVE.C F0282:744-806");
    check_int_eq(result.rotationConsumed, 1, "rotation consumed",
                 "CHAMPION.C F0302:662-714");
    check_int_eq(result.initialLeaderIndex, 0, "initial leader",
                 "CHAMPION.C F0302");
    check_int_eq(result.finalLeaderIndex, 1, "final leader",
                 "CHAMPION.C F0301:606-614");
    check_int_eq(result.pendingLeaderIndexBefore, 1, "pending leader before",
                 "CHAMPION.C F0302");
    check_int_eq(result.pendingLeaderIndexAfter, DM1_V1_MC_CC_ROT_NONE_PC34,
                 "pending leader consumed", "CHAMPION.C F0301");
    check_int_eq(result.g0299BeforeClick, 0, "G0299 before click", "G0299");
    check_true(result.g0299AfterClick != 0, "G0299 set by click",
               "COMMAND.C F0359");
    check_int_eq(result.g0299AfterCancel, 0, "G0299 cleared by cancel",
                 "REVIVE.C F0282:744-806");
    check_int_eq(result.g0299AfterRotation, 0, "G0299 clear after rotation",
                 "CHAMPION.C F0302");
    check_int_eq(result.selectedCandidateAfterClick, 42,
                 "clicked chain candidate selected", "G0299");
    check_int_eq(result.selectedCandidateAfterCancel, 0,
                 "cancel clears selected candidate", "REVIVE.C F0282");
    check_int_eq(result.selectedCandidateCommittedAfterCancel, 0,
                 "cancel prevents commit", "REVIVE.C F0282");
    check_int_eq(result.c040PanelOpenBefore, 1, "C040 panel initially open",
                 "C040");
    check_int_eq(result.c040PanelOpenAfterCancel, 0,
                 "cancel closes C040 panel", "REVIVE.C F0282");
    check_int_eq(result.c040RedrawStateBefore, 40, "redraw before",
                 "CHAMPION.C F0296");
    check_int_eq(result.c040RedrawStateAfterCancel, 0,
                 "redraw no-candidate after cancel", "PANEL.C F0354");
    check_int_eq(result.c040RedrawStateAfterRotation, 0,
                 "rotation keeps no-candidate redraw", "CHAMPION.C F0296");
    check_int_eq(result.oldLeaderOwnsCandidateChainAfter, 1,
                 "old leader retains chain", "REVIVE.C F0280");
    check_int_eq(result.newLeaderInheritedCandidate, 0,
                 "new leader did not inherit chain", "CHAMPION.C F0301");
    check_int_eq(result.chainCountBefore, 3, "chain count before",
                 "REVIVE.C F0280");
    check_int_eq(result.chainCountAfter, 3, "chain count after",
                 "REVIVE.C F0282");
    check_int_eq(result.chainIndexBefore, 1, "chain index before",
                 "REVIVE.C F0280");
    check_int_eq(result.chainIndexAfter, 1, "chain index after",
                 "REVIVE.C F0282");
    check_int_eq(result.candidateChainPreserved, 1, "candidate chain kept",
                 "REVIVE.C F0280/F0282");
    check_int_eq(result.noResurrectPendingStarted, 1,
                 "no resurrect pending started", "REVIVE.C F0282");
    check_int_eq(result.noSaveLoadOrTeleporterPath, 1,
                 "no save/load/teleporter path", "non-overlap");
    check_int_eq(result.f0302WasBlockedWhileG0299BeforeCancel, 1,
                 "rotation would block before cancel", "CHAMPION.C F0302:677-679");
    check_int_eq(result.f0302AllowedAfterCancel, 1,
                 "rotation allowed after cancel", "CHAMPION.C F0302:662-714");
    check_int_eq(result.f0301LeaderWriteCount, 1, "one F0301 write",
                 "CHAMPION.C F0301:606-614");
    check_int_eq(result.f0296PointerHideShowBalanced, 1,
                 "pointer hide/show balanced", "CHAMPION.C F0296:1208-1262");
    check_int_eq(result.panelRedrawReturnedToNoCandidate, 1,
                 "panel redraw no-candidate", "PANEL.C F0354");
    check_int_eq(result.statusBoxRedrawUsesNewLeader, 1,
                 "status redraw uses new leader", "C151..C154");
    check_int_eq(result.championIconRedrawUsesNewLeader, 1,
                 "icon redraw uses new leader", "C113..C116");
    check_int_eq(result.sourceLockAnchorsPresent, 1,
                 "source anchors present", "ReDMCSB");
    check_u32_nonzero(result.beforeHash, "before hash", "determinism");
    check_u32_nonzero(result.afterClickHash, "after click hash",
                      "determinism");
    check_u32_nonzero(result.afterCancelHash, "after cancel hash",
                      "determinism");
    check_u32_nonzero(result.afterRotationHash, "after rotation hash",
                      "determinism");
    check_u32_nonzero(result.deterministicHash, "deterministic hash",
                      "determinism");
    check_true(result.beforeHash != result.afterClickHash,
               "click mutates transient state", "COMMAND.C F0359");
    check_true(result.afterClickHash != result.afterCancelHash,
               "cancel mutates transient state", "REVIVE.C F0282");
    check_true(result.afterCancelHash != result.afterRotationHash,
               "rotation mutates leader state", "CHAMPION.C F0301/F0302");

    check_int_eq(state.leaderIndex, 1, "state leader is champion 1",
                 "CHAMPION.C F0301");
    check_int_eq(state.rotationInFlight, 0, "rotation flag consumed",
                 "CHAMPION.C F0302");
    check_int_eq(state.pendingLeaderIndex, DM1_V1_MC_CC_ROT_NONE_PC34,
                 "pending leader cleared", "CHAMPION.C F0301");
    check_int_eq(state.g0299CandidateOrdinal, 0, "state G0299 clear",
                 "G0299");
    check_int_eq(state.selectedCandidateOrdinal, 0,
                 "state selected candidate clear", "REVIVE.C F0282");
    check_int_eq(state.c040PanelOpen, 0, "state C040 closed", "C040");
    check_int_eq(state.panelContent, 0, "panel content cleared",
                 "PANEL.C F0354");
    check_int_eq(state.panelGraphic, 0, "panel graphic cleared",
                 "PANEL.C F0354");
    check_int_eq(state.candidateOwnerIndex, 0, "owner remains old leader",
                 "REVIVE.C F0280");
    check_int_eq(state.champions[0].leader, 0, "old leader flag clear",
                 "CHAMPION.C F0301");
    check_int_eq(state.champions[1].leader, 1, "new leader flag set",
                 "CHAMPION.C F0301");
    check_int_eq(state.champions[0].c040ChainLinked, 1,
                 "old leader chain linked", "REVIVE.C F0280");
    check_int_eq(state.champions[1].c040ChainLinked, 0,
                 "new leader chain not linked", "CHAMPION.C F0302");
    check_int_eq(state.chainHash == initialChainHash, 1, "chain hash kept",
                 "REVIVE.C F0280/F0282");
    check_int_eq(state.f0359SelectCount, 1, "one F0359 select",
                 "COMMAND.C F0359");
    check_int_eq(state.f0359CancelCount, 1, "one F0359 cancel",
                 "COMMAND.C F0359");
    check_int_eq(state.f0282CancelCount, 1, "one F0282 cancel",
                 "REVIVE.C F0282");
    check_int_eq(state.f0302RotationDispatchCount, 1, "one F0302 rotation",
                 "CHAMPION.C F0302");
    check_int_eq(state.f0301LeaderWriteCount, 1, "one F0301 leader write",
                 "CHAMPION.C F0301");
    check_int_eq(state.f0296HidePointerCount, 1, "one pointer hide",
                 "G0420");
    check_int_eq(state.f0296ShowPointerCount, 1, "one pointer show",
                 "G0420");
    check_int_eq(state.panelF0354Count, 1, "one panel close redraw",
                 "PANEL.C F0354");
    check_int_eq(state.panelF0344Count, 1, "one F0344 redraw accounting",
                 "PANEL.C F0344");
    check_int_eq(state.panelF0345Count, 1, "one F0345 redraw accounting",
                 "PANEL.C F0345");
    check_int_eq(state.resurrectPendingCount, 0, "still no resurrect pending",
                 "REVIVE.C F0282");
    return result.deterministicHash;
}

static void test_null_guards(void)
{
    Dm1V1MirrorCandidateClickCancelWithRotationStatePc34Compat state;
    Dm1V1MirrorCandidateClickCancelWithRotationResultPc34Compat result;

    dm1_v1_mirror_candidate_click_cancel_with_rotation_init_pc34(&state);
    check_int_eq(dm1_v1_mirror_candidate_click_cancel_with_rotation_run_pc34(
                     NULL, &result),
                 0, "null state rejected", "guard");
    check_int_eq(dm1_v1_mirror_candidate_click_cancel_with_rotation_run_pc34(
                     &state, NULL),
                 0, "null result rejected", "guard");
    state.rotationInFlight = 0;
    check_int_eq(dm1_v1_mirror_candidate_click_cancel_with_rotation_run_pc34(
                     &state, &result),
                 0, "missing rotation rejected", "non-duplicative scope");
}

int main(void)
{
    uint32_t hash;

    test_evidence();
    test_initial_state();
    hash = test_run();
    test_null_guards();

    if (g_failures) {
        printf("FAIL test_dm1_v1_mirror_candidate_click_cancel_with_rotation_pc34_compat assertions=%d failures=%d hash=0x%08x\n",
               g_assertions, g_failures, hash);
        return 1;
    }
    printf("PASS test_dm1_v1_mirror_candidate_click_cancel_with_rotation_pc34_compat assertions=%d failures=0 hash=0x%08x\n",
           g_assertions, hash);
    return 0;
}
