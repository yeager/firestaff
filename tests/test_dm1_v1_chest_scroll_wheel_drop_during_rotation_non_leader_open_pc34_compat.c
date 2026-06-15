#include "firestaff/dm1/v1/chest/scroll_wheel_drop_during_rotation_non_leader_open_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/*
 * ReDMCSB anchors asserted by this asset-free runtime ctest:
 * CHEST.C F0333:30-67 + F0334:113-132, CHAMPION.C F0297/F0298:243-298,
 * F0301/F0302:606-714, COMMAND.C F0359:1452-1662, F0380:2045-2178,
 * IO.C F0077/F0078:1102-1122, and DEFS.H C30/C537..C544/C540.
 */

static DM1_V1_ChestScrollWheelDropDuringRotationNonLeaderOpenProbePc34 g_probe;
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

static int expected_type_after_drop(int slotIndex)
{
    if (slotIndex == DM1_PC34_SW_DROP_ROT_NLO_TARGET_SLOT_INDEX) {
        return DM1_PC34_SW_DROP_ROT_NLO_DROP_ITEM;
    }
    return DM1_PC34_SW_DROP_ROT_NLO_FIRST_STABLE_ITEM + slotIndex;
}

static int expected_charges_after_drop(int slotIndex)
{
    if (slotIndex == DM1_PC34_SW_DROP_ROT_NLO_TARGET_SLOT_INDEX) {
        return DM1_PC34_SW_DROP_ROT_NLO_DROP_CHARGES;
    }
    return DM1_PC34_SW_DROP_ROT_NLO_FIRST_CHARGES + slotIndex;
}

static int expected_quantity_after_drop(int slotIndex)
{
    if (slotIndex == DM1_PC34_SW_DROP_ROT_NLO_TARGET_SLOT_INDEX) {
        return DM1_PC34_SW_DROP_ROT_NLO_DROP_QUANTITY;
    }
    return DM1_PC34_SW_DROP_ROT_NLO_FIRST_QUANTITY + slotIndex;
}

