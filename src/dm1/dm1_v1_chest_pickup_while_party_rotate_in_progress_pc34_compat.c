#include "firestaff/dm1/v1/chest/pickup_while_party_rotate_in_progress_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB anchors for this contract-only runtime driver:
 * CHEST.C F0333:30-67 + F0334:113-132, CHAMPION.C F0284:93-131,
 * F0297:243-298, F0298:270-298, F0300:511-515, F0301:606-614,
 * F0302:662-714, PANEL.C F0344/F0345/F0352, COMMAND.C F0359:1985-1990,
 * and DEFS.H C30/G0425/G0426/G0423/G0305/M070/M516/C537..C544.
 */

typedef struct {
    int itemType;
    int weight;
    int charges;
    int quantity;
    int allowedSlots;
} RotatePickupThingPc34;

typedef struct {
    M11_InventoryState inventory;
    RotatePickupThingPc34 linked[DM1_PC34_ROTATE_PICKUP_SLOT_COUNT];
    RotatePickupThingPc34 rotateBuffer[DM1_PC34_ROTATE_PICKUP_SLOT_COUNT];
    RotatePickupThingPc34 pendingPickup;
    int oldLeaderIndex;
    int newLeaderIndex;
    int currentLeaderIndex;
    int partyDirection;
    int championDirection[DM1_PC34_ROTATE_PICKUP_CHAMPION_COUNT];
    int championCell[DM1_PC34_ROTATE_PICKUP_CHAMPION_COUNT];
    int quantityByChampion[DM1_PC34_ROTATE_PICKUP_CHAMPION_COUNT]
                          [DM1_PC34_ROTATE_PICKUP_SLOT_COUNT];
    int handQuantity[DM1_PC34_ROTATE_PICKUP_CHAMPION_COUNT];
    int rotateInProgress;
    int queued;
    int queuedChampion;
    int queuedOpenChestThing;
} RotatePickupRuntimePc34;

static const char s_source_evidence[] =
    "CHEST.C F0333:30-67 materializes G0425/C537..C544 from the open chest\n"
    "CHEST.C F0334:113-132 relinks non-empty G0425 slots on close\n"
    "CHAMPION.C F0284:93-131 rotates party direction/cell state before commit\n"
    "CHAMPION.C F0297:243-298/F0298:270-298 preserve leader-hand thing metadata\n"
    "CHAMPION.C F0300:511-515 clears C30+ slots through G0425\n"
    "CHAMPION.C F0301:606-614 writes C30+ slots through G0425\n"
    "CHAMPION.C F0302:662-714 routes C537..C544 slot-box clicks to C30+ slots\n"
    "PANEL.C F0344/F0345/F0352 and COMMAND.C F0359:1985-1990 feed the pointer route\n"
    "DEFS.H:2088 and C30/G0425/G0426/G0423/G0305/M070/M516/C537..C544 define the live chain";

static const DM1_V1_ChestPickupWhilePartyRotateSpecPc34 s_spec = {
    "Source-locked contract-only gate: C537 pickup queued between party-rotate trigger and commit lands on the new leader without overwriting the rotate buffer.",
    "CHEST.C F0333 lines 30-67 open chest materializes C537..C544",
    "CHEST.C F0334 lines 113-132 close rewrites visible C537..C544",
    "CHAMPION.C F0284 lines 93-131 party rotate trigger/commit",
    "CHAMPION.C F0297 lines 243-298 put object in leader hand",
    "CHAMPION.C F0298 lines 270-298 remove object from leader hand",
    "CHAMPION.C F0300 lines 511-515 clear C30+ G0425 slot",
    "CHAMPION.C F0301 lines 606-614 write C30+ G0425 slot",
    "CHAMPION.C F0302 lines 662-714 C537 slot-box dispatch",
    "PANEL.C F0344/F0345/F0352 pointer panel route",
    "COMMAND.C F0359 lines 1985-1990 command slot-box dispatch",
    "DEFS.H C30/G0425/G0426/G0423/G0305/M070/M516/C537..C544",
    DM1_PC34_ROTATE_PICKUP_OLD_LEADER,
    DM1_PC34_ROTATE_PICKUP_NEW_LEADER,
    DM1_PC34_ROTATE_PICKUP_PICKED_ZONE,
    DM1_PC34_ROTATE_PICKUP_PICKED_PC34_SLOT,
    DM1_PC34_ROTATE_PICKUP_SLOT_BOX_C38,
    DM1_PC34_ROTATE_PICKUP_SLOT_COUNT - 1
};

