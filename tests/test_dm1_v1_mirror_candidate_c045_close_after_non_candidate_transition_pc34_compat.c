#include "firestaff/dm1/v1/mirror/dm1_v1_mirror_candidate_c045_close_after_non_candidate_transition_pc34_compat.h"

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
        printf("FAIL %s missing=%s [%s]\n", message,
               needle ? needle : "(null)", anchor ? anchor : "(null)");
    }
}

static void test_evidence(void)
{
    const Dm1V1MirrorCandidateC045AfterNonCandidateEvidencePc34Compat *e =
        dm1_v1_mirror_candidate_c045_close_after_non_candidate_transition_evidence_pc34();
    const char *text =
        dm1_v1_mirror_candidate_c045_close_after_non_candidate_transition_source_evidence_pc34();

    check_true(e != NULL, "evidence exists", "source lock");
    check_contains(e->chestOpenAnchor, "F0333:30-67", "F0333 anchor",
                   e->chestOpenAnchor);
    check_contains(e->chestCloseAnchor, "F0334:113-132", "F0334 anchor",
                   e->chestCloseAnchor);
    check_contains(e->championAnchor, "F0297:243-298", "F0297 anchor",
                   e->championAnchor);
    check_contains(e->championAnchor, "F0298:270-298", "F0298 anchor",
                   e->championAnchor);
    check_contains(e->championAnchor, "F0300:511-515", "F0300 anchor",
                   e->championAnchor);
    check_contains(e->championAnchor, "F0301:606-614", "F0301 anchor",
                   e->championAnchor);
    check_contains(e->championAnchor, "F0302:662-714", "F0302 anchor",
                   e->championAnchor);
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
    check_contains(e->panelAnchor, "F0354:2299-2352", "F0354 anchor",
                   e->panelAnchor);
    check_contains(e->defsAnchor, "C030", "C030 defs", e->defsAnchor);
    check_contains(e->defsAnchor, "C040", "C040 defs", e->defsAnchor);
    check_contains(e->defsAnchor, "C045", "C045 defs", e->defsAnchor);
    check_contains(e->defsAnchor, "C537..C544", "C537..C544 defs",
                   e->defsAnchor);
    check_contains(e->defsAnchor, "G0425", "G0425 defs", e->defsAnchor);
    check_contains(e->defsAnchor, "G0426", "G0426 defs", e->defsAnchor);
    check_contains(e->scope, "pass674", "pass674 disjoint", e->scope);
    check_contains(e->scope, "pass686", "pass686 disjoint", e->scope);
    check_contains(e->scope, "pass710", "pass710 disjoint", e->scope);
    check_contains(e->scope, "pass711", "pass711 disjoint", e->scope);
    check_contains(e->scope, "pass736", "pass736 disjoint", e->scope);
    check_contains(e->scope, "pass745", "pass745 disjoint", e->scope);
    check_contains(e->scope, "pass765plus", "pass765plus disjoint",
                   e->scope);
    check_contains(text, "CHEST.C F0333:30-67", "source F0333", text);
    check_contains(text, "CHEST.C F0334:113-132", "source F0334", text);
    check_contains(text, "CHAMPION.C F0297:243-298", "source F0297", text);
    check_contains(text, "F0298:270-298", "source F0298", text);
    check_contains(text, "F0300:511-515", "source F0300", text);
    check_contains(text, "F0301:606-614", "source F0301", text);
    check_contains(text, "F0302:662-714", "source F0302", text);
    check_contains(text, "REVIVE.C F0280:124-132", "source F0280", text);
    check_contains(text, "F0282:744-806", "source F0282", text);
    check_contains(text, "COMMAND.C F0359:1985-1990", "source F0359", text);
    check_contains(text, "PANEL.C F0344:1493-1561", "source F0344", text);
    check_contains(text, "F0345:1563-1616", "source F0345", text);
    check_contains(text, "F0354:2299-2352", "source F0354", text);
    check_contains(text, "DEFS.H:267 C030", "source C030", text);
    check_contains(text, "2200 C040", "source C040", text);
    check_contains(text, "2205 C045", "source C045", text);
    check_contains(text, "3906-3913 C537..C544", "source C537..C544",
                   text);
}

