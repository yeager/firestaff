#include "dm1/dm1_v1_chest_non_leader_hand_occupied_slot_swap_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static DM1_V1_ChestNonLeaderHandSwapProbePc34 g_probe;
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

static int expect_slot_array(const char* label,
                             const int* got,
                             const int* want,
                             const char* anchor)
{
    int i;
    int ok = 1;

    for (i = 0; i < DM1_PC34_CHEST_NON_LEADER_SWAP_SLOT_COUNT; ++i) {
        char slotLabel[96];

        snprintf(slotLabel, sizeof(slotLabel), "%s C%d", label,
                 DM1_PC34_CHEST_NON_LEADER_SWAP_C30_BASE + i);
        ok &= expect_int(slotLabel, got[i], want[i], anchor);
    }
    return ok;
}

static int test_source_evidence(
    const DM1_V1_ChestNonLeaderHandSwapSpecPc34* spec)
{
    const char* evidence =
        dm1_v1_chest_non_leader_hand_occupied_slot_swap_source_evidence_pc34();
    int ok = 1;

    ok &= expect_contains("sourceEvidence F0334", evidence,
                          "CHEST.C F0334:117-132", spec->chestCloseAnchor);
    ok &= expect_contains("sourceEvidence F0297", evidence,
                          "CHAMPION.C F0297:243-298",
                          spec->leaderPutAnchor);
    ok &= expect_contains("sourceEvidence F0298", evidence,
                          "F0298:270-298", spec->leaderRemoveAnchor);
    ok &= expect_contains("sourceEvidence F0300", evidence,
                          "F0300:511-515", spec->slotClearAnchor);
    ok &= expect_contains("sourceEvidence F0301", evidence,
                          "F0301:606-614", spec->slotWriteAnchor);
    ok &= expect_contains("sourceEvidence F0302", evidence,
                          "F0302:662-710", spec->occupiedSwapAnchor);
    ok &= expect_contains("sourceEvidence OBJECT", evidence,
                          "OBJECT.C F0033:147-212",
                          spec->objectIconAnchor);
    ok &= expect_contains("sourceEvidence BLITMASK", evidence,
                          "BLITMASK.C F0133:30-33",
                          spec->blitMaskAnchor);
    ok &= expect_contains("sourceEvidence defs", evidence,
                          "C30/G0425/G0426/G0423/G0305/M070/M516",
                          spec->defsAnchor);
    return ok;
}

static int test_spec(
    const DM1_V1_ChestNonLeaderHandSwapSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("contractOnly", spec->contractOnly, 1,
                     spec->chestCloseAnchor);
    ok &= expect_int("thing none C0xFFFF", spec->thingNone,
                     DM1_PC34_CHEST_NON_LEADER_SWAP_NONE,
                     spec->slotClearAnchor);
    ok &= expect_int("end of list C0xFFFE", spec->endOfList,
                     DM1_PC34_CHEST_NON_LEADER_SWAP_END_OF_LIST,
                     spec->chestCloseAnchor);
    ok &= expect_int("C30 base", spec->c30BaseSlot,
                     DM1_PC34_CHEST_NON_LEADER_SWAP_C30_BASE,
                     spec->defsAnchor);
    ok &= expect_int("leader ordinal", spec->leaderOrdinal, 1,
                     spec->defsAnchor);
    ok &= expect_int("inventory ordinal", spec->inventoryChampionOrdinal, 1,
                     spec->defsAnchor);
    ok &= expect_int("non-leader ordinal", spec->nonLeaderOrdinal, 2,
                     spec->defsAnchor);
    ok &= expect_int("non-leader source slot box",
                     spec->nonLeaderSourceSlotBox, 2,
                     spec->occupiedSwapAnchor);
    ok &= expect_int("target chest index", spec->targetChestIndex,
                     DM1_PC34_CHEST_NON_LEADER_SWAP_TARGET_INDEX,
                     spec->slotWriteAnchor);
    ok &= expect_int("compacted chest index", spec->compactedChestIndex,
                     DM1_PC34_CHEST_NON_LEADER_SWAP_COMPACTED_INDEX,
                     spec->chestCloseAnchor);
    ok &= expect_int("non-leader hand item",
                     spec->nonLeaderHandItem.thing,
                     DM1_PC34_CHEST_NON_LEADER_SWAP_NON_LEADER_ITEM,
                     spec->occupiedSwapAnchor);
    ok &= expect_int("leader hand item", spec->leaderHandItem.thing,
                     DM1_PC34_CHEST_NON_LEADER_SWAP_LEADER_ITEM,
                     spec->leaderPutAnchor);
    ok &= expect_int("non-leader item allowed container",
                     spec->nonLeaderHandItem.allowedSlots, 0x0400,
                     spec->defsAnchor);
    ok &= expect_int("leader item allowed container",
                     spec->leaderHandItem.allowedSlots, 0x0400,
                     spec->defsAnchor);
    return ok;
}

