#include "dm1_v1_chest_scroll_wheel_pickup_non_leader_runtime_pc34_compat.h"

#include <string.h>

typedef struct {
    DM1_V1_ChestScrollWheelPickupNonLeaderItemPc34
        chest[DM1_PC34_NL_CHEST_SLOT_COUNT];
    DM1_V1_ChestScrollWheelPickupNonLeaderItemPc34
        closed[DM1_PC34_NL_CHEST_SLOT_COUNT];
    DM1_V1_ChestScrollWheelPickupNonLeaderItemPc34
        leaderStack[DM1_PC34_NL_LEADER_STACK_COUNT];
    DM1_V1_ChestScrollWheelPickupNonLeaderItemPc34
        championC30[DM1_PC34_NL_CHAMPION_COUNT];
    int championLoad[DM1_PC34_NL_CHAMPION_COUNT];
    int championAttributes[DM1_PC34_NL_CHAMPION_COUNT];
    int openChestThing;
    int partyChampionCount;
    int inventoryChampionOrdinal;
    int leaderIndex;
    int screenUpdateDepth;
} RuntimePc34;

static const char s_source_evidence[] =
    "CHEST.C F0333:30-67; CHEST.C F0334:113-132; "
    "CHAMPION.C F0297:243-298; CHAMPION.C F0298:270-298; "
    "CHAMPION.C F0300:511-515; CHAMPION.C F0301:606-614; "
    "CHAMPION.C F0302:662-714; CHAMPION.C F0284:93-131; "
    "PANEL.C F0344:1895-1944; PANEL.C F0345:1946-1999; "
    "PANEL.C F0352; COMMAND.C F0378:1973-1983; "
    "COMMAND.C F0359:1985-1990; MOUSE.C F0077:97-126; "
    "MOUSE.C F0078:128-168; OBJECT.C F0033:147-212; "
    "BLITMASK.C F0133:30-33; DEFS.H:2088; DEFS.H:810-816; "
    "DEFS.H:3906-3913; C30/G0425/G0426/G0423/G0305/M070/M516";

static const DM1_V1_ChestScrollWheelPickupNonLeaderSpecPc34 s_spec = {
    s_source_evidence,
    "ReDMCSB CHEST.C F0333:30-67 opens and materializes C537..C544 in G0425.",
    "ReDMCSB CHEST.C F0334:113-132 closes G0426 and rewrites only non-empty "
    "visible cells.",
    "ReDMCSB CHAMPION.C F0297:243-298 leader-hand put path is skipped here.",
    "ReDMCSB CHAMPION.C F0298:270-298 leader-hand remove path is skipped "
    "because the occupied leader hand is preserved.",
    "ReDMCSB CHAMPION.C F0300:511-515 clears a C30+ G0425 slot.",
    "ReDMCSB CHAMPION.C F0301:606-614 writes the picked thing into the "
    "non-leader C30 landing slot.",
    "ReDMCSB CHAMPION.C F0302:662-714 dispatches slot-box commands after "
    "empty/allowed-slot guards.",
    "ReDMCSB CHAMPION.C F0284:93-131 rotates party cells and moves the "
    "leader pointer.",
    "ReDMCSB PANEL.C F0344:1895-1944 panel click route.",
    "ReDMCSB PANEL.C F0345:1946-1999 per-cell highlight route.",
    "ReDMCSB PANEL.C F0352 pressing-eye route is inactive.",
    "ReDMCSB COMMAND.C F0378:1973-1983 M568/C040 chest-panel dispatch.",
    "ReDMCSB COMMAND.C F0359:1985-1990 alternate M568/C040 dispatch.",
    "ReDMCSB MOUSE.C F0077:97-126 scroll-wheel queue write.",
    "ReDMCSB MOUSE.C F0078:128-168 scroll-wheel queue read.",
    "ReDMCSB OBJECT.C F0033:147-212 supplies icon identity.",
    "ReDMCSB BLITMASK.C F0133:30-33 partial-mask dispatch.",
    "ReDMCSB DEFS.H:2088 C30/G0425/G0426/G0423/G0305/M070/M516; "
    "DEFS.H:810-816 C30..C36; DEFS.H:3906-3913 C537..C544."
};

