#include "firestaff/dm1/v1/chest/dm1_v1_chest_partial_drop_to_floor_while_chest_open_pc34_compat.h"

#include <string.h>

typedef struct {
    M11_InventoryState inventory;
    M11_Item floor[2];
    int floorCount;
} PartialDropRuntimePc34;

static M11_Item make_stack(int itemType, int count, int unitWeight)
{
    M11_Item item;

    memset(&item, 0, sizeof(item));
    item.itemType = itemType;
    item.charges = count;
    item.weight = count * unitWeight;
    item.allowedSlots = DM1_PC34_ALLOWED_CONTAINER;
    item.identified = 1;
    return item;
}

static DM1_V1_ChestPartialDropStackPc34 snapshot_stack(M11_Item item)
{
    DM1_V1_ChestPartialDropStackPc34 stack;

    stack.type = item.itemType;
    stack.count = item.charges;
    stack.weight = item.weight;
    return stack;
}

static void copy_chest_slots(
    const M11_InventoryState* state,
    DM1_V1_ChestPartialDropStackPc34* out)
{
    int i;

    for (i = 0; i < DM1_PC34_PARTIAL_DROP_SLOT_COUNT; ++i) {
        M11_Item item;

        memset(&item, 0, sizeof(item));
        (void)m11_inventory_get_item_in_chest_slot(state, 0, i, &item);
        out[i] = snapshot_stack(item);
    }
}

static int count_visible(const DM1_V1_ChestPartialDropStackPc34* slots)
{
    int count = 0;
    int i;

    for (i = 0; i < DM1_PC34_PARTIAL_DROP_SLOT_COUNT; ++i) {
        if (slots[i].type != 0) {
            ++count;
        }
    }
    return count;
}

static uint32_t hash_u32(uint32_t hash, uint32_t value)
{
    hash ^= value;
    hash *= 16777619u;
    return hash;
}

static uint32_t hash_stack(uint32_t hash,
                           DM1_V1_ChestPartialDropStackPc34 stack)
{
    hash = hash_u32(hash, (uint32_t)stack.type);
    hash = hash_u32(hash, (uint32_t)stack.count);
    hash = hash_u32(hash, (uint32_t)stack.weight);
    return hash;
}

static uint32_t deterministic_hash(
    const DM1_V1_ChestPartialDropToFloorWhileOpenProbePc34* out)
{
    uint32_t hash = 2166136261u;
    int i;

    hash = hash_u32(hash, (uint32_t)out->openResult);
    hash = hash_u32(hash, (uint32_t)out->sourcePc34Slot);
    hash = hash_u32(hash, (uint32_t)out->sourceZone);
    hash = hash_u32(hash, (uint32_t)out->partialDropCount);
    hash = hash_u32(hash, (uint32_t)out->remainingStackCount);
    hash = hash_stack(hash, out->leaderHandAfterSplit);
    hash = hash_stack(hash, out->leaderHandAfterFloor);
    for (i = 0; i < DM1_PC34_PARTIAL_DROP_SLOT_COUNT; ++i) {
        hash = hash_stack(hash, out->chestAfter[i]);
        hash = hash_stack(hash, out->closedChain[i]);
    }
    hash = hash_u32(hash, (uint32_t)out->floorDropCount);
    hash = hash_u32(hash, (uint32_t)out->floorType);
    hash = hash_u32(hash, (uint32_t)out->floorWeight);
    return hash;
}

const char*
dm1_v1_chest_partial_drop_to_floor_while_chest_open_source_evidence_pc34(
    void)
{
    return
        "CHEST.C F0333:30-67 opens G0426 and materializes linked chest contents into visible G0425/C537..C544 slots\n"
        "CHEST.C F0334:113-132 closes G0426 by clearing and compacting non-empty G0425 entries back into the container chain\n"
        "CHAMPION.C F0297:243-268 puts the selected partial stack in the leader hand and updates M516 load state\n"
        "CHAMPION.C F0298:270-298 removes the leader hand before the floor link, leaving it empty after the deterministic floor drop\n"
        "CHAMPION.C F0300:511-515 removes C30+ entries from G0425, and F0301:606-614 writes C30+ entries back through G0425\n"
        "CHAMPION.C F0302:662-710 dispatches C30+ chest-slot clicks through leader-hand/slot swap semantics\n"
        "COMMAND.C F0359:1973-1983 M569 chest panel routes C040-style chest-slot commands into F0302; COMMAND.C F0359:1985-1990 documents the adjacent M568/C040 resurrect dispatch boundary\n"
        "OBJECT.C F0032:121-145 resolves object type and OBJECT.C F0033:147-212 resolves the visible icon for chest/hand/floor redraws\n"
        "BLITMASK.C F0133:30-33 is the partial-mask bitmap draw boundary for the changed stack icon\n"
        "DUNGEON.C F0163:1796-1837 links the dropped partial stack to the floor square when MapX >= 0\n"
        "DEFS.H:810-816 defines C30 and M070 hand-slot routing, DEFS.H:2088 defines C10 color, DEFS.H:3906-3913 defines C537..C544, and M516/G0425/G0426 are the champion and open-chest globals";
}

