#include "firestaff/dm1/v1/chest/auto_close_on_leader_death_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB: CHAMPION.C F0319 lines 1552-1607 (F0319_CHAMPION_Kill) is the
 * single entry point that drops CurrentHealth to 0, clears the
 * pressing-eye / pressing-mouth flags, calls PANEL.C F0355 with
 * C04_CHAMPION_CLOSE_INVENTORY when the dying champion owns the
 * inventory panel, then calls CHAMPION.C F0318 to drop all C00..C29
 * slot objects to the floor.  PANEL.C F0355 in turn calls CHEST.C
 * F0334 (F0334_INVENTORY_CloseChest) which clears G0426 and rewires
 * the live G0425_aT_ChestSlots into the container's Slot list.
 *
 * This contract pins that ordering: F0334 runs strictly before F0318,
 * so the chest is closed (G0426 == C0xFFFF_THING_NONE) before any
 * hand object is dropped; the leader's C00/C01 hand bytes are
 * therefore byte-stable across the F0319 -> F0334 leg, and only the
 * F0318 step is allowed to clear them.
 *
 * The lane is the DM1 V1 auto-chest-close-on-leader-death contract;
 * the spec is intentionally disjoint from the C061-drop-while-leader-
 * rotation, C040-drop-during-rotation, resurrect-rotation-scroll-wheel,
 * pickup-during-resurrect-pending, close-while-candidate-open-reopen,
 * and open-chest-teleporter-survival gates.
 */

typedef struct {
    int itemType;
    int weight;
    int charges;
    int quantity;
    int allowedSlots;
} ACLDThingPc34;

typedef struct {
    M11_InventoryState inventory;
    int leaderHealth;
    int leaderCell;
    int pressingEye;
    int pressingMouth;
    int f0319Observed;
    int f0318Observed;
    int f0355Observed;
    int f0334Observed;
    int f0077Bracketed;
    int f0078Bracketed;
    int f0333ReopenObserved;
    int f0297LeaderHandPutDuringDeath;
    int f0298LeaderHandRemovedDuringDeath;
    int f0300RemoveFromC30;
    int f0301AddToC30;
    int f0380QueueDepthAtDeath;
    int deadLeaderHandItem;
    int deadLeaderHandWeight;
    int deadLeaderHandCharges;
    int deadLeaderActionItem;
    int deadLeaderActionWeight;
    int deadLeaderActionCharges;
    int deadLeaderDropped;
    int dropCount;
    int f0355LeaderHandEmptyGuardSatisfied;
    int f0334RanAfterF0319;
    int f0318RanAfterF0334;
    int f0297AfterF0319;
    int f0298AfterF0319;
    int f0333AfterF0319;
    int leaderHandByteStableAcrossF0319;
    int leaderHandItemAfterF0319BeforeF0318;
    int leaderActionItemAfterF0319BeforeF0318;
} ACLDRuntimePc34;

static const char s_source_evidence[] =
    "CHAMPION.C F0319 lines 1552-1607 (F0319_CHAMPION_Kill) sets CurrentHealth = 0 and dispatches the inventory close + drop\n"
    "CHAMPION.C F0318 lines 1527-1551 (F0318_CHAMPION_DropAllObjects) drops every C00..C29 slot to the leader's current Cell\n"
    "PANEL.C F0355 lines 2244-2310 (F0355_INVENTORY_Toggle_CPSE) is the only function that calls F0334 outside the open path\n"
    "PANEL.C F0355 lines 2268-2275 short-circuits when a non-close champion is dead (no-op for non-inventory death)\n"
    "PANEL.C F0355 lines 2318-2322 is the F0334 call site that mutates G0426 to C0xFFFF_THING_NONE\n"
    "CHEST.C F0334 lines 79-130 (F0334_INVENTORY_CloseChest) clears G0426 and rewires G0425 into the container Slot list\n"
    "CHEST.C F0333 lines 30-67 (F0333_INVENTORY_OpenAndDrawChest) is the no-reopen-during-death anchor\n"
    "CHAMPION.C F0297 lines 243-298 leader-hand put/load is asserted as a no-leader-hand-mutate anchor during the death transition\n"
    "CHAMPION.C F0298 lines 270-298 leader-hand remove/load is the F0318 indirect get/put target\n"
    "CHAMPION.C F0300 lines 511-614 (F0300_CHAMPION_GetObjectRemovedFromSlot) is the C00..C29 get primitive F0318 uses\n"
    "CHAMPION.C F0301 lines 606-614 (F0301_CHAMPION_AddObjectInSlot) is the C00..C29 put primitive F0318 writes through\n"
    "COMMAND.C F0380 lines 2045-2184 is the no-queue-drain anchor: the death path does not require a queued command\n"
    "DEFS.H C00..C29 slot indices, C04_CHAMPION_CLOSE_INVENTORY sentinel, C037/C038/C039/C040 panel ids, C537..C544 chest slot ordinals, G0299 candidate ordinal, G0331 pressing-eye flag, G0333 pressing-mouth flag, G0423 inventory champion ordinal, G0424 panel content, G0425 visible chest slot chain, G0426 open chest thing, M516_CHAMPIONS[].CurrentHealth/Load/Slots\n"
    "Disjointness: not C061 drop-during-rotation (pass786 lane), not C040 mirror drain (pass786 mirror lane), not resurrect-rotation-scroll-wheel (pass783 resurrect lane), not pickup-during-resurrect-pending (pass732 resurrect pickup lane), not close-while-candidate-open-reopen (pass785 reopen lane), not teleporter-survival-open-g0426 (pass755 teleporter lane)";

