#include "dm1_v1_chest_open_stack_split_press_eye_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/*
 * Slice chosen: press-eye F0333 auto-close gate (P0694_B_PressingEye =
 * TRUE) plus the C09 action-hand (DM1_PC34_SLOT_ACTION_HAND) click
 * pickup of one stack member mid-close. The contract pins (1) the
 * press-eye C145-blit skip, (2) the C09 pickup of a single stack
 * member (the closed list is one shorter than the opened list), (3)
 * the remaining stack member is linked in close-order, (4) the
 * action hand holds the picked item after close, (5) the C0xFFFE
 * end-of-list terminator placement, and (6) the reopen preserves
 * the stack pattern (one stack member visible, the other absent).
 *
 * Non-duplicative: pass797 covers the press-eye + C09 PUT-replacement
 * scenario (replacement container dropped into a not-yet-visited
 * slot mid-close). pass799 covers the press-eye + C09 PICKUP-from
 * -stack scenario (one stack member picked up mid-close, the
 * remaining stack member is linked in close-order; the closed list
 * is one shorter, and the action hand holds the picked item).
 *
 * This test pins the stack-split path: when one stack member is
 * picked up by C09 mid-close, the closed list is one shorter, the
 * stack pattern is preserved (one member visible, the other absent
 * from the closed list), and the action hand holds the picked
 * stack member after close.
 */

static DM1_V1_ChestOpenStackSplitPressEyeProbePc34 g_probe;
static int g_assertions;
static int g_failures;

static int expect_int(const char* label,
                      int got,
                      int want,
                      const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        ++g_failures;
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n",
               label, got, want, redmcsbAnchor);
        return 0;
    }
    printf("PASS %s=%d anchor=%s\n", label, got, redmcsbAnchor);
    return 1;
}

static int expect_contains(const char* label,
                           const char* haystack,
                           const char* needle,
                           const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        ++g_failures;
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (!haystack || !needle || !strstr(haystack, needle)) {
        ++g_failures;
        printf("FAIL %s missing=%s anchor=%s\n",
               label, needle ? needle : "(null)", redmcsbAnchor);
        return 0;
    }
    printf("PASS %s contains=%s anchor=%s\n",
           label, needle, redmcsbAnchor);
    return 1;
}