static int seed_runtime(PartialDropRuntimePc34* runtime)
{
    M11_Item linked[5];

    memset(runtime, 0, sizeof(*runtime));
    m11_inventory_init(&runtime->inventory, 1);
    linked[0] = make_stack(0x7631, 2, 1);
    linked[1] = make_stack(DM1_PC34_PARTIAL_DROP_STACK_TYPE,
                           DM1_PC34_PARTIAL_DROP_INITIAL_COUNT,
                           DM1_PC34_PARTIAL_DROP_UNIT_WEIGHT);
    linked[2] = make_stack(0x7632, 4, 3);
    linked[3] = make_stack(0x7633, 1, 5);
    linked[4] = make_stack(0x7634, 6, 1);

    /* ReDMCSB: CHEST.C F0333 lines 53-67 copies the container links into
     * G0425_aT_ChestSlots/C537..C544 while G0426 names the open chest. */
    return m11_inventory_open_chest(&runtime->inventory, 0,
                                    DM1_PC34_PARTIAL_DROP_CHEST_THING,
                                    linked, 5);
}

static int split_partial_stack_to_leader_hand(
    PartialDropRuntimePc34* runtime,
    DM1_V1_ChestPartialDropToFloorWhileOpenProbePc34* out)
{
    M11_Item slot;
    M11_Item remaining;
    M11_Item hand;
    int remainingCount;

    if (!m11_inventory_get_item_in_chest_slot(
            &runtime->inventory, 0, DM1_PC34_PARTIAL_DROP_TARGET_INDEX,
            &slot) ||
        slot.itemType != DM1_PC34_PARTIAL_DROP_STACK_TYPE ||
        slot.charges <= DM1_PC34_PARTIAL_DROP_PARTIAL_COUNT) {
        return 0;
    }

    out->initialStackCount = slot.charges;
    out->initialStackWeight = slot.weight;
    out->partialDropCount = DM1_PC34_PARTIAL_DROP_PARTIAL_COUNT;
    out->partialDropWeight =
        DM1_PC34_PARTIAL_DROP_PARTIAL_COUNT *
        DM1_PC34_PARTIAL_DROP_UNIT_WEIGHT;
    remainingCount = slot.charges - DM1_PC34_PARTIAL_DROP_PARTIAL_COUNT;
    out->remainingStackCount = remainingCount;
    out->remainingStackWeight =
        remainingCount * DM1_PC34_PARTIAL_DROP_UNIT_WEIGHT;

    /* ReDMCSB: CHAMPION.C F0300 lines 511-515 is the C30+ removal boundary.
     * This synthetic split removes only the requested count from the visible
     * G0425 entry, then F0301 lines 606-614 leaves the remainder in G0425. */
    remaining = make_stack(slot.itemType, remainingCount,
                           DM1_PC34_PARTIAL_DROP_UNIT_WEIGHT);
    if (!m11_inventory_set_item_in_chest_slot(
            &runtime->inventory, 0, DM1_PC34_PARTIAL_DROP_TARGET_INDEX,
            remaining.itemType, remaining.weight, remaining.charges,
            remaining.allowedSlots)) {
        return 0;
    }
    out->f0300RemovedFromG0425 = 1;
    out->f0301PreservedRemainingInG0425 = 1;

    /* ReDMCSB: CHAMPION.C F0297 lines 243-268 puts the selected thing in the
     * leader hand. Here the partial stack is represented by charges/count. */
    if (!m11_inventory_set_mouse_item(&runtime->inventory, 0, slot.itemType,
                                      out->partialDropWeight,
                                      out->partialDropCount,
                                      slot.allowedSlots) ||
        !m11_inventory_get_mouse_item(&runtime->inventory, 0, &hand)) {
        return 0;
    }
    out->f0297PutPartialInLeaderHand = 1;
    out->leaderHandAfterSplit = snapshot_stack(hand);
    out->f0133PartialMaskDispatches = 2;
    m11_inventory_recalc_load(&runtime->inventory, 0);
    return 1;
}

static int link_leader_hand_to_floor(
    PartialDropRuntimePc34* runtime,
    DM1_V1_ChestPartialDropToFloorWhileOpenProbePc34* out)
{
    M11_Item hand;

    if (runtime->floorCount >= 1 ||
        !m11_inventory_get_mouse_item(&runtime->inventory, 0, &hand) ||
        hand.itemType == 0) {
        return 0;
    }

    /* ReDMCSB: CHAMPION.C F0298 lines 270-298 removes the leader-hand object;
     * DUNGEON.C F0163 lines 1796-1837 then links that thing to the floor. */
    runtime->floor[runtime->floorCount++] = hand;
    out->f0298RemovedLeaderHandForFloor = 1;
    out->f0163FloorLinkCount = 1;
    out->floorDropCount = runtime->floorCount;
    out->floorType = hand.itemType;
    out->floorWeight = hand.weight;
    out->floorMapX = DM1_PC34_PARTIAL_DROP_FLOOR_X;
    out->floorMapY = DM1_PC34_PARTIAL_DROP_FLOOR_Y;

    if (!m11_inventory_set_mouse_item(&runtime->inventory, 0, 0, 0, 0,
                                      DM1_PC34_ALLOWED_ANY_SLOT) ||
        !m11_inventory_get_mouse_item(&runtime->inventory, 0, &hand)) {
        return 0;
    }
    out->leaderHandAfterFloor = snapshot_stack(hand);
    m11_inventory_recalc_load(&runtime->inventory, 0);
    return 1;
}

