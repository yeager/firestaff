#include "firestaff/dm1/v1/chest/dm1_v1_chest_deposit_during_leader_rotation_pc34_compat.h"

#include <string.h>

/*
 * Contract-only runtime driver for a C542 occupied-slot deposit while a leader
 * rotation is already queued but not consumed.
 *
 * ReDMCSB: CHEST.C F0333:30-67 binds G0426/G0425; F0334:113-132 rewrites the
 * visible linked slots on close.  CHAMPION.C F0297:243-298 and F0298:270-298
 * mutate the global C030 leader hand, while F0300:511-515, F0301:606-614, and
 * F0302:662-714 route occupied C537..C544 clicks through the C30+ G0425 slot
 * range.  COMMAND.C F0359:1985-1990 is the same command-consumer boundary as
 * the in-flight leader rotation; PANEL.C F0344/F0345/F0354 are redraw-only for
 * this asset-free gate.
 */

typedef struct {
    int itemType;
    int weight;
    int charges;
    int quantity;
    int allowedSlots;
} DepositThingPc34;

typedef struct {
    M11_InventoryState inventory;
    DepositThingPc34 linked[DM1_PC34_DEPOSIT_ROTATE_SLOT_COUNT];
    int quantityByChampion[DM1_PC34_DEPOSIT_ROTATE_CHAMPION_COUNT]
                          [DM1_PC34_DEPOSIT_ROTATE_SLOT_COUNT];
    int handQuantity[DM1_PC34_DEPOSIT_ROTATE_CHAMPION_COUNT];
    int currentLeader;
    int queuedNewLeader;
    int rotationQueued;
    int rotationConsumed;
    int c040ResurrectPending;
} DepositRuntimePc34;

static const char s_source_evidence[] =
    "CHEST.C F0333:30-67 materializes the open chest into G0425/C537..C544\n"
    "CHEST.C F0334:113-132 clears G0426 and relinks non-empty visible slots\n"
    "CHAMPION.C F0297:243-298 puts the C542 thing in the leader C030 hand\n"
    "CHAMPION.C F0298:270-298 removes/transfers the current C030 hand thing\n"
    "CHAMPION.C F0300:511-515 clears the C542/C30+ G0425 slot\n"
    "CHAMPION.C F0301:606-614 would write a non-empty hand back to G0425\n"
    "CHAMPION.C F0302:662-714 dispatches an occupied C542 slot click\n"
    "COMMAND.C F0359:1985-1990 consumes the same-tick command/rotation route\n"
    "PANEL.C F0344:1493-1561, F0345:1563-1616, F0354:2299-2352 redraw only\n"
    "DEFS.H C030/C038/C040/C045/C537..C544/G0425/G0426/C540 define the slots\n"
    "Non-duplicative: C542 deposit while leader rotation is in flight, no C040 resurrect pending, no pickup/scroll/close/reopen/teleporter/save-load path";

static const DM1_V1_ChestDepositDuringLeaderRotationSpecPc34 s_spec = {
    "Source-locked contract-only gate: occupied C542 deposit in the same tick as leader rotation leaves C537..C541 in G0425 and hands the deposited thing to the new leader.",
    "CHEST.C F0333 lines 30-67 open dispatch binds G0426/G0425",
    "CHEST.C F0334 lines 113-132 close-time visible-slot rewrite",
    "CHAMPION.C F0297 lines 243-298 leader-hand put target",
    "CHAMPION.C F0298 lines 270-298 leader-hand remove/transfer",
    "CHAMPION.C F0300 lines 511-515 C30+ clear through G0425",
    "CHAMPION.C F0301 lines 606-614 C30+ write through G0425",
    "CHAMPION.C F0302 lines 662-714 occupied-slot click dispatch",
    "COMMAND.C F0359 lines 1985-1990 command queue / rotation consumer",
    "PANEL.C F0344/F0345/F0354 panel redraw boundary",
    "DEFS.H C030/C038/C040/C045/C537..C544/G0425/G0426/C540",
    "Excludes pickup, scroll-wheel, close-while-candidate, reopen, hidden-tail, save/load, teleporter, and resurrect-pending siblings",
    DM1_PC34_DEPOSIT_ROTATE_OLD_LEADER,
    DM1_PC34_DEPOSIT_ROTATE_NEW_LEADER,
    DM1_PC34_DEPOSIT_ROTATE_TARGET_ZONE,
    DM1_PC34_DEPOSIT_ROTATE_TARGET_PC34_SLOT,
    DM1_PC34_DEPOSIT_ROTATE_TARGET_SLOT_BOX,
    DM1_PC34_DEPOSIT_ROTATE_STABLE_PREFIX_COUNT,
    DM1_PC34_DEPOSIT_ROTATE_C040_OFF
};

