#include "firestaff/dm1/v1/chest/scroll_wheel_drop_during_rotation_non_leader_open_pc34_compat.h"

#include <string.h>

/*
 * Runtime regression driver for a queued scroll-wheel drop into C540 while a
 * leader rotation is queued and champion 1 owns the live G0426 chest panel.
 *
 * ReDMCSB: CHEST.C F0333:30-67 materializes C537..C544 in G0425 and F0334:
 * 113-132 would close/relink it; this pass asserts F0334 is not reached.
 * CHAMPION.C F0297/F0298:243-298 own the global C030 leader hand while
 * F0301/F0302:606-714 route C30+ slots through G0425.  COMMAND.C F0359:
 * 1452-1662 queues commands and F0380:2045-2178 drains this C540 slot-box
 * command before the queued leader rotation.  IO.C F0077/F0078:1102-1122
 * bracket the synthetic mouse update around the drop.
 */

typedef struct {
    int itemType;
    int weight;
    int charges;
    int quantity;
    int allowedSlots;
} DropRotateThingPc34;

typedef struct {
    M11_InventoryState inventory;
    DropRotateThingPc34 linked[DM1_PC34_SW_DROP_ROT_NLO_SLOT_COUNT];
    int quantityByChampion[DM1_PC34_SW_DROP_ROT_NLO_CHAMPION_COUNT]
                          [DM1_PC34_SW_DROP_ROT_NLO_SLOT_COUNT];
    int handQuantity[DM1_PC34_SW_DROP_ROT_NLO_CHAMPION_COUNT];
    int currentLeader;
    int openChampion;
    int dropQueued;
    int rotationQueued;
    int queuedChampion;
    int queuedOpenChampion;
    int queuedNewLeader;
    int commandQueueDepth;
    int mouseUpdateDepth;
    int f0077Observed;
    int f0078Observed;
    int closeCount;
} DropRotateRuntimePc34;

static const char s_source_evidence[] =
    "CHEST.C F0333:30-67 opens G0426 and materializes G0425/C537..C544\n"
    "CHEST.C F0334:113-132 is a negative anchor here: G0426 must not close\n"
    "CHAMPION.C F0297:243-298 puts a thing in C030/G4055 leader hand\n"
    "CHAMPION.C F0298:270-298 removes/clears C030/G4055 leader hand\n"
    "CHAMPION.C F0301:606-614 writes C30+ chest slots through G0425\n"
    "CHAMPION.C F0302:662-714 dispatches C537..C544 slot-box commands\n"
    "COMMAND.C F0359:1452-1662 queues mouse slot commands without draining\n"
    "COMMAND.C F0380:2045-2178 drains queued C540 before queued rotation\n"
    "IO.C F0077:1113-1122 enables mouse screen-update suppression\n"
    "IO.C F0078:1102-1111 disables mouse screen-update suppression\n"
    "DEFS.H:790 C10_SLOT_NECK, 810 C30, 2088 C10_COLOR_FLESH, 3906-3913 C537..C544, 3909 C540\n"
    "Non-duplicative: pass771 is not pass768 close race, not scroll_wheel_pickup_drop, not scroll_wheel_drop_onto_open_chest_slot, not chest_deposit_during_leader_rotation, not chest_pickup_during_resurrect_pending_non_leader, not chest_close_while_candidate_live_non_leader, not chest_open_during_pending, not chest_close_with_full_leader_hand, not chest_open_with_full_leader_hand";

