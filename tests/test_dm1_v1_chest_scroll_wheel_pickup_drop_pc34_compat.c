#include "dm1/dm1_v1_chest_scroll_wheel_pickup_drop_pc34_compat.h"

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

static int expect_slot_array(const char* label,
                             const int* got,
                             const int* want,
                             const char* anchor)
{
    int i;
    int ok = 1;

    for (i = 0; i < DM1_PC34_CHEST_SCROLL_WHEEL_SLOT_COUNT; ++i) {
        char slotLabel[96];

        snprintf(slotLabel, sizeof(slotLabel), "%s C%d", label,
                 DM1_PC34_CHEST_SCROLL_WHEEL_C537 + i);
        ok &= expect_int(slotLabel, got[i], want[i], anchor);
    }
    return ok;
}

static int test_source_evidence(
    const DM1_V1_ChestScrollWheelPickupDropSpecPc34* spec)
{
    const char* evidence =
        dm1_v1_chest_scroll_wheel_pickup_drop_source_evidence_pc34();
    int ok = 1;

    ok &= expect_contains("source F0333", evidence, "CHEST.C F0333:30-67",
                          spec->f0333OpenAnchor);
    ok &= expect_contains("source F0334", evidence, "CHEST.C F0334:113-132",
                          spec->f0334CloseAnchor);
    ok &= expect_contains("source F0302", evidence, "CHAMPION.C F0302:662-710",
                          spec->f0302DispatchAnchor);
    ok &= expect_contains("source F0297", evidence, "F0297:243-298",
                          spec->f0297PutAnchor);
    ok &= expect_contains("source F0298", evidence, "F0298:270-298",
                          spec->f0298RemoveAnchor);
    ok &= expect_contains("source PANEL", evidence,
                          "PANEL.C F0344:1895-1944",
                          spec->panelHighlightAnchor);
    ok &= expect_contains("source COMMAND", evidence,
                          "COMMAND.C F0359:1985-1990",
                          spec->commandDispatchAnchor);
    ok &= expect_contains("source MOUSE", evidence, "MOUSE.C F0077:97-126",
                          spec->mouseWheelAnchor);
    ok &= expect_contains("source OBJECT", evidence, "OBJECT.C F0033:147-212",
                          spec->objectIconAnchor);
    ok &= expect_contains("source BLITMASK", evidence,
                          "BLITMASK.C F0133:30-33",
                          spec->blitMaskAnchor);
    ok &= expect_contains("source pixel marker", evidence,
                          "no real-asset pixel parity claimed",
                          spec->pixelParityMarker);
    return ok;
}