static DM1_V1_ChestScrollWheelPickupNonLeaderItemPc34 make_item(int thing,
                                                                 int weight)
{
    DM1_V1_ChestScrollWheelPickupNonLeaderItemPc34 item;

    item.thing = thing;
    item.icon = thing & 0x00FF;
    item.weight = weight;
    return item;
}

static DM1_V1_ChestScrollWheelPickupNonLeaderItemPc34 none_item(void)
{
    return make_item(DM1_PC34_NL_NONE, 0);
}

static int is_none(DM1_V1_ChestScrollWheelPickupNonLeaderItemPc34 item)
{
    return item.thing == DM1_PC34_NL_NONE;
}

static void copy_things(
    const DM1_V1_ChestScrollWheelPickupNonLeaderItemPc34* items,
    int count,
    int* out)
{
    int i;

    for (i = 0; i < count; ++i) {
        out[i] = items[i].thing;
    }
}

static void copy_icons(
    const DM1_V1_ChestScrollWheelPickupNonLeaderItemPc34* items,
    int count,
    int* out)
{
    int i;

    for (i = 0; i < count; ++i) {
        out[i] = is_none(items[i]) ? DM1_PC34_NL_NONE : items[i].icon;
    }
}

static int count_items(
    const DM1_V1_ChestScrollWheelPickupNonLeaderItemPc34* items,
    int count)
{
    int i;
    int total = 0;

    for (i = 0; i < count; ++i) {
        if (!is_none(items[i])) {
            ++total;
        }
    }
    return total;
}

static int count_champion_c30(const RuntimePc34* runtime)
{
    return count_items(runtime->championC30, DM1_PC34_NL_CHAMPION_COUNT);
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
    const int things[DM1_PC34_NL_CHEST_SLOT_COUNT] = {
        DM1_PC34_NL_CHEST_ITEM0,
        DM1_PC34_NL_CHEST_ITEM1_PICKED,
        DM1_PC34_NL_CHEST_ITEM2,
        DM1_PC34_NL_CHEST_ITEM3,
        DM1_PC34_NL_CHEST_ITEM4,
        DM1_PC34_NL_CHEST_ITEM5,
        DM1_PC34_NL_CHEST_ITEM6,
        DM1_PC34_NL_CHEST_ITEM7
    };
    const int stack[DM1_PC34_NL_LEADER_STACK_COUNT] = {
        DM1_PC34_NL_LEADER_ITEM0,
        DM1_PC34_NL_LEADER_ITEM1,
        DM1_PC34_NL_LEADER_ITEM2,
        DM1_PC34_NL_LEADER_ITEM3
    };

    memset(runtime, 0, sizeof(*runtime));
    runtime->openChestThing = DM1_PC34_NL_OPEN_CHEST;
    runtime->partyChampionCount = DM1_PC34_NL_CHAMPION_COUNT;
    runtime->inventoryChampionOrdinal = 3;
    runtime->leaderIndex = 0;
    for (i = 0; i < DM1_PC34_NL_CHEST_SLOT_COUNT; ++i) {
        runtime->chest[i] = make_item(things[i], i + 1);
        runtime->closed[i] = none_item();
    }
    for (i = 0; i < DM1_PC34_NL_LEADER_STACK_COUNT; ++i) {
        runtime->leaderStack[i] = make_item(stack[i], i + 9);
        runtime->championC30[i] = none_item();
        runtime->championLoad[i] = 40 + i;
        runtime->championAttributes[i] = 0;
    }
}

static void rotate_leader_pointer(RuntimePc34* runtime)
{
    runtime->leaderIndex = 1;
}

static int leader_hand_stack_full(const RuntimePc34* runtime)
{
    return count_items(runtime->leaderStack,
                       DM1_PC34_NL_LEADER_STACK_COUNT) ==
           DM1_PC34_NL_LEADER_STACK_COUNT;
}

static int total_things(const RuntimePc34* runtime)
{
    return count_items(runtime->chest, DM1_PC34_NL_CHEST_SLOT_COUNT) +
           count_items(runtime->leaderStack, DM1_PC34_NL_LEADER_STACK_COUNT) +
           count_champion_c30(runtime);
}

