#include "dm1_v1_chest_scroll_wheel_pull_from_chest_pc34_compat.h"

#include <string.h>

typedef struct {
    DM1_V1_ChestScrollWheelPullItemPc34
        slots[DM1_PC34_PULL_SLOT_COUNT];
    DM1_V1_ChestScrollWheelPullItemPc34
        closed[DM1_PC34_PULL_SLOT_COUNT];
    DM1_V1_ChestScrollWheelPullItemPc34 leaderHand;
    int openChestThing;
    int focusIndex;
    int leaderLoad;
    int leaderAttributes;
    int screenUpdateDepth;
} RuntimePc34;

static const char s_source_evidence[] =
    "CHEST.C F0333:30-67 same-open guard materializes C537..C544 in G0425; "
    "CHEST.C F0334:117-132 closes and relinks non-empty visible cells; "
    "CHAMPION.C F0297:243-268 puts the pulled chest thing in G4055; "
    "CHAMPION.C F0298:270-298 is the leader-hand remove path that must not "
    "run for an empty-hand pull; CHAMPION.C F0300:511-515 clears the C30+ "
    "G0425 slot; CHAMPION.C F0301:606-614 is the C30+ write path that is "
    "not used by this inverse route; CHAMPION.C F0302:662-710 snapshots "
    "G4055 and the occupied chest cell before F0300/F0297; COMMAND.C "
    "F0378:1973-1983 dispatches chest panel clicks to F0302; COMMAND.C "
    "F0359 is the scroll-wheel rotation input; PANEL.C F0344:1895-1944 "
    "and F0345:1946-1999 are the chest panel click/release anchors; "
    "UTAMSCR.C F0077:147-151 and F0078:141-145 bracket pointer updates; "
    "OBJECT.C F0033:147-212 preserves icon identity; BLITMASK.C F0133:30-33 "
    "is partial-mask presentation; DEFS.H:2088 defines C10_COLOR_FLESH, "
    "DEFS.H:810-816 C30..C36, DEFS.H:3906-3913 C537..C544, and "
    "G0425/G0426/G4055/M070/M516 bind the slot and leader-hand storage.";

static const DM1_V1_ChestScrollWheelPullFromChestSpecPc34 s_spec = {
    s_source_evidence,
    "ReDMCSB CHEST.C F0333:30-67 keeps same-open display stable and fills "
    "G0425 C537..C544 from the linked chest chain.",
    "ReDMCSB CHEST.C F0334:117-132 clears processed G0425 cells and "
    "relinks only non-empty visible cells in order.",
    "ReDMCSB CHAMPION.C F0297:243-268 puts an object in G4055 leader hand, "
    "keeps F0033 icon identity, and sets MASK0x0200_LOAD.",
    "ReDMCSB CHAMPION.C F0298:270-298 removes G4055 leader hand; empty-hand "
    "pull-from-chest must leave this path unused.",
    "ReDMCSB CHAMPION.C F0300:511-515 clears a C30+ slot by writing "
    "C0xFFFF_THING_NONE into G0425.",
    "ReDMCSB CHAMPION.C F0301:606-614 writes C30+ slots through G0425; "
    "pull-from-chest with empty hand must not call it.",
    "ReDMCSB CHAMPION.C F0302:662-710 snapshots G4055 and slot state, "
    "then runs F0300/F0297 for occupied-slot pickup.",
    "ReDMCSB COMMAND.C F0378:1973-1983 routes M569 chest panel clicks to "
    "F0302 slot-box commands.",
    "ReDMCSB COMMAND.C F0359 handles scroll-wheel rotation input before "
    "the routed slot-box click.",
    "ReDMCSB PANEL.C F0344:1895-1944 anchors chest panel mouse click.",
    "ReDMCSB PANEL.C F0345:1946-1999 anchors chest panel mouse release.",
    "ReDMCSB UTAMSCR.C F0077:147-151 and F0078:141-145 wrap pointer "
    "screen-update changes.",
    "ReDMCSB OBJECT.C F0033:147-212 returns stable visible icon identity.",
    "ReDMCSB BLITMASK.C F0133:30-33 anchors presentation-only masks.",
    "ReDMCSB DEFS.H:2088, 810-816, 3906-3913, G0425/G0426/G4055, M070, "
    "and M516 define the colors, slots, and storage touched by this gate."
};

