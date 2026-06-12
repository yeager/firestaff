#include "dm1_v1_chest_c545_non_leader_hand_to_mid_cast_leader_pc34_compat.h"

#include <string.h>

typedef struct {
    int thing;
    int icon;
    int weight;
    int closed;
} ItemPc34;

typedef struct {
    ItemPc34 leaderHand;
    ItemPc34 actionHand[DM1_PC34_C545_MIDCAST_CHAMPION_COUNT];
    ItemPc34 chest[DM1_PC34_C545_MIDCAST_SLOT_COUNT];
    ItemPc34 closedChest[DM1_PC34_C545_MIDCAST_SLOT_COUNT];
    int openChestThing;
    int leaderIndex;
    int inventoryChampionIndex;
    int inventoryChampionOrdinal;
    int partyChampionCount;
    int actingChampionOrdinal;
    int leaderMidCast;
    int leaderSpellRuneCount;
    int load[DM1_PC34_C545_MIDCAST_CHAMPION_COUNT];
    int attributes[DM1_PC34_C545_MIDCAST_CHAMPION_COUNT];
    int screenUpdateDepth;
} RuntimePc34;

static const char s_source_evidence[] =
    "CHEST.C F0333:30-67 materializes G0425 visible chest slots; "
    "CHEST.C F0334:113-132 compacts non-empty G0425 slots while closing G0426; "
    "CHAMPION.C F0284:93-131 preserves champion ownership across direction changes; "
    "CHAMPION.C F0297:243-298 puts the selected non-leader hand object into the leader hand; "
    "CHAMPION.C F0298:270-298 is skipped because the leader hand starts empty; "
    "CHAMPION.C F0300:511-515 and F0300:541-550 clear the non-leader action hand and close a scroll; "
    "CHAMPION.C F0301:606-614 is skipped because no previous leader-hand object is written back; "
    "CHAMPION.C F0302:662-714 snapshots the leader hand and selected C30/C01 slot before dispatch; "
    "PANEL.C F0344:1493-1561 and F0345:1563-1580 stay inactive; "
    "PANEL.C F0352:2111-2159 stays inactive while the C545 mouth route runs; "
    "COMMAND.C F0359:1985-1990 rejects resurrect-panel commands once the leader hand is full; "
    "DEFS.H:2088 C10, 5876-5881 G0423/G0425/G0426, 810-817 C30..C37, "
    "3906-3914 C537..C545, plus G0305/M070/M516 context.";

static const DM1_V1_ChestC545NonLeaderHandToMidCastLeaderSpecPc34 s_spec = {
    s_source_evidence,
    "ReDMCSB CHEST.C F0333 lines 30-67 open/materialize G0425.",
    "ReDMCSB CHEST.C F0334 lines 113-132 close/compact G0425.",
    "ReDMCSB CHAMPION.C F0284 lines 93-131 party direction ownership.",
    "ReDMCSB CHAMPION.C F0297 lines 243-298 leader-hand put.",
    "ReDMCSB CHAMPION.C F0298 lines 270-298 leader-hand remove.",
    "ReDMCSB CHAMPION.C F0300 lines 511-515 and 541-550 slot clear.",
    "ReDMCSB CHAMPION.C F0301 lines 606-614 slot write.",
    "ReDMCSB CHAMPION.C F0302 lines 662-714 slot-box dispatch.",
    "ReDMCSB PANEL.C F0344 lines 1493-1561 food/water bar route.",
    "ReDMCSB PANEL.C F0345 lines 1563-1580 food/water panel route.",
    "ReDMCSB PANEL.C F0352 lines 2111-2159 eye route.",
    "ReDMCSB COMMAND.C F0359 lines 1985-1990 resurrect-panel guard.",
    "ReDMCSB DEFS.H:2088, 5876-5881, 810-817, 3906-3914 C30/G0425/G0426/G0423/G0305/M070/M516/C537..C545."
};

static ItemPc34 make_item(int thing, int weight)
{
    ItemPc34 item;

    item.thing = thing;
    item.icon = thing == DM1_PC34_C545_MIDCAST_NONE ? 0 : (thing & 0x00FF);
    item.weight = weight;
    item.closed = 0;
    return item;
}

