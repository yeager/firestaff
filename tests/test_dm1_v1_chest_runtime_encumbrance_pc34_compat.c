#include "dm1_v1_chest_runtime_encumbrance_pc34_compat.h"

#include <stdio.h>

static DM1_V1_ChestRuntimeEncumbranceProbePc34 g_probe;

static int expect_int(const char* label,
                      int got,
                      int want,
                      const char* redmcsbAnchor)
{
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n",
               label, got, want, redmcsbAnchor);
        return 0;
    }
    printf("ok %s=%d anchor=%s\n", label, got, redmcsbAnchor);
    return 1;
}

static int test_open_runtime_load(void)
{
    const char* f0333 =
        "ReDMCSB CHEST.C F0333 lines 53-76";
    const char* f0301 =
        "ReDMCSB CHAMPION.C F0301 lines 609-615";
    const char* f0309F0310 =
        "ReDMCSB CHAMPION.C F0309/F0310 lines 1157-1205";
    int ok = 1;

    ok &= expect_int("leader base slot setup",
                     g_probe.leaderBaseSetResult, 1, f0301);
    ok &= expect_int("bystander base slot setup",
                     g_probe.bystanderBaseSetResult, 1, f0301);
    ok &= expect_int("leader base load",
                     g_probe.leaderBaseLoad, 15, f0309F0310);
    ok &= expect_int("bystander base load",
                     g_probe.bystanderLoadBeforeOpen, 77, f0309F0310);
    ok &= expect_int("runtime chest open",
                     g_probe.openResult, 1, f0333);
    ok &= expect_int("open chest thing set",
                     g_probe.openChestThingAfterOpen, 0x6E71, f0333);
    ok &= expect_int("visible chest contents weight",
                     g_probe.visibleContentsWeight, 60, f0333);
    ok &= expect_int("leader load includes open chest contents",
                     g_probe.leaderLoadAfterOpen, 75, f0309F0310);

    return ok;
}

static int test_close_runtime_encumbrance(void)
{
    const char* f0140 =
        "ReDMCSB DUNGEON.C F0140 lines 1114-1120";
    const char* f0334 =
        "ReDMCSB CHEST.C F0334 lines 113-132";
    const char* f0300F0301 =
        "ReDMCSB CHAMPION.C F0300/F0301 lines 582-615";
    const char* f0309F0310 =
        "ReDMCSB CHAMPION.C F0309/F0310 lines 1157-1205";
    int ok = 1;

    ok &= expect_int("open container base plus contents weight",
                     g_probe.openContainerWeight, 110, f0140);
    ok &= expect_int("close compacts three visible links",
                     g_probe.closeCount, 3, f0334);
    ok &= expect_int("close snapshots container weight before reset",
                     g_probe.closeContainerWeightSnapshot, 110, f0334);
    ok &= expect_int("closed link 0 type",
                     g_probe.closedItemTypes[0], 0x5101, f0334);
    ok &= expect_int("closed link 1 weight",
                     g_probe.closedItemWeights[1], 20, f0334);
    ok &= expect_int("closed link 2 type",
                     g_probe.closedItemTypes[2], 0x5103, f0334);
    ok &= expect_int("closed link 3 remains empty",
                     g_probe.closedItemTypes[3], 0, f0334);
    ok &= expect_int("close clears open container weight",
                     g_probe.closeContainerWeightAfter, 0, f0140);
    ok &= expect_int("close clears open chest thing",
                     g_probe.openChestThingAfterClose, 0, f0334);
    ok &= expect_int("leader load drops to base after close",
                     g_probe.leaderLoadAfterClose, g_probe.leaderBaseLoad,
                     f0300F0301);
    ok &= expect_int("leader encumbrance delta removed",
                     g_probe.leaderLoadAfterOpen - g_probe.leaderLoadAfterClose,
                     g_probe.visibleContentsWeight, f0309F0310);
    ok &= expect_int("bystander load unchanged by leader close",
                     g_probe.bystanderLoadAfterClose,
                     g_probe.bystanderLoadBeforeOpen, f0309F0310);

    return ok;
}

int main(void)
{
    const char* f0334 =
        "ReDMCSB CHEST.C F0334 lines 113-132";
    int ok = 1;

    printf("probe=dm1_v1_chest_runtime_encumbrance_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_chest_runtime_encumbrance_source_evidence_pc34());

    ok &= expect_int("probe setup",
                     dm1_v1_chest_runtime_encumbrance_run_pc34(&g_probe),
                     1, f0334);
    if (!ok) {
        printf("chestRuntimeEncumbranceResultOk=0\n");
        return 1;
    }

    ok &= test_open_runtime_load();
    ok &= test_close_runtime_encumbrance();

    printf("chestRuntimeEncumbranceResultOk=%d baseLoad=%d openLoad=%d "
           "closeLoad=%d visibleWeight=%d containerSnapshot=%d closeCount=%d "
           "bystanderLoad=%d\n",
           ok ? 1 : 0,
           g_probe.leaderBaseLoad,
           g_probe.leaderLoadAfterOpen,
           g_probe.leaderLoadAfterClose,
           g_probe.visibleContentsWeight,
           g_probe.closeContainerWeightSnapshot,
           g_probe.closeCount,
           g_probe.bystanderLoadAfterClose);
    return ok ? 0 : 1;
}
