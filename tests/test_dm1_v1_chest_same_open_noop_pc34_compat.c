/*
 * ReDMCSB CHEST.C F0333 lines 30-32: same G0426_T_OpenChest requests return
 * before F0334 close or F0333 G0425 materialization can reorganize contents.
 * ReDMCSB CHEST.C F0333 lines 53-76: first open fills C537..C544 from the
 * linked container chain and THING_NONE-pads the remaining visible slots.
 * ReDMCSB CHAMPION.C F0302 lines 688-710: C30+ slot clicks swap the leader
 * hand with the currently materialized G0425_aT_ChestSlots entry.
 */
#include "dm1_v1_chest_same_open_noop_pc34_compat.h"

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

static int expect_contains(const char* label,
                           const char* haystack,
                           const char* needle,
                           const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (!haystack || !needle || !strstr(haystack, needle)) {
        printf("FAIL %s missing=%s anchor=%s\n",
               label, needle ? needle : "(null)", redmcsbAnchor);
        return 0;
    }
    printf("PASS %s contains=%s anchor=%s\n", label, needle, redmcsbAnchor);
    return 1;
}

static int test_source_evidence(void)
{
    const char* f0333Guard = "ReDMCSB CHEST.C F0333 lines 30-32";
    const char* f0333Slots = "ReDMCSB CHEST.C F0333 lines 53-76";
    const char* f0334 = "ReDMCSB CHEST.C F0334 lines 113-132";
    const char* f0302 = "ReDMCSB CHAMPION.C F0302 lines 688-710";
    const char* evidence =
        dm1_v1_chest_same_open_noop_source_evidence_pc34();
    int ok = 1;

    ok &= expect_contains("evidence same chest guard", evidence,
                          "lines 30-32", f0333Guard);
    ok &= expect_contains("evidence panel side effect", evidence,
                          "line 28", "ReDMCSB CHEST.C F0333 line 28");
    ok &= expect_contains("evidence G0425 materialization", evidence,
                          "lines 53-76", f0333Slots);
    ok &= expect_contains("evidence close bypass", evidence,
                          "lines 113-132", f0334);
    ok &= expect_contains("evidence C30+ swap", evidence,
                          "lines 688-710", f0302);
    return ok;
}

static int test_first_open(
    const DM1_V1_ChestSameOpenNoopProbePc34* probe)
{
    const char* f0333Slots = "ReDMCSB CHEST.C F0333 lines 53-76";
    int ok = 1;
    int i;

    ok &= expect_int("setup result", probe->setupResult, 1, f0333Slots);
    ok &= expect_int("first open result", probe->firstOpenResult, 1,
                     f0333Slots);
    ok &= expect_int("open thing after first open",
                     probe->openThingAfterFirstOpen,
                     DM1_PC34_CHEST_SAME_OPEN_THING, f0333Slots);
    for (i = 0; i < 4; ++i) {
        ok &= expect_int("first open visible item",
                         probe->firstOpenSlotTypes[i],
                         DM1_PC34_CHEST_SAME_OPEN_ITEM_A1 + i,
                         f0333Slots);
    }
    for (; i < DM1_PC34_CHEST_SAME_OPEN_SLOT_COUNT; ++i) {
        ok &= expect_int("first open empty tail",
                         probe->firstOpenSlotTypes[i], 0, f0333Slots);
    }
    ok &= expect_int("first open visible weight",
                     probe->firstOpenVisibleWeight, 100, f0333Slots);
    return ok;
}

static int test_pickup_sparse_panel(
    const DM1_V1_ChestSameOpenNoopProbePc34* probe)
{
    const char* f0302 = "ReDMCSB CHAMPION.C F0302 lines 688-710";
    int ok = 1;

    ok &= expect_int("pickup C538 result", probe->pickupC538Result, 1,
                     f0302);
    ok &= expect_int("leader hand after C538 pickup",
                     probe->leaderHandAfterPickup,
                     DM1_PC34_CHEST_SAME_OPEN_ITEM_A1 + 1, f0302);
    ok &= expect_int("C538 empty after pickup", probe->c538AfterPickup, 0,
                     f0302);
    ok &= expect_int("visible weight after pickup",
                     probe->visibleWeightAfterPickup, 80, f0302);
    return ok;
}

