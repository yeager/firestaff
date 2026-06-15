#include "dm1_v1_chest_round_trip_hand_swap_pc34_compat.h"

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
    printf("PASS %s contains=%s anchor=%s\n",
           label, needle, redmcsbAnchor);
    return 1;
}

static int assert_empty_tail(const char* phase,
                             const int* types,
                             int startIndex,
                             const char* redmcsbAnchor)
{
    int ok = 1;
    int i;

    for (i = startIndex; i < DM1_PC34_CHEST_ROUND_TRIP_SLOT_COUNT; ++i) {
        char label[96];

        snprintf(label, sizeof(label), "%s C%d empty",
                 phase, 537 + i);
        ok &= expect_int(label, types[i], 0, redmcsbAnchor);
    }
    return ok;
}

static int test_spec_and_source(void)
{
    const DM1_V1_ChestRoundTripHandSwapSpecPc34* spec =
        dm1_v1_chest_round_trip_hand_swap_spec_pc34();
    const char* f0333 = "ReDMCSB CHEST.C F0333:53-67";
    const char* f0334 = "ReDMCSB CHEST.C F0334:117-132";
    const char* f0297 = "ReDMCSB CHAMPION.C F0297:263-265";
    const char* f0300 = "ReDMCSB CHAMPION.C F0300:582-615";
    const char* f0301 = "ReDMCSB CHAMPION.C F0301:582-615";
    int ok = 1;

    ok &= expect_int("spec contract only", spec->contractOnly, 1, f0333);
    ok &= expect_int("spec C537", spec->c537Slot,
                     DM1_PC34_SLOT_CHEST_1, f0333);
    ok &= expect_int("spec C538", spec->c538Slot,
                     DM1_PC34_SLOT_CHEST_2, f0333);
    ok &= expect_int("spec C544", spec->c544Slot,
                     DM1_PC34_SLOT_CHEST_8, f0333);
    ok &= expect_int("spec visible slots", spec->visibleSlotCount,
                     DM1_PC34_CHEST_SLOT_COUNT, f0333);
    ok &= expect_int("spec linked input count", spec->linkedInputCount,
                     DM1_PC34_CHEST_ROUND_TRIP_LINKED_INPUT_COUNT, f0333);
    ok &= expect_int("spec hidden tail not claimed", spec->hiddenTailClaimed,
                     0, f0333);
    ok &= expect_int("spec dagger id", spec->dagger.itemType,
                     DM1_PC34_CHEST_ROUND_TRIP_DAGGER, f0333);
    ok &= expect_int("spec dagger weight", spec->dagger.weight,
                     DM1_PC34_CHEST_ROUND_TRIP_DAGGER_WEIGHT, f0297);
    ok &= expect_int("spec dagger allowed", spec->dagger.allowedSlots,
                     DM1_PC34_ALLOWED_CONTAINER, f0301);
    ok &= expect_int("spec torch id", spec->torch.itemType,
                     DM1_PC34_CHEST_ROUND_TRIP_TORCH, f0333);
    ok &= expect_int("spec torch weight", spec->torch.weight,
                     DM1_PC34_CHEST_ROUND_TRIP_TORCH_WEIGHT, f0300);
    ok &= expect_int("spec torch allowed", spec->torch.allowedSlots,
                     DM1_PC34_ALLOWED_CONTAINER, f0301);
    ok &= expect_int("spec base item id", spec->baseItem.itemType,
                     DM1_PC34_CHEST_ROUND_TRIP_BASE_ITEM, f0297);
    ok &= expect_int("spec base item weight", spec->baseItem.weight,
                     DM1_PC34_CHEST_ROUND_TRIP_BASE_WEIGHT, f0297);
    ok &= expect_contains("spec F0333 anchor", spec->f0333Anchor,
                          "CHEST.C F0333:53-67", f0333);
    ok &= expect_contains("spec F0334 anchor", spec->f0334Anchor,
                          "CHEST.C F0334:117-132", f0334);
    ok &= expect_contains("spec F0297 anchor", spec->f0297Anchor,
                          "CHAMPION.C F0297:263-265", f0297);
    ok &= expect_contains("spec F0300 anchor", spec->f0300Anchor,
                          "CHAMPION.C F0300:582-615", f0300);
    ok &= expect_contains("spec F0301 anchor", spec->f0301Anchor,
                          "CHAMPION.C F0301:582-615", f0301);
    ok &= expect_contains("source says contract only",
                          dm1_v1_chest_round_trip_hand_swap_source_evidence_pc34(),
                          "contract_only=1", f0333);
    ok &= expect_contains("source says no hidden tail",
                          dm1_v1_chest_round_trip_hand_swap_source_evidence_pc34(),
                          "does not claim the hidden-tail", f0333);
    return ok;
}

