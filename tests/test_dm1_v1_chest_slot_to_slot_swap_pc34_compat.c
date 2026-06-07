#include "../src/dm1/dm1_v1_chest_slot_to_slot_swap_pc34_compat.h"

#include <stdio.h>
#include <string.h>

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

static int assert_view(const char* prefix,
                       const int* got,
                       const int* expected,
                       const char* anchor)
{
    int ok = 1;
    int i;

    for (i = 0; i < DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_SLOT_COUNT; ++i) {
        char label[96];

        snprintf(label, sizeof(label), "%s slot %d", prefix, i);
        ok &= expect_int(label, got[i], expected[i], anchor);
    }
    return ok;
}

static int assert_allowed_view(const char* prefix,
                               const int* got,
                               const int* expected,
                               const char* anchor)
{
    int ok = 1;
    int i;

    for (i = 0; i < DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_SLOT_COUNT; ++i) {
        char label[96];

        snprintf(label, sizeof(label), "%s allowed %d", prefix, i);
        ok &= expect_int(label, got[i], expected[i], anchor);
    }
    return ok;
}

static int test_spec(
    const DM1_V1_ChestSlotToSlotSwapSpecPc34* spec)
{
    const char* f0333 =
        "ReDMCSB CHEST.C F0333 lines 53-67";
    const char* dataAllowed =
        "ReDMCSB INVENTORY.C PC3.4 AllowedSlots; DATA.C G0038 lines 1049-1087; DEFS.H C545/C546/C547";
    const char* f0133 =
        "ReDMCSB BLITMASK.C F0133 lines 30-33";
    int ok = 1;

    ok &= expect_str("contract marker", spec->contractMarker,
                     "Source-locked contract gate only; not full real-asset chest runtime parity.",
                     f0333);
    ok &= expect_int("C537 pc34 slot", spec->c537Pc34Slot,
                     DM1_PC34_SLOT_CHEST_1, f0333);
    ok &= expect_int("C544 pc34 slot", spec->c544Pc34Slot,
                     DM1_PC34_SLOT_CHEST_8, f0333);
    ok &= expect_int("source pc34 slot", spec->sourcePc34Slot,
                     DM1_PC34_SLOT_CHEST_1, f0333);
    ok &= expect_int("destination pc34 slot", spec->destinationPc34Slot,
                     DM1_PC34_SLOT_CHEST_5, f0333);
    ok &= expect_int("source index", spec->sourceIndex, 0, f0333);
    ok &= expect_int("destination index", spec->destinationIndex, 4, f0333);
    ok &= expect_int("empty index A", spec->emptyIndexA, 6, f0333);
    ok &= expect_int("empty index B", spec->emptyIndexB, 7, f0333);
    ok &= expect_int("chest slot mask", spec->chestSlotMask,
                     DM1_PC34_ALLOWED_CONTAINER, dataAllowed);
    ok &= expect_int("pouch mask", spec->pouchMask,
                     DM1_PC34_ALLOWED_POUCH, dataAllowed);
    ok &= expect_int("quiver line1 mask", spec->quiverLine1Mask,
                     DM1_PC34_ALLOWED_QUIVER_LINE1, dataAllowed);
    ok &= expect_int("backpack mask", spec->backpackMask,
                     DM1_PC34_ALLOWED_ANY_SLOT, dataAllowed);
    ok &= expect_int("C10 transparency", spec->transparentColorC10, 10,
                     f0133);
    ok &= expect_int("assertion budget", spec->assertionBudget,
                     DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_CONTRACT_ASSERTION_BUDGET,
                     f0333);
    return ok;
}

