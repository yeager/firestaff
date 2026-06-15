#include "../src/dm1/dm1_v1_chest_scroll_wheel_pickup_overflow_pc34_compat.h"

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

static int expect_slots(const char* label,
                        const int* got,
                        const int* want,
                        const char* anchor)
{
    int ok = 1;
    int i;

    for (i = 0; i < DM1_PC34_PICKUP_OVERFLOW_SLOT_COUNT; ++i) {
        char slotLabel[96];

        snprintf(slotLabel, sizeof(slotLabel), "%s[%d]", label, i);
        ok &= expect_int(slotLabel, got[i], want[i], anchor);
    }
    return ok;
}

static int test_source_evidence(
    const DM1_V1_ChestScrollWheelPickupOverflowSpecPc34* spec)
{
    const char* evidence =
        dm1_v1_chest_scroll_wheel_pickup_overflow_source_evidence_pc34();
    int ok = 1;

    ok &= expect_contains("source CHEST F0333", evidence,
                          "CHEST.C F0333:30-67", spec->f0333OpenAnchor);
    ok &= expect_contains("source CHEST F0334", evidence,
                          "CHEST.C F0334:113-132", spec->f0334CloseAnchor);
    ok &= expect_contains("source CHAMPION F0297", evidence,
                          "CHAMPION.C F0297:243-268", spec->f0297PutAnchor);
    ok &= expect_contains("source CHAMPION F0298", evidence,
                          "CHAMPION.C F0298:270-298",
                          spec->f0298RemoveAnchor);
    ok &= expect_contains("source CHAMPION F0302", evidence,
                          "CHAMPION.C F0302:662-710",
                          spec->f0302DispatchAnchor);
    ok &= expect_contains("source PANEL", evidence,
                          "PANEL.C F0344:1895-1944", spec->panelAnchor);
    ok &= expect_contains("source COMMAND", evidence,
                          "COMMAND.C F0359:1985-1990",
                          spec->commandAnchor);
    ok &= expect_contains("source MOUSE", evidence, "MOUSE.C F0077:97-126",
                          spec->mouseAnchor);
    ok &= expect_contains("source OBJECT", evidence,
                          "OBJECT.C F0033:147-212", spec->objectAnchor);
    ok &= expect_contains("source BLITMASK", evidence,
                          "BLITMASK.C F0133:30-33", spec->blitMaskAnchor);
    ok &= expect_contains("source DEFS", evidence, "DEFS.H:2088",
                          spec->defsAnchor);
    ok &= expect_contains("source route", evidence, "ROUTE_C544_REPLACEMENT",
                          spec->f0302DispatchAnchor);
    ok &= expect_contains("source non-first-free", evidence,
                          "not ROUTE_TO_FIRST_FREE",
                          spec->f0302DispatchAnchor);
    return ok;
}

static int test_spec(
    const DM1_V1_ChestScrollWheelPickupOverflowSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("contract only", spec->contractOnly, 1,
                     spec->f0333OpenAnchor);
    ok &= expect_int("source locked route", spec->sourceLockedRoute,
                     DM1_V1_PICKUP_OVERFLOW_ROUTE_C544_REPLACEMENT,
                     spec->f0302DispatchAnchor);
    ok &= expect_contains("route label", spec->routeLabel,
                          "ROUTE_C544_REPLACEMENT",
                          spec->f0302DispatchAnchor);
    ok &= expect_int("C30 constant", DM1_PC34_PICKUP_OVERFLOW_C30, 30,
                     spec->defsAnchor);
    ok &= expect_int("C537 zone", DM1_PC34_PICKUP_OVERFLOW_C537, 537,
                     spec->defsAnchor);
    ok &= expect_int("C544 zone", DM1_PC34_PICKUP_OVERFLOW_C544, 544,
                     spec->defsAnchor);
    ok &= expect_int("M568 marker", DM1_PC34_PICKUP_OVERFLOW_PANEL_M568, 568,
                     spec->commandAnchor);
    ok &= expect_int("C040 marker", DM1_PC34_PICKUP_OVERFLOW_COMMAND_C040,
                     40, spec->commandAnchor);
    return ok;
}