static int test_open_step(const DM1_V1_ChestRoundTripHandSwapProbePc34* p)
{
    const char* f0333 = "ReDMCSB CHEST.C F0333:53-67";
    const char* f0297 = "ReDMCSB CHAMPION.C F0297:263-265";
    const char* f0300 = "ReDMCSB CHAMPION.C F0300:582-615";
    int ok = 1;

    ok &= expect_int("base item set", p->baseItemSetResult, 1, f0297);
    ok &= expect_int("probe contract only", p->contractOnly, 1, f0333);
    ok &= expect_int("probe hidden tail not claimed",
                     p->hiddenTailClaimed, 0, f0333);
    ok &= expect_int("original dagger id", p->originalDaggerId,
                     DM1_PC34_CHEST_ROUND_TRIP_DAGGER, f0333);
    ok &= expect_int("original torch id", p->originalTorchId,
                     DM1_PC34_CHEST_ROUND_TRIP_TORCH, f0333);
    ok &= expect_int("open result", p->open.openResult, 1, f0333);
    ok &= expect_int("open thing", p->open.openThing,
                     DM1_PC34_CHEST_ROUND_TRIP_CHEST_THING, f0333);
    ok &= expect_int("open C537 dagger", p->open.openedTypes[0],
                     DM1_PC34_CHEST_ROUND_TRIP_DAGGER, f0333);
    ok &= expect_int("open C537 dagger weight", p->open.openedWeights[0],
                     DM1_PC34_CHEST_ROUND_TRIP_DAGGER_WEIGHT, f0297);
    ok &= expect_int("open C538 torch", p->open.openedTypes[1],
                     DM1_PC34_CHEST_ROUND_TRIP_TORCH, f0333);
    ok &= expect_int("open C538 torch weight", p->open.openedWeights[1],
                     DM1_PC34_CHEST_ROUND_TRIP_TORCH_WEIGHT, f0297);
    ok &= assert_empty_tail("open", p->open.openedTypes, 2, f0333);
    ok &= expect_int("open visible count", p->open.openedVisibleCount, 2,
                     f0333);
    ok &= expect_int("open has dagger flag", p->open.openedHasDaggerAtC537,
                     1, f0333);
    ok &= expect_int("open has torch flag", p->open.openedHasTorchAtC538,
                     1, f0333);
    ok &= expect_int("open only visible eight flag",
                     p->open.openedOnlyVisibleSlots, 1, f0333);
    ok &= expect_int("pre-open load", p->open.loadBeforeOpen,
                     DM1_PC34_CHEST_ROUND_TRIP_BASE_WEIGHT, f0297);
    ok &= expect_int("open visible weight", p->open.openVisibleWeight,
                     DM1_PC34_CHEST_ROUND_TRIP_DAGGER_WEIGHT +
                     DM1_PC34_CHEST_ROUND_TRIP_TORCH_WEIGHT, f0300);
    ok &= expect_int("open container base contribution",
                     p->open.openContainerBaseContribution,
                     DM1_PC34_CHEST_EMPTY_THING_WEIGHT, f0333);
    ok &= expect_int("open container weight", p->open.openContainerWeight,
                     DM1_PC34_CHEST_EMPTY_THING_WEIGHT +
                     DM1_PC34_CHEST_ROUND_TRIP_DAGGER_WEIGHT +
                     DM1_PC34_CHEST_ROUND_TRIP_TORCH_WEIGHT, f0333);
    ok &= expect_int("open load", p->open.loadAfterOpen,
                     DM1_PC34_CHEST_ROUND_TRIP_BASE_WEIGHT +
                     DM1_PC34_CHEST_ROUND_TRIP_DAGGER_WEIGHT +
                     DM1_PC34_CHEST_ROUND_TRIP_TORCH_WEIGHT, f0300);
    ok &= expect_int("open load delta", p->open.loadDeltaAfterOpen,
                     DM1_PC34_CHEST_ROUND_TRIP_DAGGER_WEIGHT +
                     DM1_PC34_CHEST_ROUND_TRIP_TORCH_WEIGHT, f0300);
    return ok;
}