static int test_non_leader_drop(
    const DM1_V1_ChestNonLeaderHandSwapSpecPc34* spec)
{
    const int initialWant[DM1_PC34_CHEST_NON_LEADER_SWAP_SLOT_COUNT] = {
        DM1_PC34_CHEST_NON_LEADER_SWAP_SEED_A,
        DM1_PC34_CHEST_NON_LEADER_SWAP_NONE,
        DM1_PC34_CHEST_NON_LEADER_SWAP_SEED_B,
        DM1_PC34_CHEST_NON_LEADER_SWAP_NONE,
        DM1_PC34_CHEST_NON_LEADER_SWAP_NONE,
        DM1_PC34_CHEST_NON_LEADER_SWAP_NONE,
        DM1_PC34_CHEST_NON_LEADER_SWAP_NONE,
        DM1_PC34_CHEST_NON_LEADER_SWAP_NONE
    };
    int ok = 1;

    ok &= expect_int("source slot box", g_probe.sourceSlotBox, 2,
                     spec->occupiedSwapAnchor);
    ok &= expect_int("resolved champion index",
                     g_probe.championIndexResolved, 1,
                     spec->occupiedSwapAnchor);
    ok &= expect_int("resolved ready hand",
                     g_probe.sourceHandSlotResolved, 0,
                     spec->defsAnchor);
    ok &= expect_int("initial leader pointer empty",
                     g_probe.initialLeaderHand,
                     DM1_PC34_CHEST_NON_LEADER_SWAP_NONE,
                     spec->leaderRemoveAnchor);
    ok &= expect_int("initial non-leader hand occupied",
                     g_probe.initialNonLeaderHand,
                     DM1_PC34_CHEST_NON_LEADER_SWAP_NON_LEADER_ITEM,
                     spec->occupiedSwapAnchor);
    ok &= expect_slot_array("initial G0425",
                            g_probe.initialChestTypes,
                            initialWant, spec->chestCloseAnchor);
    ok &= expect_int("pickup non-leader hand click",
                     g_probe.pickupClickResult, 1,
                     spec->occupiedSwapAnchor);
    ok &= expect_int("leader pointer carries non-leader item",
                     g_probe.leaderHandAfterNonLeaderPickup,
                     DM1_PC34_CHEST_NON_LEADER_SWAP_NON_LEADER_ITEM,
                     spec->leaderPutAnchor);
    ok &= expect_int("non-leader source hand cleared",
                     g_probe.nonLeaderHandAfterPickup,
                     DM1_PC34_CHEST_NON_LEADER_SWAP_NONE,
                     spec->occupiedSwapAnchor);
    ok &= expect_int("C0xFFFF applies to source hand",
                     g_probe.c0ffffClearAppliedToSourceHand, 1,
                     spec->occupiedSwapAnchor);
    ok &= expect_int("empty C34 target accepts non-leader item",
                     g_probe.emptyTargetClickResult, 1,
                     spec->slotWriteAnchor);
    ok &= expect_int("C34 holds non-leader item before close",
                     g_probe.targetChestSlotAfterNonLeaderDrop,
                     DM1_PC34_CHEST_NON_LEADER_SWAP_NON_LEADER_ITEM,
                     spec->slotWriteAnchor);
    ok &= expect_int("leader pointer empty after non-leader drop",
                     g_probe.leaderHandAfterNonLeaderDrop,
                     DM1_PC34_CHEST_NON_LEADER_SWAP_NONE,
                     spec->leaderRemoveAnchor);
    ok &= expect_int("non-leader hand stays empty after drop",
                     g_probe.nonLeaderHandAfterNonLeaderDrop,
                     DM1_PC34_CHEST_NON_LEADER_SWAP_NONE,
                     spec->occupiedSwapAnchor);
    ok &= expect_int("target occupied before close",
                     g_probe.targetStillOccupiedBeforeClose, 1,
                     spec->slotWriteAnchor);
    return ok;
}

