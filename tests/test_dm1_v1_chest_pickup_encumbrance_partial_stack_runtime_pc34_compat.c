#include "dm1_v1_chest_pickup_encumbrance_partial_stack_runtime_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static int expect_int(const char* label,
                      int got,
                      int want,
                      const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        ++g_failures;
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n",
               label, got, want, redmcsbAnchor);
        return 0;
    }
    printf("PASS %s=%d anchor=%s\n", label, got, redmcsbAnchor);
    return 1;
}

static int expect_contains(const char* label,
                           const char* haystack,
                           const char* needle,
                           const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        ++g_failures;
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (!haystack || !needle || !strstr(haystack, needle)) {
        ++g_failures;
        printf("FAIL %s missing=%s anchor=%s\n",
               label, needle ? needle : "(null)", redmcsbAnchor);
        return 0;
    }
    printf("PASS %s contains=%s anchor=%s\n", label, needle, redmcsbAnchor);
    return 1;
}

static int expect_slot(const char* prefix,
                       const int* types,
                       const int* weights,
                       int index,
                       int wantType,
                       int wantWeight,
                       const char* anchor)
{
    int ok = 1;
    char label[128];

    snprintf(label, sizeof(label), "%s slot %d type", prefix, index);
    ok &= expect_int(label, types[index], wantType, anchor);
    snprintf(label, sizeof(label), "%s slot %d weight", prefix, index);
    ok &= expect_int(label, weights[index], wantWeight, anchor);
    return ok;
}

static int test_source_evidence(void)
{
    const char* evidence =
        dm1_v1_chest_pickup_encumbrance_partial_stack_runtime_source_evidence_pc34();
    const char* f0334 = "ReDMCSB CHEST.C F0334 lines 117-132";
    const char* f0284 = "ReDMCSB CHAMPION.C F0284 lines 93-130";
    const char* f0310 = "ReDMCSB CHAMPION.C F0310 lines 1198-1205";
    const char* f0140 = "ReDMCSB DUNGEON.C F0140 lines 1082-1133";
    int ok = 1;

    ok &= expect_contains("source cites F0333", evidence, "CHEST.C F0333",
                          f0334);
    ok &= expect_contains("source cites F0334", evidence, "F0334 lines 117-132",
                          f0334);
    ok &= expect_contains("source cites F0284", evidence, "F0284 lines 93-130",
                          f0284);
    ok &= expect_contains("source cites F0297", evidence, "F0297", f0310);
    ok &= expect_contains("source cites F0300/F0301", evidence, "F0300/F0301",
                          f0310);
    ok &= expect_contains("source cites F0310", evidence, "F0310 lines 1198-1205",
                          f0310);
    ok &= expect_contains("source cites F0140", evidence, "DUNGEON.C F0140",
                          f0140);
    ok &= expect_contains("source cites WEIGHT absence", evidence, "WEIGHT.C is absent",
                          f0140);
    return ok;
}

static int test_setup(
    const DM1_V1_ChestPickupEncumbrancePartialStackProbePc34* probe)
{
    const char* f0333 = "ReDMCSB CHEST.C F0333 lines 53-76";
    const char* f0284 = "ReDMCSB CHAMPION.C F0284 lines 93-130";
    int ok = 1;

    ok &= expect_int("probe setup", probe->setupResult, 1, f0284);
    ok &= expect_int("champion count", probe->championCount, 2, f0284);
    ok &= expect_int("leader index", probe->leaderIndex,
                     DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_LEADER, f0284);
    ok &= expect_int("target champion index", probe->targetChampionIndex,
                     DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_TARGET, f0284);
    ok &= expect_int("target is non-leader", probe->targetIsNonLeader, 1,
                     f0284);
    ok &= expect_int("capacity N", probe->capacityN,
                     DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_CAPACITY_N,
                     f0284);
    ok &= expect_int("initial carried load", probe->initialCarriedLoad,
                     DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_BASE_LOAD,
                     f0284);
    ok &= expect_int("stack K", probe->stackK,
                     DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_K, f0333);
    ok &= expect_int("open result", probe->openResult, 1, f0333);
    ok &= expect_int("open chest thing", probe->openChestThing,
                     DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_CHEST_THING,
                     f0333);
    ok &= expect_int("initial stack count", probe->initialStackCount, 3,
                     f0333);
    ok &= expect_int("initial bottom type", probe->initialBottomType,
                     DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_BOTTOM, f0333);
    ok &= expect_int("initial top type", probe->initialTopType,
                     DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_TOP, f0333);
    ok &= expect_int("initial visible weight", probe->initialVisibleWeight,
                     72, f0333);
    ok &= expect_int("initial leader hand", probe->initialLeaderHandType,
                     DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_LEADER_HAND,
                     f0284);
    ok &= expect_int("initial target hand empty", probe->initialTargetHandType,
                     0, f0284);
    return ok;
}