static int test_first_swap_step(
    const DM1_V1_ChestRoundTripHandSwapProbePc34* p)
{
    const char* f0297 = "ReDMCSB CHAMPION.C F0297:263-265";
    const char* f0300 = "ReDMCSB CHAMPION.C F0300:582-615";
    const char* f0301 = "ReDMCSB CHAMPION.C F0301:582-615";
    const char* f0333 = "ReDMCSB CHEST.C F0333:53-67";
    int ok = 1;

    ok &= expect_int("first C538 click", p->firstSwap.firstClickResult, 1,
                     f0300);
    ok &= expect_int("first hand torch",
                     p->firstSwap.leaderHandAfterFirstType,
                     DM1_PC34_CHEST_ROUND_TRIP_TORCH, f0297);
    ok &= expect_int("first hand torch weight",
                     p->firstSwap.leaderHandAfterFirstWeight,
                     DM1_PC34_CHEST_ROUND_TRIP_TORCH_WEIGHT, f0297);
    ok &= expect_int("first hand can enter chest",
                     p->firstSwap.leaderHandAfterFirstCanEnterChest, 1,
                     f0301);
    ok &= expect_int("first C537 still dagger",
                     p->firstSwap.afterFirstTypes[0],
                     DM1_PC34_CHEST_ROUND_TRIP_DAGGER, f0333);
    ok &= expect_int("first C537 weight",
                     p->firstSwap.afterFirstWeights[0],
                     DM1_PC34_CHEST_ROUND_TRIP_DAGGER_WEIGHT, f0300);
    ok &= expect_int("first C538 empty",
                     p->firstSwap.afterFirstTypes[1], 0, f0300);
    ok &= expect_int("first C538 weight zero",
                     p->firstSwap.afterFirstWeights[1], 0, f0300);
    ok &= assert_empty_tail("first swap",
                            p->firstSwap.afterFirstTypes, 2, f0333);
    ok &= expect_int("first visible count",
                     p->firstSwap.afterFirstVisibleCount, 1, f0333);
    ok &= expect_int("first dagger flag",
                     p->firstSwap.afterFirstC537StillDagger, 1, f0333);
    ok &= expect_int("first C538 empty flag",
                     p->firstSwap.afterFirstC538Empty, 1, f0300);
    ok &= expect_int("first visible weight",
                     p->firstSwap.afterFirstVisibleWeight,
                     DM1_PC34_CHEST_ROUND_TRIP_DAGGER_WEIGHT, f0300);
    ok &= expect_int("first container base contribution",
                     p->firstSwap.afterFirstContainerBaseContribution,
                     DM1_PC34_CHEST_EMPTY_THING_WEIGHT, f0333);
    ok &= expect_int("first container weight",
                     p->firstSwap.afterFirstContainerWeight,
                     DM1_PC34_CHEST_EMPTY_THING_WEIGHT +
                     DM1_PC34_CHEST_ROUND_TRIP_DAGGER_WEIGHT, f0333);
    ok &= expect_int("first load",
                     p->firstSwap.loadAfterFirstSwap,
                     DM1_PC34_CHEST_ROUND_TRIP_BASE_WEIGHT +
                     DM1_PC34_CHEST_ROUND_TRIP_DAGGER_WEIGHT, f0300);
    ok &= expect_int("first slot-load delta",
                     p->firstSwap.loadDeltaAfterFirstSwap,
                     -DM1_PC34_CHEST_ROUND_TRIP_TORCH_WEIGHT, f0300);
    ok &= expect_int("first effective load",
                     p->firstSwap.effectiveLoadAfterFirstSwap,
                     p->open.loadAfterOpen, f0297);
    ok &= expect_int("first effective load delta",
                     p->firstSwap.effectiveLoadDeltaAfterFirstSwap,
                     0, f0297);
    return ok;
}