static void test_initial_state(void)
{
    Dm1V1MirrorCandidateC045AfterNonCandidateStatePc34Compat state;
    int i;

    dm1_v1_mirror_candidate_c045_close_after_non_candidate_transition_init_pc34(
        &state);
    check_int_eq(state.contractOnly, 1, "contract-only state",
                 "asset-free");
    check_int_eq(state.transitionKind,
                 DM1_V1_MC_C045_AFTER_NC_TRANSITION_C040_CHROME_PC34,
                 "default transition", "pass686 disjoint");
    check_int_eq(state.transitionApplied, 0, "transition not yet applied",
                 "determinism");
    check_int_eq(state.nonCandidateTransition, 1, "non-candidate transition",
                 "G0299");
    check_int_eq(state.leaderIndex, 0, "leader index", "M516");
    check_int_eq(state.inventoryChampionOrdinal, 1, "inventory ordinal",
                 "G0423");
    check_int_eq(state.leaderEmptyHanded, 0, "leader hand occupied",
                 "REVIVE.C F0280:124-132");
    check_u16_eq(state.leaderHandC30Thing, 0x4c30u,
                 "leader-hand C30 thing", "CHAMPION.C F0297/F0298");
    check_u16_eq(state.g0426OpenChest, 0x6420u, "G0426 open chest",
                 "CHEST.C F0333/F0334");
    check_int_eq(state.c540SlotIndex, 3, "C540 slot index", "C540");
    check_int_eq(state.c540Zone, 540, "C540 route", "DEFS.H:3909");
    check_int_eq(state.c540Command, 61, "C540 command", "COMMAND.C:502");
    check_int_eq(state.c045Graphic, 45, "C045 graphic", "DEFS.H:2205");
    check_int_eq(state.panelContentBefore, 1, "food/water panel",
                 "M565");
    check_int_eq(state.panelContentAfterTransition, 1,
                 "transition preserves food/water panel", "M565");
    check_int_eq(state.panelContentAfterClose, 1, "pre-close panel content",
                 "M565");
    check_int_eq(state.panelOpen, 1, "panel open before close",
                 "PANEL.C F0345");
    check_int_eq(state.candidateChampionOrdinal, 0, "no candidate",
                 "G0299");
    check_int_eq(state.f0280Entered, 0, "F0280 not entered",
                 "REVIVE.C F0280");
    check_int_eq(state.f0282Entered, 0, "F0282 not entered",
                 "REVIVE.C F0282");
    check_int_eq(state.f0333MaterializeCount, 0, "F0333 not on close path",
                 "CHEST.C F0333");
    check_int_eq(state.f0334RelinkCount, 0, "F0334 not on close path",
                 "CHEST.C F0334");
    check_int_eq(state.f0344FoodWaterReadCount, 2, "food/water reads seeded",
                 "PANEL.C F0344");
    check_int_eq(state.f0345FoodWaterDrawCount, 1, "food/water draw seeded",
                 "PANEL.C F0345");
    check_true(state.beforeHash != 0u, "before hash nonzero",
               "determinism");
    for (i = 0; i < DM1_V1_MC_C045_AFTER_NC_SLOT_COUNT_PC34; ++i) {
        check_u16_eq(state.sourceChain[i], (uint16_t)(0x6100u + i),
                     "source chain seed", "CHEST.C F0333:30-67");
        check_u16_eq(state.visibleSlots[i], state.sourceChain[i],
                     "visible slots mirror source", "G0425");
    }
}