static M11_Item to_m11_item(RotatePickupThingPc34 thing)
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

static RotatePickupThingPc34 make_thing(int index)
{
    RotatePickupThingPc34 thing;

    thing.itemType = DM1_PC34_ROTATE_PICKUP_FIRST_ITEM + index;
    thing.weight = 7 + index;
    thing.charges = DM1_PC34_ROTATE_PICKUP_FIRST_CHARGES + index;
    thing.quantity = DM1_PC34_ROTATE_PICKUP_FIRST_QUANTITY + index;
    thing.allowedSlots = DM1_PC34_ALLOWED_CONTAINER;
    return thing;
}

static int normalize_direction(int value)
{
    return value & 3;
}

static void copy_linked_to_m11(const RotatePickupThingPc34* linked,
                               M11_Item* items,
                               int count)
{
    int i;

    for (i = 0; i < count; ++i) {
        items[i] = to_m11_item(linked[i]);
    }
}

static void record_slots(const M11_InventoryState* inventory,
                         int champion,
                         const int quantities[DM1_PC34_ROTATE_PICKUP_SLOT_COUNT],
                         int* types,
                         int* charges,
                         int* outQuantities)
{
    int i;

    for (i = 0; i < DM1_PC34_ROTATE_PICKUP_SLOT_COUNT; ++i) {
        M11_Item item;

        if (m11_inventory_get_item_in_chest_slot(
                inventory, champion, i, &item)) {
            types[i] = item.itemType;
            charges[i] = item.charges;
            outQuantities[i] = quantities[i];
        } else {
            types[i] = 0;
            charges[i] = 0;
            outQuantities[i] = 0;
        }
    }
}

static void record_things(const RotatePickupThingPc34* things,
                          int* types,
                          int* charges,
                          int* quantities)
{
    int i;

    for (i = 0; i < DM1_PC34_ROTATE_PICKUP_SLOT_COUNT; ++i) {
        types[i] = things[i].itemType;
        charges[i] = things[i].charges;
        quantities[i] = things[i].quantity;
    }
}

static int arrays_equal(const int* a, const int* b)
{
    int i;

    for (i = 0; i < DM1_PC34_ROTATE_PICKUP_SLOT_COUNT; ++i) {
        if (a[i] != b[i]) {
            return 0;
        }
    }
    return 1;
}

static int count_nonempty_types(const int* types)
{
    int i;
    int count = 0;

    for (i = 0; i < DM1_PC34_ROTATE_PICKUP_SLOT_COUNT; ++i) {
        if (types[i] != 0) {
            ++count;
        }
    }
    return count;
}

static int count_type_in_slots(const int* types, int itemType)
{
    int i;
    int count = 0;

    for (i = 0; i < DM1_PC34_ROTATE_PICKUP_SLOT_COUNT; ++i) {
        if (types[i] == itemType) {
            ++count;
        }
    }
    return count;
}

static void runtime_init(RotatePickupRuntimePc34* runtime)
{
    M11_Item linked[DM1_PC34_ROTATE_PICKUP_SLOT_COUNT];
    int i;

    memset(runtime, 0, sizeof(*runtime));
    runtime->oldLeaderIndex = DM1_PC34_ROTATE_PICKUP_OLD_LEADER;
    runtime->newLeaderIndex = DM1_PC34_ROTATE_PICKUP_NEW_LEADER;
    runtime->currentLeaderIndex = runtime->oldLeaderIndex;
    runtime->partyDirection = 0;
    for (i = 0; i < DM1_PC34_ROTATE_PICKUP_CHAMPION_COUNT; ++i) {
        runtime->championDirection[i] = i;
        runtime->championCell[i] = i;
    }
    for (i = 0; i < DM1_PC34_ROTATE_PICKUP_SLOT_COUNT; ++i) {
        runtime->linked[i] = make_thing(i);
        runtime->quantityByChampion[runtime->oldLeaderIndex][i] =
            runtime->linked[i].quantity;
    }

    m11_inventory_init(&runtime->inventory,
                       DM1_PC34_ROTATE_PICKUP_CHAMPION_COUNT);
    copy_linked_to_m11(runtime->linked, linked,
                       DM1_PC34_ROTATE_PICKUP_SLOT_COUNT);
    (void)m11_inventory_open_chest(
        &runtime->inventory, runtime->oldLeaderIndex,
        DM1_PC34_ROTATE_PICKUP_CHEST_THING, linked,
        DM1_PC34_ROTATE_PICKUP_SLOT_COUNT);
}

