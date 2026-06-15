#include "dm1_v1_chest_multi_champion_close_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/*
 * Non-overlap: this is a three-champion close/isolation gate. It does not
 * assert real-asset icon parity or original DOS pixel parity, and it does not
 * duplicate same-leader reopen, cross-champion reopen, stale-click, hidden-tail,
 * or single-leader encumbrance gates.
 */

static DM1_V1_ChestMultiChampionCloseProbePc34 g_probe;
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

static int test_spec_and_masks(void)
{
    const char* f0333 = "ReDMCSB CHEST.C F0333 lines 31-67";
    const char* dataMasks =
        "ReDMCSB DATA.C G0038 lines 1049-1087";
    const char* staff =
        "ReDMCSB DUNGEON.C object info line 108 and CHAMPION.C F0302 lines 688-699";
    const DM1_V1_ChestMultiChampionCloseSpecPc34* spec =
        dm1_v1_chest_multi_champion_close_spec_pc34();
    int ok = 1;

    ok &= expect_str("contract marker",
                     spec->contractMarker,
                     "Source-locked deterministic contract gate only; no real-asset icon or DOS pixel parity claim.",
                     f0333);
    ok &= expect_int("contract marker flag", g_probe.contractOnly, 1, f0333);
    ok &= expect_int("champion count", spec->championCount,
                     DM1_PC34_CHEST_MULTI_CHAMPION_COUNT, f0333);
    ok &= expect_int("C537 slot", spec->c537Pc34Slot,
                     DM1_PC34_SLOT_CHEST_1, dataMasks);
    ok &= expect_int("C540 slot", spec->c540Pc34Slot,
                     DM1_PC34_SLOT_CHEST_4, dataMasks);
    ok &= expect_int("C544 slot", spec->c544Pc34Slot,
                     DM1_PC34_SLOT_CHEST_8, dataMasks);
    ok &= expect_int("C537 mask", g_probe.c537Mask,
                     DM1_PC34_ALLOWED_CONTAINER, dataMasks);
    ok &= expect_int("C540 mask", g_probe.c540Mask,
                     DM1_PC34_ALLOWED_CONTAINER, dataMasks);
    ok &= expect_int("C544 mask", g_probe.c544Mask,
                     DM1_PC34_ALLOWED_CONTAINER, dataMasks);
    ok &= expect_int("Staff Of Claws allowed slots",
                     g_probe.staffOfClawsAllowedSlots,
                     DM1_PC34_ALLOWED_QUIVER_LINE1, staff);
    ok &= expect_int("Staff Of Claws rejected from chest",
                     g_probe.staffRejectedFromChest, 1, staff);
    ok &= expect_int("Staff Of Claws accepted in quiver",
                     g_probe.staffAcceptedInQuiver, 1, staff);
    return ok;
}

static int test_champion_a(void)
{
    const char* f0333 = "ReDMCSB CHEST.C F0333 lines 31-67";
    const char* f0334 = "ReDMCSB CHEST.C F0334 lines 117-132";
    const char* f0140 = "ReDMCSB DUNGEON.C F0140 lines 1114-1120";
    const char* f0163 = "ReDMCSB DUNGEON.C F0163 lines 1796-1837";
    const char* f0302 = "ReDMCSB CHAMPION.C F0302 lines 688-710";
    const char* f0309 = "ReDMCSB CHAMPION.C F0309/F0310 lines 1157-1205";
    const char* f0297 = "ReDMCSB CHAMPION.C F0297/F0300/F0301 lines 243-268,489-615";
    int c = 0;
    int ok = 1;

    ok &= expect_int("A base load", g_probe.baseLoads[c], 19, f0309);
    ok &= expect_int("A target C537", g_probe.targetPc34Slots[c],
                     DM1_PC34_SLOT_CHEST_1, f0302);
    ok &= expect_int("A open result", g_probe.openResults[c], 1, f0333);
    ok &= expect_int("A visible weight after open",
                     g_probe.visibleWeightsAfterOpen[c], 12, f0140);
    ok &= expect_int("A load after open", g_probe.loadsAfterOpen[c], 31,
                     f0309);
    ok &= expect_int("A movement ticks after open",
                     g_probe.movementTicksAfterOpen[c], 2, f0309);
    ok &= expect_int("A displaced type", g_probe.displacedTypes[c], 0x7401,
                     f0302);
    ok &= expect_int("A click result", g_probe.clickResults[c], 1, f0302);
    ok &= expect_int("A hand receives displaced item",
                     g_probe.handAfterTypes[c], 0x7401, f0302);
    ok &= expect_int("A C537 receives hand item",
                     g_probe.slotAfterTypes[c], 0x7301, f0302);
    ok &= expect_int("A visible weight after click",
                     g_probe.visibleWeightsAfterClick[c], 48, f0140);
    ok &= expect_int("A load after click", g_probe.loadsAfterClick[c], 67,
                     f0297);
    ok &= expect_int("A movement ticks after click",
                     g_probe.movementTicksAfterClick[c], 4, f0309);
    ok &= expect_int("A close count", g_probe.closeCounts[c], 2, f0334);
    ok &= expect_int("A close snapshot",
                     g_probe.closeContainerSnapshots[c], 98, f0140);
    ok &= expect_int("A closed target identity",
                     g_probe.replacementClosedAtTarget[c], 1, f0163);
    ok &= expect_int("A displaced absent from closed links",
                     g_probe.displacedAbsentFromClosed[c], 1, f0163);
    ok &= expect_int("A open thing cleared",
                     g_probe.openThingsAfterClose[c], 0, f0334);
    ok &= expect_int("A close load returns to base",
                     g_probe.loadsAfterClose[c], 19, f0297);
    ok &= expect_int("A movement ticks after close",
                     g_probe.movementTicksAfterClose[c], 2, f0309);
    return ok;
}

