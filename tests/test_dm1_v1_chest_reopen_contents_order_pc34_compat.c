#include "dm1_v1_chest_reopen_contents_order_pc34_compat.h"

#include <stdio.h>

static M11_GameView_ChestReopenContentsOrderProbePc34 g_probe;
static int g_assertions;

static int expect_int(const char* label, int got, int want,
                      const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n", label, got, want,
               redmcsbAnchor);
        return 0;
    }
    printf("ok %s=%d anchor=%s\n", label, got, redmcsbAnchor);
    return 1;
}

static int test_probe_spec(void)
{
    const char* f0333Open = "ReDMCSB CHEST.C F0333 lines 53-67";
    int ok = 1;

    ok &= expect_int("contract-only marker",
                     g_probe.sourceLockedContractOnly, 1, f0333Open);
    ok &= expect_int("C537 slot constant",
                     g_probe.c537Pc34Slot, DM1_PC34_SLOT_CHEST_1, f0333Open);
    ok &= expect_int("C544 slot constant",
                     g_probe.c544Pc34Slot, DM1_PC34_SLOT_CHEST_8, f0333Open);
    ok &= expect_int("chest slot count",
                     g_probe.chestSlotCount, DM1_PC34_CHEST_SLOT_COUNT,
                     f0333Open);
    return ok;
}

static int test_single_item_round_trip(void)
{
    const char* f0333Open = "ReDMCSB CHEST.C F0333 lines 53-67";
    const char* f0334Close = "ReDMCSB CHEST.C F0334 lines 117-132";
    const M11_GameView_ChestReopenContentsOrderCasePc34* c =
        &g_probe.cases[DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_CASE_ONE];
    int ok = 1;

    ok &= expect_int("case 1 opens",
                     c->openResult, 1, f0333Open);
    ok &= expect_int("case 1 closes one visible item",
                     c->closeCount, 1, f0334Close);
    ok &= expect_int("case 1 reopens",
                     c->reopenResult, 1, f0333Open);
    ok &= expect_int("case 1 reopened visible count",
                     c->reopenedVisibleCount, 1, f0333Open);
    ok &= expect_int("case 1 original head",
                     c->originalHead, 701, f0333Open);
    ok &= expect_int("case 1 closed head is original item",
                     c->closedHead, c->originalHead, f0334Close);
    ok &= expect_int("case 1 closed tail is original item",
                     c->closedTail, c->originalTail, f0334Close);
    ok &= expect_int("case 1 reopened head is original item",
                     c->reopenedHead, c->originalHead, f0333Open);
    ok &= expect_int("case 1 reopened tail is original item",
                     c->reopenedTail, c->originalTail, f0333Open);
    ok &= expect_int("case 1 leader hand remains empty",
                     c->leaderHandAfterReopen, 0, f0333Open);
    return ok;
}

static int test_three_item_head_middle_tail_order(void)
{
    const char* f0333Open = "ReDMCSB CHEST.C F0333 lines 53-67";
    const char* f0334Close = "ReDMCSB CHEST.C F0334 lines 117-132";
    const char* f0163Next = "ReDMCSB DUNGEON.C F0163 lines 1796-1837";
    const M11_GameView_ChestReopenContentsOrderCasePc34* c =
        &g_probe.cases[DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_CASE_THREE];
    int ok = 1;

    ok &= expect_int("case 2 opens",
                     c->openResult, 1, f0333Open);
    ok &= expect_int("case 2 closes three visible items",
                     c->closeCount, 3, f0334Close);
    ok &= expect_int("case 2 reopens",
                     c->reopenResult, 1, f0333Open);
    ok &= expect_int("case 2 original head weapon",
                     c->originalHead,
                     DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_WEAPON, f0333Open);
    ok &= expect_int("case 2 original middle potion",
                     c->originalMiddle,
                     DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_POTION, f0333Open);
    ok &= expect_int("case 2 original tail junk",
                     c->originalTail,
                     DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_JUNK, f0333Open);
    ok &= expect_int("case 2 closed head keeps weapon",
                     c->closedHead, c->originalHead, f0334Close);
    ok &= expect_int("case 2 closed middle keeps potion",
                     c->closedMiddle, c->originalMiddle, f0163Next);
    ok &= expect_int("case 2 closed tail keeps junk",
                     c->closedTail, c->originalTail, f0163Next);
    ok &= expect_int("case 2 reopened head keeps weapon",
                     c->reopenedHead, c->originalHead, f0333Open);
    ok &= expect_int("case 2 reopened middle keeps potion",
                     c->reopenedMiddle, c->originalMiddle, f0333Open);
    ok &= expect_int("case 2 reopened tail keeps junk",
                     c->reopenedTail, c->originalTail, f0333Open);
    ok &= expect_int("case 2 no dropped or duplicated visible items",
                     c->noDroppedOrDuplicatedVisibleItems, 1, f0163Next);
    return ok;
}