static void trigger_rotate(RotatePickupRuntimePc34* runtime, int newDirection)
{
    int delta;
    int i;

    runtime->rotateInProgress = 1;
    runtime->partyDirection = newDirection;
    delta = newDirection;

    /* ReDMCSB: CHAMPION.C F0284 lines 117-131 applies the direction delta to
     * every M516 champion before G0308 commits the new party direction. */
    for (i = 0; i < DM1_PC34_ROTATE_PICKUP_CHAMPION_COUNT; ++i) {
        runtime->championDirection[i] =
            normalize_direction(runtime->championDirection[i] + delta);
        runtime->championCell[i] =
            normalize_direction(runtime->championCell[i] + delta);
    }

    for (i = 0; i < DM1_PC34_ROTATE_PICKUP_SLOT_COUNT; ++i) {
        M11_Item item;

        if (m11_inventory_get_item_in_chest_slot(
                &runtime->inventory, runtime->oldLeaderIndex, i, &item)) {
            runtime->rotateBuffer[i].itemType = item.itemType;
            runtime->rotateBuffer[i].weight = item.weight;
            runtime->rotateBuffer[i].charges = item.charges;
            runtime->rotateBuffer[i].quantity =
                runtime->quantityByChampion[runtime->oldLeaderIndex][i];
            runtime->rotateBuffer[i].allowedSlots = item.allowedSlots;
        }
    }
}

static int pointer_route_queue_c537_pickup(RotatePickupRuntimePc34* runtime)
{
    int slotIndex = DM1_PC34_ROTATE_PICKUP_PICKED_SLOT_INDEX;

    if (!runtime || !runtime->rotateInProgress || runtime->queued ||
        m11_inventory_get_open_chest_thing(
            &runtime->inventory, runtime->oldLeaderIndex) !=
            DM1_PC34_ROTATE_PICKUP_CHEST_THING) {
        return 0;
    }

    /* ReDMCSB: PANEL.C F0344/F0345/F0352 and COMMAND.C F0359 feed the C537
     * slot-box command, while CHAMPION.C F0302 lines 688-690 reads the C30+
     * G0425 slot.  During this in-flight rotate window, the driver queues the
     * selected thing and intentionally leaves the rotate buffer untouched. */
    runtime->pendingPickup = runtime->rotateBuffer[slotIndex];
    runtime->queued = 1;
    runtime->queuedChampion = runtime->oldLeaderIndex;
    runtime->queuedOpenChestThing = DM1_PC34_ROTATE_PICKUP_CHEST_THING;
    return 1;
}

static int commit_rotate_and_pickup(RotatePickupRuntimePc34* runtime)
{
    M11_Item linked[DM1_PC34_ROTATE_PICKUP_SLOT_COUNT];
    M11_Item picked;
    int i;

    if (!runtime || !runtime->rotateInProgress || !runtime->queued) {
        return 0;
    }

    copy_linked_to_m11(runtime->rotateBuffer, linked,
                       DM1_PC34_ROTATE_PICKUP_SLOT_COUNT);
    runtime->currentLeaderIndex = runtime->newLeaderIndex;
    for (i = 0; i < DM1_PC34_ROTATE_PICKUP_SLOT_COUNT; ++i) {
        runtime->quantityByChampion[runtime->newLeaderIndex][i] =
            runtime->rotateBuffer[i].quantity;
    }
    if (!m11_inventory_open_chest(
            &runtime->inventory, runtime->newLeaderIndex,
            DM1_PC34_ROTATE_PICKUP_CHEST_THING, linked,
            DM1_PC34_ROTATE_PICKUP_SLOT_COUNT)) {
        return 0;
    }

    /* ReDMCSB: F0302 dispatches the already queued C537/C30 click after the
     * rotate commit.  The landing target is now the new leader's G0425 chain,
     * so F0300 clears that C30 slot and F0297 puts the same thing in hand. */
    if (!m11_inventory_click_open_chest_slot_for_thing(
            &runtime->inventory, runtime->newLeaderIndex,
            DM1_PC34_ROTATE_PICKUP_CHEST_THING,
            DM1_PC34_ROTATE_PICKUP_PICKED_SLOT_INDEX)) {
        return 0;
    }
    if (!m11_inventory_get_mouse_item(
            &runtime->inventory, runtime->newLeaderIndex, &picked)) {
        return 0;
    }
    if (picked.itemType != runtime->pendingPickup.itemType ||
        picked.charges != runtime->pendingPickup.charges ||
        picked.weight != runtime->pendingPickup.weight) {
        return 0;
    }
    runtime->handQuantity[runtime->newLeaderIndex] =
        runtime->pendingPickup.quantity;
    runtime->quantityByChampion[runtime->newLeaderIndex]
                               [DM1_PC34_ROTATE_PICKUP_PICKED_SLOT_INDEX] = 0;
    runtime->rotateInProgress = 0;
    return 1;
}

