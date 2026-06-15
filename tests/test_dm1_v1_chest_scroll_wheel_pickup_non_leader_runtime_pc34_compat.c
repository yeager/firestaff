#include "dm1_v1_chest_scroll_wheel_pickup_non_leader_runtime_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static int check_eq(const char* label, int got, int want, const char* anchor)
{
    ++g_assertions;
    if (!anchor || anchor[0] == '\0') {
        ++g_failures;
        printf("FAIL CHECK_EQ %s missing-anchor\n", label);
        return 0;
    }
    if (got != want) {
        ++g_failures;
        printf("FAIL CHECK_EQ %s got=%d want=%d anchor=%s\n",
               label, got, want, anchor);
        return 0;
    }
    printf("PASS CHECK_EQ %s=%d anchor=%s\n", label, got, anchor);
    return 1;
}

static int check_contains(const char* label,
                          const char* haystack,
                          const char* needle,
                          const char* anchor)
{
    ++g_assertions;
    if (!anchor || anchor[0] == '\0' || !haystack || !needle ||
        !strstr(haystack, needle)) {
        ++g_failures;
        printf("FAIL CHECK_EQ %s missing=%s anchor=%s\n",
               label, needle ? needle : "(null)",
               anchor ? anchor : "(null)");
        return 0;
    }
    printf("PASS CHECK_EQ %s contains=%s anchor=%s\n",
           label, needle, anchor);
    return 1;
}

#define CHECK_EQ(label, got, want, anchor) \
    check_eq((label), (int)(got), (int)(want), (anchor))
#define CHECK_CONTAINS(label, haystack, needle, anchor) \
    check_contains((label), (haystack), (needle), (anchor))

static int check_array(const char* label,
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
        ok &= CHECK_EQ(itemLabel, got[i], want[i], anchor);
    }
    return ok;
}

static int test_source_evidence(
    const DM1_V1_ChestScrollWheelPickupNonLeaderSpecPc34* spec)
{
    const char* evidence =
        dm1_v1_chest_scroll_wheel_pickup_non_leader_runtime_source_evidence_pc34();
    int ok = 1;

    ok &= CHECK_EQ("spec evidence pointer stable",
                   evidence == spec->sourceEvidence, 1,
                   spec->f0333OpenAnchor);
    ok &= CHECK_CONTAINS("evidence F0333", evidence,
                         "CHEST.C F0333:30-67", spec->f0333OpenAnchor);
    ok &= CHECK_CONTAINS("evidence F0334", evidence,
                         "CHEST.C F0334:113-132", spec->f0334CloseAnchor);
    ok &= CHECK_CONTAINS("evidence F0297", evidence,
                         "CHAMPION.C F0297:243-298", spec->f0297PutAnchor);
    ok &= CHECK_CONTAINS("evidence F0298", evidence,
                         "CHAMPION.C F0298:270-298",
                         spec->f0298RemoveAnchor);
    ok &= CHECK_CONTAINS("evidence F0300", evidence,
                         "CHAMPION.C F0300:511-515",
                         spec->f0300ClearAnchor);
    ok &= CHECK_CONTAINS("evidence F0301", evidence,
                         "CHAMPION.C F0301:606-614",
                         spec->f0301WriteAnchor);
    ok &= CHECK_CONTAINS("evidence F0302", evidence,
                         "CHAMPION.C F0302:662-714",
                         spec->f0302DispatchAnchor);
    ok &= CHECK_CONTAINS("evidence F0284", evidence,
                         "CHAMPION.C F0284:93-131",
                         spec->f0284RotationAnchor);
    ok &= CHECK_CONTAINS("evidence F0344", evidence,
                         "PANEL.C F0344:1895-1944",
                         spec->panelF0344Anchor);
    ok &= CHECK_CONTAINS("evidence F0345", evidence,
                         "PANEL.C F0345:1946-1999",
                         spec->panelF0345Anchor);
    ok &= CHECK_CONTAINS("evidence F0352", evidence, "PANEL.C F0352",
                         spec->panelF0352Anchor);
    ok &= CHECK_CONTAINS("evidence F0378", evidence,
                         "COMMAND.C F0378:1973-1983",
                         spec->commandF0378Anchor);
    ok &= CHECK_CONTAINS("evidence F0359", evidence,
                         "COMMAND.C F0359:1985-1990",
                         spec->commandF0359Anchor);
    ok &= CHECK_CONTAINS("evidence F0077", evidence,
                         "MOUSE.C F0077:97-126", spec->mouseF0077Anchor);
    ok &= CHECK_CONTAINS("evidence F0078", evidence,
                         "MOUSE.C F0078:128-168", spec->mouseF0078Anchor);
    ok &= CHECK_CONTAINS("evidence F0033", evidence,
                         "OBJECT.C F0033:147-212", spec->objectAnchor);
    ok &= CHECK_CONTAINS("evidence F0133", evidence,
                         "BLITMASK.C F0133:30-33",
                         spec->blitMaskAnchor);
    ok &= CHECK_CONTAINS("evidence DEFS 2088", evidence, "DEFS.H:2088",
                         spec->defsAnchor);
    ok &= CHECK_CONTAINS("evidence DEFS 810", evidence, "DEFS.H:810-816",
                         spec->defsAnchor);
    ok &= CHECK_CONTAINS("evidence DEFS 3906", evidence, "DEFS.H:3906-3913",
                         spec->defsAnchor);
    return ok;
}

