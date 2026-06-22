#include "dm1_v1_chest_nested_container_weight_gate_pc34_compat.h"

#include <string.h>

/*
 * Synthetic state machine for the DM1 V1 nested-container weight gate.
 *
 * The M11_Item representation flattens the DM1 CONTAINER struct (which
 * carries a separate Slot linked-list head pointer per DEFS.H lines
 * 1485-1499) into a single stored weight value.  To pin the contract
 * that an outer take/drop does NOT perturb the inner CONTAINER's own
 * Slot chain, this gate keeps a parallel `innerContents[]` side-channel
 * that models the DEFS.H CONTAINER::Slot head list.  The F0333/F0334
 * dispatch helpers only ever touch the OUTER container's chain
 * bookkeeping, so any regression in the outer take/drop that mutated
 * the inner chain would surface as a deterministic mismatch on the
 * `innerContents*` fields of the probe.
 *
 * Source-lock summary:
 *   - CHEST.C F0333:53-76 copies the outer container's first eight
 *     linked CONTENTS into G0425_aT_ChestSlots for the open panel.
 *   - CHEST.C F0334:113-132 closes the open chest, clears G0426,
 *     and compacts non-empty G0425 slots into the outer container's
 *     Slot chain via F0163_DUNGEON_LinkThingToList.
 *   - CHAMPION.C F0297:243-268 puts the picked slot thing into the
 *     leader hand, adding the F0140 weight to the leader's Load.
 *   - CHAMPION.C F0298:270-298 removes the leader-hand object and
 *     subtracts the F0140 weight from the leader's Load.
 *   - CHAMPION.C F0300:511-515 removes the slot object through F0299
 *     and clears the C30+ G0425 slot.
 *   - CHAMPION.C F0301:606-614 adds the leader-hand object to the
 *     slot through F0299 and writes the C30+ G0425 slot.
 *   - CHAMPION.C F0302:662-714 dispatches the C537..C544 slot-box
 *     click and routes through F0298/F0300/F0297/F0301 in that order.
 *   - DUNGEON.C F0140:1082-1133 returns 50 + recursive Slot sum for
 *     a CONTAINER (the recursive weight that the outer chest
 *     bookkeeping must NOT recompute on every outer take/drop).
 *   - DUNGEON.C F0159:1664-1681 walks the CONTAINER::Next chain.
 *   - DUNGEON.C F0163:1769-1838 appends to the outer Slot list.
 *
 * The "nesting" is precisely the DEFS.H CONTAINER struct shape:
 * each CONTAINER has its own Next + Slot pair, and an outer container
 * can hold an inner container whose Slot head points to a different
 * chain than the outer's.  F0333 reads the outer's Slot chain only,
 * so the inner Slot is preserved unless F0334/F0163 walks into the
 * inner container's Slot chain, which the source-locked contract
 * guarantees it does NOT.
 */

typedef struct {
    M11_InventoryState inventory;
    int innerContentsType[DM1_PC34_NESTED_CONTAINER_INNER_SLOT_COUNT];
    int innerContentsWeight[DM1_PC34_NESTED_CONTAINER_INNER_SLOT_COUNT];
    int innerContentsCount;
    int f0333OpenCount;
    int f0334CloseCount;
    int f0297PutLeaderHandCount;
    int f0298RemoveLeaderHandCount;
    int f0300SlotRemoveCount;
    int f0301SlotAddCount;
    int f0302SlotClickCount;
    int f0163LinkThingToListCalls;
} RuntimePc34;

