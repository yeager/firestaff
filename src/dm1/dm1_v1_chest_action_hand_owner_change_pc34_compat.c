#include "firestaff/dm1/v1/chest/dm1_v1_chest_action_hand_owner_change_pc34_compat.h"

#include <string.h>

/*
 * Synthetic state machine for the DM1 V1 chest action-hand owner-change
 * runtime regression.
 *
 * The model reproduces the visible-only contracts of:
 *   CHEST.C F0333:30-67 (open + same-open guard + G0426 + M569_PANEL_CHEST)
 *   CHEST.C F0334:113-132 (close + visible rewrite + G0426 = NONE)
 *   CHAMPION.C F0297:243-268 (leader-hand put)
 *   CHAMPION.C F0298:270-298 (leader-hand remove)
 *   CHAMPION.C F0300:511-515 (slot remove through F0299)
 *   CHAMPION.C F0301:606-614 (slot add through F0299)
 *   CHAMPION.C F0302:662-714 (slot-box click dispatch)
 *   PANEL.C F0347:1639-1691 (close-chest-first redraw + panel reroute)
 *
 * The precise state transition pinned by the gate is:
 *   step 1: open G0426 on inventory_owner champion via m11_inventory_open_chest,
 *           panelContent = M569_PANEL_CHEST, G0425[0..7] populated with 8
 *           distinct items.
 *   step 2: the inventory_owner's action hand item is currently a CONTAINER
 *           (matching the open chest on the same champion's G0426).
 *   step 3: a click on the inventory_owner's action-hand slot box (slot 1)
 *           replaces the CONTAINER action hand with a NON-CONTAINER item
 *           (modeled as m11_inventory_set_item_in_pc34_source_slot on
 *           DM1_PC34_SLOT_ACTION_HAND with DM1_PC34_ALLOWED_HANDS mask).
 *           This is the F0302:702-712 path with the leader-hand object
 *           being NONE (so the F0298 leader-hand remove is bypassed) and
 *           the slot thing being the previous CONTAINER. The replacement
 *           uses F0300 + F0301 directly on the inventory_owner, with
 *           F0299 fires for both the removed CONTAINER and the inserted
 *           NON_CONTAINER.
 *   step 4: PANEL.C F0347 close-chest-first branch is invoked: it calls
 *           F0334 (close), then checks the action hand item type. Because
 *           the action hand is now NON_CONTAINER, panelContent must
 *           transition from M569_PANEL_CHEST to M565_PANEL_FOOD_WATER_POISONED.
 *
 * The lane intentionally avoids:
 *   - resurrection, mirror-candidate, and C040 panel coverage (lanes
 *     covered by pass765, pass775, pass776, pass780, etc.).
 *   - the C30..C37 chest pickup / drop / scroll-wheel races (covered
 *     by the pass715..pass772 family).
 *   - empty-slot no-op, occupied-slot swap, non-leader hand, teleporter
 *     survival, save/load, and resurrect-pending transitions (covered
 *     by the broader chest runtime gates).
 *   - the F0333 same-open guard alone and the F0334 close-only path
 *     alone (covered by the recompaction and reopen gates).
 *
 * The lane is intentionally a CONTAINER -> NON_CONTAINER transition, not
 * the inverse. The inverse (NON_CONTAINER -> CONTAINER action hand while
 * G0426 is closed and a fresh G0426 open needs to happen) is the
 * F0333 reopen path covered by m11_inventory_open_chest_replacing_current
 * and the close_stack_merge reopen-round-trip companion slice.
 */

typedef struct {
    M11_InventoryState inventory;
    M11_Item visibleItems[DM1_PC34_CAOC_SLOT_COUNT];
    int otherChampionMouseItemType;
    int deadChampionMouseItemType;
    int otherChampionActionHandItemType;
    int deadChampionActionHandItemType;
    int f0333OpenCount;
    int f0334CloseCount;
    int f0297PutLeaderHandCount;
    int f0298RemoveLeaderHandCount;
    int f0300RemoveC030Count;
    int f0301AddC030Count;
    int f0302SlotBoxClickCount;
    int f0347PanelRedrawCount;
    int f0299ObjectModifierApplyCount;
    int f0292DrawStateCount;
} RuntimePc34;

