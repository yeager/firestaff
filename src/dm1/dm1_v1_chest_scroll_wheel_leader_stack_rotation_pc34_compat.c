#include "dm1_v1_chest_scroll_wheel_leader_stack_rotation_pc34_compat.h"

#include <string.h>

typedef struct {
    DM1_V1_ChestLeaderStackRotationItemPc34 chest
        [DM1_V1_CHEST_LEADER_STACK_ROTATION_SLOT_COUNT];
    DM1_V1_ChestLeaderStackRotationItemPc34 closed
        [DM1_V1_CHEST_LEADER_STACK_ROTATION_SLOT_COUNT];
    DM1_V1_ChestLeaderStackRotationItemPc34 leaderStack
        [DM1_V1_CHEST_LEADER_STACK_ROTATION_STACK_COUNT];
    int openChestThing;
    int focusIndex;
    int screenUpdateDepth;
    int leaderAttributes;
} RuntimePc34;

static const char s_source_evidence[] =
    "CHEST.C F0333:30-67 materializes the open chest into G0425 C537..C544; "
    "CHEST.C F0334:113-132 closes G0426 and rewrites non-empty visible cells; "
    "CHAMPION.C F0297:243-268 and F0298:270-298 own leader hand identity, "
    "pointer updates, and load flags; CHAMPION.C F0301:606-614 writes C30+ "
    "slots through G0425; CHAMPION.C F0302:662-710 snapshots hand and slot "
    "state before swap; COMMAND.C F0378:1973-1983 routes chest panel input to "
    "F0302; PANEL.C F0354:2307-2344 closes inventory/chest under mouse-update "
    "bracketing; UTAMSCR.C F0077:147-151 and F0078:141-145 are pointer update "
    "wrappers; OBJECT.C F0033:147-212 preserves icon identity; BLITMASK.C "
    "F0133:30-33 anchors partial-mask presentation; DEFS.H:810-816 and "
    "3906-3913 define C30..C36 and C537..C544.";

static const DM1_V1_ChestLeaderStackRotationSpecPc34 s_spec = {
    s_source_evidence,
    "ReDMCSB CHEST.C F0333:30-67 materializes linked contents into G0425 "
    "C537..C544 without rewriting cells during same-open display.",
    "ReDMCSB CHEST.C F0334:113-132 clears G0426 and relinks non-empty "
    "visible chest cells in slot order.",
    "ReDMCSB CHAMPION.C F0297:243-268 puts an object in G4055 leader hand "
    "and refreshes icon/name/load state.",
    "ReDMCSB CHAMPION.C F0298:270-298 removes G4055 leader hand and clears "
    "pointer/icon/load state.",
    "ReDMCSB CHAMPION.C F0301:606-614 writes C30+ slots through G0425.",
    "ReDMCSB CHAMPION.C F0302:662-710 reads C30+ G0425 slots, snapshots "
    "G4055, and performs the swap only after guards pass.",
    "ReDMCSB COMMAND.C F0378:1973-1983 dispatches M569 chest panel clicks "
    "to F0302 slot-box commands.",
    "ReDMCSB PANEL.C F0354:2307-2344 brackets inventory close and "
    "F0334_INVENTORY_CloseChest with F0077/F0078.",
    "ReDMCSB UTAMSCR.C F0077:147-151 and F0078:141-145 wrap pointer "
    "screen-update changes.",
    "ReDMCSB OBJECT.C F0033:147-212 returns stable visible icon identity.",
    "ReDMCSB BLITMASK.C F0133:30-33 anchors partial-mask presentation.",
    "ReDMCSB DEFS.H:810-816 defines C30..C36 and 3906-3913 defines "
    "C537..C544."
};

static DM1_V1_ChestLeaderStackRotationItemPc34 make_item(int thing, int weight)
{
    DM1_V1_ChestLeaderStackRotationItemPc34 item;

    item.thing = thing;
    item.icon = thing & 0x00FF;
    item.weight = weight;
    return item;
}

static DM1_V1_ChestLeaderStackRotationItemPc34 none_item(void)
{
    return make_item(DM1_V1_CHEST_LEADER_STACK_ROTATION_NONE, 0);
}

static int is_none(DM1_V1_ChestLeaderStackRotationItemPc34 item)
{
    return item.thing == DM1_V1_CHEST_LEADER_STACK_ROTATION_NONE;
}

static void mouse_enable(RuntimePc34* runtime)
{
    ++runtime->screenUpdateDepth;
}