static int test_first_pickup(
    const DM1_V1_ChestPickupEncumbrancePartialStackEventPc34* event)
{
    const char* f0334 = "ReDMCSB CHEST.C F0334 lines 117-132";
    const char* f0310 = "ReDMCSB CHAMPION.C F0310 lines 1198-1205";
    const char* f0301 =
        "ReDMCSB CHAMPION.C F0297/F0300/F0301 lines 263-265 and 582-615";
    const char* f0284 = "ReDMCSB CHAMPION.C F0284 lines 93-130";
    int ok = 1;

    ok &= expect_int("first result", event->result, 1, f0301);
    ok &= expect_int("first not blocked", event->blocked, 0, f0310);
    ok &= expect_int("first champion", event->championIndex,
                     DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_TARGET, f0284);
    ok &= expect_int("first storage slot", event->storagePc34Slot,
                     DM1_PC34_SLOT_BACKPACK_LINE1_1, f0301);
    ok &= expect_int("first capacity", event->capacityN, 100, f0310);
    ok &= expect_int("first load before", event->loadBefore, 40, f0310);
    ok &= expect_int("first item weight", event->itemWeight, 45, f0301);
    ok &= expect_int("first candidate load", event->candidateLoad, 85,
                     f0310);
    ok &= expect_int("first load after", event->loadAfter, 85, f0310);
    ok &= expect_int("first stack count before", event->stackCountBefore, 3,
                     f0334);
    ok &= expect_int("first stack count after", event->stackCountAfter, 2,
                     f0334);
    ok &= expect_int("first pickup type", event->pickupType,
                     DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_BOTTOM, f0301);
    ok &= expect_int("first pickup was bottom", event->pickupWasBottom, 1,
                     f0301);
    ok &= expect_int("first top before", event->topTypeBefore,
                     DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_TOP, f0334);
    ok &= expect_int("first top after", event->topTypeAfter,
                     DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_TOP, f0334);
    ok &= expect_int("first bottom before", event->bottomTypeBefore,
                     DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_BOTTOM, f0334);
    ok &= expect_int("first bottom after", event->bottomTypeAfter,
                     DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_MIDDLE, f0334);
    ok &= expect_int("first next sees count", event->nextPickupSeesCount, 2,
                     f0334);
    ok &= expect_int("first next sees bottom", event->nextPickupSeesBottomType,
                     DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_MIDDLE, f0334);
    ok &= expect_int("first next sees top", event->nextPickupSeesTopType,
                     DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_TOP, f0334);
    ok &= expect_int("first visible weight before", event->visibleWeightBefore,
                     72, f0334);
    ok &= expect_int("first visible weight after", event->visibleWeightAfter,
                     27, f0334);
    ok &= expect_int("first storage type after", event->storageTypeAfter,
                     DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_BOTTOM, f0301);
    ok &= expect_int("first storage weight after", event->storageWeightAfter,
                     45, f0301);
    ok &= expect_int("first leader hand before", event->leaderHandTypeBefore,
                     DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_LEADER_HAND,
                     f0284);
    ok &= expect_int("first leader hand after", event->leaderHandTypeAfter,
                     DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_LEADER_HAND,
                     f0284);
    ok &= expect_slot("first before", event->slotTypesBefore,
                      event->slotWeightsBefore, 0,
                      DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_BOTTOM, 45,
                      f0334);
    ok &= expect_slot("first before", event->slotTypesBefore,
                      event->slotWeightsBefore, 1,
                      DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_MIDDLE, 20,
                      f0334);
    ok &= expect_slot("first after", event->slotTypesAfter,
                      event->slotWeightsAfter, 0,
                      DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_MIDDLE, 20,
                      f0334);
    ok &= expect_slot("first after", event->slotTypesAfter,
                      event->slotWeightsAfter, 1,
                      DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_TOP, 7, f0334);
    return ok;
}