static int test_close_rewrite(
    const DM1_V1_ChestNonLeaderHandSwapSpecPc34* spec)
{
    const int closedWant[DM1_PC34_CHEST_NON_LEADER_SWAP_SLOT_COUNT] = {
        DM1_PC34_CHEST_NON_LEADER_SWAP_SEED_A,
        DM1_PC34_CHEST_NON_LEADER_SWAP_SEED_B,
        DM1_PC34_CHEST_NON_LEADER_SWAP_NON_LEADER_ITEM,
        DM1_PC34_CHEST_NON_LEADER_SWAP_NONE,
        DM1_PC34_CHEST_NON_LEADER_SWAP_NONE,
        DM1_PC34_CHEST_NON_LEADER_SWAP_NONE,
        DM1_PC34_CHEST_NON_LEADER_SWAP_NONE,
        DM1_PC34_CHEST_NON_LEADER_SWAP_NONE
    };
    int ok = 1;

    ok &= expect_int("leader pointer before close",
                     g_probe.leaderHandBeforeClose,
                     DM1_PC34_CHEST_NON_LEADER_SWAP_NONE,
                     spec->leaderRemoveAnchor);
    ok &= expect_int("close count", g_probe.closeCount, 3,
                     spec->chestCloseAnchor);
    ok &= expect_int("close container head",
                     g_probe.closeContainerHead,
                     DM1_PC34_CHEST_NON_LEADER_SWAP_SEED_A,
                     spec->chestCloseAnchor);
    ok &= expect_int("close end sentinel",
                     g_probe.closeEndSentinel,
                     DM1_PC34_CHEST_NON_LEADER_SWAP_END_OF_LIST,
                     spec->chestCloseAnchor);
    ok &= expect_slot_array("closed links",
                            g_probe.closedTypes,
                            closedWant, spec->chestCloseAnchor);
    ok &= expect_int("closed contains non-leader item",
                     g_probe.closedContainsNonLeaderItem, 1,
                     spec->chestCloseAnchor);
    ok &= expect_int("closed non-leader compacted index",
                     g_probe.closedNonLeaderItemIndex,
                     DM1_PC34_CHEST_NON_LEADER_SWAP_COMPACTED_INDEX,
                     spec->chestCloseAnchor);
    ok &= expect_int("close preserved leader pointer",
                     g_probe.closePreservedLeaderHandPointer, 1,
                     spec->chestCloseAnchor);
    ok &= expect_int("target cleared by close rewrite only",
                     g_probe.targetClearedOnlyByCloseRewrite, 1,
                     spec->chestCloseAnchor);
    ok &= expect_int("reopen result", g_probe.reopenResult, 1,
                     spec->chestCloseAnchor);
    ok &= expect_slot_array("reopened G0425",
                            g_probe.reopenedTypes,
                            closedWant, spec->chestCloseAnchor);
    ok &= expect_int("reopened non-leader item index",
                     g_probe.reopenedNonLeaderItemIndex,
                     DM1_PC34_CHEST_NON_LEADER_SWAP_COMPACTED_INDEX,
                     spec->chestCloseAnchor);
    return ok;
}

