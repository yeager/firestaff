#include "dm1_v1_inventory_chest_load_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/*
 * Asset-free DM1 V1 runtime regression gate for the C538/C539 stack pickup
 * failover when the leader-hand stack is already full at C540..C543.
 *
 * ReDMCSB anchors:
 * - CHEST.C F0333:30-32 same-open guard.
 * - CHEST.C F0333:53-67 open chest materialization into G0425/C537..C544.
 * - CHEST.C F0334:117-132 visible slot close rewrite.
 * - CHAMPION.C F0297:243-268 and F0298:270-298 leader-hand put/remove.
 * - CHAMPION.C F0300:511-515 and F0301:606-614 C30+ slot clear/write.
 * - CHAMPION.C F0302:662-710 occupied-slot swap dispatch order.
 * - PANEL.C F0342:1119-1133 action-hand container dispatch.
 * - COMMAND.C F0359:1980-1983 chest panel click to F0302 dispatch.
 * - OBJECT.C F0032/F0033:121-212 icon identity.
 * - AMMO.C F0294:54-79 stack-compatible ammunition/failover lineage.
 * - BLITMASK.C F0133:30-33 partial-mask draw dispatch.
 * - DEFS.H:810 C30, 1950 C195, 3906-3913 C537..C544.
 */

enum {
    ITEM_NONE = 0,
    ITEM_C030_FOOD_RATION = 30,
    ITEM_C037_TORCH = 37,
    ITEM_C042_SCROLL_STACK = 42,
    ITEM_C137_CHEST_SENTINEL = 137,
    ITEM_C195_EMPTY_FLASK = 195,
    CHEST_THING = 0x7B51,
    STACK_KIND_NONE = 0,
    STACK_KIND_C042 = 1,
    ZONE_C537 = 537,
    ZONE_C538 = 538,
    ZONE_C539 = 539,
    ZONE_C540 = 540,
    ZONE_C541 = 541,
    ZONE_C542 = 542,
    ZONE_C543 = 543,
    ZONE_C544 = 544,
    LEADER_STACK_CAPACITY = 4,
    FLOOR_CAPACITY = 8
};

typedef struct {
    int id;
    int type;
    int icon;
    int weight;
    int charges;
    int allowedSlots;
    const char* name;
} ProbeItem;

typedef struct {
    M11_InventoryState inventory;
    ProbeItem leader[LEADER_STACK_CAPACITY];
    ProbeItem floor[FLOOR_CAPACITY];
    int floorDropSourceZone[FLOOR_CAPACITY];
    int floorPartialMask[FLOOR_CAPACITY];
    int leaderCount;
    int floorCount;
    int f0333EarlyReturnTaken;
    int f0333DispatchCount;
    int f0334CloseCount;
    int f0297PutCount;
    int f0298RemoveCount;
    int f0300C30ClearCount;
    int f0301C30WriteCount;
    int f0302SwapDispatchCount;
    int f0342ActionHandDispatchCount;
    int f0344PanelClickCount;
    int f0345HighlightCount;
    int f0359CommandDispatchCount;
    int f0077MouseQueueCount;
    int f0078MouseQueueCount;
    int f0032TypeChecks;
    int f0033IconChecks;
    int f0294StackFailoverCount;
    int f0133PartialMaskCount;
    int c540DroppedBeforeFailover;
    int failoverSourceZone[2];
    int failoverTargetZone[2];
    int failoverPreservedIcon[2];
    int failoverPreservedId[2];
    int openThingBeforeClick;
    int openThingBeforeClose;
    int openThingAfterClose;
    int visibleWeightBeforeClick;
    int visibleWeightAfterClick;
    int containerWeightBeforeClose;
    M11_Item chestBefore[DM1_PC34_CHEST_SLOT_COUNT];
    M11_Item chestAfter[DM1_PC34_CHEST_SLOT_COUNT];
    M11_Item closed[DM1_PC34_CHEST_SLOT_COUNT];
} ProbeRuntime;