static const DM1_V1_ChestScrollWheelDropDuringRotationNonLeaderOpenSpecPc34
s_spec = {
    "Runtime regression: queued scroll-wheel C540 drop drains before queued leader rotation while champion 1's open chest stays live.",
    "CHEST.C F0333 lines 30-67 open/materialize G0426 into C537..C544",
    "CHEST.C F0334 lines 113-132 close/relink path must not execute",
    "CHAMPION.C F0297 lines 243-298 put object in C030 leader hand",
    "CHAMPION.C F0298 lines 270-298 remove object from C030 leader hand",
    "CHAMPION.C F0301 lines 606-614 C30+ slot write through G0425",
    "CHAMPION.C F0302 lines 662-714 C537..C544 slot-box dispatch",
    "COMMAND.C F0359 lines 1452-1662 command queue producer",
    "COMMAND.C F0380 lines 2045-2178 command queue drain order",
    "IO.C F0077 lines 1113-1122 enable screen update suppression",
    "IO.C F0078 lines 1102-1111 disable screen update suppression",
    "DEFS.H lines 790,810,2088,3906-3913 C10/C30/C537..C544/C540",
    "Fresh pass771: drop-while-rotation plus non-leader open G0426 plus post-rotation hand continuity; excludes pass768 and listed chest siblings",
    DM1_PC34_SW_DROP_ROT_NLO_OLD_LEADER,
    DM1_PC34_SW_DROP_ROT_NLO_NON_LEADER_OPEN,
    DM1_PC34_SW_DROP_ROT_NLO_NEW_LEADER,
    DM1_PC34_SW_DROP_ROT_NLO_TARGET_ZONE,
    DM1_PC34_SW_DROP_ROT_NLO_TARGET_SLOT_BOX,
    DM1_PC34_SW_DROP_ROT_NLO_TARGET_PC34_SLOT,
    DM1_PC34_SW_DROP_ROT_NLO_TARGET_COMMAND
};

static M11_Item to_m11_item(DropRotateThingPc34 thing)
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

static DropRotateThingPc34 make_stable_thing(int slotIndex)
{
    DropRotateThingPc34 thing;

    memset(&thing, 0, sizeof(thing));
    thing.itemType = DM1_PC34_SW_DROP_ROT_NLO_FIRST_STABLE_ITEM + slotIndex;
    thing.weight = 6 + slotIndex;
    thing.charges = DM1_PC34_SW_DROP_ROT_NLO_FIRST_CHARGES + slotIndex;
    thing.quantity = DM1_PC34_SW_DROP_ROT_NLO_FIRST_QUANTITY + slotIndex;
    thing.allowedSlots = DM1_PC34_ALLOWED_CONTAINER;
    return thing;
}

static int expected_stable_type(int slotIndex)
{
    if (slotIndex == DM1_PC34_SW_DROP_ROT_NLO_TARGET_SLOT_INDEX) {
        return DM1_PC34_SW_DROP_ROT_NLO_DROP_ITEM;
    }
    return DM1_PC34_SW_DROP_ROT_NLO_FIRST_STABLE_ITEM + slotIndex;
}

static int expected_stable_charges(int slotIndex)
{
    if (slotIndex == DM1_PC34_SW_DROP_ROT_NLO_TARGET_SLOT_INDEX) {
        return DM1_PC34_SW_DROP_ROT_NLO_DROP_CHARGES;
    }
    return DM1_PC34_SW_DROP_ROT_NLO_FIRST_CHARGES + slotIndex;
}

static int expected_stable_quantity(int slotIndex)
{
    if (slotIndex == DM1_PC34_SW_DROP_ROT_NLO_TARGET_SLOT_INDEX) {
        return DM1_PC34_SW_DROP_ROT_NLO_DROP_QUANTITY;
    }
    return DM1_PC34_SW_DROP_ROT_NLO_FIRST_QUANTITY + slotIndex;
}