static int test_occupied_swap(
    const DM1_V1_ChestNonLeaderHandSwapSpecPc34* spec)
{
    const int finalWant[DM1_PC34_CHEST_NON_LEADER_SWAP_SLOT_COUNT] = {
        DM1_PC34_CHEST_NON_LEADER_SWAP_SEED_A,
        DM1_PC34_CHEST_NON_LEADER_SWAP_SEED_B,
        DM1_PC34_CHEST_NON_LEADER_SWAP_LEADER_ITEM,
        DM1_PC34_CHEST_NON_LEADER_SWAP_NONE,
        DM1_PC34_CHEST_NON_LEADER_SWAP_NONE,
        DM1_PC34_CHEST_NON_LEADER_SWAP_NONE,
        DM1_PC34_CHEST_NON_LEADER_SWAP_NONE,
        DM1_PC34_CHEST_NON_LEADER_SWAP_NONE
    };
    int ok = 1;

    ok &= expect_int("leader hand before occupied swap",
                     g_probe.leaderHandBeforeOccupiedSwap,
                     DM1_PC34_CHEST_NON_LEADER_SWAP_LEADER_ITEM,
                     spec->leaderPutAnchor);
    ok &= expect_int("non-leader hand before occupied swap",
                     g_probe.nonLeaderHandBeforeOccupiedSwap,
                     DM1_PC34_CHEST_NON_LEADER_SWAP_NONE,
                     spec->occupiedSwapAnchor);
    ok &= expect_int("occupied C32 swap click",
                     g_probe.occupiedSwapClickResult, 1,
                     spec->occupiedSwapAnchor);
    ok &= expect_int("C30+ clear slot index",
                     g_probe.c30ClearSlotIndex,
                     DM1_PC34_CHEST_NON_LEADER_SWAP_COMPACTED_INDEX,
                     spec->slotClearAnchor);
    ok &= expect_int("C30+ value after clear",
                     g_probe.c30ValueAfterClear,
                     DM1_PC34_CHEST_NON_LEADER_SWAP_NONE,
                     spec->slotClearAnchor);
    ok &= expect_int("C30+ value after write",
                     g_probe.c30ValueAfterWrite,
                     DM1_PC34_CHEST_NON_LEADER_SWAP_LEADER_ITEM,
                     spec->slotWriteAnchor);
    ok &= expect_int("leader receives non-leader item",
                     g_probe.leaderHandAfterOccupiedSwap,
                     DM1_PC34_CHEST_NON_LEADER_SWAP_NON_LEADER_ITEM,
                     spec->leaderPutAnchor);
    ok &= expect_int("non-leader hand remains empty",
                     g_probe.nonLeaderHandAfterOccupiedSwap,
                     DM1_PC34_CHEST_NON_LEADER_SWAP_NONE,
                     spec->occupiedSwapAnchor);
    ok &= expect_int("swap used leader pointer",
                     g_probe.swapUsedLeaderPointer, 1,
                     spec->occupiedSwapAnchor);
    ok &= expect_int("swap did not restore non-leader hand",
                     g_probe.swapDidNotRestoreNonLeaderHand, 1,
                     spec->occupiedSwapAnchor);
    ok &= expect_int("C0xFFFF clear did not survive target",
                     g_probe.c0ffffClearDidNotSurviveTarget, 1,
                     spec->slotWriteAnchor);
    ok &= expect_int("adjacent C30 preserved",
                     g_probe.adjacentSlot0AfterSwap,
                     DM1_PC34_CHEST_NON_LEADER_SWAP_SEED_A,
                     spec->slotClearAnchor);
    ok &= expect_int("adjacent C31 preserved",
                     g_probe.adjacentSlot1AfterSwap,
                     DM1_PC34_CHEST_NON_LEADER_SWAP_SEED_B,
                     spec->slotClearAnchor);
    ok &= expect_int("adjacent C33 remains none",
                     g_probe.adjacentSlot3AfterSwap,
                     DM1_PC34_CHEST_NON_LEADER_SWAP_NONE,
                     spec->slotClearAnchor);
    ok &= expect_int("final close count", g_probe.finalCloseCount, 3,
                     spec->chestCloseAnchor);
    ok &= expect_slot_array("final closed links",
                            g_probe.finalClosedTypes,
                            finalWant, spec->chestCloseAnchor);
    ok &= expect_int("final closed contains leader item",
                     g_probe.finalClosedContainsLeaderItem, 1,
                     spec->chestCloseAnchor);
    ok &= expect_int("final closed excludes non-leader item",
                     g_probe.finalClosedContainsNonLeaderItem, 0,
                     spec->chestCloseAnchor);
    return ok;
}

int main(void)
{
    const DM1_V1_ChestNonLeaderHandSwapSpecPc34* spec =
        dm1_v1_chest_non_leader_hand_occupied_slot_swap_spec_pc34();
    int ok = 1;

    printf("probe=dm1_v1_chest_non_leader_hand_occupied_slot_swap_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_chest_non_leader_hand_occupied_slot_swap_source_evidence_pc34());
    ok &= expect_int("probe run",
                     dm1_v1_chest_non_leader_hand_occupied_slot_swap_pc34(
                         &g_probe),
                     1, spec->occupiedSwapAnchor);
    ok &= test_source_evidence(spec);
    ok &= test_spec(spec);
    ok &= test_non_leader_drop(spec);
    ok &= test_close_rewrite(spec);
    ok &= test_occupied_swap(spec);

    printf("assertions=%d failures=%d\n", g_assertions, g_failures);
    if (!ok || g_failures || g_assertions < 60) {
        printf("FAIL dm1_v1_chest_non_leader_hand_occupied_slot_swap_pc34_compat\n");
        return 1;
    }
    printf("PASS dm1_v1_chest_non_leader_hand_occupied_slot_swap_pc34_compat assertions=%d\n",
           g_assertions);
    return 0;
}