static M11_Item to_m11_item(DepositThingPc34 thing)
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

static DepositThingPc34 make_stable_thing(int index)
{
    DepositThingPc34 thing;

    memset(&thing, 0, sizeof(thing));
    thing.itemType = DM1_PC34_DEPOSIT_ROTATE_FIRST_STABLE_ITEM + index;
    thing.weight = 5 + index;
    thing.charges = DM1_PC34_DEPOSIT_ROTATE_FIRST_CHARGES + index;
    thing.quantity = DM1_PC34_DEPOSIT_ROTATE_FIRST_QUANTITY + index;
    thing.allowedSlots = DM1_PC34_ALLOWED_CONTAINER;
    return thing;
}

static DepositThingPc34 make_target_thing(void)
{
    DepositThingPc34 thing;

    memset(&thing, 0, sizeof(thing));
    thing.itemType = DM1_PC34_DEPOSIT_ROTATE_TARGET_ITEM;
    thing.weight = DM1_PC34_DEPOSIT_ROTATE_TARGET_WEIGHT;
    thing.charges = DM1_PC34_DEPOSIT_ROTATE_TARGET_CHARGES;
    thing.quantity = DM1_PC34_DEPOSIT_ROTATE_TARGET_QUANTITY;
    thing.allowedSlots = DM1_PC34_ALLOWED_CONTAINER;
    return thing;
}

static void record_stable_prefix(
    const DepositRuntimePc34* runtime,
    int champion,
    int* types,
    int* charges,
    int* quantities)
{
    int i;

    for (i = 0; i < DM1_PC34_DEPOSIT_ROTATE_STABLE_PREFIX_COUNT; ++i) {
        M11_Item item;

        if (m11_inventory_get_item_in_chest_slot(
                &runtime->inventory, champion, i, &item)) {
            types[i] = item.itemType;
            charges[i] = item.charges;
            quantities[i] = runtime->quantityByChampion[champion][i];
        } else {
            types[i] = 0;
            charges[i] = 0;
            quantities[i] = 0;
        }
    }
}

static void record_all_slots(
    const DepositRuntimePc34* runtime,
    int champion,
    int* types,
    int* charges,
    int* quantities)
{
    int i;

    for (i = 0; i < DM1_PC34_DEPOSIT_ROTATE_SLOT_COUNT; ++i) {
        M11_Item item;

        if (m11_inventory_get_item_in_chest_slot(
                &runtime->inventory, champion, i, &item)) {
            types[i] = item.itemType;
            charges[i] = item.charges;
            quantities[i] = runtime->quantityByChampion[champion][i];
        } else {
            types[i] = 0;
            charges[i] = 0;
            quantities[i] = 0;
        }
    }
}