static int test_champion_b(void)
{
    const char* f0333 = "ReDMCSB CHEST.C F0333 lines 31-67";
    const char* f0334 = "ReDMCSB CHEST.C F0334 lines 117-132";
    const char* f0140 = "ReDMCSB DUNGEON.C F0140 lines 1114-1120";
    const char* f0163 = "ReDMCSB DUNGEON.C F0163 lines 1796-1837";
    const char* f0302 = "ReDMCSB CHAMPION.C F0302 lines 688-710";
    const char* f0309 = "ReDMCSB CHAMPION.C F0309/F0310 lines 1157-1205";
    const char* f0297 = "ReDMCSB CHAMPION.C F0297/F0300/F0301 lines 243-268,489-615";
    int c = 1;
    int ok = 1;

    ok &= expect_int("B base load", g_probe.baseLoads[c], 29, f0309);
    ok &= expect_int("B target C540", g_probe.targetPc34Slots[c],
                     DM1_PC34_SLOT_CHEST_4, f0302);
    ok &= expect_int("B open result", g_probe.openResults[c], 1, f0333);
    ok &= expect_int("B visible weight after open",
                     g_probe.visibleWeightsAfterOpen[c], 60, f0140);
    ok &= expect_int("B load after open", g_probe.loadsAfterOpen[c], 89,
                     f0309);
    ok &= expect_int("B movement ticks after open",
                     g_probe.movementTicksAfterOpen[c], 3, f0309);
    ok &= expect_int("B displaced type", g_probe.displacedTypes[c], 0x7504,
                     f0302);
    ok &= expect_int("B click result", g_probe.clickResults[c], 1, f0302);
    ok &= expect_int("B hand receives displaced item",
                     g_probe.handAfterTypes[c], 0x7504, f0302);
    ok &= expect_int("B C540 receives hand item",
                     g_probe.slotAfterTypes[c], 0x7302, f0302);
    ok &= expect_int("B visible weight after click",
                     g_probe.visibleWeightsAfterClick[c], 84, f0140);
    ok &= expect_int("B load after click", g_probe.loadsAfterClick[c], 113,
                     f0297);
    ok &= expect_int("B movement ticks after click",
                     g_probe.movementTicksAfterClick[c], 4, f0309);
    ok &= expect_int("B close count", g_probe.closeCounts[c], 4, f0334);
    ok &= expect_int("B close snapshot",
                     g_probe.closeContainerSnapshots[c], 134, f0140);
    ok &= expect_int("B closed target identity",
                     g_probe.replacementClosedAtTarget[c], 1, f0163);
    ok &= expect_int("B displaced absent from closed links",
                     g_probe.displacedAbsentFromClosed[c], 1, f0163);
    ok &= expect_int("B open thing cleared",
                     g_probe.openThingsAfterClose[c], 0, f0334);
    ok &= expect_int("B close load returns to base",
                     g_probe.loadsAfterClose[c], 29, f0297);
    ok &= expect_int("B movement ticks after close",
                     g_probe.movementTicksAfterClose[c], 2, f0309);
    return ok;
}

