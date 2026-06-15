#include "firestaff/dm1/v1/chest/c040_drop_during_rotation_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/*
 * Source-lock anchors asserted below:
 * CHEST.C F0333:30-67/F0334:113-132, CHAMPION.C F0297/F0298:243-298
 * and F0301/F0302:606-714, COMMAND.C F0359:1452-1662 and
 * F0380:1985-1990/2045-2178, REVIVE.C F0280:124-132/F0282:744-806,
 * PANEL.C F0346/F0347:1619-1657, IO.C F0077/F0078:1102-1122, and
 * DEFS.H C160..C162/C30..C37/C38/C040/M568/M569/C537..C544/G0299/G0425.
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
        DM1_V1_CHEST_C040_DROP_ROT_STEP_OPEN_CHEST_PC34,
        DM1_V1_CHEST_C040_DROP_ROT_STEP_OPEN_C040_PC34,
        DM1_V1_CHEST_C040_DROP_ROT_STEP_QUEUE_DROP_ROTATION_PC34,
        DM1_V1_CHEST_C040_DROP_ROT_STEP_DRAIN_C540_DROP_PC34,
        DM1_V1_CHEST_C040_DROP_ROT_STEP_DRAIN_ROTATION_PC34,
        DM1_V1_CHEST_C040_DROP_ROT_STEP_ASSERT_STABLE_PC34
    };

    return steps[index];
}

static int test_source_metadata(
    const DM1_V1_ChestC040DropDuringRotationSpecPc34* spec)
{
    const char* evidence =
        dm1_v1_chest_c040_drop_during_rotation_source_evidence_pc34();
    int ok = 1;

    ok &= expect_contains("source F0333", evidence, "CHEST.C F0333:30-67",
                          spec->f0333OpenAnchor);
    ok &= expect_contains("source F0334", evidence, "CHEST.C F0334:113-132",
                          spec->f0334CloseNegativeAnchor);
    ok &= expect_contains("source F0297", evidence, "CHAMPION.C F0297:243-298",
                          spec->f0297HandAnchor);
    ok &= expect_contains("source F0298", evidence, "CHAMPION.C F0298:270-298",
                          spec->f0298HandAnchor);
    ok &= expect_contains("source F0301", evidence, "CHAMPION.C F0301:606-614",
                          spec->f0301SlotAnchor);
    ok &= expect_contains("source F0302", evidence, "CHAMPION.C F0302:662-714",
                          spec->f0302DispatchAnchor);
    ok &= expect_contains("source F0359", evidence, "COMMAND.C F0359:1452-1662",
                          spec->f0359QueueAnchor);
    ok &= expect_contains("source F0380 drain", evidence,
                          "COMMAND.C F0380:2045-2178",
                          spec->f0380DrainAnchor);
    ok &= expect_contains("source F0380 C040", evidence,
                          "COMMAND.C F0380:1985-1990",
                          spec->f0380C040Anchor);
    ok &= expect_contains("source F0280", evidence, "REVIVE.C F0280:124-132",
                          spec->f0280CandidateAnchor);
    ok &= expect_contains("source F0282", evidence, "REVIVE.C F0282:744-806",
                          spec->f0282CandidateAnchor);
    ok &= expect_contains("source panel", evidence,
                          "PANEL.C F0346/F0347:1619-1657",
                          spec->f0346PanelAnchor);
    ok &= expect_contains("source F0077", evidence, "IO.C F0077:1113-1122",
                          spec->f0077Anchor);
    ok &= expect_contains("source F0078", evidence, "IO.C F0078:1102-1111",
                          spec->f0078Anchor);
    ok &= expect_contains("source defs C040", evidence, "2200 C040",
                          spec->defsAnchor);
    ok &= expect_contains("source defs G0299", evidence, "5694 G0299",
                          spec->defsAnchor);
    ok &= expect_contains("source disjoint pass771", evidence, "pass771",
                          spec->disjointness);
    ok &= expect_contains("source disjoint C545", evidence,
                          "mirror_candidate_c545_drop_while_panel_live",
                          spec->disjointness);
    ok &= expect_contains("source disjoint redraw", evidence,
                          "mirror_candidate_c040_redraw_after_chest_close",
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
    ok &= expect_u32("spec seed", spec->deterministicSeed,
                     DM1_V1_CHEST_C040_DROP_ROT_DETERMINISTIC_SEED_PC34,
                     spec->contractMarker);
    ok &= expect_int("spec rng calls", spec->expectedRngCallCount,
                     DM1_V1_CHEST_C040_DROP_ROT_EXPECTED_RNG_CALLS_PC34,
                     spec->contractMarker);
    return ok;
}

static int test_probe_flags(
    const DM1_V1_ChestC040DropDuringRotationProbePc34* p,
    const DM1_V1_ChestC040DropDuringRotationSpecPc34* spec)
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
    ok &= expect_u32("probe seed", p->deterministicSeed,
                     spec->deterministicSeed, spec->contractMarker);
    ok &= expect_u32("probe post seed", p->postResolveSeed,
                     spec->expectedPostResolveSeed, spec->contractMarker);
    ok &= expect_int("probe rng calls", p->rngCallCount,
                     spec->expectedRngCallCount, spec->contractMarker);
    ok &= expect_true("probe deterministic hash", p->deterministicHash != 0u,
                      spec->contractMarker);
    return ok;
}