static int test_initial_state(
    const DM1_V1_ChestScrollWheelPickupOverflowProbePc34* probe,
    const DM1_V1_ChestScrollWheelPickupOverflowSpecPc34* spec)
{
    const int slotsWant[DM1_PC34_PICKUP_OVERFLOW_SLOT_COUNT] = {
        DM1_PC34_PICKUP_OVERFLOW_CHEST_ITEM0,
        DM1_PC34_PICKUP_OVERFLOW_NONE,
        DM1_PC34_PICKUP_OVERFLOW_CHEST_ITEM2,
        DM1_PC34_PICKUP_OVERFLOW_NONE,
        DM1_PC34_PICKUP_OVERFLOW_NONE,
        DM1_PC34_PICKUP_OVERFLOW_CHEST_ITEM5,
        DM1_PC34_PICKUP_OVERFLOW_NONE,
        DM1_PC34_PICKUP_OVERFLOW_CHEST_ITEM7
    };
    const int iconsWant[DM1_PC34_PICKUP_OVERFLOW_SLOT_COUNT] = {
        DM1_PC34_PICKUP_OVERFLOW_CHEST_ITEM0 & 0x00FF,
        DM1_PC34_PICKUP_OVERFLOW_NONE,
        DM1_PC34_PICKUP_OVERFLOW_CHEST_ITEM2 & 0x00FF,
        DM1_PC34_PICKUP_OVERFLOW_NONE,
        DM1_PC34_PICKUP_OVERFLOW_NONE,
        DM1_PC34_PICKUP_OVERFLOW_CHEST_ITEM5 & 0x00FF,
        DM1_PC34_PICKUP_OVERFLOW_NONE,
        DM1_PC34_PICKUP_OVERFLOW_CHEST_ITEM7 & 0x00FF
    };
    int ok = 1;

    ok &= expect_int("open chest thing", probe->openChestThing,
                     DM1_PC34_PICKUP_OVERFLOW_OPEN_CHEST,
                     spec->f0333OpenAnchor);
    ok &= expect_slots("initial G0425 slots", probe->initialSlots,
                       slotsWant, spec->f0333OpenAnchor);
    ok &= expect_slots("initial F0033 icons", probe->initialIcons,
                       iconsWant, spec->objectAnchor);
    ok &= expect_int("initial leader full", probe->initialLeaderThing,
                     DM1_PC34_PICKUP_OVERFLOW_LEADER_ITEM,
                     spec->f0297PutAnchor);
    ok &= expect_int("initial leader icon", probe->initialLeaderIcon,
                     DM1_PC34_PICKUP_OVERFLOW_LEADER_ITEM & 0x00FF,
                     spec->objectAnchor);
    ok &= expect_int("initial leader load", probe->initialLeaderLoad,
                     DM1_PC34_PICKUP_OVERFLOW_BASE_LOAD +
                         DM1_PC34_PICKUP_OVERFLOW_LEADER_WEIGHT,
                     spec->f0297PutAnchor);
    ok &= expect_int("initial load mask", probe->initialLeaderAttributes,
                     DM1_PC34_PICKUP_OVERFLOW_LOAD_MASK,
                     spec->f0297PutAnchor);
    ok &= expect_int("initial chest count", probe->initialChestItemCount, 4,
                     spec->f0333OpenAnchor);
    ok &= expect_int("first free before", probe->firstFreeSlotBefore, 1,
                     spec->f0333OpenAnchor);
    ok &= expect_int("C544 initially occupied", probe->c544InitiallyOccupied,
                     1, spec->f0333OpenAnchor);
    ok &= expect_int("total things before", probe->totalThingsBefore, 5,
                     spec->f0302DispatchAnchor);
    return ok;
}

