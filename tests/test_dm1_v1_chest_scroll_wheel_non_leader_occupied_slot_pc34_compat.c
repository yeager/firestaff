#include "dm1_v1_chest_scroll_wheel_non_leader_occupied_slot_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static int expect_int(const char* label, int got, int want, const char* anchor)
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
    if (!anchor || !anchor[0] || !haystack || !needle ||
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

#define EXPECT_INT(label, got, want, anchor) \
    expect_int((label), (int)(got), (int)(want), (anchor))
#define EXPECT_CONTAINS(label, haystack, needle, anchor) \
    expect_contains((label), (haystack), (needle), (anchor))

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
        ok &= EXPECT_INT(itemLabel, got[i], want[i], anchor);
    }
    return ok;
}

static int test_source_evidence(
    const DM1_V1_ChestScrollWheelNonLeaderOccupiedSpecPc34* spec)
{
    const char* evidence =
        dm1_v1_chest_scroll_wheel_non_leader_occupied_slot_source_evidence_pc34();
    int ok = 1;

    ok &= EXPECT_INT("evidence pointer stable",
                     evidence == spec->sourceEvidence, 1,
                     spec->f0333OpenAnchor);
    ok &= EXPECT_CONTAINS("evidence F0333", evidence,
                          "CHEST.C F0333:30-67", spec->f0333OpenAnchor);
    ok &= EXPECT_CONTAINS("evidence F0334", evidence,
                          "CHEST.C F0334:117-132", spec->f0334CloseAnchor);
    ok &= EXPECT_CONTAINS("evidence F0297", evidence,
                          "CHAMPION.C F0297:243-268",
                          spec->f0297PutAnchor);
    ok &= EXPECT_CONTAINS("evidence F0298", evidence,
                          "CHAMPION.C F0298:270-298",
                          spec->f0298RemoveAnchor);
    ok &= EXPECT_CONTAINS("evidence F0300", evidence,
                          "CHAMPION.C F0300:511-515",
                          spec->f0300ClearAnchor);
    ok &= EXPECT_CONTAINS("evidence F0301", evidence,
                          "CHAMPION.C F0301:606-614",
                          spec->f0301WriteAnchor);
    ok &= EXPECT_CONTAINS("evidence F0302", evidence,
                          "CHAMPION.C F0302:662-710",
                          spec->f0302DispatchAnchor);
    ok &= EXPECT_CONTAINS("evidence F0344", evidence,
                          "PANEL.C F0344:1895-1944",
                          spec->panelF0344Anchor);
    ok &= EXPECT_CONTAINS("evidence F0345", evidence,
                          "F0345:1946-1999", spec->panelF0345Anchor);
    ok &= EXPECT_CONTAINS("evidence F0359", evidence,
                          "COMMAND.C F0359:1985-1990",
                          spec->commandF0359Anchor);
    ok &= EXPECT_CONTAINS("evidence F0077", evidence,
                          "MOUSE.C F0077:97-126",
                          spec->mouseF0077Anchor);
    ok &= EXPECT_CONTAINS("evidence F0078", evidence,
                          "MOUSE.C F0078:128-168",
                          spec->mouseF0078Anchor);
    ok &= EXPECT_CONTAINS("evidence F0033", evidence,
                          "OBJECT.C F0033:147-212",
                          spec->objectAnchor);
    ok &= EXPECT_CONTAINS("evidence F0133", evidence,
                          "BLITMASK.C F0133:30-33",
                          spec->blitMaskAnchor);
    ok &= EXPECT_CONTAINS("evidence DEFS", evidence, "DEFS.H:2088",
                          spec->defsAnchor);
    return ok;
}