static ItemPc34 none_item(void)
{
    return make_item(DM1_PC34_C545_MIDCAST_NONE, 0);
}

static int is_none(ItemPc34 item)
{
    return item.thing == DM1_PC34_C545_MIDCAST_NONE;
}

static void copy_things(const ItemPc34* items, int* out)
{
    int i;

    for (i = 0; i < DM1_PC34_C545_MIDCAST_SLOT_COUNT; ++i) {
        out[i] = items[i].thing;
    }
}

static void init_runtime(RuntimePc34* runtime)
{
    int i;

    memset(runtime, 0, sizeof(*runtime));
    runtime->leaderHand = none_item();
    runtime->openChestThing = DM1_PC34_C545_MIDCAST_OPEN_CHEST;
    runtime->leaderIndex = DM1_PC34_C545_MIDCAST_LEADER_INDEX;
    runtime->inventoryChampionIndex = DM1_PC34_C545_MIDCAST_OWNER_INDEX;
    runtime->inventoryChampionOrdinal = DM1_PC34_C545_MIDCAST_OWNER_ORDINAL;
    runtime->partyChampionCount = DM1_PC34_C545_MIDCAST_CHAMPION_COUNT;
    runtime->actingChampionOrdinal = DM1_PC34_C545_MIDCAST_LEADER_ORDINAL;
    runtime->leaderMidCast = 1;
    runtime->leaderSpellRuneCount = 2;
    for (i = 0; i < DM1_PC34_C545_MIDCAST_CHAMPION_COUNT; ++i) {
        runtime->actionHand[i] = none_item();
        runtime->load[i] = 40 + i;
        runtime->attributes[i] = 0;
    }
    runtime->actionHand[DM1_PC34_C545_MIDCAST_OWNER_INDEX] =
        make_item(DM1_PC34_C545_MIDCAST_SCROLL, 7);
    runtime->load[DM1_PC34_C545_MIDCAST_OWNER_INDEX] +=
        runtime->actionHand[DM1_PC34_C545_MIDCAST_OWNER_INDEX].weight;
    for (i = 0; i < DM1_PC34_C545_MIDCAST_SLOT_COUNT; ++i) {
        runtime->chest[i] =
            make_item(DM1_PC34_C545_MIDCAST_CHEST_BASE + i, 3 + i);
        runtime->closedChest[i] = none_item();
    }
}

static void enable_screen_update(RuntimePc34* runtime)
{
    ++runtime->screenUpdateDepth;
}

static void disable_screen_update(RuntimePc34* runtime)
{
    --runtime->screenUpdateDepth;
}

static int c545_pull_non_leader_action_hand_to_leader(
    RuntimePc34* runtime,
    DM1_V1_ChestC545NonLeaderHandToMidCastLeaderProbePc34* out)
{
    ItemPc34 pulled;
    int owner;

    if (!runtime || !out) {
        return 0;
    }
    owner = runtime->inventoryChampionIndex;
    out->c545EventZone = DM1_PC34_C545_MIDCAST_C545;
    out->c545Command = DM1_PC34_C545_MIDCAST_C070;
    out->commandPanelBefore = DM1_PC34_C545_MIDCAST_M569;
    out->f0302LeaderSnapshot = runtime->leaderHand.thing;
    out->f0302SlotSnapshot = runtime->actionHand[owner].thing;
    out->f0302ChampionIndex = owner;
    out->f0302Pc34Slot = DM1_PC34_C545_MIDCAST_C30 + 1;
    out->f0302AllowedBySlotMask = 1;
    out->f0302EmptyEmptyRejected =
        is_none(runtime->leaderHand) && is_none(runtime->actionHand[owner]);
    if (!is_none(runtime->leaderHand) || out->f0302EmptyEmptyRejected ||
        runtime->openChestThing == DM1_PC34_C545_MIDCAST_NONE) {
        return 0;
    }

    /*
     * ReDMCSB CHAMPION.C F0302:688-710 snapshots leader hand and source slot,
     * then F0300 removes the slot object before F0297 puts it into G4055.
     * F0300:542-545 only clears the acting champion if the removed action hand
     * belongs to G0506; this probe keeps G0506 on the leader ordinal.
     */
    enable_screen_update(runtime);
    pulled = runtime->actionHand[owner];
    runtime->actionHand[owner] = none_item();
    runtime->load[owner] -= pulled.weight;
    runtime->attributes[owner] |=
        DM1_PC34_C545_MIDCAST_LOAD_MASK |
        DM1_PC34_C545_MIDCAST_PANEL_MASK |
        DM1_PC34_C545_MIDCAST_VIEWPORT_MASK |
        DM1_PC34_C545_MIDCAST_ACTION_HAND_MASK;
    ++out->f0300ClearCount;
    pulled.closed = 1;
    runtime->leaderHand = pulled;
    runtime->load[runtime->leaderIndex] += pulled.weight;
    runtime->attributes[runtime->leaderIndex] |=
        DM1_PC34_C545_MIDCAST_LOAD_MASK;
    ++out->f0297PutCount;
    out->panelRedrawAfterC545 = 1;
    disable_screen_update(runtime);
    return 1;
}

