#include "dm1/dm1_v1_chest_scroll_wheel_drop_onto_open_chest_slot_pc34_compat.h"

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
        char itemLabel[128];

        snprintf(itemLabel, sizeof(itemLabel), "%s[%d]", label, i);
        ok &= expect_int(itemLabel, got[i], want[i], anchor);
    }
    return ok;
}

static int test_source_evidence(
    const DM1_V1_ChestScrollWheelDropOntoOpenChestSlotSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_contains("source F0333", spec->sourceEvidence,
                          "CHEST.C F0333:30-67",
                          spec->f0333OpenAnchor);
    ok &= expect_contains("source F0334", spec->sourceEvidence,
                          "CHEST.C F0334:113-132",
                          spec->f0334CloseAnchor);
    ok &= expect_contains("source F0297", spec->sourceEvidence,
                          "CHAMPION.C F0297:243-298",
                          spec->f0297PutAnchor);
    ok &= expect_contains("source F0298", spec->sourceEvidence,
                          "CHAMPION.C F0298:270-298",
                          spec->f0298RemoveAnchor);
    ok &= expect_contains("source F0300", spec->sourceEvidence,
                          "CHAMPION.C F0300:511-515",
                          spec->f0300ClearAnchor);
    ok &= expect_contains("source F0301", spec->sourceEvidence,
                          "CHAMPION.C F0301:606-614",
                          spec->f0301WriteAnchor);
    ok &= expect_contains("source F0302", spec->sourceEvidence,
                          "CHAMPION.C F0302:662-714",
                          spec->f0302DispatchAnchor);
    ok &= expect_contains("source F0352", spec->sourceEvidence,
                          "PANEL.C F0352", spec->panelF0352Anchor);
    ok &= expect_contains("source F0378", spec->sourceEvidence,
                          "COMMAND.C F0378:1973-1983",
                          spec->commandF0378Anchor);
    ok &= expect_contains("source C539", spec->sourceEvidence, "C539",
                          spec->defsAnchor);
    ok &= expect_contains("source G4055", spec->sourceEvidence, "G4055",
                          spec->defsAnchor);
    ok &= expect_contains("source G0425", spec->sourceEvidence, "G0425",
                          spec->defsAnchor);
    ok &= expect_contains("source G0426", spec->sourceEvidence, "G0426",
                          spec->defsAnchor);
    return ok;
}

static int test_defs(
    const DM1_V1_ChestScrollWheelDropOntoOpenChestSlotSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("C10_COLOR_FLESH",
                     DM1_PC34_C539_DROP_C10_COLOR_FLESH, 10,
                     spec->defsAnchor);
    ok &= expect_int("C30", DM1_PC34_C539_DROP_C30, 30,
                     spec->defsAnchor);
    ok &= expect_int("C32", DM1_PC34_C539_DROP_C32, 32,
                     spec->defsAnchor);
    ok &= expect_int("C37", DM1_PC34_C539_DROP_C37, 37,
                     spec->defsAnchor);
    ok &= expect_int("C537", DM1_PC34_C539_DROP_C537, 537,
                     spec->defsAnchor);
    ok &= expect_int("C539", DM1_PC34_C539_DROP_C539, 539,
                     spec->defsAnchor);
    ok &= expect_int("C544", DM1_PC34_C539_DROP_C544, 544,
                     spec->defsAnchor);
    ok &= expect_int("mouth focus zone", DM1_PC34_C539_DROP_C545_MOUTH,
                     545, spec->defsAnchor);
    ok &= expect_int("slot count", DM1_PC34_C539_DROP_SLOT_COUNT, 8,
                     spec->defsAnchor);
    ok &= expect_int("panel M569", DM1_PC34_C539_DROP_PANEL_M569, 569,
                     spec->commandF0378Anchor);
    ok &= expect_int("click C059", DM1_PC34_C539_DROP_CLICK_C059, 59,
                     spec->commandF0378Anchor);
    return ok;
}

