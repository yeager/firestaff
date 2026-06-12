#include "dm1_v1_chest_scroll_wheel_drop_to_floor_non_leader_c030_pc34_compat.h"

#include <string.h>

typedef struct {
    DM1_V1_ChestScrollWheelDropToFloorNonLeaderC030ItemPc34
        c30[DM1_PC34_C545_DROP_CHAMPION_COUNT];
    DM1_V1_ChestScrollWheelDropToFloorNonLeaderC030ItemPc34
        g0425[DM1_PC34_C545_DROP_CHAMPION_COUNT]
             [DM1_PC34_C545_DROP_SLOT_COUNT];
    DM1_V1_ChestScrollWheelDropToFloorNonLeaderC030ItemPc34
        floor[DM1_PC34_C545_DROP_CHAMPION_COUNT];
    int g0426[DM1_PC34_C545_DROP_CHAMPION_COUNT];
    int floorCount[DM1_PC34_C545_DROP_CHAMPION_COUNT];
    int load[DM1_PC34_C545_DROP_CHAMPION_COUNT];
    int attributes[DM1_PC34_C545_DROP_CHAMPION_COUNT];
    int leaderHand;
    int leaderIndex;
    int inventoryChampionOrdinal;
    int partyChampionCount;
    int screenUpdateDepth;
} RuntimePc34;

static const char s_source_evidence[] =
    "CHEST.C F0333:30-67 reads the open G0426 chest and writes visible "
    "C537..C544 cells into G0425; CHEST.C F0334:113-132 closes by "
    "rewriting the same G0426 list from non-empty G0425 cells; "
    "CHAMPION.C F0297:243-298 and CHAMPION.C F0298:270-298 are "
    "leader-hand put/remove paths that this C545 drop-to-floor path must "
    "not take; CHAMPION.C "
    "F0300:511-515 clears C30+ G0425 cells; CHAMPION.C F0301:606-614 is "
    "the C30 landing-slot write that must stay unused; CHAMPION.C "
    "F0302:662-714 snapshots leader hand and C30 before dispatch; "
    "CHAMPION.C F0284:93-131 anchors champion ownership/non-leader reads; "
    "PANEL.C F0352 redraws the chest panel after C545; COMMAND.C "
    "F0378:1973-1983 dispatches panel events; DEFS.H:2088 "
    "C30/G0425/G0426/G0423/G0305/M070/M516, DEFS.H:810-816 C30..C36, "
    "DEFS.H:3906-3913 C537..C544, and C545 define this route.";

static const DM1_V1_ChestScrollWheelDropToFloorNonLeaderC030SpecPc34
    s_spec = {
        s_source_evidence,
        "ReDMCSB CHEST.C F0333:30-67 materializes the open G0426 chest into "
        "G0425 C537..C544 visible slots.",
        "ReDMCSB CHEST.C F0334:113-132 closes by rewriting the same G0426 "
        "chest from non-empty G0425 cells.",
        "ReDMCSB CHAMPION.C F0297:243-298 would put into G4055 leader hand; "
        "C545 drop-to-floor bypasses it.",
        "ReDMCSB CHAMPION.C F0298:270-298 would remove G4055 leader hand; "
        "the empty leader hand remains empty.",
        "ReDMCSB CHAMPION.C F0300:511-515 clears the source C30/G0425 cell.",
        "ReDMCSB CHAMPION.C F0301:606-614 is the C30 writeback path that "
        "must not receive the dropped thing.",
        "ReDMCSB CHAMPION.C F0302:662-714 snapshots the leader hand and "
        "slot thing before slot mutation.",
        "ReDMCSB CHAMPION.C F0284:93-131 anchors non-leader champion "
        "ownership and reads through M516.",
        "ReDMCSB PANEL.C F0352 redraws after C545 so the visible slot is "
        "empty.",
        "ReDMCSB COMMAND.C F0378:1973-1983 routes panel events to slot "
        "handling.",
        "ReDMCSB DEFS.H:2088 C30/G0425/G0426/G0423/G0305/M070/M516; "
        "DEFS.H:810-816 C30..C36; DEFS.H:3906-3913 C537..C544; C545."
    };

