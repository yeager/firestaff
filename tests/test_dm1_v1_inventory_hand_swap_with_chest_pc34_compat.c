#include "dm1_v1_inventory_hand_swap_with_chest_pc34_compat.h"
#include "dm1_v1_inventory_chest_incompatible_swap_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static DM1_V1_InventoryHandSwapWithChestProbePc34 g_probe;
static int g_assertions;

static int expect_int(const char* label,
                      int got,
                      int want,
                      const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n",
               label, got, want, redmcsbAnchor);
        return 0;
    }
    printf("PASS %s=%d anchor=%s\n", label, got, redmcsbAnchor);
    return 1;
}

static int expect_str(const char* label,
                      const char* got,
                      const char* want,
                      const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (!got || !want || strcmp(got, want) != 0) {
        printf("FAIL %s got=%s want=%s anchor=%s\n",
               label, got ? got : "(null)", want ? want : "(null)",
               redmcsbAnchor);
        return 0;
    }
    printf("PASS %s=%s anchor=%s\n", label, got, redmcsbAnchor);
    return 1;
}

static int test_spec(void)
{
    const char* f0333 =
        "ReDMCSB CHEST.C F0333 lines 31-67";
    const char* data =
        "ReDMCSB DATA.C lines 320-357,1016-1023";
    const DM1_V1_InventoryHandSwapWithChestSpecPc34* spec =
        dm1_v1_inventory_hand_swap_with_chest_spec_pc34();
    int ok = 1;

    ok &= expect_str("contract marker", spec->contractMarker,
                     "Source-locked contract gate only; not full real-asset chest runtime parity.",
                     f0333);
    ok &= expect_int("probe contract-only marker",
                     g_probe.sourceLockedContractOnly, 1, f0333);
    ok &= expect_int("C09 action hand icon slot",
                     spec->c09SlotBoxInventoryActionHand, 9, f0333);
    ok &= expect_int("C145 chest open icon",
                     spec->c145IconContainerChestOpen, 145, f0333);
    ok &= expect_int("C537 pc34 slot", spec->c537Pc34Slot,
                     DM1_PC34_SLOT_CHEST_1, data);
    ok &= expect_int("C544 pc34 slot", spec->c544Pc34Slot,
                     DM1_PC34_SLOT_CHEST_8, data);
    ok &= expect_int("C30 first chest slot", spec->c30ChestFirstSlot,
                     30, data);
    ok &= expect_int("C37 last chest slot", spec->c37ChestLastSlot,
                     37, data);
    ok &= expect_int("chest slot mask", spec->chestSlotMask,
                     DM1_PC34_ALLOWED_CONTAINER, data);
    ok &= expect_int("action hand descriptor", spec->actionHandIndex,
                     0, f0333);
    ok &= expect_int("stacking not applicable", spec->stackingNotApplicable,
                     1, data);
    return ok;
}