static int test_initial_state(
    const DM1_V1_ChestScrollWheelDropOntoOpenChestSlotProbePc34* probe,
    const DM1_V1_ChestScrollWheelDropOntoOpenChestSlotSpecPc34* spec)
{
    const int slotsWant[DM1_PC34_C539_DROP_SLOT_COUNT] = {
        DM1_PC34_C539_DROP_CHEST_ITEM0,
        DM1_PC34_C539_DROP_CHEST_ITEM1,
        DM1_PC34_C539_DROP_NONE,
        DM1_PC34_C539_DROP_CHEST_ITEM3,
        DM1_PC34_C539_DROP_CHEST_ITEM4,
        DM1_PC34_C539_DROP_NONE,
        DM1_PC34_C539_DROP_NONE,
        DM1_PC34_C539_DROP_NONE
    };
    const int iconsWant[DM1_PC34_C539_DROP_SLOT_COUNT] = {
        DM1_PC34_C539_DROP_CHEST_ITEM0 & 0x00FF,
        DM1_PC34_C539_DROP_CHEST_ITEM1 & 0x00FF,
        DM1_PC34_C539_DROP_NONE,
        DM1_PC34_C539_DROP_CHEST_ITEM3 & 0x00FF,
        DM1_PC34_C539_DROP_CHEST_ITEM4 & 0x00FF,
        DM1_PC34_C539_DROP_NONE,
        DM1_PC34_C539_DROP_NONE,
        DM1_PC34_C539_DROP_NONE
    };
    int ok = 1;

    ok &= expect_int("open chest thing", probe->openChestThing,
                     DM1_PC34_C539_DROP_OPEN_CHEST,
                     spec->f0333OpenAnchor);
    ok &= expect_array("initial slots", probe->initialSlots, slotsWant,
                       DM1_PC34_C539_DROP_SLOT_COUNT,
                       spec->f0333OpenAnchor);
    ok &= expect_array("initial icons", probe->initialIcons, iconsWant,
                       DM1_PC34_C539_DROP_SLOT_COUNT,
                       spec->objectAnchor);
    ok &= expect_int("initial leader hand", probe->initialLeaderHand,
                     DM1_PC34_C539_DROP_HAND_ITEM,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("initial leader icon", probe->initialLeaderIcon,
                     DM1_PC34_C539_DROP_HAND_ITEM & 0x00FF,
                     spec->objectAnchor);
    ok &= expect_int("initial leader load", probe->initialLeaderLoad, 45,
                     spec->f0298RemoveAnchor);
    ok &= expect_int("initial leader attributes",
                     probe->initialLeaderAttributes, 0,
                     spec->f0298RemoveAnchor);
    ok &= expect_int("visible count before drop",
                     probe->materializedVisibleCount, 4,
                     spec->f0333OpenAnchor);
    ok &= expect_int("same-open guard stable",
                     probe->sameOpenGuardKeptSlots, 1,
                     spec->f0333OpenAnchor);
    return ok;
}

static int test_scroll_route(
    const DM1_V1_ChestScrollWheelDropOntoOpenChestSlotProbePc34* probe,
    const DM1_V1_ChestScrollWheelDropOntoOpenChestSlotSpecPc34* spec)
{
    const int focusWant[4] = { 0, 1, 2, 0 };
    const int zoneWant[4] = {
        DM1_PC34_C539_DROP_C545_MOUTH,
        DM1_PC34_C539_DROP_C540,
        DM1_PC34_C539_DROP_C541,
        DM1_PC34_C539_DROP_C545_MOUTH
    };
    int ok = 1;

    ok &= expect_array("focus trace", probe->focusTrace, focusWant, 4,
                       spec->commandF0378Anchor);
    ok &= expect_array("focus zone trace", probe->focusZoneTrace, zoneWant,
                       4, spec->defsAnchor);
    ok &= expect_int("wheel ticks", probe->wheelTicks, 3,
                     spec->commandF0378Anchor);
    ok &= expect_int("mouth focus start", probe->mouthFocusStartZone,
                     DM1_PC34_C539_DROP_C545_MOUTH, spec->defsAnchor);
    ok &= expect_int("focus preserved after drop",
                     probe->focusPreservedAfterDrop, 1,
                     spec->commandF0378Anchor);
    ok &= expect_int("mouth focus survived drop",
                     probe->mouthFocusSurvivedDrop, 1,
                     spec->commandF0378Anchor);
    ok &= expect_int("focus rotates after drop",
                     probe->focusRotatesAfterDrop, 1,
                     spec->commandF0378Anchor);
    ok &= expect_int("drop target C539", probe->dropTargetZone,
                     DM1_PC34_C539_DROP_C539, spec->defsAnchor);
    ok &= expect_int("drop target slot index", probe->dropTargetSlotIndex, 2,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("drop target slot box", probe->dropTargetSlotBox,
                     DM1_PC34_C539_DROP_C32,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("drop command", probe->dropTargetCommand,
                     DM1_PC34_C539_DROP_CLICK_C059,
                     spec->commandF0378Anchor);
    ok &= expect_int("drop C30 offset", probe->dropC30Offset, 2,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("F0378 dispatch count",
                     probe->commandF0378DispatchCount, 1,
                     spec->commandF0378Anchor);
    ok &= expect_int("F0352 redraw count", probe->panelF0352RedrawCount, 1,
                     spec->panelF0352Anchor);
    ok &= expect_int("F0302 dispatch count", probe->f0302DispatchCount, 1,
                     spec->f0302DispatchAnchor);
    return ok;
}

static int test_drop_mutation(
    const DM1_V1_ChestScrollWheelDropOntoOpenChestSlotProbePc34* probe,
    const DM1_V1_ChestScrollWheelDropOntoOpenChestSlotSpecPc34* spec)
{
    const int slotsWant[DM1_PC34_C539_DROP_SLOT_COUNT] = {
        DM1_PC34_C539_DROP_CHEST_ITEM0,
        DM1_PC34_C539_DROP_CHEST_ITEM1,
        DM1_PC34_C539_DROP_HAND_ITEM,
        DM1_PC34_C539_DROP_CHEST_ITEM3,
        DM1_PC34_C539_DROP_CHEST_ITEM4,
        DM1_PC34_C539_DROP_NONE,
        DM1_PC34_C539_DROP_NONE,
        DM1_PC34_C539_DROP_NONE
    };
    const int iconsWant[DM1_PC34_C539_DROP_SLOT_COUNT] = {
        DM1_PC34_C539_DROP_CHEST_ITEM0 & 0x00FF,
        DM1_PC34_C539_DROP_CHEST_ITEM1 & 0x00FF,
        DM1_PC34_C539_DROP_HAND_ITEM & 0x00FF,
        DM1_PC34_C539_DROP_CHEST_ITEM3 & 0x00FF,
        DM1_PC34_C539_DROP_CHEST_ITEM4 & 0x00FF,
        DM1_PC34_C539_DROP_NONE,
        DM1_PC34_C539_DROP_NONE,
        DM1_PC34_C539_DROP_NONE
    };
    const int tailWant[3] = {
        DM1_PC34_C539_DROP_HAND_ITEM,
        DM1_PC34_C539_DROP_CHEST_ITEM3,
        DM1_PC34_C539_DROP_CHEST_ITEM4
    };
    int ok = 1;

    ok &= expect_int("F0302 leader snapshot",
                     probe->f0302LeaderSnapshot,
                     DM1_PC34_C539_DROP_HAND_ITEM,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("F0302 slot snapshot", probe->f0302SlotSnapshot,
                     DM1_PC34_C539_DROP_NONE,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("F0302 empty-empty rejected",
                     probe->f0302EmptyEmptyRejected, 0,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("F0302 allowed slot guard",
                     probe->f0302AllowedSlotGuardPassed, 1,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("F0298 remove count", probe->f0298RemoveCount, 1,
                     spec->f0298RemoveAnchor);
    ok &= expect_int("F0298 removed thing", probe->f0298RemovedThing,
                     DM1_PC34_C539_DROP_HAND_ITEM,
                     spec->f0298RemoveAnchor);
    ok &= expect_int("F0298 cleared leader",
                     probe->f0298ClearedLeaderHand, 1,
                     spec->f0298RemoveAnchor);
    ok &= expect_int("F0300 clear skipped", probe->f0300ClearCount, 0,
                     spec->f0300ClearAnchor);
    ok &= expect_int("F0301 write count", probe->f0301WriteCount, 1,
                     spec->f0301WriteAnchor);
    ok &= expect_int("F0301 written thing", probe->f0301WrittenThing,
                     DM1_PC34_C539_DROP_HAND_ITEM,
                     spec->f0301WriteAnchor);
    ok &= expect_int("F0301 slot value", probe->f0301WrittenSlotValue,
                     DM1_PC34_C539_DROP_HAND_ITEM,
                     spec->f0301WriteAnchor);
    ok &= expect_int("F0297 put skipped", probe->f0297PutCount, 0,
                     spec->f0297PutAnchor);
    ok &= expect_int("leader hand after drop", probe->leaderHandAfterDrop,
                     DM1_PC34_C539_DROP_NONE,
                     spec->f0298RemoveAnchor);
    ok &= expect_int("leader icon after drop", probe->leaderIconAfterDrop,
                     DM1_PC34_C539_DROP_NONE, spec->objectAnchor);
    ok &= expect_int("leader load after drop", probe->leaderLoadAfterDrop,
                     45, spec->f0301WriteAnchor);
    ok &= expect_int("leader attrs after drop",
                     probe->leaderAttributesAfterDrop,
                     DM1_PC34_C539_DROP_LOAD_MASK |
                         DM1_PC34_C539_DROP_VIEWPORT_MASK,
                     spec->f0301WriteAnchor);
    ok &= expect_array("chest after drop", probe->chestAfterDrop,
                       slotsWant, DM1_PC34_C539_DROP_SLOT_COUNT,
                       spec->f0301WriteAnchor);
    ok &= expect_array("chest icons after drop", probe->chestIconsAfterDrop,
                       iconsWant, DM1_PC34_C539_DROP_SLOT_COUNT,
                       spec->objectAnchor);
    ok &= expect_int("inserted slot filled", probe->insertedSlotFilled, 1,
                     spec->f0301WriteAnchor);
    ok &= expect_int("inserted stored once", probe->insertedThingStoredOnce,
                     1, spec->f0301WriteAnchor);
    ok &= expect_array("C30 tail preserved after drop",
                       probe->c30TailPreservedAfterDrop, tailWant, 3,
                       spec->f0334CloseAnchor);
    ok &= expect_int("screen update enables",
                     probe->screenUpdateEnableCount, 4, spec->mouseAnchor);
    ok &= expect_int("screen update disables",
                     probe->screenUpdateDisableCount, 4, spec->mouseAnchor);
    ok &= expect_int("screen update balanced",
                     probe->screenUpdateBalanced, 1, spec->mouseAnchor);
    return ok;
}

static int test_close_reopen(
    const DM1_V1_ChestScrollWheelDropOntoOpenChestSlotProbePc34* probe,
    const DM1_V1_ChestScrollWheelDropOntoOpenChestSlotSpecPc34* spec)
{
    const int chainWant[DM1_PC34_C539_DROP_SLOT_COUNT] = {
        DM1_PC34_C539_DROP_CHEST_ITEM0,
        DM1_PC34_C539_DROP_CHEST_ITEM1,
        DM1_PC34_C539_DROP_HAND_ITEM,
        DM1_PC34_C539_DROP_CHEST_ITEM3,
        DM1_PC34_C539_DROP_CHEST_ITEM4,
        DM1_PC34_C539_DROP_NONE,
        DM1_PC34_C539_DROP_NONE,
        DM1_PC34_C539_DROP_NONE
    };
    const int iconsWant[DM1_PC34_C539_DROP_SLOT_COUNT] = {
        DM1_PC34_C539_DROP_CHEST_ITEM0 & 0x00FF,
        DM1_PC34_C539_DROP_CHEST_ITEM1 & 0x00FF,
        DM1_PC34_C539_DROP_HAND_ITEM & 0x00FF,
        DM1_PC34_C539_DROP_CHEST_ITEM3 & 0x00FF,
        DM1_PC34_C539_DROP_CHEST_ITEM4 & 0x00FF,
        DM1_PC34_C539_DROP_NONE,
        DM1_PC34_C539_DROP_NONE,
        DM1_PC34_C539_DROP_NONE
    };
    const int tailWant[3] = {
        DM1_PC34_C539_DROP_HAND_ITEM,
        DM1_PC34_C539_DROP_CHEST_ITEM3,
        DM1_PC34_C539_DROP_CHEST_ITEM4
    };
    int ok = 1;

    ok &= expect_int("close count", probe->closeCount, 5,
                     spec->f0334CloseAnchor);
    ok &= expect_int("close head", probe->closeHead,
                     DM1_PC34_C539_DROP_CHEST_ITEM0,
                     spec->f0334CloseAnchor);
    ok &= expect_int("close end sentinel", probe->closeEndSentinel,
                     DM1_PC34_C539_DROP_END,
                     spec->f0334CloseAnchor);
    ok &= expect_array("closed slots", probe->closedSlots, chainWant,
                       DM1_PC34_C539_DROP_SLOT_COUNT,
                       spec->f0334CloseAnchor);
    ok &= expect_array("closed icons", probe->closedIcons, iconsWant,
                       DM1_PC34_C539_DROP_SLOT_COUNT,
                       spec->objectAnchor);
    ok &= expect_array("closed tail preserved", probe->closedTailPreserved,
                       tailWant, 3, spec->f0334CloseAnchor);
    ok &= expect_int("G0425 cleared after close",
                     probe->g0425ClearedAfterClose, 1,
                     spec->f0334CloseAnchor);
    ok &= expect_int("open chest after close", probe->openChestAfterClose,
                     DM1_PC34_C539_DROP_NONE,
                     spec->f0334CloseAnchor);
    ok &= expect_int("reopen count", probe->reopenCount, 5,
                     spec->f0333OpenAnchor);
    ok &= expect_int("reopened open chest", probe->reopenedOpenChest,
                     DM1_PC34_C539_DROP_OPEN_CHEST,
                     spec->f0333OpenAnchor);
    ok &= expect_array("reopened slots", probe->reopenedSlots, chainWant,
                       DM1_PC34_C539_DROP_SLOT_COUNT,
                       spec->f0333OpenAnchor);
    ok &= expect_array("reopened icons", probe->reopenedIcons, iconsWant,
                       DM1_PC34_C539_DROP_SLOT_COUNT,
                       spec->objectAnchor);
    ok &= expect_int("reopened C539 thing", probe->reopenedC539Thing,
                     DM1_PC34_C539_DROP_HAND_ITEM,
                     spec->f0333OpenAnchor);
    ok &= expect_int("reopened contains inserted",
                     probe->reopenedChainContainsInserted, 1,
                     spec->f0333OpenAnchor);
    ok &= expect_int("reopened chain order",
                     probe->reopenedChainOrderPreserved, 1,
                     spec->f0333OpenAnchor);
    return ok;
}

static int test_negative_empty_empty(
    const DM1_V1_ChestScrollWheelDropOntoOpenChestSlotProbePc34* probe,
    const DM1_V1_ChestScrollWheelDropOntoOpenChestSlotSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("negative empty hand before",
                     probe->negativeEmptyHandBefore,
                     DM1_PC34_C539_DROP_NONE,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("negative slot before", probe->negativeSlotBefore,
                     DM1_PC34_C539_DROP_NONE,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("negative drop rejected", probe->negativeDropRejected,
                     1, spec->f0302DispatchAnchor);
    ok &= expect_int("negative slot after", probe->negativeSlotAfter,
                     DM1_PC34_C539_DROP_NONE,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("negative leader after", probe->negativeLeaderAfter,
                     DM1_PC34_C539_DROP_NONE,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("negative write skipped", probe->negativeWriteCount, 0,
                     spec->f0301WriteAnchor);
    return ok;
}

int main(void)
{
    DM1_V1_ChestScrollWheelDropOntoOpenChestSlotProbePc34 probe;
    const DM1_V1_ChestScrollWheelDropOntoOpenChestSlotSpecPc34* spec =
        dm1_v1_chest_scroll_wheel_drop_onto_open_chest_slot_spec_pc34();
    int ok = 1;

    ok &= expect_int(
        "probe run",
        dm1_v1_chest_scroll_wheel_drop_onto_open_chest_slot_pc34(&probe),
        1,
        spec->f0333OpenAnchor);
    ok &= test_source_evidence(spec);
    ok &= test_defs(spec);
    ok &= test_initial_state(&probe, spec);
    ok &= test_scroll_route(&probe, spec);
    ok &= test_drop_mutation(&probe, spec);
    ok &= test_close_reopen(&probe, spec);
    ok &= test_negative_empty_empty(&probe, spec);

    printf("SUMMARY assertions=%d failures=%d\n", g_assertions, g_failures);
    return (ok && g_failures == 0) ? 0 : 1;
}
