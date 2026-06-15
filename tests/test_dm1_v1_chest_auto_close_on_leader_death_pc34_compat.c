#include "firestaff/dm1/v1/chest/auto_close_on_leader_death_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/*
 * Source-lock anchors asserted below:
 * CHAMPION.C F0319:1552-1607; F0318:1527-1551; PANEL.C F0355:2244-2310;
 * F0355:2268-2275; F0355:2318-2322; CHEST.C F0334:79-130; F0333:30-67;
 * CHAMPION.C F0297/F0298:243-298, F0300/F0301:511-614;
 * COMMAND.C F0380:2045-2184; DEFS.H C00..C29, C04_CHAMPION_CLOSE_INVENTORY,
 * C037..C040, C537..C544, G0299, G0331, G0333, G0423, G0424,
 * G0425, G0426, M516_CHAMPIONS[].CurrentHealth.
 */

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

static int expect_u32(const char* label,
                      uint32_t got,
                      uint32_t want,
                      const char* anchor)
{
    ++g_assertions;
    if (!anchor || anchor[0] == '\0') {
        ++g_failures;
        printf("FAIL %s missing-anchor\n", label);
        return 0;
    }
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=0x%08X want=0x%08X anchor=%s\n",
               label, (unsigned)got, (unsigned)want, anchor);
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
    static const int steps[] = {
        DM1_V1_CHEST_ACLD_STEP_OPEN_CHEST_PC34,
        DM1_V1_CHEST_ACLD_STEP_PRESSING_EYE_PC34,
        DM1_V1_CHEST_ACLD_STEP_F0319_KILL_PC34,
        DM1_V1_CHEST_ACLD_STEP_F0355_CLOSE_PC34,
        DM1_V1_CHEST_ACLD_STEP_F0334_REWIRE_PC34,
        DM1_V1_CHEST_ACLD_STEP_F0318_DROP_PC34,
        DM1_V1_CHEST_ACLD_STEP_ASSERT_STABLE_PC34
    };

    return steps[index];
}

static int test_source_metadata(
    const DM1_V1_ChestAutoCloseOnLeaderDeathSpecPc34* spec)
{
    const char* evidence =
        dm1_v1_chest_auto_close_on_leader_death_source_evidence_pc34();
    int ok = 1;

    ok &= expect_contains("source F0319", evidence, "F0319 lines 1552-1607",
                          spec->f0319KillAnchor);
    ok &= expect_contains("source F0318", evidence, "F0318 lines 1527-1551",
                          spec->f0318DropAnchor);
    ok &= expect_contains("source F0355", evidence, "F0355 lines 2244-2310",
                          spec->f0355ToggleAnchor);
    ok &= expect_contains("source F0355 short", evidence,
                          "F0355 lines 2268-2275",
                          spec->f0355DeathShortCircuitAnchor);
    ok &= expect_contains("source F0334", evidence, "F0334 lines 79-130",
                          spec->f0334CloseAnchor);
    ok &= expect_contains("source F0333", evidence, "F0333 lines 30-67",
                          spec->f0333OpenAnchor);
    ok &= expect_contains("source F0297", evidence, "F0297 lines 243-298",
                          spec->f0297HandAnchor);
    ok &= expect_contains("source F0298", evidence, "F0298 lines 270-298",
                          spec->f0298HandAnchor);
    ok &= expect_contains("source F0300", evidence, "F0300 lines 511-614",
                          spec->f0300ClearAnchor);
    ok &= expect_contains("source F0301", evidence, "F0301 lines 606-614",
                          spec->f0301WriteAnchor);
    ok &= expect_contains("source F0380", evidence, "F0380 lines 2045-2184",
                          spec->f0380QueueAnchor);
    ok &= expect_contains("defs G0426", evidence, "G0426",
                          spec->defsAnchor);
    ok &= expect_contains("defs G0423", evidence, "G0423",
                          spec->defsAnchor);
    ok &= expect_contains("defs M516", evidence, "M516_CHAMPIONS",
                          spec->defsAnchor);
    ok &= expect_contains("defs C04", evidence, "C04_CHAMPION_CLOSE_INVENTORY",
                          spec->defsAnchor);
    ok &= expect_contains("disjoint C061", evidence, "C061 drop-during-rotation",
                          spec->disjointness);
    ok &= expect_contains("disjoint C040", evidence, "C040 mirror drain",
                          spec->disjointness);
    ok &= expect_contains("disjoint resurrect", evidence,
                          "resurrect-rotation-scroll-wheel",
                          spec->disjointness);
    ok &= expect_contains("disjoint pickup pending", evidence,
                          "pickup-during-resurrect-pending",
                          spec->disjointness);
    ok &= expect_contains("disjoint candidate reopen", evidence,
                          "close-while-candidate-open-reopen",
                          spec->disjointness);
    ok &= expect_contains("disjoint teleporter", evidence,
                          "teleporter-survival-open-g0426",
                          spec->disjointness);
    ok &= expect_int("contract-only", spec->contractOnly, 1,
                     spec->contractMarker);
    ok &= expect_int("no game data", spec->noGameData, 1,
                     spec->contractMarker);
    ok &= expect_int("no graphics", spec->noGraphicsDatLoad, 1,
                     spec->contractMarker);
    ok &= expect_int("no dungeon", spec->noDungeonDatLoad, 1,
                     spec->contractMarker);
    ok &= expect_int("no pixels", spec->noRealAssetPixels, 1,
                     spec->contractMarker);
    ok &= expect_u32("seed", spec->deterministicSeed,
                     DM1_V1_CHEST_ACLD_DETERMINISTIC_SEED_PC34,
                     spec->contractMarker);
    return ok;
}

