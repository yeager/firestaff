#include "dm1_v1_chest_scroll_wheel_pull_from_chest_pc34_compat.h"

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
    const DM1_V1_ChestScrollWheelPullFromChestSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_contains("source F0333", spec->sourceEvidence,
                          "CHEST.C F0333:30-67",
                          spec->f0333OpenAnchor);
    ok &= expect_contains("source F0334", spec->sourceEvidence,
                          "CHEST.C F0334:117-132",
                          spec->f0334CloseAnchor);
    ok &= expect_contains("source F0297", spec->sourceEvidence,
                          "CHAMPION.C F0297:243-268",
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
                          "CHAMPION.C F0302:662-710",
                          spec->f0302DispatchAnchor);
    ok &= expect_contains("source F0378", spec->sourceEvidence,
                          "COMMAND.C F0378:1973-1983",
                          spec->commandF0378Anchor);
    ok &= expect_contains("source F0359", spec->sourceEvidence,
                          "COMMAND.C F0359", spec->commandF0359Anchor);
    ok &= expect_contains("source F0344", spec->sourceEvidence,
                          "PANEL.C F0344:1895-1944",
                          spec->panelF0344Anchor);
    ok &= expect_contains("source F0345", spec->sourceEvidence,
                          "F0345:1946-1999", spec->panelF0345Anchor);
    ok &= expect_contains("source mouse", spec->sourceEvidence,
                          "UTAMSCR.C F0077:147-151", spec->mouseAnchor);
    ok &= expect_contains("source object", spec->sourceEvidence,
                          "OBJECT.C F0033:147-212", spec->objectAnchor);
    ok &= expect_contains("source blit", spec->sourceEvidence,
                          "BLITMASK.C F0133:30-33",
                          spec->blitMaskAnchor);
    ok &= expect_contains("source defs", spec->sourceEvidence,
                          "DEFS.H:2088", spec->defsAnchor);
    ok &= expect_contains("source G4055", spec->sourceEvidence, "G4055",
                          spec->defsAnchor);
    return ok;
}

static int test_defs(
    const DM1_V1_ChestScrollWheelPullFromChestSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("C10_COLOR_FLESH",
                     DM1_PC34_PULL_C10_COLOR_FLESH, 10,
                     spec->defsAnchor);
    ok &= expect_int("C30", DM1_PC34_PULL_C30, 30, spec->defsAnchor);
    ok &= expect_int("C31", DM1_PC34_PULL_C31, 31, spec->defsAnchor);
    ok &= expect_int("C32", DM1_PC34_PULL_C32, 32, spec->defsAnchor);
    ok &= expect_int("C33", DM1_PC34_PULL_C33, 33, spec->defsAnchor);
    ok &= expect_int("C34", DM1_PC34_PULL_C34, 34, spec->defsAnchor);
    ok &= expect_int("C35", DM1_PC34_PULL_C35, 35, spec->defsAnchor);
    ok &= expect_int("C36", DM1_PC34_PULL_C36, 36, spec->defsAnchor);
    ok &= expect_int("C37", DM1_PC34_PULL_C37, 37, spec->defsAnchor);
    ok &= expect_int("C537", DM1_PC34_PULL_C537, 537, spec->defsAnchor);
    ok &= expect_int("C538", DM1_PC34_PULL_C538, 538, spec->defsAnchor);
    ok &= expect_int("C539", DM1_PC34_PULL_C539, 539, spec->defsAnchor);
    ok &= expect_int("C540", DM1_PC34_PULL_C540, 540, spec->defsAnchor);
    ok &= expect_int("C541", DM1_PC34_PULL_C541, 541, spec->defsAnchor);
    ok &= expect_int("C542", DM1_PC34_PULL_C542, 542, spec->defsAnchor);
    ok &= expect_int("C543", DM1_PC34_PULL_C543, 543, spec->defsAnchor);
    ok &= expect_int("C544", DM1_PC34_PULL_C544, 544, spec->defsAnchor);
    ok &= expect_int("slot count", DM1_PC34_PULL_SLOT_COUNT, 8,
                     spec->defsAnchor);
    ok &= expect_int("focus count", DM1_PC34_PULL_FOCUS_COUNT, 3,
                     spec->commandF0359Anchor);
    ok &= expect_int("panel M569", DM1_PC34_PULL_PANEL_M569, 569,
                     spec->commandF0378Anchor);
    ok &= expect_int("command C040", DM1_PC34_PULL_COMMAND_C040, 40,
                     spec->commandF0378Anchor);
    ok &= expect_int("click C059", DM1_PC34_PULL_CLICK_C059, 59,
                     spec->commandF0378Anchor);
    return ok;
}

