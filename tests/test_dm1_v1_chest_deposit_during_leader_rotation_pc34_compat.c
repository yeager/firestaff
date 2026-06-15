#include "firestaff/dm1/v1/chest/dm1_v1_chest_deposit_during_leader_rotation_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/*
 * ReDMCSB anchors asserted by this asset-free runtime ctest:
 * CHEST.C F0333:30-67 + F0334:113-132, CHAMPION.C F0297:243-298,
 * F0298:270-298, F0300:511-515, F0301:606-614, F0302:662-714,
 * COMMAND.C F0359:1985-1990, PANEL.C F0344/F0345/F0354, and DEFS.H
 * C030/C038/C040/C045/C537..C544/G0425/G0426/C540.
 */

static DM1_V1_ChestDepositDuringLeaderRotationProbePc34 g_probe;
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

static int expected_stable_type(int index)
{
    return DM1_PC34_DEPOSIT_ROTATE_FIRST_STABLE_ITEM + index;
}

static int expected_stable_charges(int index)
{
    return DM1_PC34_DEPOSIT_ROTATE_FIRST_CHARGES + index;
}

static int expected_stable_quantity(int index)
{
    return DM1_PC34_DEPOSIT_ROTATE_FIRST_QUANTITY + index;
}

static int test_source_and_spec(
    const DM1_V1_ChestDepositDuringLeaderRotationSpecPc34* spec)
{
    const char* evidence =
        dm1_v1_chest_deposit_during_leader_rotation_source_evidence_pc34();
    int ok = 1;

    ok &= expect_contains("evidence F0333", evidence,
                          "CHEST.C F0333:30-67",
                          spec->f0333OpenAnchor);
    ok &= expect_contains("evidence F0334", evidence,
                          "CHEST.C F0334:113-132",
                          spec->f0334CloseAnchor);
    ok &= expect_contains("evidence F0297", evidence,
                          "CHAMPION.C F0297:243-298",
                          spec->f0297PutAnchor);
    ok &= expect_contains("evidence F0298", evidence,
                          "CHAMPION.C F0298:270-298",
                          spec->f0298RemoveAnchor);
    ok &= expect_contains("evidence F0300", evidence,
                          "CHAMPION.C F0300:511-515",
                          spec->f0300ClearAnchor);
    ok &= expect_contains("evidence F0301", evidence,
                          "CHAMPION.C F0301:606-614",
                          spec->f0301WriteAnchor);
    ok &= expect_contains("evidence F0302", evidence,
                          "CHAMPION.C F0302:662-714",
                          spec->f0302DispatchAnchor);
    ok &= expect_contains("evidence COMMAND", evidence,
                          "COMMAND.C F0359:1985-1990",
                          spec->commandAnchor);
    ok &= expect_contains("evidence PANEL", evidence,
                          "PANEL.C F0344:1493-1561",
                          spec->panelAnchor);
    ok &= expect_contains("evidence DEFS", evidence,
                          "C030/C038/C040/C045/C537..C544/G0425/G0426/C540",
                          spec->defsAnchor);
    ok &= expect_contains("evidence nonduplicate", evidence,
                          "no C040 resurrect pending",
                          spec->nonDuplicateMarker);

    ok &= expect_int("spec old leader", spec->oldLeaderIndex,
                     DM1_PC34_DEPOSIT_ROTATE_OLD_LEADER,
                     spec->commandAnchor);
    ok &= expect_int("spec new leader", spec->newLeaderIndex,
                     DM1_PC34_DEPOSIT_ROTATE_NEW_LEADER,
                     spec->commandAnchor);
    ok &= expect_int("spec target zone C542", spec->targetZone, 542,
                     spec->defsAnchor);
    ok &= expect_int("spec pc34 slot C35", spec->targetPc34Slot,
                     DM1_PC34_SLOT_CHEST_6,
                     spec->defsAnchor);
    ok &= expect_int("spec slot box C43", spec->targetSlotBox,
                     DM1_PC34_DEPOSIT_ROTATE_TARGET_SLOT_BOX,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("spec stable prefix", spec->stablePrefixCount, 5,
                     spec->f0333OpenAnchor);
    ok &= expect_int("spec no C040 pending",
                     spec->resurrectPendingExpected, 0,
                     spec->defsAnchor);
    return ok;
}

static int test_setup_and_step_order(
    const DM1_V1_ChestDepositDuringLeaderRotationProbePc34* p,
    const DM1_V1_ChestDepositDuringLeaderRotationSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("contract-only flag", p->sourceLockedContractOnly, 1,
                     spec->nonDuplicateMarker);
    ok &= expect_int("step count", p->stepCount, 5,
                     spec->commandAnchor);
    ok &= expect_int("step setup", p->stepTrace[0],
                     DM1_PC34_DEPOSIT_ROTATE_STEP_SETUP_OPEN,
                     spec->f0333OpenAnchor);
    ok &= expect_int("step rotation queued", p->stepTrace[1],
                     DM1_PC34_DEPOSIT_ROTATE_STEP_ROTATION_QUEUED,
                     spec->commandAnchor);
    ok &= expect_int("step C542 deposit", p->stepTrace[2],
                     DM1_PC34_DEPOSIT_ROTATE_STEP_C542_DEPOSIT,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("step rotation consumed", p->stepTrace[3],
                     DM1_PC34_DEPOSIT_ROTATE_STEP_ROTATION_CONSUMED,
                     spec->commandAnchor);
    ok &= expect_int("step close rewrite", p->stepTrace[4],
                     DM1_PC34_DEPOSIT_ROTATE_STEP_CLOSE_REWRITE,
                     spec->f0334CloseAnchor);
    ok &= expect_int("open result", p->openResult, 1,
                     spec->f0333OpenAnchor);
    ok &= expect_int("open chest thing before", p->openChestThingBefore,
                     DM1_PC34_DEPOSIT_ROTATE_CHEST_THING,
                     spec->f0333OpenAnchor);
    ok &= expect_int("leader before", p->leaderBefore,
                     spec->oldLeaderIndex,
                     spec->commandAnchor);
    ok &= expect_int("old leader hand empty before",
                     p->oldLeaderHandTypeBefore, 0,
                     spec->f0298RemoveAnchor);
    ok &= expect_int("new leader hand empty before",
                     p->newLeaderHandTypeBefore, 0,
                     spec->f0298RemoveAnchor);
    ok &= expect_int("C040 resurrect pending before",
                     p->c040ResurrectPendingBefore, 0,
                     spec->defsAnchor);
    ok &= expect_int("panel content chest before",
                     p->panelContentBefore,
                     DM1_PC34_DEPOSIT_ROTATE_PANEL_CHEST,
                     spec->panelAnchor);
    return ok;
}

static int test_rotation_and_deposit(
    const DM1_V1_ChestDepositDuringLeaderRotationProbePc34* p,
    const DM1_V1_ChestDepositDuringLeaderRotationSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("rotation queued", p->rotationQueued, 1,
                     spec->commandAnchor);
    ok &= expect_int("queued champion old leader", p->queuedChampion,
                     spec->oldLeaderIndex, spec->commandAnchor);
    ok &= expect_int("queued new leader", p->queuedNewLeader,
                     spec->newLeaderIndex, spec->commandAnchor);
    ok &= expect_int("leader after queue still old", p->leaderAfterQueue,
                     spec->oldLeaderIndex, spec->commandAnchor);
    ok &= expect_int("pointer zone C542", p->pointerZone,
                     DM1_PC34_DEPOSIT_ROTATE_TARGET_ZONE,
                     spec->defsAnchor);
    ok &= expect_int("pointer slot box C43", p->pointerSlotBox,
                     DM1_PC34_DEPOSIT_ROTATE_TARGET_SLOT_BOX,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("pointer pc34 C35", p->pointerPc34Slot,
                     DM1_PC34_DEPOSIT_ROTATE_TARGET_PC34_SLOT,
                     spec->defsAnchor);
    ok &= expect_int("deposit click result", p->depositClickResult, 1,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("deposit against old leader",
                     p->depositAgainstOldLeader, 1,
                     spec->f0297PutAnchor);
    ok &= expect_int("deposit against open chest",
                     p->depositAgainstOpenChest, 1,
                     spec->f0333OpenAnchor);
    ok &= expect_int("old leader hand target after deposit",
                     p->oldLeaderHandTypeAfterDeposit,
                     DM1_PC34_DEPOSIT_ROTATE_TARGET_ITEM,
                     spec->f0297PutAnchor);
    ok &= expect_int("old leader hand charges after deposit",
                     p->oldLeaderHandChargesAfterDeposit,
                     DM1_PC34_DEPOSIT_ROTATE_TARGET_CHARGES,
                     spec->f0297PutAnchor);
    ok &= expect_int("old leader hand quantity after deposit",
                     p->oldLeaderHandQuantityAfterDeposit,
                     DM1_PC34_DEPOSIT_ROTATE_TARGET_QUANTITY,
                     spec->f0297PutAnchor);
    ok &= expect_int("old leader hand weight after deposit",
                     p->oldLeaderHandWeightAfterDeposit,
                     DM1_PC34_DEPOSIT_ROTATE_TARGET_WEIGHT,
                     spec->f0297PutAnchor);
    ok &= expect_int("target type before", p->targetTypeBefore,
                     DM1_PC34_DEPOSIT_ROTATE_TARGET_ITEM,
                     spec->f0333OpenAnchor);
    ok &= expect_int("target charges before", p->targetChargesBefore,
                     DM1_PC34_DEPOSIT_ROTATE_TARGET_CHARGES,
                     spec->f0333OpenAnchor);
    ok &= expect_int("target quantity before", p->targetQuantityBefore,
                     DM1_PC34_DEPOSIT_ROTATE_TARGET_QUANTITY,
                     spec->f0333OpenAnchor);
    ok &= expect_int("target weight before", p->targetWeightBefore,
                     DM1_PC34_DEPOSIT_ROTATE_TARGET_WEIGHT,
                     spec->f0333OpenAnchor);
    ok &= expect_int("target type after deposit empty",
                     p->targetTypeAfterDeposit, 0,
                     spec->f0300ClearAnchor);
    ok &= expect_int("target removed from C542",
                     p->targetRemovedFromC542, 1,
                     spec->f0300ClearAnchor);
    ok &= expect_int("stable prefix after deposit",
                     p->stablePrefixPreservedAfterDeposit, 1,
                     spec->f0300ClearAnchor);
    return ok;
}

static int test_stable_prefix(
    const DM1_V1_ChestDepositDuringLeaderRotationProbePc34* p,
    const DM1_V1_ChestDepositDuringLeaderRotationSpecPc34* spec)
{
    int i;
    int ok = 1;

    for (i = 0; i < DM1_PC34_DEPOSIT_ROTATE_STABLE_PREFIX_COUNT; ++i) {
        char label[96];

        snprintf(label, sizeof(label), "stable before C%d type", 537 + i);
        ok &= expect_int(label, p->stableTypesBefore[i],
                         expected_stable_type(i), spec->f0333OpenAnchor);
        snprintf(label, sizeof(label), "stable before C%d charges", 537 + i);
        ok &= expect_int(label, p->stableChargesBefore[i],
                         expected_stable_charges(i), spec->f0333OpenAnchor);
        snprintf(label, sizeof(label), "stable before C%d quantity", 537 + i);
        ok &= expect_int(label, p->stableQuantitiesBefore[i],
                         expected_stable_quantity(i), spec->f0333OpenAnchor);
        snprintf(label, sizeof(label), "stable after deposit C%d type",
                 537 + i);
        ok &= expect_int(label, p->stableTypesAfterDeposit[i],
                         expected_stable_type(i), spec->f0300ClearAnchor);
        snprintf(label, sizeof(label), "stable after deposit C%d charges",
                 537 + i);
        ok &= expect_int(label, p->stableChargesAfterDeposit[i],
                         expected_stable_charges(i), spec->f0300ClearAnchor);
        snprintf(label, sizeof(label), "stable after deposit C%d quantity",
                 537 + i);
        ok &= expect_int(label, p->stableQuantitiesAfterDeposit[i],
                         expected_stable_quantity(i), spec->f0300ClearAnchor);
    }
    return ok;
}

static int test_rotation_consume_invariants(
    const DM1_V1_ChestDepositDuringLeaderRotationProbePc34* p,
    const DM1_V1_ChestDepositDuringLeaderRotationSpecPc34* spec)
{
    int i;
    int ok = 1;

    ok &= expect_int("rotation consumed", p->rotationConsumed, 1,
                     spec->commandAnchor);
    ok &= expect_int("leader after consume", p->leaderAfterConsume,
                     spec->newLeaderIndex, spec->commandAnchor);
    ok &= expect_int("deposit same tick as rotation",
                     p->depositSameTickAsRotation, 1,
                     spec->commandAnchor);
    ok &= expect_int("new leader open chest after consume",
                     p->newLeaderOpenChestThingAfterConsume,
                     DM1_PC34_DEPOSIT_ROTATE_CHEST_THING,
                     spec->f0333OpenAnchor);
    ok &= expect_int("new leader hand target after consume",
                     p->newLeaderHandTypeAfterConsume,
                     DM1_PC34_DEPOSIT_ROTATE_TARGET_ITEM,
                     spec->f0297PutAnchor);
    ok &= expect_int("new leader hand charges after consume",
                     p->newLeaderHandChargesAfterConsume,
                     DM1_PC34_DEPOSIT_ROTATE_TARGET_CHARGES,
                     spec->f0297PutAnchor);
    ok &= expect_int("new leader hand quantity after consume",
                     p->newLeaderHandQuantityAfterConsume,
                     DM1_PC34_DEPOSIT_ROTATE_TARGET_QUANTITY,
                     spec->f0297PutAnchor);
    ok &= expect_int("new leader hand weight after consume",
                     p->newLeaderHandWeightAfterConsume,
                     DM1_PC34_DEPOSIT_ROTATE_TARGET_WEIGHT,
                     spec->f0297PutAnchor);
    ok &= expect_int("new leader inherited deposit",
                     p->newLeaderInheritedDeposit, 1,
                     spec->f0297PutAnchor);
    ok &= expect_int("old leader hand empty after consume",
                     p->oldLeaderHandTypeAfterConsume, 0,
                     spec->f0298RemoveAnchor);
    ok &= expect_int("no resurrect pending after consume",
                     p->noResurrectPendingAfterConsume, 1,
                     spec->defsAnchor);
    ok &= expect_int("C537-C541 remain in G0425",
                     p->c537ToC541RemainInG0425, 1,
                     spec->defsAnchor);
    ok &= expect_int("C542 empty after consume",
                     p->c542EmptyAfterConsume, 1,
                     spec->f0300ClearAnchor);

    for (i = 0; i < DM1_PC34_DEPOSIT_ROTATE_STABLE_PREFIX_COUNT; ++i) {
        char label[96];

        snprintf(label, sizeof(label), "visible after consume C%d type",
                 537 + i);
        ok &= expect_int(label, p->visibleTypesAfterConsume[i],
                         expected_stable_type(i), spec->defsAnchor);
        snprintf(label, sizeof(label), "visible after consume C%d charges",
                 537 + i);
        ok &= expect_int(label, p->visibleChargesAfterConsume[i],
                         expected_stable_charges(i), spec->defsAnchor);
        snprintf(label, sizeof(label), "visible after consume C%d quantity",
                 537 + i);
        ok &= expect_int(label, p->visibleQuantitiesAfterConsume[i],
                         expected_stable_quantity(i), spec->defsAnchor);
    }
    for (i = DM1_PC34_DEPOSIT_ROTATE_TARGET_SLOT_INDEX;
         i < DM1_PC34_DEPOSIT_ROTATE_SLOT_COUNT; ++i) {
        char label[96];

        snprintf(label, sizeof(label), "visible after consume C%d empty",
                 537 + i);
        ok &= expect_int(label, p->visibleTypesAfterConsume[i], 0,
                         spec->f0300ClearAnchor);
    }
    return ok;
}

static int test_close_rewrite(
    const DM1_V1_ChestDepositDuringLeaderRotationProbePc34* p,
    const DM1_V1_ChestDepositDuringLeaderRotationSpecPc34* spec)
{
    int i;
    int ok = 1;

    ok &= expect_int("close count", p->closeCount,
                     DM1_PC34_DEPOSIT_ROTATE_STABLE_PREFIX_COUNT,
                     spec->f0334CloseAnchor);
    ok &= expect_int("close against new leader", p->closeAgainstNewLeader, 1,
                     spec->f0334CloseAnchor);
    ok &= expect_int("closed stable prefix preserved",
                     p->closedStablePrefixPreserved, 1,
                     spec->f0334CloseAnchor);
    ok &= expect_int("closed target absent", p->closedTargetAbsent, 1,
                     spec->f0334CloseAnchor);
    ok &= expect_int("total target copies after close",
                     p->totalTargetCopiesAfterClose, 1,
                     spec->f0297PutAnchor);

    for (i = 0; i < DM1_PC34_DEPOSIT_ROTATE_STABLE_PREFIX_COUNT; ++i) {
        char label[96];

        snprintf(label, sizeof(label), "closed C%d type", 537 + i);
        ok &= expect_int(label, p->closedTypes[i],
                         expected_stable_type(i), spec->f0334CloseAnchor);
        snprintf(label, sizeof(label), "closed C%d charges", 537 + i);
        ok &= expect_int(label, p->closedCharges[i],
                         expected_stable_charges(i), spec->f0334CloseAnchor);
        snprintf(label, sizeof(label), "closed C%d quantity", 537 + i);
        ok &= expect_int(label, p->closedQuantities[i],
                         expected_stable_quantity(i), spec->f0334CloseAnchor);
    }
    for (i = DM1_PC34_DEPOSIT_ROTATE_STABLE_PREFIX_COUNT;
         i < DM1_PC34_DEPOSIT_ROTATE_SLOT_COUNT; ++i) {
        char label[96];

        snprintf(label, sizeof(label), "closed tail C%d empty", 537 + i);
        ok &= expect_int(label, p->closedTypes[i], 0,
                         spec->f0334CloseAnchor);
    }
    return ok;
}

int main(void)
{
    const DM1_V1_ChestDepositDuringLeaderRotationSpecPc34* spec =
        dm1_v1_chest_deposit_during_leader_rotation_spec_pc34();
    int ok = 1;

    printf("probe=dm1_v1_chest_deposit_during_leader_rotation_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_chest_deposit_during_leader_rotation_source_evidence_pc34());

    ok &= expect_int(
        "probe run",
        dm1_v1_chest_deposit_during_leader_rotation_run_pc34(&g_probe),
        1, spec->f0302DispatchAnchor);
    ok &= test_source_and_spec(spec);
    ok &= test_setup_and_step_order(&g_probe, spec);
    ok &= test_rotation_and_deposit(&g_probe, spec);
    ok &= test_stable_prefix(&g_probe, spec);
    ok &= test_rotation_consume_invariants(&g_probe, spec);
    ok &= test_close_rewrite(&g_probe, spec);
    ok &= expect_u32("deterministic hash",
                     g_probe.deterministicHash,
                     0x29DEF301u,
                     spec->f0334CloseAnchor);

    printf("deterministicHash=0x%08X\n",
           (unsigned)g_probe.deterministicHash);
    printf("assertions=%d failures=%d\n", g_assertions, g_failures);
    if (!ok || g_failures || g_assertions < 110 || g_assertions > 220) {
        printf("FAIL dm1_v1_chest_deposit_during_leader_rotation_pc34_compat\n");
        return 1;
    }
    printf("PASS dm1_v1_chest_deposit_during_leader_rotation_pc34_compat assertions=%d failures=0 deterministicHash=0x%08X\n",
           g_assertions, (unsigned)g_probe.deterministicHash);
    return 0;
}
