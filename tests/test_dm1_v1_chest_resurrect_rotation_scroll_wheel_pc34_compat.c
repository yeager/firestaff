#include "firestaff/dm1/v1/chest/resurrect_rotation_scroll_wheel_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/*
 * ReDMCSB anchors asserted by this asset-free runtime ctest:
 * CHEST.C F0333:30-67 + F0334:113-132, CHAMPION.C F0297/F0298:243-298,
 * F0301/F0302:606-714, COMMAND.C F0359:1452-1662, F0361:1709-1813,
 * F0380:2045-2178, IO.C F0077/F0078:1102-1122, REVIVE.C F0280/F0282,
 * PANEL.C F0349/F0350/F0351, and DEFS.H C030/C30/C537..C544/G0425/G0426.
 */

static DM1_V1_ChestResurrectRotationScrollWheelProbePc34 g_probe;
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

static int expected_initial_type(int slotIndex)
{
    if (slotIndex == DM1_PC34_CRR_SW_TARGET_SLOT_INDEX) {
        return DM1_PC34_CRR_SW_TARGET_SLOT_ITEM;
    }
    return DM1_PC34_CRR_SW_FIRST_STABLE_ITEM + slotIndex;
}

static int expected_initial_charges(int slotIndex)
{
    if (slotIndex == DM1_PC34_CRR_SW_TARGET_SLOT_INDEX) {
        return DM1_PC34_CRR_SW_TARGET_SLOT_CHARGES;
    }
    return DM1_PC34_CRR_SW_FIRST_CHARGES + slotIndex;
}

static int expected_initial_quantity(int slotIndex)
{
    if (slotIndex == DM1_PC34_CRR_SW_TARGET_SLOT_INDEX) {
        return DM1_PC34_CRR_SW_TARGET_SLOT_QUANTITY;
    }
    return DM1_PC34_CRR_SW_FIRST_QUANTITY + slotIndex;
}

static int expected_after_type(int slotIndex)
{
    if (slotIndex == DM1_PC34_CRR_SW_TARGET_SLOT_INDEX) {
        return DM1_PC34_CRR_SW_HAND_ITEM;
    }
    return expected_initial_type(slotIndex);
}

static int expected_after_charges(int slotIndex)
{
    if (slotIndex == DM1_PC34_CRR_SW_TARGET_SLOT_INDEX) {
        return DM1_PC34_CRR_SW_HAND_CHARGES;
    }
    return expected_initial_charges(slotIndex);
}

static int expected_after_quantity(int slotIndex)
{
    if (slotIndex == DM1_PC34_CRR_SW_TARGET_SLOT_INDEX) {
        return DM1_PC34_CRR_SW_HAND_QUANTITY;
    }
    return expected_initial_quantity(slotIndex);
}

