#include "dm1_v1_chest_reopen_cross_champion_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static DM1_V1_ChestReopenCrossChampionProbePc34 g_probe;
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

static int expect_order(const char* label,
                        const int* types,
                        int count,
                        int firstType,
                        const char* redmcsbAnchor)
{
    int ok = 1;
    int i;

    for (i = 0; i < count; ++i) {
        char slotLabel[96];

        snprintf(slotLabel, sizeof(slotLabel), "%s slot %d", label, i);
        ok &= expect_int(slotLabel, types[i], firstType + i, redmcsbAnchor);
    }
    return ok;
}

static int test_spec(void)
{
    const char* f0333 = "ReDMCSB CHEST.C F0333 lines 31-67";
    const DM1_V1_ChestReopenCrossChampionSpecPc34* spec =
        M11_GameView_ChestReopenCrossChampionSpecPc34();
    int ok = 1;

    ok &= expect_str("contract marker", spec->contractMarker,
                     "Source-locked contract gate only; not full real-asset chest runtime parity.",
                     f0333);
    ok &= expect_int("contract marker flag", g_probe.contractOnly, 1, f0333);
    ok &= expect_int("leader A index", spec->leaderAIndex,
                     DM1_PC34_CHEST_REOPEN_CROSS_LEADER_A, f0333);
    ok &= expect_int("leader B index", spec->leaderBIndex,
                     DM1_PC34_CHEST_REOPEN_CROSS_LEADER_B, f0333);
    ok &= expect_int("C537 slot constant", spec->c537Pc34Slot,
                     DM1_PC34_SLOT_CHEST_1, f0333);
    ok &= expect_int("C544 slot constant", spec->c544Pc34Slot,
                     DM1_PC34_SLOT_CHEST_8, f0333);
    ok &= expect_int("chest slot count", spec->chestSlotCount,
                     DM1_PC34_CHEST_SLOT_COUNT, f0333);
    ok &= expect_int("leader hand sentinel", spec->leaderHandItem,
                     DM1_PC34_CHEST_REOPEN_CROSS_HAND_ITEM, f0333);
    ok &= expect_int("leader hand weight", spec->leaderHandWeight,
                     DM1_PC34_CHEST_REOPEN_CROSS_HAND_WEIGHT, f0333);
    return ok;
}