static int test_spec_and_evidence(void)
{
    const DM1_V1_ChestOpenStackSplitPressEyeSpecPc34* spec =
        dm1_v1_chest_open_stack_split_press_eye_spec_pc34();
    const char* evidence =
        dm1_v1_chest_open_stack_split_press_eye_source_evidence_pc34();
    const char* f0333_press =
        "ReDMCSB CHEST.C F0333 P0694_B_PressingEye lines 32-42 + 44";
    const char* f0333_copy =
        "ReDMCSB CHEST.C F0333 lines 53-67 + 70-74";
    const char* f0334_begin =
        "ReDMCSB CHEST.C F0334 lines 113-117";
    const char* f0334_loop =
        "ReDMCSB CHEST.C F0334 lines 118-132";
    const char* f0297 = "ReDMCSB CHAMPION.C F0297/F0298 lines 243-298";
    const char* f0300 =
        "ReDMCSB CHAMPION.C F0300/F0301/F0302 lines 511-515,606-614,688-710";
    const char* panel = "ReDMCSB PANEL.C F0347:1639-1691";
    const char* object = "ReDMCSB OBJECT.C F0033 lines 147-212";
    const char* blitmask = "ReDMCSB BLITMASK.C F0133 lines 30-33";
    int ok = 1;

    ok &= expect_int("spec contract only", spec->contractOnly, 1, f0333_copy);
    ok &= expect_int("spec slot count", spec->chestSlotCount,
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_SLOT_COUNT,
                     f0333_copy);
    ok &= expect_int("spec prefix count", spec->prefixCount,
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_PREFIX_COUNT,
                     f0334_loop);
    ok &= expect_int("spec stack index", spec->stackIndex,
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_STACK_INDEX,
                     f0333_copy);
    ok &= expect_int("spec stack pc34 slot", spec->stackPc34Slot,
                     DM1_PC34_SLOT_CHEST_1 +
                         DM1_PC34_CHEST_OPEN_STACK_SPLIT_STACK_INDEX,
                     f0300);
    ok &= expect_int("spec pickup index", spec->pickupIndex,
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_PICKUP_INDEX,
                     f0333_copy);
    ok &= expect_int("spec pickup pc34 slot", spec->pickupPc34Slot,
                     DM1_PC34_SLOT_CHEST_1 +
                         DM1_PC34_CHEST_OPEN_STACK_SPLIT_PICKUP_INDEX,
                     f0300);
    ok &= expect_int("spec chest thing", spec->chestThing,
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_CHEST_THING,
                     f0333_press);
    ok &= expect_int("spec first item", spec->firstItemType,
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_FIRST_ITEM,
                     f0333_copy);
    ok &= expect_int("spec stack A", spec->stackAItemType,
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_STACK_A, f0333_copy);
    ok &= expect_int("spec stack B", spec->stackBItemType,
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_STACK_B, f0333_copy);
    ok &= expect_int("spec press eye path", spec->pressEyePath, 1,
                     f0333_press);
    ok &= expect_int("spec end of list sentinel", spec->endOfListSentinel,
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_ENDOFLIST, f0334_begin);
    ok &= expect_int("spec none sentinel", spec->noneSentinel,
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_NONE, f0333_copy);
    ok &= expect_contains("spec marker contract", spec->contractMarker,
                          "contract gate only", f0333_copy);
    ok &= expect_contains("spec marker no pixel", spec->contractMarker,
                          "no real-asset pixel", blitmask);
    ok &= expect_contains("spec why non-duplicate", spec->chosenBecause,
                          "press-eye", f0333_press);
    ok &= expect_contains("spec why pickup", spec->chosenBecause,
                          "PICKUP-from-stack", panel);
    ok &= expect_contains("evidence F0333 press", evidence,
                          "P0694_B_PressingEye", f0333_press);
    ok &= expect_contains("evidence F0333 C145 skip", evidence,
                          "C145_ICON_CONTAINER_CHEST_OPEN", f0333_press);
    ok &= expect_contains("evidence F0333 copy", evidence,
                          "F0333:53-67", f0333_copy);
    ok &= expect_contains("evidence F0333 fill", evidence,
                          "F0333:70-74", f0333_copy);
    ok &= expect_contains("evidence F0334 begin", evidence,
                          "F0334:113-117", f0334_begin);
    ok &= expect_contains("evidence F0334 ENDOFLIST", evidence,
                          "C0xFFFE_THING_ENDOFLIST", f0334_begin);
    ok &= expect_contains("evidence F0334 loop", evidence,
                          "F0334:118-132", f0334_loop);
    ok &= expect_contains("evidence F0297", evidence,
                          "F0297:243-268", f0297);
    ok &= expect_contains("evidence F0298", evidence,
                          "F0298:270-298", f0297);
    ok &= expect_contains("evidence F0300", evidence,
                          "F0300:511-515", f0300);
    ok &= expect_contains("evidence F0301", evidence,
                          "F0301:606-614", f0300);
    ok &= expect_contains("evidence F0302", evidence,
                          "F0302:688-710", f0300);
    ok &= expect_contains("evidence F0302 pickup", evidence,
                          "pickup path", f0300);
    ok &= expect_contains("evidence PANEL C09", evidence,
                          "PANEL.C F0347:1639-1691", panel);
    ok &= expect_contains("evidence PANEL action hand", evidence,
                          "C09_SLOT_BOX_INVENTORY_ACTION_HAND", panel);
    ok &= expect_contains("evidence OBJECT", evidence,
                          "OBJECT.C F0033:147-212", object);
    ok &= expect_contains("evidence BLITMASK", evidence,
                          "BLITMASK.C F0133:30-33", blitmask);
    return ok;
}