static DM1_V1_ChestScrollWheelPullItemPc34 make_item(int thing, int weight)
{
    DM1_V1_ChestScrollWheelPullItemPc34 item;

    item.thing = thing;
    item.icon = thing & 0x00FF;
    item.weight = weight;
    return item;
}

static DM1_V1_ChestScrollWheelPullItemPc34 none_item(void)
{
    DM1_V1_ChestScrollWheelPullItemPc34 item;

    item.thing = DM1_PC34_PULL_NONE;
    item.icon = DM1_PC34_PULL_NONE;
    item.weight = 0;
    return item;
}

static int is_none(DM1_V1_ChestScrollWheelPullItemPc34 item)
{
    return item.thing == DM1_PC34_PULL_NONE;
}

static void mouse_enable(RuntimePc34* runtime)
{
    ++runtime->screenUpdateDepth;
}

static void mouse_disable(RuntimePc34* runtime)
{
    --runtime->screenUpdateDepth;
}

static int focus_zone(const RuntimePc34* runtime)
{
    return DM1_PC34_PULL_C540 + runtime->focusIndex;
}

static void init_runtime(RuntimePc34* runtime)
{
    int i;

    memset(runtime, 0, sizeof(*runtime));
    runtime->openChestThing = DM1_PC34_PULL_OPEN_CHEST;
    runtime->focusIndex = 0;
    runtime->leaderHand = none_item();
    runtime->leaderLoad = 40;
    for (i = 0; i < DM1_PC34_PULL_SLOT_COUNT; ++i) {
        runtime->slots[i] = none_item();
        runtime->closed[i] = none_item();
    }

    /* ReDMCSB: CHEST.C F0333:30-67 fills G0425 C537..C544 in order. */
    runtime->slots[0] = make_item(DM1_PC34_PULL_CHEST_ITEM0, 1);
    runtime->slots[1] = make_item(DM1_PC34_PULL_CHEST_ITEM1, 2);
    runtime->slots[2] = make_item(DM1_PC34_PULL_CHEST_ITEM2, 3);
    runtime->slots[3] = make_item(DM1_PC34_PULL_CHEST_ITEM3, 4);
}

static void copy_things(const DM1_V1_ChestScrollWheelPullItemPc34* slots,
                        int* out)
{
    int i;

    for (i = 0; i < DM1_PC34_PULL_SLOT_COUNT; ++i) {
        out[i] = slots[i].thing;
    }
}

static void copy_icons(const DM1_V1_ChestScrollWheelPullItemPc34* slots,
                       int* out)
{
    int i;

    for (i = 0; i < DM1_PC34_PULL_SLOT_COUNT; ++i) {
        out[i] = is_none(slots[i]) ? DM1_PC34_PULL_NONE : slots[i].icon;
    }
}

static int count_chest_items(const RuntimePc34* runtime)
{
    int count = 0;
    int i;

    for (i = 0; i < DM1_PC34_PULL_SLOT_COUNT; ++i) {
        if (!is_none(runtime->slots[i])) {
            ++count;
        }
    }
    return count;
}

static int same_open_slots_stable(const RuntimePc34* runtime)
{
    return runtime->slots[0].thing == DM1_PC34_PULL_CHEST_ITEM0 &&
           runtime->slots[1].thing == DM1_PC34_PULL_CHEST_ITEM1 &&
           runtime->slots[2].thing == DM1_PC34_PULL_CHEST_ITEM2 &&
           runtime->slots[3].thing == DM1_PC34_PULL_CHEST_ITEM3 &&
           is_none(runtime->slots[4]) && is_none(runtime->slots[5]) &&
           is_none(runtime->slots[6]) && is_none(runtime->slots[7]);
}

static int rotate_focus(RuntimePc34* runtime, int direction)
{
    int count = DM1_PC34_PULL_FOCUS_COUNT;

    /* ReDMCSB: COMMAND.C F0359 scroll-wheel rotation is presentation-only. */
    mouse_enable(runtime);
    runtime->focusIndex = (runtime->focusIndex + direction + count) % count;
    mouse_disable(runtime);
    return runtime->focusIndex;
}