static int test_same_open_noop(
    const DM1_V1_ChestSameOpenNoopProbePc34* probe)
{
    const char* f0333Guard = "ReDMCSB CHEST.C F0333 lines 30-32";
    const char* f0334 = "ReDMCSB CHEST.C F0334 lines 113-132";
    int ok = 1;

    ok &= expect_int("same open result", probe->sameOpenResult, 1,
                     f0333Guard);
    ok &= expect_int("same open keeps G0426",
                     probe->openThingAfterSameOpen,
                     DM1_PC34_CHEST_SAME_OPEN_THING, f0333Guard);
    ok &= expect_int("same open preserves leader hand",
                     probe->leaderHandAfterSameOpen,
                     DM1_PC34_CHEST_SAME_OPEN_ITEM_A1 + 1,
                     f0333Guard);
    ok &= expect_int("same open keeps C537 item",
                     probe->c537AfterSameOpen,
                     DM1_PC34_CHEST_SAME_OPEN_ITEM_A1,
                     f0333Guard);
    ok &= expect_int("same open keeps sparse C538 empty",
                     probe->c538AfterSameOpen, 0, f0333Guard);
    ok &= expect_int("same open keeps C539 item",
                     probe->c539AfterSameOpen,
                     DM1_PC34_CHEST_SAME_OPEN_ITEM_A1 + 2,
                     f0333Guard);
    ok &= expect_int("same open keeps C540 item",
                     probe->c540AfterSameOpen,
                     DM1_PC34_CHEST_SAME_OPEN_ITEM_A1 + 3,
                     f0333Guard);
    ok &= expect_int("replacing helper stale panel setup",
                     probe->panelContentBeforeReplacingSameOpen,
                     DM1_PC34_PANEL_SCROLL,
                     "ReDMCSB CHEST.C F0333 line 28");
    ok &= expect_int("replacing helper same-open result",
                     probe->replacingSameOpenResult, 0, f0333Guard);
    ok &= expect_int("replacing helper restores chest panel",
                     probe->panelContentAfterReplacingSameOpen,
                     DM1_PC34_PANEL_CHEST,
                     "ReDMCSB CHEST.C F0333 line 28");
    ok &= expect_int("replacing helper same-open keeps G0426",
                     probe->openThingAfterReplacingSameOpen,
                     DM1_PC34_CHEST_SAME_OPEN_THING, f0333Guard);
    ok &= expect_int("replacing helper keeps sparse C538 empty",
                     probe->c538AfterReplacingSameOpen, 0, f0333Guard);
    ok &= expect_int("same open leaks no alternate chain",
                     probe->bItemsLeakedAfterSameOpen, 0, f0333Guard);
    ok &= expect_int("same open preserves visible weight",
                     probe->visibleWeightAfterSameOpen,
                     probe->visibleWeightAfterPickup, f0333Guard);
    ok &= expect_int("same open preserves load",
                     probe->loadAfterSameOpen,
                     probe->loadAfterPickup, f0333Guard);
    ok &= expect_int("final close sees three visible items",
                     probe->closeCountAfterSameOpen, 3, f0334);
    ok &= expect_int("final close first item",
                     probe->closedItemTypes[0],
                     DM1_PC34_CHEST_SAME_OPEN_ITEM_A1, f0334);
    ok &= expect_int("final close second item skips picked C538",
                     probe->closedItemTypes[1],
                     DM1_PC34_CHEST_SAME_OPEN_ITEM_A1 + 2, f0334);
    ok &= expect_int("final close third item",
                     probe->closedItemTypes[2],
                     DM1_PC34_CHEST_SAME_OPEN_ITEM_A1 + 3, f0334);
    ok &= expect_int("final close no alternate item",
                     probe->closedItemTypes[3], 0, f0334);
    return ok;
}

int main(void)
{
    const char* f0333Guard = "ReDMCSB CHEST.C F0333 lines 30-32";
    DM1_V1_ChestSameOpenNoopProbePc34 probe;
    int ok = 1;

    printf("probe=dm1_v1_chest_same_open_noop_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_chest_same_open_noop_source_evidence_pc34());

    ok &= expect_int("probe run",
                     dm1_v1_chest_same_open_noop_run_pc34(&probe),
                     1, f0333Guard);
    if (!ok) {
        printf("assertionCount=%d\n", g_assertions);
        printf("chestSameOpenNoopInvariantOk=0\n");
        return 1;
    }

    ok &= test_source_evidence();
    ok &= test_first_open(&probe);
    ok &= test_pickup_sparse_panel(&probe);
    ok &= test_same_open_noop(&probe);
    ok &= expect_int("minimum assertion count",
                     g_assertions >= 40 ? 1 : 0, 1, f0333Guard);

    printf("assertionCount=%d\n", g_assertions);
    printf("chestSameOpenNoopInvariantOk=%d\n", ok ? 1 : 0);
    return ok ? 0 : 1;
}
