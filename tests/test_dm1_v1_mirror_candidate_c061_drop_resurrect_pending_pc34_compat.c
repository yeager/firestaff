#include "firestaff/dm1/v1/mirror_candidate/c061_drop_resurrect_pending_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static int expect_int(const char* label, int got, int want, const char* anchor)
{
    ++g_assertions;
    if (!anchor || anchor[0] == '\0') {
        ++g_failures;
        printf("FAIL %s missing-anchor\n", label);
        return 0;
    }
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n",
               label, got, want, anchor);
        return 0;
    }
    return 1;
}

static int expect_u32_nonzero(const char* label,
                              uint32_t got,
                              const char* anchor)
{
    ++g_assertions;
    if (!anchor || anchor[0] == '\0' || got == 0u) {
        ++g_failures;
        printf("FAIL %s got=0x%08X anchor=%s\n",
               label, (unsigned)got, anchor ? anchor : "(null)");
        return 0;
    }
    return 1;
}

static int expect_true(const char* label, int condition, const char* anchor)
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

static int expect_contains(const char* label,
                           const char* haystack,
                           const char* needle,
                           const char* anchor)
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

static int expected_step(int index)
{
    static const int steps[DM1_V1_MC_C061_RES_TRACE_COUNT_PC34] = {
        DM1_V1_MC_C061_RES_STEP_OPEN_CHEST_PC34,
        DM1_V1_MC_C061_RES_STEP_OPEN_C040_PC34,
        DM1_V1_MC_C061_RES_STEP_C028_PENDING_PC34,
        DM1_V1_MC_C061_RES_STEP_CAPTURE_C061_PC34,
        DM1_V1_MC_C061_RES_STEP_DRAIN_C061_PC34,
        DM1_V1_MC_C061_RES_STEP_ASSERT_PENDING_STABLE_PC34,
        DM1_V1_MC_C061_RES_STEP_DRY_RUN_CLOSE_PC34,
        DM1_V1_MC_C061_RES_STEP_ASSERT_STABLE_PC34
    };

    return steps[index];
}