static int test_second_swap_step(
    const DM1_V1_ChestRoundTripHandSwapProbePc34* p)
{
    const char* f0297 = "ReDMCSB CHAMPION.C F0297:263-265";
    const char* f0300 = "ReDMCSB CHAMPION.C F0300:582-615";
    const char* f0301 = "ReDMCSB CHAMPION.C F0301:582-615";
    const char* f0333 = "ReDMCSB CHEST.C F0333:53-67";
    int ok = 1;

    ok &= expect_int("second C537 click", p->secondSwap.secondClickResult,
                     1, f0300);
    ok &= expect_int("second hand dagger",
                     p->secondSwap.leaderHandAfterSecondType,
                     DM1_PC34_CHEST_ROUND_TRIP_DAGGER, f0297);
    ok &= expect_int("second hand dagger weight",
                     p->secondSwap.leaderHandAfterSecondWeight,
                     DM1_PC34_CHEST_ROUND_TRIP_DAGGER_WEIGHT, f0297);
    ok &= expect_int("second hand can enter chest",
                     p->secondSwap.leaderHandAfterSecondCanEnterChest, 1,
                     f0301);
    ok &= expect_int("second C537 torch",
                     p->secondSwap.afterSecondTypes[0],
                     DM1_PC34_CHEST_ROUND_TRIP_TORCH, f0301);
    ok &= expect_int("second C537 torch weight",
                     p->secondSwap.afterSecondWeights[0],
                     DM1_PC34_CHEST_ROUND_TRIP_TORCH_WEIGHT, f0301);
    ok &= expect_int("second C538 empty",
                     p->secondSwap.afterSecondTypes[1], 0, f0300);
    ok &= expect_int("second C538 weight zero",
                     p->secondSwap.afterSecondWeights[1], 0, f0300);
    ok &= assert_empty_tail("second swap",
                            p->secondSwap.afterSecondTypes, 2, f0333);
    ok &= expect_int("second visible count",
                     p->secondSwap.afterSecondVisibleCount, 1, f0333);
    ok &= expect_int("second C537 torch flag",
                     p->secondSwap.afterSecondC537Torch, 1, f0301);
    ok &= expect_int("second C538 empty flag",
                     p->secondSwap.afterSecondC538Empty, 1, f0300);
    ok &= expect_int("dagger only in leader hand",
                     p->secondSwap.afterSecondDaggerOnlyInLeaderHand, 1,
                     f0297);
    ok &= expect_int("torch only in chest",
                     p->secondSwap.afterSecondTorchOnlyInChest, 1, f0301);
    ok &= expect_int("second visible weight",
                     p->secondSwap.afterSecondVisibleWeight,
                     DM1_PC34_CHEST_ROUND_TRIP_TORCH_WEIGHT, f0301);
    ok &= expect_int("second container base contribution",
                     p->secondSwap.afterSecondContainerBaseContribution,
                     DM1_PC34_CHEST_EMPTY_THING_WEIGHT, f0333);
    ok &= expect_int("second container weight",
                     p->secondSwap.afterSecondContainerWeight,
                     DM1_PC34_CHEST_EMPTY_THING_WEIGHT +
                     DM1_PC34_CHEST_ROUND_TRIP_TORCH_WEIGHT, f0333);
    ok &= expect_int("second load",
                     p->secondSwap.loadAfterSecondSwap,
                     DM1_PC34_CHEST_ROUND_TRIP_BASE_WEIGHT +
                     DM1_PC34_CHEST_ROUND_TRIP_TORCH_WEIGHT, f0301);
    ok &= expect_int("second slot-load delta",
                     p->secondSwap.loadDeltaAfterSecondSwap,
                     0, f0301);
    ok &= expect_int("second effective load",
                     p->secondSwap.effectiveLoadAfterSecondSwap,
                     p->open.loadAfterOpen, f0297);
    ok &= expect_int("second effective load delta",
                     p->secondSwap.effectiveLoadDeltaAfterSecondSwap,
                     0, f0297);
    return ok;
}