static uint32_t run_one(int transition_kind, uint32_t expected_close_hash)
{
    Dm1V1MirrorCandidateC045AfterNonCandidateStatePc34Compat state;
    Dm1V1MirrorCandidateC045AfterNonCandidateResultPc34Compat result;
    int ok;
    int i;

    dm1_v1_mirror_candidate_c045_close_after_non_candidate_transition_init_pc34(
        &state);
    state.transitionKind = transition_kind;
    ok = dm1_v1_mirror_candidate_c045_close_after_non_candidate_transition_run_pc34(
        &state, &result);
    check_int_eq(ok, 1, "run accepted", "COMMAND.C F0359");
    check_int_eq(result.accepted, 1, "result accepted", "contract");
    check_int_eq(result.transitionWasAfterNonCandidate, 1,
                 "transition is non-candidate", "G0299");
    check_int_eq(result.closeFiredAfterTransition, 1,
                 "close fired after transition", "PANEL.C F0354");
    check_int_eq(result.leaderHandPreserved, 1, "leader hand preserved",
                 "CHAMPION.C F0297/F0298");
    check_int_eq(result.c30ChainPreserved, 1, "C30 chain preserved",
                 "CHAMPION.C F0300/F0301");
    check_int_eq(result.noLeaderHandMutation, 1, "no leader-hand mutation",
                 "CHAMPION.C F0297/F0298/F0300/F0301/F0302");
    check_int_eq(result.noF0280C040Entry, 1, "no F0280 C040 entry",
                 "REVIVE.C F0280:124-132");
    check_int_eq(result.noF0282CandidateEntry, 1, "no F0282 entry",
                 "REVIVE.C F0282:744-806");
    check_int_eq(result.g0426Preserved, 1, "G0426 preserved",
                 "CHEST.C F0334");
    check_int_eq(result.visibleSlotsPreserved, 1, "visible slots preserved",
                 "G0425");
    check_int_eq(result.c540RoutePreserved, 1, "C540 route preserved",
                 "DEFS.H:3909");
    check_int_eq(result.noF0333MaterializeOnClose, 1,
                 "no F0333 chain materialize", "CHEST.C F0333:30-67");
    check_int_eq(result.noF0334RelinkOnClose, 1, "no F0334 relink",
                 "CHEST.C F0334:113-132");
    check_int_eq(result.noC040Dispatch, 1, "no C040 dispatch",
                 "COMMAND.C F0359:1985-1990");
    check_int_eq(result.c045PanelClosed, 1, "C045 panel closed",
                 "PANEL.C F0354");
    check_int_eq(result.c503CloseObserved, 1, "C503 close observed",
                 "PANEL.C F0354");
    check_int_eq(result.foodWaterReadStable, 1, "food/water read stable",
                 "PANEL.C F0344/F0345");
    check_int_eq(result.deterministicAgainstNoTransition, 1,
                 "invariant to prior transition", "determinism");
    check_int_eq(result.transitionHashChanged, 1, "transition hash sane",
                 "determinism");
    check_int_eq(result.closeHashStable, 1, "close hash stable",
                 "determinism");
    check_int_eq(result.guardRejectsEmptyLeaderHand, 1,
                 "empty leader-hand guard", "REVIVE.C F0280");
    check_int_eq(result.guardRejectsCandidate, 1, "candidate guard",
                 "G0299");
    check_int_eq(result.guardRejectsWrongRoute, 1, "wrong C540 guard",
                 "C537..C544");
    check_int_eq(result.guardRejectsClosedChest, 1, "closed chest guard",
                 "G0426");
    check_u16_eq(result.leaderHandBefore, 0x4c30u, "leader hand before",
                 "C30");
    check_u16_eq(result.leaderHandAfter, 0x4c30u, "leader hand after",
                 "C30");
    check_u16_eq(result.g0426Before, 0x6420u, "G0426 before", "G0426");
    check_u16_eq(result.g0426After, 0x6420u, "G0426 after", "G0426");
    check_int_eq(state.panelOpen, 0, "panel closed", "PANEL.C F0354");
    check_int_eq(state.panelContentAfterClose, 0, "panel content closed",
                 "PANEL.C F0354");
    check_u16_eq(state.leaderHandC30Thing, 0x4c30u,
                 "state leader hand survives", "CHAMPION.C F0297/F0298");
    check_u16_eq(state.g0426OpenChest, 0x6420u, "state G0426 survives",
                 "CHEST.C F0334");
    check_int_eq(state.f0280Entered, 0, "state F0280 count zero",
                 "REVIVE.C F0280");
    check_int_eq(state.f0282Entered, 0, "state F0282 count zero",
                 "REVIVE.C F0282");
    check_int_eq(state.f0333MaterializeCount, 0, "state F0333 zero",
                 "CHEST.C F0333");
    check_int_eq(state.f0334RelinkCount, 0, "state F0334 zero",
                 "CHEST.C F0334");
    check_int_eq(state.f0297PutLeaderHandCount, 0, "state F0297 zero",
                 "CHAMPION.C F0297");
    check_int_eq(state.f0298RemoveLeaderHandCount, 0, "state F0298 zero",
                 "CHAMPION.C F0298");
    check_int_eq(state.f0300RemoveSlotCount, 0, "state F0300 zero",
                 "CHAMPION.C F0300");
    check_int_eq(state.f0301AddSlotCount, 0, "state F0301 zero",
                 "CHAMPION.C F0301");
    check_int_eq(state.f0302SlotCommandCount, 0, "state F0302 zero",
                 "CHAMPION.C F0302");
    check_int_eq(state.f0354CloseCount, 1, "state F0354 close",
                 "PANEL.C F0354");
    check_true(state.closeHash != 0u, "state close hash nonzero",
               "determinism");
    check_u32_eq(state.closeHash, result.baselineCloseHash,
                 "close hash equals no-transition baseline", "determinism");
    if (expected_close_hash != 0u) {
        check_u32_eq(state.closeHash, expected_close_hash,
                     "close hash matches first transition", "determinism");
    }
    for (i = 0; i < DM1_V1_MC_C045_AFTER_NC_SLOT_COUNT_PC34; ++i) {
        check_u16_eq(state.sourceChain[i], (uint16_t)(0x6100u + i),
                     "source chain still seeded", "CHEST.C F0334");
        check_u16_eq(state.visibleSlots[i], (uint16_t)(0x6100u + i),
                     "visible slots still seeded", "G0425");
        check_u16_eq(result.visibleSlotsAfter[i], state.visibleSlots[i],
                     "result visible slots mirror state", "G0425");
    }
    return state.closeHash;
}