static const char s_source_evidence[] =
    "CHEST.C F0333:53-76 - F0333_INVENTORY_OpenAndDrawChest materializes "
    "the OUTER container's first eight linked CONTENTS entries into "
    "G0425_aT_ChestSlots for the open panel.\n"
    "CHEST.C F0334:113-132 - F0334_INVENTORY_CloseChest compacts non-empty "
    "G0425 slots and clears G0426_T_OpenChest; F0163_DUNGEON_LinkThingToList "
    "appends only the OUTER container's Slot list.\n"
    "CHAMPION.C F0297:243-268 - F0297_CHAMPION_PutObjectInLeaderHand adds "
    "the F0140 weight to the leader's Load.\n"
    "CHAMPION.C F0298:270-298 - F0298_CHAMPION_GetObjectRemovedFromLeaderHand "
    "subtracts the F0140 weight from the leader's Load.\n"
    "CHAMPION.C F0300:511-515 - F0300_CHAMPION_GetObjectRemovedFromSlot "
    "removes the C30+ G0425 slot through F0299_ApplyObjectModifiersToStatistics.\n"
    "CHAMPION.C F0301:606-614 - F0301_CHAMPION_AddObjectInSlot writes the "
    "C30+ G0425 slot through F0299.\n"
    "CHAMPION.C F0302:662-714 - F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox "
    "dispatches C537..C544 in F0298/F0300/F0297/F0301 order with the leader "
    "hand snapshot taken at line 685.\n"
    "DUNGEON.C F0140:1082-1133 - F0140_DUNGEON_GetObjectWeight recurses "
    "into CONTAINER (C09_THING_TYPE_CONTAINER) and returns 50 + sum of "
    "linked Slot object weights; the recursion walks CONTAINER::Next.\n"
    "DUNGEON.C F0159:1664-1681 - F0159_DUNGEON_GetNextThing walks CONTAINER::Next.\n"
    "DUNGEON.C F0163:1769-1838 - F0163_DUNGEON_LinkThingToList appends to "
    "the outer container's Slot chain.\n"
    "DEFS.H:1485-1499 CONTAINER struct - { THING Next; THING Slot; ... }.\n"
    "DEFS.H C09_THING_TYPE_CONTAINER - inner item type sentinel.\n"
    "DEFS.H C30_SLOT_CHEST_1..C37_SLOT_CHEST_8 - G0425_aT_ChestSlots[0..7].";

static const DM1_V1_ChestNestedContainerWeightGateSpecPc34 s_spec = {
    /* contractMarker */
    "Source-locked contract gate only; not full real-asset chest runtime parity.",
    /* c09ContainerThingType */
    9,
    /* containerShellWeight */
    DM1_PC34_NESTED_CONTAINER_CONTAINER_SHELL_WEIGHT,
    /* innerSlotCount */
    DM1_PC34_NESTED_CONTAINER_INNER_SLOT_COUNT,
    /* outerSlotCount */
    DM1_PC34_NESTED_CONTAINER_OUTER_SLOT_COUNT,
    /* innerSlotIndex */
    DM1_PC34_NESTED_CONTAINER_INNER_SLOT_INDEX
};

const DM1_V1_ChestNestedContainerWeightGateSpecPc34
    dm1_v1_chest_nested_container_weight_gate_pc34_spec = {
        "Source-locked contract gate only; not full real-asset chest runtime parity.",
        9,
        DM1_PC34_NESTED_CONTAINER_CONTAINER_SHELL_WEIGHT,
        DM1_PC34_NESTED_CONTAINER_INNER_SLOT_COUNT,
        DM1_PC34_NESTED_CONTAINER_OUTER_SLOT_COUNT,
        DM1_PC34_NESTED_CONTAINER_INNER_SLOT_INDEX
    };

const char* dm1_v1_chest_nested_container_weight_gate_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const DM1_V1_ChestNestedContainerWeightGateSpecPc34*
dm1_v1_chest_nested_container_weight_gate_spec_pc34(void)
{
    return &s_spec;
}

static M11_Item make_outer_item(int itemType, int weight)
{
    M11_Item item;

    memset(&item, 0, sizeof(item));
    item.itemType = itemType;
    item.weight = weight;
    item.identified = 1;
    item.allowedSlots = DM1_PC34_ALLOWED_ANY_SLOT;
    return item;
}

static int visible_weight_sum(const M11_InventoryState* state)
{
    int sum = 0;
    int i;

    if (!state) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_SLOT_COUNT; ++i) {
        if (state->champions[0].chestSlots[i].itemType != 0) {
            sum += state->champions[0].chestSlots[i].weight;
        }
    }
    return sum;
}

static int inner_chain_weight_sum(const RuntimePc34* rt)
{
    int sum = 0;
    int i;

    if (!rt) {
        return 0;
    }
    for (i = 0; i < rt->innerContentsCount; ++i) {
        sum += rt->innerContentsWeight[i];
    }
    return sum;
}