static int pickup_from_c538_to_non_leader(RuntimePc34* runtime,
                                          int targetChampion,
                                          int wheelQueued,
                                          DM1_V1_ChestScrollWheelPickupNonLeaderProbePc34*
                                              out)
{
    DM1_V1_ChestScrollWheelPickupNonLeaderItemPc34 picked;

    if (!wheelQueued || targetChampion < 0 ||
        targetChampion >= DM1_PC34_NL_CHAMPION_COUNT ||
        !is_none(runtime->championC30[targetChampion]) ||
        is_none(runtime->chest[1])) {
        return 0;
    }

    mouse_enable(runtime);
    picked = runtime->chest[1];
    runtime->chest[1] = none_item();
    ++out->f0300ClearCount;
    runtime->championC30[targetChampion] = picked;
    runtime->championLoad[targetChampion] += picked.weight;
    runtime->championAttributes[targetChampion] |=
        DM1_PC34_NL_LOAD_MASK | DM1_PC34_NL_PANEL_MASK;
    ++out->f0301WriteCount;
    mouse_disable(runtime);
    return 1;
}

static int close_chest(RuntimePc34* runtime, int* skipped)
{
    int i;
    int count = 0;

    if (skipped) {
        *skipped = 0;
    }
    for (i = 0; i < DM1_PC34_NL_CHEST_SLOT_COUNT; ++i) {
        runtime->closed[i] = none_item();
    }
    for (i = 0; i < DM1_PC34_NL_CHEST_SLOT_COUNT; ++i) {
        if (is_none(runtime->chest[i])) {
            if (skipped) {
                ++*skipped;
            }
            continue;
        }
        runtime->closed[count++] = runtime->chest[i];
        runtime->chest[i] = none_item();
    }
    runtime->openChestThing = DM1_PC34_NL_NONE;
    return count;
}

static int closed_chain_contains_picked(const RuntimePc34* runtime)
{
    int i;

    for (i = 0; i < DM1_PC34_NL_CHEST_SLOT_COUNT; ++i) {
        if (runtime->closed[i].thing == DM1_PC34_NL_CHEST_ITEM1_PICKED) {
            return 1;
        }
    }
    return 0;
}

static void run_negative_case(
    DM1_V1_ChestScrollWheelPickupNonLeaderProbePc34* out)
{
    RuntimePc34 runtime;

    init_runtime(&runtime);
    runtime.championC30[2] = make_item(DM1_PC34_NL_NEGATIVE_TARGET_ITEM, 5);
    out->negativeWheelQueued = 0;
    out->negativeTargetBefore = runtime.championC30[2].thing;
    out->negativeChestC538Before = runtime.chest[1].thing;
    out->negativeMutated = pickup_from_c538_to_non_leader(&runtime, 2, 0, out);
    out->negativeTargetAfter = runtime.championC30[2].thing;
    out->negativeChestC538After = runtime.chest[1].thing;
    out->negativeF0301WriteCount = out->f0301WriteCount;
}

const DM1_V1_ChestScrollWheelPickupNonLeaderSpecPc34*
dm1_v1_chest_scroll_wheel_pickup_non_leader_runtime_spec_pc34(void)
{
    return &s_spec;
}

const char*
dm1_v1_chest_scroll_wheel_pickup_non_leader_runtime_source_evidence_pc34(void)
{
    return s_source_evidence;
}