static int test_probe_flags(
    const DM1_V1_ChestAutoCloseOnLeaderDeathProbePc34* p,
    const DM1_V1_ChestAutoCloseOnLeaderDeathSpecPc34* spec)
{
    int ok = 1;

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
    ok &= expect_u32("probe seed", p->deterministicSeed, spec->deterministicSeed,
                     spec->contractMarker);
    ok &= expect_true("probe hash", p->deterministicHash != 0u,
                      spec->contractMarker);
    return ok;
}

static int test_sequence_and_disjoint(
    const DM1_V1_ChestAutoCloseOnLeaderDeathProbePc34* p,
    const DM1_V1_ChestAutoCloseOnLeaderDeathSpecPc34* spec)
{
    int i;
    int ok = 1;

    ok &= expect_int("step count", p->stepCount, 7,
                     spec->f0319KillAnchor);
    for (i = 0; i < 7; ++i) {
        char label[48];
        (void)snprintf(label, sizeof(label), "step %d", i);
        ok &= expect_int(label, p->stepTrace[i], expected_step(i),
                         spec->f0319KillAnchor);
    }
    ok &= expect_int("leader health before",
                     p->leaderHealthBefore,
                     DM1_V1_CHEST_ACLD_LEADER_HEALTH_BEFORE_PC34,
                     spec->f0319KillAnchor);
    ok &= expect_int("leader health after",
                     p->leaderHealthAfter,
                     DM1_V1_CHEST_ACLD_LEADER_HEALTH_AFTER_PC34,
                     spec->f0319KillAnchor);
    ok &= expect_int("fatal damage applied", p->fatalDamageApplied, 1,
                     spec->f0319KillAnchor);
    ok &= expect_int("leader current health cleared",
                     p->leaderCurrentHealthCleared, 1,
                     spec->f0319KillAnchor);
    ok &= expect_int("f0319 observed", p->f0319Observed, 1,
                     spec->f0319KillAnchor);
    ok &= expect_int("f0318 observed", p->f0318Observed, 1,
                     spec->f0318DropAnchor);
    ok &= expect_int("f0355 observed", p->f0355Observed, 1,
                     spec->f0355ToggleAnchor);
    ok &= expect_int("f0334 observed", p->f0334Observed, 1,
                     spec->f0334CloseAnchor);
    ok &= expect_int("f0333 not reopened", p->f0333NotReopened, 1,
                     spec->f0333OpenAnchor);
    ok &= expect_int("f0077 bracketed", p->f0077BracketedF0319, 1,
                     spec->f0319KillAnchor);
    ok &= expect_int("f0078 bracketed", p->f0078BracketedF0319, 1,
                     spec->f0319KillAnchor);
    ok &= expect_int("not pass C061", p->noPassC061DropDuringRotation, 1,
                     spec->disjointness);
    ok &= expect_int("not pass C040", p->noPassC040DropDuringRotation, 1,
                     spec->disjointness);
    ok &= expect_int("not pass resurrect",
                     p->noPassResurrectRotationScrollWheel, 1,
                     spec->disjointness);
    ok &= expect_int("not pass pickup pending",
                     p->noPassPickupDuringResurrectPending, 1,
                     spec->disjointness);
    ok &= expect_int("not pass candidate reopen",
                     p->noPassCloseWhileCandidateOpenReopen, 1,
                     spec->disjointness);
    ok &= expect_int("not pass teleporter",
                     p->noPassTeleporterSurvivalOpenG0426, 1,
                     spec->disjointness);
    return ok;
}