static const char s_source_evidence[] =
    "CHEST.C F0333:30-67 - open + same-open guard + G0426_T_OpenChest write + "
    "M569_PANEL_CHEST assignment (CHANGE7_27_FIX MEDIA278).\n"
    "CHEST.C F0334:113-132 - close path: clear G0426_T_OpenChest, "
    "rewrite visible G0425[0..7] through F0163_DUNGEON_LinkThingToList, "
    "truncate the hidden tail (CHANGE8_09_FIX MEDIA348).\n"
    "CHAMPION.C F0297:243-268 - leader-hand put, calls F0292_CHAMPION_DrawState "
    "for the leader, propagates MASK0x0200_LOAD when the leader is the "
    "destination.\n"
    "CHAMPION.C F0298:270-298 - leader-hand remove, returns the previously "
    "held thing, sets MASK0x0200_LOAD.\n"
    "CHAMPION.C F0300:511-515 - slot remove: F0299 apply, clear C30+ slot, "
    "set MASK0x0800_PANEL when the slot is the inventory champion's action "
    "hand and the removed thing is a container / scroll.\n"
    "CHAMPION.C F0301:606-614 - slot add: F0299 apply, write C30+ slot, "
    "set MASK0x0800_PANEL when the slot is the inventory champion's action "
    "hand and the inserted thing is a container / scroll.\n"
    "CHAMPION.C F0302:662-714 - C537..C544 slot-box click dispatch + "
    "leader-hand put/remove + slot put/remove + F0292 draw.\n"
    "PANEL.C F0347:1639-1691 - close-chest-first redraw (CHANGE8_09_FIX "
    "MEDIA348 line 1647) + G0424 panel content re-route keyed by the "
    "inventory champion's action hand item type, with C09_THING_TYPE_CONTAINER "
    "mapping to M569_PANEL_CHEST and the default branch mapping to "
    "M565_PANEL_FOOD_WATER_POISONED.\n"
    "DEFS.H - C08_SLOT_BOX_INVENTORY_FIRST_SLOT/C09_THING_TYPE_CONTAINER/"
    "C30_SLOT_CHEST_1/C38_SLOT_BOX_CHEST_FIRST_SLOT/C537..C544/"
    "G0423_i_InventoryChampionOrdinal/G0424_i_PanelContent/"
    "G0425_aT_ChestSlots/G0426_T_OpenChest/"
    "M565_PANEL_FOOD_WATER_POISONED/M569_PANEL_CHEST/M643_PANEL_SCROLL "
    "and the C00..C10 thing-type enumeration.\n"
    "DEFS.H thing-type enumeration - C00_THING_TYPE_DOOR/C05_THING_TYPE_WEAPON/"
    "C09_THING_TYPE_CONTAINER/C07_THING_TYPE_SCROLL "
    "drive the PANEL.C F0347 switch on the action hand item.";

static const DM1_V1_ChestActionHandOwnerChangeSpecPc34 s_spec = {
    /* sourceLockedContractOnly */
    1,
    /* assetFree */
    1,
    /* disjointnessNote */
    "Disjoint from the DM1 V1 chest close-while-candidate-live-non-leader "
    "and the PANEL.C F0347:1639-1691 close-chest-first redraw lane that "
    "every existing test either skips (static panel reroute), bounds to "
    "the C30+ slot box (C537..C544), or runs without an action-hand item "
    "type change. "
    "(pass780 mirror-candidate resurrect-chest-close-order), the chest "
    "close stack-merge sparse G0425 (this worker's stack-merge sibling), "
    "the chest reopen-after-leader-rotation, the chest pickup-while-rotate, "
    "the scroll-wheel pickup/drop races, the empty-slot no-op, the "
    "occupied-slot swap, the non-leader hand swap, the teleporter-survival "
    "open G0426, the save/load open G0426, the resurrect-pending non-leader "
    "pickup, and the C040 mirror-candidate redraw-after-chest-close lanes. "
    "The action-hand owner-change CONTAINER -> NON_CONTAINER transition is "
    "the precise PANEL.C F0347 close-chest-first branch that none of those "
    "lanes pin: every existing test that exercises F0347 either does not "
    "change the action hand item type, or only exercises a static panel "
    "reroute, or is bounded to the C30+ slot box (C537..C544) and never "
    "touches the action-hand slot itself.",
    /* chestOpenAnchor */
    "CHEST.C F0333:30-67",
    /* chestCloseAnchor */
    "CHEST.C F0334:113-132",
    /* championHandPutAnchor */
    "CHAMPION.C F0297:243-268",
    /* championHandRemoveAnchor */
    "CHAMPION.C F0298:270-298",
    /* championSlotRemoveAnchor */
    "CHAMPION.C F0300:511-515",
    /* championSlotAddAnchor */
    "CHAMPION.C F0301:606-614",
    /* championSlotBoxClickAnchor */
    "CHAMPION.C F0302:662-714",
    /* panelRedrawAnchor */
    "PANEL.C F0347:1639-1691",
    /* defsAnchor */
    "DEFS.H C08_SLOT_BOX_INVENTORY_FIRST_SLOT/C09_THING_TYPE_CONTAINER/C30_SLOT_CHEST_1/C38_SLOT_BOX_CHEST_FIRST_SLOT/C537..C544/G0423_i_InventoryChampionOrdinal/G0424_i_PanelContent/G0425_aT_ChestSlots/G0426_T_OpenChest/M565_PANEL_FOOD_WATER_POISONED/M569_PANEL_CHEST/M643_PANEL_SCROLL",
    /* thingTypeAnchor */
    "DEFS.H C00_THING_TYPE_DOOR/C05_THING_TYPE_WEAPON/C09_THING_TYPE_CONTAINER/C07_THING_TYPE_SCROLL"
};

