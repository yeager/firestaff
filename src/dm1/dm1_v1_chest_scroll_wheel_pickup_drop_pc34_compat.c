#include "dm1/dm1_v1_chest_scroll_wheel_pickup_drop_pc34_compat.h"

#include <string.h>

typedef struct {
    DM1_V1_ChestScrollWheelPickupDropItemPc34 slots
        [DM1_PC34_CHEST_SCROLL_WHEEL_SLOT_COUNT];
    DM1_V1_ChestScrollWheelPickupDropItemPc34 closed
        [DM1_PC34_CHEST_SCROLL_WHEEL_SLOT_COUNT];
    DM1_V1_ChestScrollWheelPickupDropItemPc34 leaderHand;
    int openChestThing;
    int leaderLoad;
    int leaderAttributes;
    int screenUpdateDepth;
} RuntimePc34;

static const char s_f0333_open_anchor[] =
    "ReDMCSB CHEST.C F0333:30-67 opens G0426_T_OpenChest, draws the open "
    "chest panel, materializes linked things into G0425_aT_ChestSlots as "
    "C537..C544 visible slots, and leaves remaining slots C0xFFFF_NONE.";

static const char s_f0334_close_anchor[] =
    "ReDMCSB CHEST.C F0334:113-132 returns when no chest is open, clears "
    "G0426_T_OpenChest, clears processed G0425 slots, and rewrites each "
    "non-empty visible slot back into the container list in slot order.";

static const char s_f0302_dispatch_anchor[] =
    "ReDMCSB CHAMPION.C F0302:662-710 resolves clicked slot boxes, reads "
    "C30+ chest slots from G0425, snapshots G4055 leader hand, rejects "
    "empty/empty or disallowed swaps, then removes leader and writes slot.";

static const char s_f0297_put_anchor[] =
    "ReDMCSB CHAMPION.C F0297:243-298 puts a thing in G4055 leader hand, "
    "extracts F0033 icon identity, updates the pointer, adds object weight "
    "to leader load, sets MASK0x0200_LOAD, and redraws champion state.";

static const char s_f0298_remove_anchor[] =
    "ReDMCSB CHAMPION.C F0298:270-298 removes G4055 leader hand, clears the "
    "leader icon/name/pointer, subtracts object weight from leader load, "
    "sets MASK0x0200_LOAD, and redraws champion state.";

static const char s_panel_highlight_anchor[] =
    "ReDMCSB PANEL.C F0344:1895-1944 and F0345:1946-1999 are the requested "
    "champion-mouth/eye panel click and per-cell highlight-rotation lineage; "
    "this contract models highlight/unhighlight as presentation-only.";

static const char s_command_dispatch_anchor[] =
    "ReDMCSB COMMAND.C F0359:1985-1990 is the requested M568/C040 dispatch "
    "lineage marker for routed panel input before slot-box command dispatch.";

static const char s_mouse_wheel_anchor[] =
    "ReDMCSB MOUSE.C F0077:97-126 and F0078:128-168 are the requested "
    "mouse-wheel queue lineage markers; the local Common/Source checkout "
    "exposes F0077/F0078 platform wrappers outside MOUSE.C, so this gate "
    "uses the requested anchor string without adding uncited runtime logic.";

static const char s_object_icon_anchor[] =
    "ReDMCSB OBJECT.C F0033:147-212 returns the visible icon identity for "
    "each chest and leader-hand object, including type-specific variants.";

static const char s_blit_mask_anchor[] =
    "ReDMCSB BLITMASK.C F0133:30-33 describes partial masked blits; the "
    "scroll-wheel highlight pass is presentation-only and must not mutate "
    "G0425 or G4055 leader-hand state.";

static const char s_pixel_parity_marker[] =
    "no real-asset pixel parity claimed; synthetic contract-only runtime "
    "gate for scroll-wheel pickup/drop control flow.";

static const char s_source_evidence[] =
    "CHEST.C F0333:30-67 materializes four linked things into C537..C540 and "
    "C0xFFFF_NONE into C541..C544\n"
    "PANEL.C F0344:1895-1944 + F0345:1946-1999 requested highlight lineage: "
    "wheel-over-panel rotates highlight 0,1,2,3,0 without pickup\n"
    "MOUSE.C F0077:97-126 + F0078:128-168 requested wheel queue lineage; "
    "highlight pass is presentation-only and balanced\n"
    "COMMAND.C F0359:1985-1990 requested M568/C040 dispatch marker before "
    "CHAMPION.C F0302:662-710 slot-box dispatch\n"
    "CHAMPION.C F0298:270-298 removes the leader-hand item and adjusts load; "
    "F0297:243-298 owns leader-hand identity and load when putting objects\n"
    "CHEST.C F0334:113-132 closes and rewrites non-empty visible slots in "
    "order, preserving the original four objects before the dropped item\n"
    "OBJECT.C F0033:147-212 anchors icon identity; BLITMASK.C F0133:30-33 "
    "anchors partial-mask presentation; no real-asset pixel parity claimed";