static int test_blocked_pickup(
    const DM1_V1_ChestPickupEncumbrancePartialStackEventPc34* event)
{
    const char* f0334 = "ReDMCSB CHEST.C F0334 lines 117-132";
    const char* f0310 = "ReDMCSB CHAMPION.C F0310 lines 1198-1205";
    const char* f0301 =
        "ReDMCSB CHAMPION.C F0297/F0300/F0301 lines 263-265 and 582-615";
    const char* f0284 = "ReDMCSB CHAMPION.C F0284 lines 93-130";
    int ok = 1;

    ok &= expect_int("blocked result", event->result, 0, f0310);
    ok &= expect_int("blocked flag", event->blocked, 1, f0310);
    ok &= expect_int("blocked champion", event->championIndex,
                     DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_TARGET, f0284);
    ok &= expect_int("blocked storage slot", event->storagePc34Slot,
                     DM1_PC34_SLOT_BACKPACK_LINE1_2, f0301);
    ok &= expect_int("blocked capacity", event->capacityN, 100, f0310);
    ok &= expect_int("blocked load before", event->loadBefore, 85, f0310);
    ok &= expect_int("blocked item weight", event->itemWeight, 20, f0301);
    ok &= expect_int("blocked candidate load", event->candidateLoad, 105,
                     f0310);
    ok &= expect_int("blocked load after unchanged", event->loadAfter, 85,
                     f0310);
    ok &= expect_int("blocked stack count before", event->stackCountBefore, 2,
                     f0334);
    ok &= expect_int("blocked stack count after", event->stackCountAfter, 2,
                     f0334);
    ok &= expect_int("blocked pickup type", event->pickupType,
                     DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_MIDDLE, f0301);
    ok &= expect_int("blocked pickup was bottom", event->pickupWasBottom, 1,
                     f0301);
    ok &= expect_int("blocked top before", event->topTypeBefore,
                     DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_TOP, f0334);
    ok &= expect_int("blocked top after", event->topTypeAfter,
                     DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_TOP, f0334);
    ok &= expect_int("blocked bottom before", event->bottomTypeBefore,
                     DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_MIDDLE, f0334);
    ok &= expect_int("blocked bottom after", event->bottomTypeAfter,
                     DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_MIDDLE, f0334);
    ok &= expect_int("blocked next sees count", event->nextPickupSeesCount, 2,
                     f0334);
    ok &= expect_int("blocked next sees bottom", event->nextPickupSeesBottomType,
                     DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_MIDDLE, f0334);
    ok &= expect_int("blocked next sees top", event->nextPickupSeesTopType,
                     DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_TOP, f0334);
    ok &= expect_int("blocked visible weight before", event->visibleWeightBefore,
                     27, f0334);
    ok &= expect_int("blocked visible weight after", event->visibleWeightAfter,
                     27, f0334);
    ok &= expect_int("blocked storage type after", event->storageTypeAfter, 0,
                     f0310);
    ok &= expect_int("blocked storage weight after", event->storageWeightAfter,
                     0, f0310);
    ok &= expect_int("blocked stack unchanged", event->stackUnchangedAfterBlock,
                     1, f0334);
    ok &= expect_int("blocked leader hand before", event->leaderHandTypeBefore,
                     DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_LEADER_HAND,
                     f0284);
    ok &= expect_int("blocked leader hand after", event->leaderHandTypeAfter,
                     DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_LEADER_HAND,
                     f0284);
    ok &= expect_slot("blocked before", event->slotTypesBefore,
                      event->slotWeightsBefore, 0,
                      DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_MIDDLE, 20,
                      f0334);
    ok &= expect_slot("blocked before", event->slotTypesBefore,
                      event->slotWeightsBefore, 1,
                      DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_TOP, 7, f0334);
    ok &= expect_slot("blocked after", event->slotTypesAfter,
                      event->slotWeightsAfter, 0,
                      DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_MIDDLE, 20,
                      f0334);
    ok &= expect_slot("blocked after", event->slotTypesAfter,
                      event->slotWeightsAfter, 1,
                      DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_TOP, 7, f0334);
    return ok;
}

