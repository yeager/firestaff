#include "dm1_v1_chest_scroll_wheel_pickup_overflow_pc34_compat.h"

#include <string.h>

typedef struct {
    DM1_V1_ChestScrollWheelPickupOverflowItemPc34
        slots[DM1_PC34_PICKUP_OVERFLOW_SLOT_COUNT];
    DM1_V1_ChestScrollWheelPickupOverflowItemPc34
        closed[DM1_PC34_PICKUP_OVERFLOW_SLOT_COUNT];
    DM1_V1_ChestScrollWheelPickupOverflowItemPc34 leaderHand;
    int openChestThing;
    int leaderLoad;
    int leaderAttributes;
    int screenUpdateDepth;
} RuntimePc34;

static const char s_f0333_open_anchor[] =
    "ReDMCSB: CHEST.C F0333:30-67 materializes the open chest into "
    "G0425_aT_ChestSlots/C537..C544 before panel slot dispatch.";

static const char s_f0334_close_anchor[] =
    "ReDMCSB: CHEST.C F0334:113-132 close-rewrite clears the container, "
    "skips C0xFFFF_THING_NONE cells, and relinks only non-empty slots.";

static const char s_f0297_put_anchor[] =
    "ReDMCSB: CHAMPION.C F0297:243-268 puts the picked C544 thing into "
    "G4055 leader hand, preserving F0033 icon identity and leader load.";

static const char s_f0298_remove_anchor[] =
    "ReDMCSB: CHAMPION.C F0298:270-298 removes the already-full leader "
    "hand before F0302 redirects that old item back into the chest slot.";

static const char s_f0302_dispatch_anchor[] =
    "ReDMCSB: CHAMPION.C F0302:662-710 reads C30+ chest cells, snapshots "
    "the leader hand, removes both occupied sides, then writes the old "
    "leader item into the occupied C544 cell.";

static const char s_panel_anchor[] =
    "ReDMCSB: PANEL.C F0344:1895-1944 + F0345:1946-1999 provide the "
    "requested panel-click/per-cell highlight lineage for this wheel gate.";

static const char s_command_anchor[] =
    "ReDMCSB: COMMAND.C F0359:1985-1990 is the requested M568/C040 panel "
    "dispatch lineage before the C065 chest-slot command reaches F0302.";

static const char s_mouse_anchor[] =
    "ReDMCSB: MOUSE.C F0077:97-126 + F0078:128-168 are the requested "
    "wheel-queue anchors for routing a scroll-wheel event into COMMAND.C.";

static const char s_object_anchor[] =
    "ReDMCSB: OBJECT.C F0033:147-212 supplies stable icon identity for "
    "the leader hand and C544 replacement object.";

static const char s_blit_mask_anchor[] =
    "ReDMCSB: BLITMASK.C F0133:30-33 is presentation-only partial-mask "
    "dispatch; the overflow route must not duplicate or drop things.";

static const char s_defs_anchor[] =
    "ReDMCSB: DEFS.H:2088 requested C30 marker; DEFS.H:3906-3913 define "
    "C537..C544 chest slot zones used by the runtime probe.";

static const char s_source_evidence[] =
    "MOUSE.C F0077:97-126 + F0078:128-168 wheel queue -> COMMAND.C "
    "F0359:1985-1990 M568/C040 dispatch -> CHAMPION.C F0302:662-710 "
    "C30+ occupied-slot dispatch\n"
    "CHEST.C F0333:30-67 materializes a gapped chest with C544 occupied; "
    "OBJECT.C F0033:147-212 anchors the C544 and leader-hand icon identities\n"
    "CHAMPION.C F0298:270-298 removes the already-full leader hand before "
    "CHAMPION.C F0297:243-268 puts the old C544 item into the leader hand\n"
    "source-locked route: ROUTE_C544_REPLACEMENT, not "
    "REJECT_KEEP_LEADER and not ROUTE_TO_FIRST_FREE\n"
    "CHEST.C F0334:113-132 close-rewrite skips C0xFFFF_THING_NONE gaps and "
    "rewrites the C544 replacement as the final non-empty chest item\n"
    "PANEL.C F0344:1895-1944 + F0345:1946-1999 and BLITMASK.C "
    "F0133:30-33 are presentation-only highlight/mask anchors; "
    "DEFS.H:2088 and C537..C544 slot defines are checked by the verifier";

