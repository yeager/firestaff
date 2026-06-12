#include "firestaff/dm1/v1/chest/close_while_party_rotate_pickup_pending_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB: CHEST.C F0334 lines 113-132 closes the current G0426 chest by
 * clearing G0426 and compacting non-empty G0425 entries.  This driver
 * intentionally closes after CHAMPION.C F0302/F0300/F0297 has begun a C537
 * pickup but before COMMAND.C F0380 lines 2150-2152 finishes the turn frame.
 */

typedef struct {
    int itemType;
    int weight;
    int charges;
    int quantity;
    int allowedSlots;
} CloseRotatePickupThingPc34;

typedef struct {
    M11_InventoryState inventory;
    CloseRotatePickupThingPc34 linked[DM1_PC34_CLOSE_ROTATE_PICKUP_SLOT_COUNT];
    CloseRotatePickupThingPc34 pendingPickup;
    int currentLeaderIndex;
    int oldLeaderIndex;
    int newLeaderIndex;
    int partyDirection;
    int championDirection[DM1_PC34_CLOSE_ROTATE_PICKUP_CHAMPION_COUNT];
    int championCell[DM1_PC34_CLOSE_ROTATE_PICKUP_CHAMPION_COUNT];
    int quantityBySlot[DM1_PC34_CLOSE_ROTATE_PICKUP_SLOT_COUNT];
    int leaderHandQuantity;
    int rotateInProgress;
    int queued;
    int queuedChampion;
    int queuedOpenChestThing;
} CloseRotatePickupRuntimePc34;

static const char s_source_evidence[] =
    "CHEST.C F0333:30-76 materializes G0426 into G0425/C537..C544\n"
    "CHEST.C F0334:113-132 clears G0426 and relinks non-empty G0425 slots\n"
    "CHAMPION.C F0284:93-131 rotates party direction/cell state\n"
    "CHAMPION.C F0297:243-268/F0298:270-298 own leader-hand object state\n"
    "CHAMPION.C F0300:511-515 clears C30+ slots through G0425\n"
    "CHAMPION.C F0301:606-614 writes C30+ slots through G0425\n"
    "CHAMPION.C F0302:662-714 routes C537..C544 slot-box clicks\n"
    "COMMAND.C F0359:1973-1983 and F0380:2045-2152 feed panel/pending click work\n"
    "PANEL.C:7-13 owns G0423/G0425/G0426 panel globals\n"
    "DEFS.H:810,1878,3001-3008,3906-3913,5876-5881 defines C30/M070/M568/M569/C537..C544/G0423/G0425/G0426";

static const DM1_V1_ChestCloseWhilePartyRotatePickupSpecPc34 s_spec = {
    "Source-locked contract-only gate: close current G0426 while rotate-triggered C537 pickup is in flight, compacting seven objects and rejecting stale closed-chest pickup replay.",
    "CHEST.C F0333 lines 30-76 open/materialize G0426 into C537..C544",
    "CHEST.C F0334 lines 113-132 close clears G0426 and relinks non-empty G0425",
    "CHAMPION.C F0284 lines 93-131 party rotate trigger/commit",
    "CHAMPION.C F0297 lines 243-268 put object in leader hand",
    "CHAMPION.C F0298 lines 270-298 remove object from leader hand",
    "CHAMPION.C F0300 lines 511-515 clear C30+ G0425 slot",
    "CHAMPION.C F0301 lines 606-614 write C30+ G0425 slot",
    "CHAMPION.C F0302 lines 662-714 C537 slot-box dispatch",
    "COMMAND.C F0359 lines 1973-1983 M569 chest panel dispatch",
    "COMMAND.C F0380 lines 2045-2152 queue/pending click/turn dispatch",
    "PANEL.C lines 7-13 G0423/G0425/G0426 panel globals",
    "DEFS.H lines 810,1878,3001-3008,3906-3913,5876-5881 C30/M070/M568/M569/C537..C544/G0423/G0425/G0426",
    DM1_PC34_CLOSE_ROTATE_PICKUP_SLOT_COUNT - 1
};

