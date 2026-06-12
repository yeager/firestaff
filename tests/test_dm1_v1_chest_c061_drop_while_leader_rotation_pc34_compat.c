#include "firestaff/dm1/v1/chest/c061_drop_while_leader_rotation_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/*
 * Source-lock anchors asserted below:
 * CHEST.C F0333:30-67/F0334:117-132; CHAMPION.C F0297/F0298:243-298,
 * F0300/F0301:511-614, F0302:677-712; COMMAND.C F0359:1985-1990 and
 * F0380:2045-2184; OBJECT.C F0032/F0033:121-176; DUNGEON.C F0163:1769-1795;
 * IO.C F0077/F0078:1102-1122; DEFS.H C30/C061/C540/C037/C038/C039,
 * C537..C544, G0425/G0426, M070, and M516_CHAMPIONS[].Load.
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
        DM1_V1_CHEST_C061_DROP_ROT_STEP_OPEN_CHEST_PC34,
        DM1_V1_CHEST_C061_DROP_ROT_STEP_QUEUE_ROTATION_PC34,
        DM1_V1_CHEST_C061_DROP_ROT_STEP_CAPTURE_C061_PC34,
        DM1_V1_CHEST_C061_DROP_ROT_STEP_DRAIN_ROTATION_PC34,
        DM1_V1_CHEST_C061_DROP_ROT_STEP_ASSERT_STABLE_PC34
    };

    return steps[index];
}

static int test_source_metadata(
    const DM1_V1_ChestC061DropWhileLeaderRotationSpecPc34* spec)
{
    const char* evidence =
        dm1_v1_chest_c061_drop_while_leader_rotation_source_evidence_pc34();
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
    ok &= expect_contains("source F0359", evidence, "COMMAND.C F0359:1985-1990",
                          spec->f0359PanelAnchor);
    ok &= expect_contains("source F0380", evidence, "COMMAND.C F0380:2045-2184",
                          spec->f0380QueueAnchor);
    ok &= expect_contains("source F0032", evidence, "OBJECT.C F0032:121-145",
                          spec->f0032ObjectAnchor);
    ok &= expect_contains("source F0033", evidence, "OBJECT.C F0033:147-176",
                          spec->f0033ObjectAnchor);
    ok &= expect_contains("source F0163", evidence, "DUNGEON.C F0163:1769-1795",
                          spec->f0163AppendAnchor);
    ok &= expect_contains("source F0077", evidence, "IO.C F0077:1113-1122",
                          spec->f0077Anchor);
    ok &= expect_contains("source F0078", evidence, "IO.C F0078:1102-1111",
                          spec->f0078Anchor);
    ok &= expect_contains("defs C061", evidence, "C061",
                          spec->defsAnchor);
    ok &= expect_contains("defs G0425", evidence, "G0425",
                          spec->defsAnchor);
    ok &= expect_contains("defs M516", evidence, "M516_CHAMPIONS",
                          spec->defsAnchor);
    ok &= expect_contains("disjoint pass786", evidence, "pass786",
                          spec->disjointness);
    ok &= expect_contains("disjoint pass771", evidence, "pass771",
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
                     DM1_V1_CHEST_C061_DROP_ROT_DETERMINISTIC_SEED_PC34,
                     spec->contractMarker);
    return ok;
}

static int test_probe_flags(
    const DM1_V1_ChestC061DropWhileLeaderRotationProbePc34* p,
    const DM1_V1_ChestC061DropWhileLeaderRotationSpecPc34* spec)
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
    ok &= expect_true("probe hash", p->deterministicHash != 0u,
                      spec->contractMarker);
    return ok;
}

