#include "dm1_v1_chest_pickup_skip_leader_full_hand_to_non_leader_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static DM1_V1_ChestPickupSkipLeaderFullHandToNonLeaderProbePc34 g_probe;
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

static int expected_type_at_initial(int index)
{
    return DM1_PC34_CHEST_SKIP_LEADER_FIRST_ITEM + index;
}

static int expected_weight_at_initial(int index)
{
    return 3 + index;
}

static int expected_source_index_after_compaction(int compactedIndex)
{
    if (compactedIndex >= DM1_PC34_CHEST_SKIP_LEADER_PICKED_INDEX) {
        return compactedIndex + 1;
    }
    return compactedIndex;
}

static int test_source_evidence(
    const DM1_V1_ChestPickupSkipLeaderFullHandToNonLeaderSpecPc34* spec)
{
    const char* evidence =
        dm1_v1_chest_pickup_skip_leader_full_hand_to_non_leader_source_evidence_pc34();
    int ok = 1;

    ok &= expect_contains("sourceEvidence F0333", evidence,
                          "CHEST.C F0333:53-76", spec->chestOpenAnchor);
    ok &= expect_contains("sourceEvidence F0334", evidence,
                          "CHEST.C F0334:117-132", spec->chestCloseAnchor);
    ok &= expect_contains("sourceEvidence F0284", evidence,
                          "CHAMPION.C F0284:117-130",
                          spec->championDirectionAnchor);
    ok &= expect_contains("sourceEvidence F0297", evidence,
                          "CHAMPION.C F0297:243-268",
                          spec->leaderHandAnchor);
    ok &= expect_contains("sourceEvidence F0298", evidence,
                          "F0298:270-298", spec->leaderHandAnchor);
    ok &= expect_contains("sourceEvidence F0302", evidence,
                          "CHAMPION.C F0302:677-710",
                          spec->slotDispatchAnchor);
    ok &= expect_contains("sourceEvidence defs", evidence,
                          "C537..C544", spec->defsAnchor);
    return ok;
}

static int test_spec(
    const DM1_V1_ChestPickupSkipLeaderFullHandToNonLeaderSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("party count", spec->partyCount, 2,
                     spec->slotDispatchAnchor);
    ok &= expect_int("leader index", spec->leaderIndex,
                     DM1_PC34_CHEST_SKIP_LEADER_LEADER_INDEX,
                     spec->leaderHandAnchor);
    ok &= expect_int("target champion index", spec->targetChampionIndex,
                     DM1_PC34_CHEST_SKIP_LEADER_TARGET_INDEX,
                     spec->slotDispatchAnchor);
    ok &= expect_int("picked chest index", spec->pickedChestIndex,
                     DM1_PC34_CHEST_SKIP_LEADER_PICKED_INDEX,
                     spec->chestOpenAnchor);
    ok &= expect_int("picked PC34 slot", spec->pickedPc34Slot,
                     DM1_PC34_CHEST_SKIP_LEADER_PICKED_PC34_SLOT,
                     spec->defsAnchor);
    ok &= expect_int("C537 PC34 slot", spec->c537Pc34Slot,
                     DM1_PC34_SLOT_CHEST_1, spec->defsAnchor);
    ok &= expect_int("C544 PC34 slot", spec->c544Pc34Slot,
                     DM1_PC34_SLOT_CHEST_8, spec->defsAnchor);
    ok &= expect_int("expected closed count", spec->expectedClosedCount, 7,
                     spec->chestCloseAnchor);
    ok &= expect_int("pickup lands in non-leader hand",
                     spec->pickupLandsInNonLeaderHand, 1,
                     spec->slotDispatchAnchor);
    return ok;
}

static int test_runtime_header(
    const DM1_V1_ChestPickupSkipLeaderFullHandToNonLeaderSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("probe run",
                     dm1_v1_chest_pickup_skip_leader_full_hand_to_non_leader_pc34(
                         &g_probe),
                     1, spec->slotDispatchAnchor);
    ok &= expect_int("source locked contract only",
                     g_probe.sourceLockedContractOnly, 0,
                     spec->chestOpenAnchor);
    ok &= expect_int("probe leader index", g_probe.leaderIndex,
                     spec->leaderIndex, spec->leaderHandAnchor);
    ok &= expect_int("probe target champion index",
                     g_probe.targetChampionIndex,
                     spec->targetChampionIndex, spec->slotDispatchAnchor);
    ok &= expect_int("probe picked chest index", g_probe.pickedChestIndex,
                     spec->pickedChestIndex, spec->chestOpenAnchor);
    ok &= expect_int("probe picked pc34 slot", g_probe.pickedPc34Slot,
                     spec->pickedPc34Slot, spec->defsAnchor);
    ok &= expect_int("leader hand setup", g_probe.leaderHandSetupResult, 1,
                     spec->leaderHandAnchor);
    ok &= expect_int("non-leader starts empty",
                     g_probe.nonLeaderHandStartsEmpty, 1,
                     spec->slotDispatchAnchor);
    ok &= expect_int("open result", g_probe.openResult, 1,
                     spec->chestOpenAnchor);
    ok &= expect_int("open thing", g_probe.openThing,
                     DM1_PC34_CHEST_SKIP_LEADER_THING,
                     spec->chestOpenAnchor);
    ok &= expect_int("pickup click result", g_probe.pickupClickResult, 1,
                     spec->slotDispatchAnchor);
    ok &= expect_int("close count", g_probe.closeCount,
                     spec->expectedClosedCount, spec->chestCloseAnchor);
    ok &= expect_int("reopen result", g_probe.reopenResult, 1,
                     spec->chestOpenAnchor);
    ok &= expect_int("reopen thing", g_probe.reopenThing,
                     DM1_PC34_CHEST_SKIP_LEADER_REOPEN_THING,
                     spec->chestOpenAnchor);
    return ok;
}

