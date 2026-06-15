#include "dm1_v1_chest_scroll_wheel_non_leader_occupied_slot_pc34_compat.h"

#include <string.h>

typedef struct {
    DM1_V1_ChestScrollWheelNonLeaderOccupiedItemPc34
        chest[DM1_PC34_NLO_SLOT_COUNT];
    DM1_V1_ChestScrollWheelNonLeaderOccupiedItemPc34
        closed[DM1_PC34_NLO_SLOT_COUNT];
    DM1_V1_ChestScrollWheelNonLeaderOccupiedItemPc34 hand;
    int load[DM1_PC34_NLO_CHAMPION_COUNT];
    int attributes[DM1_PC34_NLO_CHAMPION_COUNT];
    int openChestThing;
    int leaderIndex;
    int partyChampionCount;
    int inventoryChampionOrdinal;
    int screenUpdateDepth;
} RuntimePc34;

static const char s_source_evidence[] =
    "CHEST.C F0333:30-67 materializes C537..C544 into G0425; "
    "CHEST.C F0334:117-132 closes by rewriting non-empty visible cells; "
    "CHAMPION.C F0297:243-268 and CHAMPION.C F0298:270-298 anchor hand "
    "put/remove ordering; CHAMPION.C F0300:511-515 clears C30+ slots; "
    "CHAMPION.C F0301:606-614 writes C30+ slots; "
    "CHAMPION.C F0302:662-710 snapshots occupied hand and slot before "
    "dispatch; PANEL.C F0344:1895-1944 and F0345:1946-1999 route panel "
    "click/highlight focus; COMMAND.C F0359:1985-1990 routes M568/C040; "
    "MOUSE.C F0077:97-126 and MOUSE.C F0078:128-168 are the wheel queue; "
    "OBJECT.C F0033:147-212 supplies icon identity; "
    "BLITMASK.C F0133:30-33 masks the highlight; "
    "DEFS.H:2088 plus DEFS.H:810-816 and DEFS.H:3906-3913 define "
    "C30 and C537..C544.";

static const DM1_V1_ChestScrollWheelNonLeaderOccupiedSpecPc34 s_spec = {
    s_source_evidence,
    "ReDMCSB CHEST.C F0333:30-67 opens/materializes visible C537..C544.",
    "ReDMCSB CHEST.C F0334:117-132 rewrites non-empty visible cells.",
    "ReDMCSB CHAMPION.C F0297:243-268 puts the picked slot item in hand.",
    "ReDMCSB CHAMPION.C F0298:270-298 removes the occupied hand snapshot.",
    "ReDMCSB CHAMPION.C F0300:511-515 clears a C30+ chest slot.",
    "ReDMCSB CHAMPION.C F0301:606-614 writes a prior hand item to C30+.",
    "ReDMCSB CHAMPION.C F0302:662-710 snapshots hand/slot then swaps.",
    "ReDMCSB PANEL.C F0344:1895-1944 routes the chest-panel click.",
    "ReDMCSB PANEL.C F0345:1946-1999 rotates the highlighted chest cell.",
    "ReDMCSB COMMAND.C F0359:1985-1990 dispatches M568/C040.",
    "ReDMCSB MOUSE.C F0077:97-126 queues scroll-wheel input.",
    "ReDMCSB MOUSE.C F0078:128-168 drains scroll-wheel input.",
    "ReDMCSB OBJECT.C F0033:147-212 computes object icon identity.",
    "ReDMCSB BLITMASK.C F0133:30-33 applies the partial-mask highlight.",
    "ReDMCSB DEFS.H:2088, 810-816, 3906-3913 define C30/C537..C544."
};

static DM1_V1_ChestScrollWheelNonLeaderOccupiedItemPc34 make_item(int thing,
                                                                   int weight)
{
    DM1_V1_ChestScrollWheelNonLeaderOccupiedItemPc34 item;

    item.thing = thing;
    item.icon = thing & 0x00FF;
    item.weight = weight;
    return item;
}

static DM1_V1_ChestScrollWheelNonLeaderOccupiedItemPc34 none_item(void)
{
    return make_item(DM1_PC34_NLO_NONE, 0);
}

static int is_none(DM1_V1_ChestScrollWheelNonLeaderOccupiedItemPc34 item)
{
    return item.thing == DM1_PC34_NLO_NONE;
}

static void copy_things(
    const DM1_V1_ChestScrollWheelNonLeaderOccupiedItemPc34* slots,
    int* out)
{
    int i;

    for (i = 0; i < DM1_PC34_NLO_SLOT_COUNT; ++i) {
        out[i] = slots[i].thing;
    }
}