static M11_Item to_m11_item(CloseRotatePickupThingPc34 thing)
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

static CloseRotatePickupThingPc34 make_thing(int index)
{
    CloseRotatePickupThingPc34 thing;

    thing.itemType = DM1_PC34_CLOSE_ROTATE_PICKUP_FIRST_ITEM + index;
    thing.weight = 9 + index;
    thing.charges = DM1_PC34_CLOSE_ROTATE_PICKUP_FIRST_CHARGES + index;
    thing.quantity = DM1_PC34_CLOSE_ROTATE_PICKUP_FIRST_QUANTITY + index;
    thing.allowedSlots = DM1_PC34_ALLOWED_CONTAINER;
    return thing;
}

static int normalize_direction(int value)
{
    return value & 3;
}

static void copy_linked_to_m11(const CloseRotatePickupThingPc34* linked,
                               M11_Item* items)
{
    int i;

    for (i = 0; i < DM1_PC34_CLOSE_ROTATE_PICKUP_SLOT_COUNT; ++i) {
        items[i] = to_m11_item(linked[i]);
    }
}

static void runtime_init(CloseRotatePickupRuntimePc34* runtime)
{
    M11_Item linked[DM1_PC34_CLOSE_ROTATE_PICKUP_SLOT_COUNT];
    int i;

    memset(runtime, 0, sizeof(*runtime));
    runtime->oldLeaderIndex = DM1_PC34_CLOSE_ROTATE_PICKUP_OLD_LEADER;
    runtime->newLeaderIndex = DM1_PC34_CLOSE_ROTATE_PICKUP_NEW_LEADER;
    runtime->currentLeaderIndex = runtime->oldLeaderIndex;
    for (i = 0; i < DM1_PC34_CLOSE_ROTATE_PICKUP_CHAMPION_COUNT; ++i) {
        runtime->championDirection[i] = i;
        runtime->championCell[i] = i;
    }
    for (i = 0; i < DM1_PC34_CLOSE_ROTATE_PICKUP_SLOT_COUNT; ++i) {
        runtime->linked[i] = make_thing(i);
        runtime->quantityBySlot[i] = runtime->linked[i].quantity;
    }
    m11_inventory_init(&runtime->inventory,
                       DM1_PC34_CLOSE_ROTATE_PICKUP_CHAMPION_COUNT);
    copy_linked_to_m11(runtime->linked, linked);
    (void)m11_inventory_open_chest(
        &runtime->inventory, runtime->oldLeaderIndex,
        DM1_PC34_CLOSE_ROTATE_PICKUP_CHEST_THING, linked,
        DM1_PC34_CLOSE_ROTATE_PICKUP_SLOT_COUNT);
}

static void trigger_rotate(CloseRotatePickupRuntimePc34* runtime,
                           int newDirection)
{
    int delta = newDirection - runtime->partyDirection;
    int i;

    if (delta < 0) {
        delta += 4;
    }
    runtime->rotateInProgress = 1;
    runtime->partyDirection = newDirection;
    /* ReDMCSB: CHAMPION.C F0284 lines 117-130 applies the normalized delta
     * to every champion cell/direction before drawing changed object icons. */
    for (i = 0; i < DM1_PC34_CLOSE_ROTATE_PICKUP_CHAMPION_COUNT; ++i) {
        runtime->championDirection[i] =
            normalize_direction(runtime->championDirection[i] + delta);
        runtime->championCell[i] =
            normalize_direction(runtime->championCell[i] + delta);
    }
}

static int queue_c537_pickup(CloseRotatePickupRuntimePc34* runtime)
{
    if (!runtime || !runtime->rotateInProgress || runtime->queued ||
        m11_inventory_get_open_chest_thing(
            &runtime->inventory, runtime->oldLeaderIndex) !=
            DM1_PC34_CLOSE_ROTATE_PICKUP_CHEST_THING) {
        return 0;
    }

    /* ReDMCSB: COMMAND.C F0359 lines 1973-1983 routes the chest panel click
     * to CHAMPION.C F0302.  The queued click records the selected C537 thing
     * while COMMAND.C F0380 lines 2045-2152 is still processing the turn. */
    runtime->pendingPickup =
        runtime->linked[DM1_PC34_CLOSE_ROTATE_PICKUP_PICKED_SLOT_INDEX];
    runtime->queued = 1;
    runtime->queuedChampion = runtime->oldLeaderIndex;
    runtime->queuedOpenChestThing = DM1_PC34_CLOSE_ROTATE_PICKUP_CHEST_THING;
    return 1;
}