static int test_constants(
    const DM1_V1_ChestScrollWheelPickupNonLeaderSpecPc34* spec)
{
    int ok = 1;

    ok &= CHECK_EQ("slot count", DM1_PC34_NL_CHEST_SLOT_COUNT, 8,
                   spec->defsAnchor);
    ok &= CHECK_EQ("leader stack count", DM1_PC34_NL_LEADER_STACK_COUNT, 4,
                   spec->defsAnchor);
    ok &= CHECK_EQ("champion count", DM1_PC34_NL_CHAMPION_COUNT, 4,
                   spec->defsAnchor);
    ok &= CHECK_EQ("C30", DM1_PC34_NL_C30, 30, spec->defsAnchor);
    ok &= CHECK_EQ("C31", DM1_PC34_NL_C31, 31, spec->defsAnchor);
    ok &= CHECK_EQ("C36", DM1_PC34_NL_C36, 36, spec->defsAnchor);
    ok &= CHECK_EQ("C537", DM1_PC34_NL_C537, 537, spec->defsAnchor);
    ok &= CHECK_EQ("C538", DM1_PC34_NL_C538, 538, spec->defsAnchor);
    ok &= CHECK_EQ("C539", DM1_PC34_NL_C539, 539, spec->defsAnchor);
    ok &= CHECK_EQ("C540", DM1_PC34_NL_C540, 540, spec->defsAnchor);
    ok &= CHECK_EQ("C541", DM1_PC34_NL_C541, 541, spec->defsAnchor);
    ok &= CHECK_EQ("C542", DM1_PC34_NL_C542, 542, spec->defsAnchor);
    ok &= CHECK_EQ("C543", DM1_PC34_NL_C543, 543, spec->defsAnchor);
    ok &= CHECK_EQ("C544", DM1_PC34_NL_C544, 544, spec->defsAnchor);
    ok &= CHECK_EQ("M568", DM1_PC34_NL_PANEL_M568, 568,
                   spec->commandF0378Anchor);
    ok &= CHECK_EQ("C040", DM1_PC34_NL_COMMAND_C040, 40,
                   spec->commandF0359Anchor);
    ok &= CHECK_EQ("slot box 39", DM1_PC34_NL_SLOT_BOX_C39, 39,
                   spec->f0302DispatchAnchor);
    return ok;
}