static int test_runtime(void)
{
    const char* f0333 =
        "ReDMCSB CHEST.C F0333 lines 31-67";
    const char* f0334 =
        "ReDMCSB CHEST.C F0334 lines 113-132";
    const char* f0302 =
        "ReDMCSB CHAMPION.C F0297/F0298/F0300/F0301/F0302 lines 250-298,511-584,606-660,677-710";
    int ok = 1;

    ok &= expect_int("main chest opens", g_probe.mainOpenResult, 1, f0333);
    ok &= expect_int("main chest open thing", g_probe.mainOpenThing,
                     DM1_PC34_HAND_SWAP_WITH_CHEST_MAIN_THING, f0333);
    ok &= expect_int("empty hand swap accepted", g_probe.emptySwapResult,
                     1, f0302);
    ok &= expect_int("empty hand starts empty", g_probe.emptyHandBefore,
                     0, f0302);
    ok &= expect_int("empty hand receives chest occupant",
                     g_probe.emptyHandAfter,
                     DM1_PC34_HAND_SWAP_WITH_CHEST_FIRST_ITEM +
                     DM1_PC34_HAND_SWAP_WITH_CHEST_EMPTY_INDEX, f0302);
    ok &= expect_int("empty swap original occupant returned",
                     g_probe.emptyOriginalChestOccupant,
                     DM1_PC34_HAND_SWAP_WITH_CHEST_FIRST_ITEM +
                     DM1_PC34_HAND_SWAP_WITH_CHEST_EMPTY_INDEX, f0302);
    ok &= expect_int("empty swap clears C538", g_probe.emptyChestSlotAfter,
                     0, f0302);
    ok &= expect_int("empty close compacts visible slots",
                     g_probe.emptyClosedCount,
                     DM1_PC34_HAND_SWAP_WITH_CHEST_SLOT_COUNT - 1, f0334);
    ok &= expect_int("empty reopen succeeds", g_probe.emptyReopenResult,
                     1, f0333);
    ok &= expect_int("empty reopen has correct compacted occupant",
                     g_probe.emptyReopenedSlotContainsNextVisible, 1, f0334);
    ok &= expect_int("empty reopened original still in hand",
                     g_probe.emptyReopenedOriginalStillInHand, 1, f0302);

    ok &= expect_int("full hand setup succeeds",
                     g_probe.fullHandSetupResult, 1, f0302);
    ok &= expect_int("full hand swap accepted",
                     g_probe.fullSwapResult, 1, f0302);
    ok &= expect_int("full hand before swap",
                     g_probe.fullHandBefore,
                     DM1_PC34_HAND_SWAP_WITH_CHEST_FULL_HAND_ITEM, f0302);
    ok &= expect_int("full original chest occupant",
                     g_probe.fullOriginalChestOccupant,
                     DM1_PC34_HAND_SWAP_WITH_CHEST_FIRST_ITEM +
                     DM1_PC34_HAND_SWAP_WITH_CHEST_FULL_INDEX + 1, f0302);
    ok &= expect_int("full hand receives original chest occupant",
                     g_probe.fullHandAfter,
                     g_probe.fullOriginalChestOccupant, f0302);
    ok &= expect_int("full chest slot receives previous hand",
                     g_probe.fullChestSlotAfter,
                     DM1_PC34_HAND_SWAP_WITH_CHEST_FULL_HAND_ITEM, f0302);
    ok &= expect_int("full original returned to hand flag",
                     g_probe.fullOriginalReturnedToHand, 1, f0302);
    ok &= expect_int("full previous hand stored flag",
                     g_probe.fullPreviousHandStoredInSlot, 1, f0302);
    ok &= expect_int("full close count",
                     g_probe.fullClosedCount,
                     DM1_PC34_HAND_SWAP_WITH_CHEST_SLOT_COUNT - 1, f0334);
    ok &= expect_int("full reopen succeeds", g_probe.fullReopenResult,
                     1, f0333);
    ok &= expect_int("full reopen keeps previous hand in swap slot",
                     g_probe.fullReopenedSlotContainsPreviousHand, 1, f0334);

    ok &= expect_int("restore swap accepted",
                     g_probe.restoreSwapResult, 1, f0302);
    ok &= expect_int("restore returns previous hand",
                     g_probe.restoreReturnedPreviousHand, 1, f0302);
    ok &= expect_int("restore puts original occupant back",
                     g_probe.restoreSlotOriginalOccupant, 1, f0302);
    ok &= expect_int("restore close count",
                     g_probe.restoreClosedCount,
                     DM1_PC34_HAND_SWAP_WITH_CHEST_SLOT_COUNT - 1, f0334);
    ok &= expect_int("restore reopen succeeds",
                     g_probe.restoreReopenResult, 1, f0333);
    ok &= expect_int("restore reopen keeps original occupant",
                     g_probe.restoreReopenedSlotOriginalOccupant, 1, f0334);

    ok &= expect_int("incompatible swap not accepted",
                     g_probe.incompatibleSwapResult, 0, f0302);
    ok &= expect_int("incompatible rejected flag",
                     g_probe.incompatibleRejected, 1, f0302);
    ok &= expect_int("incompatible hand unchanged",
                     g_probe.incompatibleHandAfter,
                     DM1_PC34_TEST_STAFF_OF_CLAWS_OBJECT_INFO, f0302);
    ok &= expect_int("incompatible slot unchanged",
                     g_probe.incompatibleSlotAfter,
                     g_probe.fullOriginalChestOccupant, f0302);

    ok &= expect_int("other champion opens chest",
                     g_probe.otherChampionOpenResult, 1, f0333);
    ok &= expect_int("other champion swap accepted",
                     g_probe.otherChampionSwapResult, 1, f0302);
    ok &= expect_int("other champion hand receives chest item",
                     g_probe.otherChampionHandAfter,
                     DM1_PC34_HAND_SWAP_WITH_CHEST_OTHER_FIRST_ITEM, f0302);
    ok &= expect_int("other champion chest slot receives hand item",
                     g_probe.otherChampionSlotAfter,
                     DM1_PC34_HAND_SWAP_WITH_CHEST_OTHER_HAND_ITEM, f0302);
    ok &= expect_int("main champion chest slot unaffected",
                     g_probe.mainChampionUnaffectedSlot,
                     g_probe.fullOriginalChestOccupant, f0302);
    ok &= expect_int("non action hand rejected",
                     g_probe.nonActionHandRejected, 1, f0302);
    ok &= expect_int("stale chest rejected",
                     g_probe.staleChestRejected, 1, f0333);
    ok &= expect_int("charges preserved",
                     g_probe.chargesPreserved, 1, f0302);
    ok &= expect_int("stacking invariant not applicable",
                     g_probe.stackingNotApplicable, 1, f0302);
    ok &= expect_int("no duplicate tracked items",
                     g_probe.noDuplicateTrackedItems, 1, f0302);
    return ok;
}

int main(void)
{
    const char* f0333 =
        "ReDMCSB CHEST.C F0333 lines 31-67";
    int ok = 1;

    printf("probe=dm1_v1_inventory_hand_swap_with_chest_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_inventory_hand_swap_with_chest_source_evidence_pc34());

    ok &= expect_int("probe setup",
                     M11_V1_Inventory_HandSwapWithChest_RunPc34(&g_probe),
                     1, f0333);
    if (!ok) {
        printf("assertionCount=%d\n", g_assertions);
        printf("inventoryHandSwapWithChestInvariantOk=0\n");
        return 1;
    }

    ok &= test_spec();
    ok &= test_runtime();
    ok &= expect_int("minimum assertion count",
                     g_assertions >= 50 ? 1 : 0, 1, f0333);

    printf("assertionCount=%d\n", g_assertions);
    printf("inventoryHandSwapWithChestInvariantOk=%d\n", ok ? 1 : 0);
    return ok ? 0 : 1;
}
