#include "dm1_v1_auto_chest_action_hand_swap_during_close_gate_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/*
 * Slice chosen: press-eye F0333 auto-close gate (P0694_B_PressingEye =
 * TRUE) plus the C09 action-hand (DM1_PC34_SLOT_ACTION_HAND) being used
 * as the swap source for a not-yet-visited G0425 slot. The contract pins
 * (1) the press-eye C145-blit skip, (2) the C09->future-slot put, (3)
 * the F0334 already-visited slot late-write no-op for the close-link
 * list, (4) the C0xFFFE_THING_ENDOFLIST terminator, and (5) the
 * press-eye replay contract.
 *
 * Non-duplicative: existing
 * tests/test_dm1_v1_chest_mid_close_hand_swap_pc34_compat.c covers the
 * manual F0333 open + ready-hand slot-box swap. This test isolates the
 * press-eye auto-close gate, the action-hand slot-box swap, the
 * F0334 already-visited-slot late-write, and the end-of-list
 * terminator.
 */

static DM1_V1_AutoChestActionHandSwapDuringCloseGateProbePc34 g_probe;
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
    const DM1_V1_AutoChestActionHandSwapDuringCloseGateSpecPc34* spec =
        dm1_v1_auto_chest_action_hand_swap_during_close_gate_spec_pc34();
    const char* evidence =
        dm1_v1_auto_chest_action_hand_swap_during_close_gate_source_evidence_pc34();
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
                     DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT,
                     f0333_copy);
    ok &= expect_int("spec prefix count", spec->prefixCount,
                     DM1_PC34_AUTO_CHEST_ACTION_HAND_PREFIX_COUNT,
                     f0334_loop);
    ok &= expect_int("spec future index", spec->futureIndex,
                     DM1_PC34_AUTO_CHEST_ACTION_HAND_FUTURE_INDEX,
                     f0334_loop);
    ok &= expect_int("spec future pc34 slot", spec->futurePc34Slot,
                     DM1_PC34_SLOT_CHEST_1 +
                         DM1_PC34_AUTO_CHEST_ACTION_HAND_FUTURE_INDEX,
                     f0300);
    ok &= expect_int("spec already processed index",
                     spec->alreadyProcessedIndex,
                     DM1_PC34_AUTO_CHEST_ACTION_HAND_ALREADY_PROCESSED_INDEX,
                     f0334_loop);
    ok &= expect_int("spec already processed pc34",
                     spec->alreadyProcessedPc34Slot,
                     DM1_PC34_SLOT_CHEST_1 +
                         DM1_PC34_AUTO_CHEST_ACTION_HAND_ALREADY_PROCESSED_INDEX,
                     f0300);
    ok &= expect_int("spec chest thing", spec->chestThing,
                     DM1_PC34_AUTO_CHEST_ACTION_HAND_CHEST_THING,
                     f0333_press);
    ok &= expect_int("spec first item", spec->firstItemType,
                     DM1_PC34_AUTO_CHEST_ACTION_HAND_FIRST_ITEM, f0333_copy);
    ok &= expect_int("spec future original",
                     spec->futureOriginalItemType,
                     DM1_PC34_AUTO_CHEST_ACTION_HAND_FIRST_ITEM +
                         DM1_PC34_AUTO_CHEST_ACTION_HAND_FUTURE_INDEX,
                     f0333_copy);
    ok &= expect_int("spec replacement container",
                     spec->replacementContainerType,
                     DM1_PC34_AUTO_CHEST_ACTION_HAND_REPLACEMENT_CONTAINER,
                     f0297);
    ok &= expect_int("spec press eye path", spec->pressEyePath, 1, f0333_press);
    ok &= expect_int("spec end of list sentinel", spec->endOfListSentinel,
                     DM1_PC34_AUTO_CHEST_ACTION_HAND_ENDOFLIST, f0334_begin);
    ok &= expect_int("spec none sentinel", spec->noneSentinel,
                     DM1_PC34_AUTO_CHEST_ACTION_HAND_NONE, f0333_copy);
    ok &= expect_contains("spec marker contract", spec->contractMarker,
                          "contract gate only", f0333_copy);
    ok &= expect_contains("spec marker no pixel", spec->contractMarker,
                          "no real-asset pixel", blitmask);
    ok &= expect_contains("spec why non-duplicate", spec->chosenBecause,
                          "press-eye", f0333_press);
    ok &= expect_contains("spec why action hand", spec->chosenBecause,
                          "C09 action hand", panel);
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
    const char* f0297 = "ReDMCSB CHAMPION.C F0297/F0298 lines 243-298";
    const char* panel = "ReDMCSB PANEL.C F0347:1639-1691";
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
                     DM1_PC34_AUTO_CHEST_ACTION_HAND_CHEST_THING,
                     f0333_copy);
    ok &= expect_int("open visible count", g_probe.openVisibleCount,
                     DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT,
                     f0333_copy);
    ok &= expect_int("open order matches input",
                     g_probe.openedOrderMatchesInput, 1, f0333_copy);
    for (i = 0; i < DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT; ++i) {
        char label[80];

        snprintf(label, sizeof(label), "open C%d type", 537 + i);
        ok &= expect_int(label, g_probe.openedTypes[i],
                         DM1_PC34_AUTO_CHEST_ACTION_HAND_FIRST_ITEM + i,
                         f0333_copy);
        snprintf(label, sizeof(label), "open C%d weight", 537 + i);
        ok &= expect_int(label, g_probe.openedWeights[i], 4 + i, f0297);
    }
    ok &= expect_int("action hand C09 before close type",
                     g_probe.actionHandBeforeCloseType,
                     DM1_PC34_AUTO_CHEST_ACTION_HAND_REPLACEMENT_CONTAINER,
                     panel);
    ok &= expect_int("action hand C09 before close weight",
                     g_probe.actionHandBeforeCloseWeight, 29, f0297);
    ok &= expect_int("action hand allowed container",
                     g_probe.actionHandBeforeCloseAllowedSlots,
                     DM1_PC34_ALLOWED_CONTAINER, f0297);
    ok &= expect_int("replacement can enter chest",
                     g_probe.replacementCanEnterChest, 1, f0297);

    ok &= expect_int("close begin result", g_probe.closeBeginResult, 1,
                     f0334_begin);
    ok &= expect_int("open thing before close begin",
                     g_probe.openThingBeforeCloseBegin,
                     DM1_PC34_AUTO_CHEST_ACTION_HAND_CHEST_THING,
                     f0334_begin);
    ok &= expect_int("open thing after close begin",
                     g_probe.openThingAfterCloseBegin, 0, f0334_begin);
    ok &= expect_int("container slot cleared to end",
                     g_probe.containerSlotClearedToEnd, 1, f0334_begin);
    ok &= expect_int("container slot end-of-list terminator",
                     g_probe.containerSlotEndOfListTerminator,
                     DM1_PC34_AUTO_CHEST_ACTION_HAND_ENDOFLIST,
                     f0334_begin);
    ok &= expect_int("prefix processed count",
                     g_probe.prefixProcessedCount,
                     DM1_PC34_AUTO_CHEST_ACTION_HAND_PREFIX_COUNT,
                     f0334_loop);
    ok &= expect_int("prefix closed count", g_probe.prefixClosedCount,
                     DM1_PC34_AUTO_CHEST_ACTION_HAND_PREFIX_COUNT,
                     f0334_loop);
    for (i = 0; i < DM1_PC34_AUTO_CHEST_ACTION_HAND_PREFIX_COUNT; ++i) {
        char label[80];

        snprintf(label, sizeof(label), "prefix linked C%d", 537 + i);
        ok &= expect_int(label, g_probe.prefixClosedTypes[i],
                         DM1_PC34_AUTO_CHEST_ACTION_HAND_FIRST_ITEM + i,
                         f0334_loop);
        snprintf(label, sizeof(label), "prefix cleared C%d", 537 + i);
        ok &= expect_int(label, g_probe.prefixSlotsCleared[i], 1, f0334_loop);
    }
    ok &= expect_int("prefix last linked type",
                     g_probe.prefixLastLinkedType,
                     DM1_PC34_AUTO_CHEST_ACTION_HAND_FIRST_ITEM +
                         DM1_PC34_AUTO_CHEST_ACTION_HAND_PREFIX_COUNT - 1,
                     f0334_loop);
    ok &= expect_int("prefix not yet terminated",
                     g_probe.prefixEndOfListTerminator, 0, f0334_loop);
    return ok;
}

