#include "dm1_v1_chest_scroll_wheel_drop_to_floor_non_leader_c030_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static int expect_int(const char* label,
                      int got,
                      int want,
                      const char* anchor)
{
    ++g_assertions;
    if (!anchor || anchor[0] == '\0') {
        ++g_failures;
        printf("FAIL %s missing-anchor\n", label);
        return 0;
    }
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n",
               label, got, want, anchor);
        return 0;
    }
    printf("PASS %s=%d anchor=%s\n", label, got, anchor);
    return 1;
}

static int expect_contains(const char* label,
                           const char* haystack,
                           const char* needle,
                           const char* anchor)
{
    ++g_assertions;
    if (!anchor || anchor[0] == '\0' || !haystack || !needle ||
        !strstr(haystack, needle)) {
        ++g_failures;
        printf("FAIL %s missing=%s anchor=%s\n",
               label, needle ? needle : "(null)",
               anchor ? anchor : "(null)");
        return 0;
    }
    printf("PASS %s contains=%s anchor=%s\n", label, needle, anchor);
    return 1;
}

static int expect_array(const char* label,
                        const int* got,
                        const int* want,
                        int count,
                        const char* anchor)
{
    int i;
    int ok = 1;

    for (i = 0; i < count; ++i) {
        char itemLabel[96];

        snprintf(itemLabel, sizeof(itemLabel), "%s[%d]", label, i);
        ok &= expect_int(itemLabel, got[i], want[i], anchor);
    }
    return ok;
}

static int test_source_evidence(
    const DM1_V1_ChestScrollWheelDropToFloorNonLeaderC030SpecPc34* spec)
{
    const char* evidence =
        dm1_v1_chest_scroll_wheel_drop_to_floor_non_leader_c030_source_evidence_pc34();
    int ok = 1;

    ok &= expect_int("source pointer stable",
                     evidence == spec->sourceEvidence, 1,
                     spec->defsAnchor);
    ok &= expect_contains("source F0333", evidence,
                          "CHEST.C F0333:30-67",
                          spec->f0333OpenAnchor);
    ok &= expect_contains("source F0334", evidence,
                          "CHEST.C F0334:113-132",
                          spec->f0334CloseAnchor);
    ok &= expect_contains("source F0297", evidence,
                          "CHAMPION.C F0297:243-298",
                          spec->f0297PutAnchor);
    ok &= expect_contains("source F0298", evidence,
                          "CHAMPION.C F0298:270-298",
                          spec->f0298RemoveAnchor);
    ok &= expect_contains("source F0300", evidence,
                          "CHAMPION.C F0300:511-515",
                          spec->f0300ClearAnchor);
    ok &= expect_contains("source F0301", evidence,
                          "CHAMPION.C F0301:606-614",
                          spec->f0301WriteAnchor);
    ok &= expect_contains("source F0302", evidence,
                          "CHAMPION.C F0302:662-714",
                          spec->f0302DispatchAnchor);
    ok &= expect_contains("source F0284", evidence,
                          "CHAMPION.C F0284:93-131",
                          spec->f0284OwnershipAnchor);
    ok &= expect_contains("source F0352", evidence, "PANEL.C F0352",
                          spec->panelF0352Anchor);
    ok &= expect_contains("source F0378", evidence,
                          "COMMAND.C F0378:1973-1983",
                          spec->commandF0378Anchor);
    ok &= expect_contains("source defs 2088", evidence, "DEFS.H:2088",
                          spec->defsAnchor);
    ok &= expect_contains("source defs 810", evidence, "DEFS.H:810-816",
                          spec->defsAnchor);
    ok &= expect_contains("source defs 3906", evidence, "DEFS.H:3906-3913",
                          spec->defsAnchor);
    ok &= expect_contains("source C545", evidence, "C545",
                          spec->defsAnchor);
    return ok;
}