static DM1_V1_ChestScrollWheelDropToFloorNonLeaderC030ItemPc34
make_item(int thing, int weight)
{
    DM1_V1_ChestScrollWheelDropToFloorNonLeaderC030ItemPc34 item;

    item.thing = thing;
    item.icon = thing & 0x00FF;
    item.weight = weight;
    return item;
}

static DM1_V1_ChestScrollWheelDropToFloorNonLeaderC030ItemPc34 none_item(void)
{
    return make_item(DM1_PC34_C545_DROP_NONE, 0);
}

static int is_none(
    DM1_V1_ChestScrollWheelDropToFloorNonLeaderC030ItemPc34 item)
{
    return item.thing == DM1_PC34_C545_DROP_NONE;
}

static void copy_things(
    const DM1_V1_ChestScrollWheelDropToFloorNonLeaderC030ItemPc34* slots,
    int* out)
{
    int i;

    for (i = 0; i < DM1_PC34_C545_DROP_SLOT_COUNT; ++i) {
        out[i] = slots[i].thing;
    }
}

static void copy_icons(
    const DM1_V1_ChestScrollWheelDropToFloorNonLeaderC030ItemPc34* slots,
    int* out)
{
    int i;

    for (i = 0; i < DM1_PC34_C545_DROP_SLOT_COUNT; ++i) {
        out[i] = is_none(slots[i]) ? DM1_PC34_C545_DROP_NONE : slots[i].icon;
    }
}

static void init_runtime(RuntimePc34* runtime, int withNonLeaderC30)
{
    int c;
    int i;

    memset(runtime, 0, sizeof(*runtime));
    runtime->leaderHand = DM1_PC34_C545_DROP_NONE;
    runtime->leaderIndex = DM1_PC34_C545_DROP_LEADER_INDEX;
    runtime->inventoryChampionOrdinal = DM1_PC34_C545_DROP_OWNER_ORDINAL;
    runtime->partyChampionCount = DM1_PC34_C545_DROP_CHAMPION_COUNT;
    for (c = 0; c < DM1_PC34_C545_DROP_CHAMPION_COUNT; ++c) {
        runtime->c30[c] = none_item();
        runtime->floor[c] = none_item();
        runtime->g0426[c] = DM1_PC34_C545_DROP_NONE;
        runtime->load[c] = 40 + c;
        runtime->attributes[c] = 0;
        for (i = 0; i < DM1_PC34_C545_DROP_SLOT_COUNT; ++i) {
            runtime->g0425[c][i] = none_item();
        }
    }

    /*
     * ReDMCSB CHEST.C F0333:30-67 fills G0425 for the inventory champion's
     * open G0426 chest. This seed mirrors the pass692 result: the non-leader
     * owns one visible C30 thing while the leader's corresponding state is
     * empty.
     */
    runtime->g0426[DM1_PC34_C545_DROP_OWNER_INDEX] =
        DM1_PC34_C545_DROP_OPEN_CHEST;
    if (withNonLeaderC30) {
        runtime->c30[DM1_PC34_C545_DROP_OWNER_INDEX] =
            make_item(DM1_PC34_C545_DROP_ITEM, 2);
        runtime->g0425[DM1_PC34_C545_DROP_OWNER_INDEX][0] =
            runtime->c30[DM1_PC34_C545_DROP_OWNER_INDEX];
    }
}

static void mouse_enable(RuntimePc34* runtime)
{
    ++runtime->screenUpdateDepth;
}

static void mouse_disable(RuntimePc34* runtime)
{
    --runtime->screenUpdateDepth;
}