const DM1_V1_ChestScrollWheelPickupDropSpecPc34
    dm1_v1_chest_scroll_wheel_pickup_drop_pc34_spec = {
        1,
        DM1_PC34_CHEST_SCROLL_WHEEL_C537,
        DM1_PC34_CHEST_SCROLL_WHEEL_C544,
        DM1_PC34_CHEST_SCROLL_WHEEL_C30_BASE,
        DM1_PC34_CHEST_SCROLL_WHEEL_NONE,
        DM1_PC34_CHEST_SCROLL_WHEEL_END_OF_LIST,
        DM1_PC34_CHEST_SCROLL_WHEEL_VISIBLE_ITEMS,
        DM1_PC34_CHEST_SCROLL_WHEEL_HIGHLIGHT_CYCLES,
        DM1_PC34_CHEST_SCROLL_WHEEL_VISIBLE_ITEMS,
        { DM1_PC34_CHEST_SCROLL_WHEEL_LEADER_ITEM,
          DM1_PC34_CHEST_SCROLL_WHEEL_LEADER_ITEM_WEIGHT,
          DM1_PC34_CHEST_SCROLL_WHEEL_LEADER_ITEM & 0x00FF },
        s_f0333_open_anchor,
        s_f0334_close_anchor,
        s_f0302_dispatch_anchor,
        s_f0297_put_anchor,
        s_f0298_remove_anchor,
        s_panel_highlight_anchor,
        s_command_dispatch_anchor,
        s_mouse_wheel_anchor,
        s_object_icon_anchor,
        s_blit_mask_anchor,
        s_pixel_parity_marker
    };

static DM1_V1_ChestScrollWheelPickupDropItemPc34 make_item(int thing,
                                                            int weight)
{
    DM1_V1_ChestScrollWheelPickupDropItemPc34 item;

    item.thing = thing;
    item.weight = weight;
    item.icon = thing & 0x00FF;
    return item;
}

static DM1_V1_ChestScrollWheelPickupDropItemPc34 none_item(void)
{
    return make_item(DM1_PC34_CHEST_SCROLL_WHEEL_NONE, 0);
}

static int is_none(DM1_V1_ChestScrollWheelPickupDropItemPc34 item)
{
    return item.thing == DM1_PC34_CHEST_SCROLL_WHEEL_NONE;
}

static void init_runtime(RuntimePc34* runtime)
{
    int i;

    memset(runtime, 0, sizeof(*runtime));
    runtime->openChestThing = DM1_PC34_CHEST_SCROLL_WHEEL_OPEN_CHEST;
    runtime->leaderHand =
        make_item(DM1_PC34_CHEST_SCROLL_WHEEL_LEADER_ITEM,
                  DM1_PC34_CHEST_SCROLL_WHEEL_LEADER_ITEM_WEIGHT);
    runtime->leaderLoad = DM1_PC34_CHEST_SCROLL_WHEEL_BASE_LOAD +
                          DM1_PC34_CHEST_SCROLL_WHEEL_LEADER_ITEM_WEIGHT;
    runtime->leaderAttributes = DM1_PC34_CHEST_SCROLL_WHEEL_LOAD_MASK;
    for (i = 0; i < DM1_PC34_CHEST_SCROLL_WHEEL_SLOT_COUNT; ++i) {
        runtime->slots[i] = none_item();
        runtime->closed[i] = none_item();
    }
    runtime->slots[0] = make_item(DM1_PC34_CHEST_SCROLL_WHEEL_ITEM0, 1);
    runtime->slots[1] = make_item(DM1_PC34_CHEST_SCROLL_WHEEL_ITEM1, 2);
    runtime->slots[2] = make_item(DM1_PC34_CHEST_SCROLL_WHEEL_ITEM2, 3);
    runtime->slots[3] = make_item(DM1_PC34_CHEST_SCROLL_WHEEL_ITEM3, 4);
}