static int test_initial_state(
    const DM1_V1_ChestScrollWheelPickupNonLeaderProbePc34* probe,
    const DM1_V1_ChestScrollWheelPickupNonLeaderSpecPc34* spec)
{
    const int chestWant[DM1_PC34_NL_CHEST_SLOT_COUNT] = {
        DM1_PC34_NL_CHEST_ITEM0,
        DM1_PC34_NL_CHEST_ITEM1_PICKED,
        DM1_PC34_NL_CHEST_ITEM2,
        DM1_PC34_NL_CHEST_ITEM3,
        DM1_PC34_NL_CHEST_ITEM4,
        DM1_PC34_NL_CHEST_ITEM5,
        DM1_PC34_NL_CHEST_ITEM6,
        DM1_PC34_NL_CHEST_ITEM7
    };
    const int iconsWant[DM1_PC34_NL_CHEST_SLOT_COUNT] = {
        DM1_PC34_NL_CHEST_ITEM0 & 0x00FF,
        DM1_PC34_NL_CHEST_ITEM1_PICKED & 0x00FF,
        DM1_PC34_NL_CHEST_ITEM2 & 0x00FF,
        DM1_PC34_NL_CHEST_ITEM3 & 0x00FF,
        DM1_PC34_NL_CHEST_ITEM4 & 0x00FF,
        DM1_PC34_NL_CHEST_ITEM5 & 0x00FF,
        DM1_PC34_NL_CHEST_ITEM6 & 0x00FF,
        DM1_PC34_NL_CHEST_ITEM7 & 0x00FF
    };
    const int leaderWant[DM1_PC34_NL_LEADER_STACK_COUNT] = {
        DM1_PC34_NL_LEADER_ITEM0,
        DM1_PC34_NL_LEADER_ITEM1,
        DM1_PC34_NL_LEADER_ITEM2,
        DM1_PC34_NL_LEADER_ITEM3
    };
    int ok = 1;

    ok &= CHECK_EQ("open chest thing", probe->openChestThing,
                   DM1_PC34_NL_OPEN_CHEST, spec->f0333OpenAnchor);
    ok &= check_array("initial C537-C544", probe->initialChest, chestWant,
                      DM1_PC34_NL_CHEST_SLOT_COUNT, spec->f0333OpenAnchor);
    ok &= check_array("initial F0033 icons", probe->initialIcons, iconsWant,
                      DM1_PC34_NL_CHEST_SLOT_COUNT, spec->objectAnchor);
    ok &= check_array("initial leader C540-C543",
                      probe->initialLeaderStack, leaderWant,
                      DM1_PC34_NL_LEADER_STACK_COUNT, spec->f0297PutAnchor);
    ok &= CHECK_EQ("party count before", probe->partyChampionCountBefore, 4,
                   spec->defsAnchor);
    ok &= CHECK_EQ("inventory champion ordinal before",
                   probe->inventoryChampionOrdinalBefore, 3,
                   spec->defsAnchor);
    ok &= CHECK_EQ("leader before rotation", probe->leaderIndexBeforeRotation,
                   0, spec->f0284RotationAnchor);
    ok &= CHECK_EQ("M570 chain before", probe->m570ChainLengthBefore, 8,
                   spec->f0333OpenAnchor);
    ok &= CHECK_EQ("M516 C30 things before", probe->m516ThingCountBefore, 0,
                   spec->defsAnchor);
    ok &= CHECK_EQ("total things before", probe->totalThingCountBefore, 12,
                   spec->defsAnchor);
    return ok;
}