static int test_constants(
    const DM1_V1_ChestScrollWheelDropToFloorNonLeaderC030SpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("slot count", DM1_PC34_C545_DROP_SLOT_COUNT, 8,
                     spec->defsAnchor);
    ok &= expect_int("champion count", DM1_PC34_C545_DROP_CHAMPION_COUNT, 4,
                     spec->defsAnchor);
    ok &= expect_int("C30", DM1_PC34_C545_DROP_C30, 30, spec->defsAnchor);
    ok &= expect_int("C31", DM1_PC34_C545_DROP_C31, 31, spec->defsAnchor);
    ok &= expect_int("C36", DM1_PC34_C545_DROP_C36, 36, spec->defsAnchor);
    ok &= expect_int("C37", DM1_PC34_C545_DROP_C37, 37, spec->defsAnchor);
    ok &= expect_int("C537", DM1_PC34_C545_DROP_C537, 537,
                     spec->defsAnchor);
    ok &= expect_int("C538", DM1_PC34_C545_DROP_C538, 538,
                     spec->defsAnchor);
    ok &= expect_int("C539", DM1_PC34_C545_DROP_C539, 539,
                     spec->defsAnchor);
    ok &= expect_int("C540", DM1_PC34_C545_DROP_C540, 540,
                     spec->defsAnchor);
    ok &= expect_int("C541", DM1_PC34_C545_DROP_C541, 541,
                     spec->defsAnchor);
    ok &= expect_int("C542", DM1_PC34_C545_DROP_C542, 542,
                     spec->defsAnchor);
    ok &= expect_int("C543", DM1_PC34_C545_DROP_C543, 543,
                     spec->defsAnchor);
    ok &= expect_int("C544", DM1_PC34_C545_DROP_C544, 544,
                     spec->defsAnchor);
    ok &= expect_int("C545", DM1_PC34_C545_DROP_C545, 545,
                     spec->defsAnchor);
    ok &= expect_int("C070", DM1_PC34_C545_DROP_C070, 70,
                     spec->commandF0378Anchor);
    ok &= expect_int("panel M569", DM1_PC34_C545_DROP_PANEL_M569, 569,
                     spec->commandF0378Anchor);
    ok &= expect_int("leader index", DM1_PC34_C545_DROP_LEADER_INDEX, 0,
                     spec->f0284OwnershipAnchor);
    ok &= expect_int("owner index", DM1_PC34_C545_DROP_OWNER_INDEX, 2,
                     spec->f0284OwnershipAnchor);
    ok &= expect_int("owner ordinal", DM1_PC34_C545_DROP_OWNER_ORDINAL, 3,
                     spec->f0284OwnershipAnchor);
    ok &= expect_int("floor x", DM1_PC34_C545_DROP_FLOOR_X, 12,
                     spec->defsAnchor);
    ok &= expect_int("floor y", DM1_PC34_C545_DROP_FLOOR_Y, 18,
                     spec->defsAnchor);
    ok &= expect_int("load mask", DM1_PC34_C545_DROP_LOAD_MASK, 0x0200,
                     spec->defsAnchor);
    ok &= expect_int("panel mask", DM1_PC34_C545_DROP_PANEL_MASK, 0x0800,
                     spec->defsAnchor);
    ok &= expect_int("viewport mask", DM1_PC34_C545_DROP_VIEWPORT_MASK,
                     0x4000, spec->defsAnchor);
    return ok;
}