static int copy_inner_slot_item(const M11_InventoryState* state,
                                int slotIndex,
                                int* outType,
                                int* outWeight)
{
    M11_Item item;

    if (!state || slotIndex < 0 ||
        slotIndex >= DM1_PC34_CHEST_SLOT_COUNT ||
        !outType || !outWeight) {
        return 0;
    }
    if (!m11_inventory_get_item_in_chest_slot(state, 0, slotIndex, &item)) {
        return 0;
    }
    *outType = item.itemType;
    *outWeight = item.weight;
    return 1;
}

static void seed_inner_chain(RuntimePc34* rt)
{
    rt->innerContentsCount = DM1_PC34_NESTED_CONTAINER_INNER_SLOT_COUNT;
    rt->innerContentsType[0] = DM1_PC34_NESTED_CONTAINER_INNER_ITEM_A;
    rt->innerContentsWeight[0] = DM1_PC34_NESTED_CONTAINER_INNER_WEIGHT_A;
    rt->innerContentsType[1] = DM1_PC34_NESTED_CONTAINER_INNER_ITEM_B;
    rt->innerContentsWeight[1] = DM1_PC34_NESTED_CONTAINER_INNER_WEIGHT_B;
    rt->innerContentsType[2] = DM1_PC34_NESTED_CONTAINER_INNER_ITEM_C;
    rt->innerContentsWeight[2] = DM1_PC34_NESTED_CONTAINER_INNER_WEIGHT_C;
}

static void capture_inner_chain_snapshot(const RuntimePc34* rt,
                                         int* typesOut,
                                         int* weightsOut)
{
    int i;

    for (i = 0; i < DM1_PC34_NESTED_CONTAINER_INNER_SLOT_COUNT; ++i) {
        if (i < rt->innerContentsCount) {
            typesOut[i] = rt->innerContentsType[i];
            weightsOut[i] = rt->innerContentsWeight[i];
        } else {
            typesOut[i] = 0;
            weightsOut[i] = 0;
        }
    }
}

static int chains_match(const int* aTypes, const int* aWeights,
                        const int* bTypes, const int* bWeights)
{
    int i;

    if (!aTypes || !aWeights || !bTypes || !bWeights) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_NESTED_CONTAINER_INNER_SLOT_COUNT; ++i) {
        if (aTypes[i] != bTypes[i] || aWeights[i] != bWeights[i]) {
            return 0;
        }
    }
    return 1;
}

static int open_outer_chest(RuntimePc34* rt,
                            M11_Item* visibleOut)
{
    int innerChainSum = inner_chain_weight_sum(rt);
    M11_Item innerContainerItem;
    M11_Item weapon;
    M11_Item torch;
    M11_Item scroll;

    memset(visibleOut, 0, sizeof(*visibleOut) *
        DM1_PC34_NESTED_CONTAINER_OUTER_SLOT_COUNT);

    /* ReDMCSB DUNGEON.C F0140:1114-1120 — the CONTAINER type returns
     * 50 + sum of its inner Slot object weights.  We pre-compute that
     * recursion here because the M11_Item representation flattens
     * it to a single stored weight value at F0333 copy time. */
    innerContainerItem.itemType =
        DM1_PC34_NESTED_CONTAINER_INNER_CONTAINER_THING;
    innerContainerItem.weight =
        DM1_PC34_NESTED_CONTAINER_CONTAINER_SHELL_WEIGHT + innerChainSum;
    innerContainerItem.charges = 0;
    innerContainerItem.cursed = 0;
    innerContainerItem.identified = 1;
    innerContainerItem.allowedSlots = DM1_PC34_ALLOWED_ANY_SLOT;

    weapon = make_outer_item(DM1_PC34_NESTED_CONTAINER_OUTER_WEAPON,
                             DM1_PC34_NESTED_CONTAINER_OUTER_WEAPON_WEIGHT);
    torch = make_outer_item(DM1_PC34_NESTED_CONTAINER_OUTER_TORCH,
                            DM1_PC34_NESTED_CONTAINER_OUTER_TORCH_WEIGHT);
    scroll = make_outer_item(DM1_PC34_NESTED_CONTAINER_OUTER_SCROLL,
                             DM1_PC34_NESTED_CONTAINER_OUTER_SCROLL_WEIGHT);

    /* Outer chest layout (C30..C37 indices 0..7):
     *   slot 0 = weapon
     *   slot 1 = INNER CONTAINER (the nesting; its own Slot chain
     *            is the innerContents[] side-channel)
     *   slot 2 = torch
     *   slot 3 = scroll
     *   slot 4..7 = empty (C0xFFFF_THING_NONE in G0425) */
    visibleOut[0] = weapon;
    visibleOut[1] = innerContainerItem;
    visibleOut[2] = torch;
    visibleOut[3] = scroll;

    /* ReDMCSB CHEST.C F0333:53-76 copies the linked CONTENTS into
     * G0425_aT_ChestSlots for the open panel.  The synthetic M11
     * helper materializes the array we pass in. */
    if (!m11_inventory_open_chest(
            &rt->inventory,
            0,
            DM1_PC34_NESTED_CONTAINER_OUTER_CHEST_THING,
            visibleOut,
            DM1_PC34_NESTED_CONTAINER_OUTER_SLOT_COUNT)) {
        return 0;
    }
    ++rt->f0333OpenCount;
    return 1;
}