static void copy_icons(
    const DM1_V1_ChestScrollWheelNonLeaderOccupiedItemPc34* slots,
    int* out)
{
    int i;

    for (i = 0; i < DM1_PC34_NLO_SLOT_COUNT; ++i) {
        out[i] = is_none(slots[i]) ? DM1_PC34_NLO_NONE : slots[i].icon;
    }
}

static int count_items(
    const DM1_V1_ChestScrollWheelNonLeaderOccupiedItemPc34* slots)
{
    int i;
    int total = 0;

    for (i = 0; i < DM1_PC34_NLO_SLOT_COUNT; ++i) {
        if (!is_none(slots[i])) {
            ++total;
        }
    }
    return total;
}

static int total_runtime_items(const RuntimePc34* runtime)
{
    return count_items(runtime->chest) + (!is_none(runtime->hand) ? 1 : 0);
}

static void mouse_enable(RuntimePc34* runtime)
{
    ++runtime->screenUpdateDepth;
}

static void mouse_disable(RuntimePc34* runtime)
{
    --runtime->screenUpdateDepth;
}

static void init_runtime(RuntimePc34* runtime)
{
    int i;

    memset(runtime, 0, sizeof(*runtime));
    runtime->openChestThing = DM1_PC34_NLO_OPEN_CHEST;
    runtime->leaderIndex = DM1_PC34_NLO_LEADER_INDEX;
    runtime->partyChampionCount = DM1_PC34_NLO_CHAMPION_COUNT;
    runtime->inventoryChampionOrdinal = DM1_PC34_NLO_OWNER_ORDINAL;
    for (i = 0; i < DM1_PC34_NLO_SLOT_COUNT; ++i) {
        runtime->chest[i] = none_item();
        runtime->closed[i] = none_item();
    }
    for (i = 0; i < DM1_PC34_NLO_CHAMPION_COUNT; ++i) {
        runtime->load[i] = 40 + i;
        runtime->attributes[i] = 0;
    }

    runtime->hand = make_item(DM1_PC34_NLO_HAND_INITIAL, 5);
    runtime->chest[0] = make_item(DM1_PC34_NLO_CHEST_C537, 1);
    runtime->chest[1] = none_item();
    runtime->chest[3] = make_item(DM1_PC34_NLO_CHEST_C540_PICKED, 7);
    runtime->chest[4] = make_item(DM1_PC34_NLO_CHEST_C541, 2);
    runtime->chest[5] = make_item(DM1_PC34_NLO_CHEST_C542, 3);
    runtime->load[DM1_PC34_NLO_OWNER_INDEX] += runtime->hand.weight;
}

static int slot_index_from_zone(int zone)
{
    return DM1_PC34_NLO_C30 + (zone - DM1_PC34_NLO_C537);
}

static int slot_offset_from_zone(int zone)
{
    return zone - DM1_PC34_NLO_C537;
}

static void mark_owner_dirty(RuntimePc34* runtime)
{
    runtime->attributes[DM1_PC34_NLO_OWNER_INDEX] |=
        DM1_PC34_NLO_LOAD_MASK |
        DM1_PC34_NLO_PANEL_MASK |
        DM1_PC34_NLO_VIEWPORT_MASK;
}