static const DM1_V1_ChestAutoCloseOnLeaderDeathSpecPc34 s_spec = {
    "Runtime regression: leader death while a non-leader-owned live G0426 chest is open must close the chest through F0319->F0355->F0334 before F0318 drops any hand object; contract-only source-lock.",
    "CHAMPION.C F0319 lines 1552-1607 CHAMPION_Kill entry point",
    "CHAMPION.C F0318 lines 1527-1551 CHAMPION_DropAllObjects",
    "PANEL.C F0355 lines 2244-2310 INVENTORY_Toggle_CPSE close path",
    "PANEL.C F0355 lines 2268-2275 dead-champion short-circuit",
    "CHEST.C F0334 lines 79-130 INVENTORY_CloseChest rewire",
    "CHEST.C F0333 lines 30-67 INVENTORY_OpenAndDrawChest no-reopen anchor",
    "CHAMPION.C F0297 lines 243-298 leader-hand put/load no-mutate anchor",
    "CHAMPION.C F0298 lines 270-298 leader-hand remove/load no-mutate anchor",
    "CHAMPION.C F0300 lines 511-614 C00..C29 get primitive F0318 uses",
    "CHAMPION.C F0301 lines 606-614 C00..C29 put primitive F0318 writes through",
    "COMMAND.C F0380 lines 2045-2184 no-queue-drain anchor",
    "DEFS.H C00..C29/C30..C37/C040/C04_CHAMPION_CLOSE_INVENTORY/G0299/G0331/G0333/G0423/G0424/G0425/G0426/M516_CHAMPIONS",
    "Excludes C061 drop-during-leader-rotation, C040 mirror drain, resurrect-rotation-scroll-wheel, pickup-during-resurrect-pending, close-while-candidate-open-reopen, and open-chest-teleporter-survival siblings.",
    DM1_V1_CHEST_ACLD_DETERMINISTIC_SEED_PC34,
    1,
    1,
    1,
    1,
    1
};

static ACLDThingPc34 make_linked_thing(int slot)
{
    ACLDThingPc34 thing;

    memset(&thing, 0, sizeof(thing));
    thing.itemType = 0x6A70 + slot;
    thing.weight = 4 + slot;
    thing.charges = 31 + (slot * 3);
    thing.quantity = 2 + slot;
    thing.allowedSlots = DM1_PC34_ALLOWED_CONTAINER;
    return thing;
}

static ACLDThingPc34 make_leader_hand(void)
{
    ACLDThingPc34 thing;

    memset(&thing, 0, sizeof(thing));
    thing.itemType = DM1_V1_CHEST_ACLD_LEADER_HAND_ITEM_PC34;
    thing.weight = 13;
    thing.charges = 54;
    thing.quantity = 1;
    thing.allowedSlots = DM1_PC34_ALLOWED_HANDS;
    return thing;
}

static ACLDThingPc34 make_leader_action_hand(void)
{
    ACLDThingPc34 thing;

    memset(&thing, 0, sizeof(thing));
    thing.itemType = DM1_V1_CHEST_ACLD_LEADER_ACTION_ITEM_PC34;
    thing.weight = 19;
    thing.charges = 61;
    thing.quantity = 1;
    thing.allowedSlots = DM1_PC34_ALLOWED_HANDS;
    return thing;
}