static int test_hands(
    const DM1_V1_ChestPickupSkipLeaderFullHandToNonLeaderSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("leader hand before type",
                     g_probe.leaderHandBeforeType,
                     DM1_PC34_CHEST_SKIP_LEADER_LEADER_HAND_ITEM,
                     spec->leaderHandAnchor);
    ok &= expect_int("leader hand before weight",
                     g_probe.leaderHandBeforeWeight, 21,
                     spec->leaderHandAnchor);
    ok &= expect_int("leader hand before charges",
                     g_probe.leaderHandBeforeCharges, 4,
                     spec->leaderHandAnchor);
    ok &= expect_int("leader hand before allowed",
                     g_probe.leaderHandBeforeAllowedSlots,
                     DM1_PC34_ALLOWED_CONTAINER,
                     spec->defsAnchor);
    ok &= expect_int("leader hand after type",
                     g_probe.leaderHandAfterType,
                     g_probe.leaderHandBeforeType,
                     spec->leaderHandAnchor);
    ok &= expect_int("leader hand after weight",
                     g_probe.leaderHandAfterWeight,
                     g_probe.leaderHandBeforeWeight,
                     spec->leaderHandAnchor);
    ok &= expect_int("leader hand after charges",
                     g_probe.leaderHandAfterCharges,
                     g_probe.leaderHandBeforeCharges,
                     spec->leaderHandAnchor);
    ok &= expect_int("leader hand after allowed",
                     g_probe.leaderHandAfterAllowedSlots,
                     g_probe.leaderHandBeforeAllowedSlots,
                     spec->defsAnchor);
    ok &= expect_int("leader hand unchanged",
                     g_probe.leaderHandUnchanged, 1,
                     spec->leaderHandAnchor);
    ok &= expect_int("non-leader hand before",
                     g_probe.nonLeaderHandBeforeType, 0,
                     spec->slotDispatchAnchor);
    ok &= expect_int("picked item type",
                     g_probe.pickedItemType,
                     expected_type_at_initial(
                         DM1_PC34_CHEST_SKIP_LEADER_PICKED_INDEX),
                     spec->chestOpenAnchor);
    ok &= expect_int("non-leader hand after type",
                     g_probe.nonLeaderHandAfterType,
                     g_probe.pickedItemType,
                     spec->slotDispatchAnchor);
    ok &= expect_int("non-leader hand after weight",
                     g_probe.nonLeaderHandAfterWeight,
                     g_probe.pickedItemWeight,
                     spec->slotDispatchAnchor);
    ok &= expect_int("non-leader hand after charges",
                     g_probe.nonLeaderHandAfterCharges,
                     g_probe.pickedItemCharges,
                     spec->slotDispatchAnchor);
    ok &= expect_int("non-leader hand after allowed",
                     g_probe.nonLeaderHandAfterAllowedSlots,
                     g_probe.pickedItemAllowedSlots,
                     spec->defsAnchor);
    ok &= expect_int("non-leader received picked item",
                     g_probe.nonLeaderReceivedPickedItem, 1,
                     spec->slotDispatchAnchor);
    ok &= expect_int("leader collision count after pickup",
                     g_probe.leaderCollisionCountAfterPickup, 0,
                     spec->leaderHandAnchor);
    ok &= expect_int("picked item count after pickup",
                     g_probe.pickedItemCountAfterPickup, 1,
                     spec->slotDispatchAnchor);
    ok &= expect_int("leader item count after pickup",
                     g_probe.leaderItemCountAfterPickup, 1,
                     spec->leaderHandAnchor);
    ok &= expect_int("picked item count after reopen",
                     g_probe.pickedItemCountAfterReopen, 1,
                     spec->chestCloseAnchor);
    ok &= expect_int("leader item count after reopen",
                     g_probe.leaderItemCountAfterReopen, 1,
                     spec->leaderHandAnchor);
    return ok;
}