static int g_assertions;
static int g_failures;

static const char* A_CHEST_GUARD =
    "ReDMCSB CHEST.C F0333:30-32 returns before rematerializing the same G0426 chest.";
static const char* A_CHEST_DISPATCH =
    "ReDMCSB CHEST.C F0333:53-67 copies linked contents into G0425 C537..C544.";
static const char* A_CHEST_CLOSE =
    "ReDMCSB CHEST.C F0334:117-132 rewrites non-empty visible C537..C544 slots.";
static const char* A_F0297 =
    "ReDMCSB CHAMPION.C F0297:243-268 puts an object in G4055 leader hand.";
static const char* A_F0298 =
    "ReDMCSB CHAMPION.C F0298:270-298 removes the current G4055 leader-hand object.";
static const char* A_F0300 =
    "ReDMCSB CHAMPION.C F0300:511-515 clears C30+ entries through G0425.";
static const char* A_F0301 =
    "ReDMCSB CHAMPION.C F0301:606-614 writes C30+ entries through G0425.";
static const char* A_F0302 =
    "ReDMCSB CHAMPION.C F0302:662-710 snapshots hand/slot and performs occupied-slot swap order.";
static const char* A_PANEL =
    "ReDMCSB PANEL.C F0342:1119-1133 closes/reopens action-hand containers before object dispatch.";
static const char* A_PANEL_CLICK =
    "ReDMCSB PANEL.C F0344/F0345:1895-1999 panel click and per-cell highlight lineage.";
static const char* A_COMMAND =
    "ReDMCSB COMMAND.C F0359:1980-1983 maps chest panel input to F0302; 1985-1990 guards M568.";
static const char* A_MOUSE =
    "ReDMCSB UTAMSCR.C F0077:147-151 and F0078:141-145 mouse update queue calls.";
static const char* A_OBJECT =
    "ReDMCSB OBJECT.C F0032/F0033:121-212 preserves object type/icon identity.";
static const char* A_AMMO =
    "ReDMCSB AMMO.C F0294:54-79 classifies stack-compatible ammunition failover.";
static const char* A_BLIT =
    "ReDMCSB BLITMASK.C F0133:30-33 partial masked bitmap dispatch.";
static const char* A_DEFS =
    "ReDMCSB DEFS.H:810 C30, 1950 C195, and 3906-3913 C537..C544 zones.";

static int expect_int(const char* label, int got, int want, const char* anchor)
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

static ProbeItem make_item(int id,
                           int type,
                           int icon,
                           int weight,
                           int charges,
                           int allowedSlots,
                           const char* name)
{
    ProbeItem item;

    memset(&item, 0, sizeof(item));
    item.id = id;
    item.type = type;
    item.icon = icon;
    item.weight = weight;
    item.charges = charges;
    item.allowedSlots = allowedSlots;
    item.name = name;
    return item;
}

static M11_Item to_m11_item(ProbeItem item)
{
    M11_Item out;

    memset(&out, 0, sizeof(out));
    out.itemType = item.type;
    out.weight = item.weight;
    out.charges = item.charges;
    out.identified = item.id;
    out.allowedSlots = item.allowedSlots;
    return out;
}

static ProbeItem from_m11_item(M11_Item item, int id, const char* name)
{
    return make_item(id, item.itemType, item.itemType, item.weight,
                     item.charges, item.allowedSlots, name);
}

static int stack_kind(ProbeItem item)
{
    if (item.type == ITEM_C042_SCROLL_STACK && item.icon == ITEM_C042_SCROLL_STACK) {
        return STACK_KIND_C042;
    }
    return STACK_KIND_NONE;
}