static int begin_c537_pickup(CloseRotatePickupRuntimePc34* runtime)
{
    M11_Item hand;

    if (!runtime || !runtime->queued) {
        return 0;
    }
    /* ReDMCSB: CHAMPION.C F0302 lines 688-706 reads C537/G0425, clears it
     * through F0300, then F0297 puts the same object in the leader hand. */
    if (!m11_inventory_click_open_chest_slot_for_thing(
            &runtime->inventory, runtime->oldLeaderIndex,
            runtime->queuedOpenChestThing,
            DM1_PC34_CLOSE_ROTATE_PICKUP_PICKED_SLOT_INDEX)) {
        return 0;
    }
    if (!m11_inventory_get_mouse_item(
            &runtime->inventory, runtime->oldLeaderIndex, &hand)) {
        return 0;
    }
    if (hand.itemType != runtime->pendingPickup.itemType ||
        hand.charges != runtime->pendingPickup.charges ||
        hand.weight != runtime->pendingPickup.weight) {
        return 0;
    }
    runtime->leaderHandQuantity = runtime->pendingPickup.quantity;
    runtime->quantityBySlot[DM1_PC34_CLOSE_ROTATE_PICKUP_PICKED_SLOT_INDEX] = 0;
    return 1;
}

static int close_current_chest(CloseRotatePickupRuntimePc34* runtime,
                               CloseRotatePickupThingPc34* closed)
{
    M11_Item items[DM1_PC34_CLOSE_ROTATE_PICKUP_SLOT_COUNT];
    int count;
    int i;

    memset(items, 0, sizeof(items));
    count = m11_inventory_close_chest(
        &runtime->inventory, runtime->oldLeaderIndex, items,
        DM1_PC34_CLOSE_ROTATE_PICKUP_SLOT_COUNT);
    if (count < 0) {
        return -1;
    }

    /* ReDMCSB: CHEST.C F0334 lines 117-132 skips C0xFFFF_THING_NONE slots.
     * The picked C537 slot was already cleared by F0300, so close must compact
     * only C538..C544 and must not recreate the leader-hand thing. */
    for (i = 0; i < count; ++i) {
        int sourceIndex = i + 1;
        closed[i].itemType = items[i].itemType;
        closed[i].weight = items[i].weight;
        closed[i].charges = items[i].charges;
        closed[i].quantity = runtime->quantityBySlot[sourceIndex];
        closed[i].allowedSlots = items[i].allowedSlots;
    }
    return count;
}