static int test_full_eight_slots_round_trip(void)
{
    const char* f0333Open = "ReDMCSB CHEST.C F0333 lines 53-67";
    const char* f0334Close = "ReDMCSB CHEST.C F0334 lines 117-132";
    const char* f0163Next = "ReDMCSB DUNGEON.C F0163 lines 1796-1837";
    const M11_GameView_ChestReopenContentsOrderCasePc34* c =
        &g_probe.cases[DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_CASE_FULL];
    int ok = 1;
    int i;

    ok &= expect_int("case 3 opens",
                     c->openResult, 1, f0333Open);
    ok &= expect_int("case 3 closes eight visible items",
                     c->closeCount, DM1_PC34_CHEST_SLOT_COUNT, f0334Close);
    ok &= expect_int("case 3 reopens",
                     c->reopenResult, 1, f0333Open);
    ok &= expect_int("case 3 reopened visible count",
                     c->reopenedVisibleCount, DM1_PC34_CHEST_SLOT_COUNT,
                     f0333Open);
    for (i = 0; i < DM1_PC34_CHEST_SLOT_COUNT; ++i) {
        ok &= expect_int("case 3 input slot order",
                         c->inputTypes[i], 900 + i, f0333Open);
        ok &= expect_int("case 3 opened C537-C544 order",
                         c->openedTypes[i], c->inputTypes[i], f0333Open);
        ok &= expect_int("case 3 closed C537-C544 order",
                         c->closedTypes[i], c->inputTypes[i], f0334Close);
        ok &= expect_int("case 3 reopened C537-C544 order",
                         c->reopenedTypes[i], c->inputTypes[i], f0333Open);
    }
    ok &= expect_int("case 3 every visible input returns",
                     c->reopenedContainsEveryVisibleInput, 1, f0163Next);
    ok &= expect_int("case 3 unique visible count",
                     c->reopenedUniqueVisibleCount,
                     DM1_PC34_CHEST_SLOT_COUNT, f0163Next);
    ok &= expect_int("case 3 no dropped or duplicated items",
                     c->noDroppedOrDuplicatedVisibleItems, 1, f0163Next);
    return ok;
}