static int test_constants(
    const DM1_V1_ChestScrollWheelNonLeaderOccupiedSpecPc34* spec)
{
    int ok = 1;

    ok &= EXPECT_INT("slot count", DM1_PC34_NLO_SLOT_COUNT, 8,
                     spec->defsAnchor);
    ok &= EXPECT_INT("champion count", DM1_PC34_NLO_CHAMPION_COUNT, 4,
                     spec->defsAnchor);
    ok &= EXPECT_INT("C30", DM1_PC34_NLO_C30, 30, spec->defsAnchor);
    ok &= EXPECT_INT("C31", DM1_PC34_NLO_C31, 31, spec->defsAnchor);
    ok &= EXPECT_INT("C33", DM1_PC34_NLO_C33, 33, spec->defsAnchor);
    ok &= EXPECT_INT("C537", DM1_PC34_NLO_C537, 537, spec->defsAnchor);
    ok &= EXPECT_INT("C538", DM1_PC34_NLO_C538, 538, spec->defsAnchor);
    ok &= EXPECT_INT("C540", DM1_PC34_NLO_C540, 540, spec->defsAnchor);
    ok &= EXPECT_INT("C544", DM1_PC34_NLO_C544, 544, spec->defsAnchor);
    ok &= EXPECT_INT("M568", DM1_PC34_NLO_PANEL_M568, 568,
                     spec->commandF0359Anchor);
    ok &= EXPECT_INT("C040", DM1_PC34_NLO_COMMAND_C040, 40,
                     spec->commandF0359Anchor);
    return ok;
}

static int test_initial_state(
    const DM1_V1_ChestScrollWheelNonLeaderOccupiedProbePc34* probe,
    const DM1_V1_ChestScrollWheelNonLeaderOccupiedSpecPc34* spec)
{
    const int chestWant[DM1_PC34_NLO_SLOT_COUNT] = {
        DM1_PC34_NLO_CHEST_C537,
        DM1_PC34_NLO_NONE,
        DM1_PC34_NLO_NONE,
        DM1_PC34_NLO_CHEST_C540_PICKED,
        DM1_PC34_NLO_CHEST_C541,
        DM1_PC34_NLO_CHEST_C542,
        DM1_PC34_NLO_NONE,
        DM1_PC34_NLO_NONE
    };
    const int iconWant[DM1_PC34_NLO_SLOT_COUNT] = {
        DM1_PC34_NLO_CHEST_C537 & 0x00FF,
        DM1_PC34_NLO_NONE,
        DM1_PC34_NLO_NONE,
        DM1_PC34_NLO_CHEST_C540_PICKED & 0x00FF,
        DM1_PC34_NLO_CHEST_C541 & 0x00FF,
        DM1_PC34_NLO_CHEST_C542 & 0x00FF,
        DM1_PC34_NLO_NONE,
        DM1_PC34_NLO_NONE
    };
    int ok = 1;

    ok &= EXPECT_INT("open chest before", probe->openChestBefore,
                     DM1_PC34_NLO_OPEN_CHEST, spec->f0333OpenAnchor);
    ok &= EXPECT_INT("owner champion index", probe->ownerChampionIndex,
                     DM1_PC34_NLO_OWNER_INDEX, spec->f0302DispatchAnchor);
    ok &= EXPECT_INT("owner champion ordinal", probe->ownerChampionOrdinal,
                     DM1_PC34_NLO_OWNER_ORDINAL, spec->defsAnchor);
    ok &= EXPECT_INT("leader index", probe->leaderIndex,
                     DM1_PC34_NLO_LEADER_INDEX, spec->f0302DispatchAnchor);
    ok &= EXPECT_INT("party champion count", probe->partyChampionCount, 4,
                     spec->defsAnchor);
    ok &= EXPECT_INT("non-leader C30 slot 0 hand occupied",
                     probe->initialHandThing,
                     DM1_PC34_NLO_HAND_INITIAL, spec->f0302DispatchAnchor);
    ok &= EXPECT_INT("initial hand icon", probe->initialHandIcon,
                     DM1_PC34_NLO_HAND_INITIAL & 0x00FF, spec->objectAnchor);
    ok &= EXPECT_INT("initial owner load", probe->initialOwnerLoad, 47,
                     spec->f0297PutAnchor);
    ok &= expect_array("initial chest", probe->initialChest, chestWant,
                       DM1_PC34_NLO_SLOT_COUNT, spec->f0333OpenAnchor);
    ok &= expect_array("initial icons", probe->initialIcons, iconWant,
                       DM1_PC34_NLO_SLOT_COUNT, spec->objectAnchor);
    ok &= EXPECT_INT("initial C538 empty", probe->initialChest[1],
                     DM1_PC34_NLO_NONE, spec->defsAnchor);
    ok &= EXPECT_INT("initial C540 occupied", probe->initialChest[3],
                     DM1_PC34_NLO_CHEST_C540_PICKED,
                     spec->f0333OpenAnchor);
    ok &= EXPECT_INT("initial thing count", probe->initialThingCount, 5,
                     spec->defsAnchor);
    return ok;
}