static int test_sequence(
    const DM1_V1_ChestC040DropDuringRotationProbePc34* p,
    const DM1_V1_ChestC040DropDuringRotationSpecPc34* spec)
{
    int i;
    int ok = 1;

    ok &= expect_int("step count", p->stepCount, 6, spec->f0380DrainAnchor);
    for (i = 0; i < 6; ++i) {
        char label[48];
        (void)snprintf(label, sizeof(label), "step %d", i);
        ok &= expect_int(label, p->stepTrace[i], expected_step(i),
                         spec->f0380DrainAnchor);
    }
    ok &= expect_int("open owner", p->openOwnerBefore,
                     DM1_V1_CHEST_C040_DROP_ROT_OPEN_OWNER_PC34,
                     spec->f0333OpenAnchor);
    ok &= expect_true("open chest thing", p->openChestThingBefore != 0,
                      spec->f0333OpenAnchor);
    ok &= expect_int("panel after chest open", p->panelAfterChestOpen,
                     DM1_V1_CHEST_C040_DROP_ROT_PANEL_CHEST_PC34,
                     spec->f0333OpenAnchor);
    ok &= expect_int("panel after C040", p->panelAfterC040Open,
                     DM1_V1_CHEST_C040_DROP_ROT_PANEL_C040_PC34,
                     spec->f0346PanelAnchor);
    ok &= expect_int("candidate ordinal before", p->candidateOrdinalBefore,
                     DM1_V1_CHEST_C040_DROP_ROT_CANDIDATE_ORDINAL_PC34,
                     spec->f0280CandidateAnchor);
    ok &= expect_int("C040 panel before", p->c040PanelOpenBefore, 1,
                     spec->f0346PanelAnchor);
    ok &= expect_int("C040 graphic", p->c040Graphic,
                     DM1_V1_CHEST_C040_DROP_ROT_C040_GRAPHIC_PC34,
                     spec->defsAnchor);
    ok &= expect_int("C040 command", p->c040Command,
                     DM1_V1_CHEST_C040_DROP_ROT_C040_COMMAND_PC34,
                     spec->defsAnchor);
    ok &= expect_int("queued drop", p->queuedDrop, 1,
                     spec->f0359QueueAnchor);
    ok &= expect_int("queued rotation", p->queuedRotation, 1,
                     spec->f0359QueueAnchor);
    ok &= expect_int("queued command", p->queuedCommand,
                     DM1_V1_CHEST_C040_DROP_ROT_TARGET_COMMAND_PC34,
                     spec->f0359QueueAnchor);
    ok &= expect_int("queued zone", p->queuedZone,
                     DM1_V1_CHEST_C040_DROP_ROT_TARGET_ZONE_PC34,
                     spec->defsAnchor);
    ok &= expect_int("queued slot box", p->queuedSlotBox,
                     DM1_V1_CHEST_C040_DROP_ROT_TARGET_SLOT_BOX_PC34,
                     spec->defsAnchor);
    ok &= expect_int("queued pc34 slot", p->queuedPc34Slot,
                     DM1_V1_CHEST_C040_DROP_ROT_TARGET_PC34_SLOT_PC34,
                     spec->defsAnchor);
    ok &= expect_int("queue depth", p->commandQueueDepthAfterQueue, 2,
                     spec->f0359QueueAnchor);
    return ok;
}