static int test_press_eye_open_and_close_prefix(void)
{
    const char* f0333_press =
        "ReDMCSB CHEST.C F0333 P0694_B_PressingEye lines 32-42 + 44";
    const char* f0333_copy =
        "ReDMCSB CHEST.C F0333 lines 53-67 + 70-74";
    const char* f0334_begin =
        "ReDMCSB CHEST.C F0334 lines 113-117";
    const char* f0334_loop =
        "ReDMCSB CHEST.C F0334 lines 118-132";
    int ok = 1;
    int i;

    ok &= expect_int("probe contract only", g_probe.contractOnly, 1,
                     f0333_copy);
    ok &= expect_int("press eye path enabled",
                     g_probe.pressEyePathEnabled, 1, f0333_press);
    ok &= expect_int("press eye C145 blit skipped",
                     g_probe.pressEyeIconBlitSkipped, 1, f0333_press);
    ok &= expect_int("open result", g_probe.openResult, 1, f0333_copy);
    ok &= expect_int("open thing", g_probe.openThing,
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_CHEST_THING,
                     f0333_copy);
    ok &= expect_int("open visible count", g_probe.openVisibleCount,
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_SLOT_COUNT,
                     f0333_copy);
    ok &= expect_int("open order matches input",
                     g_probe.openedOrderMatchesInput, 1, f0333_copy);
    ok &= expect_int("stack pair adjacent in G0425",
                     g_probe.stackPairAdjacent, 1, f0333_copy);
    for (i = 0; i < DM1_PC34_CHEST_OPEN_STACK_SPLIT_SLOT_COUNT; ++i) {
        char label[80];
        int expected = DM1_PC34_CHEST_OPEN_STACK_SPLIT_FIRST_ITEM + i;
        int expectedWeight = 4 + i;

        if (i == DM1_PC34_CHEST_OPEN_STACK_SPLIT_PICKUP_INDEX) {
            expected = DM1_PC34_CHEST_OPEN_STACK_SPLIT_STACK_A;
            expectedWeight = 11;
        } else if (i == DM1_PC34_CHEST_OPEN_STACK_SPLIT_STACK_INDEX) {
            expected = DM1_PC34_CHEST_OPEN_STACK_SPLIT_STACK_B;
            expectedWeight = 12;
        }
        snprintf(label, sizeof(label), "open C%d type", 537 + i);
        ok &= expect_int(label, g_probe.openedTypes[i], expected, f0333_copy);
        snprintf(label, sizeof(label), "open C%d weight", 537 + i);
        ok &= expect_int(label, g_probe.openedWeights[i], expectedWeight,
                         f0333_copy);
    }
    ok &= expect_int("action hand C09 before close type",
                     g_probe.actionHandBeforeCloseType, 0, f0333_press);
    ok &= expect_int("action hand C09 before close empty",
                     g_probe.actionHandBeforeCloseEmpty, 1, f0333_press);
    ok &= expect_int("pickup allowed by empty action hand",
                     g_probe.pickupAllowedByActionHand, 1, f0333_press);

    ok &= expect_int("close begin result", g_probe.closeBeginResult, 1,
                     f0334_begin);
    ok &= expect_int("open thing before close begin",
                     g_probe.openThingBeforeCloseBegin,
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_CHEST_THING,
                     f0334_begin);
    ok &= expect_int("open thing after close begin",
                     g_probe.openThingAfterCloseBegin, 0, f0334_begin);
    ok &= expect_int("container slot cleared to end",
                     g_probe.containerSlotClearedToEnd, 1, f0334_begin);
    ok &= expect_int("container slot end-of-list terminator",
                     g_probe.containerSlotEndOfListTerminator,
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_ENDOFLIST,
                     f0334_begin);
    ok &= expect_int("prefix processed count",
                     g_probe.prefixProcessedCount,
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_PREFIX_COUNT,
                     f0334_loop);
    ok &= expect_int("prefix closed count", g_probe.prefixClosedCount,
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_PREFIX_COUNT,
                     f0334_loop);
    for (i = 0; i < DM1_PC34_CHEST_OPEN_STACK_SPLIT_PREFIX_COUNT; ++i) {
        char label[80];

        snprintf(label, sizeof(label), "prefix linked C%d", 537 + i);
        ok &= expect_int(label, g_probe.prefixClosedTypes[i],
                         DM1_PC34_CHEST_OPEN_STACK_SPLIT_FIRST_ITEM + i,
                         f0334_loop);
        snprintf(label, sizeof(label), "prefix cleared C%d", 537 + i);
        ok &= expect_int(label, g_probe.prefixSlotsCleared[i], 1,
                         f0334_loop);
    }
    ok &= expect_int("prefix last linked type",
                     g_probe.prefixLastLinkedType,
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_FIRST_ITEM +
                         DM1_PC34_CHEST_OPEN_STACK_SPLIT_PREFIX_COUNT - 1,
                     f0334_loop);
    ok &= expect_int("prefix not yet terminated",
                     g_probe.prefixEndOfListTerminator, 0, f0334_loop);
    return ok;
}