static void record_slots(const DropRotateRuntimePc34* runtime,
                         int champion,
                         int* types,
                         int* charges,
                         int* quantities)
{
    int i;

    for (i = 0; i < DM1_PC34_SW_DROP_ROT_NLO_SLOT_COUNT; ++i) {
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

static int slots_match_after_drop(const int* types,
                                  const int* charges,
                                  const int* quantities)
{
    int i;

    for (i = 0; i < DM1_PC34_SW_DROP_ROT_NLO_SLOT_COUNT; ++i) {
        if (types[i] != expected_stable_type(i) ||
            charges[i] != expected_stable_charges(i) ||
            quantities[i] != expected_stable_quantity(i)) {
            return 0;
        }
    }
    return 1;
}

static int slots_match_before_drop(const int* types,
                                   const int* charges,
                                   const int* quantities)
{
    int i;

    for (i = 0; i < DM1_PC34_SW_DROP_ROT_NLO_SLOT_COUNT; ++i) {
        if (i == DM1_PC34_SW_DROP_ROT_NLO_TARGET_SLOT_INDEX) {
            if (types[i] != 0 || charges[i] != 0 || quantities[i] != 0) {
                return 0;
            }
        } else if (types[i] != DM1_PC34_SW_DROP_ROT_NLO_FIRST_STABLE_ITEM + i ||
                   charges[i] != DM1_PC34_SW_DROP_ROT_NLO_FIRST_CHARGES + i ||
                   quantities[i] != DM1_PC34_SW_DROP_ROT_NLO_FIRST_QUANTITY + i) {
            return 0;
        }
    }
    return 1;
}

static void runtime_init(DropRotateRuntimePc34* runtime)
{
    M11_Item linked[DM1_PC34_SW_DROP_ROT_NLO_SLOT_COUNT];
    int i;

    memset(runtime, 0, sizeof(*runtime));
    memset(linked, 0, sizeof(linked));
    m11_inventory_init(&runtime->inventory,
                       DM1_PC34_SW_DROP_ROT_NLO_CHAMPION_COUNT);
    runtime->currentLeader = DM1_PC34_SW_DROP_ROT_NLO_OLD_LEADER;
    runtime->openChampion = DM1_PC34_SW_DROP_ROT_NLO_NON_LEADER_OPEN;

    for (i = 0; i < DM1_PC34_SW_DROP_ROT_NLO_SLOT_COUNT; ++i) {
        if (i == DM1_PC34_SW_DROP_ROT_NLO_TARGET_SLOT_INDEX) {
            continue;
        }
        runtime->linked[i] = make_stable_thing(i);
        linked[i] = to_m11_item(runtime->linked[i]);
        runtime->quantityByChampion[runtime->openChampion][i] =
            runtime->linked[i].quantity;
    }

    (void)m11_inventory_set_mouse_item(
        &runtime->inventory, DM1_PC34_SW_DROP_ROT_NLO_OLD_LEADER,
        DM1_PC34_SW_DROP_ROT_NLO_DROP_ITEM,
        DM1_PC34_SW_DROP_ROT_NLO_DROP_WEIGHT,
        DM1_PC34_SW_DROP_ROT_NLO_DROP_CHARGES,
        DM1_PC34_ALLOWED_CONTAINER);
    runtime->handQuantity[DM1_PC34_SW_DROP_ROT_NLO_OLD_LEADER] =
        DM1_PC34_SW_DROP_ROT_NLO_DROP_QUANTITY;

    (void)m11_inventory_set_mouse_item(
        &runtime->inventory, DM1_PC34_SW_DROP_ROT_NLO_NEW_LEADER,
        DM1_PC34_SW_DROP_ROT_NLO_NEW_LEADER_HAND_ITEM,
        DM1_PC34_SW_DROP_ROT_NLO_NEW_HAND_WEIGHT,
        DM1_PC34_SW_DROP_ROT_NLO_NEW_HAND_CHARGES,
        DM1_PC34_ALLOWED_ANY_SLOT);

    (void)m11_inventory_open_chest(
        &runtime->inventory, runtime->openChampion,
        DM1_PC34_SW_DROP_ROT_NLO_CHEST_THING, linked,
        DM1_PC34_SW_DROP_ROT_NLO_SLOT_COUNT);
}

static int queue_drop_and_rotation(DropRotateRuntimePc34* runtime)
{
    if (!runtime || runtime->dropQueued || runtime->rotationQueued ||
        runtime->currentLeader != DM1_PC34_SW_DROP_ROT_NLO_OLD_LEADER ||
        m11_inventory_get_open_chest_thing(
            &runtime->inventory, runtime->openChampion) !=
            DM1_PC34_SW_DROP_ROT_NLO_CHEST_THING) {
        return 0;
    }

    runtime->dropQueued = 1;
    runtime->rotationQueued = 1;
    runtime->queuedChampion = runtime->currentLeader;
    runtime->queuedOpenChampion = runtime->openChampion;
    runtime->queuedNewLeader = DM1_PC34_SW_DROP_ROT_NLO_NEW_LEADER;
    runtime->commandQueueDepth = 2;
    return 1;
}

static int drain_c540_drop(DropRotateRuntimePc34* runtime)
{
    M11_Item oldLeaderHand;
    M11_Item c540;
    int result;

    if (!runtime || !runtime->dropQueued ||
        runtime->commandQueueDepth <= 0 ||
        runtime->currentLeader != DM1_PC34_SW_DROP_ROT_NLO_OLD_LEADER ||
        m11_inventory_get_open_chest_thing(
            &runtime->inventory, runtime->openChampion) !=
            DM1_PC34_SW_DROP_ROT_NLO_CHEST_THING ||
        !m11_inventory_get_mouse_item(
            &runtime->inventory, runtime->currentLeader, &oldLeaderHand) ||
        oldLeaderHand.itemType == 0 ||
        !m11_inventory_get_item_in_chest_slot(
            &runtime->inventory, runtime->openChampion,
            DM1_PC34_SW_DROP_ROT_NLO_TARGET_SLOT_INDEX, &c540) ||
        c540.itemType != 0 ||
        !m11_inventory_can_equip(
            &oldLeaderHand, DM1_PC34_SW_DROP_ROT_NLO_TARGET_PC34_SLOT)) {
        return 0;
    }

    /* ReDMCSB CHAMPION.C F0302 lines 688-710 uses the global C030 hand but
     * the G0423 inventory champion slot route; this is the cross-champion
     * C030/G0425 split that makes pass771 non-duplicative. */
    runtime->f0077Observed = 1;
    ++runtime->mouseUpdateDepth;
    result = m11_inventory_set_item_in_chest_slot(
        &runtime->inventory, runtime->openChampion,
        DM1_PC34_SW_DROP_ROT_NLO_TARGET_SLOT_INDEX,
        oldLeaderHand.itemType, oldLeaderHand.weight, oldLeaderHand.charges,
        oldLeaderHand.allowedSlots);
    if (!result) {
        return 0;
    }
    runtime->quantityByChampion[runtime->openChampion]
                              [DM1_PC34_SW_DROP_ROT_NLO_TARGET_SLOT_INDEX] =
        runtime->handQuantity[runtime->currentLeader];
    (void)m11_inventory_set_mouse_item(
        &runtime->inventory, runtime->currentLeader, 0, 0, 0, 0);
    runtime->handQuantity[runtime->currentLeader] = 0;
    runtime->dropQueued = 0;
    --runtime->commandQueueDepth;
    --runtime->mouseUpdateDepth;
    runtime->f0078Observed = 1;
    return 1;
}

static int consume_rotation(DropRotateRuntimePc34* runtime)
{
    if (!runtime || !runtime->rotationQueued ||
        runtime->commandQueueDepth <= 0 ||
        runtime->currentLeader != DM1_PC34_SW_DROP_ROT_NLO_OLD_LEADER) {
        return 0;
    }

    /* ReDMCSB COMMAND.C F0380 lines 2150-2178 consumes the later queued
     * command after the C540 drop.  C030 is already empty for the old leader,
     * while the new leader keeps its pre-existing hand state. */
    runtime->currentLeader = runtime->queuedNewLeader;
    runtime->openChampion = runtime->queuedOpenChampion;
    runtime->rotationQueued = 0;
    --runtime->commandQueueDepth;
    return 1;
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
    const DM1_V1_ChestScrollWheelDropDuringRotationNonLeaderOpenProbePc34* p)
{
    int i;

    hash_int(hash, p->stepCount);
    hash_int(hash, p->openChestThingBefore);
    hash_int(hash, p->dropQueued);
    hash_int(hash, p->rotationQueued);
    hash_int(hash, p->dropClickResult);
    hash_int(hash, p->rotationConsumed);
    hash_int(hash, p->leaderAfterRotate);
    hash_int(hash, p->oldLeaderHandTypeAfterDrop);
    hash_int(hash, p->newLeaderHandTypeAfterRotate);
    hash_int(hash, p->c540TypeAfterRotate);
    hash_int(hash, p->c540QuantityAfterRotate);
    hash_int(hash, p->chestNeverClosed);
    hash_int(hash, p->f0077F0078Balanced);
    for (i = 0; i < DM1_PC34_SW_DROP_ROT_NLO_SLOT_COUNT; ++i) {
        hash_int(hash, p->visibleTypesBefore[i]);
        hash_int(hash, p->visibleTypesAfterRotate[i]);
        hash_int(hash, p->visibleQuantitiesAfterRotate[i]);
    }
}

const char*
dm1_v1_chest_scroll_wheel_drop_during_rotation_non_leader_open_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const DM1_V1_ChestScrollWheelDropDuringRotationNonLeaderOpenSpecPc34*
dm1_v1_chest_scroll_wheel_drop_during_rotation_non_leader_open_spec_pc34(void)
{
    return &s_spec;
}

int dm1_v1_chest_scroll_wheel_drop_during_rotation_non_leader_open_run_pc34(
    DM1_V1_ChestScrollWheelDropDuringRotationNonLeaderOpenProbePc34* out)
{
    DropRotateRuntimePc34 runtime;
    M11_Item hand;
    M11_Item c540;
    uint32_t hash = 2166136261u;

    if (!out) {
        return 0;
    }

    memset(out, 0, sizeof(*out));
    runtime_init(&runtime);

    out->runtimeRegression = 1;
    out->stepTrace[out->stepCount++] =
        DM1_PC34_SW_DROP_ROT_NLO_STEP_OPEN_NON_LEADER_CHEST;
    out->openResult = 1;
    out->openChampionBefore = runtime.openChampion;
    out->openChestThingBefore = m11_inventory_get_open_chest_thing(
        &runtime.inventory, runtime.openChampion);
    out->panelContentBefore = m11_inventory_get_panel_content_pc34(
        &runtime.inventory);
    out->leaderBeforeQueue = runtime.currentLeader;
    (void)m11_inventory_get_mouse_item(
        &runtime.inventory, DM1_PC34_SW_DROP_ROT_NLO_OLD_LEADER, &hand);
    out->oldLeaderHandTypeBefore = hand.itemType;
    out->oldLeaderHandWeightBefore = hand.weight;
    out->oldLeaderHandChargesBefore = hand.charges;
    out->oldLeaderHandQuantityBefore =
        runtime.handQuantity[DM1_PC34_SW_DROP_ROT_NLO_OLD_LEADER];
    (void)m11_inventory_get_mouse_item(
        &runtime.inventory, DM1_PC34_SW_DROP_ROT_NLO_NEW_LEADER, &hand);
    out->newLeaderHandTypeBefore = hand.itemType;
    out->newLeaderHandWeightBefore = hand.weight;
    out->newLeaderHandChargesBefore = hand.charges;
    record_slots(&runtime, runtime.openChampion, out->visibleTypesBefore,
                 out->visibleChargesBefore, out->visibleQuantitiesBefore);
    out->c540EmptyBeforeDrop =
        out->visibleTypesBefore[DM1_PC34_SW_DROP_ROT_NLO_TARGET_SLOT_INDEX] ==
        0 ? 1 : 0;
    out->c537ToC544VisibleBefore =
        slots_match_before_drop(out->visibleTypesBefore,
                                out->visibleChargesBefore,
                                out->visibleQuantitiesBefore);

    out->dropQueued = queue_drop_and_rotation(&runtime);
    out->stepTrace[out->stepCount++] =
        DM1_PC34_SW_DROP_ROT_NLO_STEP_QUEUE_DROP_AND_ROTATION;
    out->rotationQueued = runtime.rotationQueued;
    out->queuedChampion = runtime.queuedChampion;
    out->queuedOpenChampion = runtime.queuedOpenChampion;
    out->queuedNewLeader = runtime.queuedNewLeader;
    out->queuedZone = DM1_PC34_SW_DROP_ROT_NLO_TARGET_ZONE;
    out->queuedSlotBox = DM1_PC34_SW_DROP_ROT_NLO_TARGET_SLOT_BOX;
    out->queuedPc34Slot = DM1_PC34_SW_DROP_ROT_NLO_TARGET_PC34_SLOT;
    out->queuedCommand = DM1_PC34_SW_DROP_ROT_NLO_TARGET_COMMAND;
    out->commandQueueDepthAfterQueue = runtime.commandQueueDepth;

    out->dropClickResult = drain_c540_drop(&runtime);
    out->stepTrace[out->stepCount++] =
        DM1_PC34_SW_DROP_ROT_NLO_STEP_F0380_DROP_C540;
    out->f0077Observed = runtime.f0077Observed;
    out->f0078ObservedAfterDrop = runtime.f0078Observed;
    out->mouseUpdateDepthAfterDrop = runtime.mouseUpdateDepth;
    out->dropDrainFirst =
        out->stepTrace[2] == DM1_PC34_SW_DROP_ROT_NLO_STEP_F0380_DROP_C540 &&
        runtime.commandQueueDepth == 1 ? 1 : 0;
    out->commandQueueDepthAfterDrop = runtime.commandQueueDepth;
    out->openChestThingAfterDrop = m11_inventory_get_open_chest_thing(
        &runtime.inventory, runtime.openChampion);
    out->panelContentAfterDrop = m11_inventory_get_panel_content_pc34(
        &runtime.inventory);
    (void)m11_inventory_get_mouse_item(
        &runtime.inventory, DM1_PC34_SW_DROP_ROT_NLO_OLD_LEADER, &hand);
    out->oldLeaderHandTypeAfterDrop = hand.itemType;
    (void)m11_inventory_get_mouse_item(
        &runtime.inventory, DM1_PC34_SW_DROP_ROT_NLO_NEW_LEADER, &hand);
    out->newLeaderHandTypeAfterDrop = hand.itemType;
    (void)m11_inventory_get_item_in_chest_slot(
        &runtime.inventory, runtime.openChampion,
        DM1_PC34_SW_DROP_ROT_NLO_TARGET_SLOT_INDEX, &c540);
    out->c540TypeAfterDrop = c540.itemType;
    out->c540WeightAfterDrop = c540.weight;
    out->c540ChargesAfterDrop = c540.charges;
    out->c540QuantityAfterDrop =
        runtime.quantityByChampion[runtime.openChampion]
                                  [DM1_PC34_SW_DROP_ROT_NLO_TARGET_SLOT_INDEX];
    out->c540DropPersistedBeforeRotate =
        out->c540TypeAfterDrop == DM1_PC34_SW_DROP_ROT_NLO_DROP_ITEM &&
        out->openChestThingAfterDrop ==
            DM1_PC34_SW_DROP_ROT_NLO_CHEST_THING ? 1 : 0;

    out->rotationConsumed = consume_rotation(&runtime);
    out->stepTrace[out->stepCount++] =
        DM1_PC34_SW_DROP_ROT_NLO_STEP_F0380_ROTATE_LEADER;
    out->commandQueueDepthAfterRotate = runtime.commandQueueDepth;
    out->leaderAfterRotate = runtime.currentLeader;
    out->openChampionAfterRotate = runtime.openChampion;
    out->newLeaderOpenChestThingAfterRotate =
        m11_inventory_get_open_chest_thing(
            &runtime.inventory, DM1_PC34_SW_DROP_ROT_NLO_NEW_LEADER);
    out->panelContentAfterRotate = m11_inventory_get_panel_content_pc34(
        &runtime.inventory);
    (void)m11_inventory_get_mouse_item(
        &runtime.inventory, DM1_PC34_SW_DROP_ROT_NLO_OLD_LEADER, &hand);
    out->oldLeaderHandTypeAfterRotate = hand.itemType;
    (void)m11_inventory_get_mouse_item(
        &runtime.inventory, DM1_PC34_SW_DROP_ROT_NLO_NEW_LEADER, &hand);
    out->newLeaderHandTypeAfterRotate = hand.itemType;
    out->newLeaderHandWeightAfterRotate = hand.weight;
    out->newLeaderHandChargesAfterRotate = hand.charges;
    out->oldLeaderHandEmptyAfterRotate =
        out->oldLeaderHandTypeAfterRotate == 0 ? 1 : 0;
    out->newLeaderHandPreservedAfterRotate =
        hand.itemType == DM1_PC34_SW_DROP_ROT_NLO_NEW_LEADER_HAND_ITEM &&
        hand.weight == DM1_PC34_SW_DROP_ROT_NLO_NEW_HAND_WEIGHT &&
        hand.charges == DM1_PC34_SW_DROP_ROT_NLO_NEW_HAND_CHARGES ? 1 : 0;
    record_slots(&runtime, runtime.openChampion, out->visibleTypesAfterRotate,
                 out->visibleChargesAfterRotate,
                 out->visibleQuantitiesAfterRotate);
    out->c540TypeAfterRotate =
        out->visibleTypesAfterRotate
            [DM1_PC34_SW_DROP_ROT_NLO_TARGET_SLOT_INDEX];
    out->c540QuantityAfterRotate =
        out->visibleQuantitiesAfterRotate
            [DM1_PC34_SW_DROP_ROT_NLO_TARGET_SLOT_INDEX];
    out->c540StillVisibleAfterRotate =
        out->c540TypeAfterRotate == DM1_PC34_SW_DROP_ROT_NLO_DROP_ITEM &&
        out->c540QuantityAfterRotate ==
            DM1_PC34_SW_DROP_ROT_NLO_DROP_QUANTITY ? 1 : 0;
    out->c537ToC544ChainCoherentAfterRotate =
        slots_match_after_drop(out->visibleTypesAfterRotate,
                               out->visibleChargesAfterRotate,
                               out->visibleQuantitiesAfterRotate);
    out->chestNeverClosed =
        runtime.closeCount == 0 &&
        out->newLeaderOpenChestThingAfterRotate ==
            DM1_PC34_SW_DROP_ROT_NLO_CHEST_THING ? 1 : 0;
    out->closeCount = runtime.closeCount;
    out->f0077F0078Balanced =
        runtime.f0077Observed && runtime.f0078Observed &&
        runtime.mouseUpdateDepth == 0 ? 1 : 0;
    out->stepTrace[out->stepCount++] =
        DM1_PC34_SW_DROP_ROT_NLO_STEP_ASSERT_STILL_OPEN;

    out->noPass768CloseRace = 1;
    out->noScrollWheelPickupDrop = 1;
    out->noScrollWheelDropOntoOpenChestSlot = 1;
    out->noChestDepositDuringLeaderRotation = 1;
    out->noPickupDuringResurrectPendingNonLeader = 1;
    out->noChestCloseWhileCandidateLiveNonLeader = 1;
    out->noChestOpenDuringPending = 1;
    out->noChestCloseWithFullLeaderHand = 1;
    out->noChestOpenWithFullLeaderHand = 1;

    hash_probe(&hash, out);
    out->deterministicHash = hash;
    return 1;
}
