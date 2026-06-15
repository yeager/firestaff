#include "dm1_v1_chest_drop_to_floor_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static DM1_V1_ChestDropToFloorProbePc34 g_probe;
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

static int test_spec_and_evidence(void)
{
    const DM1_V1_ChestDropToFloorSpecPc34* spec =
        dm1_v1_chest_drop_to_floor_spec_pc34();
    const char* evidence =
        dm1_v1_chest_drop_to_floor_source_evidence_pc34();
    const char* command =
        "ReDMCSB COMMAND.C F0360 lines 489-506,2174-2177";
    const char* panel =
        "ReDMCSB PANEL.C F0342/F0347 lines 1119-1133,1651-1669";
    const char* chest =
        "ReDMCSB CHEST.C F0333/F0334 lines 53-75,113-132";
    const char* dungeon =
        "ReDMCSB DUNGEON.C F0163 lines 1800-1837";
    int ok = 1;

    ok &= expect_str("contract marker", spec->contractMarker,
                     "Source-locked contract gate only; not full real-asset chest/floor runtime parity.",
                     chest);
    ok &= expect_int("action hand slot", spec->commandActionHand,
                     DM1_PC34_SLOT_ACTION_HAND, command);
    ok &= expect_int("first chest slot", spec->firstChestSlot,
                     DM1_PC34_SLOT_CHEST_1, command);
    ok &= expect_int("last chest slot", spec->lastChestSlot,
                     DM1_PC34_SLOT_CHEST_8, command);
    ok &= expect_int("chest slot count", spec->chestSlotCount,
                     DM1_PC34_CHEST_DROP_TO_FLOOR_SLOT_COUNT, chest);
    ok &= expect_int("initial chest count", spec->initialChestCount,
                     DM1_PC34_CHEST_DROP_TO_FLOOR_INITIAL_CHEST_COUNT, chest);
    ok &= expect_int("hand stack count", spec->handStackCount,
                     DM1_PC34_CHEST_DROP_TO_FLOOR_HAND_STACK_COUNT, command);
    ok &= expect_int("front map x", spec->frontMapX,
                     DM1_PC34_CHEST_DROP_TO_FLOOR_FRONT_MAP_X, dungeon);
    ok &= expect_int("front map y", spec->frontMapY,
                     DM1_PC34_CHEST_DROP_TO_FLOOR_FRONT_MAP_Y, dungeon);
    ok &= expect_contains("evidence command", evidence, "COMMAND.C F0360",
                          command);
    ok &= expect_contains("evidence panel", evidence, "PANEL.C F0342/F0347",
                          panel);
    ok &= expect_contains("evidence chest open", evidence, "CHEST.C F0333",
                          chest);
    ok &= expect_contains("evidence chest close", evidence, "CHEST.C F0334",
                          chest);
    ok &= expect_contains("evidence dungeon square link", evidence,
                          "DUNGEON.C F0163", dungeon);
    return ok;
}

static int test_open_and_deposit_counts(void)
{
    const char* command =
        "ReDMCSB COMMAND.C F0360 lines 2174-2177";
    const char* chestOpen =
        "ReDMCSB CHEST.C F0333 lines 53-75";
    const char* champion =
        "ReDMCSB CHAMPION.C F0302 lines 688-710";
    int ok = 1;
    int i;

    ok &= expect_int("contract-only marker",
                     g_probe.sourceLockedContractOnly, 1, chestOpen);
    ok &= expect_int("open succeeds", g_probe.openResult, 1, chestOpen);
    ok &= expect_int("open thing", g_probe.openThing,
                     DM1_PC34_CHEST_DROP_TO_FLOOR_CHEST_THING, chestOpen);
    ok &= expect_int("initial chest count", g_probe.initialChestCount,
                     DM1_PC34_CHEST_DROP_TO_FLOOR_INITIAL_CHEST_COUNT,
                     chestOpen);
    ok &= expect_int("hand stack before", g_probe.handStackCountBefore,
                     DM1_PC34_CHEST_DROP_TO_FLOOR_HAND_STACK_COUNT, command);
    ok &= expect_int("free chest slots before",
                     g_probe.freeChestSlotsBefore, 2, chestOpen);
    ok &= expect_int("deposit attempts", g_probe.depositAttempts, 2,
                     champion);
    ok &= expect_int("deposited count", g_probe.depositedCount, 2,
                     champion);
    ok &= expect_int("hand stack after deposit",
                     g_probe.handStackCountAfterDeposit, 3, champion);
    ok &= expect_int("hand count reduced by deposits",
                     g_probe.handCountReducedByDeposits, 1, champion);
    ok &= expect_int("leader hand empty after run",
                     g_probe.leaderHandEmptyAfterRun, 1, champion);

    for (i = 0; i < DM1_PC34_CHEST_DROP_TO_FLOOR_INITIAL_CHEST_COUNT; ++i) {
        ok &= expect_int("opened chest order",
                         g_probe.openedTypes[i],
                         DM1_PC34_CHEST_DROP_TO_FLOOR_FIRST_CHEST_ITEM + i,
                         chestOpen);
    }
    return ok;
}