static int test_scroll_route(
    const DM1_V1_ChestScrollWheelPickupNonLeaderProbePc34* probe,
    const DM1_V1_ChestScrollWheelPickupNonLeaderSpecPc34* spec)
{
    const int highlightWant[3] = {
        DM1_PC34_NL_C537,
        DM1_PC34_NL_C538,
        DM1_PC34_NL_C538
    };
    int ok = 1;

    ok &= CHECK_EQ("leader after rotation", probe->leaderIndexAfterRotation,
                   1, spec->f0284RotationAnchor);
    ok &= CHECK_EQ("target champion", probe->targetChampionIndex, 2,
                   spec->f0302DispatchAnchor);
    ok &= CHECK_EQ("target C30 slot", probe->targetSlotIndex,
                   DM1_PC34_NL_C30, spec->f0301WriteAnchor);
    ok &= CHECK_EQ("target slot before", probe->targetSlotBefore,
                   DM1_PC34_NL_NONE, spec->f0302DispatchAnchor);
    ok &= CHECK_EQ("wheel queued F0077", probe->wheelQueuedByF0077, 1,
                   spec->mouseF0077Anchor);
    ok &= CHECK_EQ("wheel read F0078", probe->wheelReadByF0078, 1,
                   spec->mouseF0078Anchor);
    ok &= CHECK_EQ("wheel queue drained", probe->wheelQueueDepthAfterRead, 0,
                   spec->mouseF0078Anchor);
    ok &= CHECK_EQ("wheel target C538", probe->wheelTargetZone,
                   DM1_PC34_NL_C538, spec->panelF0345Anchor);
    ok &= CHECK_EQ("panel F0344 dispatch",
                   probe->panelF0344DispatchCount, 1,
                   spec->panelF0344Anchor);
    ok &= CHECK_EQ("panel F0345 highlight",
                   probe->panelF0345HighlightCount, 2,
                   spec->panelF0345Anchor);
    ok &= check_array("highlight trace", probe->highlightTrace,
                      highlightWant, 3, spec->panelF0345Anchor);
    ok &= CHECK_EQ("partial-mask dispatches", probe->partialMaskDispatches, 2,
                   spec->blitMaskAnchor);
    ok &= CHECK_EQ("COMMAND F0378 dispatch",
                   probe->commandF0378Dispatch, 1,
                   spec->commandF0378Anchor);
    ok &= CHECK_EQ("COMMAND F0359 dispatch",
                   probe->commandF0359Dispatch, 1,
                   spec->commandF0359Anchor);
    ok &= CHECK_EQ("command M568", probe->commandM568,
                   DM1_PC34_NL_PANEL_M568, spec->commandF0378Anchor);
    ok &= CHECK_EQ("command C040", probe->commandC040,
                   DM1_PC34_NL_COMMAND_C040, spec->commandF0359Anchor);
    ok &= CHECK_EQ("slot box index", probe->commandSlotBoxIndex,
                   DM1_PC34_NL_SLOT_BOX_C39, spec->f0302DispatchAnchor);
    ok &= CHECK_EQ("dispatch C31 from C538",
                   probe->championSlotIndexFromDispatch, DM1_PC34_NL_C31,
                   spec->f0302DispatchAnchor);
    ok &= CHECK_EQ("dispatch chest offset", probe->chestOffsetFromDispatch, 1,
                   spec->f0302DispatchAnchor);
    ok &= CHECK_EQ("dispatch read thing", probe->dispatchReadThing,
                   DM1_PC34_NL_CHEST_ITEM1_PICKED,
                   spec->f0302DispatchAnchor);
    ok &= CHECK_EQ("dispatch read icon", probe->dispatchReadIcon,
                   DM1_PC34_NL_CHEST_ITEM1_PICKED & 0x00FF,
                   spec->objectAnchor);
    return ok;
}