static int test_initial_state(
    const DM1_V1_ChestScrollWheelPullFromChestProbePc34* probe,
    const DM1_V1_ChestScrollWheelPullFromChestSpecPc34* spec)
{
    const int slotsWant[DM1_PC34_PULL_SLOT_COUNT] = {
        DM1_PC34_PULL_CHEST_ITEM0,
        DM1_PC34_PULL_CHEST_ITEM1,
        DM1_PC34_PULL_CHEST_ITEM2,
        DM1_PC34_PULL_CHEST_ITEM3,
        DM1_PC34_PULL_NONE,
        DM1_PC34_PULL_NONE,
        DM1_PC34_PULL_NONE,
        DM1_PC34_PULL_NONE
    };
    const int iconsWant[DM1_PC34_PULL_SLOT_COUNT] = {
        DM1_PC34_PULL_CHEST_ITEM0 & 0x00FF,
        DM1_PC34_PULL_CHEST_ITEM1 & 0x00FF,
        DM1_PC34_PULL_CHEST_ITEM2 & 0x00FF,
        DM1_PC34_PULL_CHEST_ITEM3 & 0x00FF,
        DM1_PC34_PULL_NONE,
        DM1_PC34_PULL_NONE,
        DM1_PC34_PULL_NONE,
        DM1_PC34_PULL_NONE
    };
    int ok = 1;

    ok &= expect_int("open chest thing", probe->openChestThing,
                     DM1_PC34_PULL_OPEN_CHEST, spec->f0333OpenAnchor);
    ok &= expect_array("initial C537-C544 things", probe->initialSlots,
                       slotsWant, DM1_PC34_PULL_SLOT_COUNT,
                       spec->f0333OpenAnchor);
    ok &= expect_array("initial C537-C544 icons", probe->initialIcons,
                       iconsWant, DM1_PC34_PULL_SLOT_COUNT,
                       spec->objectAnchor);
    ok &= expect_int("initial leader hand empty", probe->initialLeaderHand,
                     DM1_PC34_PULL_NONE, spec->f0302DispatchAnchor);
    ok &= expect_int("initial leader icon empty", probe->initialLeaderIcon,
                     DM1_PC34_PULL_NONE, spec->objectAnchor);
    ok &= expect_int("initial leader load", probe->initialLeaderLoad, 40,
                     spec->f0297PutAnchor);
    ok &= expect_int("initial leader attributes",
                     probe->initialLeaderAttributes, 0,
                     spec->f0297PutAnchor);
    ok &= expect_int("materialized visible count",
                     probe->materializedVisibleCount, 4,
                     spec->f0333OpenAnchor);
    ok &= expect_int("same-open guard kept slots",
                     probe->sameOpenGuardKeptSlots, 1,
                     spec->f0333OpenAnchor);
    ok &= expect_int("initial focus index", probe->focusTrace[0], 0,
                     spec->commandF0359Anchor);
    ok &= expect_int("initial focus zone", probe->focusZoneTrace[0],
                     DM1_PC34_PULL_C540, spec->defsAnchor);
    return ok;
}