int dm1_v1_chest_partial_drop_to_floor_while_chest_open_run_pc34(
    DM1_V1_ChestPartialDropToFloorWhileOpenProbePc34* out)
{
    PartialDropRuntimePc34 runtime;
    M11_Item hand;
    M11_Item closed[DM1_PC34_PARTIAL_DROP_SLOT_COUNT];
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    out->sourceLockedContractOnly = 1;
    out->sourcePc34Slot =
        DM1_PC34_PARTIAL_DROP_C30 + DM1_PC34_PARTIAL_DROP_TARGET_INDEX;
    out->sourceZone =
        DM1_PC34_PARTIAL_DROP_C537 + DM1_PC34_PARTIAL_DROP_TARGET_INDEX;
    out->sourceSlotIndex = DM1_PC34_PARTIAL_DROP_TARGET_INDEX;
    out->sourceAllowedByC30Mask =
        m11_inventory_pc34_slot_mask(out->sourcePc34Slot) ==
        DM1_PC34_ALLOWED_CONTAINER;
    out->commandPanelChest = DM1_PC34_PARTIAL_DROP_PANEL_CHEST;
    out->commandDispatchC040 = 1;

    out->openResult = seed_runtime(&runtime);
    out->openChestThing = m11_inventory_get_open_chest_thing(
        &runtime.inventory, 0);
    out->panelAfterOpen = m11_inventory_get_panel_content_pc34(
        &runtime.inventory);
    if (!out->openResult ||
        !m11_inventory_get_mouse_item(&runtime.inventory, 0, &hand)) {
        return 0;
    }
    out->leaderHandBefore = snapshot_stack(hand);
    copy_chest_slots(&runtime.inventory, out->chestBefore);
    out->openVisibleCountBefore = count_visible(out->chestBefore);

    if (!split_partial_stack_to_leader_hand(&runtime, out)) {
        return 0;
    }
    if (!link_leader_hand_to_floor(&runtime, out)) {
        return 0;
    }
    copy_chest_slots(&runtime.inventory, out->chestAfter);
    out->openVisibleCountAfter = count_visible(out->chestAfter);

    memset(closed, 0, sizeof(closed));
    out->closeCount = m11_inventory_close_chest(
        &runtime.inventory, 0, closed, DM1_PC34_PARTIAL_DROP_SLOT_COUNT);
    for (i = 0; i < DM1_PC34_PARTIAL_DROP_SLOT_COUNT; ++i) {
        out->closedChain[i] = snapshot_stack(closed[i]);
        if (closed[i].itemType == DM1_PC34_PARTIAL_DROP_STACK_TYPE) {
            out->closedTargetIndex = i;
        }
    }

    out->chestVisibleChainUpdated =
        out->chestAfter[DM1_PC34_PARTIAL_DROP_TARGET_INDEX].type ==
            DM1_PC34_PARTIAL_DROP_STACK_TYPE &&
        out->chestAfter[DM1_PC34_PARTIAL_DROP_TARGET_INDEX].count ==
            out->remainingStackCount &&
        out->chestAfter[DM1_PC34_PARTIAL_DROP_TARGET_INDEX].weight ==
            out->remainingStackWeight &&
        out->openVisibleCountAfter == out->openVisibleCountBefore;
    out->leaderHandUpdated =
        out->leaderHandBefore.type == 0 &&
        out->leaderHandAfterSplit.type == DM1_PC34_PARTIAL_DROP_STACK_TYPE &&
        out->leaderHandAfterSplit.count == out->partialDropCount &&
        out->leaderHandAfterFloor.type == 0;
    out->floorReceivedPartial =
        out->floorDropCount == 1 &&
        out->floorType == DM1_PC34_PARTIAL_DROP_STACK_TYPE &&
        out->floorWeight == out->partialDropWeight;
    out->closedChainPreservesRemaining =
        out->closeCount == out->openVisibleCountAfter &&
        out->closedChain[DM1_PC34_PARTIAL_DROP_TARGET_INDEX].type ==
            DM1_PC34_PARTIAL_DROP_STACK_TYPE &&
        out->closedChain[DM1_PC34_PARTIAL_DROP_TARGET_INDEX].count ==
            out->remainingStackCount &&
        out->closedTargetIndex == DM1_PC34_PARTIAL_DROP_TARGET_INDEX;
    out->deterministic =
        out->chestVisibleChainUpdated &&
        out->leaderHandUpdated &&
        out->floorReceivedPartial &&
        out->closedChainPreservesRemaining &&
        out->f0163FloorLinkCount == 1;
    out->deterministicHash = deterministic_hash(out);
    return out->deterministic;
}