static int test_pickup_and_close(
    const DM1_V1_ChestScrollWheelPickupNonLeaderProbePc34* probe,
    const DM1_V1_ChestScrollWheelPickupNonLeaderSpecPc34* spec)
{
    const int leaderWant[DM1_PC34_NL_LEADER_STACK_COUNT] = {
        DM1_PC34_NL_LEADER_ITEM0,
        DM1_PC34_NL_LEADER_ITEM1,
        DM1_PC34_NL_LEADER_ITEM2,
        DM1_PC34_NL_LEADER_ITEM3
    };
    const int chestAfter[DM1_PC34_NL_CHEST_SLOT_COUNT] = {
        DM1_PC34_NL_CHEST_ITEM0,
        DM1_PC34_NL_NONE,
        DM1_PC34_NL_CHEST_ITEM2,
        DM1_PC34_NL_CHEST_ITEM3,
        DM1_PC34_NL_CHEST_ITEM4,
        DM1_PC34_NL_CHEST_ITEM5,
        DM1_PC34_NL_CHEST_ITEM6,
        DM1_PC34_NL_CHEST_ITEM7
    };
    const int iconsAfter[DM1_PC34_NL_CHEST_SLOT_COUNT] = {
        DM1_PC34_NL_CHEST_ITEM0 & 0x00FF,
        DM1_PC34_NL_NONE,
        DM1_PC34_NL_CHEST_ITEM2 & 0x00FF,
        DM1_PC34_NL_CHEST_ITEM3 & 0x00FF,
        DM1_PC34_NL_CHEST_ITEM4 & 0x00FF,
        DM1_PC34_NL_CHEST_ITEM5 & 0x00FF,
        DM1_PC34_NL_CHEST_ITEM6 & 0x00FF,
        DM1_PC34_NL_CHEST_ITEM7 & 0x00FF
    };
    const int closedWant[DM1_PC34_NL_CHEST_SLOT_COUNT] = {
        DM1_PC34_NL_CHEST_ITEM0,
        DM1_PC34_NL_CHEST_ITEM2,
        DM1_PC34_NL_CHEST_ITEM3,
        DM1_PC34_NL_CHEST_ITEM4,
        DM1_PC34_NL_CHEST_ITEM5,
        DM1_PC34_NL_CHEST_ITEM6,
        DM1_PC34_NL_CHEST_ITEM7,
        DM1_PC34_NL_NONE
    };
    const int closedIconWant[DM1_PC34_NL_CHEST_SLOT_COUNT] = {
        DM1_PC34_NL_CHEST_ITEM0 & 0x00FF,
        DM1_PC34_NL_CHEST_ITEM2 & 0x00FF,
        DM1_PC34_NL_CHEST_ITEM3 & 0x00FF,
        DM1_PC34_NL_CHEST_ITEM4 & 0x00FF,
        DM1_PC34_NL_CHEST_ITEM5 & 0x00FF,
        DM1_PC34_NL_CHEST_ITEM6 & 0x00FF,
        DM1_PC34_NL_CHEST_ITEM7 & 0x00FF,
        DM1_PC34_NL_NONE
    };
    int ok = 1;

    ok &= CHECK_EQ("leader hand full", probe->leaderHandWasFull, 1,
                   spec->f0297PutAnchor);
    ok &= CHECK_EQ("skipped occupied leader hand",
                   probe->skippedOccupiedLeaderHand, 1,
                   spec->f0298RemoveAnchor);
    ok &= CHECK_EQ("F0297 skipped", probe->f0297PutCount, 0,
                   spec->f0297PutAnchor);
    ok &= CHECK_EQ("F0298 skipped", probe->f0298RemoveCount, 0,
                   spec->f0298RemoveAnchor);
    ok &= CHECK_EQ("F0302 dispatch count", probe->f0302DispatchCount, 1,
                   spec->f0302DispatchAnchor);
    ok &= CHECK_EQ("F0300 clears C538", probe->f0300ClearCount, 1,
                   spec->f0300ClearAnchor);
    ok &= CHECK_EQ("F0301 writes non-leader C30",
                   probe->f0301WriteCount, 1, spec->f0301WriteAnchor);
    ok &= CHECK_EQ("pressing-eye inactive", probe->pressingEyeRouteCount, 0,
                   spec->panelF0352Anchor);
    ok &= CHECK_EQ("draw state champion", probe->drawStateChampion, 2,
                   spec->f0302DispatchAnchor);
    ok &= CHECK_EQ("target slot after", probe->targetSlotAfter,
                   DM1_PC34_NL_CHEST_ITEM1_PICKED,
                   spec->f0301WriteAnchor);
    ok &= CHECK_EQ("target icon after", probe->targetSlotIconAfter,
                   DM1_PC34_NL_CHEST_ITEM1_PICKED & 0x00FF,
                   spec->objectAnchor);
    ok &= CHECK_EQ("target load before", probe->targetLoadBefore, 42,
                   spec->defsAnchor);
    ok &= CHECK_EQ("target load after", probe->targetLoadAfter, 44,
                   spec->f0301WriteAnchor);
    ok &= CHECK_EQ("target load mask",
                   probe->targetAttributesAfter & DM1_PC34_NL_LOAD_MASK,
                   DM1_PC34_NL_LOAD_MASK, spec->defsAnchor);
    ok &= CHECK_EQ("target panel mask",
                   probe->targetAttributesAfter & DM1_PC34_NL_PANEL_MASK,
                   DM1_PC34_NL_PANEL_MASK, spec->defsAnchor);
    ok &= check_array("leader stack after pickup",
                      probe->leaderStackAfterPickup, leaderWant,
                      DM1_PC34_NL_LEADER_STACK_COUNT,
                      spec->f0297PutAnchor);
    ok &= check_array("chest after pickup", probe->chestAfterPickup,
                      chestAfter, DM1_PC34_NL_CHEST_SLOT_COUNT,
                      spec->f0300ClearAnchor);
    ok &= check_array("icons after pickup", probe->iconsAfterPickup,
                      iconsAfter, DM1_PC34_NL_CHEST_SLOT_COUNT,
                      spec->objectAnchor);
    ok &= CHECK_EQ("close count", probe->closeCount, 7,
                   spec->f0334CloseAnchor);
    ok &= CHECK_EQ("close skipped C538 none",
                   probe->closeSkippedNoneEntries, 1,
                   spec->f0334CloseAnchor);
    ok &= CHECK_EQ("close head", probe->closeHead,
                   DM1_PC34_NL_CHEST_ITEM0, spec->f0334CloseAnchor);
    ok &= check_array("closed slots", probe->closedSlots, closedWant,
                      DM1_PC34_NL_CHEST_SLOT_COUNT,
                      spec->f0334CloseAnchor);
    ok &= check_array("closed icons", probe->closedIcons, closedIconWant,
                      DM1_PC34_NL_CHEST_SLOT_COUNT, spec->objectAnchor);
    ok &= CHECK_EQ("picked item absent from closed chain",
                   probe->pickedItemInClosedChain, 0,
                   spec->f0334CloseAnchor);
    ok &= CHECK_EQ("close end sentinel", probe->closeEndSentinel,
                   DM1_PC34_NL_END_OF_LIST, spec->f0334CloseAnchor);
    ok &= CHECK_EQ("open chest after close", probe->openChestAfterClose,
                   DM1_PC34_NL_NONE, spec->f0334CloseAnchor);
    ok &= CHECK_EQ("G0425 cleared after close",
                   probe->g0425ClearedAfterClose, 1,
                   spec->f0334CloseAnchor);
    ok &= check_array("leader stack after close",
                      probe->leaderStackAfterClose, leaderWant,
                      DM1_PC34_NL_LEADER_STACK_COUNT,
                      spec->f0298RemoveAnchor);
    ok &= CHECK_EQ("party count after", probe->partyChampionCountAfter, 4,
                   spec->defsAnchor);
    ok &= CHECK_EQ("M570 closed chain length", probe->m570ChainLengthAfter, 7,
                   spec->f0334CloseAnchor);
    ok &= CHECK_EQ("M516 C30 things after", probe->m516ThingCountAfter, 1,
                   spec->defsAnchor);
    ok &= CHECK_EQ("total thing invariant", probe->totalThingCountAfter,
                   probe->totalThingCountBefore, spec->defsAnchor);
    ok &= CHECK_EQ("screen update balanced", probe->screenUpdateBalanced, 1,
                   spec->mouseF0078Anchor);
    return ok;
}

