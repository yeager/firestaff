#include "dm1_v1_chest_cross_champion_pickup_race_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/*
 * Slice chosen: DM1 V1 chest cross-champion pickup race. It is not a duplicate
 * of the existing capacity, occupied-slot, reopen, or multi-champion close
 * gates because it asserts the second empty-hand pickup against the same now
 * empty G0425 slot reaches CHAMPION.C F0302 lines 694-695's no-op path.
 *
 * Source anchors used by every assertion:
 * - ReDMCSB CHEST.C F0333 lines 31-67: open chest and visible G0425 materialize.
 * - ReDMCSB CHEST.C F0334 lines 113-132: close skips empty G0425 slots.
 * - ReDMCSB CHAMPION.C F0297/F0298 lines 243-298: leader-hand identity/weight.
 * - ReDMCSB CHAMPION.C F0300/F0301/F0302 lines 511-515,606-610,688-710:
 *   C30+ chest-slot reads, clears, no-op, and swaps.
 * - ReDMCSB CHAMDRAW.C F0291/F0296 lines 551-552,1184-1185: changed visible
 *   open-chest icons are drawn from the same G0425 slot window.
 */

static DM1_V1_ChestCrossChampionPickupRaceProbePc34 g_probe;
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

static int expect_str_contains(const char* label,
                               const char* got,
                               const char* want,
                               const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (!got || !want || !strstr(got, want)) {
        printf("FAIL %s missing=%s anchor=%s\n",
               label, want ? want : "(null)", redmcsbAnchor);
        return 0;
    }
    printf("PASS %s contains=%s anchor=%s\n", label, want, redmcsbAnchor);
    return 1;
}

static int test_spec_and_evidence(void)
{
    const char* f0333 = "ReDMCSB CHEST.C F0333 lines 31-67";
    const char* f0334 = "ReDMCSB CHEST.C F0334 lines 113-132";
    const char* f0302 =
        "ReDMCSB CHAMPION.C F0300/F0301/F0302 lines 511-515,606-610,688-710";
    const char* chamdraw =
        "ReDMCSB CHAMDRAW.C F0291/F0296 lines 551-552,1184-1185";
    const DM1_V1_ChestCrossChampionPickupRaceSpecPc34* spec =
        dm1_v1_chest_cross_champion_pickup_race_spec_pc34();
    const char* evidence =
        dm1_v1_chest_cross_champion_pickup_race_source_evidence_pc34();
    int ok = 1;

    ok &= expect_str_contains("contract marker", spec->contractMarker,
                              "Source-locked deterministic contract gate only",
                              f0333);
    ok &= expect_int("contract-only flag", g_probe.contractOnly, 1, f0333);
    ok &= expect_int("champion count", spec->championCount,
                     DM1_PC34_CHEST_PICKUP_RACE_CHAMPION_COUNT, f0302);
    ok &= expect_int("chest slot count", spec->chestSlotCount,
                     DM1_PC34_CHEST_PICKUP_RACE_SLOT_COUNT, f0333);
    ok &= expect_int("race slot index", spec->raceSlotIndex,
                     DM1_PC34_CHEST_PICKUP_RACE_SLOT_INDEX, f0302);
    ok &= expect_int("race pc34 slot", spec->racePc34Slot,
                     DM1_PC34_CHEST_PICKUP_RACE_PC34_SLOT, f0302);
    ok &= expect_int("chest thing", spec->chestThing,
                     DM1_PC34_CHEST_PICKUP_RACE_CHEST_THING, f0333);
    ok &= expect_int("picked item", spec->pickedItem,
                     DM1_PC34_CHEST_PICKUP_RACE_PICKED_ITEM, f0302);
    ok &= expect_str_contains("evidence F0333", evidence,
                              "CHEST.C F0333:31-67", f0333);
    ok &= expect_str_contains("evidence F0334", evidence,
                              "CHEST.C F0334:113-132", f0334);
    ok &= expect_str_contains("evidence F0302", evidence,
                              "CHAMPION.C F0302:688-710", f0302);
    ok &= expect_str_contains("evidence CHAMDRAW", evidence,
                              "CHAMDRAW.C F0291:551-552", chamdraw);
    return ok;
}