static int close_new_leader_chest(RotatePickupRuntimePc34* runtime,
                                  RotatePickupThingPc34* closed)
{
    M11_Item items[DM1_PC34_ROTATE_PICKUP_SLOT_COUNT];
    int count;
    int outIndex = 0;
    int i;

    memset(items, 0, sizeof(items));
    count = m11_inventory_close_chest(
        &runtime->inventory, runtime->newLeaderIndex, items,
        DM1_PC34_ROTATE_PICKUP_SLOT_COUNT);
    if (count < 0) {
        return -1;
    }

    /* ReDMCSB: CHEST.C F0334 lines 117-132 skips empty G0425 entries while it
     * relinks the visible chain.  The sidecar quantity follows the same
     * compacted visible order used by the runtime thing metadata in this gate. */
    for (i = 0; i < DM1_PC34_ROTATE_PICKUP_SLOT_COUNT; ++i) {
        if (runtime->quantityByChampion[runtime->newLeaderIndex][i] != 0) {
            closed[outIndex].itemType = items[outIndex].itemType;
            closed[outIndex].weight = items[outIndex].weight;
            closed[outIndex].charges = items[outIndex].charges;
            closed[outIndex].quantity =
                runtime->quantityByChampion[runtime->newLeaderIndex][i];
            closed[outIndex].allowedSlots = items[outIndex].allowedSlots;
            ++outIndex;
        }
    }
    return count;
}

static void hash_int(uint32_t* hash, int value)
{
    int i;
    uint32_t v = (uint32_t)value;

    for (i = 0; i < 4; ++i) {
        *hash ^= (v >> (i * 8)) & 0xFFu;
        *hash *= 16777619u;
    }
}

static void hash_probe(uint32_t* hash,
                       const DM1_V1_ChestPickupWhilePartyRotateProbePc34* p)
{
    int i;

    hash_int(hash, p->stepCount);
    hash_int(hash, p->leaderAfterCommit);
    hash_int(hash, p->pointerRouteQueued);
    hash_int(hash, p->queuedThingType);
    hash_int(hash, p->queuedThingCharges);
    hash_int(hash, p->queuedThingQuantity);
    hash_int(hash, p->rotateBufferPreservedByQueue);
    hash_int(hash, p->commitPickupResult);
    hash_int(hash, p->newLeaderHandTypeAfterCommit);
    hash_int(hash, p->newLeaderHandChargesAfterCommit);
    hash_int(hash, p->newLeaderHandQuantityAfterCommit);
    hash_int(hash, p->closeCount);
    hash_int(hash, p->totalPickedCopiesAfterClose);
    for (i = 0; i < DM1_PC34_ROTATE_PICKUP_SLOT_COUNT; ++i) {
        hash_int(hash, p->visibleTypesAfterCommit[i]);
        hash_int(hash, p->closedTypes[i]);
        hash_int(hash, p->closedQuantities[i]);
    }
}

const char*
dm1_v1_chest_pickup_while_party_rotate_in_progress_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const DM1_V1_ChestPickupWhilePartyRotateSpecPc34*
dm1_v1_chest_pickup_while_party_rotate_in_progress_spec_pc34(void)
{
    return &s_spec;
}