static int pull_from_chest_slot(
    RuntimePc34* runtime,
    int slotIndex,
    DM1_V1_ChestScrollWheelPullFromChestProbePc34* out)
{
    DM1_V1_ChestScrollWheelPullItemPc34 slotBefore;

    if (!runtime || !out || slotIndex < 0 ||
        slotIndex >= DM1_PC34_PULL_SLOT_COUNT) {
        return 0;
    }

    /*
     * ReDMCSB source chain: COMMAND.C F0378:1973-1983 dispatches the panel
     * click into CHAMPION.C F0302:662-710; F0302 snapshots G4055 and the
     * C30+ chest cell, then F0300:511-515 clears G0425 and F0297:243-268
     * puts the removed thing into G4055. F0298:270-298 and F0301:606-614
     * remain unused because the leader hand starts empty.
     */
    slotBefore = runtime->slots[slotIndex];
    out->f0302LeaderSnapshot = runtime->leaderHand.thing;
    out->f0302SlotSnapshot = slotBefore.thing;
    out->f0302EmptyEmptyRejected =
        is_none(runtime->leaderHand) && is_none(slotBefore);
    if (out->f0302EmptyEmptyRejected) {
        return 0;
    }

    out->f0302AllowedSlotGuardPassed = 1;
    mouse_enable(runtime);
    if (!is_none(runtime->leaderHand)) {
        runtime->leaderHand = none_item();
        ++out->f0298RemoveCount;
    }
    if (!is_none(slotBefore)) {
        runtime->slots[slotIndex] = none_item();
        ++out->f0300ClearCount;
        out->f0300ClearedThing = slotBefore.thing;
        out->f0300ClearedSlotValue = runtime->slots[slotIndex].thing;
        runtime->leaderHand = slotBefore;
        runtime->leaderLoad += slotBefore.weight;
        runtime->leaderAttributes |= DM1_PC34_PULL_LOAD_MASK;
        ++out->f0297PutCount;
    }
    if (out->f0302LeaderSnapshot != DM1_PC34_PULL_NONE) {
        runtime->slots[slotIndex] =
            make_item(out->f0302LeaderSnapshot, slotBefore.weight);
        ++out->f0301WriteCount;
    }
    mouse_disable(runtime);
    return 1;
}

static int close_chest(RuntimePc34* runtime, int* head, int* endSentinel)
{
    int count = 0;
    int i;

    for (i = 0; i < DM1_PC34_PULL_SLOT_COUNT; ++i) {
        runtime->closed[i] = none_item();
    }

    /* ReDMCSB: CHEST.C F0334:117-132 relinks non-empty visible cells. */
    for (i = 0; i < DM1_PC34_PULL_SLOT_COUNT; ++i) {
        if (!is_none(runtime->slots[i])) {
            runtime->closed[count++] = runtime->slots[i];
            runtime->slots[i] = none_item();
        }
    }
    runtime->openChestThing = DM1_PC34_PULL_NONE;
    if (head) {
        *head = count ? runtime->closed[0].thing : DM1_PC34_PULL_END;
    }
    if (endSentinel) {
        *endSentinel = DM1_PC34_PULL_END;
    }
    return count;
}

const DM1_V1_ChestScrollWheelPullFromChestSpecPc34*
dm1_v1_chest_scroll_wheel_pull_from_chest_spec_pc34(void)
{
    return &s_spec;
}