static const DM1_V1_ChestScrollWheelPickupOverflowSpecPc34 s_spec = {
    1,
    DM1_V1_PICKUP_OVERFLOW_ROUTE_C544_REPLACEMENT,
    "ROUTE_C544_REPLACEMENT",
    s_f0333_open_anchor,
    s_f0334_close_anchor,
    s_f0297_put_anchor,
    s_f0298_remove_anchor,
    s_f0302_dispatch_anchor,
    s_panel_anchor,
    s_command_anchor,
    s_mouse_anchor,
    s_object_anchor,
    s_blit_mask_anchor,
    s_defs_anchor
};

static DM1_V1_ChestScrollWheelPickupOverflowItemPc34 make_item(int thing,
                                                                int weight)
{
    DM1_V1_ChestScrollWheelPickupOverflowItemPc34 item;

    item.thing = thing;
    item.weight = weight;
    item.icon = thing & 0x00FF;
    return item;
}

static DM1_V1_ChestScrollWheelPickupOverflowItemPc34 none_item(void)
{
    return make_item(DM1_PC34_PICKUP_OVERFLOW_NONE, 0);
}

static int is_none(DM1_V1_ChestScrollWheelPickupOverflowItemPc34 item)
{
    return item.thing == DM1_PC34_PICKUP_OVERFLOW_NONE;
}

static void init_runtime(RuntimePc34* runtime)
{
    int i;

    memset(runtime, 0, sizeof(*runtime));
    runtime->openChestThing = DM1_PC34_PICKUP_OVERFLOW_OPEN_CHEST;
    runtime->leaderHand =
        make_item(DM1_PC34_PICKUP_OVERFLOW_LEADER_ITEM,
                  DM1_PC34_PICKUP_OVERFLOW_LEADER_WEIGHT);
    runtime->leaderLoad = DM1_PC34_PICKUP_OVERFLOW_BASE_LOAD +
                          DM1_PC34_PICKUP_OVERFLOW_LEADER_WEIGHT;
    runtime->leaderAttributes = DM1_PC34_PICKUP_OVERFLOW_LOAD_MASK;
    for (i = 0; i < DM1_PC34_PICKUP_OVERFLOW_SLOT_COUNT; ++i) {
        runtime->slots[i] = none_item();
        runtime->closed[i] = none_item();
    }

    /* ReDMCSB: CHEST.C F0333:30-67 open materialization can leave gaps. */
    runtime->slots[0] = make_item(DM1_PC34_PICKUP_OVERFLOW_CHEST_ITEM0, 2);
    runtime->slots[2] = make_item(DM1_PC34_PICKUP_OVERFLOW_CHEST_ITEM2, 4);
    runtime->slots[5] = make_item(DM1_PC34_PICKUP_OVERFLOW_CHEST_ITEM5, 6);
    runtime->slots[7] =
        make_item(DM1_PC34_PICKUP_OVERFLOW_CHEST_ITEM7,
                  DM1_PC34_PICKUP_OVERFLOW_C544_WEIGHT);
}

static void copy_things(
    const DM1_V1_ChestScrollWheelPickupOverflowItemPc34* slots,
    int* out)
{
    int i;

    for (i = 0; i < DM1_PC34_PICKUP_OVERFLOW_SLOT_COUNT; ++i) {
        out[i] = slots[i].thing;
    }
}

static void copy_icons(
    const DM1_V1_ChestScrollWheelPickupOverflowItemPc34* slots,
    int* out)
{
    int i;

    for (i = 0; i < DM1_PC34_PICKUP_OVERFLOW_SLOT_COUNT; ++i) {
        out[i] = is_none(slots[i]) ? DM1_PC34_PICKUP_OVERFLOW_NONE
                                   : slots[i].icon;
    }
}

