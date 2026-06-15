#include "firestaff/dm1/v1/chest/pickup_while_party_rotate_in_progress_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/*
 * ReDMCSB anchors asserted by this asset-free runtime ctest:
 * CHEST.C F0333:30-67 + F0334:113-132, CHAMPION.C F0284:93-131,
 * F0297:243-298, F0298:270-298, F0300:511-515, F0301:606-614,
 * F0302:662-714, PANEL.C F0344/F0345/F0352, COMMAND.C F0359:1985-1990,
 * and DEFS.H:2088 C30/G0425/G0426/G0423/G0305/M070/M516/C537..C544.
 */

static DM1_V1_ChestPickupWhilePartyRotateProbePc34 g_probe;
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
    return DM1_PC34_ROTATE_PICKUP_FIRST_ITEM + index;
}

static int expected_charges(int index)
{
    return DM1_PC34_ROTATE_PICKUP_FIRST_CHARGES + index;
}

static int expected_quantity(int index)
{
    return DM1_PC34_ROTATE_PICKUP_FIRST_QUANTITY + index;
}

static int test_source_and_spec(
    const DM1_V1_ChestPickupWhilePartyRotateSpecPc34* spec)
{
    const char* evidence =
        dm1_v1_chest_pickup_while_party_rotate_in_progress_source_evidence_pc34();
    int ok = 1;

    ok &= expect_contains("evidence F0333", evidence,
                          "CHEST.C F0333:30-67",
                          spec->f0333OpenAnchor);
    ok &= expect_contains("evidence F0334", evidence,
                          "CHEST.C F0334:113-132",
                          spec->f0334CloseAnchor);
    ok &= expect_contains("evidence F0284", evidence,
                          "CHAMPION.C F0284:93-131",
                          spec->f0284RotateAnchor);
    ok &= expect_contains("evidence F0297/F0298", evidence,
                          "CHAMPION.C F0297:243-298/F0298:270-298",
                          spec->f0297PutAnchor);
    ok &= expect_contains("evidence F0300", evidence,
                          "CHAMPION.C F0300:511-515",
                          spec->f0300ClearAnchor);
    ok &= expect_contains("evidence F0301", evidence,
                          "CHAMPION.C F0301:606-614",
                          spec->f0301WriteAnchor);
    ok &= expect_contains("evidence F0302", evidence,
                          "CHAMPION.C F0302:662-714",
                          spec->f0302DispatchAnchor);
    ok &= expect_contains("evidence PANEL", evidence,
                          "PANEL.C F0344/F0345/F0352",
                          spec->panelAnchor);
    ok &= expect_contains("evidence COMMAND", evidence,
                          "COMMAND.C F0359:1985-1990",
                          spec->commandAnchor);
    ok &= expect_contains("evidence DEFS", evidence,
                          "C30/G0425/G0426/G0423/G0305/M070/M516/C537..C544",
                          spec->defsAnchor);

    ok &= expect_int("spec old leader", spec->oldLeaderIndex,
                     DM1_PC34_ROTATE_PICKUP_OLD_LEADER,
                     spec->f0284RotateAnchor);
    ok &= expect_int("spec new leader", spec->newLeaderIndex,
                     DM1_PC34_ROTATE_PICKUP_NEW_LEADER,
                     spec->f0284RotateAnchor);
    ok &= expect_int("spec C537 zone", spec->pickedZone, 537,
                     spec->defsAnchor);
    ok &= expect_int("spec C544 zone", DM1_PC34_ROTATE_PICKUP_LAST_ZONE, 544,
                     spec->defsAnchor);
    ok &= expect_int("spec C30 pc34 slot", spec->pickedPc34Slot,
                     DM1_PC34_SLOT_CHEST_1, spec->defsAnchor);
    ok &= expect_int("spec slot box C38", spec->pickedSlotBox,
                     DM1_PC34_ROTATE_PICKUP_SLOT_BOX_C38,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("spec expected close count", spec->expectedCloseCount, 7,
                     spec->f0334CloseAnchor);
    return ok;
}

static int test_step_order(
    const DM1_V1_ChestPickupWhilePartyRotateProbePc34* p,
    const DM1_V1_ChestPickupWhilePartyRotateSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("contract-only flag", p->sourceLockedContractOnly, 1,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("step count", p->stepCount, 5,
                     spec->commandAnchor);
    ok &= expect_int("step setup", p->stepTrace[0],
                     DM1_PC34_ROTATE_PICKUP_STEP_SETUP_OPEN,
                     spec->f0333OpenAnchor);
    ok &= expect_int("step rotate trigger", p->stepTrace[1],
                     DM1_PC34_ROTATE_PICKUP_STEP_PARTY_ROTATE_TRIGGER,
                     spec->f0284RotateAnchor);
    ok &= expect_int("step pointer queue", p->stepTrace[2],
                     DM1_PC34_ROTATE_PICKUP_STEP_C537_POINTER_QUEUE,
                     spec->panelAnchor);
    ok &= expect_int("step rotate commit", p->stepTrace[3],
                     DM1_PC34_ROTATE_PICKUP_STEP_PARTY_ROTATE_COMMIT,
                     spec->f0284RotateAnchor);
    ok &= expect_int("step close", p->stepTrace[4],
                     DM1_PC34_ROTATE_PICKUP_STEP_CHEST_CLOSE_REWRITE,
                     spec->f0334CloseAnchor);
    ok &= expect_int("queue between trigger and commit",
                     p->queueStepBetweenTriggerAndCommit, 1,
                     spec->f0284RotateAnchor);
    return ok;
}

static int test_pointer_queue(
    const DM1_V1_ChestPickupWhilePartyRotateProbePc34* p,
    const DM1_V1_ChestPickupWhilePartyRotateSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("open result", p->openResult, 1,
                     spec->f0333OpenAnchor);
    ok &= expect_int("open chest thing before rotate",
                     p->openChestThingBeforeRotate,
                     DM1_PC34_ROTATE_PICKUP_CHEST_THING,
                     spec->f0333OpenAnchor);
    ok &= expect_int("leader before rotate", p->leaderBeforeRotate,
                     spec->oldLeaderIndex, spec->f0284RotateAnchor);
    ok &= expect_int("leader after trigger still old",
                     p->leaderAfterTrigger, spec->oldLeaderIndex,
                     spec->f0284RotateAnchor);
    ok &= expect_int("party direction before", p->partyDirectionBefore, 0,
                     spec->f0284RotateAnchor);
    ok &= expect_int("party direction after trigger",
                     p->partyDirectionAfterTrigger, 1,
                     spec->f0284RotateAnchor);

    ok &= expect_int("pointer zone C537", p->pointerZone,
                     DM1_PC34_ROTATE_PICKUP_PICKED_ZONE,
                     spec->panelAnchor);
    ok &= expect_int("pointer panel command M568", p->pointerPanelCommand,
                     DM1_PC34_ROTATE_PICKUP_PANEL_M568,
                     spec->panelAnchor);
    ok &= expect_int("pointer slot box", p->pointerSlotBox,
                     DM1_PC34_ROTATE_PICKUP_SLOT_BOX_C38,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("pointer pc34 slot C30", p->pointerPc34Slot,
                     DM1_PC34_SLOT_CHEST_1,
                     spec->defsAnchor);
    ok &= expect_int("pointer route queued", p->pointerRouteQueued, 1,
                     spec->commandAnchor);
    ok &= expect_int("queued against in-rotate champion",
                     p->queuedAgainstChampion, spec->oldLeaderIndex,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("queued against open chest",
                     p->queuedAgainstOpenChestThing,
                     DM1_PC34_ROTATE_PICKUP_CHEST_THING,
                     spec->f0333OpenAnchor);
    ok &= expect_int("queued type", p->queuedThingType, expected_type(0),
                     spec->f0302DispatchAnchor);
    ok &= expect_int("queued charges", p->queuedThingCharges,
                     expected_charges(0), spec->f0297PutAnchor);
    ok &= expect_int("queued quantity", p->queuedThingQuantity,
                     expected_quantity(0), spec->f0297PutAnchor);
    ok &= expect_int("queued weight", p->queuedThingWeight, 7,
                     spec->f0297PutAnchor);
    ok &= expect_int("queued allowed slots", p->queuedThingAllowedSlots,
                     DM1_PC34_ALLOWED_CONTAINER,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("rotate buffer preserved by queue",
                     p->rotateBufferPreservedByQueue, 1,
                     spec->f0284RotateAnchor);
    return ok;
}

static int test_rotate_buffer(
    const DM1_V1_ChestPickupWhilePartyRotateProbePc34* p,
    const DM1_V1_ChestPickupWhilePartyRotateSpecPc34* spec)
{
    int i;
    int ok = 1;

    for (i = 0; i < DM1_PC34_ROTATE_PICKUP_SLOT_COUNT; ++i) {
        char label[96];

        snprintf(label, sizeof(label), "buffer before type C%d", 537 + i);
        ok &= expect_int(label, p->rotateBufferTypeBeforeQueue[i],
                         expected_type(i), spec->f0333OpenAnchor);
        snprintf(label, sizeof(label), "buffer after queue type C%d", 537 + i);
        ok &= expect_int(label, p->rotateBufferTypeAfterQueue[i],
                         expected_type(i), spec->f0284RotateAnchor);
        snprintf(label, sizeof(label), "buffer before charges C%d", 537 + i);
        ok &= expect_int(label, p->rotateBufferChargesBeforeQueue[i],
                         expected_charges(i), spec->f0302DispatchAnchor);
        snprintf(label, sizeof(label), "buffer after queue charges C%d",
                 537 + i);
        ok &= expect_int(label, p->rotateBufferChargesAfterQueue[i],
                         expected_charges(i), spec->f0284RotateAnchor);
        snprintf(label, sizeof(label), "buffer before quantity C%d", 537 + i);
        ok &= expect_int(label, p->rotateBufferQuantityBeforeQueue[i],
                         expected_quantity(i), spec->f0297PutAnchor);
        snprintf(label, sizeof(label), "buffer after queue quantity C%d",
                 537 + i);
        ok &= expect_int(label, p->rotateBufferQuantityAfterQueue[i],
                         expected_quantity(i), spec->f0284RotateAnchor);
    }
    return ok;
}

static int test_commit(
    const DM1_V1_ChestPickupWhilePartyRotateProbePc34* p,
    const DM1_V1_ChestPickupWhilePartyRotateSpecPc34* spec)
{
    int i;
    int ok = 1;

    ok &= expect_int("commit pickup result", p->commitPickupResult, 1,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("leader after commit", p->leaderAfterCommit,
                     spec->newLeaderIndex, spec->f0284RotateAnchor);
    ok &= expect_int("party direction after commit",
                     p->partyDirectionAfterCommit, 1,
                     spec->f0284RotateAnchor);
    ok &= expect_int("old leader direction after commit",
                     p->oldLeaderDirectionAfterCommit, 1,
                     spec->f0284RotateAnchor);
    ok &= expect_int("new leader direction after commit",
                     p->newLeaderDirectionAfterCommit, 2,
                     spec->f0284RotateAnchor);
    ok &= expect_int("new leader open result", p->newLeaderOpenResult, 1,
                     spec->f0333OpenAnchor);
    ok &= expect_int("new leader open chest thing after commit",
                     p->newLeaderOpenChestThingAfterCommit,
                     DM1_PC34_ROTATE_PICKUP_CHEST_THING,
                     spec->f0333OpenAnchor);
    ok &= expect_int("commit landed against new leader",
                     p->commitLandedAgainstNewLeader, 1,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("new leader hand type after commit",
                     p->newLeaderHandTypeAfterCommit, expected_type(0),
                     spec->f0297PutAnchor);
    ok &= expect_int("new leader hand charges after commit",
                     p->newLeaderHandChargesAfterCommit, expected_charges(0),
                     spec->f0297PutAnchor);
    ok &= expect_int("new leader hand quantity after commit",
                     p->newLeaderHandQuantityAfterCommit, expected_quantity(0),
                     spec->f0297PutAnchor);
    ok &= expect_int("new leader hand weight after commit",
                     p->newLeaderHandWeightAfterCommit, 7,
                     spec->f0297PutAnchor);
    ok &= expect_int("picked metadata preserved",
                     p->pickedMetadataPreserved, 1,
                     spec->f0297PutAnchor);
    ok &= expect_int("picked C537 empty after commit",
                     p->pickedSlotEmptyAfterCommit, 1,
                     spec->f0300ClearAnchor);
    ok &= expect_int("visible count after commit",
                     p->visibleCountAfterCommit, 7,
                     spec->f0300ClearAnchor);
    ok &= expect_int("visible C537 empty",
                     p->visibleTypesAfterCommit[0], 0,
                     spec->f0300ClearAnchor);

    for (i = 1; i < DM1_PC34_ROTATE_PICKUP_SLOT_COUNT; ++i) {
        char label[96];

        snprintf(label, sizeof(label), "visible after commit C%d type",
                 537 + i);
        ok &= expect_int(label, p->visibleTypesAfterCommit[i],
                         expected_type(i), spec->f0302DispatchAnchor);
        snprintf(label, sizeof(label), "visible after commit C%d charges",
                 537 + i);
        ok &= expect_int(label, p->visibleChargesAfterCommit[i],
                         expected_charges(i), spec->f0302DispatchAnchor);
        snprintf(label, sizeof(label), "visible after commit C%d quantity",
                 537 + i);
        ok &= expect_int(label, p->visibleQuantitiesAfterCommit[i],
                         expected_quantity(i), spec->f0302DispatchAnchor);
    }
    return ok;
}

static int test_close(
    const DM1_V1_ChestPickupWhilePartyRotateProbePc34* p,
    const DM1_V1_ChestPickupWhilePartyRotateSpecPc34* spec)
{
    int i;
    int ok = 1;

    ok &= expect_int("close count", p->closeCount,
                     spec->expectedCloseCount,
                     spec->f0334CloseAnchor);
    ok &= expect_int("close against champion", p->closeAgainstChampion,
                     spec->newLeaderIndex, spec->f0334CloseAnchor);
    ok &= expect_int("close against new leader", p->closeAgainstNewLeader, 1,
                     spec->f0334CloseAnchor);
    ok &= expect_int("close rewrote visible chain against new leader",
                     p->closeRewroteVisibleChainAgainstNewLeader, 1,
                     spec->f0334CloseAnchor);
    ok &= expect_int("closed picked thing absent",
                     p->closedPickedThingAbsent, 1,
                     spec->f0334CloseAnchor);
    ok &= expect_int("total picked copies after close",
                     p->totalPickedCopiesAfterClose, 1,
                     spec->f0297PutAnchor);

    for (i = 0; i < DM1_PC34_ROTATE_PICKUP_SLOT_COUNT - 1; ++i) {
        char label[96];
        int sourceIndex = i + 1;

        snprintf(label, sizeof(label), "closed compact type %d", i);
        ok &= expect_int(label, p->closedTypes[i],
                         expected_type(sourceIndex),
                         spec->f0334CloseAnchor);
        snprintf(label, sizeof(label), "closed compact charges %d", i);
        ok &= expect_int(label, p->closedCharges[i],
                         expected_charges(sourceIndex),
                         spec->f0334CloseAnchor);
        snprintf(label, sizeof(label), "closed compact quantity %d", i);
        ok &= expect_int(label, p->closedQuantities[i],
                         expected_quantity(sourceIndex),
                         spec->f0334CloseAnchor);
    }
    ok &= expect_int("closed tail empty type",
                     p->closedTypes[DM1_PC34_ROTATE_PICKUP_SLOT_COUNT - 1],
                     0, spec->f0334CloseAnchor);
    ok &= expect_int("closed tail empty charges",
                     p->closedCharges[DM1_PC34_ROTATE_PICKUP_SLOT_COUNT - 1],
                     0, spec->f0334CloseAnchor);
    ok &= expect_int("closed tail empty quantity",
                     p->closedQuantities[DM1_PC34_ROTATE_PICKUP_SLOT_COUNT - 1],
                     0, spec->f0334CloseAnchor);
    return ok;
}

int main(void)
{
    const DM1_V1_ChestPickupWhilePartyRotateSpecPc34* spec =
        dm1_v1_chest_pickup_while_party_rotate_in_progress_spec_pc34();
    int ok = 1;

    printf("probe=dm1_v1_chest_pickup_while_party_rotate_in_progress_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_chest_pickup_while_party_rotate_in_progress_source_evidence_pc34());

    ok &= expect_int(
        "probe run",
        dm1_v1_chest_pickup_while_party_rotate_in_progress_run_pc34(
            &g_probe),
        1, spec->f0302DispatchAnchor);
    ok &= test_source_and_spec(spec);
    ok &= test_step_order(&g_probe, spec);
    ok &= test_pointer_queue(&g_probe, spec);
    ok &= test_rotate_buffer(&g_probe, spec);
    ok &= test_commit(&g_probe, spec);
    ok &= test_close(&g_probe, spec);
    ok &= expect_u32("deterministic hash",
                     g_probe.deterministicHash,
                     0x939E89BEu,
                     spec->f0334CloseAnchor);

    printf("deterministicHash=0x%08X\n",
           (unsigned)g_probe.deterministicHash);
    printf("assertions=%d failures=%d\n", g_assertions, g_failures);
    if (!ok || g_failures || g_assertions < 130 || g_assertions > 230) {
        printf("FAIL dm1_v1_chest_pickup_while_party_rotate_in_progress_pc34_compat\n");
        return 1;
    }
    printf("PASS dm1_v1_chest_pickup_while_party_rotate_in_progress_pc34_compat assertions=%d failures=0 deterministicHash=0x%08X\n",
           g_assertions, (unsigned)g_probe.deterministicHash);
    return 0;
}