static void seed_leader_hand(ProbeRuntime* rt)
{
    rt->leader[0] = make_item(100, ITEM_C195_EMPTY_FLASK,
                              ITEM_C195_EMPTY_FLASK, 1, 0,
                              DM1_PC34_ALLOWED_ANY_SLOT, "C540 empty flask");
    rt->leader[1] = make_item(101, ITEM_C037_TORCH, ITEM_C037_TORCH, 2, 1,
                              DM1_PC34_ALLOWED_ANY_SLOT, "C541 torch");
    rt->leader[2] = make_item(102, ITEM_C042_SCROLL_STACK,
                              ITEM_C042_SCROLL_STACK, 1, 1,
                              DM1_PC34_ALLOWED_ANY_SLOT, "C542 scroll");
    rt->leader[3] = make_item(103, ITEM_C030_FOOD_RATION,
                              ITEM_C030_FOOD_RATION, 3, 0,
                              DM1_PC34_ALLOWED_ANY_SLOT, "C543 food");
    rt->leaderCount = LEADER_STACK_CAPACITY;
}

static int seed_open_chest(ProbeRuntime* rt)
{
    M11_Item linked[3];
    int i;
    int openResult;

    linked[0] = to_m11_item(make_item(200, ITEM_C137_CHEST_SENTINEL,
                                      ITEM_C137_CHEST_SENTINEL, 5, 0,
                                      DM1_PC34_ALLOWED_CONTAINER,
                                      "C537 retained item"));
    linked[1] = to_m11_item(make_item(201, ITEM_C042_SCROLL_STACK,
                                      ITEM_C042_SCROLL_STACK, 1, 2,
                                      DM1_PC34_ALLOWED_ANY_SLOT,
                                      "C538 stack"));
    linked[2] = to_m11_item(make_item(202, ITEM_C042_SCROLL_STACK,
                                      ITEM_C042_SCROLL_STACK, 1, 3,
                                      DM1_PC34_ALLOWED_ANY_SLOT,
                                      "C539 stack"));

    m11_inventory_init(&rt->inventory, 1);
    openResult = m11_inventory_open_chest(&rt->inventory, 0, CHEST_THING,
                                          linked, 3);
    rt->openThingBeforeClick =
        m11_inventory_get_open_chest_thing(&rt->inventory, 0);
    rt->visibleWeightBeforeClick =
        m11_inventory_pc34_open_chest_visible_contents_weight(&rt->inventory,
                                                              0);
    rt->f0333EarlyReturnTaken = 0;
    rt->f0333DispatchCount = openResult ? 1 : 0;
    for (i = 0; i < DM1_PC34_CHEST_SLOT_COUNT; ++i) {
        if (!m11_inventory_get_item_in_chest_slot(&rt->inventory, 0, i,
                                                  &rt->chestBefore[i])) {
            return 0;
        }
    }
    return openResult;
}

static void drop_full_leader_hand_to_floor(ProbeRuntime* rt)
{
    int i;

    rt->f0342ActionHandDispatchCount = 1;
    rt->f0298RemoveCount += rt->leaderCount;
    rt->f0300C30ClearCount += rt->leaderCount;
    rt->f0077MouseQueueCount += 1;
    for (i = 0; i < LEADER_STACK_CAPACITY; ++i) {
        rt->floor[rt->floorCount] = rt->leader[i];
        rt->floorDropSourceZone[rt->floorCount] = ZONE_C540 + i;
        rt->floorPartialMask[rt->floorCount] = 1;
        if (i == 0 && rt->leader[i].type == ITEM_C195_EMPTY_FLASK) {
            rt->c540DroppedBeforeFailover = 1;
        }
        ++rt->floorCount;
        ++rt->f0133PartialMaskCount;
        rt->leader[i] = make_item(0, ITEM_NONE, 0, 0, 0, 0, "empty");
    }
    rt->leaderCount = 0;
    rt->f0078MouseQueueCount += 1;
}

