#include "dm1_v1_inventory_chest_drop_to_floor_pc34_compat.h"

#include <string.h>

static M11_Item make_item_pc34(int itemType, int weight)
{
    M11_Item item;

    memset(&item, 0, sizeof(item));
    item.itemType = itemType;
    item.weight = weight;
    item.identified = 1;
    item.allowedSlots = DM1_PC34_ALLOWED_CONTAINER;
    return item;
}

static int count_chest_items_pc34(
    const DM1_V1_InventoryChestDropToFloorStatePc34* state)
{
    M11_Item item;
    int count = 0;
    int i;

    if (!state) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_DROP_FLOOR_SLOT_COUNT; ++i) {
        if (m11_inventory_get_item_in_chest_slot(&state->inventory, 0, i,
                                                 &item) &&
            item.itemType != 0) {
            ++count;
        }
    }
    return count;
}

static int chest_load_pc34(
    const DM1_V1_InventoryChestDropToFloorStatePc34* state)
{
    M11_Item item;
    int load = 0;
    int i;

    if (!state) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_DROP_FLOOR_SLOT_COUNT; ++i) {
        if (m11_inventory_get_item_in_chest_slot(&state->inventory, 0, i,
                                                 &item)) {
            load += item.weight;
        }
    }
    return load;
}

static int close_types_after_drop_pc34(
    DM1_V1_InventoryChestDropToFloorStatePc34* state,
    DM1_V1_InventoryChestDropToFloorEventPc34* event)
{
    M11_Item closed[DM1_PC34_CHEST_DROP_FLOOR_SLOT_COUNT];
    int count;
    int i;

    if (!state || !event) {
        return 0;
    }
    memset(closed, 0, sizeof(closed));

    /* ReDMCSB: CHEST.C F0334 lines 112-132 compacts the visible
     * G0425_aT_ChestSlots into the container links while skipping NONE. */
    count = m11_inventory_close_chest(
        &state->inventory, 0, closed, DM1_PC34_CHEST_DROP_FLOOR_SLOT_COUNT);
    if (count < 0) {
        return 0;
    }
    event->closedCountAfterDrop = count;
    for (i = 0; i < DM1_PC34_CHEST_DROP_FLOOR_SLOT_COUNT; ++i) {
        event->closedTypes[i] = closed[i].itemType;
        event->closedWeights[i] = closed[i].weight;
    }
    return 1;
}

static int append_mouse_to_floor_pc34(
    DM1_V1_InventoryChestDropToFloorStatePc34* state,
    DM1_V1_InventoryChestDropToFloorEventPc34* event)
{
    M11_Item item;

    if (!state || !event ||
        state->floorCount >= DM1_PC34_CHEST_DROP_FLOOR_CELL_CAPACITY ||
        !m11_inventory_get_mouse_item(&state->inventory, 0, &item) ||
        item.itemType == 0) {
        return 0;
    }

    /* ReDMCSB: DUNGEON.C F0163 lines 1800-1837 links a thing to the square
     * list when MapX is non-negative.  This gate shadows that target square
     * as G0189_aT_Dungeon so a C30+ chest-slot pickup can be proven to land
     * on the floor rather than vanish during chest redraw/close processing. */
    event->floorCellIndex = state->floorCount;
    state->g0189_aT_Dungeon[state->floorCount++] = item;
    event->floorCountAfter = state->floorCount;
    event->floorCellType =
        state->g0189_aT_Dungeon[event->floorCellIndex].itemType;
    event->floorCellWeight =
        state->g0189_aT_Dungeon[event->floorCellIndex].weight;
    event->g0189FirstThing = state->g0189_aT_Dungeon[0].itemType;
    event->g0189Terminator = DM1_PC34_CHEST_DROP_FLOOR_THING_ENDOFLIST;
    event->droppedThingPreserved =
        event->floorCellType == event->slotTypeBefore &&
        event->floorCellWeight == event->slotWeightBefore;

    m11_inventory_set_mouse_item(&state->inventory, 0, 0, 0, 0,
                                 DM1_PC34_ALLOWED_ANY_SLOT);
    m11_inventory_recalc_load(&state->inventory, 0);
    return 1;
}

const char*
dm1_v1_inventory_chest_drop_to_floor_source_evidence_pc34(void)
{
    return
        "CHEST.C:2-90 F0333 opens and draws G0425_aT_ChestSlots from a container\n"
        "CHEST.C:112-132 F0334 closes by compacting non-empty chest slots\n"
        "CHAMPION.C:243-340 F0297 leader-hand pickup; BUG0_39 documents the chest re-shuffle hazard not reproduced here\n"
        "CHAMPION.C:587-700 F0301/F0302 route C30+ slots through G0425_aT_ChestSlots\n"
        "CHAMDRAW.C:551-560,1237-1260 reads and redraws C30+ chest slots from G0425\n"
        "DUNGEON.C:1082-1300 F0140-F0149 object weights and square/group lookups\n"
        "DUNGEON.C:1800-1837 F0163 links a thing to a map square when MapX >= 0\n"
        "DEFS.H:1712-1738 WEAPON_INFO/ARMOUR_INFO Weight accumulator cells";
}