static int test_g0426_g0423_g0424(
    const DM1_V1_ChestAutoCloseOnLeaderDeathProbePc34* p,
    const DM1_V1_ChestAutoCloseOnLeaderDeathSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("g0426 before non-zero", p->g0426Before,
                     DM1_V1_CHEST_ACLD_CHEST_THING_PC34,
                     spec->f0334CloseAnchor);
    ok &= expect_int("g0426 after f0319 cleared",
                     p->g0426AfterF0319,
                     DM1_V1_CHEST_ACLD_NO_THING_PC34,
                     spec->f0334CloseAnchor);
    ok &= expect_int("g0426 after f0334 cleared",
                     p->g0426AfterF0334,
                     DM1_V1_CHEST_ACLD_NO_THING_PC34,
                     spec->f0334CloseAnchor);
    ok &= expect_int("g0426 final cleared",
                     p->g0426Final,
                     DM1_V1_CHEST_ACLD_NO_THING_PC34,
                     spec->f0334CloseAnchor);
    ok &= expect_int("g0426 cleared by f0334", p->g0426ClearedByF0334, 1,
                     spec->f0334CloseAnchor);
    ok &= expect_int("g0425 visible before",
                     p->g0425VisibleCountBefore,
                     DM1_V1_CHEST_ACLD_SLOT_COUNT_PC34,
                     spec->f0334CloseAnchor);
    ok &= expect_int("g0425 visible after f0334",
                     p->g0425VisibleCountAfterF0334, 0,
                     spec->f0334CloseAnchor);
    ok &= expect_int("g0425 all slots cleared by f0334",
                     p->g0425AllSlotsClearedByF0334, 1,
                     spec->f0334CloseAnchor);
    ok &= expect_int("container slot head preserved",
                     p->containerSlotHeadAfterF0334,
                     p->containerSlotHeadBefore,
                     spec->f0334CloseAnchor);

    ok &= expect_int("g0423 before ordinal",
                     p->g0423Before, 0,
                     spec->f0355ToggleAnchor);
    ok &= expect_int("g0423 after f0319 ordinal",
                     p->g0423AfterF0319, 0,
                     spec->f0355ToggleAnchor);
    ok &= expect_int("g0423 final ordinal",
                     p->g0423Final, 0,
                     spec->f0355ToggleAnchor);
    ok &= expect_int("g0423 cleared to none",
                     p->g0423ClearedToNone, 1,
                     spec->f0355ToggleAnchor);

    ok &= expect_int("g0424 before chest",
                     p->g0424Before,
                     DM1_V1_CHEST_ACLD_PANEL_CHEST_PC34,
                     spec->f0355ToggleAnchor);
    ok &= expect_int("g0424 after f0355",
                     p->g0424AfterF0355,
                     DM1_V1_CHEST_ACLD_PANEL_INVENTORY_PC34,
                     spec->f0355ToggleAnchor);
    ok &= expect_int("g0424 final",
                     p->g0424Final,
                     DM1_V1_CHEST_ACLD_PANEL_INVENTORY_PC34,
                     spec->f0355ToggleAnchor);
    ok &= expect_int("g0424 ended at inventory",
                     p->g0424EndedAtInventory, 1,
                     spec->f0355ToggleAnchor);
    return ok;
}

static int test_pressing_and_hand(
    const DM1_V1_ChestAutoCloseOnLeaderDeathProbePc34* p,
    const DM1_V1_ChestAutoCloseOnLeaderDeathSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("pressing eye before", p->pressingEyeBefore, 1,
                     spec->f0319KillAnchor);
    ok &= expect_int("pressing eye after", p->pressingEyeAfterF0319, 0,
                     spec->f0319KillAnchor);
    ok &= expect_int("pressing eye cleared by f0319",
                     p->pressingEyeClearedByF0319, 1,
                     spec->f0319KillAnchor);
    ok &= expect_int("pressing mouth before", p->pressingMouthBefore, 1,
                     spec->f0319KillAnchor);
    ok &= expect_int("pressing mouth after", p->pressingMouthAfterF0319, 0,
                     spec->f0319KillAnchor);
    ok &= expect_int("pressing mouth cleared by f0319",
                     p->pressingMouthClearedByF0319, 1,
                     spec->f0319KillAnchor);

    ok &= expect_int("leader hand item before",
                     p->leaderHandItemBefore,
                     DM1_V1_CHEST_ACLD_LEADER_HAND_ITEM_PC34,
                     spec->f0297HandAnchor);
    ok &= expect_int("leader action hand item before",
                     p->leaderActionHandItemBefore,
                     DM1_V1_CHEST_ACLD_LEADER_ACTION_ITEM_PC34,
                     spec->f0301WriteAnchor);
    ok &= expect_int("leader hand stable across f0319",
                     p->leaderHandByteStableAcrossF0319, 1,
                     spec->f0297HandAnchor);
    ok &= expect_int("leader hand after f0319 before f0318",
                     p->leaderHandItemAfterF0319BeforeF0318,
                     DM1_V1_CHEST_ACLD_LEADER_HAND_ITEM_PC34,
                     spec->f0297HandAnchor);
    ok &= expect_int("leader action after f0319 before f0318",
                     p->leaderActionHandItemAfterF0319BeforeF0318,
                     DM1_V1_CHEST_ACLD_LEADER_ACTION_ITEM_PC34,
                     spec->f0301WriteAnchor);
    ok &= expect_int("leader hand after f0318",
                     p->leaderHandItemAfterF0318,
                     DM1_V1_CHEST_ACLD_NO_THING_PC34,
                     spec->f0318DropAnchor);
    ok &= expect_int("leader action after f0318",
                     p->leaderActionHandItemAfterF0318,
                     DM1_V1_CHEST_ACLD_NO_THING_PC34,
                     spec->f0318DropAnchor);
    ok &= expect_int("leader hand cleared by f0318",
                     p->leaderHandClearedByF0318, 1,
                     spec->f0318DropAnchor);
    return ok;
}

