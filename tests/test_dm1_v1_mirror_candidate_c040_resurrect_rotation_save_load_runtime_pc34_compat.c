#include "firestaff/dm1/v1/mirror_candidate/c040_resurrect_rotation_save_load_runtime_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static int expect_int(const char *label, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (!anchor || anchor[0] == '\0' || got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n",
               label, got, want, anchor ? anchor : "(null)");
        return 0;
    }
    return 1;
}

static int expect_u32_eq(const char *label,
                         uint32_t got,
                         uint32_t want,
                         const char *anchor)
{
    ++g_assertions;
    if (!anchor || anchor[0] == '\0' || got != want) {
        ++g_failures;
        printf("FAIL %s got=0x%08X want=0x%08X anchor=%s\n",
               label, (unsigned)got, (unsigned)want,
               anchor ? anchor : "(null)");
        return 0;
    }
    return 1;
}

static int expect_true(const char *label, int condition, const char *anchor)
{
    ++g_assertions;
    if (!anchor || anchor[0] == '\0' || !condition) {
        ++g_failures;
        printf("FAIL %s condition=%d anchor=%s\n",
               label, condition, anchor ? anchor : "(null)");
        return 0;
    }
    return 1;
}

static int expect_contains(const char *label,
                           const char *haystack,
                           const char *needle,
                           const char *anchor)
{
    ++g_assertions;
    if (!anchor || anchor[0] == '\0' || !haystack || !needle ||
        !strstr(haystack, needle)) {
        ++g_failures;
        printf("FAIL %s missing=%s anchor=%s\n",
               label, needle ? needle : "(null)",
               anchor ? anchor : "(null)");
        return 0;
    }
    return 1;
}

static int expected_trace(int index)
{
    static const int trace[DM1_V1_MC_C040_RRSL_TRACE_COUNT_PC34] = {
        620, 621, 622, 623, 624, 625, 626, 627, 628, 629
    };

    return trace[index];
}

static void assert_source_metadata(
    const Dm1V1MirrorCandidateC040ResurrectRotationSaveLoadEvidencePc34 *e)
{
    const char *source =
        dm1_v1_mirror_candidate_c040_resurrect_rotation_save_load_source_evidence_pc34();

    expect_contains("source F0359", source, "F0359:1452-1662",
                    e->commandQueueAnchor);
    expect_contains("source F0380 drain", source, "F0380:2045-2178",
                    e->commandQueueAnchor);
    expect_contains("source C140 guard", source, "F0380:2366-2369",
                    e->commandQueueAnchor);
    expect_contains("source F0433", source, "F0433:1502-1707",
                    e->saveAnchor);
    expect_contains("source F0433 global", source, "F0433:1517-1538",
                    e->saveAnchor);
    expect_contains("source F0433 party", source, "F0433:1565-1584",
                    e->saveAnchor);
    expect_contains("source F0435", source, "F0435:2192-2660",
                    e->loadAnchor);
    expect_contains("source F0280", source, "F0280:124-132",
                    e->revivePublishAnchor);
    expect_contains("source F0282", source, "F0282:744-806",
                    e->reviveClearAnchor);
    expect_contains("source F0284", source, "F0284:93-131",
                    e->partyRotationAnchor);
    expect_contains("source F0297", source, "F0297:243-268",
                    e->leaderHandAnchor);
    expect_contains("source F0298", source, "F0298:270-298",
                    e->leaderHandAnchor);
    expect_contains("source F0300", source, "F0300:511-515",
                    e->slotMutationAnchor);
    expect_contains("source F0301", source, "F0301:606-614",
                    e->slotMutationAnchor);
    expect_contains("source F0302", source, "F0302:662-714",
                    e->slotMutationAnchor);
    expect_contains("source F0346", source, "F0346:1619-1637",
                    e->panelAnchor);
    expect_contains("source F0347", source, "F0347:1639-1693",
                    e->panelAnchor);
    expect_contains("defs C040", source, "C040", e->defsAnchor);
    expect_contains("defs M568", source, "M568", e->defsAnchor);
    expect_contains("defs G0299", source, "G0299", e->defsAnchor);
    expect_contains("defs G0423", source, "G0423", e->defsAnchor);
    expect_contains("defs G0424", source, "G0424", e->defsAnchor);
    expect_contains("defs G0425", source, "G0425", e->defsAnchor);
    expect_contains("defs G0426", source, "G0426", e->defsAnchor);
    expect_contains("non-overlap reopen", e->nonOverlap,
                    "not reopen-after-save-load", e->nonOverlap);
    expect_contains("non-overlap inventory click", e->nonOverlap,
                    "not inventory-click-during-rotation", e->nonOverlap);
    expect_contains("non-overlap rotation confirmation", e->nonOverlap,
                    "not rotation-during-resurrect-confirmation",
                    e->nonOverlap);
    expect_contains("non-overlap c160", e->nonOverlap,
                    "not c160-close-while-rotation-pending", e->nonOverlap);
    expect_contains("non-overlap full-chain", e->nonOverlap,
                    "not full-chain", e->nonOverlap);
}