static void mouse_disable(RuntimePc34* runtime)
{
    --runtime->screenUpdateDepth;
}

static void copy_chest(const RuntimePc34* runtime, int* out)
{
    int i;

    for (i = 0; i < DM1_V1_CHEST_LEADER_STACK_ROTATION_SLOT_COUNT; ++i) {
        out[i] = runtime->chest[i].thing;
    }
}

static void copy_leader_stack(const RuntimePc34* runtime, int* out)
{
    int i;

    for (i = 0; i < DM1_V1_CHEST_LEADER_STACK_ROTATION_STACK_COUNT; ++i) {
        out[i] = runtime->leaderStack[i].thing;
    }
}

static int chest_matches_initial(const RuntimePc34* runtime)
{
    return runtime->chest[0].thing ==
               DM1_V1_CHEST_LEADER_STACK_ROTATION_ITEM0 &&
           runtime->chest[1].thing ==
               DM1_V1_CHEST_LEADER_STACK_ROTATION_ITEM1 &&
           runtime->chest[2].thing ==
               DM1_V1_CHEST_LEADER_STACK_ROTATION_ITEM2 &&
           is_none(runtime->chest[3]) &&
           is_none(runtime->chest[4]) &&
           is_none(runtime->chest[5]) &&
           is_none(runtime->chest[6]) &&
           is_none(runtime->chest[7]);
}

static int leader_stack_matches_initial(const RuntimePc34* runtime)
{
    return runtime->leaderStack[0].thing ==
               DM1_V1_CHEST_LEADER_STACK_ROTATION_LEADER0 &&
           runtime->leaderStack[1].thing ==
               DM1_V1_CHEST_LEADER_STACK_ROTATION_LEADER1 &&
           runtime->leaderStack[2].thing ==
               DM1_V1_CHEST_LEADER_STACK_ROTATION_LEADER2;
}

static void init_runtime(RuntimePc34* runtime)
{
    int i;

    memset(runtime, 0, sizeof(*runtime));
    runtime->openChestThing = DM1_V1_CHEST_LEADER_STACK_ROTATION_OPEN_CHEST;
    runtime->focusIndex = 0;
    runtime->leaderAttributes = DM1_V1_CHEST_LEADER_STACK_ROTATION_LOAD_MASK;
    for (i = 0; i < DM1_V1_CHEST_LEADER_STACK_ROTATION_SLOT_COUNT; ++i) {
        runtime->chest[i] = none_item();
        runtime->closed[i] = none_item();
    }
    runtime->chest[0] =
        make_item(DM1_V1_CHEST_LEADER_STACK_ROTATION_ITEM0, 1);
    runtime->chest[1] =
        make_item(DM1_V1_CHEST_LEADER_STACK_ROTATION_ITEM1, 2);
    runtime->chest[2] =
        make_item(DM1_V1_CHEST_LEADER_STACK_ROTATION_ITEM2, 3);
    runtime->leaderStack[0] =
        make_item(DM1_V1_CHEST_LEADER_STACK_ROTATION_LEADER0, 4);
    runtime->leaderStack[1] =
        make_item(DM1_V1_CHEST_LEADER_STACK_ROTATION_LEADER1, 5);
    runtime->leaderStack[2] =
        make_item(DM1_V1_CHEST_LEADER_STACK_ROTATION_LEADER2, 6);
}

static int focus_zone(const RuntimePc34* runtime)
{
    return DM1_V1_CHEST_LEADER_STACK_ROTATION_C540 + runtime->focusIndex;
}

static int rotate_focus(RuntimePc34* runtime, int direction)
{
    int count = DM1_V1_CHEST_LEADER_STACK_ROTATION_STACK_COUNT;

    mouse_enable(runtime);
    runtime->focusIndex = (runtime->focusIndex + direction + count) % count;
    mouse_disable(runtime);
    return runtime->focusIndex;
}

static int commit_focused_stack_to_chest(RuntimePc34* runtime,
                                         int* outCommandSlot,
                                         int* outBefore)
{
    int chestIndex = 3 + runtime->focusIndex;
    DM1_V1_ChestLeaderStackRotationItemPc34 selected =
        runtime->leaderStack[runtime->focusIndex];

    if (is_none(selected)) {
        return 0;
    }
    if (outCommandSlot) {
        *outCommandSlot = DM1_V1_CHEST_LEADER_STACK_ROTATION_C30 + chestIndex;
    }
    if (outBefore) {
        *outBefore = runtime->chest[chestIndex].thing;
    }
    mouse_enable(runtime);
    runtime->leaderStack[runtime->focusIndex] = none_item();
    runtime->chest[chestIndex] = selected;
    runtime->leaderAttributes |= DM1_V1_CHEST_LEADER_STACK_ROTATION_LOAD_MASK;
    mouse_disable(runtime);
    return 1;
}