static int test_initial_open(void)
{
    const char* f0333 = "ReDMCSB CHEST.C F0333 lines 31-67";
    const char* chamdraw =
        "ReDMCSB CHAMDRAW.C F0291/F0296 lines 551-552,1184-1185";
    int ok = 1;

    ok &= expect_int("open result", g_probe.openResult, 1, f0333);
    ok &= expect_int("open thing", g_probe.openThing,
                     DM1_PC34_CHEST_PICKUP_RACE_CHEST_THING, f0333);
    ok &= expect_int("initial visible count", g_probe.initialVisibleCount, 4,
                     f0333);
    ok &= expect_int("initial C539 type", g_probe.initialRaceSlotType,
                     DM1_PC34_CHEST_PICKUP_RACE_PICKED_ITEM, f0333);
    ok &= expect_int("initial C539 weight", g_probe.initialRaceSlotWeight, 7,
                     "ReDMCSB CHAMPION.C F0297/F0298 lines 243-298");
    ok &= expect_int("initial slot 0", g_probe.initialTypes[0],
                     DM1_PC34_CHEST_PICKUP_RACE_FIRST_ITEM, chamdraw);
    ok &= expect_int("initial slot 1", g_probe.initialTypes[1],
                     DM1_PC34_CHEST_PICKUP_RACE_FIRST_ITEM + 1, chamdraw);
    ok &= expect_int("initial slot 2 race item", g_probe.initialTypes[2],
                     DM1_PC34_CHEST_PICKUP_RACE_PICKED_ITEM, chamdraw);
    ok &= expect_int("initial slot 3", g_probe.initialTypes[3],
                     DM1_PC34_CHEST_PICKUP_RACE_FIRST_ITEM + 3, chamdraw);
    return ok;
}

static int test_race_pickup(void)
{
    const char* f0297 =
        "ReDMCSB CHAMPION.C F0297/F0298 lines 243-298";
    const char* f0302 =
        "ReDMCSB CHAMPION.C F0300/F0301/F0302 lines 511-515,606-610,688-710";
    const char* chamdraw =
        "ReDMCSB CHAMDRAW.C F0291/F0296 lines 551-552,1184-1185";
    int ok = 1;

    ok &= expect_int("first click result", g_probe.firstClickResult, 1,
                     f0302);
    ok &= expect_int("first hand receives picked item",
                     g_probe.firstHandAfterType,
                     DM1_PC34_CHEST_PICKUP_RACE_PICKED_ITEM, f0297);
    ok &= expect_int("first hand picked weight",
                     g_probe.firstHandAfterWeight, 7, f0297);
    ok &= expect_int("first race slot emptied",
                     g_probe.firstRaceSlotAfterType, 0, f0302);
    ok &= expect_int("first champion won pickup",
                     g_probe.firstChampionWonPickup, 1, f0302);

    ok &= expect_int("second click no-op result",
                     g_probe.secondClickResult, 0, f0302);
    ok &= expect_int("second hand remains empty",
                     g_probe.secondHandAfterType, 0, f0297);
    ok &= expect_int("second race slot remains empty",
                     g_probe.secondRaceSlotAfterType, 0, f0302);
    ok &= expect_int("second no-op path",
                     g_probe.secondNoopPath, 1, f0302);
    ok &= expect_int("race slot stable after second click",
                     g_probe.raceSlotStableAfterSecondClick, 1, chamdraw);

    ok &= expect_int("only one champion wins", g_probe.winnerCount, 1,
                     f0297);
    ok &= expect_int("single picked item copy after race",
                     g_probe.pickedItemCopyCountAfterRace, 1, f0302);
    ok &= expect_int("visible count after race",
                     g_probe.visibleCountAfterRace, 3,
                     "ReDMCSB CHEST.C F0334 lines 113-132");
    ok &= expect_int("after race slot 0", g_probe.afterRaceTypes[0],
                     DM1_PC34_CHEST_PICKUP_RACE_FIRST_ITEM, chamdraw);
    ok &= expect_int("after race slot 1", g_probe.afterRaceTypes[1],
                     DM1_PC34_CHEST_PICKUP_RACE_FIRST_ITEM + 1, chamdraw);
    ok &= expect_int("after race slot 2 empty", g_probe.afterRaceTypes[2], 0,
                     chamdraw);
    ok &= expect_int("after race slot 3", g_probe.afterRaceTypes[3],
                     DM1_PC34_CHEST_PICKUP_RACE_FIRST_ITEM + 3, chamdraw);
    return ok;
}