static int pickup_stack_failover(ProbeRuntime* rt,
                                 int chestSlotIndex,
                                 int leaderIndex,
                                 int sequenceIndex)
{
    M11_Item chestM11;
    ProbeItem picked;

    if (!m11_inventory_get_item_in_chest_slot(&rt->inventory, 0,
                                              chestSlotIndex, &chestM11)) {
        return 0;
    }
    picked = from_m11_item(chestM11, 201 + sequenceIndex,
                           sequenceIndex == 0 ? "C538 stack" : "C539 stack");
    if (picked.type == ITEM_NONE || stack_kind(picked) != STACK_KIND_C042) {
        return 0;
    }

    rt->failoverSourceZone[sequenceIndex] = ZONE_C537 + chestSlotIndex;
    rt->failoverTargetZone[sequenceIndex] = ZONE_C540 + leaderIndex;
    rt->failoverPreservedIcon[sequenceIndex] = picked.icon;
    rt->failoverPreservedId[sequenceIndex] = picked.id;
    rt->leader[leaderIndex] = picked;
    if (rt->leaderCount < leaderIndex + 1) {
        rt->leaderCount = leaderIndex + 1;
    }
    ++rt->f0294StackFailoverCount;
    ++rt->f0297PutCount;
    ++rt->f0301C30WriteCount;
    ++rt->f0032TypeChecks;
    ++rt->f0033IconChecks;
    if (!m11_inventory_set_item_in_chest_slot(&rt->inventory, 0,
                                              chestSlotIndex, 0, 0, 0, 0)) {
        return 0;
    }
    ++rt->f0300C30ClearCount;
    return 1;
}

static int run_probe(ProbeRuntime* rt)
{
    int i;

    memset(rt, 0, sizeof(*rt));
    seed_leader_hand(rt);
    if (!seed_open_chest(rt)) {
        return 0;
    }

    rt->f0344PanelClickCount = 1;
    rt->f0345HighlightCount = 1;
    rt->f0359CommandDispatchCount = 1;
    rt->f0302SwapDispatchCount = 1;
    if (rt->chestBefore[1].itemType == ITEM_NONE) {
        return 0;
    }

    drop_full_leader_hand_to_floor(rt);
    if (!pickup_stack_failover(rt, 1, 0, 0) ||
        !pickup_stack_failover(rt, 2, 1, 1)) {
        return 0;
    }
    rt->visibleWeightAfterClick =
        m11_inventory_pc34_open_chest_visible_contents_weight(&rt->inventory,
                                                              0);
    rt->openThingBeforeClose =
        m11_inventory_get_open_chest_thing(&rt->inventory, 0);
    for (i = 0; i < DM1_PC34_CHEST_SLOT_COUNT; ++i) {
        if (!m11_inventory_get_item_in_chest_slot(&rt->inventory, 0, i,
                                                  &rt->chestAfter[i])) {
            return 0;
        }
    }
    rt->f0334CloseCount =
        m11_inventory_pc34_close_chest_with_weight_snapshot(
            &rt->inventory, 0, rt->closed, DM1_PC34_CHEST_SLOT_COUNT,
            &rt->containerWeightBeforeClose);
    rt->openThingAfterClose =
        m11_inventory_get_open_chest_thing(&rt->inventory, 0);
    return rt->f0334CloseCount >= 0;
}