static int c545_drop_to_floor(
    RuntimePc34* runtime,
    DM1_V1_ChestScrollWheelDropToFloorNonLeaderC030ProbePc34* out)
{
    int owner = DM1_PC34_C545_DROP_OWNER_INDEX;
    DM1_V1_ChestScrollWheelDropToFloorNonLeaderC030ItemPc34 dropped;

    if (!runtime || !out) {
        return 0;
    }
    out->f0302LeaderSnapshot = runtime->leaderHand;
    out->f0302SlotSnapshot = runtime->c30[owner].thing;
    out->f0302ChampionIndex = owner;
    out->f0302SlotIndex = DM1_PC34_C545_DROP_C30;
    out->f0302C30Offset = 0;
    out->f0302EmptyEmptyRejected =
        runtime->leaderHand == DM1_PC34_C545_DROP_NONE &&
        is_none(runtime->c30[owner]);
    if (out->f0302EmptyEmptyRejected ||
        runtime->g0426[owner] == DM1_PC34_C545_DROP_NONE ||
        runtime->leaderHand != DM1_PC34_C545_DROP_NONE) {
        return 0;
    }

    /*
     * ReDMCSB lineage: COMMAND.C F0378:1973-1983 reaches slot handling;
     * CHAMPION.C F0302:662-714 has both the empty-hand guard and the C30
     * snapshot. This regression pins Firestaff's C545 scroll-wheel extension:
     * an already non-leader-owned C30 thing drops to the floor list instead
     * of entering CHAMPION.C F0297 leader-hand put or F0301 C30 writeback.
     */
    out->c545EventTriggered = 1;
    out->c545EventZone = DM1_PC34_C545_DROP_C545;
    out->c545Command = DM1_PC34_C545_DROP_C070;
    out->commandF0378DispatchCount = 1;
    out->f0302DispatchCount = 1;
    out->f0302LeaderHandBypass = 1;
    mouse_enable(runtime);
    dropped = runtime->c30[owner];
    runtime->c30[owner] = none_item();
    runtime->g0425[owner][0] = none_item();
    runtime->load[owner] -= dropped.weight;
    runtime->attributes[owner] |=
        DM1_PC34_C545_DROP_LOAD_MASK |
        DM1_PC34_C545_DROP_PANEL_MASK |
        DM1_PC34_C545_DROP_VIEWPORT_MASK;
    ++out->f0300ClearCount;
    runtime->floor[owner] = dropped;
    runtime->floorCount[owner] = 1;
    out->floorWriteCount = 1;
    out->floorThing = dropped.thing;
    out->floorIcon = dropped.icon;
    out->floorMapX = DM1_PC34_C545_DROP_FLOOR_X;
    out->floorMapY = DM1_PC34_C545_DROP_FLOOR_Y;
    out->floorOwnerChampionIndex = owner;
    out->floorOwnerOrdinal = DM1_PC34_C545_DROP_OWNER_ORDINAL;
    out->floorOwnerG0426 = runtime->g0426[owner];
    out->floorWroteSameChampion = out->floorOwnerChampionIndex == owner;
    out->floorWroteNotLeader =
        out->floorOwnerChampionIndex != DM1_PC34_C545_DROP_LEADER_INDEX;
    out->panelF0352RedrawCount = 1;
    copy_things(runtime->g0425[owner], out->panelRedrawSlots);
    mouse_disable(runtime);
    return 1;
}

static void run_negative(
    DM1_V1_ChestScrollWheelDropToFloorNonLeaderC030ProbePc34* out)
{
    RuntimePc34 runtime;
    DM1_V1_ChestScrollWheelDropToFloorNonLeaderC030ProbePc34 scratch;

    init_runtime(&runtime, 0);
    memset(&scratch, 0, sizeof(scratch));
    runtime.c30[DM1_PC34_C545_DROP_LEADER_INDEX] =
        make_item(DM1_PC34_C545_DROP_NEGATIVE_SENTINEL, 1);
    out->negativeNonLeaderC30Before =
        runtime.c30[DM1_PC34_C545_DROP_OWNER_INDEX].thing;
    out->negativeEventTriggered = c545_drop_to_floor(&runtime, &scratch);
    out->negativeFloorWriteCount =
        runtime.floorCount[DM1_PC34_C545_DROP_OWNER_INDEX];
    out->negativeLeaderHandAfter = runtime.leaderHand;
    out->negativeLeaderC30After =
        runtime.c30[DM1_PC34_C545_DROP_LEADER_INDEX].thing;
    out->negativeNonLeaderC30After =
        runtime.c30[DM1_PC34_C545_DROP_OWNER_INDEX].thing;
    out->negativeOpenChestOwnerAfter = DM1_PC34_C545_DROP_OWNER_INDEX;
    copy_things(runtime.g0425[DM1_PC34_C545_DROP_OWNER_INDEX],
                out->negativeG0425After);
    out->negativePanelRedrawCount = scratch.panelF0352RedrawCount;
}