int dm1_v1_chest_scroll_wheel_pickup_non_leader_runtime_pc34(
    DM1_V1_ChestScrollWheelPickupNonLeaderProbePc34* out)
{
    RuntimePc34 runtime;
    int skipped = 0;
    int targetChampion = 2;
    int targetLoadBefore;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    init_runtime(&runtime);

    out->openChestThing = runtime.openChestThing;
    copy_things(runtime.chest, DM1_PC34_NL_CHEST_SLOT_COUNT,
                out->initialChest);
    copy_icons(runtime.chest, DM1_PC34_NL_CHEST_SLOT_COUNT,
               out->initialIcons);
    copy_things(runtime.leaderStack, DM1_PC34_NL_LEADER_STACK_COUNT,
                out->initialLeaderStack);
    out->partyChampionCountBefore = runtime.partyChampionCount;
    out->inventoryChampionOrdinalBefore = runtime.inventoryChampionOrdinal;
    out->leaderIndexBeforeRotation = runtime.leaderIndex;
    out->m570ChainLengthBefore =
        count_items(runtime.chest, DM1_PC34_NL_CHEST_SLOT_COUNT);
    out->m516ThingCountBefore = count_champion_c30(&runtime);
    out->totalThingCountBefore = total_things(&runtime);

    rotate_leader_pointer(&runtime);
    out->leaderIndexAfterRotation = runtime.leaderIndex;
    out->targetChampionIndex = targetChampion;
    out->targetSlotIndex = DM1_PC34_NL_C30;
    out->targetSlotBefore = runtime.championC30[targetChampion].thing;
    targetLoadBefore = runtime.championLoad[targetChampion];
    out->targetLoadBefore = targetLoadBefore;

    out->wheelQueuedByF0077 = 1;
    out->wheelReadByF0078 = 1;
    out->wheelQueueDepthAfterRead = 0;
    out->wheelTargetZone = DM1_PC34_NL_C538;
    out->panelF0344DispatchCount = 1;
    out->panelF0345HighlightCount = 2;
    out->highlightTrace[0] = DM1_PC34_NL_C537;
    out->highlightTrace[1] = DM1_PC34_NL_C538;
    out->highlightTrace[2] = DM1_PC34_NL_C538;
    out->partialMaskDispatches = 2;
    out->commandF0378Dispatch = 1;
    out->commandF0359Dispatch = 1;
    out->commandM568 = DM1_PC34_NL_PANEL_M568;
    out->commandC040 = DM1_PC34_NL_COMMAND_C040;
    out->commandSlotBoxIndex = DM1_PC34_NL_SLOT_BOX_C39;
    out->championSlotIndexFromDispatch = DM1_PC34_NL_C31;
    out->chestOffsetFromDispatch = 1;
    out->dispatchReadThing = runtime.chest[1].thing;
    out->dispatchReadIcon = runtime.chest[1].icon;
    out->leaderHandWasFull = leader_hand_stack_full(&runtime);
    out->skippedOccupiedLeaderHand = out->leaderHandWasFull;
    out->f0302DispatchCount = 1;
    out->pressingEyeRouteCount = 0;

    if (!pickup_from_c538_to_non_leader(&runtime, targetChampion, 1, out)) {
        return 0;
    }
    out->drawStateChampion = targetChampion;
    out->targetSlotAfter = runtime.championC30[targetChampion].thing;
    out->targetSlotIconAfter = runtime.championC30[targetChampion].icon;
    out->targetLoadAfter = runtime.championLoad[targetChampion];
    out->targetAttributesAfter = runtime.championAttributes[targetChampion];
    copy_things(runtime.chest, DM1_PC34_NL_CHEST_SLOT_COUNT,
                out->chestAfterPickup);
    copy_icons(runtime.chest, DM1_PC34_NL_CHEST_SLOT_COUNT,
               out->iconsAfterPickup);
    copy_things(runtime.leaderStack, DM1_PC34_NL_LEADER_STACK_COUNT,
                out->leaderStackAfterPickup);

    out->closeCount = close_chest(&runtime, &skipped);
    out->closeSkippedNoneEntries = skipped;
    out->closeHead = out->closeCount ? runtime.closed[0].thing
                                     : DM1_PC34_NL_END_OF_LIST;
    out->closeEndSentinel = DM1_PC34_NL_END_OF_LIST;
    copy_things(runtime.closed, DM1_PC34_NL_CHEST_SLOT_COUNT,
                out->closedSlots);
    copy_icons(runtime.closed, DM1_PC34_NL_CHEST_SLOT_COUNT,
               out->closedIcons);
    out->pickedItemInClosedChain = closed_chain_contains_picked(&runtime);
    out->openChestAfterClose = runtime.openChestThing;
    out->g0425ClearedAfterClose =
        count_items(runtime.chest, DM1_PC34_NL_CHEST_SLOT_COUNT) == 0;
    copy_things(runtime.leaderStack, DM1_PC34_NL_LEADER_STACK_COUNT,
                out->leaderStackAfterClose);

    out->partyChampionCountAfter = runtime.partyChampionCount;
    out->m570ChainLengthAfter = out->closeCount;
    out->m516ThingCountAfter = count_champion_c30(&runtime);
    out->totalThingCountAfter = total_things(&runtime) + out->closeCount;
    out->screenUpdateBalanced = runtime.screenUpdateDepth == 0;

    run_negative_case(out);
    return out->screenUpdateBalanced;
}