static int test_initial_state(
    const DM1_V1_ChestScrollWheelDropToFloorNonLeaderC030ProbePc34* probe,
    const DM1_V1_ChestScrollWheelDropToFloorNonLeaderC030SpecPc34* spec)
{
    const int ownerSlots[DM1_PC34_C545_DROP_SLOT_COUNT] = {
        DM1_PC34_C545_DROP_ITEM,
        DM1_PC34_C545_DROP_NONE,
        DM1_PC34_C545_DROP_NONE,
        DM1_PC34_C545_DROP_NONE,
        DM1_PC34_C545_DROP_NONE,
        DM1_PC34_C545_DROP_NONE,
        DM1_PC34_C545_DROP_NONE,
        DM1_PC34_C545_DROP_NONE
    };
    const int ownerIcons[DM1_PC34_C545_DROP_SLOT_COUNT] = {
        DM1_PC34_C545_DROP_ITEM & 0x00FF,
        DM1_PC34_C545_DROP_NONE,
        DM1_PC34_C545_DROP_NONE,
        DM1_PC34_C545_DROP_NONE,
        DM1_PC34_C545_DROP_NONE,
        DM1_PC34_C545_DROP_NONE,
        DM1_PC34_C545_DROP_NONE,
        DM1_PC34_C545_DROP_NONE
    };
    const int leaderSlots[DM1_PC34_C545_DROP_SLOT_COUNT] = {
        DM1_PC34_C545_DROP_NONE,
        DM1_PC34_C545_DROP_NONE,
        DM1_PC34_C545_DROP_NONE,
        DM1_PC34_C545_DROP_NONE,
        DM1_PC34_C545_DROP_NONE,
        DM1_PC34_C545_DROP_NONE,
        DM1_PC34_C545_DROP_NONE,
        DM1_PC34_C545_DROP_NONE
    };
    int ok = 1;

    ok &= expect_int("party count", probe->partyChampionCount, 4,
                     spec->defsAnchor);
    ok &= expect_int("leader index", probe->leaderIndex,
                     DM1_PC34_C545_DROP_LEADER_INDEX,
                     spec->f0284OwnershipAnchor);
    ok &= expect_int("inventory ordinal", probe->inventoryChampionOrdinal,
                     DM1_PC34_C545_DROP_OWNER_ORDINAL,
                     spec->f0284OwnershipAnchor);
    ok &= expect_int("target champion", probe->targetChampionIndex,
                     DM1_PC34_C545_DROP_OWNER_INDEX,
                     spec->f0284OwnershipAnchor);
    ok &= expect_int("target ordinal", probe->targetChampionOrdinal,
                     DM1_PC34_C545_DROP_OWNER_ORDINAL,
                     spec->f0284OwnershipAnchor);
    ok &= expect_int("open chest before", probe->openChestThingBefore,
                     DM1_PC34_C545_DROP_OPEN_CHEST,
                     spec->f0333OpenAnchor);
    ok &= expect_int("open chest owner before", probe->openChestOwnerBefore,
                     DM1_PC34_C545_DROP_OWNER_INDEX,
                     spec->f0284OwnershipAnchor);
    ok &= expect_int("leader hand before", probe->leaderHandBefore,
                     DM1_PC34_C545_DROP_NONE, spec->f0297PutAnchor);
    ok &= expect_int("leader C30 before", probe->leaderC30Before,
                     DM1_PC34_C545_DROP_NONE, spec->f0301WriteAnchor);
    ok &= expect_int("non-leader C30 before", probe->nonLeaderC30Before,
                     DM1_PC34_C545_DROP_ITEM, spec->f0302DispatchAnchor);
    ok &= expect_int("non-leader C30 icon before",
                     probe->nonLeaderC30IconBefore,
                     DM1_PC34_C545_DROP_ITEM & 0x00FF,
                     spec->defsAnchor);
    ok &= expect_int("non-leader load before", probe->nonLeaderLoadBefore,
                     42, spec->f0284OwnershipAnchor);
    ok &= expect_array("initial owner G0425", probe->initialG0425,
                       ownerSlots, DM1_PC34_C545_DROP_SLOT_COUNT,
                       spec->f0333OpenAnchor);
    ok &= expect_array("initial owner icons", probe->initialIcons,
                       ownerIcons, DM1_PC34_C545_DROP_SLOT_COUNT,
                       spec->defsAnchor);
    ok &= expect_array("initial leader G0425", probe->initialLeaderG0425,
                       leaderSlots, DM1_PC34_C545_DROP_SLOT_COUNT,
                       spec->f0284OwnershipAnchor);
    return ok;
}