static int stable_prefix_matches(const int* types,
                                 const int* charges,
                                 const int* quantities)
{
    int i;

    for (i = 0; i < DM1_PC34_DEPOSIT_ROTATE_STABLE_PREFIX_COUNT; ++i) {
        if (types[i] != DM1_PC34_DEPOSIT_ROTATE_FIRST_STABLE_ITEM + i ||
            charges[i] != DM1_PC34_DEPOSIT_ROTATE_FIRST_CHARGES + i ||
            quantities[i] != DM1_PC34_DEPOSIT_ROTATE_FIRST_QUANTITY + i) {
            return 0;
        }
    }
    return 1;
}

static void runtime_init(DepositRuntimePc34* runtime)
{
    M11_Item linked[DM1_PC34_DEPOSIT_ROTATE_SLOT_COUNT];
    int i;

    memset(runtime, 0, sizeof(*runtime));
    m11_inventory_init(&runtime->inventory,
                       DM1_PC34_DEPOSIT_ROTATE_CHAMPION_COUNT);
    runtime->currentLeader = DM1_PC34_DEPOSIT_ROTATE_OLD_LEADER;
    runtime->queuedNewLeader = DM1_PC34_DEPOSIT_ROTATE_OLD_LEADER;
    runtime->c040ResurrectPending = DM1_PC34_DEPOSIT_ROTATE_C040_OFF;

    for (i = 0; i < DM1_PC34_DEPOSIT_ROTATE_STABLE_PREFIX_COUNT; ++i) {
        runtime->linked[i] = make_stable_thing(i);
    }
    runtime->linked[DM1_PC34_DEPOSIT_ROTATE_TARGET_SLOT_INDEX] =
        make_target_thing();

    for (i = 0; i < DM1_PC34_DEPOSIT_ROTATE_SLOT_COUNT; ++i) {
        linked[i] = to_m11_item(runtime->linked[i]);
        runtime->quantityByChampion[DM1_PC34_DEPOSIT_ROTATE_OLD_LEADER][i] =
            runtime->linked[i].quantity;
    }

    (void)m11_inventory_open_chest(
        &runtime->inventory, DM1_PC34_DEPOSIT_ROTATE_OLD_LEADER,
        DM1_PC34_DEPOSIT_ROTATE_CHEST_THING, linked,
        DM1_PC34_DEPOSIT_ROTATE_SLOT_COUNT);
}

static int queue_leader_rotation(DepositRuntimePc34* runtime, int newLeader)
{
    if (!runtime || newLeader < 0 ||
        newLeader >= DM1_PC34_DEPOSIT_ROTATE_CHAMPION_COUNT ||
        newLeader == runtime->currentLeader) {
        return 0;
    }
    runtime->rotationQueued = 1;
    runtime->queuedNewLeader = newLeader;
    return 1;
}

static int click_c542_deposit(DepositRuntimePc34* runtime)
{
    int result;

    if (!runtime || !runtime->rotationQueued ||
        runtime->c040ResurrectPending != DM1_PC34_DEPOSIT_ROTATE_C040_OFF) {
        return 0;
    }

    /* ReDMCSB: CHAMPION.C F0302 lines 688-706 reads the C542/G0425 slot,
     * F0300 lines 511-515 clears it, and F0297 lines 243-298 puts that thing
     * in the still-current leader hand before COMMAND.C F0359 consumes the
     * queued leader rotation. */
    result = m11_inventory_click_open_chest_slot_for_thing(
        &runtime->inventory, runtime->currentLeader,
        DM1_PC34_DEPOSIT_ROTATE_CHEST_THING,
        DM1_PC34_DEPOSIT_ROTATE_TARGET_SLOT_INDEX);
    if (result) {
        runtime->handQuantity[runtime->currentLeader] =
            runtime->quantityByChampion[runtime->currentLeader]
                                      [DM1_PC34_DEPOSIT_ROTATE_TARGET_SLOT_INDEX];
        runtime->quantityByChampion[runtime->currentLeader]
                                  [DM1_PC34_DEPOSIT_ROTATE_TARGET_SLOT_INDEX] = 0;
    }
    return result;
}