static int test_hidden_tail_excluded_after_reopen(void)
{
    const char* f0333Open = "ReDMCSB CHEST.C F0333 lines 53-67";
    const char* f0334Close = "ReDMCSB CHEST.C F0334 lines 117-132";
    const M11_GameView_ChestReopenContentsOrderCasePc34* c =
        &g_probe.cases[DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_CASE_HIDDEN_TAIL];
    int ok = 1;

    ok &= expect_int("case 4 hidden tail input",
                     c->inputTypes[8],
                     DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_HIDDEN_TAIL,
                     f0333Open);
    ok &= expect_int("case 4 closes only visible slots",
                     c->closeCount, DM1_PC34_CHEST_SLOT_COUNT, f0334Close);
    ok &= expect_int("case 4 closed excludes hidden tail",
                     c->closedContainsHiddenTail, 0, f0334Close);
    ok &= expect_int("case 4 reopens visible list",
                     c->reopenResult, 1, f0333Open);
    ok &= expect_int("case 4 reopened excludes hidden tail",
                     c->reopenedContainsHiddenTail, 0, f0333Open);
    ok &= expect_int("case 4 reopened visible head",
                     c->reopenedHead, 800, f0333Open);
    ok &= expect_int("case 4 reopened visible tail",
                     c->reopenedTail, 807, f0333Open);
    ok &= expect_int("case 4 no dropped or duplicated visible items",
                     c->noDroppedOrDuplicatedVisibleItems, 1, f0334Close);
    return ok;
}

static int test_leader_hand_helmet_preserved(void)
{
    const char* f0333Open = "ReDMCSB CHEST.C F0333 lines 53-67";
    const char* f0334Close = "ReDMCSB CHEST.C F0334 lines 117-132";
    const char* f0302Leader = "ReDMCSB CHAMPION.C F0302 lines 688-710";
    const M11_GameView_ChestReopenContentsOrderCasePc34* c =
        &g_probe.cases[
            DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_CASE_LEADER_HELMET];
    int ok = 1;

    ok &= expect_int("case 5 leader hand starts with helmet",
                     c->leaderHandBeforeOpen,
                     DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_HELMET,
                     f0302Leader);
    ok &= expect_int("case 5 leader hand unchanged by open",
                     c->leaderHandAfterOpen,
                     DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_HELMET,
                     f0333Open);
    ok &= expect_int("case 5 closes three chest items",
                     c->closeCount, 3, f0334Close);
    ok &= expect_int("case 5 leader hand unchanged by close",
                     c->leaderHandAfterClose,
                     DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_HELMET,
                     f0334Close);
    ok &= expect_int("case 5 reopens",
                     c->reopenResult, 1, f0333Open);
    ok &= expect_int("case 5 leader hand unchanged by reopen",
                     c->leaderHandAfterReopen,
                     DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_HELMET,
                     f0333Open);
    ok &= expect_int("case 5 leader hand stable through cycle",
                     c->leaderHandUnchangedAcrossCycle, 1, f0302Leader);
    ok &= expect_int("case 5 reopened head order",
                     c->reopenedHead, c->originalHead, f0333Open);
    ok &= expect_int("case 5 reopened middle order",
                     c->reopenedMiddle, c->originalMiddle, f0333Open);
    ok &= expect_int("case 5 reopened tail order",
                     c->reopenedTail, c->originalTail, f0333Open);
    ok &= expect_int("case 5 no dropped or duplicated items",
                     c->noDroppedOrDuplicatedVisibleItems, 1, f0334Close);
    return ok;
}

int main(void)
{
    const char* f0333Open = "ReDMCSB CHEST.C F0333 lines 53-67";
    int ok = 1;

    printf("probe=dm1_v1_chest_reopen_contents_order_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           M11_GameView_ChestReopenContentsOrderSourceEvidencePc34());

    ok &= expect_int("probe setup",
                     M11_GameView_ChestReopenContentsOrderRunPc34(&g_probe),
                     1, f0333Open);
    if (!ok) {
        printf("assertionCount=%d\n", g_assertions);
        printf("chestReopenContentsOrderInvariantOk=0\n");
        return 1;
    }

    ok &= test_probe_spec();
    ok &= test_single_item_round_trip();
    ok &= test_three_item_head_middle_tail_order();
    ok &= test_full_eight_slots_round_trip();
    ok &= test_hidden_tail_excluded_after_reopen();
    ok &= test_leader_hand_helmet_preserved();

    printf("assertionCount=%d\n", g_assertions);
    printf("chestReopenContentsOrderInvariantOk=%d\n", ok ? 1 : 0);
    return ok ? 0 : 1;
}