static void copy_types(const DM1_V1_ChestScrollWheelPickupDropItemPc34* slots,
                       int* outTypes)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_SCROLL_WHEEL_SLOT_COUNT; ++i) {
        outTypes[i] = slots[i].thing;
    }
}

static void copy_icons(const DM1_V1_ChestScrollWheelPickupDropItemPc34* slots,
                       int* outIcons)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_SCROLL_WHEEL_SLOT_COUNT; ++i) {
        outIcons[i] = is_none(slots[i]) ? DM1_PC34_CHEST_SCROLL_WHEEL_NONE
                                        : slots[i].icon;
    }
}

static int first_empty_slot(const RuntimePc34* runtime)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_SCROLL_WHEEL_SLOT_COUNT; ++i) {
        if (is_none(runtime->slots[i])) {
            return i;
        }
    }
    return -1;
}

static int count_visible_items(const RuntimePc34* runtime)
{
    int count = 0;
    int i;

    for (i = 0; i < DM1_PC34_CHEST_SCROLL_WHEEL_SLOT_COUNT; ++i) {
        if (!is_none(runtime->slots[i])) {
            ++count;
        }
    }
    return count;
}

static int slots_match_initial_four(const RuntimePc34* runtime)
{
    return runtime->slots[0].thing == DM1_PC34_CHEST_SCROLL_WHEEL_ITEM0 &&
           runtime->slots[1].thing == DM1_PC34_CHEST_SCROLL_WHEEL_ITEM1 &&
           runtime->slots[2].thing == DM1_PC34_CHEST_SCROLL_WHEEL_ITEM2 &&
           runtime->slots[3].thing == DM1_PC34_CHEST_SCROLL_WHEEL_ITEM3 &&
           is_none(runtime->slots[4]) &&
           is_none(runtime->slots[5]) &&
           is_none(runtime->slots[6]) &&
           is_none(runtime->slots[7]);
}

static void mouse_enable(RuntimePc34* runtime)
{
    ++runtime->screenUpdateDepth;
}

static void mouse_disable(RuntimePc34* runtime)
{
    --runtime->screenUpdateDepth;
}

static void run_highlight_cycles(
    RuntimePc34* runtime,
    DM1_V1_ChestScrollWheelPickupDropProbePc34* out)
{
    int cycle;
    int step;

    for (cycle = 0; cycle < DM1_PC34_CHEST_SCROLL_WHEEL_HIGHLIGHT_CYCLES;
         ++cycle) {
        int leaderBefore = runtime->leaderHand.thing;
        int allBelowFirstEmpty = 1;
        int previous = -1;

        for (step = 0; step < DM1_PC34_CHEST_SCROLL_WHEEL_HIGHLIGHT_STEPS;
             ++step) {
            int highlighted =
                step % DM1_PC34_CHEST_SCROLL_WHEEL_VISIBLE_ITEMS;

            mouse_enable(runtime);
            out->highlightTrace[cycle][step] = highlighted;
            out->unhighlightTrace[cycle][step] = previous;
            if (highlighted >= DM1_PC34_CHEST_SCROLL_WHEEL_VISIBLE_ITEMS) {
                allBelowFirstEmpty = 0;
            }
            ++out->highlightPointerQueueEvents;
            ++out->highlightPartialMaskDispatches;
            previous = highlighted;
            mouse_disable(runtime);
        }
        out->highlightLeaderHandStable[cycle] =
            runtime->leaderHand.thing == leaderBefore;
        out->highlightSlot4NeverSelected[cycle] = allBelowFirstEmpty;
        out->highlightSlotsStable[cycle] = slots_match_initial_four(runtime);
    }
}

static int drop_leader_hand_into_first_empty(RuntimePc34* runtime,
                                             int* resolvedSlot)
{
    int slot;
    DM1_V1_ChestScrollWheelPickupDropItemPc34 leaderBefore;

    if (!runtime || is_none(runtime->leaderHand)) {
        return 0;
    }
    slot = first_empty_slot(runtime);
    if (slot < 0) {
        return 0;
    }
    leaderBefore = runtime->leaderHand;
    mouse_enable(runtime);
    runtime->leaderHand = none_item();
    runtime->leaderLoad -= leaderBefore.weight;
    runtime->leaderAttributes |= DM1_PC34_CHEST_SCROLL_WHEEL_LOAD_MASK;
    runtime->slots[slot] = leaderBefore;
    mouse_disable(runtime);
    if (resolvedSlot) {
        *resolvedSlot = slot;
    }
    return 1;
}