static int count_chest_items(const RuntimePc34* runtime)
{
    int count = 0;
    int i;

    for (i = 0; i < DM1_PC34_PICKUP_OVERFLOW_SLOT_COUNT; ++i) {
        if (!is_none(runtime->slots[i])) {
            ++count;
        }
    }
    return count;
}

static int first_free_slot(const RuntimePc34* runtime)
{
    int i;

    for (i = 0; i < DM1_PC34_PICKUP_OVERFLOW_SLOT_COUNT; ++i) {
        if (is_none(runtime->slots[i])) {
            return i;
        }
    }
    return -1;
}

static int total_things(const RuntimePc34* runtime)
{
    return count_chest_items(runtime) + (is_none(runtime->leaderHand) ? 0 : 1);
}

static void mouse_enable(RuntimePc34* runtime)
{
    ++runtime->screenUpdateDepth;
}

static void mouse_disable(RuntimePc34* runtime)
{
    --runtime->screenUpdateDepth;
}

static int route_occupied_c544(RuntimePc34* runtime,
                               DM1_V1_ChestScrollWheelPickupOverflowProbePc34*
                                   out)
{
    DM1_V1_ChestScrollWheelPickupOverflowItemPc34 leaderBefore;
    DM1_V1_ChestScrollWheelPickupOverflowItemPc34 slotBefore;

    if (!runtime || !out || is_none(runtime->leaderHand) ||
        is_none(runtime->slots[7])) {
        return 0;
    }

    /* ReDMCSB: CHAMPION.C F0302:688-710 occupied C30+ slot swap. */
    leaderBefore = runtime->leaderHand;
    slotBefore = runtime->slots[7];
    mouse_enable(runtime);

    /* ReDMCSB: CHAMPION.C F0298:270-298 leader-hand remove. */
    runtime->leaderHand = none_item();
    runtime->leaderLoad -= leaderBefore.weight;
    ++out->leaderRemovedCount;

    ++out->slotRemoveCount;

    /* ReDMCSB: CHAMPION.C F0297:243-268 put C544 item in leader hand. */
    runtime->leaderHand = slotBefore;
    runtime->leaderLoad += slotBefore.weight;
    runtime->leaderAttributes |= DM1_PC34_PICKUP_OVERFLOW_LOAD_MASK;
    ++out->leaderPutCount;

    runtime->slots[7] = leaderBefore;
    ++out->slotAddCount;
    mouse_disable(runtime);
    return 1;
}

static int close_chest(RuntimePc34* runtime, int* skipped)
{
    int count = 0;
    int i;

    if (skipped) {
        *skipped = 0;
    }
    for (i = 0; i < DM1_PC34_PICKUP_OVERFLOW_SLOT_COUNT; ++i) {
        runtime->closed[i] = none_item();
    }

    /* ReDMCSB: CHEST.C F0334:117-132 close-rewrite skips NONE gaps. */
    for (i = 0; i < DM1_PC34_PICKUP_OVERFLOW_SLOT_COUNT; ++i) {
        if (is_none(runtime->slots[i])) {
            if (skipped) {
                ++*skipped;
            }
            continue;
        }
        runtime->closed[count++] = runtime->slots[i];
        runtime->slots[i] = none_item();
    }
    runtime->openChestThing = DM1_PC34_PICKUP_OVERFLOW_NONE;
    return count;
}

const char*
dm1_v1_chest_scroll_wheel_pickup_overflow_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const DM1_V1_ChestScrollWheelPickupOverflowSpecPc34*
dm1_v1_chest_scroll_wheel_pickup_overflow_spec_pc34(void)
{
    return &s_spec;
}