static int count_type(const CloseRotatePickupThingPc34* things,
                      int count,
                      int itemType)
{
    int i;
    int result = 0;

    for (i = 0; i < count; ++i) {
        if (things[i].itemType == itemType) {
            ++result;
        }
    }
    return result;
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

static void hash_probe(
    uint32_t* hash,
    const DM1_V1_ChestCloseWhilePartyRotatePickupProbePc34* p)
{
    int i;

    hash_int(hash, p->stepCount);
    hash_int(hash, p->queuedThingType);
    hash_int(hash, p->pickupBeginResult);
    hash_int(hash, p->pickedSlotEmptyBeforeClose);
    hash_int(hash, p->leaderHandTypeBeforeClose);
    hash_int(hash, p->closeCount);
    hash_int(hash, p->openChestThingAfterClose);
    hash_int(hash, p->leaderAfterCommit);
    hash_int(hash, p->stalePickupResultAfterClose);
    hash_int(hash, p->latePickupRejectedAgainstClosedG0426);
    hash_int(hash, p->pickedCopiesIncludingHand);
    for (i = 0; i < DM1_PC34_CLOSE_ROTATE_PICKUP_SLOT_COUNT; ++i) {
        hash_int(hash, p->closedTypes[i]);
        hash_int(hash, p->closedCharges[i]);
        hash_int(hash, p->closedQuantities[i]);
    }
}

const char*
dm1_v1_chest_close_while_party_rotate_pickup_pending_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const DM1_V1_ChestCloseWhilePartyRotatePickupSpecPc34*
dm1_v1_chest_close_while_party_rotate_pickup_pending_spec_pc34(void)
{
    return &s_spec;
}

int dm1_v1_chest_close_while_party_rotate_pickup_pending_run_pc34(
    DM1_V1_ChestCloseWhilePartyRotatePickupProbePc34* out)
{
    CloseRotatePickupRuntimePc34 runtime;
    CloseRotatePickupThingPc34 closed[DM1_PC34_CLOSE_ROTATE_PICKUP_SLOT_COUNT];
    M11_Item hand;
    M11_Item slot;
    uint32_t hash = 2166136261u;
    int pickedType =
        DM1_PC34_CLOSE_ROTATE_PICKUP_FIRST_ITEM +
        DM1_PC34_CLOSE_ROTATE_PICKUP_PICKED_SLOT_INDEX;
    int i;

    if (!out) {
        return 0;
    }

    memset(out, 0, sizeof(*out));
    memset(closed, 0, sizeof(closed));
    runtime_init(&runtime);
    out->sourceLockedContractOnly = 1;
    out->openResult = 1;
    out->stepTrace[out->stepCount++] =
        DM1_PC34_CLOSE_ROTATE_PICKUP_STEP_OPEN;
    out->openChestThingBeforeRotate = m11_inventory_get_open_chest_thing(
        &runtime.inventory, runtime.oldLeaderIndex);
    out->leaderBeforeRotate = runtime.currentLeaderIndex;
    out->partyDirectionBefore = runtime.partyDirection;

    trigger_rotate(&runtime, 1);
    out->stepTrace[out->stepCount++] =
        DM1_PC34_CLOSE_ROTATE_PICKUP_STEP_ROTATE_TRIGGER;
    out->leaderAfterTrigger = runtime.currentLeaderIndex;
    out->partyDirectionAfterTrigger = runtime.partyDirection;

    out->queuedCommand = DM1_PC34_CLOSE_ROTATE_PICKUP_COMMAND_C040;
    out->queuedPanel = DM1_PC34_CLOSE_ROTATE_PICKUP_PANEL_M569;
    out->queuedZone = DM1_PC34_CLOSE_ROTATE_PICKUP_PICKED_ZONE;
    out->queuedSlotBox = DM1_PC34_CLOSE_ROTATE_PICKUP_SLOT_BOX_C38;
    out->queuedPc34Slot = DM1_PC34_SLOT_CHEST_1;
    (void)queue_c537_pickup(&runtime);
    out->stepTrace[out->stepCount++] =
        DM1_PC34_CLOSE_ROTATE_PICKUP_STEP_QUEUE_C537;
    out->queuedAgainstChampion = runtime.queuedChampion;
    out->queuedAgainstOpenChestThing = runtime.queuedOpenChestThing;
    out->queuedThingType = runtime.pendingPickup.itemType;
    out->queuedThingCharges = runtime.pendingPickup.charges;
    out->queuedThingQuantity = runtime.pendingPickup.quantity;
    out->queuedThingWeight = runtime.pendingPickup.weight;
    out->queuedThingAllowedSlots = runtime.pendingPickup.allowedSlots;

    out->pickupBeginResult = begin_c537_pickup(&runtime);
    out->stepTrace[out->stepCount++] =
        DM1_PC34_CLOSE_ROTATE_PICKUP_STEP_BEGIN_PICKUP;
    out->pickedSlotEmptyBeforeClose =
        !m11_inventory_get_item_in_chest_slot(
            &runtime.inventory, runtime.oldLeaderIndex,
            DM1_PC34_CLOSE_ROTATE_PICKUP_PICKED_SLOT_INDEX, &slot) ||
        slot.itemType == 0;
    if (m11_inventory_get_mouse_item(
            &runtime.inventory, runtime.oldLeaderIndex, &hand)) {
        out->leaderHandTypeBeforeClose = hand.itemType;
        out->leaderHandChargesBeforeClose = hand.charges;
        out->leaderHandWeightBeforeClose = hand.weight;
    }
    out->leaderHandQuantityBeforeClose = runtime.leaderHandQuantity;

    out->closeCount = close_current_chest(&runtime, closed);
    out->stepTrace[out->stepCount++] =
        DM1_PC34_CLOSE_ROTATE_PICKUP_STEP_CLOSE_G0426;
    out->openChestThingAfterClose = m11_inventory_get_open_chest_thing(
        &runtime.inventory, runtime.oldLeaderIndex);
    for (i = 0; i < DM1_PC34_CLOSE_ROTATE_PICKUP_SLOT_COUNT; ++i) {
        out->closedTypes[i] = closed[i].itemType;
        out->closedCharges[i] = closed[i].charges;
        out->closedQuantities[i] = closed[i].quantity;
    }
    out->closeSkippedPickedSlot =
        count_type(closed, DM1_PC34_CLOSE_ROTATE_PICKUP_SLOT_COUNT,
                   pickedType) == 0;
    out->closeCompactedTail =
        out->closedTypes[0] == DM1_PC34_CLOSE_ROTATE_PICKUP_FIRST_ITEM + 1 &&
        out->closedTypes[DM1_PC34_CLOSE_ROTATE_PICKUP_SLOT_COUNT - 2] ==
            DM1_PC34_CLOSE_ROTATE_PICKUP_FIRST_ITEM + 7 &&
        out->closedTypes[DM1_PC34_CLOSE_ROTATE_PICKUP_SLOT_COUNT - 1] == 0;
    out->totalVisibleAfterClose = out->closeCount;

    runtime.currentLeaderIndex = runtime.newLeaderIndex;
    runtime.rotateInProgress = 0;
    out->stepTrace[out->stepCount++] =
        DM1_PC34_CLOSE_ROTATE_PICKUP_STEP_ROTATE_COMMIT;
    out->leaderAfterCommit = runtime.currentLeaderIndex;
    out->partyDirectionAfterCommit = runtime.partyDirection;
    out->oldLeaderDirectionAfterCommit =
        runtime.championDirection[runtime.oldLeaderIndex];
    out->newLeaderDirectionAfterCommit =
        runtime.championDirection[runtime.newLeaderIndex];
    out->newLeaderOpenChestThingAfterCommit = m11_inventory_get_open_chest_thing(
        &runtime.inventory, runtime.newLeaderIndex);

    out->stalePickupResultAfterClose =
        m11_inventory_click_open_chest_slot_for_thing(
            &runtime.inventory, runtime.newLeaderIndex,
            DM1_PC34_CLOSE_ROTATE_PICKUP_CHEST_THING,
            DM1_PC34_CLOSE_ROTATE_PICKUP_PICKED_SLOT_INDEX);
    out->stepTrace[out->stepCount++] =
        DM1_PC34_CLOSE_ROTATE_PICKUP_STEP_STALE_PICKUP_REJECT;
    out->latePickupRejectedAgainstClosedG0426 =
        out->stalePickupResultAfterClose == 0 &&
        out->newLeaderOpenChestThingAfterCommit == 0 &&
        out->openChestThingAfterClose == 0;
    out->pickedCopiesInClosedChain =
        count_type(closed, DM1_PC34_CLOSE_ROTATE_PICKUP_SLOT_COUNT,
                   pickedType);
    out->pickedCopiesIncludingHand =
        out->pickedCopiesInClosedChain +
        (out->leaderHandTypeBeforeClose == pickedType ? 1 : 0);

    hash_probe(&hash, out);
    out->deterministicHash = hash;
    return out->openResult && out->pickupBeginResult &&
           out->closeCount == DM1_PC34_CLOSE_ROTATE_PICKUP_SLOT_COUNT - 1 &&
           out->latePickupRejectedAgainstClosedG0426 &&
           out->pickedCopiesIncludingHand == 1;
}