static int test_action_hand_mid_close_swap(void)
{
    const char* f0334_loop =
        "ReDMCSB CHEST.C F0334 lines 118-132";
    const char* f0297 = "ReDMCSB CHAMPION.C F0297/F0298 lines 243-298";
    const char* f0302 =
        "ReDMCSB CHAMPION.C F0300/F0301/F0302 lines 511-515,606-614,688-710";
    const char* panel = "ReDMCSB PANEL.C F0347:1639-1691";
    int ok = 1;

    ok &= expect_int("mid swap result", g_probe.midSwapResult, 1, f0302);
    ok &= expect_int("mid swap source pc34 slot C09",
                     g_probe.midSwapPc34Slot,
                     DM1_PC34_SLOT_ACTION_HAND, panel);
    ok &= expect_int("mid swap destination pc34 slot",
                     g_probe.midSwapDestinationPc34Slot,
                     DM1_PC34_SLOT_CHEST_1 +
                         DM1_PC34_AUTO_CHEST_ACTION_HAND_FUTURE_INDEX,
                     f0302);
    ok &= expect_int("mid swap leader before type",
                     g_probe.midSwapLeaderBeforeType,
                     DM1_PC34_AUTO_CHEST_ACTION_HAND_REPLACEMENT_CONTAINER,
                     panel);
    ok &= expect_int("mid swap slot before type",
                     g_probe.midSwapSlotBeforeType,
                     DM1_PC34_AUTO_CHEST_ACTION_HAND_FIRST_ITEM +
                         DM1_PC34_AUTO_CHEST_ACTION_HAND_FUTURE_INDEX,
                     f0334_loop);
    ok &= expect_int("mid swap removes C09 action hand",
                     g_probe.midSwapLeaderRemovedCall, 1, f0302);
    ok &= expect_int("mid swap removes future slot",
                     g_probe.midSwapSlotRemovedCall, 1, f0302);
    ok &= expect_int("mid swap puts displaced in C09",
                     g_probe.midSwapPutDisplacedInLeaderCall, 1, f0297);
    ok &= expect_int("mid swap puts replacement in slot",
                     g_probe.midSwapPutReplacementInSlotCall, 1, f0302);
    ok &= expect_int("mid swap C09 after holds displaced",
                     g_probe.midSwapLeaderAfterType,
                     DM1_PC34_AUTO_CHEST_ACTION_HAND_FIRST_ITEM +
                         DM1_PC34_AUTO_CHEST_ACTION_HAND_FUTURE_INDEX,
                     f0297);
    ok &= expect_int("mid swap slot after holds replacement",
                     g_probe.midSwapSlotAfterType,
                     DM1_PC34_AUTO_CHEST_ACTION_HAND_REPLACEMENT_CONTAINER,
                     f0302);
    ok &= expect_int("future original absent from slot after swap",
                     g_probe.futureOriginalAbsentFromSlotAfterSwap, 1,
                     f0302);
    ok &= expect_int("action hand becomes empty after swap",
                     g_probe.actionHandBecomesEmptyAfterSwap, 0, f0302);
    return ok;
}