static int test_c545_dispatch(
    const DM1_V1_ChestScrollWheelDropToFloorNonLeaderC030ProbePc34* probe,
    const DM1_V1_ChestScrollWheelDropToFloorNonLeaderC030SpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("C545 triggered", probe->c545EventTriggered, 1,
                     spec->commandF0378Anchor);
    ok &= expect_int("C545 event zone", probe->c545EventZone,
                     DM1_PC34_C545_DROP_C545, spec->commandF0378Anchor);
    ok &= expect_int("C545 command", probe->c545Command,
                     DM1_PC34_C545_DROP_C070, spec->commandF0378Anchor);
    ok &= expect_int("F0378 dispatch count",
                     probe->commandF0378DispatchCount, 1,
                     spec->commandF0378Anchor);
    ok &= expect_int("F0302 dispatch count", probe->f0302DispatchCount, 1,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("F0302 leader snapshot", probe->f0302LeaderSnapshot,
                     DM1_PC34_C545_DROP_NONE, spec->f0302DispatchAnchor);
    ok &= expect_int("F0302 C30 snapshot", probe->f0302SlotSnapshot,
                     DM1_PC34_C545_DROP_ITEM, spec->f0302DispatchAnchor);
    ok &= expect_int("F0302 champion index", probe->f0302ChampionIndex,
                     DM1_PC34_C545_DROP_OWNER_INDEX,
                     spec->f0284OwnershipAnchor);
    ok &= expect_int("F0302 slot index", probe->f0302SlotIndex,
                     DM1_PC34_C545_DROP_C30, spec->f0302DispatchAnchor);
    ok &= expect_int("F0302 C30 offset", probe->f0302C30Offset, 0,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("F0302 empty-empty rejected",
                     probe->f0302EmptyEmptyRejected, 0,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("leader-hand bypass", probe->f0302LeaderHandBypass, 1,
                     spec->f0297PutAnchor);
    ok &= expect_int("F0297 skipped", probe->f0297PutCount, 0,
                     spec->f0297PutAnchor);
    ok &= expect_int("F0298 skipped", probe->f0298RemoveCount, 0,
                     spec->f0298RemoveAnchor);
    ok &= expect_int("F0300 clear count", probe->f0300ClearCount, 1,
                     spec->f0300ClearAnchor);
    ok &= expect_int("F0301 write skipped", probe->f0301WriteCount, 0,
                     spec->f0301WriteAnchor);
    return ok;
}

static int test_drop_mutation(
    const DM1_V1_ChestScrollWheelDropToFloorNonLeaderC030ProbePc34* probe,
    const DM1_V1_ChestScrollWheelDropToFloorNonLeaderC030SpecPc34* spec)
{
    const int emptySlots[DM1_PC34_C545_DROP_SLOT_COUNT] = {
        DM1_PC34_C545_DROP_NONE,
        DM1_PC34_C545_DROP_NONE,
        DM1_PC34_C545_DROP_NONE,
        DM1_PC34_C545_DROP_NONE,
        DM1_PC34_C545_DROP_NONE,
        DM1_PC34_C545_DROP_NONE,
        DM1_PC34_C545_DROP_NONE,
        DM1_PC34_C545_DROP_NONE
    };
    int ok = 1;

    ok &= expect_int("floor write count", probe->floorWriteCount, 1,
                     spec->f0300ClearAnchor);
    ok &= expect_int("floor thing", probe->floorThing,
                     DM1_PC34_C545_DROP_ITEM, spec->f0300ClearAnchor);
    ok &= expect_int("floor icon", probe->floorIcon,
                     DM1_PC34_C545_DROP_ITEM & 0x00FF,
                     spec->defsAnchor);
    ok &= expect_int("floor x", probe->floorMapX,
                     DM1_PC34_C545_DROP_FLOOR_X, spec->defsAnchor);
    ok &= expect_int("floor y", probe->floorMapY,
                     DM1_PC34_C545_DROP_FLOOR_Y, spec->defsAnchor);
    ok &= expect_int("floor owner champion", probe->floorOwnerChampionIndex,
                     DM1_PC34_C545_DROP_OWNER_INDEX,
                     spec->f0284OwnershipAnchor);
    ok &= expect_int("floor owner ordinal", probe->floorOwnerOrdinal,
                     DM1_PC34_C545_DROP_OWNER_ORDINAL,
                     spec->f0284OwnershipAnchor);
    ok &= expect_int("floor owner G0426", probe->floorOwnerG0426,
                     DM1_PC34_C545_DROP_OPEN_CHEST,
                     spec->f0333OpenAnchor);
    ok &= expect_int("floor same champion", probe->floorWroteSameChampion, 1,
                     spec->f0284OwnershipAnchor);
    ok &= expect_int("floor not leader", probe->floorWroteNotLeader, 1,
                     spec->f0284OwnershipAnchor);
    ok &= expect_int("leader hand after", probe->leaderHandAfter,
                     DM1_PC34_C545_DROP_NONE, spec->f0297PutAnchor);
    ok &= expect_int("leader C30 after", probe->leaderC30After,
                     DM1_PC34_C545_DROP_NONE, spec->f0301WriteAnchor);
    ok &= expect_int("leader C30 unchanged", probe->leaderC30Unchanged, 1,
                     spec->f0301WriteAnchor);
    ok &= expect_int("non-leader C30 after", probe->nonLeaderC30After,
                     DM1_PC34_C545_DROP_NONE, spec->f0300ClearAnchor);
    ok &= expect_int("non-leader load after", probe->nonLeaderLoadAfter, 40,
                     spec->f0300ClearAnchor);
    ok &= expect_int("load mask",
                     probe->nonLeaderAttributesAfter &
                         DM1_PC34_C545_DROP_LOAD_MASK,
                     DM1_PC34_C545_DROP_LOAD_MASK, spec->defsAnchor);
    ok &= expect_int("panel mask",
                     probe->nonLeaderAttributesAfter &
                         DM1_PC34_C545_DROP_PANEL_MASK,
                     DM1_PC34_C545_DROP_PANEL_MASK, spec->defsAnchor);
    ok &= expect_int("viewport mask",
                     probe->nonLeaderAttributesAfter &
                         DM1_PC34_C545_DROP_VIEWPORT_MASK,
                     DM1_PC34_C545_DROP_VIEWPORT_MASK, spec->defsAnchor);
    ok &= expect_int("open chest after", probe->openChestThingAfter,
                     DM1_PC34_C545_DROP_OPEN_CHEST,
                     spec->f0333OpenAnchor);
    ok &= expect_int("open chest owner after", probe->openChestOwnerAfter,
                     DM1_PC34_C545_DROP_OWNER_INDEX,
                     spec->f0284OwnershipAnchor);
    ok &= expect_int("G0426 owned by non-leader",
                     probe->g0426StillOwnedByNonLeader, 1,
                     spec->f0334CloseAnchor);
    ok &= expect_array("G0425 after drop", probe->g0425AfterDrop,
                       emptySlots, DM1_PC34_C545_DROP_SLOT_COUNT,
                       spec->f0300ClearAnchor);
    ok &= expect_array("icons after drop", probe->iconsAfterDrop,
                       emptySlots, DM1_PC34_C545_DROP_SLOT_COUNT,
                       spec->defsAnchor);
    ok &= expect_array("leader G0425 after drop", probe->leaderG0425AfterDrop,
                       emptySlots, DM1_PC34_C545_DROP_SLOT_COUNT,
                       spec->f0284OwnershipAnchor);
    ok &= expect_int("visible slot empty", probe->visibleSlotEmptyAfterDrop,
                     1, spec->panelF0352Anchor);
    ok &= expect_int("panel redraw count", probe->panelF0352RedrawCount, 1,
                     spec->panelF0352Anchor);
    ok &= expect_array("panel redraw slots", probe->panelRedrawSlots,
                       emptySlots, DM1_PC34_C545_DROP_SLOT_COUNT,
                       spec->panelF0352Anchor);
    ok &= expect_int("screen update balanced", probe->screenUpdateBalanced, 1,
                     spec->commandF0378Anchor);
    return ok;
}

static int test_negative_case(
    const DM1_V1_ChestScrollWheelDropToFloorNonLeaderC030ProbePc34* probe,
    const DM1_V1_ChestScrollWheelDropToFloorNonLeaderC030SpecPc34* spec)
{
    const int emptySlots[DM1_PC34_C545_DROP_SLOT_COUNT] = {
        DM1_PC34_C545_DROP_NONE,
        DM1_PC34_C545_DROP_NONE,
        DM1_PC34_C545_DROP_NONE,
        DM1_PC34_C545_DROP_NONE,
        DM1_PC34_C545_DROP_NONE,
        DM1_PC34_C545_DROP_NONE,
        DM1_PC34_C545_DROP_NONE,
        DM1_PC34_C545_DROP_NONE
    };
    int ok = 1;

    ok &= expect_int("negative no C545", probe->negativeEventTriggered, 0,
                     spec->commandF0378Anchor);
    ok &= expect_int("negative no floor write",
                     probe->negativeFloorWriteCount, 0,
                     spec->f0300ClearAnchor);
    ok &= expect_int("negative leader hand", probe->negativeLeaderHandAfter,
                     DM1_PC34_C545_DROP_NONE, spec->f0297PutAnchor);
    ok &= expect_int("negative leader C30 sentinel",
                     probe->negativeLeaderC30After,
                     DM1_PC34_C545_DROP_NEGATIVE_SENTINEL,
                     spec->f0301WriteAnchor);
    ok &= expect_int("negative non-leader before",
                     probe->negativeNonLeaderC30Before,
                     DM1_PC34_C545_DROP_NONE, spec->f0302DispatchAnchor);
    ok &= expect_int("negative non-leader after",
                     probe->negativeNonLeaderC30After,
                     DM1_PC34_C545_DROP_NONE, spec->f0302DispatchAnchor);
    ok &= expect_int("negative owner after",
                     probe->negativeOpenChestOwnerAfter,
                     DM1_PC34_C545_DROP_OWNER_INDEX,
                     spec->f0284OwnershipAnchor);
    ok &= expect_array("negative G0425 after", probe->negativeG0425After,
                       emptySlots, DM1_PC34_C545_DROP_SLOT_COUNT,
                       spec->f0333OpenAnchor);
    ok &= expect_int("negative no redraw", probe->negativePanelRedrawCount, 0,
                     spec->panelF0352Anchor);
    return ok;
}

int main(void)
{
    DM1_V1_ChestScrollWheelDropToFloorNonLeaderC030ProbePc34 probe;
    const DM1_V1_ChestScrollWheelDropToFloorNonLeaderC030SpecPc34* spec =
        dm1_v1_chest_scroll_wheel_drop_to_floor_non_leader_c030_spec_pc34();
    int ok = 1;

    ok &= expect_int(
        "probe run",
        dm1_v1_chest_scroll_wheel_drop_to_floor_non_leader_c030_pc34(&probe),
        1, spec->f0333OpenAnchor);
    ok &= test_source_evidence(spec);
    ok &= test_constants(spec);
    ok &= test_initial_state(&probe, spec);
    ok &= test_c545_dispatch(&probe, spec);
    ok &= test_drop_mutation(&probe, spec);
    ok &= test_negative_case(&probe, spec);
    ok &= expect_int("assertion floor", g_assertions >= 100, 1,
                     spec->defsAnchor);

    printf("SUMMARY assertions=%d failures=%d\n", g_assertions, g_failures);
    return (ok && g_failures == 0 && g_assertions >= 100) ? 0 : 1;
}