static int test_sequence_and_queue(
    const DM1_V1_ChestC061DropWhileLeaderRotationProbePc34* p,
    const DM1_V1_ChestC061DropWhileLeaderRotationSpecPc34* spec)
{
    int i;
    int ok = 1;

    ok &= expect_int("step count", p->stepCount, 5, spec->f0380QueueAnchor);
    for (i = 0; i < 5; ++i) {
        char label[48];

        (void)snprintf(label, sizeof(label), "step %d", i);
        ok &= expect_int(label, p->stepTrace[i], expected_step(i),
                         spec->f0380QueueAnchor);
    }
    ok &= expect_int("leader before", p->leaderBeforeQueue,
                     DM1_V1_CHEST_C061_DROP_ROT_OLD_LEADER_PC34,
                     spec->f0380QueueAnchor);
    ok &= expect_int("leader after rotation", p->leaderAfterRotation,
                     DM1_V1_CHEST_C061_DROP_ROT_NEW_LEADER_PC34,
                     spec->f0380QueueAnchor);
    ok &= expect_int("open owner before", p->openOwnerBefore,
                     DM1_V1_CHEST_C061_DROP_ROT_OPEN_OWNER_PC34,
                     spec->f0333OpenAnchor);
    ok &= expect_int("open owner after", p->openOwnerAfterRotation,
                     p->openOwnerBefore, spec->f0333OpenAnchor);
    ok &= expect_true("open chest thing before",
                      p->openChestThingBefore != 0,
                      spec->f0333OpenAnchor);
    ok &= expect_int("open chest thing stable",
                     p->openChestThingAfterRotation,
                     p->openChestThingBefore, spec->f0333OpenAnchor);
    ok &= expect_int("rotation queued", p->rotationQueued, 1,
                     spec->f0380QueueAnchor);
    ok &= expect_int("C061 captured", p->c061CapturedWhileRotationQueued, 1,
                     spec->f0380QueueAnchor);
    ok &= expect_int("queue depth capture", p->commandQueueDepthAfterCapture,
                     2, spec->f0380QueueAnchor);
    ok &= expect_int("queue depth after rotation",
                     p->commandQueueDepthAfterRotation, 1,
                     spec->f0380QueueAnchor);
    ok &= expect_int("C061 pending after rotation",
                     p->pendingC061AfterRotation, 1,
                     spec->f0380QueueAnchor);
    ok &= expect_int("C061 not applied during rotation",
                     p->c061AppliedDuringRotation, 0,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("C061 not in leader action hand",
                     p->c061EndsInLeaderActionHand, 0,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("C061 remains queued", p->c061EndsInPendingQueue, 1,
                     spec->f0380QueueAnchor);
    ok &= expect_int("C061 command", p->c061Command,
                     DM1_V1_CHEST_C061_DROP_ROT_TARGET_COMMAND_PC34,
                     spec->defsAnchor);
    ok &= expect_int("C061 zone", p->c061Zone,
                     DM1_V1_CHEST_C061_DROP_ROT_TARGET_ZONE_PC34,
                     spec->defsAnchor);
    ok &= expect_int("C061 slot box", p->c061SlotBox,
                     DM1_V1_CHEST_C061_DROP_ROT_TARGET_SLOT_BOX_PC34,
                     spec->defsAnchor);
    ok &= expect_int("C061 pc34 slot", p->c061Pc34Slot,
                     DM1_V1_CHEST_C061_DROP_ROT_TARGET_PC34_SLOT_PC34,
                     spec->defsAnchor);
    ok &= expect_int("mouse route accepted", p->c061MouseRouteAccepted, 1,
                     spec->f0077Anchor);
    return ok;
}

static int test_panel_g0426_and_hand(
    const DM1_V1_ChestC061DropWhileLeaderRotationProbePc34* p,
    const DM1_V1_ChestC061DropWhileLeaderRotationSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("panel before", p->panelBeforeRace,
                     DM1_V1_CHEST_C061_DROP_ROT_PANEL_CHEST_PC34,
                     spec->f0333OpenAnchor);
    ok &= expect_int("panel after capture", p->panelAfterCapture,
                     p->panelBeforeRace, spec->f0359PanelAnchor);
    ok &= expect_int("panel after rotation", p->panelAfterRotation,
                     p->panelBeforeRace, spec->f0359PanelAnchor);
    ok &= expect_int("panel repaint champ", p->panelRepaintChampionDuringRace,
                     DM1_V1_CHEST_C061_DROP_ROT_OPEN_OWNER_PC34,
                     spec->f0333OpenAnchor);
    ok &= expect_int("old leader not repainted",
                     p->leaderPanelRepaintedDuringRace, 0,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("new leader not repainted",
                     p->newLeaderPanelRepaintedDuringRace, 0,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("open owner repainted",
                     p->openOwnerPanelRepaintedDuringRace, 1,
                     spec->f0333OpenAnchor);
    ok &= expect_int("G0426 hash stable", p->g0426ByteHashAfterRotation,
                     p->g0426ByteHashBefore, spec->f0333OpenAnchor);
    ok &= expect_int("G0426 stable flag", p->g0426ByteStableAcrossRotation,
                     1, spec->f0333OpenAnchor);
    ok &= expect_int("close count", p->closeCountDuringRotation, 0,
                     spec->f0334CloseAnchor);
    ok &= expect_int("F0334 suppressed", p->f0334CloseSuppressed, 1,
                     spec->f0334CloseAnchor);
    ok &= expect_true("leader hand before",
                      p->leaderHandTypeBefore ==
                          DM1_V1_CHEST_C061_DROP_ROT_LEADER_HAND_ITEM_PC34,
                      spec->f0297HandAnchor);
    ok &= expect_true("leader hand weight before",
                      p->leaderHandWeightBefore > 0,
                      spec->f0297HandAnchor);
    ok &= expect_true("leader hand charges before",
                      p->leaderHandChargesBefore > 0,
                      spec->f0297HandAnchor);
    ok &= expect_int("leader hand type stable",
                     p->leaderHandTypeAfterRotation,
                     p->leaderHandTypeBefore, spec->f0297HandAnchor);
    ok &= expect_int("leader hand byte stable",
                     p->leaderHandByteStableAcrossRotation, 1,
                     spec->f0297HandAnchor);
    ok &= expect_int("action hand pc34", p->leaderActionHandSlotPc34,
                     DM1_V1_CHEST_C061_DROP_ROT_ACTION_HAND_PC34,
                     spec->defsAnchor);
    ok &= expect_int("action type before", p->leaderActionHandTypeBefore,
                     DM1_V1_CHEST_C061_DROP_ROT_LEADER_ACTION_ITEM_PC34,
                     spec->f0300ClearAnchor);
    ok &= expect_int("action type after", p->leaderActionHandTypeAfterRotation,
                     p->leaderActionHandTypeBefore, spec->f0300ClearAnchor);
    ok &= expect_int("action weight after",
                     p->leaderActionHandWeightAfterRotation,
                     p->leaderActionHandWeightBefore, spec->f0300ClearAnchor);
    ok &= expect_int("action charges after",
                     p->leaderActionHandChargesAfterRotation,
                     p->leaderActionHandChargesBefore, spec->f0300ClearAnchor);
    ok &= expect_int("action byte stable",
                     p->leaderActionHandByteStableAcrossRotation, 1,
                     spec->f0300ClearAnchor);
    return ok;
}

static int test_g0425_slots_and_loads(
    const DM1_V1_ChestC061DropWhileLeaderRotationProbePc34* p,
    const DM1_V1_ChestC061DropWhileLeaderRotationSpecPc34* spec)
{
    int i;
    int ok = 1;

    ok &= expect_int("G0425 hash stable", p->g0425ByteHashAfterRotation,
                     p->g0425ByteHashBefore, spec->f0301WriteAnchor);
    ok &= expect_int("G0425 stable flag", p->g0425ByteStableAcrossRotation,
                     1, spec->f0301WriteAnchor);
    ok &= expect_int("target empty before", p->targetSlotEmptyBefore, 1,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("target empty after", p->targetSlotEmptyAfterRotation, 1,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("target not mutated", p->targetSlotNotMutatedByPendingC061,
                     1, spec->f0302DispatchAnchor);
    for (i = 0; i < DM1_V1_CHEST_C061_DROP_ROT_SLOT_COUNT_PC34; ++i) {
        char label[72];

        (void)snprintf(label, sizeof(label), "G0425 slot %d type stable", i);
        ok &= expect_int(label, p->g0425TypesAfterRotation[i],
                         p->g0425TypesBefore[i], spec->f0301WriteAnchor);
        (void)snprintf(label, sizeof(label), "G0425 slot %d weight stable", i);
        ok &= expect_int(label, p->g0425WeightsAfterRotation[i],
                         p->g0425WeightsBefore[i], spec->f0301WriteAnchor);
        (void)snprintf(label, sizeof(label), "G0425 slot %d charge stable", i);
        ok &= expect_int(label, p->g0425ChargesAfterRotation[i],
                         p->g0425ChargesBefore[i], spec->f0301WriteAnchor);
        (void)snprintf(label, sizeof(label), "G0425 slot %d qty stable", i);
        ok &= expect_int(label, p->g0425QuantitiesAfterRotation[i],
                         p->g0425QuantitiesBefore[i], spec->f0301WriteAnchor);
        (void)snprintf(label, sizeof(label), "G0425 slot %d byte stable", i);
        ok &= expect_int(label, p->g0425SlotByteStable[i], 1,
                         spec->f0301WriteAnchor);
    }
    ok &= expect_int("all loads stable", p->allLoadsByteStableAcrossRotation,
                     1, spec->f0297HandAnchor);
    for (i = 0; i < DM1_V1_CHEST_C061_DROP_ROT_CHAMPION_COUNT_PC34; ++i) {
        char label[72];

        (void)snprintf(label, sizeof(label), "champ %d load after", i);
        ok &= expect_int(label, p->loadAfterRotation[i],
                         p->loadBefore[i], spec->f0297HandAnchor);
        (void)snprintf(label, sizeof(label), "champ %d load delta", i);
        ok &= expect_int(label, p->loadDelta[i], 0,
                         spec->f0298HandAnchor);
        (void)snprintf(label, sizeof(label), "champ %d load stable", i);
        ok &= expect_int(label, p->loadByteStable[i], 1,
                         spec->f0301WriteAnchor);
    }
    return ok;
}

static int test_io_append_and_disjoint(
    const DM1_V1_ChestC061DropWhileLeaderRotationProbePc34* p,
    const DM1_V1_ChestC061DropWhileLeaderRotationSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("F0077 observed", p->f0077Observed, 1,
                     spec->f0077Anchor);
    ok &= expect_int("F0078 observed", p->f0078Observed, 1,
                     spec->f0078Anchor);
    ok &= expect_int("mouse depth", p->mouseUpdateDepthAfterCapture, 0,
                     spec->f0078Anchor);
    ok &= expect_int("mouse balanced", p->mouseUpdateBalanced, 1,
                     spec->f0078Anchor);
    ok &= expect_int("object masks checked", p->objectMaskCheckedByF0032F0033,
                     1, spec->f0032ObjectAnchor);
    ok &= expect_int("F0163 append not reached", p->f0163AppendNotReached,
                     1, spec->f0163AppendAnchor);
    ok &= expect_int("not pass786", p->noPass786C040MirrorCandidateDrain, 1,
                     spec->disjointness);
    ok &= expect_int("not pass771", p->noPass771ScrollWheelDropDuringRotation,
                     1, spec->disjointness);
    ok &= expect_int("not close rotate pickup",
                     p->noChestCloseWhilePartyRotatePickupPending, 1,
                     spec->disjointness);
    ok &= expect_int("not mirror C160 close",
                     p->noMirrorCandidateC160CloseRotation, 1,
                     spec->disjointness);
    ok &= expect_int("not HUD food water",
                     p->noChampionPanelHudFoodWaterRecompute, 1,
                     spec->disjointness);
    return ok;
}

int main(void)
{
    const DM1_V1_ChestC061DropWhileLeaderRotationSpecPc34* spec =
        dm1_v1_chest_c061_drop_while_leader_rotation_spec_pc34();
    DM1_V1_ChestC061DropWhileLeaderRotationProbePc34 probe;
    int ok = 1;

    memset(&probe, 0, sizeof(probe));
    ok &= expect_true("spec accessor", spec != NULL,
                      "CHEST.C F0333:30-67");
    ok &= test_source_metadata(spec);
    ok &= expect_int("run accepted",
                     dm1_v1_chest_c061_drop_while_leader_rotation_run_pc34(
                         &probe),
                     1, spec->f0380QueueAnchor);
    ok &= test_probe_flags(&probe, spec);
    ok &= test_sequence_and_queue(&probe, spec);
    ok &= test_panel_g0426_and_hand(&probe, spec);
    ok &= test_g0425_slots_and_loads(&probe, spec);
    ok &= test_io_append_and_disjoint(&probe, spec);
    ok &= expect_int("null run rejected",
                     dm1_v1_chest_c061_drop_while_leader_rotation_run_pc34(
                         NULL),
                     0, spec->contractMarker);
    ok &= expect_true("assertion floor", g_assertions >= 100,
                      spec->contractMarker);

    if (!ok || g_failures) {
        printf("FAIL test_dm1_v1_chest_c061_drop_while_leader_rotation_pc34_compat assertions=%d failures=%d hash=0x%08X\n",
               g_assertions, g_failures,
               (unsigned)probe.deterministicHash);
        return 1;
    }

    printf("PASS test_dm1_v1_chest_c061_drop_while_leader_rotation_pc34_compat assertions=%d failures=0 hash=0x%08X\n",
           g_assertions,
           (unsigned)probe.deterministicHash);
    return 0;
}