static int test_close_rest_and_reopen(void)
{
    const char* f0333_press =
        "ReDMCSB CHEST.C F0333 P0694_B_PressingEye lines 32-42 + 44";
    const char* f0333_copy =
        "ReDMCSB CHEST.C F0333 lines 53-67 + 70-74";
    const char* f0334_begin =
        "ReDMCSB CHEST.C F0334 lines 113-117";
    const char* f0334_loop =
        "ReDMCSB CHEST.C F0334 lines 118-132";
    const char* f0297 = "ReDMCSB CHAMPION.C F0297/F0298 lines 243-298";
    int ok = 1;
    int i;

    ok &= expect_int("close count", g_probe.closeCount,
                     DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT,
                     f0334_loop);
    for (i = 0; i < DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT; ++i) {
        char label[96];
        int expected = DM1_PC34_AUTO_CHEST_ACTION_HAND_FIRST_ITEM + i;

        if (i == DM1_PC34_AUTO_CHEST_ACTION_HAND_FUTURE_INDEX) {
            expected = DM1_PC34_AUTO_CHEST_ACTION_HAND_REPLACEMENT_CONTAINER;
        }
        snprintf(label, sizeof(label), "closed C%d current type", 537 + i);
        ok &= expect_int(label, g_probe.closedTypes[i], expected,
                         f0334_loop);
        snprintf(label, sizeof(label), "closed C%d weight", 537 + i);
        ok &= expect_int(label, g_probe.closedWeights[i],
                         i == DM1_PC34_AUTO_CHEST_ACTION_HAND_FUTURE_INDEX ?
                             29 : 4 + i,
                         f0334_loop);
    }
    ok &= expect_int("replacement linked at future position",
                     g_probe.replacementLinkedAtFuturePosition, 1,
                     f0334_loop);
    ok &= expect_int("displaced absent from closed links",
                     g_probe.displacedAbsentFromClosedLinks, 1,
                     f0334_loop);
    ok &= expect_int("closed order uses current future slot",
                     g_probe.closedOrderUsesCurrentFutureSlot, 1,
                     f0334_loop);
    ok &= expect_int("future slot cleared by close",
                     g_probe.futureSlotClearedByClose, 1, f0334_loop);
    ok &= expect_int("all slots empty after close",
                     g_probe.allSlotsEmptyAfterClose, 1, f0334_loop);
    ok &= expect_int("action hand C09 after close holds displaced",
                     g_probe.actionHandAfterCloseType,
                     DM1_PC34_AUTO_CHEST_ACTION_HAND_FIRST_ITEM +
                         DM1_PC34_AUTO_CHEST_ACTION_HAND_FUTURE_INDEX,
                     f0297);
    ok &= expect_int("action hand C09 after close weight",
                     g_probe.actionHandAfterCloseWeight,
                     4 + DM1_PC34_AUTO_CHEST_ACTION_HAND_FUTURE_INDEX,
                     f0297);
    ok &= expect_int("closed end-of-list terminator",
                     g_probe.closedEndOfListTerminator,
                     DM1_PC34_AUTO_CHEST_ACTION_HAND_ENDOFLIST,
                     f0334_begin);

    ok &= expect_int("reopen result", g_probe.reopenResult, 1, f0333_copy);
    ok &= expect_int("reopen press eye path", 1, 1, f0333_press);
    ok &= expect_int("reopen thing", g_probe.reopenThing,
                     DM1_PC34_AUTO_CHEST_ACTION_HAND_REOPEN_THING,
                     f0333_copy);
    ok &= expect_int("reopened visible count",
                     g_probe.reopenedVisibleCount,
                     DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT,
                     f0333_copy);
    for (i = 0; i < DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT; ++i) {
        char label[96];
        int expected = DM1_PC34_AUTO_CHEST_ACTION_HAND_FIRST_ITEM + i;

        if (i == DM1_PC34_AUTO_CHEST_ACTION_HAND_FUTURE_INDEX) {
            expected = DM1_PC34_AUTO_CHEST_ACTION_HAND_REPLACEMENT_CONTAINER;
        }
        snprintf(label, sizeof(label), "reopened C%d type", 537 + i);
        ok &= expect_int(label, g_probe.reopenedTypes[i], expected,
                         f0333_copy);
    }
    ok &= expect_int("reopened order matches closed",
                     g_probe.reopenedOrderMatchesClosed, 1, f0333_copy);
    ok &= expect_int("replacement visible after reopen",
                     g_probe.replacementVisibleAfterReopen, 1, f0333_copy);
    ok &= expect_int("displaced absent after reopen",
                     g_probe.displacedAbsentAfterReopen, 1, f0333_copy);
    return ok;
}