static int test_drop_and_rotation(
    const DM1_V1_ChestC040DropDuringRotationProbePc34* p,
    const DM1_V1_ChestC040DropDuringRotationSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("leader before", p->leaderBeforeQueue,
                     DM1_V1_CHEST_C040_DROP_ROT_OLD_LEADER_PC34,
                     spec->f0380DrainAnchor);
    ok &= expect_true("old leader hand before",
                      p->oldLeaderHandTypeBefore != 0,
                      spec->f0297HandAnchor);
    ok &= expect_true("old leader weight before",
                      p->oldLeaderHandWeightBefore > 0,
                      spec->f0297HandAnchor);
    ok &= expect_true("old leader charges before",
                      p->oldLeaderHandChargesBefore > 0,
                      spec->f0297HandAnchor);
    ok &= expect_true("old leader quantity before",
                      p->oldLeaderHandQuantityBefore > 0,
                      spec->f0297HandAnchor);
    ok &= expect_true("new leader hand before",
                      p->newLeaderHandTypeBefore != 0,
                      spec->f0297HandAnchor);
    ok &= expect_int("C040 click suppressed", p->c040ClickSuppressedWhileHandFull,
                     1, spec->f0380C040Anchor);
    ok &= expect_int("drop drains first", p->dropDrainFirst, 1,
                     spec->f0380DrainAnchor);
    ok &= expect_int("queue depth after drop", p->commandQueueDepthAfterDrop, 1,
                     spec->f0380DrainAnchor);
    ok &= expect_int("panel after drop", p->panelAfterDrop,
                     DM1_V1_CHEST_C040_DROP_ROT_PANEL_C040_PC34,
                     spec->f0346PanelAnchor);
    ok &= expect_int("C040 after drop", p->c040PanelOpenAfterDrop, 1,
                     spec->f0346PanelAnchor);
    ok &= expect_int("candidate after drop", p->candidateOrdinalAfterDrop,
                     p->candidateOrdinalBefore, spec->f0280CandidateAnchor);
    ok &= expect_int("candidate live drop", p->candidateStillLiveAfterDrop, 1,
                     spec->f0280CandidateAnchor);
    ok &= expect_int("old hand after drop", p->oldLeaderHandTypeAfterDrop, 0,
                     spec->f0298HandAnchor);
    ok &= expect_int("old hand empty after drop", p->oldLeaderHandEmptyAfterDrop,
                     1, spec->f0298HandAnchor);
    ok &= expect_int("C540 type after drop", p->c540TypeAfterDrop,
                     p->oldLeaderHandTypeBefore, spec->f0302DispatchAnchor);
    ok &= expect_int("C540 weight after drop", p->c540WeightAfterDrop,
                     p->oldLeaderHandWeightBefore, spec->f0302DispatchAnchor);
    ok &= expect_int("C540 charges after drop", p->c540ChargesAfterDrop,
                     p->oldLeaderHandChargesBefore, spec->f0302DispatchAnchor);
    ok &= expect_int("C540 quantity after drop", p->c540QuantityAfterDrop,
                     p->oldLeaderHandQuantityBefore, spec->f0302DispatchAnchor);
    ok &= expect_int("leader after rotate", p->leaderAfterRotate,
                     DM1_V1_CHEST_C040_DROP_ROT_NEW_LEADER_PC34,
                     spec->f0380DrainAnchor);
    ok &= expect_int("queue depth after rotate",
                     p->commandQueueDepthAfterRotate, 0,
                     spec->f0380DrainAnchor);
    ok &= expect_int("open owner after rotate", p->openOwnerAfterRotate,
                     DM1_V1_CHEST_C040_DROP_ROT_OPEN_OWNER_PC34,
                     spec->f0333OpenAnchor);
    ok &= expect_int("panel after rotate", p->panelAfterRotate,
                     DM1_V1_CHEST_C040_DROP_ROT_PANEL_C040_PC34,
                     spec->f0346PanelAnchor);
    ok &= expect_int("panel stayed C040", p->panelStayedC040, 1,
                     spec->f0346PanelAnchor);
    ok &= expect_int("candidate after rotate", p->candidateOrdinalAfterRotate,
                     p->candidateOrdinalBefore, spec->f0280CandidateAnchor);
    ok &= expect_int("candidate live rotate", p->candidateStillLiveAfterRotate,
                     1, spec->f0280CandidateAnchor);
    ok &= expect_int("F0282 clear count", p->f0282ClearCount, 0,
                     spec->f0282CandidateAnchor);
    ok &= expect_int("old hand after rotate", p->oldLeaderHandTypeAfterRotate,
                     0, spec->f0298HandAnchor);
    ok &= expect_int("old hand empty rotate", p->oldLeaderHandEmptyAfterRotate,
                     1, spec->f0298HandAnchor);
    ok &= expect_int("new leader preserved", p->newLeaderHandPreservedAfterRotate,
                     1, spec->f0297HandAnchor);
    ok &= expect_int("C540 type after rotate", p->c540TypeAfterRotate,
                     p->c540TypeAfterDrop, spec->f0302DispatchAnchor);
    ok &= expect_int("C540 qty after rotate", p->c540QuantityAfterRotate,
                     p->c540QuantityAfterDrop, spec->f0302DispatchAnchor);
    ok &= expect_int("C540 visible after rotate", p->c540StillVisibleAfterRotate,
                     1, spec->f0302DispatchAnchor);
    return ok;
}