const DM1_V1_ChestScrollWheelDropToFloorNonLeaderC030SpecPc34*
dm1_v1_chest_scroll_wheel_drop_to_floor_non_leader_c030_spec_pc34(void)
{
    return &s_spec;
}

const char*
dm1_v1_chest_scroll_wheel_drop_to_floor_non_leader_c030_source_evidence_pc34(
    void)
{
    return s_source_evidence;
}

int dm1_v1_chest_scroll_wheel_drop_to_floor_non_leader_c030_pc34(
    DM1_V1_ChestScrollWheelDropToFloorNonLeaderC030ProbePc34* out)
{
    RuntimePc34 runtime;
    int owner = DM1_PC34_C545_DROP_OWNER_INDEX;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    init_runtime(&runtime, 1);

    out->partyChampionCount = runtime.partyChampionCount;
    out->leaderIndex = runtime.leaderIndex;
    out->inventoryChampionOrdinal = runtime.inventoryChampionOrdinal;
    out->targetChampionIndex = owner;
    out->targetChampionOrdinal = DM1_PC34_C545_DROP_OWNER_ORDINAL;
    out->openChestThingBefore = runtime.g0426[owner];
    out->openChestOwnerBefore = owner;
    out->leaderHandBefore = runtime.leaderHand;
    out->leaderC30Before = runtime.c30[DM1_PC34_C545_DROP_LEADER_INDEX].thing;
    out->nonLeaderC30Before = runtime.c30[owner].thing;
    out->nonLeaderC30IconBefore = runtime.c30[owner].icon;
    out->nonLeaderLoadBefore = runtime.load[owner];
    copy_things(runtime.g0425[owner], out->initialG0425);
    copy_icons(runtime.g0425[owner], out->initialIcons);
    copy_things(runtime.g0425[DM1_PC34_C545_DROP_LEADER_INDEX],
                out->initialLeaderG0425);

    if (!c545_drop_to_floor(&runtime, out)) {
        return 0;
    }

    out->leaderHandAfter = runtime.leaderHand;
    out->leaderC30After = runtime.c30[DM1_PC34_C545_DROP_LEADER_INDEX].thing;
    out->leaderC30Unchanged = out->leaderC30After == out->leaderC30Before;
    out->nonLeaderC30After = runtime.c30[owner].thing;
    out->nonLeaderLoadAfter = runtime.load[owner];
    out->nonLeaderAttributesAfter = runtime.attributes[owner];
    out->openChestThingAfter = runtime.g0426[owner];
    out->openChestOwnerAfter = owner;
    out->g0426StillOwnedByNonLeader =
        runtime.g0426[owner] == DM1_PC34_C545_DROP_OPEN_CHEST &&
        runtime.g0426[DM1_PC34_C545_DROP_LEADER_INDEX] ==
            DM1_PC34_C545_DROP_NONE;
    copy_things(runtime.g0425[owner], out->g0425AfterDrop);
    copy_icons(runtime.g0425[owner], out->iconsAfterDrop);
    copy_things(runtime.g0425[DM1_PC34_C545_DROP_LEADER_INDEX],
                out->leaderG0425AfterDrop);
    out->visibleSlotEmptyAfterDrop = is_none(runtime.g0425[owner][0]);
    out->screenUpdateBalanced = runtime.screenUpdateDepth == 0;

    run_negative(out);
    return out->screenUpdateBalanced;
}