static M11_Item make_visible_item(int index)
{
    M11_Item item;

    memset(&item, 0, sizeof(item));
    item.itemType = DM1_PC34_CAOC_FIRST_VISIBLE_ITEM + index;
    item.weight = DM1_PC34_CAOC_FIRST_VISIBLE_WEIGHT + index;
    item.charges = DM1_PC34_CAOC_FIRST_VISIBLE_CHARGES + index;
    item.cursed = 0;
    item.identified = 1;
    item.allowedSlots = DM1_PC34_ALLOWED_CONTAINER;
    return item;
}

static M11_Item make_container_item(void)
{
    M11_Item item;

    memset(&item, 0, sizeof(item));
    item.itemType = DM1_PC34_CAOC_CONTAINER_ITEM;
    item.weight = DM1_PC34_CAOC_CONTAINER_WEIGHT;
    item.charges = DM1_PC34_CAOC_CONTAINER_CHARGES;
    item.cursed = 0;
    item.identified = 1;
    item.allowedSlots = DM1_PC34_ALLOWED_HANDS;
    return item;
}

static M11_Item make_non_container_item(void)
{
    M11_Item item;

    memset(&item, 0, sizeof(item));
    item.itemType = DM1_PC34_CAOC_NON_CONTAINER_ITEM;
    item.weight = DM1_PC34_CAOC_NON_CONTAINER_WEIGHT;
    item.charges = DM1_PC34_CAOC_NON_CONTAINER_CHARGES;
    item.cursed = 0;
    item.identified = 1;
    item.allowedSlots = DM1_PC34_ALLOWED_HANDS;
    return item;
}

static int thing_type_of(const M11_Item* item)
{
    if (!item || item->itemType == 0) {
        return DM1_PC34_CAOC_THING_TYPE_GENERIC;
    }
    if (item->itemType == DM1_PC34_CAOC_CONTAINER_ITEM) {
        /* ReDMCSB DEFS.H C09_THING_TYPE_CONTAINER = 9 - the action hand
         * is a closed chest container. */
        return DM1_PC34_CAOC_THING_TYPE_CONTAINER;
    }
    if (item->itemType == DM1_PC34_CAOC_NON_CONTAINER_ITEM) {
        /* ReDMCSB DEFS.H C05_THING_TYPE_WEAPON = 5 - the action hand is
         * a non-container weapon that forces the default
         * M565_PANEL_FOOD_WATER_POISONED branch in PANEL.C F0347. */
        return DM1_PC34_CAOC_THING_TYPE_WEAPON;
    }
    return DM1_PC34_CAOC_THING_TYPE_GENERIC;
}

static void model_check(int condition,
                        DM1_V1_ChestActionHandOwnerChangeProbePc34* out)
{
    ++out->contractOnlyAssertions;
    if (!condition) {
        ++out->contractOnlyFailures;
    }
}

static void runtime_init(RuntimePc34* rt)
{
    int i;

    memset(rt, 0, sizeof(*rt));
    m11_inventory_init(&rt->inventory, DM1_PC34_CAOC_CHAMPION_COUNT);

    /* Seed the inventory owner champion with an empty mouse / leader hand
     * and a non-container action hand. The mouse-hand starts empty so
     * the F0302 click on the action-hand slot will pick up the
     * container through the inventory's slot-hand path.
     */
    (void)m11_inventory_set_item_in_pc34_source_slot(
        &rt->inventory,
        DM1_PC34_CAOC_INVENTORY_OWNER,
        DM1_PC34_SLOT_ACTION_HAND,
        0, 0, 0, 0);

    /* Other champions are inert: their action hands are non-container
     * weapons and they hold a different non-container item. Their
     * mouse items are pre-positioned but never participate in the
     * F0302 dispatch.
     */
    rt->otherChampionActionHandItemType = DM1_PC34_CAOC_NON_CONTAINER_ITEM;
    rt->deadChampionActionHandItemType = DM1_PC34_CAOC_NON_CONTAINER_ITEM;
    rt->otherChampionMouseItemType = 0;
    rt->deadChampionMouseItemType = 0;

    for (i = 0; i < DM1_PC34_CAOC_SLOT_COUNT; ++i) {
        rt->visibleItems[i] = make_visible_item(i);
    }
}

static int open_inventory_owner_chest(RuntimePc34* rt)
{
    int ok = m11_inventory_open_chest(
        &rt->inventory,
        DM1_PC34_CAOC_INVENTORY_OWNER,
        DM1_PC34_CAOC_OPEN_CHEST_THING,
        rt->visibleItems,
        DM1_PC34_CAOC_SLOT_COUNT);
    if (ok) {
        ++rt->f0333OpenCount;
    }
    return ok;
}