static int test_initial_and_open_after_pickup(
    const DM1_V1_ChestPickupSkipLeaderFullHandToNonLeaderSpecPc34* spec)
{
    int i;
    int ok = 1;

    for (i = 0; i < DM1_PC34_CHEST_SKIP_LEADER_SLOT_COUNT; ++i) {
        char label[96];

        snprintf(label, sizeof(label), "initial C%d type", 537 + i);
        ok &= expect_int(label, g_probe.initialTypes[i],
                         expected_type_at_initial(i),
                         spec->chestOpenAnchor);
        snprintf(label, sizeof(label), "initial C%d weight", 537 + i);
        ok &= expect_int(label, g_probe.initialWeights[i],
                         expected_weight_at_initial(i),
                         spec->chestOpenAnchor);
    }

    ok &= expect_int("picked slot cleared in open view",
                     g_probe.pickedSlotClearedInOpenView, 1,
                     spec->slotDispatchAnchor);
    ok &= expect_int("open view preserves other slots",
                     g_probe.openViewPreservesOtherSlots, 1,
                     spec->slotDispatchAnchor);

    for (i = 0; i < DM1_PC34_CHEST_SKIP_LEADER_SLOT_COUNT; ++i) {
        char label[96];
        int wantType = expected_type_at_initial(i);
        int wantWeight = expected_weight_at_initial(i);

        if (i == DM1_PC34_CHEST_SKIP_LEADER_PICKED_INDEX) {
            wantType = 0;
            wantWeight = 0;
        }
        snprintf(label, sizeof(label), "open after pickup C%d type", 537 + i);
        ok &= expect_int(label, g_probe.openAfterPickupTypes[i],
                         wantType, spec->slotDispatchAnchor);
        snprintf(label, sizeof(label), "open after pickup C%d weight",
                 537 + i);
        ok &= expect_int(label, g_probe.openAfterPickupWeights[i],
                         wantWeight, spec->slotDispatchAnchor);
    }
    return ok;
}

static int test_close_and_reopen(
    const DM1_V1_ChestPickupSkipLeaderFullHandToNonLeaderSpecPc34* spec)
{
    int i;
    int ok = 1;

    ok &= expect_int("close compacts minus picked",
                     g_probe.closeCompactsMinusPicked, 1,
                     spec->chestCloseAnchor);
    ok &= expect_int("reopen compacts minus picked",
                     g_probe.reopenCompactsMinusPicked, 1,
                     spec->chestCloseAnchor);

    for (i = 0; i < spec->expectedClosedCount; ++i) {
        char label[96];
        int sourceIndex = expected_source_index_after_compaction(i);

        snprintf(label, sizeof(label), "closed C%d type", 537 + i);
        ok &= expect_int(label, g_probe.closedTypes[i],
                         expected_type_at_initial(sourceIndex),
                         spec->chestCloseAnchor);
        snprintf(label, sizeof(label), "closed C%d weight", 537 + i);
        ok &= expect_int(label, g_probe.closedWeights[i],
                         expected_weight_at_initial(sourceIndex),
                         spec->chestCloseAnchor);
        snprintf(label, sizeof(label), "reopened C%d type", 537 + i);
        ok &= expect_int(label, g_probe.reopenedTypes[i],
                         expected_type_at_initial(sourceIndex),
                         spec->chestOpenAnchor);
        snprintf(label, sizeof(label), "reopened C%d weight", 537 + i);
        ok &= expect_int(label, g_probe.reopenedWeights[i],
                         expected_weight_at_initial(sourceIndex),
                         spec->chestOpenAnchor);
    }

    ok &= expect_int("closed final slot empty",
                     g_probe.closedTypes[spec->expectedClosedCount], 0,
                     spec->chestCloseAnchor);
    ok &= expect_int("closed final weight empty",
                     g_probe.closedWeights[spec->expectedClosedCount], 0,
                     spec->chestCloseAnchor);
    ok &= expect_int("reopened final slot empty",
                     g_probe.reopenedTypes[spec->expectedClosedCount], 0,
                     spec->chestOpenAnchor);
    ok &= expect_int("reopened final weight empty",
                     g_probe.reopenedWeights[spec->expectedClosedCount], 0,
                     spec->chestOpenAnchor);
    return ok;
}

int main(void)
{
    const DM1_V1_ChestPickupSkipLeaderFullHandToNonLeaderSpecPc34* spec =
        dm1_v1_chest_pickup_skip_leader_full_hand_to_non_leader_spec_pc34();
    int ok = 1;

    printf("probe=dm1_v1_chest_pickup_skip_leader_full_hand_to_non_leader_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_chest_pickup_skip_leader_full_hand_to_non_leader_source_evidence_pc34());

    ok &= test_source_evidence(spec);
    ok &= test_spec(spec);
    ok &= test_runtime_header(spec);
    ok &= test_hands(spec);
    ok &= test_initial_and_open_after_pickup(spec);
    ok &= test_close_and_reopen(spec);

    printf("assertions=%d failures=%d\n", g_assertions, g_failures);
    if (!ok || g_failures || g_assertions < 80 || g_assertions > 120) {
        printf("FAIL dm1_v1_chest_pickup_skip_leader_full_hand_to_non_leader_pc34_compat\n");
        return 1;
    }
    printf("PASS dm1_v1_chest_pickup_skip_leader_full_hand_to_non_leader_pc34_compat assertions=%d\n",
           g_assertions);
    return 0;
}