static int test_slot_arrays(
    const DM1_V1_ChestC040DropDuringRotationProbePc34* p,
    const DM1_V1_ChestC040DropDuringRotationSpecPc34* spec)
{
    int i;
    int ok = 1;

    ok &= expect_int("C540 empty before", p->c540EmptyBeforeDrop, 1,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("chain coherent before", p->chestSlotChainCoherentBefore,
                     1, spec->f0333OpenAnchor);
    ok &= expect_int("chain coherent drop", p->chestSlotChainCoherentAfterDrop,
                     1, spec->f0302DispatchAnchor);
    ok &= expect_int("chain coherent rotate",
                     p->chestSlotChainCoherentAfterRotate, 1,
                     spec->f0302DispatchAnchor);
    for (i = 0; i < DM1_V1_CHEST_C040_DROP_ROT_SLOT_COUNT_PC34; ++i) {
        char label[64];

        if (i == DM1_V1_CHEST_C040_DROP_ROT_TARGET_SLOT_INDEX_PC34) {
            (void)snprintf(label, sizeof(label), "slot %d before empty", i);
            ok &= expect_int(label, p->visibleTypesBefore[i], 0,
                             spec->f0333OpenAnchor);
            (void)snprintf(label, sizeof(label), "slot %d after target", i);
            ok &= expect_int(label, p->visibleTypesAfterDrop[i],
                             p->oldLeaderHandTypeBefore,
                             spec->f0302DispatchAnchor);
            continue;
        }
        (void)snprintf(label, sizeof(label), "slot %d type stable drop", i);
        ok &= expect_int(label, p->visibleTypesAfterDrop[i],
                         p->visibleTypesBefore[i], spec->f0302DispatchAnchor);
        (void)snprintf(label, sizeof(label), "slot %d charge stable drop", i);
        ok &= expect_int(label, p->visibleChargesAfterDrop[i],
                         p->visibleChargesBefore[i],
                         spec->f0302DispatchAnchor);
        (void)snprintf(label, sizeof(label), "slot %d quantity stable rotate", i);
        ok &= expect_int(label, p->visibleQuantitiesAfterRotate[i],
                         p->visibleQuantitiesBefore[i],
                         spec->f0302DispatchAnchor);
    }
    return ok;
}

static int test_stability_and_disjoint(
    const DM1_V1_ChestC040DropDuringRotationProbePc34* p,
    const DM1_V1_ChestC040DropDuringRotationSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("panel hash drop", p->panelHashAfterDrop,
                     p->panelHashBeforeDrop, spec->f0346PanelAnchor);
    ok &= expect_int("panel hash rotate", p->panelHashAfterRotate,
                     p->panelHashBeforeDrop, spec->f0346PanelAnchor);
    ok &= expect_int("panel hash stable", p->panelHashStable, 1,
                     spec->f0346PanelAnchor);
    ok &= expect_int("open thing after drop", p->openChestThingAfterDrop,
                     p->openChestThingBefore, spec->f0333OpenAnchor);
    ok &= expect_int("open thing after rotate", p->openChestThingAfterRotate,
                     p->openChestThingBefore, spec->f0333OpenAnchor);
    ok &= expect_int("chest never closed", p->chestNeverClosed, 1,
                     spec->f0334CloseNegativeAnchor);
    ok &= expect_int("close count", p->closeCount, 0,
                     spec->f0334CloseNegativeAnchor);
    ok &= expect_int("F0077 observed", p->f0077Observed, 1,
                     spec->f0077Anchor);
    ok &= expect_int("F0078 observed", p->f0078Observed, 1,
                     spec->f0078Anchor);
    ok &= expect_int("mouse depth after drop", p->mouseUpdateDepthAfterDrop, 0,
                     spec->f0078Anchor);
    ok &= expect_int("mouse balanced", p->f0077F0078Balanced, 1,
                     spec->f0078Anchor);
    ok &= expect_int("no pass771 duplicate", p->noPass771PlainDropDuringRotation,
                     1, spec->disjointness);
    ok &= expect_int("no C545 live duplicate",
                     p->noMirrorCandidateC040LiveC545Drop, 1,
                     spec->disjointness);
    ok &= expect_int("no C040 redraw close duplicate",
                     p->noMirrorCandidateC040RedrawAfterChestClose, 1,
                     spec->disjointness);
    ok &= expect_int("no candidate-live close duplicate",
                     p->noChestCloseWhileCandidateLive, 1,
                     spec->disjointness);
    ok &= expect_int("no close race duplicate", p->noChestScrollWheelCloseRace,
                     1, spec->disjointness);
    ok &= expect_int("no resurrect wheel duplicate",
                     p->noChestResurrectRotationScrollWheel, 1,
                     spec->disjointness);
    ok &= expect_int("no C545 accept duplicate",
                     p->noMirrorCandidateC545AcceptDuringRotation, 1,
                     spec->disjointness);
    ok &= expect_int("no inventory-exit redraw duplicate",
                     p->noMirrorCandidatePanelRedrawAfterInventoryExit, 1,
                     spec->disjointness);
    return ok;
}

int main(void)
{
    const DM1_V1_ChestC040DropDuringRotationSpecPc34* spec =
        dm1_v1_chest_c040_drop_during_rotation_spec_pc34();
    DM1_V1_ChestC040DropDuringRotationProbePc34 probe;
    int ok = 1;

    memset(&probe, 0, sizeof(probe));
    ok &= expect_true("spec accessor", spec != NULL,
                      "CHEST.C F0333:30-67");
    ok &= test_source_metadata(spec);
    ok &= expect_int("run accepted",
                     dm1_v1_chest_c040_drop_during_rotation_run_pc34(&probe),
                     1, spec->f0380DrainAnchor);
    ok &= test_probe_flags(&probe, spec);
    ok &= test_sequence(&probe, spec);
    ok &= test_drop_and_rotation(&probe, spec);
    ok &= test_slot_arrays(&probe, spec);
    ok &= test_stability_and_disjoint(&probe, spec);
    ok &= expect_int("null run rejected",
                     dm1_v1_chest_c040_drop_during_rotation_run_pc34(NULL),
                     0, spec->contractMarker);

    if (!ok || g_failures) {
        printf("FAIL test_dm1_v1_chest_c040_drop_during_rotation_pc34_compat assertions=%d failures=%d hash=0x%08X post_seed=0x%08X\n",
               g_assertions, g_failures,
               (unsigned)probe.deterministicHash,
               (unsigned)probe.postResolveSeed);
        return 1;
    }

    printf("PASS test_dm1_v1_chest_c040_drop_during_rotation_pc34_compat assertions=%d failures=0 hash=0x%08X post_seed=0x%08X\n",
           g_assertions,
           (unsigned)probe.deterministicHash,
           (unsigned)probe.postResolveSeed);
    return 0;
}