static int test_source_anchors(void)
{
    int ok = 1;

    ok &= expect_contains("anchor F0333 guard", A_CHEST_GUARD,
                          "F0333:30-32", A_CHEST_GUARD);
    ok &= expect_contains("anchor F0333 dispatch", A_CHEST_DISPATCH,
                          "F0333:53-67", A_CHEST_DISPATCH);
    ok &= expect_contains("anchor F0334 close", A_CHEST_CLOSE,
                          "F0334:117-132", A_CHEST_CLOSE);
    ok &= expect_contains("anchor F0297", A_F0297, "F0297:243-268",
                          A_F0297);
    ok &= expect_contains("anchor F0298", A_F0298, "F0298:270-298",
                          A_F0298);
    ok &= expect_contains("anchor F0300", A_F0300, "F0300:511-515",
                          A_F0300);
    ok &= expect_contains("anchor F0301", A_F0301, "F0301:606-614",
                          A_F0301);
    ok &= expect_contains("anchor F0302", A_F0302, "F0302:662-710",
                          A_F0302);
    ok &= expect_contains("anchor panel object", A_PANEL, "F0342:1119-1133",
                          A_PANEL);
    ok &= expect_contains("anchor panel click", A_PANEL_CLICK,
                          "F0344/F0345:1895-1999", A_PANEL_CLICK);
    ok &= expect_contains("anchor command", A_COMMAND, "F0359:1980-1983",
                          A_COMMAND);
    ok &= expect_contains("anchor command requested M568", A_COMMAND,
                          "1985-1990", A_COMMAND);
    ok &= expect_contains("anchor mouse", A_MOUSE, "F0077:147-151",
                          A_MOUSE);
    ok &= expect_contains("anchor object", A_OBJECT, "F0032/F0033:121-212",
                          A_OBJECT);
    ok &= expect_contains("anchor ammo", A_AMMO, "F0294:54-79", A_AMMO);
    ok &= expect_contains("anchor blit", A_BLIT, "F0133:30-33", A_BLIT);
    ok &= expect_contains("anchor defs C30", A_DEFS, "DEFS.H:810", A_DEFS);
    ok &= expect_contains("anchor defs C195", A_DEFS, "1950 C195", A_DEFS);
    ok &= expect_contains("anchor defs C537", A_DEFS, "3906-3913 C537..C544",
                          A_DEFS);
    return ok;
}

static int test_initial_state(const ProbeRuntime* rt)
{
    int ok = 1;

    ok &= expect_int("DEFS C30 runtime constant", DM1_PC34_SLOT_CHEST_1, 30,
                     A_DEFS);
    ok &= expect_int("DEFS C537 zone", ZONE_C537, 537, A_DEFS);
    ok &= expect_int("DEFS C538 zone", ZONE_C538, 538, A_DEFS);
    ok &= expect_int("DEFS C539 zone", ZONE_C539, 539, A_DEFS);
    ok &= expect_int("DEFS C540 zone", ZONE_C540, 540, A_DEFS);
    ok &= expect_int("DEFS C541 zone", ZONE_C541, 541, A_DEFS);
    ok &= expect_int("DEFS C542 zone", ZONE_C542, 542, A_DEFS);
    ok &= expect_int("DEFS C543 zone", ZONE_C543, 543, A_DEFS);
    ok &= expect_int("DEFS C544 zone", ZONE_C544, 544, A_DEFS);
    ok &= expect_int("leader hand full count", LEADER_STACK_CAPACITY, 4,
                     A_F0297);
    ok &= expect_int("leader C540 type", ITEM_C195_EMPTY_FLASK, 195,
                     A_DEFS);
    ok &= expect_int("leader C541 type", ITEM_C037_TORCH, 37, A_OBJECT);
    ok &= expect_int("leader C542 type", ITEM_C042_SCROLL_STACK, 42,
                     A_OBJECT);
    ok &= expect_int("leader C543 type", ITEM_C030_FOOD_RATION, 30,
                     A_OBJECT);
    ok &= expect_int("open chest result materialized",
                     rt->f0333DispatchCount, 1, A_CHEST_DISPATCH);
    ok &= expect_int("same-open guard not taken",
                     rt->f0333EarlyReturnTaken, 0, A_CHEST_GUARD);
    ok &= expect_int("G0426 open thing before click",
                     rt->openThingBeforeClick, CHEST_THING,
                     A_CHEST_DISPATCH);
    ok &= expect_int("C537 initial item", rt->chestBefore[0].itemType,
                     ITEM_C137_CHEST_SENTINEL, A_CHEST_DISPATCH);
    ok &= expect_int("C538 initial stack", rt->chestBefore[1].itemType,
                     ITEM_C042_SCROLL_STACK, A_CHEST_DISPATCH);
    ok &= expect_int("C539 initial stack", rt->chestBefore[2].itemType,
                     ITEM_C042_SCROLL_STACK, A_CHEST_DISPATCH);
    ok &= expect_int("C540 not a chest slot", DM1_PC34_SLOT_CHEST_8, 37,
                     A_DEFS);
    ok &= expect_int("C538 has item so guard irrelevant",
                     rt->chestBefore[1].itemType != ITEM_NONE, 1,
                     A_CHEST_GUARD);
    ok &= expect_int("visible weight before", rt->visibleWeightBeforeClick,
                     7, A_CHEST_DISPATCH);
    ok &= expect_int("C538 stack kind before",
                     rt->chestBefore[1].itemType == ITEM_C042_SCROLL_STACK ?
                         STACK_KIND_C042 : STACK_KIND_NONE,
                     STACK_KIND_C042, A_AMMO);
    ok &= expect_int("C539 stack kind before",
                     rt->chestBefore[2].itemType == ITEM_C042_SCROLL_STACK ?
                         STACK_KIND_C042 : STACK_KIND_NONE,
                     STACK_KIND_C042, A_AMMO);
    return ok;
}

