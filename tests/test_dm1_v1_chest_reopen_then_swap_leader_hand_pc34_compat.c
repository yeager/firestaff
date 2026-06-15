#include "dm1_v1_chest_reopen_then_swap_leader_hand_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static DM1_V1_ChestReopenThenSwapLeaderHandProbePc34 g_probe;
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

static int expect_uint_equal(const char* label,
                             unsigned int got,
                             unsigned int want,
                             const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (got != want) {
        printf("FAIL %s got=%u want=%u anchor=%s\n",
               label, got, want, redmcsbAnchor);
        return 0;
    }
    printf("PASS %s=%u anchor=%s\n", label, got, redmcsbAnchor);
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
    const char* f0333 = "ReDMCSB CHEST.C F0333 lines 31-67";
    const DM1_V1_ChestReopenThenSwapLeaderHandSpecPc34* spec =
        M11_GameView_ChestReopenThenSwapLeaderHandSpecPc34();
    int ok = 1;

    ok &= expect_str("contract marker", spec->contractMarker,
                     "Source-locked contract gate only; not full real-asset chest runtime parity.",
                     f0333);
    ok &= expect_int("contract_only marker", g_probe.contract_only, 1,
                     f0333);
    ok &= expect_int("C537 slot constant", spec->c537Pc34Slot,
                     DM1_PC34_SLOT_CHEST_1, f0333);
    ok &= expect_int("C544 slot constant", spec->c544Pc34Slot,
                     DM1_PC34_SLOT_CHEST_8, f0333);
    ok &= expect_int("chest slot count", spec->chestSlotCount,
                     DM1_PC34_CHEST_SLOT_COUNT, f0333);
    ok &= expect_int("max linked input count", spec->maxLinkedInputCount,
                     DM1_PC34_CHEST_REOPEN_SWAP_MAX_LINKED, f0333);
    ok &= expect_int("chest B destination index",
                     spec->chestBDestinationIndex,
                     DM1_PC34_CHEST_REOPEN_SWAP_B_DEST_INDEX, f0333);
    ok &= expect_int("chest A first visible sentinel",
                     spec->chestAVisibleFirstItemType,
                     DM1_PC34_CHEST_REOPEN_SWAP_A_FIRST, f0333);
    ok &= expect_int("chest A hidden tail sentinel",
                     spec->chestAHiddenTailItemType,
                     DM1_PC34_CHEST_REOPEN_SWAP_A_HIDDEN_TAIL, f0333);
    return ok;
}

static int assert_visible_order(const char* phase,
                                const int* types,
                                const char* redmcsbAnchor)
{
    int ok = 1;
    int i;

    for (i = 0; i < DM1_PC34_CHEST_REOPEN_SWAP_SLOT_COUNT; ++i) {
        char label[96];

        snprintf(label, sizeof(label), "%s visible slot %d", phase, i);
        ok &= expect_int(label, types[i],
                         DM1_PC34_CHEST_REOPEN_SWAP_A_FIRST + i,
                         redmcsbAnchor);
    }
    return ok;
}