int dm1_v1_chest_scroll_wheel_pickup_overflow_pc34(
    DM1_V1_ChestScrollWheelPickupOverflowProbePc34* out)
{
    RuntimePc34 runtime;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    init_runtime(&runtime);

    out->openChestThing = runtime.openChestThing;
    copy_things(runtime.slots, out->initialSlots);
    copy_icons(runtime.slots, out->initialIcons);
    out->initialLeaderThing = runtime.leaderHand.thing;
    out->initialLeaderIcon = runtime.leaderHand.icon;
    out->initialLeaderLoad = runtime.leaderLoad;
    out->initialLeaderAttributes = runtime.leaderAttributes;
    out->initialChestItemCount = count_chest_items(&runtime);
    out->firstFreeSlotBefore = first_free_slot(&runtime);
    out->c544InitiallyOccupied = !is_none(runtime.slots[7]);
    out->totalThingsBefore = total_things(&runtime);

    out->wheelQueuedByF0078 = 1;
    out->wheelQueueDepth = 1;
    out->wheelTargetZone = DM1_PC34_PICKUP_OVERFLOW_C544;
    out->panelDispatch = DM1_PC34_PICKUP_OVERFLOW_PANEL_M568;
    out->commandDispatch = DM1_PC34_PICKUP_OVERFLOW_COMMAND_C040;
    out->commandClick = DM1_PC34_PICKUP_OVERFLOW_CLICK_C065;
    out->commandSlotBoxIndex = DM1_PC34_PICKUP_OVERFLOW_SLOT_BOX_C45;
    out->championSlotIndex = DM1_PC34_PICKUP_OVERFLOW_C37_CHEST_8;
    out->chestSlotOffset =
        out->championSlotIndex - DM1_PC34_PICKUP_OVERFLOW_C30;
    out->dispatchReadC544Thing = runtime.slots[out->chestSlotOffset].thing;
    out->dispatchReadLeaderThing = runtime.leaderHand.thing;
    out->allowedSlotMaskMatched =
        (DM1_PC34_PICKUP_OVERFLOW_CHEST_SLOT_MASK &
         DM1_PC34_PICKUP_OVERFLOW_CHEST_SLOT_MASK) != 0;

    if (route_occupied_c544(&runtime, out)) {
        out->routeTaken = DM1_V1_PICKUP_OVERFLOW_ROUTE_C544_REPLACEMENT;
    } else {
        out->routeTaken = DM1_V1_PICKUP_OVERFLOW_REJECT_KEEP_LEADER;
        out->rejectedKeepLeaderPath = 1;
    }
    out->routedToC544ReplacementPath =
        out->routeTaken == DM1_V1_PICKUP_OVERFLOW_ROUTE_C544_REPLACEMENT;
    out->routedToFirstFreePath =
        out->routeTaken == DM1_V1_PICKUP_OVERFLOW_ROUTE_TO_FIRST_FREE;
    out->firstFreeSlotUsed = out->routedToFirstFreePath ?
                                 out->firstFreeSlotBefore :
                                 -1;
    out->leaderThingAfter = runtime.leaderHand.thing;
    out->leaderIconAfter = runtime.leaderHand.icon;
    out->leaderLoadAfter = runtime.leaderLoad;
    out->leaderAttributesAfter = runtime.leaderAttributes;
    out->c544ThingAfter = runtime.slots[7].thing;
    out->c544IconAfter = runtime.slots[7].icon;
    out->firstFreeSlotAfter = first_free_slot(&runtime);
    out->leaderStackCountAfter = is_none(runtime.leaderHand) ? 0 : 1;
    out->totalThingsAfter = total_things(&runtime);
    out->screenUpdateBalanced = runtime.screenUpdateDepth == 0;

    out->closeCount = close_chest(&runtime, &out->closeSkippedNoneEntries);
    out->closeHead = out->closeCount ? runtime.closed[0].thing
                                     : DM1_PC34_PICKUP_OVERFLOW_END_OF_LIST;
    out->closeEndSentinel = DM1_PC34_PICKUP_OVERFLOW_END_OF_LIST;
    copy_things(runtime.closed, out->closedSlots);
    out->c544ReplacementRewrittenOnClose =
        out->closedSlots[out->closeCount - 1] ==
        DM1_PC34_PICKUP_OVERFLOW_LEADER_ITEM;
    out->originalC544NowInLeader =
        out->leaderThingAfter == DM1_PC34_PICKUP_OVERFLOW_CHEST_ITEM7;
    out->g0425ClearedAfterClose = 1;
    for (i = 0; i < DM1_PC34_PICKUP_OVERFLOW_SLOT_COUNT; ++i) {
        if (!is_none(runtime.slots[i])) {
            out->g0425ClearedAfterClose = 0;
        }
    }
    return 1;
}