static int test_close_and_floor_overflow(void)
{
    const char* chestClose =
        "ReDMCSB CHEST.C F0334 lines 113-132";
    const char* champion =
        "ReDMCSB CHAMPION.C F0302 lines 688-710";
    const char* dungeon =
        "ReDMCSB DUNGEON.C F0163 lines 1800-1837";
    int ok = 1;
    int i;

    ok &= expect_int("close count", g_probe.closeCount,
                     DM1_PC34_CHEST_DROP_TO_FLOOR_SLOT_COUNT, chestClose);
    ok &= expect_int("closed chest count", g_probe.closedChestCount,
                     DM1_PC34_CHEST_DROP_TO_FLOOR_SLOT_COUNT, chestClose);
    ok &= expect_int("open thing after close", g_probe.openThingAfterClose,
                     0, chestClose);
    ok &= expect_int("overflow count", g_probe.overflowCount, 3, dungeon);
    ok &= expect_int("floor drop count", g_probe.floorDropCount, 3,
                     dungeon);
    ok &= expect_int("floor front x", g_probe.frontMapX,
                     DM1_PC34_CHEST_DROP_TO_FLOOR_FRONT_MAP_X, dungeon);
    ok &= expect_int("floor front y", g_probe.frontMapY,
                     DM1_PC34_CHEST_DROP_TO_FLOOR_FRONT_MAP_Y, dungeon);
    ok &= expect_int("chest prefix order preserved",
                     g_probe.chestOrderPreserved, 1, chestClose);
    ok &= expect_int("deposit order preserved",
                     g_probe.depositOrderPreserved, 1, champion);
    ok &= expect_int("floor overflow order preserved",
                     g_probe.floorOverflowOrderPreserved, 1, dungeon);
    ok &= expect_int("overflow absent from chest",
                     g_probe.overflowAbsentFromChest, 1, dungeon);

    for (i = 0; i < DM1_PC34_CHEST_DROP_TO_FLOOR_INITIAL_CHEST_COUNT; ++i) {
        ok &= expect_int("closed original chest order",
                         g_probe.closedTypes[i],
                         DM1_PC34_CHEST_DROP_TO_FLOOR_FIRST_CHEST_ITEM + i,
                         chestClose);
    }
    for (i = 0; i < g_probe.depositedCount; ++i) {
        ok &= expect_int("closed deposited hand order",
                         g_probe.closedTypes[
                             DM1_PC34_CHEST_DROP_TO_FLOOR_INITIAL_CHEST_COUNT + i],
                         DM1_PC34_CHEST_DROP_TO_FLOOR_FIRST_HAND_ITEM + i,
                         champion);
    }
    for (i = 0; i < g_probe.floorDropCount; ++i) {
        ok &= expect_int("floor overflow item order",
                         g_probe.floorTypes[i],
                         DM1_PC34_CHEST_DROP_TO_FLOOR_FIRST_HAND_ITEM +
                         g_probe.depositedCount + i,
                         dungeon);
    }
    return ok;
}

int main(void)
{
    const char* chest =
        "ReDMCSB CHEST.C F0333/F0334 lines 53-75,113-132";
    int ok = 1;

    printf("probe=dm1_v1_chest_drop_to_floor_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_chest_drop_to_floor_source_evidence_pc34());

    ok &= expect_int("probe setup",
                     dm1_v1_chest_drop_to_floor_pc34_compat_run(&g_probe),
                     1, chest);
    if (!ok) {
        printf("assertionCount=%d\n", g_assertions);
        printf("chestDropToFloorInvariantOk=0\n");
        return 1;
    }

    ok &= test_spec_and_evidence();
    ok &= test_open_and_deposit_counts();
    ok &= test_close_and_floor_overflow();
    ok &= expect_int("minimum assertion count",
                     g_assertions >= 50 ? 1 : 0, 1, chest);

    printf("assertionCount=%d\n", g_assertions);
    printf("chestDropToFloorInvariantOk=%d\n", ok ? 1 : 0);
    printf("PASS dm1_v1_chest_drop_to_floor_pc34_compat assertions=%d\n",
           g_assertions);
    return ok ? 0 : 1;
}