static int test_cross_champion_reopen(void)
{
    const char* f0333 = "ReDMCSB CHEST.C F0333 lines 31-67";
    const char* f0334 = "ReDMCSB CHEST.C F0334 lines 113-132";
    const char* f0297f0298 =
        "ReDMCSB CHAMPION.C F0297/F0298 lines 250-298";
    const char* f0300f0301 =
        "ReDMCSB CHAMPION.C F0300/F0301 lines 511-515,606-615";
    const char* f0309f0310 =
        "ReDMCSB CHAMPION.C F0309/F0310 lines 1157-1205";
    const char* f0368 = "ReDMCSB CLIKCHAM.C F0368 lines 54-72";
    const char* f0140 = "ReDMCSB DUNGEON.C F0140 lines 1114-1120";
    const char* f0163 = "ReDMCSB DUNGEON.C F0163 lines 1796-1837";
    int ok = 1;

    ok &= expect_int("leader A base slot setup",
                     g_probe.leaderABaseSetResult, 1, f0300f0301);
    ok &= expect_int("leader B base slot setup",
                     g_probe.leaderBBaseSetResult, 1, f0300f0301);
    ok &= expect_int("leader A base load",
                     g_probe.leaderABaseLoad, 17, f0309f0310);
    ok &= expect_int("leader B base load",
                     g_probe.leaderBBaseLoad, 23, f0309f0310);
    ok &= expect_int("leader A hand setup",
                     g_probe.leaderAHandSetupResult, 1, f0297f0298);
    ok &= expect_int("leader A hand before switch type",
                     g_probe.leaderAHandBeforeSwitchType,
                     DM1_PC34_CHEST_REOPEN_CROSS_HAND_ITEM, f0297f0298);
    ok &= expect_int("leader A hand before switch weight",
                     g_probe.leaderAHandBeforeSwitchWeight,
                     DM1_PC34_CHEST_REOPEN_CROSS_HAND_WEIGHT, f0297f0298);

    ok &= expect_int("leader A chest opens",
                     g_probe.chestAOpenResult, 1, f0333);
    ok &= expect_int("leader A open thing",
                     g_probe.chestAOpenThing,
                     DM1_PC34_CHEST_REOPEN_CROSS_A_THING, f0333);
    ok &= expect_order("leader A opened order",
                       g_probe.chestAOpenedTypes,
                       DM1_PC34_CHEST_REOPEN_CROSS_A_COUNT,
                       DM1_PC34_CHEST_REOPEN_CROSS_A_FIRST, f0333);
    ok &= expect_int("leader A visible weight after open",
                     g_probe.chestAOpenedVisibleWeight, 18, f0140);
    ok &= expect_int("leader A panel load after open",
                     g_probe.leaderAPanelLoadAfterOpen, 48, f0309f0310);

    ok &= expect_int("leader A close count",
                     g_probe.chestACloseCount,
                     DM1_PC34_CHEST_REOPEN_CROSS_A_COUNT, f0334);
    ok &= expect_order("leader A close rewrite order",
                       g_probe.chestAClosedTypes,
                       DM1_PC34_CHEST_REOPEN_CROSS_A_COUNT,
                       DM1_PC34_CHEST_REOPEN_CROSS_A_FIRST, f0163);
    ok &= expect_int("leader A close container snapshot",
                     g_probe.chestACloseContainerSnapshot, 68, f0140);
    ok &= expect_int("leader A panel load after close",
                     g_probe.leaderAPanelLoadAfterClose, 30, f0309f0310);

    ok &= expect_int("leader switch result",
                     g_probe.leaderSwitchResult, 1, f0368);
    ok &= expect_int("leader switch previous",
                     g_probe.leaderSwitchPrevious,
                     DM1_PC34_CHEST_REOPEN_CROSS_LEADER_A, f0368);
    ok &= expect_int("leader switch new",
                     g_probe.leaderSwitchNew,
                     DM1_PC34_CHEST_REOPEN_CROSS_LEADER_B, f0368);
    ok &= expect_int("leader A state load after switch",
                     g_probe.leaderAStateLoadAfterSwitch,
                     g_probe.leaderABaseLoad, f0368);
    ok &= expect_int("leader B state load after switch",
                     g_probe.leaderBStateLoadAfterSwitch,
                     g_probe.leaderBBaseLoad +
                         DM1_PC34_CHEST_REOPEN_CROSS_HAND_WEIGHT,
                     f0368);
    ok &= expect_int("leader A hand cleared after switch type",
                     g_probe.leaderAHandAfterSwitchType, 0, f0297f0298);
    ok &= expect_int("leader A hand cleared flag",
                     g_probe.leaderAHandClearedAfterSwitch, 1, f0297f0298);
    ok &= expect_int("leader B hand rebuilt type",
                     g_probe.leaderBHandAfterSwitchType,
                     DM1_PC34_CHEST_REOPEN_CROSS_HAND_ITEM, f0297f0298);
    ok &= expect_int("leader B hand rebuilt weight",
                     g_probe.leaderBHandAfterSwitchWeight,
                     DM1_PC34_CHEST_REOPEN_CROSS_HAND_WEIGHT, f0297f0298);
    ok &= expect_int("leader B hand occupied",
                     g_probe.leaderBHandOccupiedAfterSwitch, 1, f0297f0298);
    ok &= expect_int("leader B panel load after switch",
                     g_probe.leaderBPanelLoadAfterSwitch, 36, f0309f0310);

    ok &= expect_int("leader B chest reopens",
                     g_probe.chestBReopenResult, 1, f0333);
    ok &= expect_int("leader B open thing",
                     g_probe.chestBOpenThing,
                     DM1_PC34_CHEST_REOPEN_CROSS_B_THING, f0333);
    ok &= expect_int("leader B reopen visible count",
                     g_probe.chestBReopenedVisibleCount,
                     DM1_PC34_CHEST_REOPEN_CROSS_B_COUNT, f0333);
    ok &= expect_order("leader B reopened order",
                       g_probe.chestBReopenedTypes,
                       DM1_PC34_CHEST_REOPEN_CROSS_B_COUNT,
                       DM1_PC34_CHEST_REOPEN_CROSS_B_FIRST, f0333);
    ok &= expect_int("leader B order matches B snapshot",
                     g_probe.chestBOrderMatchesLeaderB, 1, f0333);
    ok &= expect_int("leader B order does not leak A snapshot",
                     g_probe.chestBOrderLeaksLeaderA, 0, f0333);
    ok &= expect_int("leader B visible weight after reopen",
                     g_probe.chestBReopenedVisibleWeight, 50, f0140);
    ok &= expect_int("leader B panel load after reopen",
                     g_probe.leaderBPanelLoadAfterReopen, 86, f0309f0310);
    ok &= expect_int("leader B reopen load delta",
                     g_probe.leaderBReopenLoadDelta,
                     g_probe.chestBReopenedVisibleWeight, f0309f0310);
    ok &= expect_int("reopen does not double count linked weights",
                     g_probe.reopenDoesNotDoubleCountLinkWeights, 1,
                     f0309f0310);

    ok &= expect_int("leader B close count",
                     g_probe.chestBCloseCount,
                     DM1_PC34_CHEST_REOPEN_CROSS_B_COUNT, f0334);
    ok &= expect_order("leader B close rewrite order",
                       g_probe.chestBClosedTypes,
                       DM1_PC34_CHEST_REOPEN_CROSS_B_COUNT,
                       DM1_PC34_CHEST_REOPEN_CROSS_B_FIRST, f0163);
    ok &= expect_int("leader B close container snapshot",
                     g_probe.chestBCloseContainerSnapshot, 100, f0140);
    ok &= expect_int("container snapshot weight consistent",
                     g_probe.containerSnapshotWeightConsistent, 1, f0140);
    ok &= expect_int("leader B panel load after close",
                     g_probe.leaderBPanelLoadAfterClose,
                     g_probe.leaderBPanelLoadAfterSwitch, f0309f0310);
    return ok;
}