static int test_source_metadata(
    const DM1_V1_MirrorCandidateC061DropResurrectPendingSpecPc34* spec)
{
    const char* evidence =
        dm1_v1_mirror_candidate_c061_drop_resurrect_pending_source_evidence_pc34();
    int ok = 1;

    ok &= expect_contains("source F0333", evidence, "CHEST.C F0333:30-67",
                          spec->f0333OpenAnchor);
    ok &= expect_contains("source F0334", evidence, "CHEST.C F0334:117-132",
                          spec->f0334CloseAnchor);
    ok &= expect_contains("source F0297", evidence, "CHAMPION.C F0297:243-298",
                          spec->f0297HandAnchor);
    ok &= expect_contains("source F0298", evidence, "CHAMPION.C F0298:270-298",
                          spec->f0298HandAnchor);
    ok &= expect_contains("source F0300", evidence, "CHAMPION.C F0300:511-614",
                          spec->f0300ClearAnchor);
    ok &= expect_contains("source F0301", evidence, "CHAMPION.C F0301:606-614",
                          spec->f0301WriteAnchor);
    ok &= expect_contains("source F0302", evidence, "CHAMPION.C F0302:677-712",
                          spec->f0302DispatchAnchor);
    ok &= expect_contains("source F0280", evidence, "REVIVE.C F0280:63-132",
                          spec->f0280CandidateAnchor);
    ok &= expect_contains("source F0282", evidence, "REVIVE.C F0282:744-806",
                          spec->f0282PanelAnchor);
    ok &= expect_contains("source F0344", evidence, "PANEL.C F0344:1493-1561",
                          spec->f0344FoodWaterAnchor);
    ok &= expect_contains("source F0345", evidence, "F0345:1563-1617",
                          spec->f0345FoodWaterAnchor);
    ok &= expect_contains("source F0346", evidence, "PANEL.C F0346:1619-1637",
                          spec->f0346ResurrectAnchor);
    ok &= expect_contains("source F0359", evidence, "COMMAND.C F0359:1452-1662",
                          spec->f0359ClickAnchor);
    ok &= expect_contains("source F0378", evidence, "COMMAND.C F0378:1985-1990",
                          spec->f0378PanelRouteAnchor);
    ok &= expect_contains("source F0380", evidence, "COMMAND.C F0380:2045-2184",
                          spec->f0380QueueAnchor);
    ok &= expect_contains("defs C028", evidence, "C028", spec->defsAnchor);
    ok &= expect_contains("defs C040", evidence, "C040", spec->defsAnchor);
    ok &= expect_contains("defs C061", evidence, "C061", spec->defsAnchor);
    ok &= expect_contains("defs C540", evidence, "C540", spec->defsAnchor);
    ok &= expect_contains("defs G0299", evidence, "G0299", spec->defsAnchor);
    ok &= expect_contains("defs G0425", evidence, "G0425", spec->defsAnchor);
    ok &= expect_contains("defs G0426", evidence, "G0426", spec->defsAnchor);
    ok &= expect_contains("disjoint leader rotation", evidence,
                          "c061_drop_while_leader_rotation",
                          spec->disjointness);
    ok &= expect_contains("disjoint candidate live", evidence,
                          "c061_drop_while_candidate_live",
                          spec->disjointness);
    ok &= expect_contains("disjoint C160", evidence, "C160 close",
                          spec->disjointness);
    ok &= expect_int("spec contract-only", spec->contractOnly, 1,
                     spec->contractMarker);
    ok &= expect_int("spec no game data", spec->noGameData, 1,
                     spec->contractMarker);
    ok &= expect_int("spec no graphics", spec->noGraphicsDatLoad, 1,
                     spec->contractMarker);
    ok &= expect_int("spec no dungeon", spec->noDungeonDatLoad, 1,
                     spec->contractMarker);
    ok &= expect_int("spec no pixels", spec->noRealAssetPixels, 1,
                     spec->contractMarker);
    ok &= expect_int("spec seed", (int)spec->deterministicSeed,
                     (int)DM1_V1_MC_C061_RES_SEED_PC34,
                     spec->contractMarker);
    return ok;
}

static int test_flags_and_sequence(
    const DM1_V1_MirrorCandidateC061DropResurrectPendingProbePc34* p,
    const DM1_V1_MirrorCandidateC061DropResurrectPendingSpecPc34* spec)
{
    int ok = 1;
    int i;

    ok &= expect_int("probe contract-only", p->contractOnly, 1,
                     spec->contractMarker);
    ok &= expect_int("probe no game data", p->noGameData, 1,
                     spec->contractMarker);
    ok &= expect_int("probe no graphics", p->noGraphicsDatLoad, 1,
                     spec->contractMarker);
    ok &= expect_int("probe no dungeon", p->noDungeonDatLoad, 1,
                     spec->contractMarker);
    ok &= expect_int("probe no pixels", p->noRealAssetPixels, 1,
                     spec->contractMarker);
    ok &= expect_int("probe runtime regression", p->runtimeRegression, 1,
                     spec->contractMarker);
    ok &= expect_int("probe seed", (int)p->deterministicSeed,
                     (int)spec->deterministicSeed, spec->contractMarker);
    ok &= expect_u32_nonzero("probe deterministic hash",
                             p->deterministicHash, spec->contractMarker);
    ok &= expect_int("step count", p->stepCount,
                     DM1_V1_MC_C061_RES_TRACE_COUNT_PC34,
                     spec->f0380QueueAnchor);
    for (i = 0; i < DM1_V1_MC_C061_RES_TRACE_COUNT_PC34; ++i) {
        char label[48];

        (void)snprintf(label, sizeof(label), "step %d", i);
        ok &= expect_int(label, p->stepTrace[i], expected_step(i),
                         spec->f0380QueueAnchor);
    }
    return ok;
}