static int close_chest(RuntimePc34* runtime)
{
    int i;
    int count = 0;

    if (!runtime || runtime->openChestThing == DM1_PC34_C545_MIDCAST_NONE) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_C545_MIDCAST_SLOT_COUNT; ++i) {
        runtime->closedChest[i] = none_item();
    }
    for (i = 0; i < DM1_PC34_C545_MIDCAST_SLOT_COUNT; ++i) {
        if (!is_none(runtime->chest[i])) {
            runtime->closedChest[count++] = runtime->chest[i];
        }
        runtime->chest[i] = none_item();
    }
    runtime->openChestThing = DM1_PC34_C545_MIDCAST_NONE;
    return count;
}

static void run_negative(
    DM1_V1_ChestC545NonLeaderHandToMidCastLeaderProbePc34* out)
{
    RuntimePc34 runtime;
    DM1_V1_ChestC545NonLeaderHandToMidCastLeaderProbePc34 scratch;

    init_runtime(&runtime);
    runtime.leaderHand = make_item(0x7499, 5);
    memset(&scratch, 0, sizeof(scratch));
    out->negativeLeaderBusyRejected =
        c545_pull_non_leader_action_hand_to_leader(&runtime, &scratch) ? 0 : 1;
    out->negativeLeaderHandAfter = runtime.leaderHand.thing;
    out->negativeNonLeaderHandAfter =
        runtime.actionHand[DM1_PC34_C545_MIDCAST_OWNER_INDEX].thing;
    out->negativeActingChampionAfter = runtime.actingChampionOrdinal;
}

static unsigned int hash_u32(unsigned int hash, unsigned int value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        hash ^= (value >> (i * 8)) & 0xFFu;
        hash *= 16777619u;
    }
    return hash;
}

static unsigned int deterministic_hash(
    const DM1_V1_ChestC545NonLeaderHandToMidCastLeaderProbePc34* out)
{
    unsigned int hash = 2166136261u;
    int i;

    hash = hash_u32(hash, (unsigned int)out->leaderHandAfter);
    hash = hash_u32(hash, (unsigned int)out->nonLeaderActionHandAfter);
    hash = hash_u32(hash, (unsigned int)out->actingChampionOrdinalAfter);
    hash = hash_u32(hash, (unsigned int)out->leaderMidCastAfter);
    hash = hash_u32(hash, (unsigned int)out->leaderSpellRuneCountAfter);
    hash = hash_u32(hash, (unsigned int)out->f0297PutCount);
    hash = hash_u32(hash, (unsigned int)out->f0300ClearCount);
    hash = hash_u32(hash, (unsigned int)out->f0388ClearActingCount);
    hash = hash_u32(hash, (unsigned int)out->c540Preserved);
    hash = hash_u32(hash, (unsigned int)out->c541Preserved);
    hash = hash_u32(hash, (unsigned int)out->negativeLeaderBusyRejected);
    for (i = 0; i < DM1_PC34_C545_MIDCAST_SLOT_COUNT; ++i) {
        hash = hash_u32(hash, (unsigned int)out->closedChest[i]);
    }
    return hash;
}

const DM1_V1_ChestC545NonLeaderHandToMidCastLeaderSpecPc34*
dm1_v1_chest_c545_non_leader_hand_to_mid_cast_leader_spec_pc34(void)
{
    return &s_spec;
}