static int test_close_step(const DM1_V1_ChestRoundTripHandSwapProbePc34* p)
{
    const char* f0334 = "ReDMCSB CHEST.C F0334:117-132";
    const char* f0297 = "ReDMCSB CHAMPION.C F0297:263-265";
    const char* f0300 = "ReDMCSB CHAMPION.C F0300:582-615";
    int ok = 1;

    ok &= expect_int("close count", p->close.closeCount, 1, f0334);
    ok &= expect_int("close clears open chest",
                     p->close.closeClearsOpenChest, 1, f0334);
    ok &= expect_int("closed C537 torch", p->close.closedTypes[0],
                     DM1_PC34_CHEST_ROUND_TRIP_TORCH, f0334);
    ok &= expect_int("closed C537 torch weight", p->close.closedWeights[0],
                     DM1_PC34_CHEST_ROUND_TRIP_TORCH_WEIGHT, f0334);
    ok &= expect_int("closed C538 empty", p->close.closedTypes[1], 0,
                     f0334);
    ok &= expect_int("closed C538 weight zero", p->close.closedWeights[1],
                     0, f0334);
    ok &= assert_empty_tail("closed", p->close.closedTypes, 1, f0334);
    ok &= expect_int("closed visible count", p->close.closedVisibleCount,
                     1, f0334);
    ok &= expect_int("closed C537 torch flag", p->close.closedC537Torch,
                     1, f0334);
    ok &= expect_int("closed C538 empty flag", p->close.closedC538Empty,
                     1, f0334);
    ok &= expect_int("closed excludes hand dagger",
                     p->close.closedDaggerExcludedBecauseLeaderHand, 1,
                     f0334);
    ok &= expect_int("close weight snapshot",
                     p->close.closeContainerWeightSnapshot,
                     DM1_PC34_CHEST_EMPTY_THING_WEIGHT +
                     DM1_PC34_CHEST_ROUND_TRIP_TORCH_WEIGHT, f0334);
    ok &= expect_int("close base contribution",
                     p->close.closeContainerBaseContribution,
                     DM1_PC34_CHEST_EMPTY_THING_WEIGHT, f0334);
    ok &= expect_int("close container after", p->close.closeContainerWeightAfter,
                     0, f0334);
    ok &= expect_int("close load back to pre-open",
                     p->close.loadAfterClose,
                     p->open.loadBeforeOpen, f0300);
    ok &= expect_int("close load delta from pre-open",
                     p->close.loadDeltaAfterClose, 0, f0300);
    ok &= expect_int("close leader hand dagger",
                     p->close.leaderHandAfterCloseType,
                     DM1_PC34_CHEST_ROUND_TRIP_DAGGER, f0297);
    ok &= expect_int("close leader hand weight",
                     p->close.leaderHandAfterCloseWeight,
                     DM1_PC34_CHEST_ROUND_TRIP_DAGGER_WEIGHT, f0297);
    return ok;
}