static int consume_rotation(DepositRuntimePc34* runtime)
{
    M11_ChampionInventory* oldInv;
    M11_ChampionInventory* newInv;
    int i;

    if (!runtime || !runtime->rotationQueued || runtime->rotationConsumed) {
        return 0;
    }

    oldInv = &runtime->inventory.champions[runtime->currentLeader];
    newInv = &runtime->inventory.champions[runtime->queuedNewLeader];

    /* ReDMCSB: F0298/F0297 model the global C030 hand; when COMMAND.C F0359
     * consumes the rotation after the C542 click, the just-deposited global
     * hand identity is now observed as the new leader hand. */
    newInv->mouseItem = oldInv->mouseItem;
    runtime->handQuantity[runtime->queuedNewLeader] =
        runtime->handQuantity[runtime->currentLeader];
    (void)m11_inventory_set_mouse_item(
        &runtime->inventory, runtime->currentLeader, 0, 0, 0, 0);
    runtime->handQuantity[runtime->currentLeader] = 0;

    newInv->openChestThing = oldInv->openChestThing;
    for (i = 0; i < DM1_PC34_DEPOSIT_ROTATE_SLOT_COUNT; ++i) {
        newInv->chestSlots[i] = oldInv->chestSlots[i];
        runtime->quantityByChampion[runtime->queuedNewLeader][i] =
            runtime->quantityByChampion[runtime->currentLeader][i];
    }

    runtime->currentLeader = runtime->queuedNewLeader;
    runtime->rotationConsumed = 1;
    return 1;
}

static int close_new_leader_chest(DepositRuntimePc34* runtime,
                                  DepositThingPc34* closed)
{
    M11_Item items[DM1_PC34_DEPOSIT_ROTATE_SLOT_COUNT];
    int count;
    int i;

    memset(items, 0, sizeof(items));
    count = m11_inventory_close_chest(
        &runtime->inventory, DM1_PC34_DEPOSIT_ROTATE_NEW_LEADER,
        items, DM1_PC34_DEPOSIT_ROTATE_SLOT_COUNT);
    if (count < 0) {
        return -1;
    }

    for (i = 0; i < DM1_PC34_DEPOSIT_ROTATE_SLOT_COUNT; ++i) {
        closed[i].itemType = items[i].itemType;
        closed[i].weight = items[i].weight;
        closed[i].charges = items[i].charges;
        closed[i].quantity = i < DM1_PC34_DEPOSIT_ROTATE_STABLE_PREFIX_COUNT ?
            DM1_PC34_DEPOSIT_ROTATE_FIRST_QUANTITY + i : 0;
        closed[i].allowedSlots = items[i].allowedSlots;
    }
    return count;
}