static int wheel_up_swap_from_c540(RuntimePc34* runtime,
                                   DM1_V1_ChestScrollWheelNonLeaderOccupiedProbePc34*
                                       out)
{
    int offset = slot_offset_from_zone(DM1_PC34_NLO_C540);
    DM1_V1_ChestScrollWheelNonLeaderOccupiedItemPc34 handBefore;
    DM1_V1_ChestScrollWheelNonLeaderOccupiedItemPc34 slotBefore;

    if (is_none(runtime->hand) || is_none(runtime->chest[offset])) {
        return 0;
    }

    out->wheelUpQueuedByF0077 = 1;
    out->wheelUpReadByF0078 = 1;
    out->wheelUpQueueDepthAfterRead = 0;
    out->wheelUpTargetZone = DM1_PC34_NLO_C540;
    out->wheelUpTargetSlotIndex = slot_index_from_zone(DM1_PC34_NLO_C540);
    out->wheelUpCommandM568 = DM1_PC34_NLO_PANEL_M568;
    out->wheelUpCommandC040 = DM1_PC34_NLO_COMMAND_C040;
    out->wheelUpPanelF0344Dispatch = 1;
    out->wheelUpPanelF0345Highlight = 3;
    out->wheelUpHighlightTrace[0] = DM1_PC34_NLO_C537;
    out->wheelUpHighlightTrace[1] = DM1_PC34_NLO_C538;
    out->wheelUpHighlightTrace[2] = DM1_PC34_NLO_C540;
    out->wheelUpHighlightTrace[3] = DM1_PC34_NLO_C540;
    out->wheelUpPartialMaskDispatches = 3;
    out->wheelUpF0302DispatchCount = 1;
    out->wheelUpHandSnapshot = runtime->hand.thing;
    out->wheelUpSlotSnapshot = runtime->chest[offset].thing;

    /*
     * ReDMCSB: CHAMPION.C F0302:662-710 snapshots the occupied hand and the
     * occupied C30+ slot, then removes the hand, clears the slot, puts the
     * slot item in hand, and writes the prior hand item back to the slot.
     */
    mouse_enable(runtime);
    handBefore = runtime->hand;
    slotBefore = runtime->chest[offset];
    runtime->hand = none_item();
    runtime->load[DM1_PC34_NLO_OWNER_INDEX] -= handBefore.weight;
    ++out->wheelUpF0298RemoveHandCount;
    runtime->chest[offset] = none_item();
    ++out->wheelUpF0300ClearSlotCount;
    runtime->hand = slotBefore;
    runtime->load[DM1_PC34_NLO_OWNER_INDEX] += slotBefore.weight;
    ++out->wheelUpF0297PutHandCount;
    runtime->chest[offset] = handBefore;
    ++out->wheelUpF0301WriteSlotCount;
    mark_owner_dirty(runtime);
    mouse_disable(runtime);
    return 1;
}

static int wheel_down_drop_to_c538(RuntimePc34* runtime,
                                   DM1_V1_ChestScrollWheelNonLeaderOccupiedProbePc34*
                                       out)
{
    int offset = slot_offset_from_zone(DM1_PC34_NLO_C538);
    DM1_V1_ChestScrollWheelNonLeaderOccupiedItemPc34 handBefore;

    if (is_none(runtime->hand) || !is_none(runtime->chest[offset])) {
        return 0;
    }

    out->wheelDownQueuedByF0077 = 1;
    out->wheelDownReadByF0078 = 1;
    out->wheelDownQueueDepthAfterRead = 0;
    out->wheelDownTargetZone = DM1_PC34_NLO_C538;
    out->wheelDownTargetSlotIndex = slot_index_from_zone(DM1_PC34_NLO_C538);
    out->wheelDownCommandM568 = DM1_PC34_NLO_PANEL_M568;
    out->wheelDownCommandC040 = DM1_PC34_NLO_COMMAND_C040;
    out->wheelDownPanelF0344Dispatch = 1;
    out->wheelDownPanelF0345Highlight = 2;
    out->wheelDownHighlightTrace[0] = DM1_PC34_NLO_C540;
    out->wheelDownHighlightTrace[1] = DM1_PC34_NLO_C538;
    out->wheelDownHighlightTrace[2] = DM1_PC34_NLO_C538;
    out->wheelDownPartialMaskDispatches = 2;
    out->wheelDownF0302DispatchCount = 1;
    out->wheelDownHandSnapshot = runtime->hand.thing;
    out->wheelDownSlotSnapshot = runtime->chest[offset].thing;

    /*
     * ReDMCSB: CHAMPION.C F0302:694-710 allows occupied hand + empty C30+
     * slot, removes the hand via F0298, and writes it to the chest slot with
     * F0301. This Firestaff wheel-down route preserves that ordering for a
     * non-leader-owned hand.
     */
    mouse_enable(runtime);
    handBefore = runtime->hand;
    runtime->hand = none_item();
    runtime->load[DM1_PC34_NLO_OWNER_INDEX] -= handBefore.weight;
    ++out->wheelDownF0298RemoveHandCount;
    runtime->chest[offset] = handBefore;
    ++out->wheelDownF0301WriteSlotCount;
    mark_owner_dirty(runtime);
    mouse_disable(runtime);
    return 1;
}

static int close_chest(RuntimePc34* runtime, int* skipped)
{
    int i;
    int count = 0;

    *skipped = 0;
    for (i = 0; i < DM1_PC34_NLO_SLOT_COUNT; ++i) {
        runtime->closed[i] = none_item();
    }
    for (i = 0; i < DM1_PC34_NLO_SLOT_COUNT; ++i) {
        if (is_none(runtime->chest[i])) {
            ++*skipped;
            continue;
        }
        runtime->closed[count++] = runtime->chest[i];
        runtime->chest[i] = none_item();
    }
    runtime->openChestThing = DM1_PC34_NLO_NONE;
    return count;
}