static int test_cancel(
    const DM1_V1_ChestPickupEncumbrancePartialStackProbePc34* probe)
{
    const char* f0334 = "ReDMCSB CHEST.C F0334 lines 117-132";
    const char* f0301 =
        "ReDMCSB CHAMPION.C F0297/F0300/F0301 lines 263-265 and 582-615";
    const char* f0284 = "ReDMCSB CHAMPION.C F0284 lines 93-130";
    int ok = 1;

    ok &= expect_int("first storage after cancel prep",
                     probe->firstStoragePc34SlotTypeAfter,
                     DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_BOTTOM, f0301);
    ok &= expect_int("second storage empty after block",
                     probe->secondStoragePc34SlotTypeAfter, 0, f0301);
    ok &= expect_int("cancel result", probe->cancelResult, 1, f0334);
    ok &= expect_int("cancel closed count", probe->cancelClosedCount, 2,
                     f0334);
    ok &= expect_int("cancel remaining count", probe->cancelRemainingStackCount,
                     2, f0334);
    ok &= expect_int("cancel remaining bottom", probe->cancelRemainingBottomType,
                     DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_MIDDLE, f0334);
    ok &= expect_int("cancel remaining top", probe->cancelRemainingTopType,
                     DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_TOP, f0334);
    ok &= expect_int("cancel remaining visible weight",
                     probe->cancelRemainingVisibleWeight, 27, f0334);
    ok &= expect_int("cancel leader hand type", probe->cancelLeaderHandType,
                     DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_LEADER_HAND,
                     f0284);
    ok &= expect_int("cancel leader hand weight", probe->cancelLeaderHandWeight,
                     6, f0284);
    ok &= expect_int("cancel leader hand preserved",
                     probe->cancelLeaderHandPreserved, 1, f0284);
    ok &= expect_int("cancel target hand type", probe->cancelTargetHandType, 0,
                     f0284);
    ok &= expect_int("cancel target hand preserved",
                     probe->cancelTargetHandPreserved, 1, f0284);
    ok &= expect_int("cancel remaining stack matches",
                     probe->cancelRemainingStackMatches, 1, f0334);
    ok &= expect_int("cancel picked item still stored",
                     probe->cancelPickedItemStillStored, 1, f0301);
    ok &= expect_int("cancel blocked slot still empty",
                     probe->cancelBlockedSlotStillEmpty, 1, f0301);
    ok &= expect_int("cancel closed slot0 type", probe->cancelClosedTypes[0],
                     DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_MIDDLE, f0334);
    ok &= expect_int("cancel closed slot0 weight", probe->cancelClosedWeights[0],
                     20, f0334);
    ok &= expect_int("cancel closed slot1 type", probe->cancelClosedTypes[1],
                     DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_TOP, f0334);
    ok &= expect_int("cancel closed slot1 weight", probe->cancelClosedWeights[1],
                     7, f0334);
    ok &= expect_int("cancel closed slot2 empty", probe->cancelClosedTypes[2],
                     0, f0334);
    ok &= expect_int("assertion budget lower bound",
                     g_assertions >= 80 ? 1 : 0, 1, f0334);
    ok &= expect_int("assertion budget upper bound",
                     g_assertions <= 120 ? 1 : 0, 1, f0334);
    return ok;
}

int main(void)
{
    DM1_V1_ChestPickupEncumbrancePartialStackProbePc34 probe;
    const char* f0334 = "ReDMCSB CHEST.C F0334 lines 117-132";
    int ok = 1;

    printf("probe=dm1_v1_chest_pickup_encumbrance_partial_stack_runtime_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_chest_pickup_encumbrance_partial_stack_runtime_source_evidence_pc34());

    ok &= expect_int(
        "run probe",
        dm1_v1_chest_pickup_encumbrance_partial_stack_runtime_run_pc34(
            &probe),
        1, f0334);
    if (ok) {
        ok &= test_source_evidence();
        ok &= test_setup(&probe);
        ok &= test_first_pickup(&probe.firstPickup);
        ok &= test_blocked_pickup(&probe.blockedPickup);
        ok &= test_cancel(&probe);
    }

    printf("assertionCount=%d failureCount=%d\n", g_assertions, g_failures);
    printf("chestPickupEncumbrancePartialStackRuntimeOk=%d\n",
           ok && g_failures == 0 ? 1 : 0);
    if (ok && g_failures == 0) {
        printf("PASS dm1_v1_chest_pickup_encumbrance_partial_stack_runtime_pc34_compat assertions=%d failures=%d\n",
               g_assertions, g_failures);
        return 0;
    }
    return 1;
}
