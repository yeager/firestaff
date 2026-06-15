#include "firestaff/dm1/v1/chest/scroll_wheel_close_race_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/*
 * ReDMCSB anchors asserted by this runtime ctest:
 * CHEST.C F0333:30-67 and F0334:113-132, CHAMPION.C F0297/F0298:243-298,
 * F0301/F0302:606-714, COMMAND.C F0359:1452-1662, F0380:2045-2178,
 * IO.C F0077:1113-1122, F0078:1102-1111, and DEFS.H
 * C030/C10/C30/C38/C537..C544/G0425/G0426/C540.
 */

static DM1_V1_ChestScrollWheelCloseRaceProbePc34 g_probe;
static DM1_V1_ChestScrollWheelCloseRaceProbePc34 g_probe_repeat;
static int g_assertions;
static int g_failures;

static int expect_int(const char* label,
                      int got,
                      int want,
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
        printf("FAIL %s got=%d want=%d anchor=%s\n",
               label, got, want, anchor);
        return 0;
    }
    printf("PASS %s=%d anchor=%s\n", label, got, anchor);
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
    printf("PASS %s=0x%08X anchor=%s\n",
           label, (unsigned)got, anchor);
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
    printf("PASS %s contains=%s anchor=%s\n", label, needle, anchor);
    return 1;
}

static int expected_type(int index)
{
    if (index >= DM1_PC34_SCROLL_CLOSE_VISIBLE_COUNT) {
        return 0;
    }
    return DM1_PC34_SCROLL_CLOSE_FIRST_ITEM + index;
}

static int expected_charges(int index)
{
    if (index >= DM1_PC34_SCROLL_CLOSE_VISIBLE_COUNT) {
        return 0;
    }
    return DM1_PC34_SCROLL_CLOSE_FIRST_CHARGES + index;
}

static int expected_quantity(int index)
{
    if (index >= DM1_PC34_SCROLL_CLOSE_VISIBLE_COUNT) {
        return 0;
    }
    return DM1_PC34_SCROLL_CLOSE_FIRST_QUANTITY + index;
}