void dm1_v1_inventory_chest_drop_to_floor_init_pc34(
    DM1_V1_InventoryChestDropToFloorStatePc34* state)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->mapX = DM1_PC34_CHEST_DROP_FLOOR_MAP_X;
    state->mapY = DM1_PC34_CHEST_DROP_FLOOR_MAP_Y;
    m11_inventory_init(&state->inventory, 1);
}

int dm1_v1_inventory_chest_drop_to_floor_open_pc34(
    DM1_V1_InventoryChestDropToFloorStatePc34* state,
    const M11_Item* linkedItems,
    int linkedItemCount)
{
    if (!state || linkedItemCount < 0 ||
        (linkedItemCount > 0 && !linkedItems)) {
        return 0;
    }

    /* ReDMCSB: CHEST.C F0333 lines 53-76 copies the first eight linked
     * objects into G0425_aT_ChestSlots and fills the rest with NONE. */
    return m11_inventory_open_chest(&state->inventory, 0,
                                    DM1_PC34_CHEST_DROP_FLOOR_CHEST_THING,
                                    linkedItems, linkedItemCount);
}

int dm1_v1_inventory_chest_drop_to_floor_run_case_pc34(
    DM1_V1_InventoryChestDropToFloorStatePc34* state,
    int chestSlotIndex,
    DM1_V1_InventoryChestDropToFloorEventPc34* outEvent)
{
    M11_Item before;
    M11_Item leftBefore;
    M11_Item leftAfter;
    M11_Item rightBefore;
    M11_Item rightAfter;
    M11_Item mouse;
    int pc34Slot;

    if (outEvent) {
        memset(outEvent, 0, sizeof(*outEvent));
    }
    if (!state || !outEvent || chestSlotIndex < 0 ||
        chestSlotIndex >= DM1_PC34_CHEST_DROP_FLOOR_SLOT_COUNT ||
        !m11_inventory_get_item_in_chest_slot(&state->inventory, 0,
                                              chestSlotIndex, &before) ||
        before.itemType == 0) {
        return 0;
    }

    pc34Slot = DM1_PC34_SLOT_CHEST_1 + chestSlotIndex;
    outEvent->championIndex = 0;
    outEvent->pc34Slot = pc34Slot;
    outEvent->chestSlotIndex = chestSlotIndex;
    outEvent->sourceIsChestSlot =
        m11_inventory_pc34_is_chest_source_slot(pc34Slot);
    outEvent->slotTypeBefore = before.itemType;
    outEvent->slotWeightBefore = before.weight;
    outEvent->loadBefore = m11_inventory_get_load(&state->inventory, 0);
    outEvent->chestLoadBefore = chest_load_pc34(state);
    outEvent->nonEmptyBefore = count_chest_items_pc34(state);
    outEvent->floorCountBefore = state->floorCount;

    if (chestSlotIndex > 0) {
        m11_inventory_get_item_in_chest_slot(&state->inventory, 0,
                                             chestSlotIndex - 1, &leftBefore);
        outEvent->adjacentLeftTypeBefore = leftBefore.itemType;
    }
    if (chestSlotIndex + 1 < DM1_PC34_CHEST_DROP_FLOOR_SLOT_COUNT) {
        m11_inventory_get_item_in_chest_slot(&state->inventory, 0,
                                             chestSlotIndex + 1, &rightBefore);
        outEvent->adjacentRightTypeBefore = rightBefore.itemType;
    }

    /* ReDMCSB: CHAMPION.C F0302 lines 688-710 reads a C30+ slot from
     * G0425_aT_ChestSlots, removes it with F0300, and puts it in the leader
     * hand through F0297.  This single-item floor drop intentionally avoids
     * the BUG0_39 Rabbit's Foot redraw/re-shuffle path at lines 337-340. */
    if (!m11_inventory_click_pc34_source_slot(&state->inventory, 0,
                                              pc34Slot) ||
        !m11_inventory_get_mouse_item(&state->inventory, 0, &mouse)) {
        return 0;
    }

    outEvent->loadAfterPickup = m11_inventory_get_load(&state->inventory, 0);
    outEvent->mouseTypeAfterPickup = mouse.itemType;
    outEvent->chestSlotCleared =
        m11_inventory_get_item_in_chest_slot(&state->inventory, 0,
                                             chestSlotIndex, &before) &&
        before.itemType == 0;

    if (!append_mouse_to_floor_pc34(state, outEvent) ||
        !m11_inventory_get_mouse_item(&state->inventory, 0, &mouse)) {
        return 0;
    }

    outEvent->loadAfterDrop = m11_inventory_get_load(&state->inventory, 0);
    outEvent->chestLoadAfter = chest_load_pc34(state);
    outEvent->expectedLoadAfterDrop =
        outEvent->loadBefore - outEvent->slotWeightBefore;
    outEvent->mouseTypeAfterDrop = mouse.itemType;
    outEvent->nonEmptyAfter = count_chest_items_pc34(state);

    if (chestSlotIndex > 0) {
        m11_inventory_get_item_in_chest_slot(&state->inventory, 0,
                                             chestSlotIndex - 1, &leftAfter);
        outEvent->adjacentLeftTypeAfter = leftAfter.itemType;
    }
    if (chestSlotIndex + 1 < DM1_PC34_CHEST_DROP_FLOOR_SLOT_COUNT) {
        m11_inventory_get_item_in_chest_slot(&state->inventory, 0,
                                             chestSlotIndex + 1, &rightAfter);
        outEvent->adjacentRightTypeAfter = rightAfter.itemType;
    }
    outEvent->noAdjacentOrphan =
        outEvent->adjacentLeftTypeBefore == outEvent->adjacentLeftTypeAfter &&
        outEvent->adjacentRightTypeBefore == outEvent->adjacentRightTypeAfter;
    outEvent->rabbitFootRefreshNotUsed = 1;
    outEvent->slotZeroOrphanBugReproduced = 0;
    outEvent->result =
        outEvent->sourceIsChestSlot &&
        outEvent->chestSlotCleared &&
        outEvent->floorCellType == outEvent->slotTypeBefore &&
        outEvent->loadAfterDrop == outEvent->expectedLoadAfterDrop &&
        outEvent->noAdjacentOrphan;

    if (!close_types_after_drop_pc34(state, outEvent)) {
        return 0;
    }
    return outEvent->result;
}