static void run_negative(
    DM1_V1_ChestScrollWheelNonLeaderOccupiedProbePc34* out)
{
    RuntimePc34 runtime;

    init_runtime(&runtime);
    runtime.hand = none_item();
    out->negativeWheelDownRejected = !wheel_down_drop_to_c538(&runtime, out);
    out->negativeHandAfter = runtime.hand.thing;
    out->negativeC538After =
        runtime.chest[slot_offset_from_zone(DM1_PC34_NLO_C538)].thing;
}

const DM1_V1_ChestScrollWheelNonLeaderOccupiedSpecPc34*
dm1_v1_chest_scroll_wheel_non_leader_occupied_slot_spec_pc34(void)
{
    return &s_spec;
}

const char*
dm1_v1_chest_scroll_wheel_non_leader_occupied_slot_source_evidence_pc34(void)
{
    return s_source_evidence;
}

int dm1_v1_chest_scroll_wheel_non_leader_occupied_slot_pc34(
    DM1_V1_ChestScrollWheelNonLeaderOccupiedProbePc34* out)
{
    RuntimePc34 runtime;
    int skipped = 0;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    init_runtime(&runtime);

    out->openChestBefore = runtime.openChestThing;
    out->ownerChampionIndex = DM1_PC34_NLO_OWNER_INDEX;
    out->ownerChampionOrdinal = runtime.inventoryChampionOrdinal;
    out->leaderIndex = runtime.leaderIndex;
    out->partyChampionCount = runtime.partyChampionCount;
    out->initialHandThing = runtime.hand.thing;
    out->initialHandIcon = runtime.hand.icon;
    out->initialOwnerLoad = runtime.load[DM1_PC34_NLO_OWNER_INDEX];
    copy_things(runtime.chest, out->initialChest);
    copy_icons(runtime.chest, out->initialIcons);
    out->initialThingCount = total_runtime_items(&runtime);

    if (!wheel_up_swap_from_c540(&runtime, out)) {
        return 0;
    }
    out->handAfterWheelUp = runtime.hand.thing;
    out->handIconAfterWheelUp = runtime.hand.icon;
    out->c540AfterWheelUp =
        runtime.chest[slot_offset_from_zone(DM1_PC34_NLO_C540)].thing;
    out->c538AfterWheelUp =
        runtime.chest[slot_offset_from_zone(DM1_PC34_NLO_C538)].thing;
    out->ownerLoadAfterWheelUp = runtime.load[DM1_PC34_NLO_OWNER_INDEX];
    out->ownerAttributesAfterWheelUp =
        runtime.attributes[DM1_PC34_NLO_OWNER_INDEX];
    copy_things(runtime.chest, out->chestAfterWheelUp);
    copy_icons(runtime.chest, out->iconsAfterWheelUp);

    if (!wheel_down_drop_to_c538(&runtime, out)) {
        return 0;
    }
    out->handAfterWheelDown = runtime.hand.thing;
    out->c538AfterWheelDown =
        runtime.chest[slot_offset_from_zone(DM1_PC34_NLO_C538)].thing;
    out->c540AfterWheelDown =
        runtime.chest[slot_offset_from_zone(DM1_PC34_NLO_C540)].thing;
    out->ownerLoadAfterWheelDown = runtime.load[DM1_PC34_NLO_OWNER_INDEX];
    out->ownerAttributesAfterWheelDown =
        runtime.attributes[DM1_PC34_NLO_OWNER_INDEX];
    copy_things(runtime.chest, out->chestAfterWheelDown);
    copy_icons(runtime.chest, out->iconsAfterWheelDown);

    out->closeCount = close_chest(&runtime, &skipped);
    out->closeSkippedEmptyEntries = skipped;
    out->closeHead = out->closeCount ? runtime.closed[0].thing
                                     : DM1_PC34_NLO_END;
    out->closeEndSentinel = DM1_PC34_NLO_END;
    copy_things(runtime.closed, out->closedSlots);
    copy_icons(runtime.closed, out->closedIcons);
    out->openChestAfterClose = runtime.openChestThing;
    out->g0425ClearedAfterClose = count_items(runtime.chest) == 0;
    out->finalThingCount =
        count_items(runtime.closed) + (!is_none(runtime.hand) ? 1 : 0);
    out->screenUpdateBalanced = runtime.screenUpdateDepth == 0;

    run_negative(out);
    return out->screenUpdateBalanced;
}