static int test_action_hand_pickup_mid_close(void)
{
    const char* f0334_loop =
        "ReDMCSB CHEST.C F0334 lines 118-132";
    const char* f0297 = "ReDMCSB CHAMPION.C F0297/F0298 lines 243-298";
    const char* f0300 =
        "ReDMCSB CHAMPION.C F0300/F0301/F0302 lines 511-515,606-614,688-710";
    const char* panel = "ReDMCSB PANEL.C F0347:1639-1691";
    int ok = 1;

    ok &= expect_int("mid pickup result", g_probe.midPickupResult, 1, f0300);
    ok &= expect_int("mid pickup source pc34 slot C09",
                     g_probe.midPickupPc34Slot,
                     DM1_PC34_SLOT_ACTION_HAND, panel);
    ok &= expect_int("mid pickup source pc34 slot chest",
                     g_probe.midPickupSourcePc34Slot,
                     DM1_PC34_SLOT_CHEST_1 +
                         DM1_PC34_CHEST_OPEN_STACK_SPLIT_PICKUP_INDEX,
                     f0300);
    ok &= expect_int("mid pickup leader before type",
                     g_probe.midPickupLeaderBeforeType, 0, panel);
    ok &= expect_int("mid pickup slot before type",
                     g_probe.midPickupSlotBeforeType,
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_STACK_A, f0334_loop);
    ok &= expect_int("mid pickup does not remove C09",
                     g_probe.midPickupLeaderRemovedCall, 0, f0300);
    ok &= expect_int("mid pickup removes stack member slot",
                     g_probe.midPickupSlotRemovedCall, 1, f0300);
    ok &= expect_int("mid pickup puts slot into C09",
                     g_probe.midPickupPutSlotInLeaderCall, 1, f0297);
    ok &= expect_int("mid pickup no put-back into slot",
                     g_probe.midPickupPutLeaderInSlotCall, 0, f0300);
    ok &= expect_int("mid pickup C09 after holds stack A",
                     g_probe.midPickupLeaderAfterType,
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_STACK_A, f0297);
    ok &= expect_int("mid pickup slot after empty",
                     g_probe.midPickupSlotAfterType, 0, f0300);
    ok &= expect_int("mid pickup C09 not empty after",
                     g_probe.midPickupLeaderEmptyAfterPickup, 0, f0300);
    ok &= expect_int("pickup source absent from slot after pickup",
                     g_probe.pickupSourceAbsentFromSlotAfterPickup, 1,
                     f0300);
    return ok;
}

static int test_close_rest_and_reopen(void)
{
    const char* f0333_copy =
        "ReDMCSB CHEST.C F0333 lines 53-67 + 70-74";
    const char* f0334_begin =
        "ReDMCSB CHEST.C F0334 lines 113-117";
    const char* f0334_loop =
        "ReDMCSB CHEST.C F0334 lines 118-132";
    const char* f0297 = "ReDMCSB CHAMPION.C F0297/F0298 lines 243-298";
    int ok = 1;

    /*
     * The closed list is one shorter than the opened list (7 items
     * instead of 8) because slot 3 (STACK_A) was picked up by C09.
     * The expected closed list, in close-order, is:
     *   [0] = FIRST_ITEM + 0
     *   [1] = FIRST_ITEM + 1
     *   [2] = FIRST_ITEM + 2
     *   [3] = STACK_B  (the surviving stack member, linked in
     *                   close-order at its original position 4)
     *   [4] = FIRST_ITEM + 5
     *   [5] = FIRST_ITEM + 6
     *   [6] = FIRST_ITEM + 7
     */
    ok &= expect_int("close count", g_probe.closeCount,
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_SLOT_COUNT - 1,
                     f0334_loop);
    ok &= expect_int("closed[0] = FIRST_ITEM+0",
                     g_probe.closedTypes[0],
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_FIRST_ITEM,
                     f0334_loop);
    ok &= expect_int("closed[1] = FIRST_ITEM+1",
                     g_probe.closedTypes[1],
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_FIRST_ITEM + 1,
                     f0334_loop);
    ok &= expect_int("closed[2] = FIRST_ITEM+2",
                     g_probe.closedTypes[2],
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_FIRST_ITEM + 2,
                     f0334_loop);
    ok &= expect_int("closed[3] = STACK_B",
                     g_probe.closedTypes[3],
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_STACK_B, f0334_loop);
    ok &= expect_int("closed[4] = FIRST_ITEM+5",
                     g_probe.closedTypes[4],
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_FIRST_ITEM + 5,
                     f0334_loop);
    ok &= expect_int("closed[5] = FIRST_ITEM+6",
                     g_probe.closedTypes[5],
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_FIRST_ITEM + 6,
                     f0334_loop);
    ok &= expect_int("closed[6] = FIRST_ITEM+7",
                     g_probe.closedTypes[6],
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_FIRST_ITEM + 7,
                     f0334_loop);
    ok &= expect_int("closed[3] weight = 12 (STACK_B)",
                     g_probe.closedWeights[3], 12, f0334_loop);
    ok &= expect_int("stack B linked at stack position",
                     g_probe.stackBLinkedAtStackPosition, 1, f0334_loop);
    ok &= expect_int("stack A absent from closed links",
                     g_probe.stackAAbsentFromClosedLinks, 1, f0334_loop);
    ok &= expect_int("pickup absent from closed links",
                     g_probe.pickupAbsentFromClosedLinks, 1, f0334_loop);
    ok &= expect_int("closed order preserves stack pattern",
                     g_probe.closedOrderPreservesStackPattern, 1,
                     f0334_loop);
    ok &= expect_int("pickup slot cleared by close",
                     g_probe.pickupSlotClearedByClose, 1, f0334_loop);
    ok &= expect_int("stack slot cleared by close",
                     g_probe.stackSlotClearedByClose, 1, f0334_loop);
    ok &= expect_int("all slots empty after close",
                     g_probe.allSlotsEmptyAfterClose, 1, f0334_loop);
    ok &= expect_int("action hand C09 after close type",
                     g_probe.actionHandAfterCloseType,
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_STACK_A, f0297);
    ok &= expect_int("action hand C09 after close weight",
                     g_probe.actionHandAfterCloseWeight, 11, f0297);
    ok &= expect_int("action hand C09 after close not empty",
                     g_probe.actionHandAfterCloseEmpty, 0, f0297);
    ok &= expect_int("closed end-of-list terminator",
                     g_probe.closedEndOfListTerminator,
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_ENDOFLIST,
                     f0334_begin);

    ok &= expect_int("reopen result", g_probe.reopenResult, 1, f0333_copy);
    ok &= expect_int("reopen thing", g_probe.reopenThing,
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_REOPEN_THING,
                     f0333_copy);
    ok &= expect_int("reopened visible count",
                     g_probe.reopenedVisibleCount,
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_SLOT_COUNT - 1,
                     f0333_copy);
    ok &= expect_int("reopened[0] = FIRST_ITEM+0",
                     g_probe.reopenedTypes[0],
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_FIRST_ITEM,
                     f0333_copy);
    ok &= expect_int("reopened[3] = STACK_B",
                     g_probe.reopenedTypes[3],
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_STACK_B, f0333_copy);
    ok &= expect_int("reopened[6] = FIRST_ITEM+7",
                     g_probe.reopenedTypes[6],
                     DM1_PC34_CHEST_OPEN_STACK_SPLIT_FIRST_ITEM + 7,
                     f0333_copy);
    ok &= expect_int("reopened order matches closed",
                     g_probe.reopenedOrderMatchesClosed, 1, f0333_copy);
    ok &= expect_int("stack B visible after reopen",
                     g_probe.stackBVisibleAfterReopen, 1, f0333_copy);
    ok &= expect_int("stack A absent after reopen",
                     g_probe.stackAAbsentAfterReopen, 1, f0333_copy);
    ok &= expect_int("pickup absent after reopen",
                     g_probe.pickupAbsentAfterReopen, 1, f0333_copy);
    return ok;
}