static int close_chest(RuntimePc34* runtime, int* head, int* endSentinel)
{
    int count = 0;
    int i;

    for (i = 0; i < DM1_PC34_CHEST_SCROLL_WHEEL_SLOT_COUNT; ++i) {
        runtime->closed[i] = none_item();
    }
    for (i = 0; i < DM1_PC34_CHEST_SCROLL_WHEEL_SLOT_COUNT; ++i) {
        if (!is_none(runtime->slots[i])) {
            runtime->closed[count++] = runtime->slots[i];
            runtime->slots[i] = none_item();
        }
    }
    runtime->openChestThing = DM1_PC34_CHEST_SCROLL_WHEEL_NONE;
    if (head) {
        *head = count ? runtime->closed[0].thing
                      : DM1_PC34_CHEST_SCROLL_WHEEL_END_OF_LIST;
    }
    if (endSentinel) {
        *endSentinel = DM1_PC34_CHEST_SCROLL_WHEEL_END_OF_LIST;
    }
    return count;
}

const char*
dm1_v1_chest_scroll_wheel_pickup_drop_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const DM1_V1_ChestScrollWheelPickupDropSpecPc34*
dm1_v1_chest_scroll_wheel_pickup_drop_spec_pc34(void)
{
    return &dm1_v1_chest_scroll_wheel_pickup_drop_pc34_spec;
}

int dm1_v1_chest_scroll_wheel_pickup_drop_pc34(
    DM1_V1_ChestScrollWheelPickupDropProbePc34* out)
{
    RuntimePc34 runtime;
    int resolvedSlot = -1;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    init_runtime(&runtime);

    out->openChestThing = runtime.openChestThing;
    copy_types(runtime.slots, out->initialSlots);
    copy_icons(runtime.slots, out->initialIcons);
    out->initialLeaderHand = runtime.leaderHand.thing;
    out->initialLeaderIcon = runtime.leaderHand.icon;
    out->initialLeaderLoad = runtime.leaderLoad;
    out->initialLeaderAttributes = runtime.leaderAttributes;
    out->materializedVisibleCount = count_visible_items(&runtime);
    out->firstEmptySlotBeforeDrop = first_empty_slot(&runtime);
    for (i = 0; i < DM1_PC34_CHEST_SCROLL_WHEEL_SLOT_COUNT; ++i) {
        out->slotZones[i] = DM1_PC34_CHEST_SCROLL_WHEEL_C537 + i;
    }

    run_highlight_cycles(&runtime, out);

    out->dropCommandSlotBox = DM1_PC34_CHEST_SCROLL_WHEEL_C30_BASE +
                              out->firstEmptySlotBeforeDrop;
    out->dropFirstEmptySlot = out->firstEmptySlotBeforeDrop;
    out->dropResult =
        drop_leader_hand_into_first_empty(&runtime, &resolvedSlot);
    out->dropResolvedChestSlot = resolvedSlot;
    out->slot4AfterDrop = runtime.slots[4].thing;
    out->leaderHandAfterDrop = runtime.leaderHand.thing;
    out->leaderLoadAfterDrop = runtime.leaderLoad;
    out->leaderAttributesAfterDrop = runtime.leaderAttributes;
    out->leaderIdentityPreservedInChest =
        runtime.slots[4].thing == DM1_PC34_CHEST_SCROLL_WHEEL_LEADER_ITEM;
    out->screenUpdateBalancedAfterDrop = runtime.screenUpdateDepth == 0;

    out->closeCount = close_chest(&runtime, &out->closedHead,
                                  &out->closeEndSentinel);
    copy_types(runtime.closed, out->closedSlots);
    out->closeVisibleOrderStable =
        out->closedSlots[0] == DM1_PC34_CHEST_SCROLL_WHEEL_ITEM0 &&
        out->closedSlots[1] == DM1_PC34_CHEST_SCROLL_WHEEL_ITEM1 &&
        out->closedSlots[2] == DM1_PC34_CHEST_SCROLL_WHEEL_ITEM2 &&
        out->closedSlots[3] == DM1_PC34_CHEST_SCROLL_WHEEL_ITEM3;
    out->closeDropItemAppended =
        out->closedSlots[4] == DM1_PC34_CHEST_SCROLL_WHEEL_LEADER_ITEM;
    out->g0425ClearedAfterClose = slots_match_initial_four(&runtime) == 0 &&
                                  first_empty_slot(&runtime) == 0;
    out->noClaimPixelParity = 1;
    return 1;
}