static int test_probe(
    const DM1_V1_ChestSlotToSlotSwapProbePc34* probe)
{
    const char* f0333Guard =
        "ReDMCSB CHEST.C F0333 lines 30-32";
    const char* f0333 =
        "ReDMCSB CHEST.C F0333 lines 53-67";
    const char* f0334 =
        "ReDMCSB CHEST.C F0334 lines 117-132";
    const char* dataAllowed =
        "ReDMCSB INVENTORY.C PC3.4 AllowedSlots; DATA.C G0038 lines 1049-1087; DEFS.H C545/C546/C547";
    const char* f0032 =
        "ReDMCSB OBJECT.C F0032/F0033 lines 121-212";
    const char* f0294 =
        "ReDMCSB AMMO.C F0294 lines 54-79";
    const char* f0133 =
        "ReDMCSB BLITMASK.C F0133 lines 30-33";
    const char* f0291 =
        "ReDMCSB CHAMDRAW.C F0291/F0296 lines 551-552,1249-1252";
    const int beforeTypes[DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_SLOT_COUNT] = {
        DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_ITEM_A,
        DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_FILLER_1,
        DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_FILLER_2,
        DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_FILLER_3,
        DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_ITEM_B,
        0, 0, 0
    };
    const int duringTypes[DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_SLOT_COUNT] = {
        0,
        DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_FILLER_1,
        DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_FILLER_2,
        DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_FILLER_3,
        DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_ITEM_B,
        0, 0, 0
    };
    const int afterTypes[DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_SLOT_COUNT] = {
        DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_ITEM_B,
        DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_FILLER_1,
        DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_FILLER_2,
        DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_FILLER_3,
        DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_ITEM_A,
        0, 0, 0
    };
    const int beforeAllowed[DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_SLOT_COUNT] = {
        DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_ALLOWED_A,
        DM1_PC34_ALLOWED_CONTAINER,
        DM1_PC34_ALLOWED_CONTAINER,
        DM1_PC34_ALLOWED_CONTAINER,
        DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_ALLOWED_B,
        0, 0, 0
    };
    const int afterAllowed[DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_SLOT_COUNT] = {
        DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_ALLOWED_B,
        DM1_PC34_ALLOWED_CONTAINER,
        DM1_PC34_ALLOWED_CONTAINER,
        DM1_PC34_ALLOWED_CONTAINER,
        DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_ALLOWED_A,
        0, 0, 0
    };
    int ok = 1;

    ok &= expect_int("contract-only marker",
                     probe->sourceLockedContractOnly, 1, f0333);
    ok &= expect_int("no asset parity claim",
                     probe->noAssetParityClaim, 1, f0333);
    ok &= expect_int("open result", probe->openResult, 1, f0333);
    ok &= expect_int("open thing", probe->openThing,
                     DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_CHEST_THING, f0333);
    ok &= expect_int("same-open no-op result",
                     probe->sameOpenNoopResult, 1, f0333Guard);
    ok &= expect_int("same-open thing preserved",
                     probe->sameOpenThing,
                     DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_CHEST_THING,
                     f0333Guard);
    ok &= expect_int("source pc34 slot", probe->sourcePc34Slot,
                     DM1_PC34_SLOT_CHEST_1, f0333);
    ok &= expect_int("destination pc34 slot", probe->destinationPc34Slot,
                     DM1_PC34_SLOT_CHEST_5, f0333);
    ok &= expect_int("source index", probe->sourceIndex, 0, f0333);
    ok &= expect_int("destination index", probe->destinationIndex, 4,
                     f0333);
    ok &= expect_int("empty index A", probe->emptyIndexA, 6, f0333);
    ok &= expect_int("empty index B", probe->emptyIndexB, 7, f0333);
    ok &= expect_int("source chest mask", probe->chestSlotMaskSource,
                     DM1_PC34_ALLOWED_CONTAINER, dataAllowed);
    ok &= expect_int("destination chest mask",
                     probe->chestSlotMaskDestination,
                     DM1_PC34_ALLOWED_CONTAINER, dataAllowed);
    ok &= expect_int("pouch mask marker", probe->pouchMask,
                     DM1_PC34_ALLOWED_POUCH, dataAllowed);
    ok &= expect_int("quiver mask marker", probe->quiverLine1Mask,
                     DM1_PC34_ALLOWED_QUIVER_LINE1, dataAllowed);
    ok &= expect_int("backpack mask marker", probe->backpackMask,
                     DM1_PC34_ALLOWED_ANY_SLOT, dataAllowed);
    ok &= expect_int("item A mask before", probe->itemAMaskBefore,
                     DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_ALLOWED_A,
                     dataAllowed);
    ok &= expect_int("item B mask before", probe->itemBMaskBefore,
                     DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_ALLOWED_B,
                     dataAllowed);
    ok &= expect_int("item A container overlap",
                     probe->itemAContainerOverlap,
                     DM1_PC34_ALLOWED_CONTAINER, dataAllowed);
    ok &= expect_int("item B container overlap",
                     probe->itemBContainerOverlap,
                     DM1_PC34_ALLOWED_CONTAINER, dataAllowed);

    ok &= assert_view("before G0425 C537-C544", probe->beforeTypes,
                      beforeTypes, f0333);
    ok &= assert_allowed_view("before G0425 C537-C544",
                              probe->beforeAllowed, beforeAllowed,
                              dataAllowed);
    ok &= expect_int("non-empty count before",
                     probe->nonEmptyCountBefore, 5, f0333);
    ok &= expect_int("leader hand before",
                     probe->leaderHandBefore,
                     DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_LEADER_SENTINEL,
                     f0333);
    ok &= expect_int("load before swap", probe->loadBeforeSwap,
                     35, f0333);

    ok &= expect_int("select source click",
                     probe->selectedSourceResult, 1, f0333);
    ok &= expect_int("selected source index",
                     probe->selectedSourceIndex, 0, f0333);
    ok &= expect_int("selected thing",
                     probe->selectedThing,
                     DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_ITEM_A, f0032);
    ok &= assert_view("during drag G0425 C537-C544",
                      probe->duringDragTypes, duringTypes, f0291);
    ok &= expect_int("source empty during drag",
                     probe->sourceEmptyDuringDrag, 1, f0291);
    ok &= expect_int("destination drop click",
                     probe->selectedDestinationResult, 1, f0333);
    ok &= expect_int("swap completed", probe->swapCompleted, 1, f0333);
    ok &= assert_view("after swap G0425 C537-C544",
                      probe->afterSwapTypes, afterTypes, f0291);
    ok &= assert_allowed_view("after swap G0425 C537-C544",
                              probe->afterSwapAllowed, afterAllowed,
                              dataAllowed);
    ok &= expect_int("source receives B", probe->sourceReceivesB, 1,
                     f0333);
    ok &= expect_int("destination receives A",
                     probe->destinationReceivesA, 1, f0333);
    ok &= expect_int("item A mask preserved",
                     probe->itemAAllowedMaskPreserved, 1, dataAllowed);
    ok &= expect_int("item B mask preserved",
                     probe->itemBAllowedMaskPreserved, 1, dataAllowed);
    ok &= expect_int("leader hand after swap",
                     probe->leaderHandAfterSwap,
                     DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_LEADER_SENTINEL,
                     f0333);
    ok &= expect_int("leader hand untouched by slot swap",
                     probe->leaderHandUntouchedBySwap, 1, f0333);
    ok &= expect_int("load after swap", probe->loadAfterSwap, 35,
                     f0333);
    ok &= expect_int("load unchanged by swap",
                     probe->loadUnchangedBySwap, 1, f0333);
    ok &= expect_int("view preserved after swap",
                     probe->viewPreservedAfterSwap, 1, f0291);
    ok &= expect_int("icon refresh acknowledged",
                     probe->iconRefreshAcknowledged, 1, f0291);
    ok &= expect_int("masked blit transparent C10",
                     probe->maskedBlitTransparentC10, 1, f0133);
    ok &= expect_int("object type/icon lookup acknowledged",
                     probe->objectTypeIconLookupAcknowledged, 1, f0032);
    ok &= expect_int("ammo compatibility unaffected",
                     probe->ammoCompatibilityUnaffected, 1, f0294);
    ok &= expect_int("non-empty count after swap",
                     probe->nonEmptyCountAfterSwap, 5, f0333);

    ok &= expect_int("empty source click no-op",
                     probe->emptySourceClickResult, 0, f0333Guard);
    ok &= expect_int("empty destination click no-op",
                     probe->emptyDestinationClickResult, 0, f0333Guard);
    ok &= assert_view("after empty no-op G0425 C537-C544",
                      probe->afterEmptyNoopTypes, afterTypes, f0333Guard);
    ok &= expect_int("empty slots no-op", probe->emptySlotsNoop, 1,
                     f0333Guard);
    ok &= expect_int("leader hand after empty no-op",
                     probe->leaderHandAfterEmptyNoop,
                     DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_LEADER_SENTINEL,
                     f0333Guard);
    ok &= expect_int("load after empty no-op",
                     probe->loadAfterEmptyNoop, 35, f0333Guard);
    ok &= expect_int("load unchanged by empty no-op",
                     probe->loadUnchangedByNoop, 1, f0333Guard);

    ok &= expect_int("invalid can equip", probe->invalidCanEquip, 0,
                     dataAllowed);
    ok &= expect_int("invalid click rejected",
                     probe->invalidClickResult, 0, dataAllowed);
    ok &= assert_view("after invalid click G0425 C537-C544",
                      probe->afterInvalidTypes, afterTypes, dataAllowed);
    ok &= expect_int("invalid slot unchanged",
                     probe->invalidSlotUnchanged, 1, dataAllowed);
    ok &= expect_int("leader hand after invalid click",
                     probe->leaderHandAfterInvalidClick,
                     DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_INVALID_HAND_ITEM,
                     dataAllowed);
    ok &= expect_int("load after invalid click",
                     probe->loadAfterInvalidClick, 35, dataAllowed);
    ok &= expect_int("load unchanged by invalid click",
                     probe->loadUnchangedByInvalidClick, 1, dataAllowed);

    ok &= expect_int("close count", probe->closeCount, 5, f0334);
    ok &= assert_view("closed C537-C544 rewrite",
                      probe->closedTypes, afterTypes, f0334);
    ok &= assert_allowed_view("closed C537-C544 allowed",
                              probe->closedAllowed, afterAllowed, f0334);
    ok &= expect_int("open chest cleared after close",
                     probe->openChestClearedAfterClose, 1, f0334);
    ok &= expect_int("leader hand after close",
                     probe->leaderHandAfterClose,
                     DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_LEADER_SENTINEL,
                     f0334);
    ok &= expect_int("load after close", probe->loadAfterClose, 0,
                     f0334);
    ok &= expect_int("non-empty count after close",
                     probe->nonEmptyCountAfterClose, 5, f0334);

    ok &= expect_int("reopen result", probe->reopenResult, 1, f0333);
    ok &= expect_int("reopened open thing",
                     probe->reopenedOpenThing,
                     DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_CHEST_THING, f0333);
    ok &= assert_view("reopened C537-C544", probe->reopenedTypes,
                      afterTypes, f0333);
    ok &= assert_allowed_view("reopened C537-C544 allowed",
                              probe->reopenedAllowed, afterAllowed,
                              dataAllowed);
    ok &= expect_int("leader hand after reopen",
                     probe->leaderHandAfterReopen,
                     DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_LEADER_SENTINEL,
                     f0333);
    ok &= expect_int("load after reopen", probe->loadAfterReopen, 35,
                     f0333);
    ok &= expect_int("reopened load matches open load",
                     probe->reopenedLoadMatchesOpenLoad, 1, f0333);
    ok &= expect_int("close rewrite preserved visible order",
                     probe->closeRewritePreservedVisibleOrder, 1, f0334);
    ok &= expect_int("reopen preserved slot-to-slot swap",
                     probe->reopenPreservedSlotToSlotSwap, 1, f0333);
    ok &= expect_int("no duplicate object ids",
                     probe->noDuplicateObjectIds, 1, f0334);
    ok &= expect_int("non-empty count after reopen",
                     probe->nonEmptyCountAfterReopen, 5, f0333);
    ok &= expect_int("no leader hand leak",
                     probe->noLeaderHandLeak, 1, f0334);
    return ok;
}

int main(void)
{
    const char* f0333 =
        "ReDMCSB CHEST.C F0333 lines 53-67";
    DM1_V1_ChestSlotToSlotSwapProbePc34 probe;
    const DM1_V1_ChestSlotToSlotSwapSpecPc34* spec =
        dm1_v1_chest_slot_to_slot_swap_spec_pc34();
    int ok = 1;

    printf("probe=dm1_v1_chest_slot_to_slot_swap_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_chest_slot_to_slot_swap_source_evidence_pc34());

    ok &= expect_int("probe setup",
                     dm1_v1_chest_slot_to_slot_swap_run_pc34(&probe),
                     1, f0333);
    if (!ok) {
        printf("assertionCount=%d\n", g_assertions);
        printf("chestSlotToSlotSwapInvariantOk=0\n");
        return 1;
    }

    ok &= test_spec(spec);
    ok &= test_probe(&probe);
    ok &= expect_int("minimum assertion count",
                     g_assertions >=
                     DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_CONTRACT_ASSERTION_BUDGET ?
                     1 : 0, 1, f0333);

    printf("assertionCount=%d\n", g_assertions);
    printf("chestSlotToSlotSwapInvariantOk=%d\n", ok ? 1 : 0);
    return ok ? 0 : 1;
}