static int test_wheel_up(
    const DM1_V1_ChestScrollWheelNonLeaderOccupiedProbePc34* probe,
    const DM1_V1_ChestScrollWheelNonLeaderOccupiedSpecPc34* spec)
{
    const int traceWant[4] = {
        DM1_PC34_NLO_C537,
        DM1_PC34_NLO_C538,
        DM1_PC34_NLO_C540,
        DM1_PC34_NLO_C540
    };
    const int chestWant[DM1_PC34_NLO_SLOT_COUNT] = {
        DM1_PC34_NLO_CHEST_C537,
        DM1_PC34_NLO_NONE,
        DM1_PC34_NLO_NONE,
        DM1_PC34_NLO_HAND_INITIAL,
        DM1_PC34_NLO_CHEST_C541,
        DM1_PC34_NLO_CHEST_C542,
        DM1_PC34_NLO_NONE,
        DM1_PC34_NLO_NONE
    };
    int ok = 1;

    ok &= EXPECT_INT("wheel-up queued", probe->wheelUpQueuedByF0077, 1,
                     spec->mouseF0077Anchor);
    ok &= EXPECT_INT("wheel-up read", probe->wheelUpReadByF0078, 1,
                     spec->mouseF0078Anchor);
    ok &= EXPECT_INT("wheel-up queue drained",
                     probe->wheelUpQueueDepthAfterRead, 0,
                     spec->mouseF0078Anchor);
    ok &= EXPECT_INT("wheel-up target C540", probe->wheelUpTargetZone,
                     DM1_PC34_NLO_C540, spec->panelF0345Anchor);
    ok &= EXPECT_INT("wheel-up target slot C33",
                     probe->wheelUpTargetSlotIndex, DM1_PC34_NLO_C33,
                     spec->defsAnchor);
    ok &= EXPECT_INT("wheel-up M568", probe->wheelUpCommandM568,
                     DM1_PC34_NLO_PANEL_M568, spec->commandF0359Anchor);
    ok &= EXPECT_INT("wheel-up C040", probe->wheelUpCommandC040,
                     DM1_PC34_NLO_COMMAND_C040, spec->commandF0359Anchor);
    ok &= EXPECT_INT("wheel-up F0344", probe->wheelUpPanelF0344Dispatch, 1,
                     spec->panelF0344Anchor);
    ok &= EXPECT_INT("wheel-up F0345", probe->wheelUpPanelF0345Highlight, 3,
                     spec->panelF0345Anchor);
    ok &= expect_array("wheel-up highlight trace",
                       probe->wheelUpHighlightTrace, traceWant, 4,
                       spec->panelF0345Anchor);
    ok &= EXPECT_INT("wheel-up mask dispatches",
                     probe->wheelUpPartialMaskDispatches, 3,
                     spec->blitMaskAnchor);
    ok &= EXPECT_INT("wheel-up F0302", probe->wheelUpF0302DispatchCount, 1,
                     spec->f0302DispatchAnchor);
    ok &= EXPECT_INT("wheel-up hand snapshot", probe->wheelUpHandSnapshot,
                     DM1_PC34_NLO_HAND_INITIAL,
                     spec->f0302DispatchAnchor);
    ok &= EXPECT_INT("wheel-up slot snapshot", probe->wheelUpSlotSnapshot,
                     DM1_PC34_NLO_CHEST_C540_PICKED,
                     spec->f0302DispatchAnchor);
    ok &= EXPECT_INT("wheel-up F0298 remove hand",
                     probe->wheelUpF0298RemoveHandCount, 1,
                     spec->f0298RemoveAnchor);
    ok &= EXPECT_INT("wheel-up F0300 clear slot",
                     probe->wheelUpF0300ClearSlotCount, 1,
                     spec->f0300ClearAnchor);
    ok &= EXPECT_INT("wheel-up F0297 put picked hand",
                     probe->wheelUpF0297PutHandCount, 1,
                     spec->f0297PutAnchor);
    ok &= EXPECT_INT("wheel-up F0301 prior hand to C540",
                     probe->wheelUpF0301WriteSlotCount, 1,
                     spec->f0301WriteAnchor);
    ok &= EXPECT_INT("hand after wheel-up", probe->handAfterWheelUp,
                     DM1_PC34_NLO_CHEST_C540_PICKED,
                     spec->f0297PutAnchor);
    ok &= EXPECT_INT("hand icon after wheel-up", probe->handIconAfterWheelUp,
                     DM1_PC34_NLO_CHEST_C540_PICKED & 0x00FF,
                     spec->objectAnchor);
    ok &= EXPECT_INT("C540 after wheel-up", probe->c540AfterWheelUp,
                     DM1_PC34_NLO_HAND_INITIAL, spec->f0301WriteAnchor);
    ok &= EXPECT_INT("C538 still empty after wheel-up",
                     probe->c538AfterWheelUp, DM1_PC34_NLO_NONE,
                     spec->f0301WriteAnchor);
    ok &= EXPECT_INT("load after wheel-up", probe->ownerLoadAfterWheelUp, 49,
                     spec->f0297PutAnchor);
    ok &= EXPECT_INT("wheel-up load dirty",
                     probe->ownerAttributesAfterWheelUp &
                         DM1_PC34_NLO_LOAD_MASK,
                     DM1_PC34_NLO_LOAD_MASK, spec->defsAnchor);
    ok &= EXPECT_INT("wheel-up panel dirty",
                     probe->ownerAttributesAfterWheelUp &
                         DM1_PC34_NLO_PANEL_MASK,
                     DM1_PC34_NLO_PANEL_MASK, spec->defsAnchor);
    ok &= expect_array("chest after wheel-up", probe->chestAfterWheelUp,
                       chestWant, DM1_PC34_NLO_SLOT_COUNT,
                       spec->f0301WriteAnchor);
    return ok;
}