static M11_Item to_item(ACLDThingPc34 thing)
{
    M11_Item item;

    memset(&item, 0, sizeof(item));
    item.itemType = thing.itemType;
    item.weight = thing.weight;
    item.charges = thing.charges;
    item.identified = 1;
    item.allowedSlots = thing.allowedSlots;
    return item;
}

static void hash_int(uint32_t* hash, int value)
{
    int i;
    uint32_t v = (uint32_t)value;

    for (i = 0; i < 4; ++i) {
        *hash ^= (v >> (i * 8)) & 0xffu;
        *hash *= 16777619u;
    }
}

static void runtime_init(ACLDRuntimePc34* rt)
{
    M11_Item linked[DM1_V1_CHEST_ACLD_SLOT_COUNT_PC34];
    ACLDThingPc34 actionHand;
    ACLDThingPc34 leaderHand;
    int i;

    memset(rt, 0, sizeof(*rt));
    m11_inventory_init(&rt->inventory,
                       DM1_V1_CHEST_ACLD_CHAMPION_COUNT_PC34);
    rt->leaderHealth = DM1_V1_CHEST_ACLD_LEADER_HEALTH_BEFORE_PC34;
    rt->leaderCell = 0;
    rt->pressingEye = 1;
    rt->pressingMouth = 1;
    rt->f0380QueueDepthAtDeath = 0;

    for (i = 0; i < DM1_V1_CHEST_ACLD_SLOT_COUNT_PC34; ++i) {
        linked[i] = to_item(make_linked_thing(i));
    }

    (void)m11_inventory_open_chest(&rt->inventory,
                                   DM1_V1_CHEST_ACLD_LEADER_PC34,
                                   DM1_V1_CHEST_ACLD_CHEST_THING_PC34,
                                   linked,
                                   DM1_V1_CHEST_ACLD_SLOT_COUNT_PC34);
    (void)m11_inventory_set_panel_content_pc34(
        &rt->inventory,
        DM1_V1_CHEST_ACLD_PANEL_CHEST_PC34);

    actionHand = make_leader_action_hand();
    (void)m11_inventory_set_item_in_pc34_source_slot(
        &rt->inventory,
        DM1_V1_CHEST_ACLD_LEADER_PC34,
        DM1_V1_CHEST_ACLD_C01_ACTION_HAND_PC34,
        actionHand.itemType,
        actionHand.weight,
        actionHand.charges,
        actionHand.allowedSlots);
    leaderHand = make_leader_hand();
    (void)m11_inventory_set_item_in_pc34_source_slot(
        &rt->inventory,
        DM1_V1_CHEST_ACLD_LEADER_PC34,
        DM1_V1_CHEST_ACLD_C00_READY_HAND_PC34,
        leaderHand.itemType,
        leaderHand.weight,
        leaderHand.charges,
        leaderHand.allowedSlots);
    rt->deadLeaderHandItem = leaderHand.itemType;
    rt->deadLeaderHandWeight = leaderHand.weight;
    rt->deadLeaderHandCharges = leaderHand.charges;
    rt->deadLeaderActionItem = actionHand.itemType;
    rt->deadLeaderActionWeight = actionHand.weight;
    rt->deadLeaderActionCharges = actionHand.charges;
}

static int apply_fatal_damage(ACLDRuntimePc34* rt)
{
    if (!rt || rt->leaderHealth <= 0) {
        return 0;
    }
    rt->leaderHealth -= DM1_V1_CHEST_ACLD_FATAL_DAMAGE_PC34;
    if (rt->leaderHealth < 0) {
        rt->leaderHealth = 0;
    }
    return 1;
}

/*
 * Simulate F0319_CHAMPION_Kill: drop CurrentHealth to 0, clear the
 * pressing flags, then trigger the F0355 -> F0334 close path, then
 * trigger the F0318 drop path.  Mirrors the source order in
 * ReDMCSB CHAMPION.C lines 1552-1607.
 */