static int setup_and_drop_pc34(
    DM1_V1_InventoryChestDropToFloorEventPc34* event,
    int linkedItemCount,
    int dropSlotIndex,
    int firstType,
    int firstWeight)
{
    DM1_V1_InventoryChestDropToFloorStatePc34 state;
    M11_Item linked[DM1_PC34_CHEST_DROP_FLOOR_SLOT_COUNT];
    int i;

    dm1_v1_inventory_chest_drop_to_floor_init_pc34(&state);
    for (i = 0; i < linkedItemCount; ++i) {
        linked[i] = make_item_pc34(firstType + i, firstWeight + (i * 3));
    }
    if (!dm1_v1_inventory_chest_drop_to_floor_open_pc34(
            &state, linked, linkedItemCount)) {
        return 0;
    }
    return dm1_v1_inventory_chest_drop_to_floor_run_case_pc34(
        &state, dropSlotIndex, event);
}

int m11_inventory_pc34_probe_chest_drop_to_floor(
    DM1_V1_InventoryChestDropToFloorProbePc34* out)
{
    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    out->sourceLockedContractOnly = 1;
    out->caseCount = DM1_PC34_CHEST_DROP_FLOOR_CASE_COUNT;
    out->firstChestSlot = DM1_PC34_SLOT_CHEST_1;
    out->lastChestSlot = DM1_PC34_SLOT_CHEST_8;
    out->floorMapX = DM1_PC34_CHEST_DROP_FLOOR_MAP_X;
    out->floorMapY = DM1_PC34_CHEST_DROP_FLOOR_MAP_Y;
    out->floorCellCapacity = DM1_PC34_CHEST_DROP_FLOOR_CELL_CAPACITY;

    if (!setup_and_drop_pc34(&out->singleDrop, 3, 1, 3100, 4) ||
        !setup_and_drop_pc34(&out->fullChestDrop, 8, 4, 3200, 7) ||
        !setup_and_drop_pc34(&out->weightedDrop, 5, 2, 3300, 30) ||
        !setup_and_drop_pc34(&out->lastItemDrop, 1, 0, 3400, 11) ||
        !setup_and_drop_pc34(&out->indexedDrop, 6, 5, 3500, 13)) {
        return 0;
    }

    out->floorDropsTotal =
        out->singleDrop.floorCountAfter +
        out->fullChestDrop.floorCountAfter +
        out->weightedDrop.floorCountAfter +
        out->lastItemDrop.floorCountAfter +
        out->indexedDrop.floorCountAfter;
    out->allDroppedItemsOnG0189Floor =
        out->singleDrop.droppedThingPreserved &&
        out->fullChestDrop.droppedThingPreserved &&
        out->weightedDrop.droppedThingPreserved &&
        out->lastItemDrop.droppedThingPreserved &&
        out->indexedDrop.droppedThingPreserved;
    out->allPostDropLoadsMatch =
        out->singleDrop.loadAfterDrop ==
            out->singleDrop.expectedLoadAfterDrop &&
        out->fullChestDrop.loadAfterDrop ==
            out->fullChestDrop.expectedLoadAfterDrop &&
        out->weightedDrop.loadAfterDrop ==
            out->weightedDrop.expectedLoadAfterDrop &&
        out->lastItemDrop.loadAfterDrop ==
            out->lastItemDrop.expectedLoadAfterDrop &&
        out->indexedDrop.loadAfterDrop ==
            out->indexedDrop.expectedLoadAfterDrop;
    out->allAdjacentItemsPreserved =
        out->singleDrop.noAdjacentOrphan &&
        out->fullChestDrop.noAdjacentOrphan &&
        out->weightedDrop.noAdjacentOrphan &&
        out->lastItemDrop.noAdjacentOrphan &&
        out->indexedDrop.noAdjacentOrphan;
    return 1;
}