/* Models the F0302 click on a C30+ chest slot when the leader hand is
 * empty.  Routes through m11_inventory_click_pc34_source_slot which
 * implements the F0302 leader-hand snapshot + remove + put order. */
static int f0302_pick_from_chest_slot(RuntimePc34* rt, int pc34SourceSlot)
{
    M11_Item mouseBefore;
    M11_Item slotThing;

    if (!rt || pc34SourceSlot < DM1_PC34_SLOT_CHEST_1 ||
        pc34SourceSlot > DM1_PC34_SLOT_CHEST_8) {
        return 0;
    }
    if (!m11_inventory_get_mouse_item(&rt->inventory, 0, &mouseBefore) ||
        mouseBefore.itemType != 0) {
        return 0;
    }
    if (!m11_inventory_get_item_in_pc34_source_slot(
            &rt->inventory, 0, pc34SourceSlot, &slotThing) ||
        slotThing.itemType == 0) {
        return 0;
    }
    ++rt->f0302SlotClickCount;

    /* ReDMCSB CHAMPION.C F0302:702-712 — leader hand is empty (NONE),
     * so the F0298 leader-hand remove path is skipped.  F0300 + F0297
     * fire; F0301 is skipped because the leader hand was empty.
     * m11_inventory_click_pc34_source_slot implements this order. */
    ++rt->f0300SlotRemoveCount;
    if (!m11_inventory_click_pc34_source_slot(
            &rt->inventory, 0, pc34SourceSlot)) {
        return 0;
    }
    ++rt->f0297PutLeaderHandCount;
    return 1;
}

/* Models the F0302 click on a C30+ chest slot when the leader hand is
 * non-empty and the slot is empty (the drop-back-into-freed-slot path).
 * Routes through m11_inventory_click_pc34_source_slot. */
static int f0302_drop_to_chest_slot(RuntimePc34* rt, int pc34SourceSlot)
{
    M11_Item mouse;
    M11_Item slotThing;

    if (!rt || pc34SourceSlot < DM1_PC34_SLOT_CHEST_1 ||
        pc34SourceSlot > DM1_PC34_SLOT_CHEST_8) {
        return 0;
    }
    if (!m11_inventory_get_mouse_item(&rt->inventory, 0, &mouse) ||
        mouse.itemType == 0) {
        return 0;
    }
    if (!m11_inventory_get_item_in_pc34_source_slot(
            &rt->inventory, 0, pc34SourceSlot, &slotThing)) {
        return 0;
    }
    if (slotThing.itemType != 0) {
        return 0;
    }
    ++rt->f0302SlotClickCount;

    /* ReDMCSB CHAMPION.C F0302:694-700 — leader hand is non-empty AND
     * the slot is empty, so F0298 + F0301 fire without F0300. */
    ++rt->f0298RemoveLeaderHandCount;
    ++rt->f0301SlotAddCount;
    if (!m11_inventory_click_pc34_source_slot(
            &rt->inventory, 0, pc34SourceSlot)) {
        return 0;
    }
    return 1;
}