static int test_negative_path(
    const DM1_V1_ChestScrollWheelPickupNonLeaderProbePc34* probe,
    const DM1_V1_ChestScrollWheelPickupNonLeaderSpecPc34* spec)
{
    int ok = 1;

    ok &= CHECK_EQ("negative wheel absent", probe->negativeWheelQueued, 0,
                   spec->mouseF0077Anchor);
    ok &= CHECK_EQ("negative target before", probe->negativeTargetBefore,
                   DM1_PC34_NL_NEGATIVE_TARGET_ITEM,
                   spec->f0302DispatchAnchor);
    ok &= CHECK_EQ("negative target after", probe->negativeTargetAfter,
                   DM1_PC34_NL_NEGATIVE_TARGET_ITEM,
                   spec->f0302DispatchAnchor);
    ok &= CHECK_EQ("negative chest C538 before",
                   probe->negativeChestC538Before,
                   DM1_PC34_NL_CHEST_ITEM1_PICKED,
                   spec->f0333OpenAnchor);
    ok &= CHECK_EQ("negative chest C538 after",
                   probe->negativeChestC538After,
                   DM1_PC34_NL_CHEST_ITEM1_PICKED,
                   spec->f0333OpenAnchor);
    ok &= CHECK_EQ("negative did not mutate", probe->negativeMutated, 0,
                   spec->f0302DispatchAnchor);
    ok &= CHECK_EQ("negative no extra F0301 write",
                   probe->negativeF0301WriteCount,
                   probe->f0301WriteCount, spec->f0301WriteAnchor);
    return ok;
}

int main(void)
{
    DM1_V1_ChestScrollWheelPickupNonLeaderProbePc34 probe;
    const DM1_V1_ChestScrollWheelPickupNonLeaderSpecPc34* spec =
        dm1_v1_chest_scroll_wheel_pickup_non_leader_runtime_spec_pc34();
    int ok = 1;

    ok &= CHECK_EQ("probe run",
                   dm1_v1_chest_scroll_wheel_pickup_non_leader_runtime_pc34(
                       &probe),
                   1, spec->f0333OpenAnchor);
    ok &= test_source_evidence(spec);
    ok &= test_constants(spec);
    ok &= test_initial_state(&probe, spec);
    ok &= test_scroll_route(&probe, spec);
    ok &= test_pickup_and_close(&probe, spec);
    ok &= test_negative_path(&probe, spec);
    ok &= CHECK_EQ("assertion floor", g_assertions >= 80, 1,
                   spec->defsAnchor);

    printf("SUMMARY assertions=%d failures=%d\n", g_assertions, g_failures);
    return (ok && g_failures == 0 && g_assertions >= 80) ? 0 : 1;
}