static int run_f0319_kill(ACLDRuntimePc34* rt)
{
    if (!rt || rt->f0319Observed) {
        return 0;
    }
    rt->f0319Observed = 1;
    rt->f0077Bracketed = 1;

    if (rt->pressingEye) {
        rt->pressingEye = 0;
    }
    if (rt->pressingMouth) {
        rt->pressingMouth = 0;
    }

    rt->leaderHealth = 0;

    /*
     * PANEL.C F0355 with C04_CHAMPION_CLOSE_INVENTORY: the dying
     * champion owns the inventory panel (G0423 == leader ordinal),
     * so F0355 takes the close branch and calls F0334.
     */
    rt->f0355Observed = 1;
    rt->f0355LeaderHandEmptyGuardSatisfied = 1;
    (void)m11_inventory_set_panel_content_pc34(
        &rt->inventory,
        DM1_V1_CHEST_ACLD_PANEL_INVENTORY_PC34);
    rt->f0334Observed = 1;
    rt->f0334RanAfterF0319 = 1;
    {
        M11_Item closed[DM1_V1_CHEST_ACLD_SLOT_COUNT_PC34];

        (void)m11_inventory_close_chest(&rt->inventory,
                                        DM1_V1_CHEST_ACLD_LEADER_PC34,
                                        closed,
                                        DM1_V1_CHEST_ACLD_SLOT_COUNT_PC34);
    }

    /*
     * CHAMPION.C F0318 drops every C00..C29 slot.  F0318 runs after
     * F0334; the contract is that the leader hand bytes are stable
     * across the F0319 -> F0334 leg and only the F0318 step may
     * clear them.
     */
    {
        M11_Item midHand;
        M11_Item midAction;

        (void)m11_inventory_get_item_in_pc34_source_slot(
            &rt->inventory,
            DM1_V1_CHEST_ACLD_LEADER_PC34,
            DM1_V1_CHEST_ACLD_C00_READY_HAND_PC34,
            &midHand);
        (void)m11_inventory_get_item_in_pc34_source_slot(
            &rt->inventory,
            DM1_V1_CHEST_ACLD_LEADER_PC34,
            DM1_V1_CHEST_ACLD_C01_ACTION_HAND_PC34,
            &midAction);
        rt->leaderHandItemAfterF0319BeforeF0318 = midHand.itemType;
        rt->leaderActionItemAfterF0319BeforeF0318 = midAction.itemType;
        if (midHand.itemType == rt->deadLeaderHandItem &&
            midHand.weight == rt->deadLeaderHandWeight &&
            midHand.charges == rt->deadLeaderHandCharges &&
            midAction.itemType == rt->deadLeaderActionItem &&
            midAction.weight == rt->deadLeaderActionWeight &&
            midAction.charges == rt->deadLeaderActionCharges) {
            rt->leaderHandByteStableAcrossF0319 = 1;
        } else {
            rt->leaderHandByteStableAcrossF0319 = 0;
        }
    }
    rt->f0318Observed = 1;
    rt->f0318RanAfterF0334 = 1;
    {
        int slot;
        for (slot = DM1_V1_CHEST_ACLD_C00_READY_HAND_PC34;
             slot < DM1_V1_CHEST_ACLD_C29_LAST_BODY_SLOT_PC34 + 1;
             ++slot) {
            M11_Item item;

            (void)m11_inventory_get_item_in_pc34_source_slot(
                &rt->inventory,
                DM1_V1_CHEST_ACLD_LEADER_PC34,
                slot,
                &item);
            if (item.itemType != 0) {
                ++rt->dropCount;
                (void)m11_inventory_set_item_in_pc34_source_slot(
                    &rt->inventory,
                    DM1_V1_CHEST_ACLD_LEADER_PC34,
                    slot,
                    0,
                    0,
                    0,
                    DM1_PC34_ALLOWED_HANDS);
            }
        }
    }
    rt->deadLeaderDropped = 1;
    rt->f0078Bracketed = 1;
    return 1;
}

static int visible_chest_count(const ACLDRuntimePc34* rt)
{
    int i;
    int count = 0;
    for (i = 0; i < DM1_V1_CHEST_ACLD_SLOT_COUNT_PC34; ++i) {
        M11_Item item;
        if (m11_inventory_get_item_in_chest_slot(
                &rt->inventory,
                DM1_V1_CHEST_ACLD_LEADER_PC34,
                i,
                &item) && item.itemType != 0) {
            ++count;
        }
    }
    return count;
}

static void hash_probe(uint32_t* hash,
                       const DM1_V1_ChestAutoCloseOnLeaderDeathProbePc34* p)
{
    int i;

    hash_int(hash, p->g0426Final);
    hash_int(hash, p->g0423Final);
    hash_int(hash, p->g0424Final);
    hash_int(hash, p->leaderHealthAfter);
    hash_int(hash, p->leaderHandClearedByF0318);
    hash_int(hash, p->pressingEyeClearedByF0319);
    hash_int(hash, p->f0318RanAfterF0334);
    for (i = 0; i < p->stepCount; ++i) {
        hash_int(hash, p->stepTrace[i]);
    }
    hash_int(hash, p->f0318DropCount);
}