static int test_wheel_down(
    const DM1_V1_ChestScrollWheelNonLeaderOccupiedProbePc34* probe,
    const DM1_V1_ChestScrollWheelNonLeaderOccupiedSpecPc34* spec)
{
    const int traceWant[3] = {
        DM1_PC34_NLO_C540,
        DM1_PC34_NLO_C538,
        DM1_PC34_NLO_C538
    };
    const int chestWant[DM1_PC34_NLO_SLOT_COUNT] = {
        DM1_PC34_NLO_CHEST_C537,
        DM1_PC34_NLO_CHEST_C540_PICKED,
        DM1_PC34_NLO_NONE,
        DM1_PC34_NLO_HAND_INITIAL,
        DM1_PC34_NLO_CHEST_C541,
        DM1_PC34_NLO_CHEST_C542,
        DM1_PC34_NLO_NONE,
        DM1_PC34_NLO_NONE
    };
    int ok = 1;

    ok &= EXPECT_INT("wheel-down queued", probe->wheelDownQueuedByF0077, 1,
                     spec->mouseF0077Anchor);
    ok &= EXPECT_INT("wheel-down read", probe->wheelDownReadByF0078, 1,
                     spec->mouseF0078Anchor);
    ok &= EXPECT_INT("wheel-down queue drained",
                     probe->wheelDownQueueDepthAfterRead, 0,
                     spec->mouseF0078Anchor);
    ok &= EXPECT_INT("wheel-down target C538", probe->wheelDownTargetZone,
                     DM1_PC34_NLO_C538, spec->panelF0345Anchor);
    ok &= EXPECT_INT("wheel-down target slot C31",
                     probe->wheelDownTargetSlotIndex, DM1_PC34_NLO_C31,
                     spec->defsAnchor);
    ok &= EXPECT_INT("wheel-down M568", probe->wheelDownCommandM568,
                     DM1_PC34_NLO_PANEL_M568, spec->commandF0359Anchor);
    ok &= EXPECT_INT("wheel-down C040", probe->wheelDownCommandC040,
                     DM1_PC34_NLO_COMMAND_C040, spec->commandF0359Anchor);
    ok &= EXPECT_INT("wheel-down F0344",
                     probe->wheelDownPanelF0344Dispatch, 1,
                     spec->panelF0344Anchor);
    ok &= EXPECT_INT("wheel-down F0345",
                     probe->wheelDownPanelF0345Highlight, 2,
                     spec->panelF0345Anchor);
    ok &= expect_array("wheel-down highlight trace",
                       probe->wheelDownHighlightTrace, traceWant, 3,
                       spec->panelF0345Anchor);
    ok &= EXPECT_INT("wheel-down mask dispatches",
                     probe->wheelDownPartialMaskDispatches, 2,
                     spec->blitMaskAnchor);
    ok &= EXPECT_INT("wheel-down F0302", probe->wheelDownF0302DispatchCount,
                     1, spec->f0302DispatchAnchor);
    ok &= EXPECT_INT("wheel-down hand snapshot",
                     probe->wheelDownHandSnapshot,
                     DM1_PC34_NLO_CHEST_C540_PICKED,
                     spec->f0302DispatchAnchor);
    ok &= EXPECT_INT("wheel-down slot snapshot",
                     probe->wheelDownSlotSnapshot, DM1_PC34_NLO_NONE,
                     spec->f0302DispatchAnchor);
    ok &= EXPECT_INT("wheel-down F0298 remove hand",
                     probe->wheelDownF0298RemoveHandCount, 1,
                     spec->f0298RemoveAnchor);
    ok &= EXPECT_INT("wheel-down F0300 unused",
                     probe->wheelDownF0300ClearSlotCount, 0,
                     spec->f0300ClearAnchor);
    ok &= EXPECT_INT("wheel-down F0297 unused",
                     probe->wheelDownF0297PutHandCount, 0,
                     spec->f0297PutAnchor);
    ok &= EXPECT_INT("wheel-down F0301 hand to C538",
                     probe->wheelDownF0301WriteSlotCount, 1,
                     spec->f0301WriteAnchor);
    ok &= EXPECT_INT("hand after wheel-down", probe->handAfterWheelDown,
                     DM1_PC34_NLO_NONE, spec->f0298RemoveAnchor);
    ok &= EXPECT_INT("C538 after wheel-down", probe->c538AfterWheelDown,
                     DM1_PC34_NLO_CHEST_C540_PICKED,
                     spec->f0301WriteAnchor);
    ok &= EXPECT_INT("C540 after wheel-down", probe->c540AfterWheelDown,
                     DM1_PC34_NLO_HAND_INITIAL, spec->f0301WriteAnchor);
    ok &= EXPECT_INT("load after wheel-down", probe->ownerLoadAfterWheelDown,
                     42, spec->f0298RemoveAnchor);
    ok &= EXPECT_INT("wheel-down viewport dirty",
                     probe->ownerAttributesAfterWheelDown &
                         DM1_PC34_NLO_VIEWPORT_MASK,
                     DM1_PC34_NLO_VIEWPORT_MASK, spec->defsAnchor);
    ok &= expect_array("chest after wheel-down", probe->chestAfterWheelDown,
                       chestWant, DM1_PC34_NLO_SLOT_COUNT,
                       spec->f0301WriteAnchor);
    return ok;
}