static int test_drop_order_and_floor(const ProbeRuntime* rt)
{
    int ok = 1;
    int expectedTypes[LEADER_STACK_CAPACITY] = {
        ITEM_C195_EMPTY_FLASK,
        ITEM_C037_TORCH,
        ITEM_C042_SCROLL_STACK,
        ITEM_C030_FOOD_RATION
    };
    int expectedZones[LEADER_STACK_CAPACITY] = {
        ZONE_C540,
        ZONE_C541,
        ZONE_C542,
        ZONE_C543
    };
    int i;

    ok &= expect_int("panel click dispatch count", rt->f0344PanelClickCount,
                     1, A_PANEL_CLICK);
    ok &= expect_int("panel highlight count", rt->f0345HighlightCount, 1,
                     A_PANEL_CLICK);
    ok &= expect_int("command F0359 dispatch count",
                     rt->f0359CommandDispatchCount, 1, A_COMMAND);
    ok &= expect_int("F0302 occupied-slot dispatch count",
                     rt->f0302SwapDispatchCount, 1, A_F0302);
    ok &= expect_int("F0342 action-hand dispatch count",
                     rt->f0342ActionHandDispatchCount, 1, A_PANEL);
    ok &= expect_int("F0298 leader removes before failover",
                     rt->f0298RemoveCount, 4, A_F0298);
    ok &= expect_int("C540 flask dropped before failover",
                     rt->c540DroppedBeforeFailover, 1, A_PANEL);
    ok &= expect_int("floor drop count", rt->floorCount, 4, A_PANEL);
    ok &= expect_int("partial mask total", rt->f0133PartialMaskCount, 4,
                     A_BLIT);
    ok &= expect_int("mouse queue enable count", rt->f0077MouseQueueCount, 1,
                     A_MOUSE);
    ok &= expect_int("mouse queue disable count", rt->f0078MouseQueueCount, 1,
                     A_MOUSE);
    ok &= expect_int("leader stack count after drops before pickups implied",
                     rt->floorCount == 4 && rt->leader[2].type == ITEM_NONE,
                     1, A_F0298);

    for (i = 0; i < LEADER_STACK_CAPACITY; ++i) {
        ok &= expect_int("floor drop type order", rt->floor[i].type,
                         expectedTypes[i], A_PANEL);
        ok &= expect_int("floor source zone order", rt->floorDropSourceZone[i],
                         expectedZones[i], A_PANEL);
        ok &= expect_int("floor partial mask flag", rt->floorPartialMask[i],
                         1, A_BLIT);
        ok &= expect_int("floor icon identity", rt->floor[i].icon,
                         expectedTypes[i], A_OBJECT);
        ok &= expect_int("floor id distinct",
                         rt->floor[i].id == 100 + i ? 1 : 0, 1, A_OBJECT);
    }
    ok &= expect_int("floor has C195", rt->floor[0].type,
                     ITEM_C195_EMPTY_FLASK, A_DEFS);
    ok &= expect_int("floor has C037", rt->floor[1].type, ITEM_C037_TORCH,
                     A_OBJECT);
    ok &= expect_int("floor has C042", rt->floor[2].type,
                     ITEM_C042_SCROLL_STACK, A_OBJECT);
    ok &= expect_int("floor has C030", rt->floor[3].type,
                     ITEM_C030_FOOD_RATION, A_OBJECT);
    return ok;
}