static int test_candidate_and_resurrect(
    const DM1_V1_MirrorCandidateC061DropResurrectPendingProbePc34* p,
    const DM1_V1_MirrorCandidateC061DropResurrectPendingSpecPc34* spec)
{
    int ok = 1;
    int i;

    ok &= expect_int("party count", p->partyChampionCount,
                     DM1_V1_MC_C061_RES_PARTY_COUNT_PC34,
                     spec->defsAnchor);
    ok &= expect_int("leader", p->leaderIndex,
                     DM1_V1_MC_C061_RES_LEADER_PC34,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("candidate owner", p->candidateOwnerIndex,
                     DM1_V1_MC_C061_RES_CANDIDATE_OWNER_PC34,
                     spec->f0280CandidateAnchor);
    ok &= expect_int("candidate not leader", p->candidateOwnerIsLeader, 0,
                     spec->f0280CandidateAnchor);
    for (i = 0; i < DM1_V1_MC_C061_RES_PARTY_COUNT_PC34; ++i) {
        char label[64];

        (void)snprintf(label, sizeof(label), "champion %d alive", i);
        ok &= expect_true(label, p->championCurrentHealth[i] > 0,
                          spec->defsAnchor);
    }
    for (i = 0; i < DM1_V1_MC_C061_RES_CHAIN_COUNT_PC34; ++i) {
        char label[72];

        (void)snprintf(label, sizeof(label), "candidate chain %d stable", i);
        ok &= expect_int(label, p->candidateChainAfter[i],
                         p->candidateChainBefore[i],
                         spec->f0280CandidateAnchor);
    }
    ok &= expect_int("candidate chain stable", p->candidateChainStable, 1,
                     spec->f0280CandidateAnchor);
    ok &= expect_int("G0299 stable", p->g0299CandidateAfterDrain,
                     p->g0299CandidateBefore, spec->defsAnchor);
    ok &= expect_int("C040 graphic before", p->candidateGraphicBefore,
                     DM1_V1_MC_C061_RES_C040_GRAPHIC_PC34,
                     spec->defsAnchor);
    ok &= expect_int("C040 graphic stable", p->candidateGraphicAfterDrain,
                     p->candidateGraphicBefore, spec->f0346ResurrectAnchor);
    ok &= expect_int("M568 command before", p->candidateCommandBefore,
                     DM1_V1_MC_C061_RES_M568_PANEL_PC34,
                     spec->defsAnchor);
    ok &= expect_int("M568 command stable", p->candidateCommandAfterDrain,
                     p->candidateCommandBefore,
                     spec->f0378PanelRouteAnchor);
    ok &= expect_int("C040 panel before", p->c040PanelBeforeDrain,
                     DM1_V1_MC_C061_RES_M568_PANEL_PC34,
                     spec->f0346ResurrectAnchor);
    ok &= expect_int("C040 panel after", p->c040PanelAfterDrain,
                     p->c040PanelBeforeDrain,
                     spec->f0346ResurrectAnchor);
    ok &= expect_int("C040 stayed live", p->c040PanelStayedLive, 1,
                     spec->f0346ResurrectAnchor);
    ok &= expect_int("C028 pending before", p->c028ResurrectPendingBefore, 1,
                     spec->f0282PanelAnchor);
    ok &= expect_int("C028 command", p->c028Command,
                     DM1_V1_MC_C061_RES_C028_COMMAND_PC34,
                     spec->defsAnchor);
    ok &= expect_int("F0280 candidate add", p->f0280CandidateAddCount, 1,
                     spec->f0280CandidateAnchor);
    ok &= expect_int("C028 pending after", p->c028ResurrectPendingAfterDrain,
                     p->c028ResurrectPendingBefore,
                     spec->f0282PanelAnchor);
    ok &= expect_int("resurrect pending stable",
                     p->resurrectConfirmationStayedPending, 1,
                     spec->f0282PanelAnchor);
    ok &= expect_int("F0282 clear count", p->f0282CandidateClearCount, 0,
                     spec->f0282PanelAnchor);
    ok &= expect_int("F0282 commit count", p->f0282ResurrectCommitCount, 0,
                     spec->f0282PanelAnchor);
    ok &= expect_int("F0282 cancel count", p->f0282CancelCount, 0,
                     spec->f0282PanelAnchor);
    return ok;
}

static int test_chest_drop(
    const DM1_V1_MirrorCandidateC061DropResurrectPendingProbePc34* p,
    const DM1_V1_MirrorCandidateC061DropResurrectPendingSpecPc34* spec)
{
    int ok = 1;
    int i;

    ok &= expect_int("open owner", p->openChestOwnerIndex,
                     DM1_V1_MC_C061_RES_LEADER_PC34,
                     spec->f0333OpenAnchor);
    ok &= expect_int("open chest before", p->openChestThingBefore,
                     DM1_V1_MC_C061_RES_OPEN_CHEST_THING_PC34,
                     spec->f0333OpenAnchor);
    ok &= expect_int("open chest after", p->openChestThingAfterDrain,
                     p->openChestThingBefore, spec->f0333OpenAnchor);
    ok &= expect_int("G0426 stayed open", p->g0426StayedOpenDuringDrain, 1,
                     spec->f0333OpenAnchor);
    ok &= expect_int("F0333 open count", p->f0333OpenCount, 1,
                     spec->f0333OpenAnchor);
    ok &= expect_int("F0334 during drain", p->f0334CloseCountDuringDrain, 0,
                     spec->f0334CloseAnchor);
    ok &= expect_int("F0334 dry run", p->f0334DryRunCloseCount, 1,
                     spec->f0334CloseAnchor);
    ok &= expect_int("close tail count", p->closeTailCountAfterDryRun,
                     DM1_V1_MC_C061_RES_SLOT_COUNT_PC34,
                     spec->f0334CloseAnchor);

    ok &= expect_int("C061 captured", p->c061Captured, 1,
                     spec->f0359ClickAnchor);
    ok &= expect_int("C061 drained", p->c061Drained, 1,
                     spec->f0380QueueAnchor);
    ok &= expect_int("C061 command", p->c061Command,
                     DM1_V1_MC_C061_RES_TARGET_COMMAND_PC34,
                     spec->defsAnchor);
    ok &= expect_int("C061 zone", p->c061Zone,
                     DM1_V1_MC_C061_RES_TARGET_ZONE_PC34,
                     spec->defsAnchor);
    ok &= expect_int("C061 slot box", p->c061SlotBox, 41,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("C061 pc34 slot", p->c061Pc34Slot, 33,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("queue depth capture", p->commandQueueDepthAfterCapture,
                     2, spec->f0380QueueAnchor);
    ok &= expect_int("queue depth drain", p->commandQueueDepthAfterDrain, 1,
                     spec->f0380QueueAnchor);
    ok &= expect_int("F0359 queued drop", p->f0359CapturedQueuedDrop, 1,
                     spec->f0359ClickAnchor);
    ok &= expect_int("F0378 panel route", p->f0378PanelRouteCount, 0,
                     spec->f0378PanelRouteAnchor);
    ok &= expect_int("F0380 drain", p->f0380DrainCount, 1,
                     spec->f0380QueueAnchor);
    ok &= expect_int("F0302 dispatch", p->f0302DropDispatchCount, 1,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("C061 no F0282 route", p->c061DidNotRouteToF0282, 1,
                     spec->f0282PanelAnchor);
    ok &= expect_int("C061 no candidate clear", p->c061DidNotClearCandidate,
                     1, spec->defsAnchor);

    ok &= expect_int("leader hand before", p->leaderHandTypeBefore,
                     DM1_V1_MC_C061_RES_LEADER_HAND_THING_PC34,
                     spec->f0298HandAnchor);
    ok &= expect_int("leader hand weight", p->leaderHandWeightBefore, 17,
                     spec->f0298HandAnchor);
    ok &= expect_int("leader hand charges", p->leaderHandChargesBefore, 61,
                     spec->f0298HandAnchor);
    ok &= expect_int("leader hand after", p->leaderHandTypeAfterDrain, 0,
                     spec->f0298HandAnchor);
    ok &= expect_int("leader hand cleared", p->leaderHandClearedByDrop, 1,
                     spec->f0298HandAnchor);
    ok &= expect_int("leader load delta", p->leaderLoadDelta,
                     -p->leaderHandWeightBefore, spec->f0298HandAnchor);
    ok &= expect_int("F0298 removed hand", p->f0298RemovedLeaderHand, 1,
                     spec->f0298HandAnchor);
    ok &= expect_int("F0300 clear empty target", p->f0300ClearCount, 0,
                     spec->f0300ClearAnchor);
    ok &= expect_int("F0301 wrote C540", p->f0301WroteC540, 1,
                     spec->f0301WriteAnchor);
    ok &= expect_int("F0297 no pickup", p->f0297PutSlotInLeaderHandCount, 0,
                     spec->f0297HandAnchor);
    ok &= expect_int("target empty before", p->targetSlotEmptyBefore, 1,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("target receives leader hand",
                     p->targetSlotReceivesLeaderHand, 1,
                     spec->f0301WriteAnchor);
    ok &= expect_true("G0425 hash before", p->g0425HashBefore != 0u,
                      spec->f0333OpenAnchor);
    ok &= expect_true("G0425 hash after", p->g0425HashAfterDrain != 0u,
                      spec->f0301WriteAnchor);
    ok &= expect_true("G0425 hash changed",
                      p->g0425HashAfterDrain != p->g0425HashBefore,
                      spec->f0301WriteAnchor);
    ok &= expect_int("G0425 target-only mutation",
                     p->g0425HashMutatedOnlyByTargetDrop, 1,
                     spec->f0301WriteAnchor);

    for (i = 0; i < DM1_V1_MC_C061_RES_SLOT_COUNT_PC34; ++i) {
        char label[96];

        (void)snprintf(label, sizeof(label), "slot %d stability", i);
        ok &= expect_int(label, p->g0425SlotStableExceptTarget[i], 1,
                         spec->f0302DispatchAnchor);
        if (i != DM1_V1_MC_C061_RES_TARGET_SLOT_INDEX_PC34) {
            (void)snprintf(label, sizeof(label), "slot %d type stable", i);
            ok &= expect_int(label, p->g0425TypesAfterDrain[i],
                             p->g0425TypesBefore[i],
                             spec->f0333OpenAnchor);
            (void)snprintf(label, sizeof(label), "slot %d weight stable", i);
            ok &= expect_int(label, p->g0425WeightsAfterDrain[i],
                             p->g0425WeightsBefore[i],
                             spec->f0333OpenAnchor);
            (void)snprintf(label, sizeof(label), "slot %d charges stable", i);
            ok &= expect_int(label, p->g0425ChargesAfterDrain[i],
                             p->g0425ChargesBefore[i],
                             spec->f0333OpenAnchor);
        }
    }
    ok &= expect_int("target slot type", p->g0425TypesAfterDrain[3],
                     DM1_V1_MC_C061_RES_LEADER_HAND_THING_PC34,
                     spec->f0301WriteAnchor);
    ok &= expect_int("target slot weight", p->g0425WeightsAfterDrain[3],
                     p->leaderHandWeightBefore,
                     spec->f0301WriteAnchor);
    ok &= expect_int("target slot charges", p->g0425ChargesAfterDrain[3],
                     p->leaderHandChargesBefore,
                     spec->f0301WriteAnchor);
    return ok;
}

static int test_no_side_effects(
    const DM1_V1_MirrorCandidateC061DropResurrectPendingProbePc34* p,
    const DM1_V1_MirrorCandidateC061DropResurrectPendingSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("F0344 food/water draw", p->f0344FoodWaterDrawCount, 0,
                     spec->f0344FoodWaterAnchor);
    ok &= expect_int("F0345 food/water panel", p->f0345FoodWaterPanelCount, 0,
                     spec->f0345FoodWaterAnchor);
    ok &= expect_int("F0346 C040 draw count", p->f0346C040DrawCount, 1,
                     spec->f0346ResurrectAnchor);
    ok &= expect_int("F0355 toggle", p->f0355InventoryToggleCount, 0,
                     spec->f0282PanelAnchor);
    ok &= expect_int("F0368 set leader", p->f0368SetLeaderCount, 0,
                     spec->f0282PanelAnchor);
    ok &= expect_int("save/load count", p->saveLoadCount, 0,
                     spec->disjointness);
    ok &= expect_int("teleporter count", p->teleporterCount, 0,
                     spec->disjointness);
    ok &= expect_int("party rotate count", p->partyRotateCount, 0,
                     spec->disjointness);
    ok &= expect_int("no C160 close", p->noC160Close, 1,
                     spec->disjointness);
    ok &= expect_int("no C045 accept", p->noC045Accept, 1,
                     spec->disjointness);
    ok &= expect_int("no leader rotation", p->noLeaderRotation, 1,
                     spec->disjointness);
    ok &= expect_int("disjoint C061 leader rotation",
                     p->disjointFromC061LeaderRotation, 1,
                     spec->disjointness);
    ok &= expect_int("disjoint C061 candidate live",
                     p->disjointFromC061CandidateLive, 1,
                     spec->disjointness);
    ok &= expect_int("disjoint C160 close rotation",
                     p->disjointFromC160CloseRotation, 1,
                     spec->disjointness);
    return ok;
}

static int test_guards(void)
{
    int ok = 1;

    ok &= expect_int("null probe rejected",
                     dm1_v1_mirror_candidate_c061_drop_resurrect_pending_run_pc34(
                         NULL),
                     0, "guard");
    return ok;
}

int main(void)
{
    const DM1_V1_MirrorCandidateC061DropResurrectPendingSpecPc34* spec;
    DM1_V1_MirrorCandidateC061DropResurrectPendingProbePc34 first;
    DM1_V1_MirrorCandidateC061DropResurrectPendingProbePc34 second;
    int ok = 1;

    spec = dm1_v1_mirror_candidate_c061_drop_resurrect_pending_spec_pc34();
    ok &= test_source_metadata(spec);
    ok &= expect_int("first run accepted",
                     dm1_v1_mirror_candidate_c061_drop_resurrect_pending_run_pc34(
                         &first),
                     1, spec->contractMarker);
    ok &= expect_int("second run accepted",
                     dm1_v1_mirror_candidate_c061_drop_resurrect_pending_run_pc34(
                         &second),
                     1, spec->contractMarker);
    ok &= test_flags_and_sequence(&first, spec);
    ok &= test_candidate_and_resurrect(&first, spec);
    ok &= test_chest_drop(&first, spec);
    ok &= test_no_side_effects(&first, spec);
    ok &= expect_int("deterministic hash stable",
                     (int)second.deterministicHash,
                     (int)first.deterministicHash,
                     spec->contractMarker);
    ok &= test_guards();

    if (!ok || g_failures || g_assertions < 150) {
        printf("FAIL test_dm1_v1_mirror_candidate_c061_drop_resurrect_pending_pc34_compat assertions=%d failures=%d hash=0x%08X rerun=0x%08X\n",
               g_assertions, g_failures, (unsigned)first.deterministicHash,
               (unsigned)second.deterministicHash);
        return 1;
    }
    printf("PASS test_dm1_v1_mirror_candidate_c061_drop_resurrect_pending_pc34_compat assertions=%d failures=0 hash=0x%08X rerun=0x%08X\n",
           g_assertions, (unsigned)first.deterministicHash,
           (unsigned)second.deterministicHash);
    return 0;
}