static int test_close_and_negative(
    const DM1_V1_ChestScrollWheelNonLeaderOccupiedProbePc34* probe,
    const DM1_V1_ChestScrollWheelNonLeaderOccupiedSpecPc34* spec)
{
    const int closedWant[DM1_PC34_NLO_SLOT_COUNT] = {
        DM1_PC34_NLO_CHEST_C537,
        DM1_PC34_NLO_CHEST_C540_PICKED,
        DM1_PC34_NLO_HAND_INITIAL,
        DM1_PC34_NLO_CHEST_C541,
        DM1_PC34_NLO_CHEST_C542,
        DM1_PC34_NLO_NONE,
        DM1_PC34_NLO_NONE,
        DM1_PC34_NLO_NONE
    };
    int ok = 1;

    ok &= EXPECT_INT("close count", probe->closeCount, 5,
                     spec->f0334CloseAnchor);
    ok &= EXPECT_INT("close skipped empty entries",
                     probe->closeSkippedEmptyEntries, 3,
                     spec->f0334CloseAnchor);
    ok &= EXPECT_INT("close head", probe->closeHead,
                     DM1_PC34_NLO_CHEST_C537, spec->f0334CloseAnchor);
    ok &= EXPECT_INT("close end sentinel", probe->closeEndSentinel,
                     DM1_PC34_NLO_END, spec->f0334CloseAnchor);
    ok &= expect_array("closed slots", probe->closedSlots, closedWant,
                       DM1_PC34_NLO_SLOT_COUNT, spec->f0334CloseAnchor);
    ok &= EXPECT_INT("open chest after close", probe->openChestAfterClose,
                     DM1_PC34_NLO_NONE, spec->f0334CloseAnchor);
    ok &= EXPECT_INT("G0425 cleared", probe->g0425ClearedAfterClose, 1,
                     spec->f0334CloseAnchor);
    ok &= EXPECT_INT("thing count preserved", probe->finalThingCount,
                     probe->initialThingCount, spec->defsAnchor);
    ok &= EXPECT_INT("screen update balanced",
                     probe->screenUpdateBalanced, 1,
                     spec->mouseF0078Anchor);
    ok &= EXPECT_INT("negative no-hand wheel-down rejected",
                     probe->negativeWheelDownRejected, 1,
                     spec->f0302DispatchAnchor);
    ok &= EXPECT_INT("negative hand stays empty", probe->negativeHandAfter,
                     DM1_PC34_NLO_NONE, spec->f0302DispatchAnchor);
    ok &= EXPECT_INT("negative C538 stays empty", probe->negativeC538After,
                     DM1_PC34_NLO_NONE, spec->f0302DispatchAnchor);
    return ok;
}

int main(void)
{
    DM1_V1_ChestScrollWheelNonLeaderOccupiedProbePc34 probe;
    const DM1_V1_ChestScrollWheelNonLeaderOccupiedSpecPc34* spec =
        dm1_v1_chest_scroll_wheel_non_leader_occupied_slot_spec_pc34();
    int ok = 1;

    ok &= EXPECT_INT("probe run",
                     dm1_v1_chest_scroll_wheel_non_leader_occupied_slot_pc34(
                         &probe),
                     1, spec->f0333OpenAnchor);
    ok &= test_source_evidence(spec);
    ok &= test_constants(spec);
    ok &= test_initial_state(&probe, spec);
    ok &= test_wheel_up(&probe, spec);
    ok &= test_wheel_down(&probe, spec);
    ok &= test_close_and_negative(&probe, spec);
    ok &= EXPECT_INT("assertion floor", g_assertions >= 100, 1,
                     spec->defsAnchor);

    printf("SUMMARY assertions=%d failures=%d\n", g_assertions, g_failures);
    return (ok && g_failures == 0 && g_assertions >= 100) ? 0 : 1;
}