static int test_spec(
    const DM1_V1_ChestScrollWheelPickupDropSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("contract only", spec->contractOnly, 1,
                     spec->f0333OpenAnchor);
    ok &= expect_int("C537 zone", spec->c537SlotZone,
                     DM1_PC34_CHEST_SCROLL_WHEEL_C537,
                     spec->f0333OpenAnchor);
    ok &= expect_int("C544 zone", spec->c544SlotZone,
                     DM1_PC34_CHEST_SCROLL_WHEEL_C544,
                     spec->f0333OpenAnchor);
    ok &= expect_int("C30 base slot", spec->c30BaseSlot,
                     DM1_PC34_CHEST_SCROLL_WHEEL_C30_BASE,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("thing none sentinel", spec->thingNone,
                     DM1_PC34_CHEST_SCROLL_WHEEL_NONE,
                     spec->f0334CloseAnchor);
    ok &= expect_int("end of list sentinel", spec->endOfList,
                     DM1_PC34_CHEST_SCROLL_WHEEL_END_OF_LIST,
                     spec->f0334CloseAnchor);
    ok &= expect_int("visible item count", spec->visibleItemCount, 4,
                     spec->f0333OpenAnchor);
    ok &= expect_int("highlight cycle count", spec->highlightCycleCount, 4,
                     spec->panelHighlightAnchor);
    ok &= expect_int("first empty slot index", spec->firstEmptySlotIndex, 4,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("leader item thing", spec->leaderHandItem.thing,
                     DM1_PC34_CHEST_SCROLL_WHEEL_LEADER_ITEM,
                     spec->f0297PutAnchor);
    ok &= expect_int("leader item weight", spec->leaderHandItem.weight,
                     DM1_PC34_CHEST_SCROLL_WHEEL_LEADER_ITEM_WEIGHT,
                     spec->f0297PutAnchor);
    ok &= expect_int("leader item icon", spec->leaderHandItem.icon,
                     DM1_PC34_CHEST_SCROLL_WHEEL_LEADER_ITEM & 0x00FF,
                     spec->objectIconAnchor);
    ok &= expect_contains("spec F0333 string", spec->f0333OpenAnchor,
                          "F0333:30-67", spec->f0333OpenAnchor);
    ok &= expect_contains("spec F0334 string", spec->f0334CloseAnchor,
                          "F0334:113-132", spec->f0334CloseAnchor);
    ok &= expect_contains("spec F0302 string", spec->f0302DispatchAnchor,
                          "F0302:662-710", spec->f0302DispatchAnchor);
    ok &= expect_contains("spec F0297 string", spec->f0297PutAnchor,
                          "F0297:243-298", spec->f0297PutAnchor);
    ok &= expect_contains("spec F0298 string", spec->f0298RemoveAnchor,
                          "F0298:270-298", spec->f0298RemoveAnchor);
    ok &= expect_contains("spec mouse string", spec->mouseWheelAnchor,
                          "F0078:128-168", spec->mouseWheelAnchor);
    ok &= expect_contains("spec panel string", spec->panelHighlightAnchor,
                          "F0345:1946-1999", spec->panelHighlightAnchor);
    ok &= expect_contains("spec pixel string", spec->pixelParityMarker,
                          "no real-asset pixel parity claimed",
                          spec->pixelParityMarker);
    return ok;
}

static int test_initial_state(
    const DM1_V1_ChestScrollWheelPickupDropProbePc34* probe,
    const DM1_V1_ChestScrollWheelPickupDropSpecPc34* spec)
{
    const int initialWant[DM1_PC34_CHEST_SCROLL_WHEEL_SLOT_COUNT] = {
        DM1_PC34_CHEST_SCROLL_WHEEL_ITEM0,
        DM1_PC34_CHEST_SCROLL_WHEEL_ITEM1,
        DM1_PC34_CHEST_SCROLL_WHEEL_ITEM2,
        DM1_PC34_CHEST_SCROLL_WHEEL_ITEM3,
        DM1_PC34_CHEST_SCROLL_WHEEL_NONE,
        DM1_PC34_CHEST_SCROLL_WHEEL_NONE,
        DM1_PC34_CHEST_SCROLL_WHEEL_NONE,
        DM1_PC34_CHEST_SCROLL_WHEEL_NONE
    };
    const int iconWant[DM1_PC34_CHEST_SCROLL_WHEEL_SLOT_COUNT] = {
        DM1_PC34_CHEST_SCROLL_WHEEL_ITEM0 & 0x00FF,
        DM1_PC34_CHEST_SCROLL_WHEEL_ITEM1 & 0x00FF,
        DM1_PC34_CHEST_SCROLL_WHEEL_ITEM2 & 0x00FF,
        DM1_PC34_CHEST_SCROLL_WHEEL_ITEM3 & 0x00FF,
        DM1_PC34_CHEST_SCROLL_WHEEL_NONE,
        DM1_PC34_CHEST_SCROLL_WHEEL_NONE,
        DM1_PC34_CHEST_SCROLL_WHEEL_NONE,
        DM1_PC34_CHEST_SCROLL_WHEEL_NONE
    };
    int i;
    int ok = 1;

    ok &= expect_int("open chest thing", probe->openChestThing,
                     DM1_PC34_CHEST_SCROLL_WHEEL_OPEN_CHEST,
                     spec->f0333OpenAnchor);
    ok &= expect_slot_array("initial G0425", probe->initialSlots,
                            initialWant, spec->f0333OpenAnchor);
    ok &= expect_slot_array("initial F0033 icons", probe->initialIcons,
                            iconWant, spec->objectIconAnchor);
    ok &= expect_int("initial leader hand", probe->initialLeaderHand,
                     DM1_PC34_CHEST_SCROLL_WHEEL_LEADER_ITEM,
                     spec->f0297PutAnchor);
    ok &= expect_int("initial leader icon", probe->initialLeaderIcon,
                     DM1_PC34_CHEST_SCROLL_WHEEL_LEADER_ITEM & 0x00FF,
                     spec->objectIconAnchor);
    ok &= expect_int("initial leader load", probe->initialLeaderLoad,
                     DM1_PC34_CHEST_SCROLL_WHEEL_BASE_LOAD +
                         DM1_PC34_CHEST_SCROLL_WHEEL_LEADER_ITEM_WEIGHT,
                     spec->f0297PutAnchor);
    ok &= expect_int("initial leader load mask",
                     probe->initialLeaderAttributes,
                     DM1_PC34_CHEST_SCROLL_WHEEL_LOAD_MASK,
                     spec->f0297PutAnchor);
    ok &= expect_int("materialized visible count",
                     probe->materializedVisibleCount, 4,
                     spec->f0333OpenAnchor);
    ok &= expect_int("first empty before drop",
                     probe->firstEmptySlotBeforeDrop, 4,
                     spec->f0333OpenAnchor);
    for (i = 0; i < DM1_PC34_CHEST_SCROLL_WHEEL_SLOT_COUNT; ++i) {
        char label[96];

        snprintf(label, sizeof(label), "slot zone %d", i);
        ok &= expect_int(label, probe->slotZones[i],
                         DM1_PC34_CHEST_SCROLL_WHEEL_C537 + i,
                         spec->f0333OpenAnchor);
    }
    return ok;
}

static int test_highlight_cycles(
    const DM1_V1_ChestScrollWheelPickupDropProbePc34* probe,
    const DM1_V1_ChestScrollWheelPickupDropSpecPc34* spec)
{
    const int highlightWant[DM1_PC34_CHEST_SCROLL_WHEEL_HIGHLIGHT_STEPS] = {
        0, 1, 2, 3, 0
    };
    const int unhighlightWant[DM1_PC34_CHEST_SCROLL_WHEEL_HIGHLIGHT_STEPS] = {
        -1, 0, 1, 2, 3
    };
    int cycle;
    int step;
    int ok = 1;

    for (cycle = 0; cycle < DM1_PC34_CHEST_SCROLL_WHEEL_HIGHLIGHT_CYCLES;
         ++cycle) {
        char label[128];

        for (step = 0; step < DM1_PC34_CHEST_SCROLL_WHEEL_HIGHLIGHT_STEPS;
             ++step) {
            snprintf(label, sizeof(label), "cycle %d highlight step %d",
                     cycle, step);
            ok &= expect_int(label, probe->highlightTrace[cycle][step],
                             highlightWant[step],
                             spec->panelHighlightAnchor);
            snprintf(label, sizeof(label), "cycle %d unhighlight step %d",
                     cycle, step);
            ok &= expect_int(label, probe->unhighlightTrace[cycle][step],
                             unhighlightWant[step],
                             spec->blitMaskAnchor);
        }
        snprintf(label, sizeof(label), "cycle %d leader hand stable", cycle);
        ok &= expect_int(label, probe->highlightLeaderHandStable[cycle], 1,
                         spec->f0297PutAnchor);
        snprintf(label, sizeof(label), "cycle %d slot4 never highlighted",
                 cycle);
        ok &= expect_int(label, probe->highlightSlot4NeverSelected[cycle], 1,
                         spec->panelHighlightAnchor);
        snprintf(label, sizeof(label), "cycle %d slots stable", cycle);
        ok &= expect_int(label, probe->highlightSlotsStable[cycle], 1,
                         spec->f0333OpenAnchor);
    }
    ok &= expect_int("wheel queue event count",
                     probe->highlightPointerQueueEvents,
                     DM1_PC34_CHEST_SCROLL_WHEEL_HIGHLIGHT_CYCLES *
                         DM1_PC34_CHEST_SCROLL_WHEEL_HIGHLIGHT_STEPS,
                     spec->mouseWheelAnchor);
    ok &= expect_int("partial mask dispatch count",
                     probe->highlightPartialMaskDispatches,
                     DM1_PC34_CHEST_SCROLL_WHEEL_HIGHLIGHT_CYCLES *
                         DM1_PC34_CHEST_SCROLL_WHEEL_HIGHLIGHT_STEPS,
                     spec->blitMaskAnchor);
    return ok;
}

static int test_drop_and_close(
    const DM1_V1_ChestScrollWheelPickupDropProbePc34* probe,
    const DM1_V1_ChestScrollWheelPickupDropSpecPc34* spec)
{
    const int closedWant[DM1_PC34_CHEST_SCROLL_WHEEL_SLOT_COUNT] = {
        DM1_PC34_CHEST_SCROLL_WHEEL_ITEM0,
        DM1_PC34_CHEST_SCROLL_WHEEL_ITEM1,
        DM1_PC34_CHEST_SCROLL_WHEEL_ITEM2,
        DM1_PC34_CHEST_SCROLL_WHEEL_ITEM3,
        DM1_PC34_CHEST_SCROLL_WHEEL_LEADER_ITEM,
        DM1_PC34_CHEST_SCROLL_WHEEL_NONE,
        DM1_PC34_CHEST_SCROLL_WHEEL_NONE,
        DM1_PC34_CHEST_SCROLL_WHEEL_NONE
    };
    int ok = 1;

    ok &= expect_int("drop command slot box", probe->dropCommandSlotBox,
                     DM1_PC34_CHEST_SCROLL_WHEEL_C30_BASE + 4,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("drop first empty", probe->dropFirstEmptySlot, 4,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("drop resolved chest slot",
                     probe->dropResolvedChestSlot, 4,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("drop result", probe->dropResult, 1,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("slot4 after drop", probe->slot4AfterDrop,
                     DM1_PC34_CHEST_SCROLL_WHEEL_LEADER_ITEM,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("leader hand after drop", probe->leaderHandAfterDrop,
                     DM1_PC34_CHEST_SCROLL_WHEEL_NONE,
                     spec->f0298RemoveAnchor);
    ok &= expect_int("leader load after drop", probe->leaderLoadAfterDrop,
                     DM1_PC34_CHEST_SCROLL_WHEEL_BASE_LOAD,
                     spec->f0298RemoveAnchor);
    ok &= expect_int("leader load mask after drop",
                     probe->leaderAttributesAfterDrop,
                     DM1_PC34_CHEST_SCROLL_WHEEL_LOAD_MASK,
                     spec->f0298RemoveAnchor);
    ok &= expect_int("leader identity preserved in chest",
                     probe->leaderIdentityPreservedInChest, 1,
                     spec->f0297PutAnchor);
    ok &= expect_int("screen update balanced after drop",
                     probe->screenUpdateBalancedAfterDrop, 1,
                     spec->mouseWheelAnchor);
    ok &= expect_int("close count", probe->closeCount, 5,
                     spec->f0334CloseAnchor);
    ok &= expect_int("close head", probe->closedHead,
                     DM1_PC34_CHEST_SCROLL_WHEEL_ITEM0,
                     spec->f0334CloseAnchor);
    ok &= expect_int("close end sentinel", probe->closeEndSentinel,
                     DM1_PC34_CHEST_SCROLL_WHEEL_END_OF_LIST,
                     spec->f0334CloseAnchor);
    ok &= expect_slot_array("closed list", probe->closedSlots, closedWant,
                            spec->f0334CloseAnchor);
    ok &= expect_int("close visible order stable",
                     probe->closeVisibleOrderStable, 1,
                     spec->f0334CloseAnchor);
    ok &= expect_int("close drop item appended",
                     probe->closeDropItemAppended, 1,
                     spec->f0334CloseAnchor);
    ok &= expect_int("G0425 cleared after close",
                     probe->g0425ClearedAfterClose, 1,
                     spec->f0334CloseAnchor);
    ok &= expect_int("no-claim real-asset pixel parity",
                     probe->noClaimPixelParity, 1,
                     spec->pixelParityMarker);
    return ok;
}

int main(void)
{
    DM1_V1_ChestScrollWheelPickupDropProbePc34 probe;
    const DM1_V1_ChestScrollWheelPickupDropSpecPc34* spec =
        dm1_v1_chest_scroll_wheel_pickup_drop_spec_pc34();
    int ok = 1;

    ok &= expect_int("probe run",
                     dm1_v1_chest_scroll_wheel_pickup_drop_pc34(&probe), 1,
                     spec->f0333OpenAnchor);
    ok &= test_source_evidence(spec);
    ok &= test_spec(spec);
    ok &= test_initial_state(&probe, spec);
    ok &= test_highlight_cycles(&probe, spec);
    ok &= test_drop_and_close(&probe, spec);

    printf("SUMMARY assertions=%d failures=%d\n", g_assertions, g_failures);
    return (ok && g_failures == 0) ? 0 : 1;
}