int main(void)
{
    const char* f0333_copy =
        "ReDMCSB CHEST.C F0333 lines 53-67 + 70-74";
    int ok = 1;

    printf("probe=dm1_v1_chest_open_stack_split_press_eye_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_chest_open_stack_split_press_eye_source_evidence_pc34());

    ok &= expect_int("probe run",
                     dm1_v1_chest_open_stack_split_press_eye_run_pc34(
                         &g_probe),
                     1, f0333_copy);
    ok &= test_spec_and_evidence();
    ok &= test_press_eye_open_and_close_prefix();
    ok &= test_action_hand_pickup_mid_close();
    ok &= test_close_rest_and_reopen();
    ok &= expect_int("minimum assertion count",
                     g_assertions >= 60 ? 1 : 0, 1, f0333_copy);

    printf("assertionCount=%d\n", g_assertions);
    printf("chestOpenStackSplitPressEyeResultOk=%d closeCount=%d "
           "stackBLinked=%d stackAAbsent=%d actionHandAfterClose=%d "
           "endOfListTerminator=0x%X reopenCount=%d\n",
           ok && g_failures == 0 ? 1 : 0,
           g_probe.closeCount,
           g_probe.stackBLinkedAtStackPosition,
           g_probe.stackAAbsentFromClosedLinks,
           g_probe.actionHandAfterCloseType,
           g_probe.closedEndOfListTerminator,
           g_probe.reopenedVisibleCount);

    if (!ok || g_failures != 0) {
        printf("FAIL dm1_v1_chest_open_stack_split_press_eye_pc34_compat "
               "assertions=%d failures=%d\n",
               g_assertions, g_failures);
        return 1;
    }
    printf("PASS dm1_v1_chest_open_stack_split_press_eye_pc34_compat "
           "assertions=%d\n",
           g_assertions);
    return 0;
}