static int test_already_processed_late_write_contract(void)
{
    const char* f0334_loop =
        "ReDMCSB CHEST.C F0334 lines 118-132";
    const char* f0302 =
        "ReDMCSB CHAMPION.C F0300/F0301/F0302 lines 511-515,606-614,688-710";
    int ok = 1;

    ok &= expect_int("already processed pc34 slot",
                     g_probe.alreadyProcessedPc34Slot,
                     DM1_PC34_SLOT_CHEST_1 +
                         DM1_PC34_AUTO_CHEST_ACTION_HAND_ALREADY_PROCESSED_INDEX,
                     f0302);
    ok &= expect_int("already processed C09 before type",
                     g_probe.alreadyProcessedLeaderBeforeType,
                     DM1_PC34_AUTO_CHEST_ACTION_HAND_REPLACEMENT_CONTAINER,
                     f0302);
    ok &= expect_int("already processed slot before late write",
                     g_probe.alreadyProcessedSlotBeforeLateWriteType, 0,
                     f0334_loop);
    ok &= expect_int("already processed swap result",
                     g_probe.alreadyProcessedSwapResult, 1, f0302);
    ok &= expect_int("already processed slot after late write",
                     g_probe.alreadyProcessedSlotAfterLateWriteType,
                     DM1_PC34_AUTO_CHEST_ACTION_HAND_REPLACEMENT_CONTAINER,
                     f0302);
    ok &= expect_int("already processed close count",
                     g_probe.alreadyProcessedCloseCount,
                     DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT,
                     f0334_loop);
    ok &= expect_int("already processed replacement not linked",
                     g_probe.alreadyProcessedReplacementLinked, 0,
                     f0334_loop);
    ok &= expect_int("already processed late write not revisited",
                     g_probe.alreadyProcessedLateWriteNotRevisited, 1,
                     f0334_loop);
    ok &= expect_int("already processed stale slot after close",
                     g_probe.alreadyProcessedStaleSlotAfterCloseType,
                     DM1_PC34_AUTO_CHEST_ACTION_HAND_REPLACEMENT_CONTAINER,
                     f0334_loop);
    ok &= expect_int("already processed C09 cleared after late write",
                     g_probe.alreadyProcessedLeaderAfterCloseType, 0,
                     f0302);
    return ok;
}