static int test_source_and_spec(
    const DM1_V1_ChestScrollWheelDropDuringRotationNonLeaderOpenSpecPc34* spec)
{
    const char* evidence =
        dm1_v1_chest_scroll_wheel_drop_during_rotation_non_leader_open_source_evidence_pc34();
    int ok = 1;

    ok &= expect_contains("evidence F0333", evidence,
                          "CHEST.C F0333:30-67",
                          spec->f0333OpenAnchor);
    ok &= expect_contains("evidence F0334 negative", evidence,
                          "CHEST.C F0334:113-132",
                          spec->f0334CloseNegativeAnchor);
    ok &= expect_contains("evidence F0297", evidence,
                          "CHAMPION.C F0297:243-298",
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
    ok &= expect_contains("evidence DEFS", evidence,
                          "C537..C544",
                          spec->defsAnchor);
    ok &= expect_contains("evidence nonduplicate", evidence,
                          "pass771 is not pass768 close race",
                          spec->nonDuplicateMarker);

    ok &= expect_int("spec old leader", spec->oldLeaderIndex,
                     DM1_PC34_SW_DROP_ROT_NLO_OLD_LEADER,
                     spec->f0380DrainAnchor);
    ok &= expect_int("spec nonleader open", spec->nonLeaderOpenIndex,
                     DM1_PC34_SW_DROP_ROT_NLO_NON_LEADER_OPEN,
                     spec->f0333OpenAnchor);
    ok &= expect_int("spec new leader", spec->newLeaderIndex,
                     DM1_PC34_SW_DROP_ROT_NLO_NEW_LEADER,
                     spec->f0380DrainAnchor);
    ok &= expect_int("spec target zone C540", spec->targetZone, 540,
                     spec->defsAnchor);
    ok &= expect_int("spec target slot box C41", spec->targetSlotBox, 41,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("spec target pc34 C33", spec->targetPc34Slot,
                     DM1_PC34_SLOT_CHEST_4,
                     spec->defsAnchor);
    ok &= expect_int("spec queued command C061", spec->targetCommand, 61,
                     spec->f0359QueueAnchor);
    return ok;
}

static int test_setup_and_queue(
    const DM1_V1_ChestScrollWheelDropDuringRotationNonLeaderOpenProbePc34* p,
    const DM1_V1_ChestScrollWheelDropDuringRotationNonLeaderOpenSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("runtime regression", p->runtimeRegression, 1,
                     spec->nonDuplicateMarker);
    ok &= expect_int("step count", p->stepCount, 5,
                     spec->f0380DrainAnchor);
    ok &= expect_int("step open nonleader", p->stepTrace[0],
                     DM1_PC34_SW_DROP_ROT_NLO_STEP_OPEN_NON_LEADER_CHEST,
                     spec->f0333OpenAnchor);
    ok &= expect_int("step queue", p->stepTrace[1],
                     DM1_PC34_SW_DROP_ROT_NLO_STEP_QUEUE_DROP_AND_ROTATION,
                     spec->f0359QueueAnchor);
    ok &= expect_int("step drop first", p->stepTrace[2],
                     DM1_PC34_SW_DROP_ROT_NLO_STEP_F0380_DROP_C540,
                     spec->f0380DrainAnchor);
    ok &= expect_int("step rotate second", p->stepTrace[3],
                     DM1_PC34_SW_DROP_ROT_NLO_STEP_F0380_ROTATE_LEADER,
                     spec->f0380DrainAnchor);
    ok &= expect_int("step still open", p->stepTrace[4],
                     DM1_PC34_SW_DROP_ROT_NLO_STEP_ASSERT_STILL_OPEN,
                     spec->f0334CloseNegativeAnchor);
    ok &= expect_int("open result", p->openResult, 1,
                     spec->f0333OpenAnchor);
    ok &= expect_int("open champion before", p->openChampionBefore,
                     spec->nonLeaderOpenIndex,
                     spec->f0333OpenAnchor);
    ok &= expect_int("open chest thing before", p->openChestThingBefore,
                     DM1_PC34_SW_DROP_ROT_NLO_CHEST_THING,
                     spec->f0333OpenAnchor);
    ok &= expect_int("panel chest before", p->panelContentBefore,
                     DM1_PC34_PANEL_CHEST,
                     spec->f0333OpenAnchor);
    ok &= expect_int("leader before queue", p->leaderBeforeQueue,
                     spec->oldLeaderIndex,
                     spec->f0380DrainAnchor);
    ok &= expect_int("old leader hand before", p->oldLeaderHandTypeBefore,
                     DM1_PC34_SW_DROP_ROT_NLO_DROP_ITEM,
                     spec->f0297HandAnchor);
    ok &= expect_int("old leader hand weight before",
                     p->oldLeaderHandWeightBefore,
                     DM1_PC34_SW_DROP_ROT_NLO_DROP_WEIGHT,
                     spec->f0297HandAnchor);
    ok &= expect_int("old leader hand charges before",
                     p->oldLeaderHandChargesBefore,
                     DM1_PC34_SW_DROP_ROT_NLO_DROP_CHARGES,
                     spec->f0297HandAnchor);
    ok &= expect_int("old leader hand quantity before",
                     p->oldLeaderHandQuantityBefore,
                     DM1_PC34_SW_DROP_ROT_NLO_DROP_QUANTITY,
                     spec->f0297HandAnchor);
    ok &= expect_int("new leader hand before", p->newLeaderHandTypeBefore,
                     DM1_PC34_SW_DROP_ROT_NLO_NEW_LEADER_HAND_ITEM,
                     spec->f0298HandAnchor);
    ok &= expect_int("new leader hand weight before",
                     p->newLeaderHandWeightBefore,
                     DM1_PC34_SW_DROP_ROT_NLO_NEW_HAND_WEIGHT,
                     spec->f0298HandAnchor);
    ok &= expect_int("new leader hand charges before",
                     p->newLeaderHandChargesBefore,
                     DM1_PC34_SW_DROP_ROT_NLO_NEW_HAND_CHARGES,
                     spec->f0298HandAnchor);
    ok &= expect_int("C540 empty before", p->c540EmptyBeforeDrop, 1,
                     spec->defsAnchor);
    ok &= expect_int("visible before coherent", p->c537ToC544VisibleBefore, 1,
                     spec->f0333OpenAnchor);
    ok &= expect_int("drop queued", p->dropQueued, 1,
                     spec->f0359QueueAnchor);
    ok &= expect_int("rotation queued", p->rotationQueued, 1,
                     spec->f0359QueueAnchor);
    ok &= expect_int("queued champion old leader", p->queuedChampion,
                     spec->oldLeaderIndex,
                     spec->f0359QueueAnchor);
    ok &= expect_int("queued open champion nonleader", p->queuedOpenChampion,
                     spec->nonLeaderOpenIndex,
                     spec->f0359QueueAnchor);
    ok &= expect_int("queued new leader", p->queuedNewLeader,
                     spec->newLeaderIndex,
                     spec->f0359QueueAnchor);
    ok &= expect_int("queued zone", p->queuedZone,
                     DM1_PC34_SW_DROP_ROT_NLO_TARGET_ZONE,
                     spec->defsAnchor);
    ok &= expect_int("queued slot box", p->queuedSlotBox,
                     DM1_PC34_SW_DROP_ROT_NLO_TARGET_SLOT_BOX,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("queued pc34 slot", p->queuedPc34Slot,
                     DM1_PC34_SW_DROP_ROT_NLO_TARGET_PC34_SLOT,
                     spec->defsAnchor);
    ok &= expect_int("queued command", p->queuedCommand,
                     DM1_PC34_SW_DROP_ROT_NLO_TARGET_COMMAND,
                     spec->f0359QueueAnchor);
    ok &= expect_int("queue depth after queue",
                     p->commandQueueDepthAfterQueue, 2,
                     spec->f0359QueueAnchor);
    return ok;
}

static int test_visible_before(
    const DM1_V1_ChestScrollWheelDropDuringRotationNonLeaderOpenProbePc34* p,
    const DM1_V1_ChestScrollWheelDropDuringRotationNonLeaderOpenSpecPc34* spec)
{
    int i;
    int ok = 1;

    for (i = 0; i < DM1_PC34_SW_DROP_ROT_NLO_SLOT_COUNT; ++i) {
        char label[96];

        if (i == DM1_PC34_SW_DROP_ROT_NLO_TARGET_SLOT_INDEX) {
            snprintf(label, sizeof(label), "before C%d empty type", 537 + i);
            ok &= expect_int(label, p->visibleTypesBefore[i], 0,
                             spec->defsAnchor);
            snprintf(label, sizeof(label), "before C%d empty charges",
                     537 + i);
            ok &= expect_int(label, p->visibleChargesBefore[i], 0,
                             spec->defsAnchor);
            snprintf(label, sizeof(label), "before C%d empty quantity",
                     537 + i);
            ok &= expect_int(label, p->visibleQuantitiesBefore[i], 0,
                             spec->defsAnchor);
            continue;
        }

        snprintf(label, sizeof(label), "before C%d type", 537 + i);
        ok &= expect_int(label, p->visibleTypesBefore[i],
                         DM1_PC34_SW_DROP_ROT_NLO_FIRST_STABLE_ITEM + i,
                         spec->f0333OpenAnchor);
        snprintf(label, sizeof(label), "before C%d charges", 537 + i);
        ok &= expect_int(label, p->visibleChargesBefore[i],
                         DM1_PC34_SW_DROP_ROT_NLO_FIRST_CHARGES + i,
                         spec->f0333OpenAnchor);
        snprintf(label, sizeof(label), "before C%d quantity", 537 + i);
        ok &= expect_int(label, p->visibleQuantitiesBefore[i],
                         DM1_PC34_SW_DROP_ROT_NLO_FIRST_QUANTITY + i,
                         spec->f0333OpenAnchor);
    }
    return ok;
}

static int test_drop_and_rotation(
    const DM1_V1_ChestScrollWheelDropDuringRotationNonLeaderOpenProbePc34* p,
    const DM1_V1_ChestScrollWheelDropDuringRotationNonLeaderOpenSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("F0077 observed", p->f0077Observed, 1,
                     spec->f0077EnableAnchor);
    ok &= expect_int("F0078 observed after drop", p->f0078ObservedAfterDrop, 1,
                     spec->f0078DisableAnchor);
    ok &= expect_int("mouse depth after drop", p->mouseUpdateDepthAfterDrop, 0,
                     spec->f0078DisableAnchor);
    ok &= expect_int("drop drains first", p->dropDrainFirst, 1,
                     spec->f0380DrainAnchor);
    ok &= expect_int("drop click result", p->dropClickResult, 1,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("queue depth after drop", p->commandQueueDepthAfterDrop,
                     1, spec->f0380DrainAnchor);
    ok &= expect_int("open chest after drop", p->openChestThingAfterDrop,
                     DM1_PC34_SW_DROP_ROT_NLO_CHEST_THING,
                     spec->f0333OpenAnchor);
    ok &= expect_int("panel chest after drop", p->panelContentAfterDrop,
                     DM1_PC34_PANEL_CHEST,
                     spec->f0333OpenAnchor);
    ok &= expect_int("old leader hand empty after drop",
                     p->oldLeaderHandTypeAfterDrop, 0,
                     spec->f0298HandAnchor);
    ok &= expect_int("new leader hand untouched after drop",
                     p->newLeaderHandTypeAfterDrop,
                     DM1_PC34_SW_DROP_ROT_NLO_NEW_LEADER_HAND_ITEM,
                     spec->f0297HandAnchor);
    ok &= expect_int("C540 type after drop", p->c540TypeAfterDrop,
                     DM1_PC34_SW_DROP_ROT_NLO_DROP_ITEM,
                     spec->f0301SlotAnchor);
    ok &= expect_int("C540 weight after drop", p->c540WeightAfterDrop,
                     DM1_PC34_SW_DROP_ROT_NLO_DROP_WEIGHT,
                     spec->f0301SlotAnchor);
    ok &= expect_int("C540 charges after drop", p->c540ChargesAfterDrop,
                     DM1_PC34_SW_DROP_ROT_NLO_DROP_CHARGES,
                     spec->f0301SlotAnchor);
    ok &= expect_int("C540 quantity after drop", p->c540QuantityAfterDrop,
                     DM1_PC34_SW_DROP_ROT_NLO_DROP_QUANTITY,
                     spec->f0301SlotAnchor);
    ok &= expect_int("C540 persisted before rotate",
                     p->c540DropPersistedBeforeRotate, 1,
                     spec->f0301SlotAnchor);

    ok &= expect_int("rotation consumed", p->rotationConsumed, 1,
                     spec->f0380DrainAnchor);
    ok &= expect_int("queue depth after rotate",
                     p->commandQueueDepthAfterRotate, 0,
                     spec->f0380DrainAnchor);
    ok &= expect_int("leader after rotate", p->leaderAfterRotate,
                     spec->newLeaderIndex,
                     spec->f0380DrainAnchor);
    ok &= expect_int("open champion after rotate",
                     p->openChampionAfterRotate,
                     spec->nonLeaderOpenIndex,
                     spec->f0333OpenAnchor);
    ok &= expect_int("new leader open chest after rotate",
                     p->newLeaderOpenChestThingAfterRotate,
                     DM1_PC34_SW_DROP_ROT_NLO_CHEST_THING,
                     spec->f0333OpenAnchor);
    ok &= expect_int("panel chest after rotate", p->panelContentAfterRotate,
                     DM1_PC34_PANEL_CHEST,
                     spec->f0333OpenAnchor);
    ok &= expect_int("old leader hand empty after rotate",
                     p->oldLeaderHandEmptyAfterRotate, 1,
                     spec->f0298HandAnchor);
    ok &= expect_int("new leader hand after rotate",
                     p->newLeaderHandTypeAfterRotate,
                     DM1_PC34_SW_DROP_ROT_NLO_NEW_LEADER_HAND_ITEM,
                     spec->f0297HandAnchor);
    ok &= expect_int("new leader hand weight after rotate",
                     p->newLeaderHandWeightAfterRotate,
                     DM1_PC34_SW_DROP_ROT_NLO_NEW_HAND_WEIGHT,
                     spec->f0297HandAnchor);
    ok &= expect_int("new leader hand charges after rotate",
                     p->newLeaderHandChargesAfterRotate,
                     DM1_PC34_SW_DROP_ROT_NLO_NEW_HAND_CHARGES,
                     spec->f0297HandAnchor);
    ok &= expect_int("new leader hand preserved after rotate",
                     p->newLeaderHandPreservedAfterRotate, 1,
                     spec->f0297HandAnchor);
    return ok;
}

static int test_visible_after_rotate(
    const DM1_V1_ChestScrollWheelDropDuringRotationNonLeaderOpenProbePc34* p,
    const DM1_V1_ChestScrollWheelDropDuringRotationNonLeaderOpenSpecPc34* spec)
{
    int i;
    int ok = 1;

    ok &= expect_int("C540 type after rotate", p->c540TypeAfterRotate,
                     DM1_PC34_SW_DROP_ROT_NLO_DROP_ITEM,
                     spec->defsAnchor);
    ok &= expect_int("C540 quantity after rotate", p->c540QuantityAfterRotate,
                     DM1_PC34_SW_DROP_ROT_NLO_DROP_QUANTITY,
                     spec->defsAnchor);
    ok &= expect_int("C540 still visible after rotate",
                     p->c540StillVisibleAfterRotate, 1,
                     spec->defsAnchor);
    ok &= expect_int("C537-C544 coherent after rotate",
                     p->c537ToC544ChainCoherentAfterRotate, 1,
                     spec->defsAnchor);
    ok &= expect_int("chest never closed", p->chestNeverClosed, 1,
                     spec->f0334CloseNegativeAnchor);
    ok &= expect_int("close count", p->closeCount, 0,
                     spec->f0334CloseNegativeAnchor);
    ok &= expect_int("F0077/F0078 balanced", p->f0077F0078Balanced, 1,
                     spec->f0078DisableAnchor);

    for (i = 0; i < DM1_PC34_SW_DROP_ROT_NLO_SLOT_COUNT; ++i) {
        char label[96];

        snprintf(label, sizeof(label), "after rotate C%d type", 537 + i);
        ok &= expect_int(label, p->visibleTypesAfterRotate[i],
                         expected_type_after_drop(i),
                         spec->defsAnchor);
        snprintf(label, sizeof(label), "after rotate C%d charges", 537 + i);
        ok &= expect_int(label, p->visibleChargesAfterRotate[i],
                         expected_charges_after_drop(i),
                         spec->defsAnchor);
        snprintf(label, sizeof(label), "after rotate C%d quantity", 537 + i);
        ok &= expect_int(label, p->visibleQuantitiesAfterRotate[i],
                         expected_quantity_after_drop(i),
                         spec->defsAnchor);
    }
    return ok;
}

static int test_non_overlap(
    const DM1_V1_ChestScrollWheelDropDuringRotationNonLeaderOpenProbePc34* p,
    const DM1_V1_ChestScrollWheelDropDuringRotationNonLeaderOpenSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("nonoverlap pass768 close race", p->noPass768CloseRace,
                     1, spec->nonDuplicateMarker);
    ok &= expect_int("nonoverlap scroll_wheel_pickup_drop",
                     p->noScrollWheelPickupDrop, 1,
                     spec->nonDuplicateMarker);
    ok &= expect_int("nonoverlap scroll_wheel_drop_onto_open_chest_slot",
                     p->noScrollWheelDropOntoOpenChestSlot, 1,
                     spec->nonDuplicateMarker);
    ok &= expect_int("nonoverlap chest_deposit_during_leader_rotation",
                     p->noChestDepositDuringLeaderRotation, 1,
                     spec->nonDuplicateMarker);
    ok &= expect_int("nonoverlap chest_pickup_during_resurrect_pending_non_leader",
                     p->noPickupDuringResurrectPendingNonLeader, 1,
                     spec->nonDuplicateMarker);
    ok &= expect_int("nonoverlap chest_close_while_candidate_live_non_leader",
                     p->noChestCloseWhileCandidateLiveNonLeader, 1,
                     spec->nonDuplicateMarker);
    ok &= expect_int("nonoverlap chest_open_during_pending",
                     p->noChestOpenDuringPending, 1,
                     spec->nonDuplicateMarker);
    ok &= expect_int("nonoverlap chest_close_with_full_leader_hand",
                     p->noChestCloseWithFullLeaderHand, 1,
                     spec->nonDuplicateMarker);
    ok &= expect_int("nonoverlap chest_open_with_full_leader_hand",
                     p->noChestOpenWithFullLeaderHand, 1,
                     spec->nonDuplicateMarker);
    return ok;
}

int main(void)
{
    const DM1_V1_ChestScrollWheelDropDuringRotationNonLeaderOpenSpecPc34* spec =
        dm1_v1_chest_scroll_wheel_drop_during_rotation_non_leader_open_spec_pc34();
    int ok = 1;

    printf("probe=dm1_v1_chest_scroll_wheel_drop_during_rotation_non_leader_open_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_chest_scroll_wheel_drop_during_rotation_non_leader_open_source_evidence_pc34());

    ok &= expect_int(
        "probe run",
        dm1_v1_chest_scroll_wheel_drop_during_rotation_non_leader_open_run_pc34(&g_probe),
        1, spec->f0380DrainAnchor);
    ok &= test_source_and_spec(spec);
    ok &= test_setup_and_queue(&g_probe, spec);
    ok &= test_visible_before(&g_probe, spec);
    ok &= test_drop_and_rotation(&g_probe, spec);
    ok &= test_visible_after_rotate(&g_probe, spec);
    ok &= test_non_overlap(&g_probe, spec);
    ok &= expect_u32("deterministic hash",
                     g_probe.deterministicHash,
                     0xCD774F78u,
                     spec->f0380DrainAnchor);

    printf("deterministicHash=0x%08X\n",
           (unsigned)g_probe.deterministicHash);
    printf("assertions=%d failures=%d hash=0x%08X\n",
           g_assertions, g_failures, (unsigned)g_probe.deterministicHash);
    if (!ok || g_failures || g_assertions < 120 || g_assertions > 240) {
        printf("FAIL dm1_v1_chest_scroll_wheel_drop_during_rotation_non_leader_open_pc34_compat\n");
        return 1;
    }
    printf("PASS dm1_v1_chest_scroll_wheel_drop_during_rotation_non_leader_open_pc34_compat\n");
    printf("assertions=%d failures=0 hash=0x%08X\n",
           g_assertions, (unsigned)g_probe.deterministicHash);
    return 0;
}