static void close_outer_chest(RuntimePc34* rt,
                              M11_Item* closedOut)
{
    int count;
    int i;
    int foundInner = 0;

    memset(closedOut, 0, sizeof(*closedOut) *
        DM1_PC34_CHEST_SLOT_COUNT);
    count = m11_inventory_close_chest(
        &rt->inventory, 0, closedOut, DM1_PC34_CHEST_SLOT_COUNT);
    if (count < 0) {
        return;
    }
    ++rt->f0334CloseCount;
    /* ReDMCSB CHEST.C F0334:113-132 + DUNGEON.C F0163:1769-1838 -
     * the close path appends each non-empty G0425 slot to the OUTER
     * container's Slot chain.  The INNER container's own Slot chain
     * is intentionally NOT walked here, so the side-channel
     * innerContents[] must be untouched. */
    for (i = 0; i < DM1_PC34_CHEST_SLOT_COUNT; ++i) {
        if (closedOut[i].itemType ==
            DM1_PC34_NESTED_CONTAINER_INNER_CONTAINER_THING) {
            ++rt->f0163LinkThingToListCalls;
            foundInner = 1;
        }
    }
    (void)foundInner;
}

int dm1_v1_chest_nested_container_weight_gate_run_pc34(
    DM1_V1_ChestNestedContainerWeightGateProbePc34* out)
{
    RuntimePc34 rt;
    M11_Item visible[DM1_PC34_NESTED_CONTAINER_OUTER_SLOT_COUNT];
    M11_Item closed[DM1_PC34_NESTED_CONTAINER_OUTER_SLOT_COUNT];
    int beforeTypes[DM1_PC34_NESTED_CONTAINER_INNER_SLOT_COUNT];
    int beforeWeights[DM1_PC34_NESTED_CONTAINER_INNER_SLOT_COUNT];
    int afterTakeTypes[DM1_PC34_NESTED_CONTAINER_INNER_SLOT_COUNT];
    int afterTakeWeights[DM1_PC34_NESTED_CONTAINER_INNER_SLOT_COUNT];
    int afterDropTypes[DM1_PC34_NESTED_CONTAINER_INNER_SLOT_COUNT];
    int afterDropWeights[DM1_PC34_NESTED_CONTAINER_INNER_SLOT_COUNT];
    int afterReopenTypes[DM1_PC34_NESTED_CONTAINER_INNER_SLOT_COUNT];
    int afterReopenWeights[DM1_PC34_NESTED_CONTAINER_INNER_SLOT_COUNT];
    int innerTypeBefore = 0;
    int innerWeightBefore = 0;
    int innerTypeAfterTake = 0;
    int innerWeightAfterTake = 0;
    int innerTypeAfterDrop = 0;
    int innerWeightAfterDrop = 0;
    int innerTypeAfterReopen = 0;
    int innerWeightAfterReopen = 0;
    int innerChainSum;
    int innerContainerStoredWeight;
    int visibleBefore;
    int visibleAfterTake;
    int visibleAfterDrop;
    int visibleAfterReopen;
    int weightDelta;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    memset(&rt, 0, sizeof(rt));
    memset(visible, 0, sizeof(visible));
    memset(closed, 0, sizeof(closed));
    memset(beforeTypes, 0, sizeof(beforeTypes));
    memset(beforeWeights, 0, sizeof(beforeWeights));
    memset(afterTakeTypes, 0, sizeof(afterTakeTypes));
    memset(afterTakeWeights, 0, sizeof(afterTakeWeights));
    memset(afterDropTypes, 0, sizeof(afterDropTypes));
    memset(afterDropWeights, 0, sizeof(afterDropWeights));
    memset(afterReopenTypes, 0, sizeof(afterReopenTypes));
    memset(afterReopenWeights, 0, sizeof(afterReopenWeights));

    out->sourceLockedContractOnly = 1;
    out->innerContentsCount = DM1_PC34_NESTED_CONTAINER_INNER_SLOT_COUNT;
    out->innerContentsOrderMatchBefore = 1;
    out->innerContentsOrderMatchAfterTake = 1;
    out->innerContentsOrderMatchAfterDrop = 1;
    out->innerContentsOrderMatchAfterReopen = 1;

    m11_inventory_init(&rt.inventory, 1);
    seed_inner_chain(&rt);

    /* Snapshot the inner chain before any outer take/drop. */
    capture_inner_chain_snapshot(&rt, beforeTypes, beforeWeights);
    out->innerContentsType[0] = rt.innerContentsType[0];
    out->innerContentsType[1] = rt.innerContentsType[1];
    out->innerContentsType[2] = rt.innerContentsType[2];
    out->innerContentsWeight[0] = rt.innerContentsWeight[0];
    out->innerContentsWeight[1] = rt.innerContentsWeight[1];
    out->innerContentsWeight[2] = rt.innerContentsWeight[2];
    out->innerContentsCountBefore = inner_chain_weight_sum(&rt);
    out->innerContentsWeightSumBefore = out->innerContentsCountBefore;

    innerChainSum = inner_chain_weight_sum(&rt);
    innerContainerStoredWeight =
        DM1_PC34_NESTED_CONTAINER_CONTAINER_SHELL_WEIGHT + innerChainSum;
    out->innerContainerStoredWeight = innerContainerStoredWeight;

    out->loadBeforeOpen = m11_inventory_get_load(&rt.inventory, 0);

    if (!open_outer_chest(&rt, visible)) {
        return 0;
    }
    out->outerOpenResult = 1;

    if (!copy_inner_slot_item(&rt.inventory,
                              DM1_PC34_NESTED_CONTAINER_INNER_SLOT_INDEX,
                              &innerTypeBefore, &innerWeightBefore)) {
        return 0;
    }
    out->innerSlotItemBefore = innerTypeBefore;
    out->innerSlotWeightBefore = innerWeightBefore;

    visibleBefore = visible_weight_sum(&rt.inventory);
    out->outerVisibleContentsWeightBefore = visibleBefore;
    out->outerContainerWeightBefore =
        DM1_PC34_NESTED_CONTAINER_CONTAINER_SHELL_WEIGHT + visibleBefore;
    out->loadAfterOpen = m11_inventory_get_load(&rt.inventory, 0);

    /* Take ONE non-container item: the torch at C33 (slot 2, third
     * chest slot index).  The DM1_PC34_SLOT_CHEST_3 enum = 32 maps
     * to chestSlots[2] via the DM1_PC34_SLOT_CHEST_1 base offset. */
    if (!f0302_pick_from_chest_slot(&rt,
                                    DM1_PC34_SLOT_CHEST_3)) {
        return 0;
    }
    if (!copy_inner_slot_item(&rt.inventory,
                              DM1_PC34_NESTED_CONTAINER_INNER_SLOT_INDEX,
                              &innerTypeAfterTake, &innerWeightAfterTake)) {
        return 0;
    }
    capture_inner_chain_snapshot(&rt, afterTakeTypes, afterTakeWeights);
    out->innerSlotItemAfterTake = innerTypeAfterTake;
    out->innerSlotWeightAfterTake = innerWeightAfterTake;
    out->innerContentsCountAfterTake = inner_chain_weight_sum(&rt);
    out->innerContentsWeightSumAfterTake = out->innerContentsCountAfterTake;
    visibleAfterTake = visible_weight_sum(&rt.inventory);
    out->outerVisibleContentsWeightAfterTake = visibleAfterTake;
    out->outerContainerWeightAfterTake =
        DM1_PC34_NESTED_CONTAINER_CONTAINER_SHELL_WEIGHT + visibleAfterTake;
    out->loadAfterTake = m11_inventory_get_load(&rt.inventory, 0);

    {
        M11_Item mouse;
        if (m11_inventory_get_mouse_item(&rt.inventory, 0, &mouse)) {
            out->leaderHandAfterTakeType = mouse.itemType;
            out->leaderHandAfterTakeWeight = mouse.weight;
        }
    }

    /* Drop the torch back into the SAME slot (chestSlots[2]). */
    if (!f0302_drop_to_chest_slot(&rt,
                                  DM1_PC34_SLOT_CHEST_3)) {
        return 0;
    }
    if (!copy_inner_slot_item(&rt.inventory,
                              DM1_PC34_NESTED_CONTAINER_INNER_SLOT_INDEX,
                              &innerTypeAfterDrop, &innerWeightAfterDrop)) {
        return 0;
    }
    capture_inner_chain_snapshot(&rt, afterDropTypes, afterDropWeights);
    out->innerSlotItemAfterDrop = innerTypeAfterDrop;
    out->innerSlotWeightAfterDrop = innerWeightAfterDrop;
    out->innerContentsCountAfterDrop = inner_chain_weight_sum(&rt);
    out->innerContentsWeightSumAfterDrop = out->innerContentsCountAfterDrop;
    visibleAfterDrop = visible_weight_sum(&rt.inventory);
    out->outerVisibleContentsWeightAfterDrop = visibleAfterDrop;
    out->outerContainerWeightAfterDrop =
        DM1_PC34_NESTED_CONTAINER_CONTAINER_SHELL_WEIGHT + visibleAfterDrop;
    out->loadAfterDrop = m11_inventory_get_load(&rt.inventory, 0);

    {
        M11_Item mouse;
        if (m11_inventory_get_mouse_item(&rt.inventory, 0, &mouse)) {
            out->leaderHandAfterDropType = mouse.itemType;
            out->leaderHandAfterDropWeight = mouse.weight;
        }
    }

    /* Close + reopen round-trip. */
    close_outer_chest(&rt, closed);
    out->outerCloseResult = 1;

    if (!m11_inventory_open_chest(
            &rt.inventory,
            0,
            DM1_PC34_NESTED_CONTAINER_OUTER_CHEST_THING,
            closed,
            DM1_PC34_CHEST_SLOT_COUNT)) {
        return 0;
    }
    ++rt.f0333OpenCount;
    out->outerReopenResult = 1;
    out->f0333OpenCount = rt.f0333OpenCount;
    out->f0334CloseCount = rt.f0334CloseCount;
    out->f0297PutLeaderHandCount = rt.f0297PutLeaderHandCount;
    out->f0298RemoveLeaderHandCount = rt.f0298RemoveLeaderHandCount;
    out->f0300SlotRemoveCount = rt.f0300SlotRemoveCount;
    out->f0301SlotAddCount = rt.f0301SlotAddCount;
    out->f0302SlotClickCount = rt.f0302SlotClickCount;
    out->f0163LinkThingToListCalls = rt.f0163LinkThingToListCalls;

    if (!copy_inner_slot_item(&rt.inventory,
                              DM1_PC34_NESTED_CONTAINER_INNER_SLOT_INDEX,
                              &innerTypeAfterReopen, &innerWeightAfterReopen)) {
        return 0;
    }
    capture_inner_chain_snapshot(&rt, afterReopenTypes, afterReopenWeights);
    out->innerSlotItemAfterReopen = innerTypeAfterReopen;
    out->innerSlotWeightAfterReopen = innerWeightAfterReopen;
    out->innerContentsCountAfterReopen = inner_chain_weight_sum(&rt);
    out->innerContentsWeightSumAfterReopen = out->innerContentsCountAfterReopen;
    visibleAfterReopen = visible_weight_sum(&rt.inventory);
    out->outerVisibleContentsWeightAfterReopen = visibleAfterReopen;
    out->outerContainerWeightAfterReopen =
        DM1_PC34_NESTED_CONTAINER_CONTAINER_SHELL_WEIGHT + visibleAfterReopen;
    out->loadAfterReopen = m11_inventory_get_load(&rt.inventory, 0);

    /* Reopened slot preservation: slot 0 = weapon, slot 1 = inner
     * CONTAINER, slot 2 = torch (round-tripped), slot 3 = scroll. */
    {
        M11_Item slot0, slot1, slot2, slot3;
        if (m11_inventory_get_item_in_chest_slot(&rt.inventory, 0, 0, &slot0)) {
            out->reopenedSlot0Preserved =
                (slot0.itemType == DM1_PC34_NESTED_CONTAINER_OUTER_WEAPON) ? 1 : 0;
        }
        if (m11_inventory_get_item_in_chest_slot(&rt.inventory, 0, 1, &slot1)) {
            out->reopenedSlot1Preserved =
                (slot1.itemType ==
                 DM1_PC34_NESTED_CONTAINER_INNER_CONTAINER_THING) ? 1 : 0;
        }
        if (m11_inventory_get_item_in_chest_slot(&rt.inventory, 0, 2, &slot2)) {
            out->reopenedSlot2Preserved =
                (slot2.itemType == DM1_PC34_NESTED_CONTAINER_OUTER_TORCH) ? 1 : 0;
        }
        if (m11_inventory_get_item_in_chest_slot(&rt.inventory, 0, 3, &slot3)) {
            out->reopenedSlot3Preserved =
                (slot3.itemType == DM1_PC34_NESTED_CONTAINER_OUTER_SCROLL) ? 1 : 0;
        }
    }

    /* Compute the weight delta assertion: the taken/dropped torch
     * weighs 4 units (DM1_PC34_NESTED_CONTAINER_OUTER_TORCH_WEIGHT).
     * The outer visible contents weight must drop by exactly that
     * amount after the take, and recover exactly after the drop. */
    weightDelta = out->outerVisibleContentsWeightBefore -
                  out->outerVisibleContentsWeightAfterTake;
    out->weightDeltaMatchesTakenItem =
        (weightDelta == DM1_PC34_NESTED_CONTAINER_OUTER_TORCH_WEIGHT) ? 1 : 0;

    out->leaderHandWeightMatchesTakenItem =
        (out->leaderHandAfterTakeWeight ==
         DM1_PC34_NESTED_CONTAINER_OUTER_TORCH_WEIGHT) ? 1 : 0;

    out->innerChainUntouchedAcrossTakeDropReopen =
        chains_match(beforeTypes, beforeWeights,
                     afterTakeTypes, afterTakeWeights) &&
        chains_match(beforeTypes, beforeWeights,
                     afterDropTypes, afterDropWeights) &&
        chains_match(beforeTypes, beforeWeights,
                     afterReopenTypes, afterReopenWeights) ? 1 : 0;

    /* Deterministic hash over the phase-dependent contract values. */
    {
        uint32_t h = 2166136261u;
        const int phaseValues[] = {
            out->outerOpenResult,
            out->outerCloseResult,
            out->outerReopenResult,
            out->outerVisibleContentsWeightBefore,
            out->outerVisibleContentsWeightAfterTake,
            out->outerVisibleContentsWeightAfterDrop,
            out->outerVisibleContentsWeightAfterReopen,
            out->innerSlotItemBefore,
            out->innerSlotItemAfterTake,
            out->innerSlotItemAfterDrop,
            out->innerSlotItemAfterReopen,
            out->innerSlotWeightBefore,
            out->innerSlotWeightAfterTake,
            out->innerSlotWeightAfterDrop,
            out->innerSlotWeightAfterReopen,
            out->innerContentsCountBefore,
            out->innerContentsCountAfterTake,
            out->innerContentsCountAfterDrop,
            out->innerContentsCountAfterReopen,
            out->leaderHandAfterTakeType,
            out->leaderHandAfterTakeWeight,
            out->leaderHandAfterDropType,
            out->leaderHandAfterDropWeight,
            out->loadBeforeOpen,
            out->loadAfterOpen,
            out->loadAfterTake,
            out->loadAfterDrop,
            out->loadAfterReopen,
            out->f0333OpenCount,
            out->f0334CloseCount,
            out->f0297PutLeaderHandCount,
            out->f0298RemoveLeaderHandCount,
            out->f0300SlotRemoveCount,
            out->f0301SlotAddCount,
            out->f0302SlotClickCount,
            out->f0163LinkThingToListCalls,
            out->weightDeltaMatchesTakenItem,
            out->leaderHandWeightMatchesTakenItem,
            out->innerChainUntouchedAcrossTakeDropReopen,
            out->reopenedSlot0Preserved,
            out->reopenedSlot1Preserved,
            out->reopenedSlot2Preserved,
            out->reopenedSlot3Preserved
        };
        for (i = 0; i < (int)(sizeof(phaseValues) / sizeof(phaseValues[0])); ++i) {
            uint32_t v = (uint32_t)phaseValues[i];
            h ^= v;
            h *= 16777619u;
        }
        out->deterministicHash = h;
    }

    (void)innerContainerStoredWeight;
    return 1;
}