static int test_press_eye_replay_contract(void)
{
    const char* f0333_press =
        "ReDMCSB CHEST.C F0333 P0694_B_PressingEye lines 32-42 + 44";
    const char* f0334_begin =
        "ReDMCSB CHEST.C F0334 lines 113-117";
    const char* f0334_loop =
        "ReDMCSB CHEST.C F0334 lines 118-132";
    int ok = 1;

    ok &= expect_int("second press eye open result",
                     g_probe.secondPressEyeOpenResult, 1, f0333_press);
    ok &= expect_int("second press eye open thing",
                     g_probe.secondPressEyeOpenThing,
                     DM1_PC34_AUTO_CHEST_ACTION_HAND_REOPEN_THING,
                     f0333_press);
    ok &= expect_int("second close begin result",
                     g_probe.secondCloseBeginResult, 1, f0334_begin);
    ok &= expect_int("second container slot cleared to end",
                     g_probe.secondContainerSlotClearedToEnd, 1, f0334_begin);
    ok &= expect_int("second action hand C09 before close type",
                     g_probe.secondActionHandBeforeCloseType,
                     DM1_PC34_AUTO_CHEST_ACTION_HAND_REPLACEMENT_CONTAINER,
                     f0333_press);
    ok &= expect_int("second close count",
                     g_probe.secondCloseCount,
                     DM1_PC34_AUTO_CHEST_ACTION_HAND_SLOT_COUNT,
                     f0334_loop);
    ok &= expect_int("second closed end-of-list terminator",
                     g_probe.secondClosedEndOfListTerminator,
                     DM1_PC34_AUTO_CHEST_ACTION_HAND_ENDOFLIST,
                     f0334_begin);
    return ok;
}

int main(void)
{
    const char* f0333_copy =
        "ReDMCSB CHEST.C F0333 lines 53-67 + 70-74";
    int ok = 1;

    printf("probe=dm1_v1_auto_chest_action_hand_swap_during_close_gate_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_auto_chest_action_hand_swap_during_close_gate_source_evidence_pc34());

    ok &= expect_int("probe run",
                     dm1_v1_auto_chest_action_hand_swap_during_close_gate_run_pc34(
                         &g_probe),
                     1, f0333_copy);
    ok &= test_spec_and_evidence();
    ok &= test_press_eye_open_and_close_prefix();
    ok &= test_action_hand_mid_close_swap();
    ok &= test_close_rest_and_reopen();
    ok &= test_already_processed_late_write_contract();
    ok &= test_press_eye_replay_contract();
    ok &= expect_int("minimum assertion count",
                     g_assertions >= 60 ? 1 : 0, 1, f0333_copy);

    printf("assertionCount=%d\n", g_assertions);
    printf("autoChestActionHandSwapResultOk=%d closeCount=%d "
           "replacementLinked=%d staleLateWrite=%d actionHandAfterClose=%d "
           "endOfListTerminator=0x%X pressEyeReplay=%d\n",
           ok && g_failures == 0 ? 1 : 0,
           g_probe.closeCount,
           g_probe.replacementLinkedAtFuturePosition,
           g_probe.alreadyProcessedStaleSlotAfterCloseType,
           g_probe.actionHandAfterCloseType,
           g_probe.closedEndOfListTerminator,
           g_probe.secondPressEyeOpenResult);

    if (!ok || g_failures != 0) {
        printf("FAIL dm1_v1_auto_chest_action_hand_swap_during_close_gate_pc34_compat "
               "assertions=%d failures=%d\n",
               g_assertions, g_failures);
        return 1;
    }
    printf("PASS dm1_v1_auto_chest_action_hand_swap_during_close_gate_pc34_compat "
           "assertions=%d\n",
           g_assertions);
    return 0;
}