static void assert_runtime(
    const Dm1V1MirrorCandidateC040ResurrectRotationSaveLoadResultPc34 *r,
    const Dm1V1MirrorCandidateC040ResurrectRotationSaveLoadEvidencePc34 *e)
{
    int i;

    expect_int("accepted", r->accepted, 1, e->contractMarker);
    expect_int("contract-only", r->contractOnly, 1, e->contractMarker);
    expect_int("no-game-data", r->noGameDataRequired, 1, e->contractMarker);
    expect_int("source anchors", r->sourceLockAnchorsPresent, 1,
               e->contractMarker);
    expect_int("guard null state", r->guardRejectsNullState, 1,
               e->contractMarker);
    expect_int("guard null result", r->guardRejectsNullResult, 1,
               e->contractMarker);
    expect_int("guard no candidate", r->guardRejectsNoCandidate, 1,
               e->contractMarker);
    expect_int("guard wrong panel", r->guardRejectsWrongPanel, 1,
               e->contractMarker);
    expect_int("guard no rotation", r->guardRejectsNoRotation, 1,
               e->contractMarker);
    expect_int("C140 blocked by candidate", r->c140BlockedByCandidate, 1,
               e->commandQueueAnchor);
    expect_int("direct boundary", r->directSaveLoadBoundaryUsed, 1,
               e->saveAnchor);
    expect_int("G0299 before save", r->g0299BeforeSave, 2,
               e->revivePublishAnchor);
    expect_int("G0299 save stable", r->g0299AfterSave, r->g0299BeforeSave,
               e->saveAnchor);
    expect_int("G0299 load stable", r->g0299AfterLoad, r->g0299BeforeSave,
               e->loadAnchor);
    expect_int("G0423 load stable", r->g0423AfterLoad, r->g0423BeforeSave,
               e->defsAnchor);
    expect_int("G0424 before save", r->g0424BeforeSave, 568,
               e->panelAnchor);
    expect_int("G0424 load stable", r->g0424AfterLoad, r->g0424BeforeSave,
               e->loadAnchor);
    expect_int("G0426 load stable", r->g0426AfterLoad, r->g0426BeforeSave,
               e->defsAnchor);
    expect_u32_eq("G0425 load stable", r->g0425HashAfterLoad,
                  r->g0425HashBeforeSave, e->defsAnchor);
    expect_int("F0282 not called", r->f0282ClearCount, 0,
               e->reviveClearAnchor);
    expect_int("F0284 called once", r->f0284RotationCount, 1,
               e->partyRotationAnchor);
    expect_int("F0297 idle", r->f0297PutLeaderHandCount, 0,
               e->leaderHandAnchor);
    expect_int("F0298 idle", r->f0298RemoveLeaderHandCount, 0,
               e->leaderHandAnchor);
    expect_int("F0300 idle", r->f0300RemoveSlotCount, 0,
               e->slotMutationAnchor);
    expect_int("F0301 idle", r->f0301AddSlotCount, 0,
               e->slotMutationAnchor);
    expect_int("F0302 idle", r->f0302SlotClickCount, 0,
               e->slotMutationAnchor);
    expect_int("F0433 once", r->f0433SaveCount, 1, e->saveAnchor);
    expect_int("F0435 once", r->f0435LoadCount, 1, e->loadAnchor);
    expect_int("candidate chain stable", r->candidateChainStableAcrossSaveLoad,
               1, e->saveAnchor);
    expect_int("candidate UI stable", r->candidateUiStableAcrossSaveLoad, 1,
               e->loadAnchor);
    expect_int("panel ordinal stable", r->panelOrdinalStableAcrossSaveLoad, 1,
               e->panelAnchor);
    expect_int("chest UI stable", r->chestUiStableAcrossSaveLoad, 1,
               e->defsAnchor);
    expect_int("leader hand stable", r->leaderHandStableAcrossSaveLoad, 1,
               e->saveAnchor);
    expect_int("rotation replay deterministic", r->rotationReplayDeterministic,
               1, e->partyRotationAnchor);
    expect_int("no candidate clear", r->noCandidateClearAcrossSaveLoad, 1,
               e->reviveClearAnchor);
    expect_int("no slot mutation", r->noSlotMutationAcrossSaveLoad, 1,
               e->slotMutationAnchor);
    expect_int("queue drained before save", r->commandQueueDrainedBeforeSave, 1,
               e->commandQueueAnchor);
    expect_u32_eq("candidate save hash", r->candidateHashAfterSave,
                  r->candidateHashBeforeSave, e->saveAnchor);
    expect_u32_eq("candidate load hash", r->candidateHashAfterLoad,
                  r->candidateHashBeforeSave, e->loadAnchor);
    expect_u32_eq("rotation replay hash", r->rotationReplayHash,
                  r->partyPoseHashAfterLoad, e->partyRotationAnchor);
    expect_true("deterministic hash nonzero", r->deterministicHash != 0u,
                e->contractMarker);
    for (i = 0; i < DM1_V1_MC_C040_RRSL_TRACE_COUNT_PC34; ++i) {
        char label[64];
        (void)snprintf(label, sizeof(label), "trace %d", i);
        expect_int(label, r->trace[i], expected_trace(i), e->contractMarker);
    }
}