static int test_source_and_spec(
    const DM1_V1_ChestResurrectRotationScrollWheelSpecPc34* spec)
{
    const char* evidence =
        dm1_v1_chest_resurrect_rotation_scroll_wheel_source_evidence_pc34();
    int ok = 1;

    ok &= expect_contains("evidence F0333", evidence,
                          "CHEST.C F0333:30-67",
                          spec->f0333OpenAnchor);
    ok &= expect_contains("evidence F0334", evidence,
                          "CHEST.C F0334:113-132",
                          spec->f0334CloseAnchor);
    ok &= expect_contains("evidence F0297/F0298", evidence,
                          "CHAMPION.C F0297/F0298:243-298",
                          spec->f0297HandAnchor);
    ok &= expect_contains("evidence F0301/F0302", evidence,
                          "CHAMPION.C F0301/F0302:606-714",
                          spec->f0302DispatchAnchor);
    ok &= expect_contains("evidence F0359", evidence,
                          "COMMAND.C F0359:1452-1662",
                          spec->f0359QueueAnchor);
    ok &= expect_contains("evidence F0361", evidence,
                          "COMMAND.C F0361:1709-1813",
                          spec->f0361WheelQueueAnchor);
    ok &= expect_contains("evidence F0380", evidence,
                          "COMMAND.C F0380:2045-2178",
                          spec->f0380DrainAnchor);
    ok &= expect_contains("evidence F0077", evidence,
                          "IO.C F0077:1113-1122",
                          spec->f0077EnableAnchor);
    ok &= expect_contains("evidence F0078", evidence,
                          "IO.C F0078:1102-1111",
                          spec->f0078DisableAnchor);
    ok &= expect_contains("evidence revive", evidence,
                          "REVIVE.C F0280/F0282",
                          spec->reviveAnchor);
    ok &= expect_contains("evidence panel", evidence,
                          "PANEL.C F0349/F0350/F0351",
                          spec->panelAnchor);
    ok &= expect_contains("evidence defs", evidence,
                          "C537..C544",
                          spec->defsAnchor);
    ok &= expect_contains("evidence nonduplicate", evidence,
                          "pass775 is C028 resurrect-panel close",
                          spec->nonDuplicateMarker);

    ok &= expect_int("spec old leader", spec->oldLeaderIndex,
                     DM1_PC34_CRR_SW_OLD_LEADER,
                     spec->f0380DrainAnchor);
    ok &= expect_int("spec nonleader open", spec->nonLeaderOpenIndex,
                     DM1_PC34_CRR_SW_NON_LEADER_OPEN,
                     spec->f0333OpenAnchor);
    ok &= expect_int("spec new leader", spec->newLeaderIndex,
                     DM1_PC34_CRR_SW_NEW_LEADER,
                     spec->f0380DrainAnchor);
    ok &= expect_int("spec C028 route", spec->c028PanelRoute,
                     DM1_PC34_CRR_SW_C028_ROUTE,
                     spec->reviveAnchor);
    ok &= expect_int("spec C029 route", spec->c029PanelRoute,
                     DM1_PC34_CRR_SW_C029_ROUTE,
                     spec->panelAnchor);
    ok &= expect_int("spec target zone C540", spec->targetZone, 540,
                     spec->defsAnchor);
    ok &= expect_int("spec target slot box", spec->targetSlotBox, 41,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("spec target pc34 slot", spec->targetPc34Slot,
                     DM1_PC34_SLOT_CHEST_4,
                     spec->defsAnchor);
    ok &= expect_int("spec queued command", spec->targetCommand, 61,
                     spec->f0359QueueAnchor);
    return ok;
}

static int test_setup_and_queue(
    const DM1_V1_ChestResurrectRotationScrollWheelProbePc34* p,
    const DM1_V1_ChestResurrectRotationScrollWheelSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("runtime regression", p->runtimeRegression, 1,
                     spec->nonDuplicateMarker);
    ok &= expect_int("step count", p->stepCount, 8,
                     spec->f0380DrainAnchor);
    ok &= expect_int("step open chest", p->stepTrace[0],
                     DM1_PC34_CRR_SW_STEP_OPEN_NON_LEADER_CHEST,
                     spec->f0333OpenAnchor);
    ok &= expect_int("step open C028 panel", p->stepTrace[1],
                     DM1_PC34_CRR_SW_STEP_OPEN_C028_PANEL,
                     spec->reviveAnchor);
    ok &= expect_int("step queue close and rotation", p->stepTrace[2],
                     DM1_PC34_CRR_SW_STEP_QUEUE_C028_CLOSE_AND_ROTATION,
                     spec->f0359QueueAnchor);
    ok &= expect_int("open result", p->openResult, 1,
                     spec->f0333OpenAnchor);
    ok &= expect_int("open champion", p->openChampionBefore,
                     spec->nonLeaderOpenIndex,
                     spec->f0333OpenAnchor);
    ok &= expect_int("open chest before", p->openChestThingBefore,
                     DM1_PC34_CRR_SW_CHEST_THING,
                     spec->f0333OpenAnchor);
    ok &= expect_int("panel chest before C028", p->panelChestBeforeC028,
                     DM1_PC34_PANEL_CHEST,
                     spec->f0333OpenAnchor);
    ok &= expect_int("C028 panel live", p->c028PanelLiveBeforeQueue, 1,
                     spec->reviveAnchor);
    ok &= expect_int("C028 route", p->c028PanelRoute,
                     DM1_PC34_CRR_SW_C028_ROUTE,
                     spec->reviveAnchor);
    ok &= expect_int("C029 route", p->c029PanelRoute,
                     DM1_PC34_CRR_SW_C029_ROUTE,
                     spec->panelAnchor);
    ok &= expect_int("candidate ordinal", p->candidateOrdinalBeforeQueue, 1,
                     spec->reviveAnchor);
    ok &= expect_int("candidate command", p->candidateCommandBeforeQueue,
                     DM1_PC34_CRR_SW_C160_RESURRECT,
                     spec->reviveAnchor);
    ok &= expect_int("leader before queue", p->leaderBeforeQueue,
                     spec->oldLeaderIndex,
                     spec->f0297HandAnchor);
    ok &= expect_int("hand before queue", p->handTypeBeforeQueue,
                     DM1_PC34_CRR_SW_HAND_ITEM,
                     spec->f0297HandAnchor);
    ok &= expect_int("hand charges before queue", p->handChargesBeforeQueue,
                     DM1_PC34_CRR_SW_HAND_CHARGES,
                     spec->f0297HandAnchor);
    ok &= expect_int("hand quantity before queue", p->handQuantityBeforeQueue,
                     DM1_PC34_CRR_SW_HAND_QUANTITY,
                     spec->f0297HandAnchor);
    ok &= expect_int("target type before queue", p->targetSlotTypeBeforeQueue,
                     DM1_PC34_CRR_SW_TARGET_SLOT_ITEM,
                     spec->defsAnchor);
    ok &= expect_int("target charges before queue",
                     p->targetSlotChargesBeforeQueue,
                     DM1_PC34_CRR_SW_TARGET_SLOT_CHARGES,
                     spec->defsAnchor);
    ok &= expect_int("target quantity before queue",
                     p->targetSlotQuantityBeforeQueue,
                     DM1_PC34_CRR_SW_TARGET_SLOT_QUANTITY,
                     spec->defsAnchor);
    ok &= expect_int("C537-C544 visible before",
                     p->c537ToC544VisibleBefore, 1,
                     spec->f0333OpenAnchor);
    ok &= expect_int("C028 close queued", p->c028CloseQueued, 1,
                     spec->f0359QueueAnchor);
    ok &= expect_int("rotation queued", p->rotationQueued, 1,
                     spec->f0359QueueAnchor);
    ok &= expect_int("queued old leader", p->queuedOldLeader,
                     spec->oldLeaderIndex,
                     spec->f0359QueueAnchor);
    ok &= expect_int("queued new leader", p->queuedNewLeader,
                     spec->newLeaderIndex,
                     spec->f0359QueueAnchor);
    ok &= expect_int("queued open champion", p->queuedOpenChampion,
                     spec->nonLeaderOpenIndex,
                     spec->f0333OpenAnchor);
    ok &= expect_int("queue depth after queue", p->commandQueueDepthAfterQueue,
                     2, spec->f0359QueueAnchor);
    ok &= expect_int("queued zone", p->queuedZone,
                     DM1_PC34_CRR_SW_TARGET_ZONE,
                     spec->defsAnchor);
    ok &= expect_int("queued slot box", p->queuedSlotBox,
                     DM1_PC34_CRR_SW_TARGET_SLOT_BOX,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("queued pc34 slot", p->queuedPc34Slot,
                     DM1_PC34_CRR_SW_TARGET_PC34_SLOT,
                     spec->defsAnchor);
    ok &= expect_int("queued command", p->queuedCommand,
                     DM1_PC34_CRR_SW_TARGET_COMMAND,
                     spec->f0361WheelQueueAnchor);
    return ok;
}

static int test_visible_before(
    const DM1_V1_ChestResurrectRotationScrollWheelProbePc34* p,
    const DM1_V1_ChestResurrectRotationScrollWheelSpecPc34* spec)
{
    int i;
    int ok = 1;

    for (i = 0; i < DM1_PC34_CRR_SW_SLOT_COUNT; ++i) {
        char label[96];

        snprintf(label, sizeof(label), "before C%d type", 537 + i);
        ok &= expect_int(label, p->visibleTypesBefore[i],
                         expected_initial_type(i),
                         spec->f0333OpenAnchor);
        snprintf(label, sizeof(label), "before C%d charges", 537 + i);
        ok &= expect_int(label, p->visibleChargesBefore[i],
                         expected_initial_charges(i),
                         spec->f0333OpenAnchor);
        snprintf(label, sizeof(label), "before C%d quantity", 537 + i);
        ok &= expect_int(label, p->visibleQuantitiesBefore[i],
                         expected_initial_quantity(i),
                         spec->f0333OpenAnchor);
    }
    return ok;
}

static int test_rejects(
    const DM1_V1_ChestResurrectRotationScrollWheelProbePc34* p,
    const DM1_V1_ChestResurrectRotationScrollWheelSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("step reject C028 live", p->stepTrace[3],
                     DM1_PC34_CRR_SW_STEP_REJECT_WHEEL_C028_LIVE,
                     spec->reviveAnchor);
    ok &= expect_int("C028 live reject attempted",
                     p->c028LiveRejectAttempted, 1,
                     spec->f0361WheelQueueAnchor);
    ok &= expect_int("C028 live wheel rejected", p->c028LiveRejectResult, 0,
                     spec->reviveAnchor);
    ok &= expect_int("C028 live reject reason", p->c028LiveRejectReason, 1,
                     spec->reviveAnchor);
    ok &= expect_int("F0077 after C028 reject",
                     p->f0077ObservedAfterC028Reject, 1,
                     spec->f0077EnableAnchor);
    ok &= expect_int("F0078 after C028 reject",
                     p->f0078ObservedAfterC028Reject, 1,
                     spec->f0078DisableAnchor);
    ok &= expect_int("mouse depth after C028 reject",
                     p->mouseDepthAfterC028Reject, 0,
                     spec->f0078DisableAnchor);
    ok &= expect_int("queue depth after C028 reject",
                     p->commandQueueDepthAfterC028Reject, 2,
                     spec->f0380DrainAnchor);
    ok &= expect_int("G0426 after C028 reject",
                     p->openChestThingAfterC028Reject,
                     DM1_PC34_CRR_SW_CHEST_THING,
                     spec->f0333OpenAnchor);
    ok &= expect_int("panel after C028 reject", p->panelAfterC028Reject,
                     DM1_PC34_PANEL_RESURRECT_REINCARNATE,
                     spec->panelAnchor);
    ok &= expect_int("target after C028 reject",
                     p->targetSlotTypeAfterC028Reject,
                     DM1_PC34_CRR_SW_TARGET_SLOT_ITEM,
                     spec->defsAnchor);
    ok &= expect_int("hand after C028 reject", p->handTypeAfterC028Reject,
                     DM1_PC34_CRR_SW_HAND_ITEM,
                     spec->f0297HandAnchor);
    ok &= expect_int("chain stable after C028 reject",
                     p->chainStableAfterC028Reject, 1,
                     spec->defsAnchor);
    ok &= expect_int("G0426 stable after C028 reject",
                     p->g0426StableAfterC028Reject, 1,
                     spec->f0333OpenAnchor);

    ok &= expect_int("step C028 close", p->stepTrace[4],
                     DM1_PC34_CRR_SW_STEP_DRAIN_C028_CLOSE,
                     spec->reviveAnchor);
    ok &= expect_int("C028 close drained", p->c028CloseDrained, 1,
                     spec->reviveAnchor);
    ok &= expect_int("C028 panel closed", p->c028PanelLiveAfterClose, 0,
                     spec->reviveAnchor);
    ok &= expect_int("candidate clear", p->candidateOrdinalAfterClose, 0,
                     spec->reviveAnchor);
    ok &= expect_int("panel after close", p->panelAfterC028Close,
                     DM1_PC34_PANEL_CHEST,
                     spec->panelAnchor);
    ok &= expect_int("queue depth after close", p->commandQueueDepthAfterC028Close,
                     1, spec->f0380DrainAnchor);
    ok &= expect_int("rotation still queued after close",
                     p->rotationStillQueuedAfterC028Close, 1,
                     spec->f0380DrainAnchor);

    ok &= expect_int("step reject rotation queued", p->stepTrace[5],
                     DM1_PC34_CRR_SW_STEP_REJECT_WHEEL_ROTATION_QUEUED,
                     spec->f0380DrainAnchor);
    ok &= expect_int("rotation queued reject attempted",
                     p->rotationQueuedRejectAttempted, 1,
                     spec->f0361WheelQueueAnchor);
    ok &= expect_int("rotation queued wheel rejected",
                     p->rotationQueuedRejectResult, 0,
                     spec->f0380DrainAnchor);
    ok &= expect_int("rotation queued reject reason",
                     p->rotationQueuedRejectReason, 2,
                     spec->f0380DrainAnchor);
    ok &= expect_int("queue depth after rotation reject",
                     p->commandQueueDepthAfterRotationReject, 1,
                     spec->f0380DrainAnchor);
    ok &= expect_int("G0426 after rotation reject",
                     p->openChestThingAfterRotationReject,
                     DM1_PC34_CRR_SW_CHEST_THING,
                     spec->f0333OpenAnchor);
    ok &= expect_int("target after rotation reject",
                     p->targetSlotTypeAfterRotationReject,
                     DM1_PC34_CRR_SW_TARGET_SLOT_ITEM,
                     spec->defsAnchor);
    ok &= expect_int("hand after rotation reject",
                     p->handTypeAfterRotationReject,
                     DM1_PC34_CRR_SW_HAND_ITEM,
                     spec->f0297HandAnchor);
    ok &= expect_int("chain stable after rotation reject",
                     p->chainStableAfterRotationReject, 1,
                     spec->defsAnchor);
    ok &= expect_int("G0426 stable after rotation reject",
                     p->g0426StableAfterRotationReject, 1,
                     spec->f0333OpenAnchor);
    return ok;
}

static int test_accept_after_drains(
    const DM1_V1_ChestResurrectRotationScrollWheelProbePc34* p,
    const DM1_V1_ChestResurrectRotationScrollWheelSpecPc34* spec)
{
    int i;
    int ok = 1;

    ok &= expect_int("step drain rotation", p->stepTrace[6],
                     DM1_PC34_CRR_SW_STEP_DRAIN_ROTATION,
                     spec->f0380DrainAnchor);
    ok &= expect_int("rotation drained", p->rotationDrained, 1,
                     spec->f0380DrainAnchor);
    ok &= expect_int("leader after rotation", p->leaderAfterRotationDrain,
                     spec->newLeaderIndex,
                     spec->f0380DrainAnchor);
    ok &= expect_int("open champion after rotation",
                     p->openChampionAfterRotationDrain,
                     spec->nonLeaderOpenIndex,
                     spec->f0333OpenAnchor);
    ok &= expect_int("queue depth after rotation",
                     p->commandQueueDepthAfterRotationDrain, 0,
                     spec->f0380DrainAnchor);
    ok &= expect_int("hand after rotation", p->handTypeAfterRotationDrain,
                     DM1_PC34_CRR_SW_HAND_ITEM,
                     spec->f0297HandAnchor);
    ok &= expect_int("panel after rotation", p->panelAfterRotationDrain,
                     DM1_PC34_PANEL_CHEST,
                     spec->panelAnchor);
    ok &= expect_int("G0426 after rotation",
                     p->openChestThingAfterRotationDrain,
                     DM1_PC34_CRR_SW_CHEST_THING,
                     spec->f0333OpenAnchor);

    ok &= expect_int("step accept wheel", p->stepTrace[7],
                     DM1_PC34_CRR_SW_STEP_ACCEPT_WHEEL_C540_SWAP,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("wheel accepted after close+rotation",
                     p->wheelAcceptedAfterCloseAndRotation, 1,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("F0302 dispatch count", p->f0302DispatchCountAfterAccept,
                     1, spec->f0302DispatchAnchor);
    ok &= expect_int("queue depth after accept", p->commandQueueDepthAfterAccept,
                     0, spec->f0380DrainAnchor);
    ok &= expect_int("target type after accept", p->targetSlotTypeAfterAccept,
                     DM1_PC34_CRR_SW_HAND_ITEM,
                     spec->f0301SlotAnchor);
    ok &= expect_int("target charges after accept",
                     p->targetSlotChargesAfterAccept,
                     DM1_PC34_CRR_SW_HAND_CHARGES,
                     spec->f0301SlotAnchor);
    ok &= expect_int("target quantity after accept",
                     p->targetSlotQuantityAfterAccept,
                     DM1_PC34_CRR_SW_HAND_QUANTITY,
                     spec->f0301SlotAnchor);
    ok &= expect_int("hand type after accept", p->handTypeAfterAccept,
                     DM1_PC34_CRR_SW_TARGET_SLOT_ITEM,
                     spec->f0297HandAnchor);
    ok &= expect_int("hand charges after accept", p->handChargesAfterAccept,
                     DM1_PC34_CRR_SW_TARGET_SLOT_CHARGES,
                     spec->f0297HandAnchor);
    ok &= expect_int("hand quantity after accept", p->handQuantityAfterAccept,
                     DM1_PC34_CRR_SW_TARGET_SLOT_QUANTITY,
                     spec->f0297HandAnchor);
    ok &= expect_int("G0426 after accept", p->openChestThingAfterAccept,
                     DM1_PC34_CRR_SW_CHEST_THING,
                     spec->f0333OpenAnchor);
    ok &= expect_int("C537-C544 coherent after accept",
                     p->c537ToC544ChainCoherentAfterAccept, 1,
                     spec->defsAnchor);
    ok &= expect_int("close then rotation then swap",
                     p->c028CloseThenRotationThenSwap, 1,
                     spec->f0380DrainAnchor);
    ok &= expect_int("F0077/F0078 balanced", p->f0077F0078Balanced, 1,
                     spec->f0078DisableAnchor);

    for (i = 0; i < DM1_PC34_CRR_SW_SLOT_COUNT; ++i) {
        char label[96];

        snprintf(label, sizeof(label), "after C%d type", 537 + i);
        ok &= expect_int(label, p->visibleTypesAfterAccept[i],
                         expected_after_type(i),
                         spec->defsAnchor);
        snprintf(label, sizeof(label), "after C%d charges", 537 + i);
        ok &= expect_int(label, p->visibleChargesAfterAccept[i],
                         expected_after_charges(i),
                         spec->defsAnchor);
        snprintf(label, sizeof(label), "after C%d quantity", 537 + i);
        ok &= expect_int(label, p->visibleQuantitiesAfterAccept[i],
                         expected_after_quantity(i),
                         spec->defsAnchor);
    }
    return ok;
}

static int test_non_overlap(
    const DM1_V1_ChestResurrectRotationScrollWheelProbePc34* p,
    const DM1_V1_ChestResurrectRotationScrollWheelSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("nonoverlap pass768 close race", p->noPass768CloseRace,
                     1, spec->nonDuplicateMarker);
    ok &= expect_int("nonoverlap pass771 drop during rotation",
                     p->noPass771DropDuringRotation, 1,
                     spec->nonDuplicateMarker);
    ok &= expect_int("nonoverlap pass772 food water accept",
                     p->noPass772FoodWaterAccept, 1,
                     spec->nonDuplicateMarker);
    ok &= expect_int("nonoverlap C040 priority",
                     p->noC040PanelPriorityRotationClick, 1,
                     spec->nonDuplicateMarker);
    ok &= expect_int("nonoverlap drop during rotation nonleader open",
                     p->noDropDuringRotationNonLeaderOpen, 1,
                     spec->nonDuplicateMarker);
    return ok;
}

int main(void)
{
    const DM1_V1_ChestResurrectRotationScrollWheelSpecPc34* spec =
        dm1_v1_chest_resurrect_rotation_scroll_wheel_spec_pc34();
    int ok = 1;

    printf("probe=dm1_v1_chest_resurrect_rotation_scroll_wheel_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_chest_resurrect_rotation_scroll_wheel_source_evidence_pc34());

    ok &= expect_int(
        "probe run",
        dm1_v1_chest_resurrect_rotation_scroll_wheel_run_pc34(&g_probe),
        1, spec->f0380DrainAnchor);
    ok &= test_source_and_spec(spec);
    ok &= test_setup_and_queue(&g_probe, spec);
    ok &= test_visible_before(&g_probe, spec);
    ok &= test_rejects(&g_probe, spec);
    ok &= test_accept_after_drains(&g_probe, spec);
    ok &= test_non_overlap(&g_probe, spec);
    ok &= expect_u32("deterministic hash",
                     g_probe.deterministicHash,
                     0xB2C57753u,
                     spec->f0380DrainAnchor);

    printf("deterministicHash=0x%08X\n",
           (unsigned)g_probe.deterministicHash);
    printf("assertions=%d failures=%d hash=0x%08X\n",
           g_assertions, g_failures, (unsigned)g_probe.deterministicHash);
    if (!ok || g_failures || g_assertions < 120 || g_assertions > 220) {
        printf("FAIL dm1_v1_chest_resurrect_rotation_scroll_wheel_pc34_compat\n");
        return 1;
    }
    printf("PASS dm1_v1_chest_resurrect_rotation_scroll_wheel_pc34_compat\n");
    printf("assertions=%d failures=0 hash=0x%08X\n",
           g_assertions, (unsigned)g_probe.deterministicHash);
    return 0;
}