static int test_close_and_reopen(void)
{
    const char* f0333 = "ReDMCSB CHEST.C F0333 lines 31-67";
    const char* f0334 = "ReDMCSB CHEST.C F0334 lines 113-132";
    const char* f0302 =
        "ReDMCSB CHAMPION.C F0300/F0301/F0302 lines 511-515,606-610,688-710";
    const char* chamdraw =
        "ReDMCSB CHAMDRAW.C F0291/F0296 lines 551-552,1184-1185";
    int ok = 1;

    ok &= expect_int("close count", g_probe.closeCount, 3, f0334);
    ok &= expect_int("closed slot 0", g_probe.closedTypes[0],
                     DM1_PC34_CHEST_PICKUP_RACE_FIRST_ITEM, f0334);
    ok &= expect_int("closed slot 1", g_probe.closedTypes[1],
                     DM1_PC34_CHEST_PICKUP_RACE_FIRST_ITEM + 1, f0334);
    ok &= expect_int("closed slot 2 compacted successor",
                     g_probe.closedTypes[2],
                     DM1_PC34_CHEST_PICKUP_RACE_FIRST_ITEM + 3, f0334);
    ok &= expect_int("picked item absent from closed links",
                     g_probe.pickedItemAbsentFromClosedLinks, 1, f0302);
    ok &= expect_int("close compacted empty race slot",
                     g_probe.closeCompactedEmptyRaceSlot, 1, f0334);

    ok &= expect_int("reopen result", g_probe.reopenResult, 1, f0333);
    ok &= expect_int("reopened visible count",
                     g_probe.reopenedVisibleCount, 3, f0333);
    ok &= expect_int("reopened slot 0", g_probe.reopenedTypes[0],
                     DM1_PC34_CHEST_PICKUP_RACE_FIRST_ITEM, chamdraw);
    ok &= expect_int("reopened slot 1", g_probe.reopenedTypes[1],
                     DM1_PC34_CHEST_PICKUP_RACE_FIRST_ITEM + 1, chamdraw);
    ok &= expect_int("reopened slot 2 compacted successor",
                     g_probe.reopenedTypes[2],
                     DM1_PC34_CHEST_PICKUP_RACE_FIRST_ITEM + 3, chamdraw);
    ok &= expect_int("reopened slot 3 empty",
                     g_probe.reopenedTypes[3], 0, chamdraw);
    ok &= expect_int("picked item absent after reopen",
                     g_probe.pickedItemAbsentAfterReopen, 1, f0302);
    ok &= expect_int("reopened order compacted",
                     g_probe.reopenedOrderCompacted, 1, f0333);
    return ok;
}

int main(void)
{
    const char* f0333 = "ReDMCSB CHEST.C F0333 lines 31-67";
    int ok = 1;

    printf("probe=dm1_v1_chest_cross_champion_pickup_race_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_chest_cross_champion_pickup_race_source_evidence_pc34());

    ok &= expect_int(
        "probe setup",
        dm1_v1_chest_cross_champion_pickup_race_run_pc34(&g_probe), 1,
        f0333);
    if (!ok) {
        printf("assertionCount=%d\n", g_assertions);
        printf("chestCrossChampionPickupRaceResultOk=0\n");
        return 1;
    }

    ok &= test_spec_and_evidence();
    ok &= test_initial_open();
    ok &= test_race_pickup();
    ok &= test_close_and_reopen();
    ok &= expect_int("minimum assertion count",
                     g_assertions >= 50 ? 1 : 0, 1, f0333);

    printf("assertionCount=%d\n", g_assertions);
    printf("chestCrossChampionPickupRaceResultOk=%d winners=%d "
           "closedCount=%d reopenedCount=%d\n",
           ok ? 1 : 0,
           g_probe.winnerCount,
           g_probe.closeCount,
           g_probe.reopenedVisibleCount);
    return ok ? 0 : 1;
}