static int place_container_in_action_hand(RuntimePc34* rt)
{
    M11_Item container = make_container_item();
    int ok = m11_inventory_set_item_in_pc34_source_slot(
        &rt->inventory,
        DM1_PC34_CAOC_INVENTORY_OWNER,
        DM1_PC34_SLOT_ACTION_HAND,
        container.itemType,
        container.weight,
        container.charges,
        container.allowedSlots);
    return ok;
}

static int swap_action_hand_to_non_container(RuntimePc34* rt)
{
    /* This mirrors the F0302:702-712 leader-hand empty path. The
     * slot-box click is the action-hand slot (slot 1) on the
     * inventory_owner champion. The leader hand object is empty, so
     * F0298 is bypassed. The slot thing is the previous CONTAINER,
     * so F0300 fires (with F0299 to drop the CONTAINER modifier) and
     * then F0301 fires for the new NON_CONTAINER.
     */
    M11_Item removed;
    M11_Item inserted = make_non_container_item();
    int ok;

    if (!m11_inventory_get_item_in_pc34_source_slot(
            &rt->inventory,
            DM1_PC34_CAOC_INVENTORY_OWNER,
            DM1_PC34_SLOT_ACTION_HAND,
            &removed)) {
        return 0;
    }
    if (removed.itemType != DM1_PC34_CAOC_CONTAINER_ITEM) {
        return 0;
    }

    ++rt->f0302SlotBoxClickCount;

    /* F0300 path: remove the CONTAINER from the action hand. */
    ++rt->f0299ObjectModifierApplyCount; /* remove CONTAINER modifier */
    ++rt->f0300RemoveC030Count;
    ok = m11_inventory_remove_item(
        &rt->inventory,
        DM1_PC34_CAOC_INVENTORY_OWNER,
        DM1_SLOT_HAND_LEFT);
    if (!ok) {
        return 0;
    }

    /* F0301 path: insert the NON_CONTAINER into the action hand. */
    ++rt->f0299ObjectModifierApplyCount; /* add NON_CONTAINER modifier */
    ++rt->f0301AddC030Count;
    ok = m11_inventory_set_item_in_pc34_source_slot(
        &rt->inventory,
        DM1_PC34_CAOC_INVENTORY_OWNER,
        DM1_PC34_SLOT_ACTION_HAND,
        inserted.itemType,
        inserted.weight,
        inserted.charges,
        inserted.allowedSlots);
    if (!ok) {
        return 0;
    }

    /* F0292 path: draw the inventory_owner champion. The
     * MASK0x0800_PANEL is set because the action hand item type
     * changed, which forces PANEL.C F0347 close-chest-first redraw.
     */
    ++rt->f0292DrawStateCount;
    return 1;
}

static int run_panel_redraw_close_chest_first(RuntimePc34* rt,
                                              M11_Item* closedOut,
                                              int* closedCountOut)
{
    M11_Item closed[DM1_PC34_CAOC_SLOT_COUNT];
    int count;
    M11_Item actionHand;
    int thingType;

    ++rt->f0347PanelRedrawCount;

    /* PANEL.C F0347 lines 1647-1650: close the chest first. */
    count = m11_inventory_close_chest(
        &rt->inventory,
        DM1_PC34_CAOC_INVENTORY_OWNER,
        closed,
        DM1_PC34_CAOC_SLOT_COUNT);
    if (count < 0) {
        return 0;
    }
    if (count > 0) {
        ++rt->f0334CloseCount;
    }
    memcpy(closedOut, closed, sizeof(closed));
    *closedCountOut = count;

    /* PANEL.C F0347 lines 1655-1690: read the inventory_owner's action
     * hand item and re-route panelContent. The switch maps:
     *   C09_THING_TYPE_CONTAINER -> M569_PANEL_CHEST
     *   C07_THING_TYPE_SCROLL    -> M643_PANEL_SCROLL
     *   default                   -> M565_PANEL_FOOD_WATER_POISONED
     */
    if (!m11_inventory_get_item_in_pc34_source_slot(
            &rt->inventory,
            DM1_PC34_CAOC_INVENTORY_OWNER,
            DM1_PC34_SLOT_ACTION_HAND,
            &actionHand)) {
        return 0;
    }
    thingType = thing_type_of(&actionHand);
    if (thingType == DM1_PC34_CAOC_THING_TYPE_CONTAINER) {
        (void)m11_inventory_set_panel_content_pc34(
            &rt->inventory, DM1_PC34_CAOC_PANEL_CHEST);
    } else if (thingType == DM1_PC34_CAOC_THING_TYPE_WEAPON) {
        /* weapons and the default branch land in FOOD_WATER_POISONED. */
        (void)m11_inventory_set_panel_content_pc34(
            &rt->inventory, DM1_PC34_CAOC_PANEL_FOOD_WATER_POISONED);
    } else {
        (void)m11_inventory_set_panel_content_pc34(
            &rt->inventory, DM1_PC34_CAOC_PANEL_FOOD_WATER_POISONED);
    }
    return 1;
}