static int test_dispatch_route(
    const DM1_V1_ChestScrollWheelPullFromChestProbePc34* probe,
    const DM1_V1_ChestScrollWheelPullFromChestSpecPc34* spec)
{
    const int focusWant[4] = { 0, 1, 2, 0 };
    const int zoneWant[4] = {
        DM1_PC34_PULL_C540,
        DM1_PC34_PULL_C541,
        DM1_PC34_PULL_C542,
        DM1_PC34_PULL_C540
    };
    int ok = 1;

    ok &= expect_array("focus trace", probe->focusTrace, focusWant, 4,
                       spec->commandF0359Anchor);
    ok &= expect_array("focus zone trace", probe->focusZoneTrace, zoneWant,
                       4, spec->defsAnchor);
    ok &= expect_int("focus preserved after pull",
                     probe->focusPreservedAfterPull, 1,
                     spec->commandF0359Anchor);
    ok &= expect_int("focus rotates after pull", probe->focusRotatesAfterPull,
                     1, spec->commandF0359Anchor);
    ok &= expect_int("rotation wheel ticks", probe->rotationWheelTicks, 3,
                     spec->commandF0359Anchor);
    ok &= expect_int("rotation partial masks",
                     probe->rotationPartialMaskDispatches, 3,
                     spec->blitMaskAnchor);
    ok &= expect_int("rotation leader hand stable",
                     probe->rotationLeaderHandStable, 1,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("pull wheel tick", probe->pullWheelTick, 1,
                     spec->commandF0359Anchor);
    ok &= expect_int("pull target C538", probe->pullTargetZone,
                     DM1_PC34_PULL_C538, spec->panelF0344Anchor);
    ok &= expect_int("pull target slot index", probe->pullTargetSlotIndex, 1,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("pull target command", probe->pullTargetCommand,
                     DM1_PC34_PULL_CLICK_C059, spec->commandF0378Anchor);
    ok &= expect_int("pull target slot box", probe->pullTargetSlotBox,
                     DM1_PC34_PULL_C31, spec->f0302DispatchAnchor);
    ok &= expect_int("pull champion slot index",
                     probe->pullChampionSlotIndex, DM1_PC34_PULL_C31,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("pull C30 offset", probe->pullC30Offset, 1,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("pull target not focus zone",
                     probe->pullTargetNotFocusZone, 1,
                     spec->panelF0344Anchor);
    ok &= expect_int("F0378 dispatch count",
                     probe->commandF0378DispatchCount, 1,
                     spec->commandF0378Anchor);
    ok &= expect_int("F0359 wheel count", probe->commandF0359WheelCount, 4,
                     spec->commandF0359Anchor);
    ok &= expect_int("panel click count", probe->panelClickCount, 1,
                     spec->panelF0344Anchor);
    ok &= expect_int("panel release count", probe->panelReleaseCount, 1,
                     spec->panelF0345Anchor);
    ok &= expect_int("F0302 dispatch count", probe->f0302DispatchCount, 1,
                     spec->f0302DispatchAnchor);
    return ok;
}

static int test_pull_mutation(
    const DM1_V1_ChestScrollWheelPullFromChestProbePc34* probe,
    const DM1_V1_ChestScrollWheelPullFromChestSpecPc34* spec)
{
    const int slotsWant[DM1_PC34_PULL_SLOT_COUNT] = {
        DM1_PC34_PULL_CHEST_ITEM0,
        DM1_PC34_PULL_NONE,
        DM1_PC34_PULL_CHEST_ITEM2,
        DM1_PC34_PULL_CHEST_ITEM3,
        DM1_PC34_PULL_NONE,
        DM1_PC34_PULL_NONE,
        DM1_PC34_PULL_NONE,
        DM1_PC34_PULL_NONE
    };
    const int iconsWant[DM1_PC34_PULL_SLOT_COUNT] = {
        DM1_PC34_PULL_CHEST_ITEM0 & 0x00FF,
        DM1_PC34_PULL_NONE,
        DM1_PC34_PULL_CHEST_ITEM2 & 0x00FF,
        DM1_PC34_PULL_CHEST_ITEM3 & 0x00FF,
        DM1_PC34_PULL_NONE,
        DM1_PC34_PULL_NONE,
        DM1_PC34_PULL_NONE,
        DM1_PC34_PULL_NONE
    };
    const int chainWant[3] = {
        DM1_PC34_PULL_CHEST_ITEM0,
        DM1_PC34_PULL_CHEST_ITEM2,
        DM1_PC34_PULL_CHEST_ITEM3
    };
    int ok = 1;

    ok &= expect_int("F0302 leader snapshot",
                     probe->f0302LeaderSnapshot, DM1_PC34_PULL_NONE,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("F0302 slot snapshot",
                     probe->f0302SlotSnapshot,
                     DM1_PC34_PULL_CHEST_ITEM1,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("F0302 empty-empty rejected",
                     probe->f0302EmptyEmptyRejected, 0,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("F0302 allowed slot guard",
                     probe->f0302AllowedSlotGuardPassed, 1,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("F0300 clear count", probe->f0300ClearCount, 1,
                     spec->f0300ClearAnchor);
    ok &= expect_int("F0300 cleared thing", probe->f0300ClearedThing,
                     DM1_PC34_PULL_CHEST_ITEM1, spec->f0300ClearAnchor);
    ok &= expect_int("F0300 cleared slot value",
                     probe->f0300ClearedSlotValue, DM1_PC34_PULL_NONE,
                     spec->f0300ClearAnchor);
    ok &= expect_int("F0301 write skipped", probe->f0301WriteCount, 0,
                     spec->f0301WriteAnchor);
    ok &= expect_int("F0297 put count", probe->f0297PutCount, 1,
                     spec->f0297PutAnchor);
    ok &= expect_int("F0298 remove skipped", probe->f0298RemoveCount, 0,
                     spec->f0298RemoveAnchor);
    ok &= expect_int("leader hand after pull", probe->leaderHandAfterPull,
                     DM1_PC34_PULL_CHEST_ITEM1, spec->f0297PutAnchor);
    ok &= expect_int("leader icon after pull", probe->leaderIconAfterPull,
                     DM1_PC34_PULL_CHEST_ITEM1 & 0x00FF,
                     spec->objectAnchor);
    ok &= expect_int("leader load after pull", probe->leaderLoadAfterPull,
                     42, spec->f0297PutAnchor);
    ok &= expect_int("leader load mask after pull",
                     probe->leaderAttributesAfterPull,
                     DM1_PC34_PULL_LOAD_MASK, spec->f0297PutAnchor);
    ok &= expect_array("chest after pull", probe->chestAfterPull,
                       slotsWant, DM1_PC34_PULL_SLOT_COUNT,
                       spec->f0300ClearAnchor);
    ok &= expect_array("chest icons after pull",
                       probe->chestIconsAfterPull, iconsWant,
                       DM1_PC34_PULL_SLOT_COUNT, spec->objectAnchor);
    ok &= expect_array("chain order after pull",
                       probe->chainOrderPreservedAfterPull, chainWant, 3,
                       spec->f0334CloseAnchor);
    ok &= expect_int("pulled slot cleared", probe->pulledSlotCleared, 1,
                     spec->f0300ClearAnchor);
    ok &= expect_int("pulled thing missing from chest",
                     probe->pulledThingMissingFromChest, 1,
                     spec->f0300ClearAnchor);
    ok &= expect_int("screen update enables",
                     probe->screenUpdateEnableCount, 4,
                     spec->mouseAnchor);
    ok &= expect_int("screen update disables",
                     probe->screenUpdateDisableCount, 4,
                     spec->mouseAnchor);
    ok &= expect_int("screen update balanced",
                     probe->screenUpdateBalanced, 1,
                     spec->mouseAnchor);
    return ok;
}

static int test_close_rewrite(
    const DM1_V1_ChestScrollWheelPullFromChestProbePc34* probe,
    const DM1_V1_ChestScrollWheelPullFromChestSpecPc34* spec)
{
    const int closedWant[DM1_PC34_PULL_SLOT_COUNT] = {
        DM1_PC34_PULL_CHEST_ITEM0,
        DM1_PC34_PULL_CHEST_ITEM2,
        DM1_PC34_PULL_CHEST_ITEM3,
        DM1_PC34_PULL_NONE,
        DM1_PC34_PULL_NONE,
        DM1_PC34_PULL_NONE,
        DM1_PC34_PULL_NONE,
        DM1_PC34_PULL_NONE
    };
    int ok = 1;

    ok &= expect_int("close count", probe->closeCount, 3,
                     spec->f0334CloseAnchor);
    ok &= expect_int("close head", probe->closeHead,
                     DM1_PC34_PULL_CHEST_ITEM0, spec->f0334CloseAnchor);
    ok &= expect_int("close end sentinel", probe->closeEndSentinel,
                     DM1_PC34_PULL_END, spec->f0334CloseAnchor);
    ok &= expect_array("closed slots", probe->closedSlots, closedWant,
                       DM1_PC34_PULL_SLOT_COUNT, spec->f0334CloseAnchor);
    ok &= expect_int("G0425 cleared after close",
                     probe->g0425ClearedAfterClose, 1,
                     spec->f0334CloseAnchor);
    ok &= expect_int("open chest after close",
                     probe->openChestAfterClose, DM1_PC34_PULL_NONE,
                     spec->f0334CloseAnchor);
    ok &= expect_int("pulled thing not closed back into chest",
                     probe->pulledThingNotClosedBackIntoChest, 1,
                     spec->f0334CloseAnchor);
    return ok;
}

int main(void)
{
    DM1_V1_ChestScrollWheelPullFromChestProbePc34 probe;
    const DM1_V1_ChestScrollWheelPullFromChestSpecPc34* spec =
        dm1_v1_chest_scroll_wheel_pull_from_chest_spec_pc34();
    int ok = 1;

    ok &= expect_int("probe run",
                     dm1_v1_chest_scroll_wheel_pull_from_chest_pc34(&probe),
                     1, spec->f0333OpenAnchor);
    ok &= test_source_evidence(spec);
    ok &= test_defs(spec);
    ok &= test_initial_state(&probe, spec);
    ok &= test_dispatch_route(&probe, spec);
    ok &= test_pull_mutation(&probe, spec);
    ok &= test_close_rewrite(&probe, spec);

    printf("SUMMARY assertions=%d failures=%d\n", g_assertions, g_failures);
    return (ok && g_failures == 0) ? 0 : 1;
}
