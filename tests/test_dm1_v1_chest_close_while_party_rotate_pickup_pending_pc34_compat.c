#include "firestaff/dm1/v1/chest/close_while_party_rotate_pickup_pending_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static DM1_V1_ChestCloseWhilePartyRotatePickupProbePc34 g_probe;
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
    return DM1_PC34_CLOSE_ROTATE_PICKUP_FIRST_ITEM + index;
}

static int expected_charges(int index)
{
    return DM1_PC34_CLOSE_ROTATE_PICKUP_FIRST_CHARGES + index;
}

static int expected_quantity(int index)
{
    return DM1_PC34_CLOSE_ROTATE_PICKUP_FIRST_QUANTITY + index;
}

static int test_source_and_spec(
    const DM1_V1_ChestCloseWhilePartyRotatePickupSpecPc34* spec)
{
    const char* evidence =
        dm1_v1_chest_close_while_party_rotate_pickup_pending_source_evidence_pc34();
    int ok = 1;

    ok &= expect_contains("evidence F0333", evidence,
                          "CHEST.C F0333:30-76",
                          spec->f0333OpenAnchor);
    ok &= expect_contains("evidence F0334", evidence,
                          "CHEST.C F0334:113-132",
                          spec->f0334CloseAnchor);
    ok &= expect_contains("evidence F0284", evidence,
                          "CHAMPION.C F0284:93-131",
                          spec->f0284RotateAnchor);
    ok &= expect_contains("evidence F0297/F0298", evidence,
                          "CHAMPION.C F0297:243-268/F0298:270-298",
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
    ok &= expect_contains("evidence F0359", evidence,
                          "COMMAND.C F0359:1973-1983",
                          spec->commandF0359Anchor);
    ok &= expect_contains("evidence F0380", evidence,
                          "F0380:2045-2152",
                          spec->commandF0380Anchor);
    ok &= expect_contains("evidence PANEL", evidence,
                          "PANEL.C:7-13",
                          spec->panelAnchor);
    ok &= expect_contains("evidence DEFS", evidence,
                          "DEFS.H:810,1878,3001-3008,3906-3913,5876-5881",
                          spec->defsAnchor);
    ok &= expect_int("spec expected close count",
                     spec->expectedCloseCount, 7,
                     spec->f0334CloseAnchor);
    return ok;
}

static int test_steps(
    const DM1_V1_ChestCloseWhilePartyRotatePickupProbePc34* p,
    const DM1_V1_ChestCloseWhilePartyRotatePickupSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("contract-only flag", p->sourceLockedContractOnly, 1,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("step count", p->stepCount, 7,
                     spec->commandF0380Anchor);
    ok &= expect_int("step open", p->stepTrace[0],
                     DM1_PC34_CLOSE_ROTATE_PICKUP_STEP_OPEN,
                     spec->f0333OpenAnchor);
    ok &= expect_int("step rotate trigger", p->stepTrace[1],
                     DM1_PC34_CLOSE_ROTATE_PICKUP_STEP_ROTATE_TRIGGER,
                     spec->f0284RotateAnchor);
    ok &= expect_int("step queue C537", p->stepTrace[2],
                     DM1_PC34_CLOSE_ROTATE_PICKUP_STEP_QUEUE_C537,
                     spec->commandF0359Anchor);
    ok &= expect_int("step begin pickup", p->stepTrace[3],
                     DM1_PC34_CLOSE_ROTATE_PICKUP_STEP_BEGIN_PICKUP,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("step close G0426", p->stepTrace[4],
                     DM1_PC34_CLOSE_ROTATE_PICKUP_STEP_CLOSE_G0426,
                     spec->f0334CloseAnchor);
    ok &= expect_int("step rotate commit", p->stepTrace[5],
                     DM1_PC34_CLOSE_ROTATE_PICKUP_STEP_ROTATE_COMMIT,
                     spec->f0284RotateAnchor);
    ok &= expect_int("step stale reject", p->stepTrace[6],
                     DM1_PC34_CLOSE_ROTATE_PICKUP_STEP_STALE_PICKUP_REJECT,
                     spec->f0334CloseAnchor);
    return ok;
}

static int test_queue_and_pickup(
    const DM1_V1_ChestCloseWhilePartyRotatePickupProbePc34* p,
    const DM1_V1_ChestCloseWhilePartyRotatePickupSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("open result", p->openResult, 1,
                     spec->f0333OpenAnchor);
    ok &= expect_int("open chest thing before rotate",
                     p->openChestThingBeforeRotate,
                     DM1_PC34_CLOSE_ROTATE_PICKUP_CHEST_THING,
                     spec->f0333OpenAnchor);
    ok &= expect_int("leader before rotate", p->leaderBeforeRotate,
                     DM1_PC34_CLOSE_ROTATE_PICKUP_OLD_LEADER,
                     spec->f0284RotateAnchor);
    ok &= expect_int("leader after trigger", p->leaderAfterTrigger,
                     DM1_PC34_CLOSE_ROTATE_PICKUP_OLD_LEADER,
                     spec->f0284RotateAnchor);
    ok &= expect_int("party direction before", p->partyDirectionBefore, 0,
                     spec->f0284RotateAnchor);
    ok &= expect_int("party direction after trigger",
                     p->partyDirectionAfterTrigger, 1,
                     spec->f0284RotateAnchor);
    ok &= expect_int("queued command C040", p->queuedCommand,
                     DM1_PC34_CLOSE_ROTATE_PICKUP_COMMAND_C040,
                     spec->commandF0359Anchor);
    ok &= expect_int("queued panel M569", p->queuedPanel,
                     DM1_PC34_CLOSE_ROTATE_PICKUP_PANEL_M569,
                     spec->defsAnchor);
    ok &= expect_int("queued zone C537", p->queuedZone,
                     DM1_PC34_CLOSE_ROTATE_PICKUP_PICKED_ZONE,
                     spec->defsAnchor);
    ok &= expect_int("queued last zone C544",
                     DM1_PC34_CLOSE_ROTATE_PICKUP_LAST_ZONE, 544,
                     spec->defsAnchor);
    ok &= expect_int("queued slot box C38", p->queuedSlotBox,
                     DM1_PC34_CLOSE_ROTATE_PICKUP_SLOT_BOX_C38,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("queued pc34 slot C30", p->queuedPc34Slot,
                     DM1_PC34_SLOT_CHEST_1,
                     spec->defsAnchor);
    ok &= expect_int("queued champion", p->queuedAgainstChampion,
                     DM1_PC34_CLOSE_ROTATE_PICKUP_OLD_LEADER,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("queued open chest", p->queuedAgainstOpenChestThing,
                     DM1_PC34_CLOSE_ROTATE_PICKUP_CHEST_THING,
                     spec->f0333OpenAnchor);
    ok &= expect_int("queued type", p->queuedThingType, expected_type(0),
                     spec->f0302DispatchAnchor);
    ok &= expect_int("queued charges", p->queuedThingCharges,
                     expected_charges(0), spec->f0302DispatchAnchor);
    ok &= expect_int("queued quantity", p->queuedThingQuantity,
                     expected_quantity(0), spec->f0297PutAnchor);
    ok &= expect_int("queued weight", p->queuedThingWeight, 9,
                     spec->f0297PutAnchor);
    ok &= expect_int("queued allowed slots", p->queuedThingAllowedSlots,
                     DM1_PC34_ALLOWED_CONTAINER,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("pickup begin result", p->pickupBeginResult, 1,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("picked slot empty before close",
                     p->pickedSlotEmptyBeforeClose, 1,
                     spec->f0300ClearAnchor);
    ok &= expect_int("leader hand type before close",
                     p->leaderHandTypeBeforeClose, expected_type(0),
                     spec->f0297PutAnchor);
    ok &= expect_int("leader hand charges before close",
                     p->leaderHandChargesBeforeClose, expected_charges(0),
                     spec->f0297PutAnchor);
    ok &= expect_int("leader hand quantity before close",
                     p->leaderHandQuantityBeforeClose, expected_quantity(0),
                     spec->f0297PutAnchor);
    ok &= expect_int("leader hand weight before close",
                     p->leaderHandWeightBeforeClose, 9,
                     spec->f0297PutAnchor);
    return ok;
}

static int test_close_and_reject(
    const DM1_V1_ChestCloseWhilePartyRotatePickupProbePc34* p,
    const DM1_V1_ChestCloseWhilePartyRotatePickupSpecPc34* spec)
{
    int i;
    int ok = 1;

    ok &= expect_int("close count", p->closeCount,
                     spec->expectedCloseCount,
                     spec->f0334CloseAnchor);
    ok &= expect_int("open chest after close", p->openChestThingAfterClose,
                     0, spec->f0334CloseAnchor);
    ok &= expect_int("close skipped picked slot",
                     p->closeSkippedPickedSlot, 1,
                     spec->f0334CloseAnchor);
    ok &= expect_int("close compacted tail", p->closeCompactedTail, 1,
                     spec->f0334CloseAnchor);
    ok &= expect_int("total visible after close",
                     p->totalVisibleAfterClose, 7,
                     spec->f0334CloseAnchor);
    for (i = 0; i < DM1_PC34_CLOSE_ROTATE_PICKUP_SLOT_COUNT - 1; ++i) {
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
                     p->closedTypes[DM1_PC34_CLOSE_ROTATE_PICKUP_SLOT_COUNT - 1],
                     0, spec->f0334CloseAnchor);
    ok &= expect_int("closed tail empty charges",
                     p->closedCharges[DM1_PC34_CLOSE_ROTATE_PICKUP_SLOT_COUNT - 1],
                     0, spec->f0334CloseAnchor);
    ok &= expect_int("closed tail empty quantity",
                     p->closedQuantities[DM1_PC34_CLOSE_ROTATE_PICKUP_SLOT_COUNT - 1],
                     0, spec->f0334CloseAnchor);
    ok &= expect_int("leader after commit", p->leaderAfterCommit,
                     DM1_PC34_CLOSE_ROTATE_PICKUP_NEW_LEADER,
                     spec->f0284RotateAnchor);
    ok &= expect_int("party direction after commit",
                     p->partyDirectionAfterCommit, 1,
                     spec->f0284RotateAnchor);
    ok &= expect_int("old leader direction after commit",
                     p->oldLeaderDirectionAfterCommit, 1,
                     spec->f0284RotateAnchor);
    ok &= expect_int("new leader direction after commit",
                     p->newLeaderDirectionAfterCommit, 2,
                     spec->f0284RotateAnchor);
    ok &= expect_int("new leader open chest after commit",
                     p->newLeaderOpenChestThingAfterCommit, 0,
                     spec->f0334CloseAnchor);
    ok &= expect_int("stale pickup result after close",
                     p->stalePickupResultAfterClose, 0,
                     spec->f0334CloseAnchor);
    ok &= expect_int("late pickup rejected against closed G0426",
                     p->latePickupRejectedAgainstClosedG0426, 1,
                     spec->f0334CloseAnchor);
    ok &= expect_int("picked copies in closed chain",
                     p->pickedCopiesInClosedChain, 0,
                     spec->f0334CloseAnchor);
    ok &= expect_int("picked copies including hand",
                     p->pickedCopiesIncludingHand, 1,
                     spec->f0297PutAnchor);
    return ok;
}

int main(void)
{
    const DM1_V1_ChestCloseWhilePartyRotatePickupSpecPc34* spec =
        dm1_v1_chest_close_while_party_rotate_pickup_pending_spec_pc34();
    int ok = 1;

    printf("probe=dm1_v1_chest_close_while_party_rotate_pickup_pending_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_chest_close_while_party_rotate_pickup_pending_source_evidence_pc34());

    ok &= expect_int(
        "probe run",
        dm1_v1_chest_close_while_party_rotate_pickup_pending_run_pc34(
            &g_probe),
        1, spec->f0302DispatchAnchor);
    ok &= test_source_and_spec(spec);
    ok &= test_steps(&g_probe, spec);
    ok &= test_queue_and_pickup(&g_probe, spec);
    ok &= test_close_and_reject(&g_probe, spec);
    ok &= expect_u32("deterministic hash",
                     g_probe.deterministicHash,
                     0x75A86729u,
                     spec->f0334CloseAnchor);

    printf("deterministicHash=0x%08X\n",
           (unsigned)g_probe.deterministicHash);
    printf("assertions=%d failures=%d\n", g_assertions, g_failures);
    if (!ok || g_failures || g_assertions < 70 || g_assertions > 180) {
        printf("FAIL dm1_v1_chest_close_while_party_rotate_pickup_pending_pc34_compat\n");
        return 1;
    }
    printf("PASS dm1_v1_chest_close_while_party_rotate_pickup_pending_pc34_compat assertions=%d failures=0 deterministicHash=0x%08X\n",
           g_assertions, (unsigned)g_probe.deterministicHash);
    return 0;
}