const char*
dm1_v1_chest_c545_non_leader_hand_to_mid_cast_leader_source_evidence_pc34(void)
{
    return s_source_evidence;
}

int dm1_v1_chest_c545_non_leader_hand_to_mid_cast_leader_run_pc34(
    DM1_V1_ChestC545NonLeaderHandToMidCastLeaderProbePc34* out)
{
    RuntimePc34 runtime;
    int owner;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    init_runtime(&runtime);
    owner = runtime.inventoryChampionIndex;

    out->sourceLockedRuntimeGate = 1;
    out->noGameDataLoad = 1;
    out->partyChampionCount = runtime.partyChampionCount;
    out->leaderIndex = runtime.leaderIndex;
    out->leaderOrdinal = DM1_PC34_C545_MIDCAST_LEADER_ORDINAL;
    out->inventoryChampionIndex = runtime.inventoryChampionIndex;
    out->inventoryChampionOrdinal = runtime.inventoryChampionOrdinal;
    out->actingChampionOrdinalBefore = runtime.actingChampionOrdinal;
    out->leaderMidCastBefore = runtime.leaderMidCast;
    out->leaderSpellRuneCountBefore = runtime.leaderSpellRuneCount;
    out->openChestThingBefore = runtime.openChestThing;
    out->leaderHandBefore = runtime.leaderHand.thing;
    out->nonLeaderActionHandBefore = runtime.actionHand[owner].thing;
    out->nonLeaderActionHandIconBefore = runtime.actionHand[owner].icon;
    out->leaderLoadBefore = runtime.load[runtime.leaderIndex];
    out->nonLeaderLoadBefore = runtime.load[owner];
    copy_things(runtime.chest, out->chestBefore);

    if (!c545_pull_non_leader_action_hand_to_leader(&runtime, out)) {
        return 0;
    }

    out->actingChampionOrdinalAfter = runtime.actingChampionOrdinal;
    out->leaderMidCastAfter = runtime.leaderMidCast;
    out->leaderSpellRuneCountAfter = runtime.leaderSpellRuneCount;
    out->openChestThingAfter = runtime.openChestThing;
    out->leaderHandAfter = runtime.leaderHand.thing;
    out->leaderHandIconAfter = runtime.leaderHand.icon;
    out->nonLeaderActionHandAfter = runtime.actionHand[owner].thing;
    out->nonLeaderActionHandClosedAfter = runtime.leaderHand.closed;
    out->leaderLoadAfter = runtime.load[runtime.leaderIndex];
    out->nonLeaderLoadAfter = runtime.load[owner];
    out->leaderAttributesAfter = runtime.attributes[runtime.leaderIndex];
    out->nonLeaderAttributesAfter = runtime.attributes[owner];
    out->commandF0359ResurrectBlockedAfterPickup =
        !is_none(runtime.leaderHand) ? 1 : 0;
    out->f0388ClearActingCount = 0;
    out->f0344FoodBarDrawCount = 0;
    out->f0345FoodWaterPanelDrawCount = 0;
    out->f0352EyePanelDrawCount = 0;
    out->screenUpdateBalanced = runtime.screenUpdateDepth == 0 ? 1 : 0;
    copy_things(runtime.chest, out->chestAfter);
    out->c540Preserved =
        out->chestAfter[3] == DM1_PC34_C545_MIDCAST_CHEST_BASE + 3 ? 1 : 0;
    out->c541Preserved =
        out->chestAfter[4] == DM1_PC34_C545_MIDCAST_CHEST_BASE + 4 ? 1 : 0;

    out->closedChestCount = close_chest(&runtime);
    copy_things(runtime.closedChest, out->closedChest);
    out->chestOrderPreserved = out->closedChestCount ==
        DM1_PC34_C545_MIDCAST_SLOT_COUNT ? 1 : 0;
    for (i = 0; i < DM1_PC34_C545_MIDCAST_SLOT_COUNT; ++i) {
        if (out->closedChest[i] != DM1_PC34_C545_MIDCAST_CHEST_BASE + i) {
            out->chestOrderPreserved = 0;
        }
    }

    run_negative(out);
    out->deterministicHash = deterministic_hash(out);
    return out->screenUpdateBalanced && out->chestOrderPreserved;
}