static int test_drop_order_and_anchors(
    const DM1_V1_ChestAutoCloseOnLeaderDeathProbePc34* p,
    const DM1_V1_ChestAutoCloseOnLeaderDeathSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("f0318 ran after f0334", p->f0318RanAfterF0334, 1,
                     spec->f0318DropAnchor);
    ok &= expect_int("f0318 drop count",
                     p->f0318DropCount, 2,
                     spec->f0318DropAnchor);
    ok &= expect_int("dead leader drop iteration count",
                     p->deadLeaderDropIterationCount, 2,
                     spec->f0318DropAnchor);
    ok &= expect_int("dead leader hand object dropped",
                     p->deadLeaderHandObjectDropped, 1,
                     spec->f0318DropAnchor);
    ok &= expect_int("dead leader action object dropped",
                     p->deadLeaderActionObjectDropped, 1,
                     spec->f0318DropAnchor);
    ok &= expect_int("f0297 no mutate during death",
                     p->f0297LeaderHandPutDuringDeath, 0,
                     spec->f0297HandAnchor);
    ok &= expect_int("f0298 no mutate during death",
                     p->f0298LeaderHandRemovedDuringDeath, 0,
                     spec->f0298HandAnchor);
    ok &= expect_int("f0300 not invoked",
                     p->f0300RemoveFromC30, 0,
                     spec->f0300ClearAnchor);
    ok &= expect_int("f0301 not invoked",
                     p->f0301AddToC30, 0,
                     spec->f0301WriteAnchor);
    ok &= expect_int("f0355 leader hand empty guard satisfied",
                     p->f0355LeaderHandEmptyGuardSatisfied, 1,
                     spec->f0355DeathShortCircuitAnchor);
    ok &= expect_int("f0380 queue depth at death",
                     p->f0380QueueDepthAtDeath, 0,
                     spec->f0380QueueAnchor);
    ok &= expect_int("f0380 queue did not drain",
                     p->f0380QueueDidNotDrain, 1,
                     spec->f0380QueueAnchor);
    return ok;
}

int main(void)
{
    const DM1_V1_ChestAutoCloseOnLeaderDeathSpecPc34* spec =
        dm1_v1_chest_auto_close_on_leader_death_spec_pc34();
    DM1_V1_ChestAutoCloseOnLeaderDeathProbePc34 probe;
    int ok = 1;

    memset(&probe, 0, sizeof(probe));
    ok &= expect_true("spec accessor", spec != NULL,
                      "CHAMPION.C F0319:1552-1607");
    ok &= test_source_metadata(spec);
    ok &= expect_int("run accepted",
                     dm1_v1_chest_auto_close_on_leader_death_run_pc34(&probe),
                     1, spec->f0319KillAnchor);
    ok &= test_probe_flags(&probe, spec);
    ok &= test_sequence_and_disjoint(&probe, spec);
    ok &= test_g0426_g0423_g0424(&probe, spec);
    ok &= test_pressing_and_hand(&probe, spec);
    ok &= test_drop_order_and_anchors(&probe, spec);
    ok &= expect_int("null run rejected",
                     dm1_v1_chest_auto_close_on_leader_death_run_pc34(NULL),
                     0, spec->contractMarker);
    ok &= expect_true("assertion floor", g_assertions >= 80,
                      spec->contractMarker);

    if (!ok || g_failures) {
        printf("FAIL test_dm1_v1_chest_auto_close_on_leader_death_pc34_compat assertions=%d failures=%d hash=0x%08X\n",
               g_assertions, g_failures,
               (unsigned)probe.deterministicHash);
        return 1;
    }

    printf("PASS test_dm1_v1_chest_auto_close_on_leader_death_pc34_compat assertions=%d failures=0 hash=0x%08X\n",
           g_assertions,
           (unsigned)probe.deterministicHash);
    return 0;
}