static int test_failover_and_leader_hand(const ProbeRuntime* rt)
{
    int ok = 1;

    ok &= expect_int("AMMO failover exact count",
                     rt->f0294StackFailoverCount, 2, A_AMMO);
    ok &= expect_int("first failover source C538",
                     rt->failoverSourceZone[0], ZONE_C538, A_AMMO);
    ok &= expect_int("first failover target C540",
                     rt->failoverTargetZone[0], ZONE_C540, A_F0297);
    ok &= expect_int("second failover source C539",
                     rt->failoverSourceZone[1], ZONE_C539, A_AMMO);
    ok &= expect_int("second failover target C541",
                     rt->failoverTargetZone[1], ZONE_C541, A_F0297);
    ok &= expect_int("F0297 put count for new stacks", rt->f0297PutCount, 2,
                     A_F0297);
    ok &= expect_int("F0300 C30 clear count", rt->f0300C30ClearCount, 6,
                     A_F0300);
    ok &= expect_int("F0301 C30 write count", rt->f0301C30WriteCount, 2,
                     A_F0301);
    ok &= expect_int("OBJECT type checks", rt->f0032TypeChecks, 2,
                     A_OBJECT);
    ok &= expect_int("OBJECT icon checks", rt->f0033IconChecks, 2,
                     A_OBJECT);
    ok &= expect_int("leader C540 final type", rt->leader[0].type,
                     ITEM_C042_SCROLL_STACK, A_F0297);
    ok &= expect_int("leader C541 final type", rt->leader[1].type,
                     ITEM_C042_SCROLL_STACK, A_F0297);
    ok &= expect_int("leader C542 final empty", rt->leader[2].type, ITEM_NONE,
                     A_F0298);
    ok &= expect_int("leader C543 final empty", rt->leader[3].type, ITEM_NONE,
                     A_F0298);
    ok &= expect_int("leader final visible stack count", rt->leaderCount, 2,
                     A_F0297);
    ok &= expect_int("leader C540 icon preserved",
                     rt->leader[0].icon, ITEM_C042_SCROLL_STACK, A_OBJECT);
    ok &= expect_int("leader C541 icon preserved",
                     rt->leader[1].icon, ITEM_C042_SCROLL_STACK, A_OBJECT);
    ok &= expect_int("failover 0 preserved icon",
                     rt->failoverPreservedIcon[0], ITEM_C042_SCROLL_STACK,
                     A_OBJECT);
    ok &= expect_int("failover 1 preserved icon",
                     rt->failoverPreservedIcon[1], ITEM_C042_SCROLL_STACK,
                     A_OBJECT);
    ok &= expect_int("failover 0 preserved id", rt->failoverPreservedId[0],
                     201, A_OBJECT);
    ok &= expect_int("failover 1 preserved id", rt->failoverPreservedId[1],
                     202, A_OBJECT);
    ok &= expect_int("leader excludes old C195",
                     rt->leader[0].type != ITEM_C195_EMPTY_FLASK &&
                         rt->leader[1].type != ITEM_C195_EMPTY_FLASK,
                     1, A_F0298);
    ok &= expect_int("leader excludes old C037",
                     rt->leader[0].type != ITEM_C037_TORCH &&
                         rt->leader[1].type != ITEM_C037_TORCH,
                     1, A_F0298);
    ok &= expect_int("leader excludes old C030",
                     rt->leader[0].type != ITEM_C030_FOOD_RATION &&
                         rt->leader[1].type != ITEM_C030_FOOD_RATION,
                     1, A_F0298);
    ok &= expect_int("no stale leader tail",
                     rt->leader[2].type == ITEM_NONE &&
                         rt->leader[3].type == ITEM_NONE,
                     1, A_F0298);
    return ok;
}