static int close_chest(RuntimePc34* runtime, int* head, int* endSentinel)
{
    int count = 0;
    int i;

    for (i = 0; i < DM1_V1_CHEST_LEADER_STACK_ROTATION_SLOT_COUNT; ++i) {
        runtime->closed[i] = none_item();
    }
    for (i = 0; i < DM1_V1_CHEST_LEADER_STACK_ROTATION_SLOT_COUNT; ++i) {
        if (!is_none(runtime->chest[i])) {
            runtime->closed[count++] = runtime->chest[i];
            runtime->chest[i] = none_item();
        }
    }
    runtime->openChestThing = DM1_V1_CHEST_LEADER_STACK_ROTATION_NONE;
    if (head) {
        *head = count ? runtime->closed[0].thing
                      : DM1_V1_CHEST_LEADER_STACK_ROTATION_END;
    }
    if (endSentinel) {
        *endSentinel = DM1_V1_CHEST_LEADER_STACK_ROTATION_END;
    }
    return count;
}

const DM1_V1_ChestLeaderStackRotationSpecPc34*
dm1_v1_chest_scroll_wheel_leader_stack_rotation_spec_pc34(void)
{
    return &s_spec;
}

int dm1_v1_chest_scroll_wheel_leader_stack_rotation_pc34(
    DM1_V1_ChestLeaderStackRotationProbePc34* out)
{
    RuntimePc34 runtime;
    int step;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    init_runtime(&runtime);

    out->openChestThing = runtime.openChestThing;
    copy_chest(&runtime, out->initialChest);
    copy_leader_stack(&runtime, out->initialLeaderStack);
    out->focusTrace[0] = runtime.focusIndex;
    out->focusZoneTrace[0] = focus_zone(&runtime);

    for (step = 0; step < DM1_V1_CHEST_LEADER_STACK_ROTATION_WHEEL_STEPS;
         ++step) {
        rotate_focus(&runtime, 1);
        out->focusTrace[step + 1] = runtime.focusIndex;
        out->focusZoneTrace[step + 1] = focus_zone(&runtime);
        copy_chest(&runtime, out->chestAfterWheel[step]);
        copy_leader_stack(&runtime, out->leaderStackAfterWheel[step]);
        out->chestStableBeforeMutation[step] = chest_matches_initial(&runtime);
        out->leaderStackStableBeforeMutation[step] =
            leader_stack_matches_initial(&runtime);
        ++out->partialMaskDispatches;
    }
    out->mouseEnableCount = DM1_V1_CHEST_LEADER_STACK_ROTATION_WHEEL_STEPS;
    out->mouseDisableCount = DM1_V1_CHEST_LEADER_STACK_ROTATION_WHEEL_STEPS;

    out->firstMutationStep =
        DM1_V1_CHEST_LEADER_STACK_ROTATION_WHEEL_STEPS;
    out->firstMutationZone = focus_zone(&runtime);
    out->firstMutationLeaderThing = runtime.leaderStack[runtime.focusIndex].thing;
    out->commandDispatchCount = 1;
    out->f0302DispatchCount = 1;
    if (!commit_focused_stack_to_chest(&runtime,
                                       &out->firstMutationCommandSlot,
                                       &out->firstMutationChestBefore)) {
        return 0;
    }
    ++out->mouseEnableCount;
    ++out->mouseDisableCount;
    out->f0298RemoveCount = 1;
    out->f0301WriteCount = 1;
    out->f0297PutCount = 0;
    out->loadMaskAfterMutation = runtime.leaderAttributes;
    copy_chest(&runtime, out->chestAfterMutation);
    copy_leader_stack(&runtime, out->leaderStackAfterMutation);

    out->closeCount = close_chest(&runtime, &out->closedHead,
                                  &out->closeEndSentinel);
    for (step = 0; step < DM1_V1_CHEST_LEADER_STACK_ROTATION_SLOT_COUNT;
         ++step) {
        out->closedSlots[step] = runtime.closed[step].thing;
    }
    out->openChestAfterClose = runtime.openChestThing;
    out->g0425ClearedAfterClose = chest_matches_initial(&runtime) == 0 &&
                                  is_none(runtime.chest[0]);
    return runtime.screenUpdateDepth == 0;
}