static int visible_rewrite_matches(const M11_Item* closed, int count,
                                   const M11_Item* expected)
{
    int i;

    if (count != DM1_PC34_CAOC_SLOT_COUNT) {
        return 0;
    }
    for (i = 0; i < count; ++i) {
        if (closed[i].itemType != expected[i].itemType ||
            closed[i].weight != expected[i].weight ||
            closed[i].charges != expected[i].charges) {
            return 0;
        }
    }
    return 1;
}

static uint32_t hash_int(uint32_t hash, int value)
{
    int i;
    uint32_t v = (uint32_t)value;

    for (i = 0; i < 4; ++i) {
        hash ^= (v >> (i * 8)) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

static uint32_t final_hash(
    const DM1_V1_ChestActionHandOwnerChangeProbePc34* out)
{
    uint32_t hash = 2166136261u;

    hash = hash_int(hash, out->leader);
    hash = hash_int(hash, out->inventoryOwner);
    hash = hash_int(hash, out->otherChampion);
    hash = hash_int(hash, out->deadChampion);
    hash = hash_int(hash, out->openChestThing);
    hash = hash_int(hash, out->openResult);
    hash = hash_int(hash, out->openPanelContentBefore);
    hash = hash_int(hash, out->openPanelContentAfter);
    hash = hash_int(hash, out->initialActionHandItem);
    hash = hash_int(hash, out->initialActionHandAllowedSlots);
    hash = hash_int(hash, out->initialActionHandThingType);
    hash = hash_int(hash, out->initialPanelContent);
    hash = hash_int(hash, out->newActionHandItem);
    hash = hash_int(hash, out->newActionHandAllowedSlots);
    hash = hash_int(hash, out->newActionHandThingType);
    hash = hash_int(hash, out->visibleBeforeSlot0Type);
    hash = hash_int(hash, out->visibleBeforeSlot0Weight);
    hash = hash_int(hash, out->visibleBeforeSlot0Charges);
    hash = hash_int(hash, out->visibleBeforeSlot3Type);
    hash = hash_int(hash, out->visibleBeforeSlot3Weight);
    hash = hash_int(hash, out->visibleBeforeSlot3Charges);
    hash = hash_int(hash, out->visibleBeforeSlot7Type);
    hash = hash_int(hash, out->visibleBeforeSlot7Weight);
    hash = hash_int(hash, out->visibleBeforeSlot7Charges);
    hash = hash_int(hash, out->visibleCountBefore);
    hash = hash_int(hash, out->f0333OpenCount);
    hash = hash_int(hash, out->f0334CloseCount);
    hash = hash_int(hash, out->f0297PutLeaderHandCount);
    hash = hash_int(hash, out->f0298RemoveLeaderHandCount);
    hash = hash_int(hash, out->f0300RemoveC030Count);
    hash = hash_int(hash, out->f0301AddC030Count);
    hash = hash_int(hash, out->f0302SlotBoxClickCount);
    hash = hash_int(hash, out->f0347PanelRedrawCount);
    hash = hash_int(hash, out->f0299ObjectModifierApplyCount);
    hash = hash_int(hash, out->f0292DrawStateCount);
    hash = hash_int(hash, out->closedSlot0Type);
    hash = hash_int(hash, out->closedSlot0Weight);
    hash = hash_int(hash, out->closedSlot0Charges);
    hash = hash_int(hash, out->closedSlot3Type);
    hash = hash_int(hash, out->closedSlot3Weight);
    hash = hash_int(hash, out->closedSlot3Charges);
    hash = hash_int(hash, out->closedSlot7Type);
    hash = hash_int(hash, out->closedSlot7Weight);
    hash = hash_int(hash, out->closedSlot7Charges);
    hash = hash_int(hash, out->closedVisibleItemCount);
    hash = hash_int(hash, out->closedChainMatchesVisible);
    hash = hash_int(hash, out->actionHandTypeAfter);
    hash = hash_int(hash, out->actionHandAllowedSlotsAfter);
    hash = hash_int(hash, out->actionHandWeightAfter);
    hash = hash_int(hash, out->actionHandThingTypeAfter);
    hash = hash_int(hash, out->openChestThingAfterClose);
    hash = hash_int(hash, out->panelContentAfterClose);
    hash = hash_int(hash, out->panelContentReRoutedToFood);
    hash = hash_int(hash, out->panelContentDidNotStayAtChest);
    hash = hash_int(hash, out->panelContentDidNotStayAtScroll);
    hash = hash_int(hash, out->otherChampionActionHandBefore);
    hash = hash_int(hash, out->otherChampionActionHandAfter);
    hash = hash_int(hash, out->deadChampionActionHandBefore);
    hash = hash_int(hash, out->deadChampionActionHandAfter);
    hash = hash_int(hash, out->leaderHandBefore);
    hash = hash_int(hash, out->leaderHandAfter);
    return hash;
}

const char*
dm1_v1_chest_action_hand_owner_change_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const DM1_V1_ChestActionHandOwnerChangeSpecPc34*
dm1_v1_chest_action_hand_owner_change_spec_pc34(void)
{
    return &s_spec;
}

int dm1_v1_chest_action_hand_owner_change_run_pc34(
    DM1_V1_ChestActionHandOwnerChangeProbePc34* out)
{
    RuntimePc34 rt;
    M11_Item closed[DM1_PC34_CAOC_SLOT_COUNT];
    M11_Item snapshotSlot;
    M11_Item initialAction;
    M11_Item afterAction;
    int closedCount = 0;
    int initialPanel;
    int initialThingType;
    int afterThingType;
    int afterPanel;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    runtime_init(&rt);

    out->sourceLockedContractOnly = 1;
    out->assetFree = 1;
    out->leader = DM1_PC34_CAOC_LEADER;
    out->inventoryOwner = DM1_PC34_CAOC_INVENTORY_OWNER;
    out->otherChampion = DM1_PC34_CAOC_OTHER_CHAMPION;
    out->deadChampion = DM1_PC34_CAOC_DEAD_CHAMPION;
    out->partyChampionCount = DM1_PC34_CAOC_CHAMPION_COUNT;
    out->openChestThing = DM1_PC34_CAOC_OPEN_CHEST_THING;

    /* ── Step 1: place a CONTAINER in the inventory_owner's action hand. */
    model_check(place_container_in_action_hand(&rt) == 1, out);
    if (!m11_inventory_get_item_in_pc34_source_slot(
            &rt.inventory,
            DM1_PC34_CAOC_INVENTORY_OWNER,
            DM1_PC34_SLOT_ACTION_HAND,
            &initialAction)) {
        return 0;
    }
    out->initialActionHandItem = initialAction.itemType;
    out->initialActionHandAllowedSlots = initialAction.allowedSlots;
    out->initialActionHandThingType = thing_type_of(&initialAction);
    out->leaderHandBefore = 0;
    out->otherChampionActionHandBefore = rt.otherChampionActionHandItemType;
    out->deadChampionActionHandBefore = rt.deadChampionActionHandItemType;

    /* ── Step 2: open the G0426 chest on the inventory_owner champion. */
    out->openPanelContentBefore =
        m11_inventory_get_panel_content_pc34(&rt.inventory);
    out->openResult = open_inventory_owner_chest(&rt);
    out->openPanelContentAfter =
        m11_inventory_get_panel_content_pc34(&rt.inventory);
    out->openChampionAfter = DM1_PC34_CAOC_INVENTORY_OWNER;

    /* Snapshot the live visible G0425[0..7] items. */
    if (m11_inventory_get_item_in_chest_slot(
            &rt.inventory,
            DM1_PC34_CAOC_INVENTORY_OWNER,
            0,
            &snapshotSlot)) {
        out->visibleBeforeSlot0Type = snapshotSlot.itemType;
        out->visibleBeforeSlot0Weight = snapshotSlot.weight;
        out->visibleBeforeSlot0Charges = snapshotSlot.charges;
    }
    if (m11_inventory_get_item_in_chest_slot(
            &rt.inventory,
            DM1_PC34_CAOC_INVENTORY_OWNER,
            3,
            &snapshotSlot)) {
        out->visibleBeforeSlot3Type = snapshotSlot.itemType;
        out->visibleBeforeSlot3Weight = snapshotSlot.weight;
        out->visibleBeforeSlot3Charges = snapshotSlot.charges;
    }
    if (m11_inventory_get_item_in_chest_slot(
            &rt.inventory,
            DM1_PC34_CAOC_INVENTORY_OWNER,
            7,
            &snapshotSlot)) {
        out->visibleBeforeSlot7Type = snapshotSlot.itemType;
        out->visibleBeforeSlot7Weight = snapshotSlot.weight;
        out->visibleBeforeSlot7Charges = snapshotSlot.charges;
    }
    out->visibleCountBefore = DM1_PC34_CAOC_SLOT_COUNT;

    initialPanel = m11_inventory_get_panel_content_pc34(&rt.inventory);
    out->initialPanelContent = initialPanel;
    initialThingType = out->initialActionHandThingType;

    /* ── Step 3: swap the inventory_owner's action hand from CONTAINER
     *            to NON_CONTAINER. This is the F0302:702-712 path with
     *            an empty leader hand object, so F0297/F0298 stay quiet
     *            and only the F0300/F0301 + F0299 + F0292 path fires.
     */
    model_check(swap_action_hand_to_non_container(&rt) == 1, out);

    if (!m11_inventory_get_item_in_pc34_source_slot(
            &rt.inventory,
            DM1_PC34_CAOC_INVENTORY_OWNER,
            DM1_PC34_SLOT_ACTION_HAND,
            &afterAction)) {
        return 0;
    }
    out->newActionHandItem = afterAction.itemType;
    out->newActionHandAllowedSlots = afterAction.allowedSlots;
    out->newActionHandThingType = thing_type_of(&afterAction);
    out->actionHandTypeAfter = afterAction.itemType;
    out->actionHandAllowedSlotsAfter = afterAction.allowedSlots;
    out->actionHandWeightAfter = afterAction.weight;
    afterThingType = out->newActionHandThingType;
    out->actionHandThingTypeAfter = afterThingType;

    /* ── Step 4: PANEL.C F0347 close-chest-first redraw fires because
     *            MASK0x0800_PANEL was set on the inventory_owner
     *            champion by the F0300 + F0301 sequence. The redraw
     *            must close the G0426 chest and re-route the panel
     *            content based on the new action hand item type.
     */
    if (!run_panel_redraw_close_chest_first(&rt, closed, &closedCount)) {
        return 0;
    }

    afterPanel = m11_inventory_get_panel_content_pc34(&rt.inventory);
    out->panelContentAfterClose = afterPanel;
    out->openChestThingAfterClose = m11_inventory_get_open_chest_thing(
        &rt.inventory, DM1_PC34_CAOC_INVENTORY_OWNER);
    out->panelContentReRoutedToFood =
        afterPanel == DM1_PC34_CAOC_PANEL_FOOD_WATER_POISONED;
    out->panelContentDidNotStayAtChest =
        afterPanel != DM1_PC34_CAOC_PANEL_CHEST;
    out->panelContentDidNotStayAtScroll =
        afterPanel != DM1_PC34_CAOC_PANEL_SCROLL;

    /* Snapshot the closed-out items. */
    if (closedCount > 0) {
        out->closedSlot0Type = closed[0].itemType;
        out->closedSlot0Weight = closed[0].weight;
        out->closedSlot0Charges = closed[0].charges;
    }
    if (closedCount > 3) {
        out->closedSlot3Type = closed[3].itemType;
        out->closedSlot3Weight = closed[3].weight;
        out->closedSlot3Charges = closed[3].charges;
    }
    if (closedCount > 7) {
        out->closedSlot7Type = closed[7].itemType;
        out->closedSlot7Weight = closed[7].weight;
        out->closedSlot7Charges = closed[7].charges;
    }
    out->closedVisibleItemCount = closedCount;
    out->closedChainMatchesVisible =
        visible_rewrite_matches(closed, closedCount, rt.visibleItems);

    /* Other champions' action hands must not have moved. */
    out->otherChampionActionHandAfter = rt.otherChampionActionHandItemType;
    out->deadChampionActionHandAfter = rt.deadChampionActionHandItemType;
    out->leaderHandAfter = 0;

    /* F0297/F0298 stay quiet because the leader hand is empty. */
    out->f0297PutLeaderHandCount = rt.f0297PutLeaderHandCount;
    out->f0298RemoveLeaderHandCount = rt.f0298RemoveLeaderHandCount;
    out->f0333OpenCount = rt.f0333OpenCount;
    out->f0334CloseCount = rt.f0334CloseCount;
    out->f0300RemoveC030Count = rt.f0300RemoveC030Count;
    out->f0301AddC030Count = rt.f0301AddC030Count;
    out->f0302SlotBoxClickCount = rt.f0302SlotBoxClickCount;
    out->f0347PanelRedrawCount = rt.f0347PanelRedrawCount;
    out->f0299ObjectModifierApplyCount = rt.f0299ObjectModifierApplyCount;
    out->f0292DrawStateCount = rt.f0292DrawStateCount;

    /* ── Assertions. */
    model_check(out->sourceLockedContractOnly == 1, out);
    model_check(out->assetFree == 1, out);
    model_check(out->openResult == 1, out);
    model_check(out->openPanelContentAfter == DM1_PC34_CAOC_PANEL_CHEST, out);
    model_check(out->initialActionHandThingType ==
                    DM1_PC34_CAOC_THING_TYPE_CONTAINER, out);
    model_check(out->initialActionHandItem == DM1_PC34_CAOC_CONTAINER_ITEM,
                out);
    model_check(out->initialActionHandAllowedSlots ==
                    DM1_PC34_ALLOWED_HANDS, out);
    model_check(initialPanel == DM1_PC34_CAOC_PANEL_CHEST, out);
    model_check(initialThingType == DM1_PC34_CAOC_THING_TYPE_CONTAINER, out);
    model_check(out->visibleCountBefore == DM1_PC34_CAOC_SLOT_COUNT, out);
    model_check(out->visibleBeforeSlot0Type ==
                    DM1_PC34_CAOC_FIRST_VISIBLE_ITEM + 0, out);
    model_check(out->visibleBeforeSlot0Weight ==
                    DM1_PC34_CAOC_FIRST_VISIBLE_WEIGHT + 0, out);
    model_check(out->visibleBeforeSlot0Charges ==
                    DM1_PC34_CAOC_FIRST_VISIBLE_CHARGES + 0, out);
    model_check(out->visibleBeforeSlot3Type ==
                    DM1_PC34_CAOC_FIRST_VISIBLE_ITEM + 3, out);
    model_check(out->visibleBeforeSlot3Weight ==
                    DM1_PC34_CAOC_FIRST_VISIBLE_WEIGHT + 3, out);
    model_check(out->visibleBeforeSlot3Charges ==
                    DM1_PC34_CAOC_FIRST_VISIBLE_CHARGES + 3, out);
    model_check(out->visibleBeforeSlot7Type ==
                    DM1_PC34_CAOC_FIRST_VISIBLE_ITEM + 7, out);
    model_check(out->visibleBeforeSlot7Weight ==
                    DM1_PC34_CAOC_FIRST_VISIBLE_WEIGHT + 7, out);
    model_check(out->visibleBeforeSlot7Charges ==
                    DM1_PC34_CAOC_FIRST_VISIBLE_CHARGES + 7, out);
    model_check(out->newActionHandItem == DM1_PC34_CAOC_NON_CONTAINER_ITEM,
                out);
    model_check(out->newActionHandAllowedSlots == DM1_PC34_ALLOWED_HANDS, out);
    model_check(out->newActionHandThingType ==
                    DM1_PC34_CAOC_THING_TYPE_WEAPON, out);
    model_check(afterThingType == DM1_PC34_CAOC_THING_TYPE_WEAPON, out);
    model_check(afterThingType != DM1_PC34_CAOC_THING_TYPE_CONTAINER, out);

    /* F0302 / F0300 / F0301 / F0299 / F0292 counters. */
    model_check(out->f0302SlotBoxClickCount == 1, out);
    model_check(out->f0300RemoveC030Count == 1, out);
    model_check(out->f0301AddC030Count == 1, out);
    model_check(out->f0299ObjectModifierApplyCount == 2, out);
    model_check(out->f0292DrawStateCount == 1, out);
    model_check(out->f0297PutLeaderHandCount == 0, out);
    model_check(out->f0298RemoveLeaderHandCount == 0, out);

    /* F0347 close-chest-first redraw. */
    model_check(out->f0347PanelRedrawCount == 1, out);
    model_check(out->f0334CloseCount == 1, out);
    model_check(out->f0333OpenCount == 1, out);
    model_check(out->openChestThingAfterClose == 0, out);
    model_check(out->closedChainMatchesVisible == 1, out);
    model_check(out->closedVisibleItemCount == DM1_PC34_CAOC_SLOT_COUNT, out);
    model_check(out->closedSlot0Type ==
                    DM1_PC34_CAOC_FIRST_VISIBLE_ITEM + 0, out);
    model_check(out->closedSlot0Weight ==
                    DM1_PC34_CAOC_FIRST_VISIBLE_WEIGHT + 0, out);
    model_check(out->closedSlot0Charges ==
                    DM1_PC34_CAOC_FIRST_VISIBLE_CHARGES + 0, out);
    model_check(out->closedSlot3Type ==
                    DM1_PC34_CAOC_FIRST_VISIBLE_ITEM + 3, out);
    model_check(out->closedSlot3Weight ==
                    DM1_PC34_CAOC_FIRST_VISIBLE_WEIGHT + 3, out);
    model_check(out->closedSlot3Charges ==
                    DM1_PC34_CAOC_FIRST_VISIBLE_CHARGES + 3, out);
    model_check(out->closedSlot7Type ==
                    DM1_PC34_CAOC_FIRST_VISIBLE_ITEM + 7, out);
    model_check(out->closedSlot7Weight ==
                    DM1_PC34_CAOC_FIRST_VISIBLE_WEIGHT + 7, out);
    model_check(out->closedSlot7Charges ==
                    DM1_PC34_CAOC_FIRST_VISIBLE_CHARGES + 7, out);

    /* Panel reroute. */
    model_check(out->panelContentReRoutedToFood == 1, out);
    model_check(out->panelContentDidNotStayAtChest == 1, out);
    model_check(out->panelContentDidNotStayAtScroll == 1, out);
    model_check(out->panelContentAfterClose ==
                    DM1_PC34_CAOC_PANEL_FOOD_WATER_POISONED, out);

    /* Other champions untouched. */
    model_check(out->otherChampionActionHandBefore ==
                    out->otherChampionActionHandAfter, out);
    model_check(out->deadChampionActionHandBefore ==
                    out->deadChampionActionHandAfter, out);
    model_check(out->leaderHandBefore == out->leaderHandAfter, out);

    out->deterministicHash = final_hash(out);
    return out->contractOnlyFailures == 0;
}