static int test_dispatch_chain(
    const DM1_V1_ChestScrollWheelPickupOverflowProbePc34* probe,
    const DM1_V1_ChestScrollWheelPickupOverflowSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("wheel queued", probe->wheelQueuedByF0078, 1,
                     spec->mouseAnchor);
    ok &= expect_int("wheel queue depth", probe->wheelQueueDepth, 1,
                     spec->mouseAnchor);
    ok &= expect_int("wheel target C544", probe->wheelTargetZone,
                     DM1_PC34_PICKUP_OVERFLOW_C544, spec->panelAnchor);
    ok &= expect_int("panel dispatch M568", probe->panelDispatch,
                     DM1_PC34_PICKUP_OVERFLOW_PANEL_M568,
                     spec->commandAnchor);
    ok &= expect_int("command dispatch C040", probe->commandDispatch,
                     DM1_PC34_PICKUP_OVERFLOW_COMMAND_C040,
                     spec->commandAnchor);
    ok &= expect_int("command C065 chest 8", probe->commandClick,
                     DM1_PC34_PICKUP_OVERFLOW_CLICK_C065,
                     spec->commandAnchor);
    ok &= expect_int("slot box index C45", probe->commandSlotBoxIndex,
                     DM1_PC34_PICKUP_OVERFLOW_SLOT_BOX_C45,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("champion slot C37", probe->championSlotIndex,
                     DM1_PC34_PICKUP_OVERFLOW_C37_CHEST_8,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("chest slot offset", probe->chestSlotOffset, 7,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("dispatch read C544", probe->dispatchReadC544Thing,
                     DM1_PC34_PICKUP_OVERFLOW_CHEST_ITEM7,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("dispatch read leader", probe->dispatchReadLeaderThing,
                     DM1_PC34_PICKUP_OVERFLOW_LEADER_ITEM,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("allowed slot mask", probe->allowedSlotMaskMatched, 1,
                     spec->f0302DispatchAnchor);
    return ok;
}

static int test_overflow_route(
    const DM1_V1_ChestScrollWheelPickupOverflowProbePc34* probe,
    const DM1_V1_ChestScrollWheelPickupOverflowSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("route taken", probe->routeTaken,
                     DM1_V1_PICKUP_OVERFLOW_ROUTE_C544_REPLACEMENT,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("rejected keep leader path",
                     probe->rejectedKeepLeaderPath, 0,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("first free route not used",
                     probe->routedToFirstFreePath, 0,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("C544 replacement route used",
                     probe->routedToC544ReplacementPath, 1,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("first free slot used", probe->firstFreeSlotUsed, -1,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("leader remove count", probe->leaderRemovedCount, 1,
                     spec->f0298RemoveAnchor);
    ok &= expect_int("leader put count", probe->leaderPutCount, 1,
                     spec->f0297PutAnchor);
    ok &= expect_int("slot remove count", probe->slotRemoveCount, 1,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("slot add count", probe->slotAddCount, 1,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("leader thing after", probe->leaderThingAfter,
                     DM1_PC34_PICKUP_OVERFLOW_CHEST_ITEM7,
                     spec->f0297PutAnchor);
    ok &= expect_int("leader icon after", probe->leaderIconAfter,
                     DM1_PC34_PICKUP_OVERFLOW_CHEST_ITEM7 & 0x00FF,
                     spec->objectAnchor);
    ok &= expect_int("leader load after", probe->leaderLoadAfter,
                     DM1_PC34_PICKUP_OVERFLOW_BASE_LOAD +
                         DM1_PC34_PICKUP_OVERFLOW_C544_WEIGHT,
                     spec->f0297PutAnchor);
    ok &= expect_int("leader load mask after", probe->leaderAttributesAfter,
                     DM1_PC34_PICKUP_OVERFLOW_LOAD_MASK,
                     spec->f0297PutAnchor);
    ok &= expect_int("C544 thing after", probe->c544ThingAfter,
                     DM1_PC34_PICKUP_OVERFLOW_LEADER_ITEM,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("C544 icon after", probe->c544IconAfter,
                     DM1_PC34_PICKUP_OVERFLOW_LEADER_ITEM & 0x00FF,
                     spec->objectAnchor);
    ok &= expect_int("first free after unchanged", probe->firstFreeSlotAfter,
                     1, spec->f0302DispatchAnchor);
    ok &= expect_int("leader stack count after",
                     probe->leaderStackCountAfter, 1,
                     spec->f0297PutAnchor);
    ok &= expect_int("total things after", probe->totalThingsAfter,
                     probe->totalThingsBefore, spec->f0302DispatchAnchor);
    ok &= expect_int("screen update balanced", probe->screenUpdateBalanced,
                     1, spec->mouseAnchor);
    return ok;
}

static int test_close_rewrite(
    const DM1_V1_ChestScrollWheelPickupOverflowProbePc34* probe,
    const DM1_V1_ChestScrollWheelPickupOverflowSpecPc34* spec)
{
    const int closedWant[DM1_PC34_PICKUP_OVERFLOW_SLOT_COUNT] = {
        DM1_PC34_PICKUP_OVERFLOW_CHEST_ITEM0,
        DM1_PC34_PICKUP_OVERFLOW_CHEST_ITEM2,
        DM1_PC34_PICKUP_OVERFLOW_CHEST_ITEM5,
        DM1_PC34_PICKUP_OVERFLOW_LEADER_ITEM,
        DM1_PC34_PICKUP_OVERFLOW_NONE,
        DM1_PC34_PICKUP_OVERFLOW_NONE,
        DM1_PC34_PICKUP_OVERFLOW_NONE,
        DM1_PC34_PICKUP_OVERFLOW_NONE
    };
    int ok = 1;

    ok &= expect_int("close count", probe->closeCount, 4,
                     spec->f0334CloseAnchor);
    ok &= expect_int("close skipped NONE gaps",
                     probe->closeSkippedNoneEntries, 4,
                     spec->f0334CloseAnchor);
    ok &= expect_int("close head", probe->closeHead,
                     DM1_PC34_PICKUP_OVERFLOW_CHEST_ITEM0,
                     spec->f0334CloseAnchor);
    ok &= expect_int("close end sentinel", probe->closeEndSentinel,
                     DM1_PC34_PICKUP_OVERFLOW_END_OF_LIST,
                     spec->f0334CloseAnchor);
    ok &= expect_slots("closed rewrite", probe->closedSlots, closedWant,
                       spec->f0334CloseAnchor);
    ok &= expect_int("C544 replacement rewritten",
                     probe->c544ReplacementRewrittenOnClose, 1,
                     spec->f0334CloseAnchor);
    ok &= expect_int("original C544 now in leader",
                     probe->originalC544NowInLeader, 1,
                     spec->f0297PutAnchor);
    ok &= expect_int("G0425 cleared after close",
                     probe->g0425ClearedAfterClose, 1,
                     spec->f0334CloseAnchor);
    return ok;
}

int main(void)
{
    DM1_V1_ChestScrollWheelPickupOverflowProbePc34 probe;
    const DM1_V1_ChestScrollWheelPickupOverflowSpecPc34* spec =
        dm1_v1_chest_scroll_wheel_pickup_overflow_spec_pc34();
    int ok = 1;

    ok &= expect_int("probe run",
                     dm1_v1_chest_scroll_wheel_pickup_overflow_pc34(&probe),
                     1, spec->f0333OpenAnchor);
    ok &= test_source_evidence(spec);
    ok &= test_spec(spec);
    ok &= test_initial_state(&probe, spec);
    ok &= test_dispatch_chain(&probe, spec);
    ok &= test_overflow_route(&probe, spec);
    ok &= test_close_rewrite(&probe, spec);

    printf("SUMMARY assertions=%d failures=%d\n", g_assertions, g_failures);
    return (ok && g_failures == 0) ? 0 : 1;
}