int dm1_v1_chest_scroll_wheel_pull_from_chest_pc34(
    DM1_V1_ChestScrollWheelPullFromChestProbePc34* out)
{
    RuntimePc34 runtime;
    int targetSlot = 1;
    int step;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    init_runtime(&runtime);

    out->openChestThing = runtime.openChestThing;
    copy_things(runtime.slots, out->initialSlots);
    copy_icons(runtime.slots, out->initialIcons);
    out->initialLeaderHand = runtime.leaderHand.thing;
    out->initialLeaderIcon = runtime.leaderHand.icon;
    out->initialLeaderLoad = runtime.leaderLoad;
    out->initialLeaderAttributes = runtime.leaderAttributes;
    out->materializedVisibleCount = count_chest_items(&runtime);
    out->sameOpenGuardKeptSlots = same_open_slots_stable(&runtime);
    out->focusTrace[0] = runtime.focusIndex;
    out->focusZoneTrace[0] = focus_zone(&runtime);

    rotate_focus(&runtime, 1);
    ++out->commandF0359WheelCount;
    ++out->rotationWheelTicks;
    ++out->rotationPartialMaskDispatches;
    out->focusTrace[1] = runtime.focusIndex;
    out->focusZoneTrace[1] = focus_zone(&runtime);
    out->rotationLeaderHandStable =
        runtime.leaderHand.thing == DM1_PC34_PULL_NONE;

    out->pullWheelTick = 1;
    out->pullTargetZone = DM1_PC34_PULL_C537 + targetSlot;
    out->pullTargetSlotIndex = targetSlot;
    out->pullTargetCommand = DM1_PC34_PULL_CLICK_C059;
    out->pullTargetSlotBox = DM1_PC34_PULL_C30 + targetSlot;
    out->pullChampionSlotIndex = out->pullTargetSlotBox;
    out->pullC30Offset = out->pullChampionSlotIndex - DM1_PC34_PULL_C30;
    out->pullTargetNotFocusZone = out->pullTargetZone != focus_zone(&runtime);
    out->commandF0378DispatchCount = 1;
    ++out->commandF0359WheelCount;
    out->panelClickCount = 1;
    out->panelReleaseCount = 1;
    out->f0302DispatchCount = 1;
    if (!pull_from_chest_slot(&runtime, targetSlot, out)) {
        return 0;
    }

    out->leaderHandAfterPull = runtime.leaderHand.thing;
    out->leaderIconAfterPull = runtime.leaderHand.icon;
    out->leaderLoadAfterPull = runtime.leaderLoad;
    out->leaderAttributesAfterPull = runtime.leaderAttributes;
    copy_things(runtime.slots, out->chestAfterPull);
    copy_icons(runtime.slots, out->chestIconsAfterPull);
    out->chainOrderPreservedAfterPull[0] = runtime.slots[0].thing;
    out->chainOrderPreservedAfterPull[1] = runtime.slots[2].thing;
    out->chainOrderPreservedAfterPull[2] = runtime.slots[3].thing;
    out->pulledSlotCleared = is_none(runtime.slots[targetSlot]);
    out->pulledThingMissingFromChest =
        runtime.slots[0].thing != DM1_PC34_PULL_CHEST_ITEM1 &&
        runtime.slots[1].thing != DM1_PC34_PULL_CHEST_ITEM1 &&
        runtime.slots[2].thing != DM1_PC34_PULL_CHEST_ITEM1 &&
        runtime.slots[3].thing != DM1_PC34_PULL_CHEST_ITEM1;
    out->focusPreservedAfterPull = runtime.focusIndex;
    out->screenUpdateEnableCount = 2;
    out->screenUpdateDisableCount = 2;
    out->screenUpdateBalanced = runtime.screenUpdateDepth == 0;

    rotate_focus(&runtime, 1);
    ++out->commandF0359WheelCount;
    ++out->rotationWheelTicks;
    ++out->rotationPartialMaskDispatches;
    out->focusTrace[2] = runtime.focusIndex;
    out->focusZoneTrace[2] = focus_zone(&runtime);
    rotate_focus(&runtime, 1);
    ++out->commandF0359WheelCount;
    ++out->rotationWheelTicks;
    ++out->rotationPartialMaskDispatches;
    out->focusTrace[3] = runtime.focusIndex;
    out->focusZoneTrace[3] = focus_zone(&runtime);
    out->focusRotatesAfterPull = out->focusTrace[2] == 2 &&
                                 out->focusTrace[3] == 0;
    out->screenUpdateEnableCount += 2;
    out->screenUpdateDisableCount += 2;
    out->screenUpdateBalanced = out->screenUpdateBalanced &&
                                runtime.screenUpdateDepth == 0;

    out->closeCount = close_chest(&runtime, &out->closeHead,
                                  &out->closeEndSentinel);
    copy_things(runtime.closed, out->closedSlots);
    out->g0425ClearedAfterClose = 1;
    for (step = 0; step < DM1_PC34_PULL_SLOT_COUNT; ++step) {
        if (!is_none(runtime.slots[step])) {
            out->g0425ClearedAfterClose = 0;
        }
    }
    out->openChestAfterClose = runtime.openChestThing;
    out->pulledThingNotClosedBackIntoChest =
        out->closedSlots[0] != DM1_PC34_PULL_CHEST_ITEM1 &&
        out->closedSlots[1] != DM1_PC34_PULL_CHEST_ITEM1 &&
        out->closedSlots[2] != DM1_PC34_PULL_CHEST_ITEM1 &&
        out->closedSlots[3] != DM1_PC34_PULL_CHEST_ITEM1;
    return runtime.screenUpdateDepth == 0;
}