int dm1_v1_chest_pickup_while_party_rotate_in_progress_run_pc34(
    DM1_V1_ChestPickupWhilePartyRotateProbePc34* out)
{
    RotatePickupRuntimePc34 runtime;
    RotatePickupThingPc34 closed[DM1_PC34_ROTATE_PICKUP_SLOT_COUNT];
    M11_Item hand;
    uint32_t hash = 2166136261u;
    int pickedItemType =
        DM1_PC34_ROTATE_PICKUP_FIRST_ITEM +
        DM1_PC34_ROTATE_PICKUP_PICKED_SLOT_INDEX;

    if (!out) {
        return 0;
    }

    memset(out, 0, sizeof(*out));
    memset(closed, 0, sizeof(closed));
    runtime_init(&runtime);
    out->sourceLockedContractOnly = 1;
    out->stepTrace[out->stepCount++] =
        DM1_PC34_ROTATE_PICKUP_STEP_SETUP_OPEN;
    out->openResult = 1;
    out->openChestThingBeforeRotate = m11_inventory_get_open_chest_thing(
        &runtime.inventory, runtime.oldLeaderIndex);
    out->leaderBeforeRotate = runtime.currentLeaderIndex;
    out->partyDirectionBefore = runtime.partyDirection;

    trigger_rotate(&runtime, 1);
    out->stepTrace[out->stepCount++] =
        DM1_PC34_ROTATE_PICKUP_STEP_PARTY_ROTATE_TRIGGER;
    out->leaderAfterTrigger = runtime.currentLeaderIndex;
    out->partyDirectionAfterTrigger = runtime.partyDirection;
    record_things(runtime.rotateBuffer,
                  out->rotateBufferTypeBeforeQueue,
                  out->rotateBufferChargesBeforeQueue,
                  out->rotateBufferQuantityBeforeQueue);

    out->pointerZone = DM1_PC34_ROTATE_PICKUP_PICKED_ZONE;
    out->pointerPanelCommand = DM1_PC34_ROTATE_PICKUP_PANEL_M568;
    out->pointerSlotBox = DM1_PC34_ROTATE_PICKUP_SLOT_BOX_C38;
    out->pointerPc34Slot = DM1_PC34_ROTATE_PICKUP_PICKED_PC34_SLOT;
    out->pointerRouteQueued = pointer_route_queue_c537_pickup(&runtime);
    out->stepTrace[out->stepCount++] =
        DM1_PC34_ROTATE_PICKUP_STEP_C537_POINTER_QUEUE;
    out->queueStepBetweenTriggerAndCommit =
        out->stepTrace[1] ==
            DM1_PC34_ROTATE_PICKUP_STEP_PARTY_ROTATE_TRIGGER &&
        out->stepTrace[2] ==
            DM1_PC34_ROTATE_PICKUP_STEP_C537_POINTER_QUEUE ? 1 : 0;
    out->queuedAgainstChampion = runtime.queuedChampion;
    out->queuedAgainstOpenChestThing = runtime.queuedOpenChestThing;
    out->queuedThingType = runtime.pendingPickup.itemType;
    out->queuedThingCharges = runtime.pendingPickup.charges;
    out->queuedThingQuantity = runtime.pendingPickup.quantity;
    out->queuedThingWeight = runtime.pendingPickup.weight;
    out->queuedThingAllowedSlots = runtime.pendingPickup.allowedSlots;
    record_things(runtime.rotateBuffer,
                  out->rotateBufferTypeAfterQueue,
                  out->rotateBufferChargesAfterQueue,
                  out->rotateBufferQuantityAfterQueue);
    out->rotateBufferPreservedByQueue =
        arrays_equal(out->rotateBufferTypeBeforeQueue,
                     out->rotateBufferTypeAfterQueue) &&
        arrays_equal(out->rotateBufferChargesBeforeQueue,
                     out->rotateBufferChargesAfterQueue) &&
        arrays_equal(out->rotateBufferQuantityBeforeQueue,
                     out->rotateBufferQuantityAfterQueue) ? 1 : 0;

    out->commitPickupResult = commit_rotate_and_pickup(&runtime);
    out->stepTrace[out->stepCount++] =
        DM1_PC34_ROTATE_PICKUP_STEP_PARTY_ROTATE_COMMIT;
    out->leaderAfterCommit = runtime.currentLeaderIndex;
    out->partyDirectionAfterCommit = runtime.partyDirection;
    out->oldLeaderDirectionAfterCommit =
        runtime.championDirection[runtime.oldLeaderIndex];
    out->newLeaderDirectionAfterCommit =
        runtime.championDirection[runtime.newLeaderIndex];
    out->newLeaderOpenChestThingAfterCommit =
        m11_inventory_get_open_chest_thing(
            &runtime.inventory, runtime.newLeaderIndex);
    out->newLeaderOpenResult =
        out->newLeaderOpenChestThingAfterCommit ==
        DM1_PC34_ROTATE_PICKUP_CHEST_THING ? 1 : 0;
    out->commitLandedAgainstNewLeader =
        out->commitPickupResult &&
        out->leaderAfterCommit == runtime.newLeaderIndex &&
        out->newLeaderOpenResult ? 1 : 0;
    (void)m11_inventory_get_mouse_item(
        &runtime.inventory, runtime.newLeaderIndex, &hand);
    out->newLeaderHandTypeAfterCommit = hand.itemType;
    out->newLeaderHandChargesAfterCommit = hand.charges;
    out->newLeaderHandQuantityAfterCommit =
        runtime.handQuantity[runtime.newLeaderIndex];
    out->newLeaderHandWeightAfterCommit = hand.weight;
    out->pickedMetadataPreserved =
        out->newLeaderHandTypeAfterCommit == out->queuedThingType &&
        out->newLeaderHandChargesAfterCommit == out->queuedThingCharges &&
        out->newLeaderHandQuantityAfterCommit == out->queuedThingQuantity &&
        out->newLeaderHandWeightAfterCommit == out->queuedThingWeight ? 1 : 0;
    record_slots(&runtime.inventory, runtime.newLeaderIndex,
                 runtime.quantityByChampion[runtime.newLeaderIndex],
                 out->visibleTypesAfterCommit,
                 out->visibleChargesAfterCommit,
                 out->visibleQuantitiesAfterCommit);
    out->pickedSlotEmptyAfterCommit =
        out->visibleTypesAfterCommit
            [DM1_PC34_ROTATE_PICKUP_PICKED_SLOT_INDEX] == 0 ? 1 : 0;
    out->visibleCountAfterCommit =
        count_nonempty_types(out->visibleTypesAfterCommit);

    out->closeCount = close_new_leader_chest(&runtime, closed);
    out->stepTrace[out->stepCount++] =
        DM1_PC34_ROTATE_PICKUP_STEP_CHEST_CLOSE_REWRITE;
    out->closeAgainstChampion = runtime.newLeaderIndex;
    out->closeAgainstNewLeader =
        out->closeAgainstChampion == DM1_PC34_ROTATE_PICKUP_NEW_LEADER ? 1 : 0;
    record_things(closed, out->closedTypes, out->closedCharges,
                  out->closedQuantities);
    out->closeRewroteVisibleChainAgainstNewLeader =
        out->closeCount == DM1_PC34_ROTATE_PICKUP_SLOT_COUNT - 1 &&
        out->closedTypes[0] == DM1_PC34_ROTATE_PICKUP_FIRST_ITEM + 1 &&
        out->closedTypes[DM1_PC34_ROTATE_PICKUP_SLOT_COUNT - 2] ==
            DM1_PC34_ROTATE_PICKUP_FIRST_ITEM +
            DM1_PC34_ROTATE_PICKUP_SLOT_COUNT - 1 &&
        out->closedTypes[DM1_PC34_ROTATE_PICKUP_SLOT_COUNT - 1] == 0 ? 1 : 0;
    out->closedPickedThingAbsent =
        count_type_in_slots(out->closedTypes, pickedItemType) == 0 ? 1 : 0;
    out->totalPickedCopiesAfterClose =
        count_type_in_slots(out->closedTypes, pickedItemType) +
        (out->newLeaderHandTypeAfterCommit == pickedItemType ? 1 : 0);

    hash_probe(&hash, out);
    out->deterministicHash = hash;
    return out->openResult &&
           out->pointerRouteQueued &&
           out->queueStepBetweenTriggerAndCommit &&
           out->rotateBufferPreservedByQueue &&
           out->commitLandedAgainstNewLeader &&
           out->pickedMetadataPreserved &&
           out->pickedSlotEmptyAfterCommit &&
           out->visibleCountAfterCommit ==
               DM1_PC34_ROTATE_PICKUP_SLOT_COUNT - 1 &&
           out->closeAgainstNewLeader &&
           out->closeRewroteVisibleChainAgainstNewLeader &&
           out->closedPickedThingAbsent &&
           out->totalPickedCopiesAfterClose == 1;
}