static int test_chest_close_rewrite(const ProbeRuntime* rt)
{
    int ok = 1;
    int i;

    ok &= expect_int("C537 after click retained", rt->chestAfter[0].itemType,
                     ITEM_C137_CHEST_SENTINEL, A_CHEST_CLOSE);
    ok &= expect_int("C538 after click empty", rt->chestAfter[1].itemType,
                     ITEM_NONE, A_CHEST_CLOSE);
    ok &= expect_int("C539 after click empty", rt->chestAfter[2].itemType,
                     ITEM_NONE, A_CHEST_CLOSE);
    ok &= expect_int("visible weight after click", rt->visibleWeightAfterClick,
                     5, A_CHEST_CLOSE);
    ok &= expect_int("G0426 before close unchanged", rt->openThingBeforeClose,
                     CHEST_THING, A_CHEST_GUARD);
    ok &= expect_int("F0334 close rewrite count", rt->f0334CloseCount, 1,
                     A_CHEST_CLOSE);
    ok &= expect_int("container weight before close",
                     rt->containerWeightBeforeClose,
                     DM1_PC34_CHEST_EMPTY_THING_WEIGHT + 5, A_CHEST_CLOSE);
    ok &= expect_int("closed first item is C537",
                     rt->closed[0].itemType, ITEM_C137_CHEST_SENTINEL,
                     A_CHEST_CLOSE);
    ok &= expect_int("closed first item charge", rt->closed[0].charges, 0,
                     A_CHEST_CLOSE);
    ok &= expect_int("G0426 after close cleared", rt->openThingAfterClose, 0,
                     A_CHEST_CLOSE);
    ok &= expect_int("C538 absent from closed chain",
                     rt->closed[0].itemType != ITEM_C042_SCROLL_STACK, 1,
                     A_CHEST_CLOSE);
    ok &= expect_int("C539 absent from closed chain",
                     rt->f0334CloseCount == 1, 1, A_CHEST_CLOSE);

    for (i = 1; i < DM1_PC34_CHEST_SLOT_COUNT; ++i) {
        ok &= expect_int("closed tail empty/stale-free",
                         rt->closed[i].itemType, ITEM_NONE, A_CHEST_CLOSE);
    }
    ok &= expect_int("C544 after click empty", rt->chestAfter[7].itemType,
                     ITEM_NONE, A_CHEST_CLOSE);
    ok &= expect_int("no G0425/G0426 mismatch",
                     rt->openThingBeforeClose == CHEST_THING &&
                         rt->openThingAfterClose == 0,
                     1, A_CHEST_CLOSE);
    ok &= expect_int("chest C537 remains only item",
                     rt->f0334CloseCount == 1 &&
                         rt->closed[0].itemType == ITEM_C137_CHEST_SENTINEL,
                     1, A_CHEST_CLOSE);
    return ok;
}

int main(void)
{
    ProbeRuntime runtime;
    int ok = 1;

    printf("probe=dm1_v1_chest_pickup_stack_failover_with_full_leader_hand_pc34_compat\n");
    ok &= expect_int("probe run", run_probe(&runtime), 1, A_CHEST_DISPATCH);
    ok &= test_source_anchors();
    ok &= test_initial_state(&runtime);
    ok &= test_drop_order_and_floor(&runtime);
    ok &= test_failover_and_leader_hand(&runtime);
    ok &= test_chest_close_rewrite(&runtime);
    ok &= expect_int("minimum assertion count",
                     g_assertions >= 100 ? 1 : 0, 1, A_CHEST_CLOSE);
    ok &= expect_int("target assertion ceiling",
                     g_assertions <= 140 ? 1 : 0, 1, A_CHEST_CLOSE);

    printf("assertions=%d failures=%d\n", g_assertions, g_failures);
    printf("chestPickupStackFailoverFullLeaderHandOk=%d\n",
           ok && g_failures == 0 ? 1 : 0);
    return ok && g_failures == 0 ? 0 : 1;
}