static int test_source_and_spec(
    const DM1_V1_ChestScrollWheelCloseRaceSpecPc34* spec)
{
    const char* evidence =
        dm1_v1_chest_scroll_wheel_close_race_source_evidence_pc34();
    int ok = 1;

    ok &= expect_contains("evidence F0333", evidence,
                          "CHEST.C F0333:30-67",
                          spec->f0333OpenAnchor);
    ok &= expect_contains("evidence F0334", evidence,
                          "CHEST.C F0334:113-132",
                          spec->f0334CloseAnchor);
    ok &= expect_contains("evidence F0297", evidence,
                          "CHAMPION.C F0297:243-268",
                          spec->f0297HandAnchor);
    ok &= expect_contains("evidence F0298", evidence,
                          "CHAMPION.C F0298:270-298",
                          spec->f0298HandAnchor);
    ok &= expect_contains("evidence F0301", evidence,
                          "CHAMPION.C F0301:606-614",
                          spec->f0301SlotAnchor);
    ok &= expect_contains("evidence F0302", evidence,
                          "CHAMPION.C F0302:662-714",
                          spec->f0302DispatchAnchor);
    ok &= expect_contains("evidence F0359", evidence,
                          "COMMAND.C F0359:1452-1662",
                          spec->f0359QueueAnchor);
    ok &= expect_contains("evidence F0380", evidence,
                          "COMMAND.C F0380:2045-2178",
                          spec->f0380DrainAnchor);
    ok &= expect_contains("evidence F0077", evidence,
                          "IO.C F0077:1113-1122",
                          spec->f0077EnableAnchor);
    ok &= expect_contains("evidence F0078", evidence,
                          "IO.C F0078:1102-1111",
                          spec->f0078DisableAnchor);
    ok &= expect_contains("evidence DEFS C540", evidence,
                          "2088 C10_COLOR_FLESH",
                          spec->defsAnchor);
    ok &= expect_contains("spec marker stale", spec->contractMarker,
                          "queued scroll-wheel C540 command",
                          spec->contractMarker);

    ok &= expect_int("spec old leader", spec->oldLeaderIndex,
                     DM1_PC34_SCROLL_CLOSE_OLD_LEADER,
                     spec->f0380DrainAnchor);
    ok &= expect_int("spec new leader", spec->newLeaderIndex,
                     DM1_PC34_SCROLL_CLOSE_NEW_LEADER,
                     spec->f0380DrainAnchor);
    ok &= expect_int("spec queued zone", spec->queuedZone,
                     DM1_PC34_SCROLL_CLOSE_QUEUE_ZONE,
                     spec->defsAnchor);
    ok &= expect_int("spec queued slot box", spec->queuedSlotBox,
                     DM1_PC34_SCROLL_CLOSE_QUEUE_SLOT_BOX,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("spec queued pc34 slot", spec->queuedPc34Slot,
                     DM1_PC34_SCROLL_CLOSE_QUEUE_PC34_SLOT,
                     spec->defsAnchor);
    ok &= expect_int("spec queued command", spec->queuedCommand,
                     DM1_PC34_SCROLL_CLOSE_QUEUE_COMMAND,
                     spec->f0359QueueAnchor);
    ok &= expect_int("spec expected close count", spec->expectedCloseCount,
                     DM1_PC34_SCROLL_CLOSE_VISIBLE_COUNT,
                     spec->f0334CloseAnchor);
    return ok;
}

static int test_non_duplicate_markers(
    const DM1_V1_ChestScrollWheelCloseRaceSpecPc34* spec)
{
    const char* evidence =
        dm1_v1_chest_scroll_wheel_close_race_source_evidence_pc34();
    int ok = 1;

    ok &= expect_contains("nondup deposit", evidence,
                          "not chest_deposit_during_leader_rotation",
                          spec->nonDuplicateMarker);
    ok &= expect_contains("nondup reopen leader", evidence,
                          "not chest_reopen_after_leader_rotation",
                          spec->nonDuplicateMarker);
    ok &= expect_contains("nondup candidate", evidence,
                          "not chest_close_while_candidate_live_non_leader",
                          spec->nonDuplicateMarker);
    ok &= expect_contains("nondup open pending", evidence,
                          "not chest_open_during_pending",
                          spec->nonDuplicateMarker);
    ok &= expect_contains("nondup full hand", evidence,
                          "not chest_open_with_full_leader_hand",
                          spec->nonDuplicateMarker);
    ok &= expect_contains("nondup partial mask", evidence,
                          "not chest_partial_mask_swap",
                          spec->nonDuplicateMarker);
    ok &= expect_contains("nondup scroll pickup drop", evidence,
                          "not chest_scroll_wheel_pickup_drop",
                          spec->nonDuplicateMarker);
    ok &= expect_contains("nondup scroll pull", evidence,
                          "not chest_scroll_wheel_pull_from_chest",
                          spec->nonDuplicateMarker);
    ok &= expect_contains("nondup pickup family", evidence,
                          "not chest_pickup_*",
                          spec->nonDuplicateMarker);
    ok &= expect_contains("nondup mirror family", evidence,
                          "not mirror_candidate_*",
                          spec->nonDuplicateMarker);
    ok &= expect_contains("nondup resurrect", evidence,
                          "not resurrect-pending",
                          spec->nonDuplicateMarker);
    ok &= expect_contains("nondup C040", evidence,
                          "not C040-panel-live",
                          spec->nonDuplicateMarker);
    ok &= expect_contains("nondup occupied swap", evidence,
                          "not occupied-slot swap",
                          spec->nonDuplicateMarker);
    ok &= expect_contains("nondup save load", evidence,
                          "not save-load",
                          spec->nonDuplicateMarker);
    ok &= expect_contains("nondup teleporter", evidence,
                          "not teleporter",
                          spec->nonDuplicateMarker);
    ok &= expect_contains("nondup capacity", evidence,
                          "not capacity",
                          spec->nonDuplicateMarker);
    ok &= expect_contains("nondup encumbrance", evidence,
                          "not encumbrance",
                          spec->nonDuplicateMarker);
    ok &= expect_int("probe no pickup/drop", g_probe.noPickupDropPath, 1,
                     spec->nonDuplicateMarker);
    ok &= expect_int("probe no deposit", g_probe.noDepositPath, 1,
                     spec->nonDuplicateMarker);
    ok &= expect_int("probe no reopen", g_probe.noReopenPath, 1,
                     spec->nonDuplicateMarker);
    ok &= expect_int("probe no mirror", g_probe.noMirrorCandidatePath, 1,
                     spec->nonDuplicateMarker);
    ok &= expect_int("probe no resurrect", g_probe.noResurrectPendingPath, 1,
                     spec->nonDuplicateMarker);
    ok &= expect_int("probe no occupied swap", g_probe.noOccupiedSlotSwapPath,
                     1, spec->nonDuplicateMarker);
    ok &= expect_int("probe no partial mask", g_probe.noPartialMaskPath, 1,
                     spec->nonDuplicateMarker);
    ok &= expect_int("probe no different open", g_probe.noDifferentChestOpenPath,
                     1, spec->nonDuplicateMarker);
    ok &= expect_int("probe no save/load teleporter",
                     g_probe.noSaveLoadTeleporterPath, 1,
                     spec->nonDuplicateMarker);
    ok &= expect_int("probe no capacity encumbrance",
                     g_probe.noCapacityEncumbrancePath, 1,
                     spec->nonDuplicateMarker);
    return ok;
}

static int test_steps(
    const DM1_V1_ChestScrollWheelCloseRaceProbePc34* p,
    const DM1_V1_ChestScrollWheelCloseRaceSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("runtime regression flag", p->runtimeRegression, 1,
                     spec->contractMarker);
    ok &= expect_int("step count", p->stepCount, 6,
                     spec->f0380DrainAnchor);
    ok &= expect_int("step open", p->stepTrace[0],
                     DM1_PC34_SCROLL_CLOSE_STEP_OPEN_CHEST,
                     spec->f0333OpenAnchor);
    ok &= expect_int("step queue", p->stepTrace[1],
                     DM1_PC34_SCROLL_CLOSE_STEP_QUEUE_SCROLL,
                     spec->f0359QueueAnchor);
    ok &= expect_int("step close", p->stepTrace[2],
                     DM1_PC34_SCROLL_CLOSE_STEP_CLOSE_CHEST,
                     spec->f0334CloseAnchor);
    ok &= expect_int("step stale drain", p->stepTrace[3],
                     DM1_PC34_SCROLL_CLOSE_STEP_DRAIN_STALE_SCROLL,
                     spec->f0380DrainAnchor);
    ok &= expect_int("step rotate", p->stepTrace[4],
                     DM1_PC34_SCROLL_CLOSE_STEP_ROTATE_LEADER,
                     spec->f0380DrainAnchor);
    ok &= expect_int("step mouse balance", p->stepTrace[5],
                     DM1_PC34_SCROLL_CLOSE_STEP_MOUSE_BALANCED,
                     spec->f0078DisableAnchor);
    return ok;
}

static int test_open_and_queue(
    const DM1_V1_ChestScrollWheelCloseRaceProbePc34* p,
    const DM1_V1_ChestScrollWheelCloseRaceSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("open result", p->openResult, 1,
                     spec->f0333OpenAnchor);
    ok &= expect_int("open chest before queue", p->openChestThingBeforeQueue,
                     DM1_PC34_SCROLL_CLOSE_CHEST_THING,
                     spec->f0333OpenAnchor);
    ok &= expect_int("panel before queue", p->panelBeforeQueue,
                     DM1_PC34_SCROLL_CLOSE_PANEL_CHEST,
                     spec->f0333OpenAnchor);
    ok &= expect_int("leader before queue", p->leaderBeforeQueue,
                     DM1_PC34_SCROLL_CLOSE_OLD_LEADER,
                     spec->f0380DrainAnchor);
    ok &= expect_int("old leader hand before type",
                     p->oldLeaderHandTypeBeforeQueue,
                     DM1_PC34_SCROLL_CLOSE_LEADER_HAND_ITEM,
                     spec->f0297HandAnchor);
    ok &= expect_int("old leader hand before weight",
                     p->oldLeaderHandWeightBeforeQueue,
                     DM1_PC34_SCROLL_CLOSE_LEADER_HAND_WEIGHT,
                     spec->f0297HandAnchor);
    ok &= expect_int("old leader hand before charges",
                     p->oldLeaderHandChargesBeforeQueue,
                     DM1_PC34_SCROLL_CLOSE_LEADER_HAND_CHARGES,
                     spec->f0297HandAnchor);
    ok &= expect_int("new leader hand before empty",
                     p->newLeaderHandTypeBeforeQueue, 0,
                     spec->f0298HandAnchor);
    ok &= expect_int("scroll queued", p->scrollQueued, 1,
                     spec->f0359QueueAnchor);
    ok &= expect_int("rotation queued", p->rotationQueued, 1,
                     spec->f0380DrainAnchor);
    ok &= expect_int("queued champion", p->queuedChampion,
                     DM1_PC34_SCROLL_CLOSE_OLD_LEADER,
                     spec->f0359QueueAnchor);
    ok &= expect_int("queued new leader", p->queuedNewLeader,
                     DM1_PC34_SCROLL_CLOSE_NEW_LEADER,
                     spec->f0380DrainAnchor);
    ok &= expect_int("queued zone C540", p->queuedZone,
                     DM1_PC34_SCROLL_CLOSE_QUEUE_ZONE,
                     spec->defsAnchor);
    ok &= expect_int("queued slot box C41", p->queuedSlotBox,
                     DM1_PC34_SCROLL_CLOSE_QUEUE_SLOT_BOX,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("queued pc34 C33", p->queuedPc34Slot,
                     DM1_PC34_SCROLL_CLOSE_QUEUE_PC34_SLOT,
                     spec->defsAnchor);
    ok &= expect_int("queued command 61", p->queuedCommand,
                     DM1_PC34_SCROLL_CLOSE_QUEUE_COMMAND,
                     spec->f0359QueueAnchor);
    ok &= expect_int("queue depth after queue",
                     p->commandQueueDepthAfterQueue, 2,
                     spec->f0359QueueAnchor);
    ok &= expect_int("mouse depth after queue",
                     p->mouseUpdateDepthAfterQueue, 1,
                     spec->f0077EnableAnchor);
    ok &= expect_int("F0077 observed", p->f0077Observed, 1,
                     spec->f0077EnableAnchor);
    ok &= expect_int("F0078 pending", p->f0078Pending, 1,
                     spec->f0078DisableAnchor);
    return ok;
}

static int test_visible_slots_before_close(
    const DM1_V1_ChestScrollWheelCloseRaceProbePc34* p,
    const DM1_V1_ChestScrollWheelCloseRaceSpecPc34* spec)
{
    int ok = 1;
    int i;

    ok &= expect_int("C537-C544 materialized",
                     p->c537ToC544MaterializedBeforeClose, 1,
                     spec->f0333OpenAnchor);
    ok &= expect_int("queued slot type before close",
                     p->queuedSlotTypeBeforeClose,
                     expected_type(DM1_PC34_SCROLL_CLOSE_QUEUE_SLOT_INDEX),
                     spec->defsAnchor);
    for (i = 0; i < DM1_PC34_SCROLL_CLOSE_SLOT_COUNT; ++i) {
        char label[96];

        snprintf(label, sizeof(label), "visible C%d type before", 537 + i);
        ok &= expect_int(label, p->visibleTypesBeforeClose[i],
                         expected_type(i), spec->f0333OpenAnchor);
        snprintf(label, sizeof(label), "visible C%d charges before", 537 + i);
        ok &= expect_int(label, p->visibleChargesBeforeClose[i],
                         expected_charges(i), spec->f0333OpenAnchor);
        snprintf(label, sizeof(label), "visible C%d quantity before", 537 + i);
        ok &= expect_int(label, p->visibleQuantitiesBeforeClose[i],
                         expected_quantity(i), spec->f0333OpenAnchor);
    }
    return ok;
}

static int test_close_and_closed_chain(
    const DM1_V1_ChestScrollWheelCloseRaceProbePc34* p,
    const DM1_V1_ChestScrollWheelCloseRaceSpecPc34* spec)
{
    int ok = 1;
    int i;

    ok &= expect_int("close count", p->closeCount,
                     DM1_PC34_SCROLL_CLOSE_VISIBLE_COUNT,
                     spec->f0334CloseAnchor);
    ok &= expect_int("open chest after close", p->openChestThingAfterClose, 0,
                     spec->f0334CloseAnchor);
    ok &= expect_int("panel after close route", p->panelAfterCloseRoute,
                     DM1_PC34_SCROLL_CLOSE_PANEL_FOOD,
                     spec->f0334CloseAnchor);
    ok &= expect_int("queue depth after close", p->commandQueueDepthAfterClose,
                     2, spec->f0359QueueAnchor);
    ok &= expect_int("mouse depth after close", p->mouseUpdateDepthAfterClose,
                     1, spec->f0077EnableAnchor);
    ok &= expect_int("leader after close", p->leaderAfterClose,
                     DM1_PC34_SCROLL_CLOSE_OLD_LEADER,
                     spec->f0380DrainAnchor);
    ok &= expect_int("old hand after close type",
                     p->oldLeaderHandTypeAfterClose,
                     DM1_PC34_SCROLL_CLOSE_LEADER_HAND_ITEM,
                     spec->f0297HandAnchor);
    ok &= expect_int("old hand after close weight",
                     p->oldLeaderHandWeightAfterClose,
                     DM1_PC34_SCROLL_CLOSE_LEADER_HAND_WEIGHT,
                     spec->f0297HandAnchor);
    ok &= expect_int("old hand after close charges",
                     p->oldLeaderHandChargesAfterClose,
                     DM1_PC34_SCROLL_CLOSE_LEADER_HAND_CHARGES,
                     spec->f0297HandAnchor);
    ok &= expect_int("closed chain preserved", p->closedChainPreserved, 1,
                     spec->f0334CloseAnchor);
    ok &= expect_int("queued slot preserved in closed chain",
                     p->queuedSlotPreservedInClosedChain, 1,
                     spec->f0334CloseAnchor);
    for (i = 0; i < DM1_PC34_SCROLL_CLOSE_SLOT_COUNT; ++i) {
        char label[96];

        snprintf(label, sizeof(label), "closed C%d type", 537 + i);
        ok &= expect_int(label, p->closedTypes[i],
                         expected_type(i), spec->f0334CloseAnchor);
        snprintf(label, sizeof(label), "closed C%d charges", 537 + i);
        ok &= expect_int(label, p->closedCharges[i],
                         expected_charges(i), spec->f0334CloseAnchor);
        snprintf(label, sizeof(label), "closed C%d quantity", 537 + i);
        ok &= expect_int(label, p->closedQuantities[i],
                         expected_quantity(i), spec->f0334CloseAnchor);
    }
    return ok;
}

static int test_stale_drain_and_rotation(
    const DM1_V1_ChestScrollWheelCloseRaceProbePc34* p,
    const DM1_V1_ChestScrollWheelCloseRaceSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("stale drain attempted", p->staleScrollDrainAttempted, 1,
                     spec->f0380DrainAnchor);
    ok &= expect_int("stale scroll rejected", p->staleScrollRejected, 1,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("queue depth after stale drain",
                     p->commandQueueDepthAfterStaleDrain, 1,
                     spec->f0380DrainAnchor);
    ok &= expect_int("mouse depth after stale drain",
                     p->mouseUpdateDepthAfterStaleDrain, 0,
                     spec->f0078DisableAnchor);
    ok &= expect_int("open chest after stale drain",
                     p->openChestThingAfterStaleDrain, 0,
                     spec->f0334CloseAnchor);
    ok &= expect_int("old hand type after stale drain",
                     p->oldLeaderHandTypeAfterStaleDrain,
                     DM1_PC34_SCROLL_CLOSE_LEADER_HAND_ITEM,
                     spec->f0297HandAnchor);
    ok &= expect_int("old hand weight after stale drain",
                     p->oldLeaderHandWeightAfterStaleDrain,
                     DM1_PC34_SCROLL_CLOSE_LEADER_HAND_WEIGHT,
                     spec->f0297HandAnchor);
    ok &= expect_int("queued slot not pulled after close",
                     p->queuedSlotNotPulledAfterClose, 1,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("no chest slot mutation after close",
                     p->noChestSlotMutationAfterClose, 1,
                     spec->f0334CloseAnchor);
    ok &= expect_int("rotation consumed", p->rotationConsumed, 1,
                     spec->f0380DrainAnchor);
    ok &= expect_int("queue depth after rotate",
                     p->commandQueueDepthAfterRotate, 0,
                     spec->f0380DrainAnchor);
    ok &= expect_int("leader after rotate", p->leaderAfterRotate,
                     DM1_PC34_SCROLL_CLOSE_NEW_LEADER,
                     spec->f0380DrainAnchor);
    ok &= expect_int("old leader open chest after rotate",
                     p->oldLeaderOpenChestAfterRotate, 0,
                     spec->f0334CloseAnchor);
    ok &= expect_int("new leader open chest after rotate",
                     p->newLeaderOpenChestAfterRotate, 0,
                     spec->f0334CloseAnchor);
    ok &= expect_int("old leader hand after rotate",
                     p->oldLeaderHandTypeAfterRotate, 0,
                     spec->f0298HandAnchor);
    ok &= expect_int("new leader hand type after rotate",
                     p->newLeaderHandTypeAfterRotate,
                     DM1_PC34_SCROLL_CLOSE_LEADER_HAND_ITEM,
                     spec->f0297HandAnchor);
    ok &= expect_int("new leader hand weight after rotate",
                     p->newLeaderHandWeightAfterRotate,
                     DM1_PC34_SCROLL_CLOSE_LEADER_HAND_WEIGHT,
                     spec->f0297HandAnchor);
    ok &= expect_int("new leader hand charges after rotate",
                     p->newLeaderHandChargesAfterRotate,
                     DM1_PC34_SCROLL_CLOSE_LEADER_HAND_CHARGES,
                     spec->f0297HandAnchor);
    ok &= expect_int("leader hand coherent after rotate",
                     p->leaderHandCoherentAfterRotate, 1,
                     spec->f0297HandAnchor);
    ok &= expect_int("F0077/F0078 balanced", p->f0077F0078Balanced, 1,
                     spec->f0078DisableAnchor);
    ok &= expect_int("panel remains closed after rotate",
                     p->panelRemainsClosedAfterRotate, 1,
                     spec->f0334CloseAnchor);
    return ok;
}

int main(void)
{
    const DM1_V1_ChestScrollWheelCloseRaceSpecPc34* spec =
        dm1_v1_chest_scroll_wheel_close_race_spec_pc34();
    int ok = 1;

    printf("probe=dm1_v1_chest_scroll_wheel_close_race_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_chest_scroll_wheel_close_race_source_evidence_pc34());

    ok &= expect_int(
        "probe run",
        dm1_v1_chest_scroll_wheel_close_race_run_pc34(&g_probe),
        1, spec->f0380DrainAnchor);
    ok &= expect_int(
        "probe repeat run",
        dm1_v1_chest_scroll_wheel_close_race_run_pc34(&g_probe_repeat),
        1, spec->f0380DrainAnchor);
    ok &= test_source_and_spec(spec);
    ok &= test_steps(&g_probe, spec);
    ok &= test_open_and_queue(&g_probe, spec);
    ok &= test_visible_slots_before_close(&g_probe, spec);
    ok &= test_close_and_closed_chain(&g_probe, spec);
    ok &= test_stale_drain_and_rotation(&g_probe, spec);
    ok &= test_non_duplicate_markers(spec);
    ok &= expect_u32("repeat deterministic hash",
                     g_probe_repeat.deterministicHash,
                     g_probe.deterministicHash,
                     spec->f0380DrainAnchor);
    ok &= expect_u32("deterministic hash",
                     g_probe.deterministicHash,
                     0x247E9257u,
                     spec->f0334CloseAnchor);

    printf("deterministicHash=0x%08X\n",
           (unsigned)g_probe.deterministicHash);
    printf("assertions=%d failures=%d\n", g_assertions, g_failures);
    if (!ok || g_failures || g_assertions < 150) {
        printf("FAIL dm1_v1_chest_scroll_wheel_close_race_pc34_compat\n");
        return 1;
    }
    printf("PASS dm1_v1_chest_scroll_wheel_close_race_pc34_compat assertions=%d failures=0 deterministicHash=0x%08X\n",
           g_assertions, (unsigned)g_probe.deterministicHash);
    return 0;
}