int main(void)
{
    Dm1V1MirrorCandidateC040ResurrectRotationSaveLoadStatePc34 state;
    Dm1V1MirrorCandidateC040ResurrectRotationSaveLoadStatePc34 rerunState;
    Dm1V1MirrorCandidateC040ResurrectRotationSaveLoadResultPc34 result;
    Dm1V1MirrorCandidateC040ResurrectRotationSaveLoadResultPc34 rerun;
    const Dm1V1MirrorCandidateC040ResurrectRotationSaveLoadEvidencePc34 *e =
        dm1_v1_mirror_candidate_c040_resurrect_rotation_save_load_evidence_pc34();

    assert_source_metadata(e);
    dm1_v1_mirror_candidate_c040_resurrect_rotation_save_load_init_pc34(
        &state, UINT32_C(0xf0435040));
    expect_int("run",
               dm1_v1_mirror_candidate_c040_resurrect_rotation_save_load_run_pc34(
                   &state, &result),
               1, e->contractMarker);
    assert_runtime(&result, e);

    dm1_v1_mirror_candidate_c040_resurrect_rotation_save_load_init_pc34(
        &rerunState, UINT32_C(0xf0435040));
    expect_int("rerun",
               dm1_v1_mirror_candidate_c040_resurrect_rotation_save_load_run_pc34(
                   &rerunState, &rerun),
               1, e->contractMarker);
    expect_u32_eq("stable deterministic hash", rerun.deterministicHash,
                  result.deterministicHash, e->contractMarker);

    if (g_failures) {
        printf("FAIL test_dm1_v1_mirror_candidate_c040_resurrect_rotation_save_load_runtime_pc34_compat assertions=%d failures=%d hash=0x%08X rerun=0x%08X\n",
               g_assertions, g_failures, (unsigned)result.deterministicHash,
               (unsigned)rerun.deterministicHash);
        return 1;
    }
    printf("PASS test_dm1_v1_mirror_candidate_c040_resurrect_rotation_save_load_runtime_pc34_compat assertions=%d failures=0 hash=0x%08X rerun=0x%08X\n",
           g_assertions, (unsigned)result.deterministicHash,
           (unsigned)rerun.deterministicHash);
    return 0;
}