int main(void)
{
    const char* f0333 = "ReDMCSB CHEST.C F0333 lines 31-67";
    int ok = 1;

    printf("probe=dm1_v1_chest_reopen_cross_champion_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           M11_GameView_ChestReopenCrossChampionSourceEvidencePc34());

    ok &= expect_int("probe setup",
                     M11_GameView_ChestReopenCrossChampionRunPc34(&g_probe),
                     1, f0333);
    if (!ok) {
        printf("chestReopenCrossChampionResultOk=0 leaderA_load=%d "
               "leaderB_load=%d containerSnapshot=%d closeCount=%d "
               "reopenCount=%d\n",
               g_probe.leaderAStateLoadAfterSwitch,
               g_probe.leaderBPanelLoadAfterReopen,
               g_probe.chestBCloseContainerSnapshot,
               g_probe.chestACloseCount,
               g_probe.chestBReopenedVisibleCount);
        return 1;
    }

    ok &= test_spec();
    ok &= test_cross_champion_reopen();
    ok &= expect_int("minimum assertion count",
                     g_assertions >= 55 ? 1 : 0, 1, f0333);

    printf("assertionCount=%d\n", g_assertions);
    printf("chestReopenCrossChampionResultOk=%d leaderA_load=%d "
           "leaderB_load=%d containerSnapshot=%d closeCount=%d "
           "reopenCount=%d\n",
           ok ? 1 : 0,
           g_probe.leaderAStateLoadAfterSwitch,
           g_probe.leaderBPanelLoadAfterReopen,
           g_probe.chestBCloseContainerSnapshot,
           g_probe.chestACloseCount,
           g_probe.chestBReopenedVisibleCount);
    return ok ? 0 : 1;
}