static int test_reopen_step(const DM1_V1_ChestRoundTripHandSwapProbePc34* p)
{
    const char* f0333 = "ReDMCSB CHEST.C F0333:53-67";
    const char* f0334 = "ReDMCSB CHEST.C F0334:117-132";
    const char* f0297 = "ReDMCSB CHAMPION.C F0297:263-265";
    const char* f0301 = "ReDMCSB CHAMPION.C F0301:582-615";
    int ok = 1;

    ok &= expect_int("reopen result", p->reopen.reopenResult, 1, f0333);
    ok &= expect_int("reopen thing", p->reopen.reopenThing,
                     DM1_PC34_CHEST_ROUND_TRIP_REOPEN_THING, f0333);
    ok &= expect_int("reopened C537 torch", p->reopen.reopenedTypes[0],
                     DM1_PC34_CHEST_ROUND_TRIP_TORCH, f0333);
    ok &= expect_int("reopened C537 torch weight",
                     p->reopen.reopenedWeights[0],
                     DM1_PC34_CHEST_ROUND_TRIP_TORCH_WEIGHT, f0333);
    ok &= expect_int("reopened C538 empty", p->reopen.reopenedTypes[1],
                     0, f0333);
    ok &= expect_int("reopened C538 weight zero",
                     p->reopen.reopenedWeights[1], 0, f0333);
    ok &= assert_empty_tail("reopened", p->reopen.reopenedTypes, 1,
                            f0333);
    ok &= expect_int("reopened visible count",
                     p->reopen.reopenedVisibleCount, 1, f0333);
    ok &= expect_int("reopened C537 torch flag",
                     p->reopen.reopenedC537Torch, 1, f0333);
    ok &= expect_int("reopened C538 empty flag",
                     p->reopen.reopenedC538Empty, 1, f0333);
    ok &= expect_int("reopened dagger leader flag",
                     p->reopen.reopenedDaggerStillLeaderHand, 1, f0297);
    ok &= expect_int("reopened torch preserved",
                     p->reopen.reopenedTorchPreserved, 1, f0334);
    ok &= expect_int("reopened identities preserved",
                     p->reopen.reopenedOriginalObjectIdentitiesPreserved,
                     1, f0334);
    ok &= expect_int("reopened visible weight",
                     p->reopen.reopenedVisibleWeight,
                     DM1_PC34_CHEST_ROUND_TRIP_TORCH_WEIGHT, f0301);
    ok &= expect_int("reopened container base contribution",
                     p->reopen.reopenedContainerBaseContribution,
                     DM1_PC34_CHEST_EMPTY_THING_WEIGHT, f0333);
    ok &= expect_int("reopened container weight",
                     p->reopen.reopenedContainerWeight,
                     DM1_PC34_CHEST_EMPTY_THING_WEIGHT +
                     DM1_PC34_CHEST_ROUND_TRIP_TORCH_WEIGHT, f0333);
    ok &= expect_int("reopened load",
                     p->reopen.loadAfterReopen,
                     DM1_PC34_CHEST_ROUND_TRIP_BASE_WEIGHT +
                     DM1_PC34_CHEST_ROUND_TRIP_TORCH_WEIGHT, f0301);
    ok &= expect_int("reopened load delta from close",
                     p->reopen.loadDeltaAfterReopen,
                     DM1_PC34_CHEST_ROUND_TRIP_TORCH_WEIGHT, f0301);
    ok &= expect_int("reopened leader hand dagger",
                     p->reopen.leaderHandAfterReopenType,
                     DM1_PC34_CHEST_ROUND_TRIP_DAGGER, f0297);
    ok &= expect_int("reopened leader hand weight",
                     p->reopen.leaderHandAfterReopenWeight,
                     DM1_PC34_CHEST_ROUND_TRIP_DAGGER_WEIGHT, f0297);
    ok &= expect_int("reopened no hidden-tail ninth path",
                     p->hiddenTailClaimed, 0, f0333);
    return ok;
}

int main(void)
{
    const DM1_V1_ChestRoundTripHandSwapProbePc34* probe;
    const char* f0333 = "ReDMCSB CHEST.C F0333:53-67";
    const char* f0334 = "ReDMCSB CHEST.C F0334:117-132";
    int runPassed = 0;
    int runFailed = 0;
    int ok = 1;

    printf("probe=dm1_v1_chest_round_trip_hand_swap_pc34_compat\n");

    ok &= expect_int("runtime run result",
                     dm1_v1_chest_round_trip_hand_swap_run(
                         &runPassed, &runFailed),
                     1, f0333);
    probe = dm1_v1_chest_round_trip_hand_swap_last_probe_pc34();
    ok &= expect_int("runtime internal passed nonzero",
                     runPassed > 0 ? 1 : 0, 1, f0333);
    ok &= expect_int("runtime internal failed zero", runFailed, 0, f0334);

    ok &= test_spec_and_source();
    ok &= test_open_step(probe);
    ok &= test_first_swap_step(probe);
    ok &= test_second_swap_step(probe);
    ok &= test_close_step(probe);
    ok &= test_reopen_step(probe);
    ok &= expect_int("minimum assertion count",
                     g_assertions >= 60 ? 1 : 0, 1, f0333);

    printf("assertions=%d failures=%d internalPassed=%d internalFailed=%d\n",
           g_assertions, g_failures, runPassed, runFailed);
    printf("chestRoundTripHandSwapInvariantOk=%d\n",
           (ok && g_failures == 0) ? 1 : 0);
    if (ok && g_failures == 0) {
        printf("PASS dm1_v1_chest_round_trip_hand_swap_pc34_compat assertions=%d\n",
               g_assertions);
        return 0;
    }
    return 1;
}