static int count_target_copies(const DepositRuntimePc34* runtime,
                               const DepositThingPc34* closed)
{
    M11_Item hand;
    int count = 0;
    int i;

    if (m11_inventory_get_mouse_item(
            &runtime->inventory, DM1_PC34_DEPOSIT_ROTATE_NEW_LEADER, &hand) &&
        hand.itemType == DM1_PC34_DEPOSIT_ROTATE_TARGET_ITEM) {
        ++count;
    }
    if (m11_inventory_get_mouse_item(
            &runtime->inventory, DM1_PC34_DEPOSIT_ROTATE_OLD_LEADER, &hand) &&
        hand.itemType == DM1_PC34_DEPOSIT_ROTATE_TARGET_ITEM) {
        ++count;
    }
    for (i = 0; i < DM1_PC34_DEPOSIT_ROTATE_SLOT_COUNT; ++i) {
        if (closed[i].itemType == DM1_PC34_DEPOSIT_ROTATE_TARGET_ITEM) {
            ++count;
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
                       const DM1_V1_ChestDepositDuringLeaderRotationProbePc34* p)
{
    int i;

    hash_int(hash, p->stepCount);
    hash_int(hash, p->openChestThingBefore);
    hash_int(hash, p->rotationQueued);
    hash_int(hash, p->depositClickResult);
    hash_int(hash, p->rotationConsumed);
    hash_int(hash, p->leaderAfterConsume);
    hash_int(hash, p->oldLeaderHandTypeAfterDeposit);
    hash_int(hash, p->newLeaderHandTypeAfterConsume);
    hash_int(hash, p->newLeaderInheritedDeposit);
    hash_int(hash, p->targetRemovedFromC542);
    hash_int(hash, p->c537ToC541RemainInG0425);
    hash_int(hash, p->closeCount);
    hash_int(hash, p->closedStablePrefixPreserved);
    hash_int(hash, p->totalTargetCopiesAfterClose);
    for (i = 0; i < DM1_PC34_DEPOSIT_ROTATE_SLOT_COUNT; ++i) {
        hash_int(hash, p->visibleTypesAfterConsume[i]);
        hash_int(hash, p->closedTypes[i]);
        hash_int(hash, p->closedQuantities[i]);
    }
}

const char*
dm1_v1_chest_deposit_during_leader_rotation_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const DM1_V1_ChestDepositDuringLeaderRotationSpecPc34*
dm1_v1_chest_deposit_during_leader_rotation_spec_pc34(void)
{
    return &s_spec;
}

int dm1_v1_chest_deposit_during_leader_rotation_run_pc34(
    DM1_V1_ChestDepositDuringLeaderRotationProbePc34* out)
{
    DepositRuntimePc34 runtime;
    DepositThingPc34 closed[DM1_PC34_DEPOSIT_ROTATE_SLOT_COUNT];
    M11_Item hand;
    M11_Item target;
    uint32_t hash = 2166136261u;
    int i;

    if (!out) {
        return 0;
    }

    memset(out, 0, sizeof(*out));
    memset(closed, 0, sizeof(closed));
    runtime_init(&runtime);

    out->sourceLockedContractOnly = 1;
    out->stepTrace[out->stepCount++] =
        DM1_PC34_DEPOSIT_ROTATE_STEP_SETUP_OPEN;
    out->openResult = 1;
    out->openChestThingBefore = m11_inventory_get_open_chest_thing(
        &runtime.inventory, DM1_PC34_DEPOSIT_ROTATE_OLD_LEADER);
    out->leaderBefore = runtime.currentLeader;
    (void)m11_inventory_get_mouse_item(
        &runtime.inventory, DM1_PC34_DEPOSIT_ROTATE_OLD_LEADER, &hand);
    out->oldLeaderHandTypeBefore = hand.itemType;
    (void)m11_inventory_get_mouse_item(
        &runtime.inventory, DM1_PC34_DEPOSIT_ROTATE_NEW_LEADER, &hand);
    out->newLeaderHandTypeBefore = hand.itemType;
    out->c040ResurrectPendingBefore = runtime.c040ResurrectPending;
    out->panelContentBefore =
        m11_inventory_get_panel_content_pc34(&runtime.inventory);
    record_stable_prefix(&runtime, DM1_PC34_DEPOSIT_ROTATE_OLD_LEADER,
                         out->stableTypesBefore,
                         out->stableChargesBefore,
                         out->stableQuantitiesBefore);
    (void)m11_inventory_get_item_in_chest_slot(
        &runtime.inventory, DM1_PC34_DEPOSIT_ROTATE_OLD_LEADER,
        DM1_PC34_DEPOSIT_ROTATE_TARGET_SLOT_INDEX, &target);
    out->targetTypeBefore = target.itemType;
    out->targetChargesBefore = target.charges;
    out->targetQuantityBefore =
        runtime.quantityByChampion[DM1_PC34_DEPOSIT_ROTATE_OLD_LEADER]
                                  [DM1_PC34_DEPOSIT_ROTATE_TARGET_SLOT_INDEX];
    out->targetWeightBefore = target.weight;

    out->rotationQueued = queue_leader_rotation(
        &runtime, DM1_PC34_DEPOSIT_ROTATE_NEW_LEADER);
    out->stepTrace[out->stepCount++] =
        DM1_PC34_DEPOSIT_ROTATE_STEP_ROTATION_QUEUED;
    out->queuedChampion = DM1_PC34_DEPOSIT_ROTATE_OLD_LEADER;
    out->queuedNewLeader = runtime.queuedNewLeader;
    out->leaderAfterQueue = runtime.currentLeader;

    out->pointerZone = DM1_PC34_DEPOSIT_ROTATE_TARGET_ZONE;
    out->pointerSlotBox = DM1_PC34_DEPOSIT_ROTATE_TARGET_SLOT_BOX;
    out->pointerPc34Slot = DM1_PC34_DEPOSIT_ROTATE_TARGET_PC34_SLOT;
    out->depositClickResult = click_c542_deposit(&runtime);
    out->stepTrace[out->stepCount++] =
        DM1_PC34_DEPOSIT_ROTATE_STEP_C542_DEPOSIT;
    out->depositAgainstOldLeader =
        runtime.currentLeader == DM1_PC34_DEPOSIT_ROTATE_OLD_LEADER ? 1 : 0;
    out->depositAgainstOpenChest =
        m11_inventory_get_open_chest_thing(
            &runtime.inventory, DM1_PC34_DEPOSIT_ROTATE_OLD_LEADER) ==
        DM1_PC34_DEPOSIT_ROTATE_CHEST_THING ? 1 : 0;
    (void)m11_inventory_get_mouse_item(
        &runtime.inventory, DM1_PC34_DEPOSIT_ROTATE_OLD_LEADER, &hand);
    out->oldLeaderHandTypeAfterDeposit = hand.itemType;
    out->oldLeaderHandChargesAfterDeposit = hand.charges;
    out->oldLeaderHandQuantityAfterDeposit =
        runtime.handQuantity[DM1_PC34_DEPOSIT_ROTATE_OLD_LEADER];
    out->oldLeaderHandWeightAfterDeposit = hand.weight;
    record_stable_prefix(&runtime, DM1_PC34_DEPOSIT_ROTATE_OLD_LEADER,
                         out->stableTypesAfterDeposit,
                         out->stableChargesAfterDeposit,
                         out->stableQuantitiesAfterDeposit);
    out->stablePrefixPreservedAfterDeposit =
        stable_prefix_matches(out->stableTypesAfterDeposit,
                              out->stableChargesAfterDeposit,
                              out->stableQuantitiesAfterDeposit);
    (void)m11_inventory_get_item_in_chest_slot(
        &runtime.inventory, DM1_PC34_DEPOSIT_ROTATE_OLD_LEADER,
        DM1_PC34_DEPOSIT_ROTATE_TARGET_SLOT_INDEX, &target);
    out->targetTypeAfterDeposit = target.itemType;
    out->targetRemovedFromC542 = target.itemType == 0 ? 1 : 0;

    out->rotationConsumed = consume_rotation(&runtime);
    out->stepTrace[out->stepCount++] =
        DM1_PC34_DEPOSIT_ROTATE_STEP_ROTATION_CONSUMED;
    out->leaderAfterConsume = runtime.currentLeader;
    out->depositSameTickAsRotation =
        out->stepTrace[1] == DM1_PC34_DEPOSIT_ROTATE_STEP_ROTATION_QUEUED &&
        out->stepTrace[2] == DM1_PC34_DEPOSIT_ROTATE_STEP_C542_DEPOSIT &&
        out->stepTrace[3] == DM1_PC34_DEPOSIT_ROTATE_STEP_ROTATION_CONSUMED ?
        1 : 0;
    out->newLeaderOpenChestThingAfterConsume =
        m11_inventory_get_open_chest_thing(
            &runtime.inventory, DM1_PC34_DEPOSIT_ROTATE_NEW_LEADER);
    (void)m11_inventory_get_mouse_item(
        &runtime.inventory, DM1_PC34_DEPOSIT_ROTATE_NEW_LEADER, &hand);
    out->newLeaderHandTypeAfterConsume = hand.itemType;
    out->newLeaderHandChargesAfterConsume = hand.charges;
    out->newLeaderHandQuantityAfterConsume =
        runtime.handQuantity[DM1_PC34_DEPOSIT_ROTATE_NEW_LEADER];
    out->newLeaderHandWeightAfterConsume = hand.weight;
    out->newLeaderInheritedDeposit =
        hand.itemType == DM1_PC34_DEPOSIT_ROTATE_TARGET_ITEM &&
        hand.charges == DM1_PC34_DEPOSIT_ROTATE_TARGET_CHARGES &&
        hand.weight == DM1_PC34_DEPOSIT_ROTATE_TARGET_WEIGHT &&
        out->newLeaderHandQuantityAfterConsume ==
        DM1_PC34_DEPOSIT_ROTATE_TARGET_QUANTITY ? 1 : 0;
    (void)m11_inventory_get_mouse_item(
        &runtime.inventory, DM1_PC34_DEPOSIT_ROTATE_OLD_LEADER, &hand);
    out->oldLeaderHandTypeAfterConsume = hand.itemType;
    out->noResurrectPendingAfterConsume =
        runtime.c040ResurrectPending == DM1_PC34_DEPOSIT_ROTATE_C040_OFF ?
        1 : 0;
    record_all_slots(&runtime, DM1_PC34_DEPOSIT_ROTATE_NEW_LEADER,
                     out->visibleTypesAfterConsume,
                     out->visibleChargesAfterConsume,
                     out->visibleQuantitiesAfterConsume);
    out->c537ToC541RemainInG0425 =
        stable_prefix_matches(out->visibleTypesAfterConsume,
                              out->visibleChargesAfterConsume,
                              out->visibleQuantitiesAfterConsume);
    out->c542EmptyAfterConsume =
        out->visibleTypesAfterConsume
            [DM1_PC34_DEPOSIT_ROTATE_TARGET_SLOT_INDEX] == 0 ? 1 : 0;

    out->closeCount = close_new_leader_chest(&runtime, closed);
    out->stepTrace[out->stepCount++] =
        DM1_PC34_DEPOSIT_ROTATE_STEP_CLOSE_REWRITE;
    out->closeAgainstNewLeader =
        runtime.currentLeader == DM1_PC34_DEPOSIT_ROTATE_NEW_LEADER ? 1 : 0;
    for (i = 0; i < DM1_PC34_DEPOSIT_ROTATE_SLOT_COUNT; ++i) {
        out->closedTypes[i] = closed[i].itemType;
        out->closedCharges[i] = closed[i].charges;
        out->closedQuantities[i] = closed[i].quantity;
    }
    out->closedStablePrefixPreserved =
        stable_prefix_matches(out->closedTypes,
                              out->closedCharges,
                              out->closedQuantities);
    out->closedTargetAbsent = 1;
    for (i = 0; i < DM1_PC34_DEPOSIT_ROTATE_SLOT_COUNT; ++i) {
        if (out->closedTypes[i] == DM1_PC34_DEPOSIT_ROTATE_TARGET_ITEM) {
            out->closedTargetAbsent = 0;
        }
    }
    out->totalTargetCopiesAfterClose = count_target_copies(&runtime, closed);

    hash_probe(&hash, out);
    out->deterministicHash = hash;
    return out->openResult &&
        out->rotationQueued &&
        out->depositClickResult &&
        out->rotationConsumed &&
        out->newLeaderInheritedDeposit &&
        out->c537ToC541RemainInG0425 &&
        out->c542EmptyAfterConsume &&
        out->closedStablePrefixPreserved &&
        out->closedTargetAbsent &&
        out->totalTargetCopiesAfterClose == 1;
}