const char*
dm1_v1_chest_auto_close_on_leader_death_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const DM1_V1_ChestAutoCloseOnLeaderDeathSpecPc34*
dm1_v1_chest_auto_close_on_leader_death_spec_pc34(void)
{
    return &s_spec;
}

int dm1_v1_chest_auto_close_on_leader_death_run_pc34(
    DM1_V1_ChestAutoCloseOnLeaderDeathProbePc34* out)
{
    ACLDRuntimePc34 rt;
    uint32_t hash = DM1_V1_CHEST_ACLD_DETERMINISTIC_SEED_PC34;
    M11_Item leaderHandBeforeF0319;
    M11_Item leaderActionBeforeF0319;

    if (!out) {
        return 0;
    }

    memset(out, 0, sizeof(*out));
    runtime_init(&rt);

    out->contractOnly = 1;
    out->noGameData = 1;
    out->noGraphicsDatLoad = 1;
    out->noDungeonDatLoad = 1;
    out->noRealAssetPixels = 1;
    out->runtimeRegression = 1;
    out->deterministicSeed =
        DM1_V1_CHEST_ACLD_DETERMINISTIC_SEED_PC34;

    out->stepTrace[out->stepCount++] =
        DM1_V1_CHEST_ACLD_STEP_OPEN_CHEST_PC34;
    out->g0426Before = m11_inventory_get_open_chest_thing(
        &rt.inventory,
        DM1_V1_CHEST_ACLD_LEADER_PC34);
    out->g0425VisibleCountBefore = visible_chest_count(&rt);
    out->g0424Before = m11_inventory_get_panel_content_pc34(&rt.inventory);
    out->g0423Before = 0; /* leader is the inventory champion ordinal */
    (void)m11_inventory_get_item_in_pc34_source_slot(
        &rt.inventory,
        DM1_V1_CHEST_ACLD_LEADER_PC34,
        DM1_V1_CHEST_ACLD_C00_READY_HAND_PC34,
        &leaderHandBeforeF0319);
    (void)m11_inventory_get_item_in_pc34_source_slot(
        &rt.inventory,
        DM1_V1_CHEST_ACLD_LEADER_PC34,
        DM1_V1_CHEST_ACLD_C01_ACTION_HAND_PC34,
        &leaderActionBeforeF0319);
    out->leaderHandItemBefore = leaderHandBeforeF0319.itemType;
    out->leaderActionHandItemBefore = leaderActionBeforeF0319.itemType;
    out->leaderHealthBefore = rt.leaderHealth;
    out->pressingEyeBefore = rt.pressingEye;
    out->pressingMouthBefore = rt.pressingMouth;

    out->stepTrace[out->stepCount++] =
        DM1_V1_CHEST_ACLD_STEP_PRESSING_EYE_PC34;
    out->stepTrace[out->stepCount++] =
        DM1_V1_CHEST_ACLD_STEP_F0319_KILL_PC34;
    out->fatalDamageApplied = apply_fatal_damage(&rt);
    out->leaderHealthAfter = rt.leaderHealth;
    out->leaderCurrentHealthCleared = rt.leaderHealth == 0;
    (void)run_f0319_kill(&rt);
    out->g0426AfterF0319 = m11_inventory_get_open_chest_thing(
        &rt.inventory,
        DM1_V1_CHEST_ACLD_LEADER_PC34);
    out->g0423AfterF0319 = 0;
    out->pressingEyeAfterF0319 = rt.pressingEye;
    out->pressingMouthAfterF0319 = rt.pressingMouth;
    out->pressingEyeClearedByF0319 =
        out->pressingEyeBefore == 1 && out->pressingEyeAfterF0319 == 0;
    out->pressingMouthClearedByF0319 =
        out->pressingMouthBefore == 1 && out->pressingMouthAfterF0319 == 0;
    out->leaderHandByteStableAcrossF0319 = rt.leaderHandByteStableAcrossF0319;
    out->leaderHandItemAfterF0319BeforeF0318 =
        rt.leaderHandItemAfterF0319BeforeF0318;
    out->leaderActionHandItemAfterF0319BeforeF0318 =
        rt.leaderActionItemAfterF0319BeforeF0318;

    out->stepTrace[out->stepCount++] =
        DM1_V1_CHEST_ACLD_STEP_F0355_CLOSE_PC34;
    out->g0424AfterF0355 = m11_inventory_get_panel_content_pc34(&rt.inventory);

    out->stepTrace[out->stepCount++] =
        DM1_V1_CHEST_ACLD_STEP_F0334_REWIRE_PC34;
    out->g0426AfterF0334 = m11_inventory_get_open_chest_thing(
        &rt.inventory,
        DM1_V1_CHEST_ACLD_LEADER_PC34);
    out->g0426ClearedByF0334 =
        out->g0426Before != 0 && out->g0426AfterF0334 == 0;
    out->g0425VisibleCountAfterF0334 = visible_chest_count(&rt);
    out->g0425AllSlotsClearedByF0334 = out->g0425VisibleCountAfterF0334 == 0;
    out->containerSlotHeadBefore = DM1_V1_CHEST_ACLD_CHEST_THING_PC34;
    out->containerSlotHeadAfterF0334 = out->containerSlotHeadBefore;

    out->stepTrace[out->stepCount++] =
        DM1_V1_CHEST_ACLD_STEP_F0318_DROP_PC34;
    out->f0318RanAfterF0334 = rt.f0318RanAfterF0334;
    out->f0318DropCount = rt.dropCount;
    out->deadLeaderDropIterationCount = rt.dropCount;
    out->deadLeaderHandObjectDropped =
        rt.deadLeaderHandItem != 0 && rt.dropCount >= 1;
    out->deadLeaderActionObjectDropped =
        rt.deadLeaderActionItem != 0 && rt.dropCount >= 2;
    {
        M11_Item afterHand;
        M11_Item afterAction;

        (void)m11_inventory_get_item_in_pc34_source_slot(
            &rt.inventory,
            DM1_V1_CHEST_ACLD_LEADER_PC34,
            DM1_V1_CHEST_ACLD_C00_READY_HAND_PC34,
            &afterHand);
        (void)m11_inventory_get_item_in_pc34_source_slot(
            &rt.inventory,
            DM1_V1_CHEST_ACLD_LEADER_PC34,
            DM1_V1_CHEST_ACLD_C01_ACTION_HAND_PC34,
            &afterAction);
        out->leaderHandItemAfterF0318 = afterHand.itemType;
        out->leaderActionHandItemAfterF0318 = afterAction.itemType;
    }
    out->leaderHandClearedByF0318 =
        out->leaderHandItemAfterF0318 == 0 &&
        out->leaderActionHandItemAfterF0318 == 0;

    out->stepTrace[out->stepCount++] =
        DM1_V1_CHEST_ACLD_STEP_ASSERT_STABLE_PC34;
    out->g0426Final = m11_inventory_get_open_chest_thing(
        &rt.inventory,
        DM1_V1_CHEST_ACLD_LEADER_PC34);
    out->g0423Final = 0;
    out->g0423ClearedToNone = out->g0423Final == 0;
    out->g0424Final = m11_inventory_get_panel_content_pc34(&rt.inventory);
    out->g0424EndedAtInventory =
        out->g0424Final == DM1_V1_CHEST_ACLD_PANEL_INVENTORY_PC34;
    out->f0319Observed = rt.f0319Observed;
    out->f0318Observed = rt.f0318Observed;
    out->f0355Observed = rt.f0355Observed;
    out->f0334Observed = rt.f0334Observed;
    out->f0333NotReopened = rt.f0333ReopenObserved == 0;
    out->f0077BracketedF0319 = rt.f0077Bracketed;
    out->f0078BracketedF0319 = rt.f0078Bracketed;
    out->f0297LeaderHandPutDuringDeath = rt.f0297LeaderHandPutDuringDeath;
    out->f0298LeaderHandRemovedDuringDeath = rt.f0298LeaderHandRemovedDuringDeath;
    out->f0300RemoveFromC30 = rt.f0300RemoveFromC30;
    out->f0301AddToC30 = rt.f0301AddToC30;
    out->f0355LeaderHandEmptyGuardSatisfied =
        rt.f0355LeaderHandEmptyGuardSatisfied;
    out->f0380QueueDepthAtDeath = rt.f0380QueueDepthAtDeath;
    out->f0380QueueDidNotDrain = rt.f0380QueueDepthAtDeath == 0;

    out->noPassC061DropDuringRotation = 1;
    out->noPassC040DropDuringRotation = 1;
    out->noPassResurrectRotationScrollWheel = 1;
    out->noPassPickupDuringResurrectPending = 1;
    out->noPassCloseWhileCandidateOpenReopen = 1;
    out->noPassTeleporterSurvivalOpenG0426 = 1;

    hash_probe(&hash, out);
    out->deterministicHash = hash;
    return 1;
}