static int test_champion_c(void)
{
    const char* f0333 = "ReDMCSB CHEST.C F0333 lines 31-67";
    const char* f0334 = "ReDMCSB CHEST.C F0334 lines 117-132";
    const char* f0140 = "ReDMCSB DUNGEON.C F0140 lines 1114-1120";
    const char* f0163 = "ReDMCSB DUNGEON.C F0163 lines 1796-1837";
    const char* f0302 = "ReDMCSB CHAMPION.C F0302 lines 688-710";
    const char* f0309 = "ReDMCSB CHAMPION.C F0309/F0310 lines 1157-1205";
    const char* f0297 = "ReDMCSB CHAMPION.C F0297/F0300/F0301 lines 243-268,489-615";
    int c = 2;
    int ok = 1;

    ok &= expect_int("C base load", g_probe.baseLoads[c], 37, f0309);
    ok &= expect_int("C target C544", g_probe.targetPc34Slots[c],
                     DM1_PC34_SLOT_CHEST_8, f0302);
    ok &= expect_int("C open result", g_probe.openResults[c], 1, f0333);
    ok &= expect_int("C visible weight after open",
                     g_probe.visibleWeightsAfterOpen[c], 81, f0140);
    ok &= expect_int("C load after open", g_probe.loadsAfterOpen[c], 118,
                     f0309);
    ok &= expect_int("C movement ticks after open",
                     g_probe.movementTicksAfterOpen[c], 3, f0309);
    ok &= expect_int("C displaced type", g_probe.displacedTypes[c], 0x7608,
                     f0302);
    ok &= expect_int("C click result", g_probe.clickResults[c], 1, f0302);
    ok &= expect_int("C hand receives displaced item",
                     g_probe.handAfterTypes[c], 0x7608, f0302);
    ok &= expect_int("C C544 receives hand item",
                     g_probe.slotAfterTypes[c], 0x7303, f0302);
    ok &= expect_int("C visible weight after click",
                     g_probe.visibleWeightsAfterClick[c], 105, f0140);
    ok &= expect_int("C load after click", g_probe.loadsAfterClick[c], 142,
                     f0297);
    ok &= expect_int("C movement ticks after click",
                     g_probe.movementTicksAfterClick[c], 4, f0309);
    ok &= expect_int("C close count", g_probe.closeCounts[c], 8, f0334);
    ok &= expect_int("C close snapshot",
                     g_probe.closeContainerSnapshots[c], 155, f0140);
    ok &= expect_int("C closed target identity",
                     g_probe.replacementClosedAtTarget[c], 1, f0163);
    ok &= expect_int("C displaced absent from closed links",
                     g_probe.displacedAbsentFromClosed[c], 1, f0163);
    ok &= expect_int("C open thing cleared",
                     g_probe.openThingsAfterClose[c], 0, f0334);
    ok &= expect_int("C close load returns to base",
                     g_probe.loadsAfterClose[c], 37, f0297);
    ok &= expect_int("C movement ticks after close",
                     g_probe.movementTicksAfterClose[c], 2, f0309);
    return ok;
}

int main(void)
{
    const char* f0333 = "ReDMCSB CHEST.C F0333 lines 31-67";
    int ok = 1;

    printf("probe=dm1_v1_chest_multi_champion_close_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_chest_multi_champion_close_source_evidence_pc34());

    ok &= expect_int("probe setup",
                     dm1_v1_chest_multi_champion_close_run_pc34(&g_probe),
                     1, f0333);
    if (!ok) {
        printf("chestMultiChampionCloseResultOk=0 assertions=%d\n",
               g_assertions);
        return 1;
    }

    ok &= test_spec_and_masks();
    ok &= test_champion_a();
    ok &= test_champion_b();
    ok &= test_champion_c();

    printf("chestMultiChampionCloseResultOk=%d assertions=%d "
           "champions=%d closeCounts=%d,%d,%d closeLoads=%d,%d,%d "
           "snapshots=%d,%d,%d\n",
           ok ? 1 : 0,
           g_assertions,
           g_probe.championCount,
           g_probe.closeCounts[0],
           g_probe.closeCounts[1],
           g_probe.closeCounts[2],
           g_probe.loadsAfterClose[0],
           g_probe.loadsAfterClose[1],
           g_probe.loadsAfterClose[2],
           g_probe.closeContainerSnapshots[0],
           g_probe.closeContainerSnapshots[1],
           g_probe.closeContainerSnapshots[2]);
    return ok ? 0 : 1;
}