static int test_reopen_then_place_into_chest_b(void)
{
    const char* f0333 = "ReDMCSB CHEST.C F0333 lines 31-67";
    const char* f0334 = "ReDMCSB CHEST.C F0334 lines 113-132";
    const char* f0297 = "ReDMCSB CHAMPION.C F0297 lines 250-298";
    const char* f0298 = "ReDMCSB CHAMPION.C F0298 lines 263-285";
    const char* f0302 = "ReDMCSB CHAMPION.C F0302 lines 688-710";
    const char* f0163 = "ReDMCSB DUNGEON.C F0163 lines 1796-1837";
    int ok = 1;

    ok &= expect_int("chest A opens", g_probe.chestAOpenResult, 1,
                     f0333);
    ok &= expect_int("chest A open thing", g_probe.chestAOpenThing,
                     DM1_PC34_CHEST_REOPEN_SWAP_A_THING, f0333);
    ok &= expect_int("chest A opened visible count",
                     g_probe.chestAOpenedVisibleCount,
                     DM1_PC34_CHEST_REOPEN_SWAP_SLOT_COUNT, f0333);
    ok &= expect_int("chest A opened order matches input",
                     g_probe.chestAOpenedOrderMatchesInput, 1, f0333);
    ok &= assert_visible_order("opened A", g_probe.chestAOpenedTypes,
                               f0333);

    ok &= expect_int("leader hand setup", g_probe.leaderHandSetupResult, 1,
                     f0297);
    ok &= expect_int("leader hand holds hidden tail before close",
                     g_probe.leaderHandBeforeClose,
                     DM1_PC34_CHEST_REOPEN_SWAP_A_HIDDEN_TAIL, f0297);
    ok &= expect_int("hidden tail can enter chest B",
                     g_probe.leaderHandCanEnterChestB, 1, f0302);

    ok &= expect_int("chest A close count", g_probe.chestACloseCount,
                     DM1_PC34_CHEST_REOPEN_SWAP_SLOT_COUNT, f0334);
    ok &= assert_visible_order("closed A", g_probe.chestAClosedTypes,
                               f0334);
    ok &= expect_int("hidden tail absent from A close rewrite",
                     g_probe.chestAHiddenTailReturnedByClose, 0, f0334);
    ok &= expect_int("leader hand after A close",
                     g_probe.leaderHandAfterClose,
                     DM1_PC34_CHEST_REOPEN_SWAP_A_HIDDEN_TAIL, f0297);
    ok &= expect_int("leader hand stable across A close",
                     g_probe.leaderHandStableAcrossClose, 1, f0297);

    ok &= expect_int("chest A reopens", g_probe.chestAReopenResult, 1,
                     f0333);
    ok &= expect_int("chest A reopen thing", g_probe.chestAReopenThing,
                     DM1_PC34_CHEST_REOPEN_SWAP_A_REOPEN_THING, f0333);
    ok &= expect_int("chest A reopened visible count",
                     g_probe.chestAReopenedVisibleCount,
                     DM1_PC34_CHEST_REOPEN_SWAP_SLOT_COUNT, f0333);
    ok &= assert_visible_order("reopened A", g_probe.chestAReopenedTypes,
                               f0333);
    ok &= expect_int("A reopened order matches close rewrite",
                     g_probe.chestAReopenedOrderMatchesClose, 1, f0163);
    ok &= expect_int("hidden tail absent after A reopen",
                     g_probe.chestAHiddenTailVisibleAfterReopen, 0, f0333);
    ok &= expect_int("leader hand after A reopen",
                     g_probe.leaderHandAfterReopen,
                     DM1_PC34_CHEST_REOPEN_SWAP_A_HIDDEN_TAIL, f0297);
    ok &= expect_int("leader hand stable across A reopen",
                     g_probe.leaderHandStableAcrossReopen, 1, f0297);

    ok &= expect_int("A closes while B opens",
                     g_probe.chestACloseWhileOpeningBCount,
                     DM1_PC34_CHEST_REOPEN_SWAP_SLOT_COUNT, f0334);
    ok &= assert_visible_order("A closed while opening B",
                               g_probe.chestAClosedWhileOpeningBTypes,
                               f0163);
    ok &= expect_int("hidden tail absent while opening B",
                     g_probe.chestAHiddenTailReturnedWhileOpeningB, 0,
                     f0334);
    ok &= expect_int("chest B open thing", g_probe.chestBOpenThing,
                     DM1_PC34_CHEST_REOPEN_SWAP_B_THING, f0333);

    ok &= expect_int("chest B place click", g_probe.chestBPlaceClickResult,
                     1, f0302);
    ok &= expect_int("chest B destination holds hidden tail",
                     g_probe.chestBDestinationAfterPlace,
                     DM1_PC34_CHEST_REOPEN_SWAP_A_HIDDEN_TAIL, f0302);
    ok &= expect_int("hidden tail stored in chest B flag",
                     g_probe.hiddenTailStoredInChestB, 1, f0302);
    ok &= expect_int("leader hand empty after B place",
                     g_probe.leaderHandAfterPlace, 0, f0298);
    ok &= expect_int("leader hand empty flag",
                     g_probe.leaderHandEmptyAfterPlace, 1, f0298);
    ok &= expect_int("chest A hidden tail still empty",
                     g_probe.chestAHiddenTailStillEmpty, 1, f0163);

    ok &= expect_int("world hash before result",
                     g_probe.worldHashBeforeResult, 1, f0163);
    ok &= expect_int("world hash after close result",
                     g_probe.worldHashAfterCloseResult, 1, f0334);
    ok &= expect_uint_equal("world hash stable after A close value",
                            g_probe.worldHashAfterClose,
                            g_probe.worldHashBefore, f0334);
    ok &= expect_int("world hash stable after A close flag",
                     g_probe.chestAWorldHashStableAfterClose, 1, f0334);
    ok &= expect_int("world hash after final result",
                     g_probe.worldHashAfterFinalResult, 1, f0302);
    ok &= expect_uint_equal("world hash stable after B place value",
                            g_probe.worldHashAfterFinal,
                            g_probe.worldHashBefore, f0302);
    ok &= expect_int("world hash stable after B place flag",
                     g_probe.chestAWorldHashStableFinal, 1, f0302);
    return ok;
}

int main(void)
{
    const char* f0333 = "ReDMCSB CHEST.C F0333 lines 31-67";
    int ok = 1;

    printf("probe=dm1_v1_chest_reopen_then_swap_leader_hand_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           M11_GameView_ChestReopenThenSwapLeaderHandSourceEvidencePc34());

    ok &= expect_int("probe setup",
                     M11_GameView_ChestReopenThenSwapLeaderHandRunPc34(
                         &g_probe),
                     1, f0333);
    if (!ok) {
        printf("assertionCount=%d\n", g_assertions);
        printf("chestReopenThenSwapLeaderHandInvariantOk=0\n");
        return 1;
    }

    ok &= test_spec();
    ok &= test_reopen_then_place_into_chest_b();

    ok &= expect_int("minimum assertion count",
                     g_assertions >= 55 ? 1 : 0, 1, f0333);

    printf("assertionCount=%d\n", g_assertions);
    printf("chestReopenThenSwapLeaderHandInvariantOk=%d\n", ok ? 1 : 0);
    return ok ? 0 : 1;
}