static void test_all_transitions(uint32_t *out_hash)
{
    uint32_t baseline_hash;
    uint32_t c038_hash;
    uint32_t c040_hash;
    uint32_t c503_hash;
    uint32_t redraw_hash;

    baseline_hash = run_one(DM1_V1_MC_C045_AFTER_NC_TRANSITION_NONE_PC34, 0u);
    c038_hash = run_one(DM1_V1_MC_C045_AFTER_NC_TRANSITION_C038_CANCEL_PC34,
                        baseline_hash);
    c040_hash = run_one(DM1_V1_MC_C045_AFTER_NC_TRANSITION_C040_CHROME_PC34,
                        baseline_hash);
    c503_hash = run_one(
        DM1_V1_MC_C045_AFTER_NC_TRANSITION_C503_C018_CHROME_PC34,
        baseline_hash);
    redraw_hash = run_one(
        DM1_V1_MC_C045_AFTER_NC_TRANSITION_PANEL_REDRAW_PC34, baseline_hash);
    check_u32_eq(c038_hash, baseline_hash, "C038 close invariant",
                 "determinism");
    check_u32_eq(c040_hash, baseline_hash, "C040 chrome close invariant",
                 "determinism");
    check_u32_eq(c503_hash, baseline_hash, "C503/C018 close invariant",
                 "determinism");
    check_u32_eq(redraw_hash, baseline_hash, "redraw close invariant",
                 "determinism");
    *out_hash = baseline_hash;
}

static void test_rejects(void)
{
    Dm1V1MirrorCandidateC045AfterNonCandidateStatePc34Compat state;
    Dm1V1MirrorCandidateC045AfterNonCandidateResultPc34Compat result;

    check_int_eq(
        dm1_v1_mirror_candidate_c045_close_after_non_candidate_transition_run_pc34(
            NULL, &result),
        0, "null state rejected", "guard");
    dm1_v1_mirror_candidate_c045_close_after_non_candidate_transition_init_pc34(
        &state);
    check_int_eq(
        dm1_v1_mirror_candidate_c045_close_after_non_candidate_transition_run_pc34(
            &state, NULL),
        0, "null result rejected", "guard");
    dm1_v1_mirror_candidate_c045_close_after_non_candidate_transition_init_pc34(
        &state);
    state.contractOnly = 0;
    check_int_eq(
        dm1_v1_mirror_candidate_c045_close_after_non_candidate_transition_run_pc34(
            &state, &result),
        0, "non-contract rejected", "asset-free");
    dm1_v1_mirror_candidate_c045_close_after_non_candidate_transition_init_pc34(
        &state);
    state.leaderEmptyHanded = 1;
    check_int_eq(
        dm1_v1_mirror_candidate_c045_close_after_non_candidate_transition_run_pc34(
            &state, &result),
        0, "empty leader hand rejected", "REVIVE.C F0280");
    dm1_v1_mirror_candidate_c045_close_after_non_candidate_transition_init_pc34(
        &state);
    state.candidateChampionOrdinal = 4;
    check_int_eq(
        dm1_v1_mirror_candidate_c045_close_after_non_candidate_transition_run_pc34(
            &state, &result),
        0, "candidate rejected", "G0299");
    dm1_v1_mirror_candidate_c045_close_after_non_candidate_transition_init_pc34(
        &state);
    state.c540Zone = 541;
    check_int_eq(
        dm1_v1_mirror_candidate_c045_close_after_non_candidate_transition_run_pc34(
            &state, &result),
        0, "wrong C540 route rejected", "C537..C544");
    dm1_v1_mirror_candidate_c045_close_after_non_candidate_transition_init_pc34(
        &state);
    state.g0426OpenChest = DM1_V1_MC_C045_AFTER_NC_NONE_PC34;
    check_int_eq(
        dm1_v1_mirror_candidate_c045_close_after_non_candidate_transition_run_pc34(
            &state, &result),
        0, "closed G0426 rejected", "G0426");
}

int main(void)
{
    uint32_t hash = 0;

    printf("probe=dm1_v1_mirror_candidate_c045_close_after_non_candidate_transition_pc34_compat\n");
    printf("%s\n",
           dm1_v1_mirror_candidate_c045_close_after_non_candidate_transition_source_evidence_pc34());
    test_evidence();
    test_initial_state();
    test_all_transitions(&hash);
    test_rejects();
    if (g_failures || g_assertions < 80) {
        printf("FAIL assertions=%d failures=%d hash=0x%08x\n",
               g_assertions, g_failures, hash);
        return 1;
    }
    printf("DM1_V1_MIRROR_CANDIDATE_C045_CLOSE_AFTER_NON_CANDIDATE_TRANSITION_PC34_COMPAT_OK assertions=%d failures=0 hash=0x%08x\n",
           g_assertions, hash);
    return 0;
}
