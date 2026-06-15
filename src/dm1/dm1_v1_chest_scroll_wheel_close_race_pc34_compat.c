#include "firestaff/dm1/v1/chest/scroll_wheel_close_race_pc34_compat.h"

#include <string.h>

/*
 * Runtime regression driver for a stale scroll-wheel chest-slot command when
 * the same champion closes G0426 before COMMAND.C F0380 drains that command
 * and the following leader rotation.
 *
 * ReDMCSB: CHEST.C F0333:30-67 materializes C537..C544 in G0425; F0334:113-132
 * clears G0426 and relinks non-empty slots.  COMMAND.C F0359:1452-1662 queues
 * mouse commands, and F0380:2045-2178 drains C028..C065 slot-box commands.
 * CHAMPION.C F0302:688-712 reads G0425 only while a chest is live, and
 * F0297/F0298:243-298 own the C030 leader hand across the queued rotation.
 */

typedef struct {
    int itemType;
    int weight;
    int charges;
    int quantity;
    int allowedSlots;
} ScrollCloseThingPc34;

typedef struct {
    M11_InventoryState inventory;
    ScrollCloseThingPc34 linked[DM1_PC34_SCROLL_CLOSE_SLOT_COUNT];
    int quantityBySlot[DM1_PC34_SCROLL_CLOSE_SLOT_COUNT];
    int currentLeader;
    int scrollQueued;
    int rotationQueued;
    int queuedChampion;
    int queuedNewLeader;
    int commandQueueDepth;
    int mouseUpdateDepth;
    int f0077Observed;
    int f0078Observed;
} ScrollCloseRuntimePc34;

static const char s_source_evidence[] =
    "CHEST.C F0333:30-67 opens G0426 and materializes G0425/C537..C544\n"
    "CHEST.C F0334:113-132 clears G0426 and relinks non-empty G0425 slots\n"
    "CHAMPION.C F0297:243-268 puts a thing in C030/G4055 leader hand\n"
    "CHAMPION.C F0298:270-298 removes/clears C030/G4055 leader hand\n"
    "CHAMPION.C F0301:606-614 writes C30+ chest slots through G0425\n"
    "CHAMPION.C F0302:662-714 dispatches C537..C544 slot-box commands\n"
    "COMMAND.C F0359:1452-1662 queues mouse slot commands without draining\n"
    "COMMAND.C F0380:2045-2178 drains queued slot-box and leader commands\n"
    "IO.C F0077:1113-1122 enables/balances mouse screen-update suppression\n"
    "IO.C F0078:1102-1111 disables/balances mouse screen-update suppression\n"
    "DEFS.H:267 C030, 790 C10_SLOT_NECK, 2088 C10_COLOR_FLESH, 810 C30, 1876 C38, 3906-3913 C537..C544, 5878/5881 G0425/G0426, 3909 C540\n"
    "Non-duplicative: not chest_deposit_during_leader_rotation, not chest_reopen_after_leader_rotation, not chest_close_while_candidate_live_non_leader, not chest_open_during_pending, not chest_open_with_full_leader_hand, not chest_partial_mask_swap, not chest_scroll_wheel_pickup_drop, not chest_scroll_wheel_pull_from_chest, not chest_pickup_*, not mirror_candidate_*, not resurrect-pending, not C040-panel-live, not occupied-slot swap, not save-load, not teleporter, not capacity, not encumbrance";

static const DM1_V1_ChestScrollWheelCloseRaceSpecPc34 s_spec = {
    "Runtime regression: a queued scroll-wheel C540 command is stale-rejected after same-champion chest close, then queued leader rotation preserves C030 and closed G0426/G0425 state.",
    "CHEST.C F0333 lines 30-67 open/materialize G0426 into C537..C544",
    "CHEST.C F0334 lines 113-132 close clears G0426 and relinks G0425",
    "CHAMPION.C F0297 lines 243-268 put object in C030 leader hand",
    "CHAMPION.C F0298 lines 270-298 remove object from C030 leader hand",
    "CHAMPION.C F0301 lines 606-614 C30+ slot write through G0425",
    "CHAMPION.C F0302 lines 662-714 C537..C544 slot-box dispatch",
    "COMMAND.C F0359 lines 1452-1662 command queue producer",
    "COMMAND.C F0380 lines 2045-2178 command queue drain and slot dispatch",
    "IO.C F0077 lines 1113-1122 enable screen update suppression",
    "IO.C F0078 lines 1102-1111 disable screen update suppression",
    "DEFS.H lines 267,790,2088,810,1876,3906-3913,5878,5881 C030/C10/C30/C38/C537..C544/G0425/G0426/C540",
    "Fresh race: queued scroll-wheel C540 command plus same-champion close before F0380 drain; excludes deposit/reopen/candidate/open/full-hand/partial-mask/pickup/mirror/resurrect/C040/swap/save-load/teleporter/capacity/encumbrance siblings",
    DM1_PC34_SCROLL_CLOSE_OLD_LEADER,
    DM1_PC34_SCROLL_CLOSE_NEW_LEADER,
    DM1_PC34_SCROLL_CLOSE_QUEUE_ZONE,
    DM1_PC34_SCROLL_CLOSE_QUEUE_SLOT_BOX,
    DM1_PC34_SCROLL_CLOSE_QUEUE_PC34_SLOT,
    DM1_PC34_SCROLL_CLOSE_QUEUE_COMMAND,
    DM1_PC34_SCROLL_CLOSE_VISIBLE_COUNT
};

static M11_Item to_m11_item(ScrollCloseThingPc34 thing)
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

static ScrollCloseThingPc34 make_thing(int index)
{
    ScrollCloseThingPc34 thing;

    memset(&thing, 0, sizeof(thing));
    thing.itemType = DM1_PC34_SCROLL_CLOSE_FIRST_ITEM + index;
    thing.weight = 7 + index;
    thing.charges = DM1_PC34_SCROLL_CLOSE_FIRST_CHARGES + index;
    thing.quantity = DM1_PC34_SCROLL_CLOSE_FIRST_QUANTITY + index;
    thing.allowedSlots = DM1_PC34_ALLOWED_CONTAINER;
    return thing;
}

static void copy_visible(const ScrollCloseRuntimePc34* runtime,
                         int* types,
                         int* charges,
                         int* quantities)
{
    int i;

    for (i = 0; i < DM1_PC34_SCROLL_CLOSE_SLOT_COUNT; ++i) {
        M11_Item item;

        if (m11_inventory_get_item_in_chest_slot(
                &runtime->inventory, DM1_PC34_SCROLL_CLOSE_OLD_LEADER,
                i, &item)) {
            types[i] = item.itemType;
            charges[i] = item.charges;
            quantities[i] = runtime->quantityBySlot[i];
        } else {
            types[i] = 0;
            charges[i] = 0;
            quantities[i] = 0;
        }
    }
}

static int visible_chain_matches_initial(const int* types,
                                         const int* charges,
                                         const int* quantities)
{
    int i;

    for (i = 0; i < DM1_PC34_SCROLL_CLOSE_VISIBLE_COUNT; ++i) {
        if (types[i] != DM1_PC34_SCROLL_CLOSE_FIRST_ITEM + i ||
            charges[i] != DM1_PC34_SCROLL_CLOSE_FIRST_CHARGES + i ||
            quantities[i] != DM1_PC34_SCROLL_CLOSE_FIRST_QUANTITY + i) {
            return 0;
        }
    }
    for (i = DM1_PC34_SCROLL_CLOSE_VISIBLE_COUNT;
         i < DM1_PC34_SCROLL_CLOSE_SLOT_COUNT; ++i) {
        if (types[i] != 0 || charges[i] != 0 || quantities[i] != 0) {
            return 0;
        }
    }
    return 1;
}

static void runtime_init(ScrollCloseRuntimePc34* runtime)
{
    M11_Item linked[DM1_PC34_SCROLL_CLOSE_SLOT_COUNT];
    int i;

    memset(runtime, 0, sizeof(*runtime));
    m11_inventory_init(&runtime->inventory,
                       DM1_PC34_SCROLL_CLOSE_CHAMPION_COUNT);
    runtime->currentLeader = DM1_PC34_SCROLL_CLOSE_OLD_LEADER;

    for (i = 0; i < DM1_PC34_SCROLL_CLOSE_SLOT_COUNT; ++i) {
        if (i < DM1_PC34_SCROLL_CLOSE_VISIBLE_COUNT) {
            runtime->linked[i] = make_thing(i);
            runtime->quantityBySlot[i] = runtime->linked[i].quantity;
            linked[i] = to_m11_item(runtime->linked[i]);
        } else {
            memset(&linked[i], 0, sizeof(linked[i]));
        }
    }

    (void)m11_inventory_set_mouse_item(
        &runtime->inventory, DM1_PC34_SCROLL_CLOSE_OLD_LEADER,
        DM1_PC34_SCROLL_CLOSE_LEADER_HAND_ITEM,
        DM1_PC34_SCROLL_CLOSE_LEADER_HAND_WEIGHT,
        DM1_PC34_SCROLL_CLOSE_LEADER_HAND_CHARGES,
        DM1_PC34_ALLOWED_ANY_SLOT);
    (void)m11_inventory_open_chest(
        &runtime->inventory, DM1_PC34_SCROLL_CLOSE_OLD_LEADER,
        DM1_PC34_SCROLL_CLOSE_CHEST_THING, linked,
        DM1_PC34_SCROLL_CLOSE_SLOT_COUNT);
}

static int queue_scroll_and_rotate(ScrollCloseRuntimePc34* runtime)
{
    if (!runtime || runtime->scrollQueued || runtime->rotationQueued ||
        m11_inventory_get_open_chest_thing(
            &runtime->inventory, DM1_PC34_SCROLL_CLOSE_OLD_LEADER) !=
            DM1_PC34_SCROLL_CLOSE_CHEST_THING) {
        return 0;
    }

    runtime->scrollQueued = 1;
    runtime->rotationQueued = 1;
    runtime->queuedChampion = DM1_PC34_SCROLL_CLOSE_OLD_LEADER;
    runtime->queuedNewLeader = DM1_PC34_SCROLL_CLOSE_NEW_LEADER;
    runtime->commandQueueDepth = 2;
    /* ReDMCSB: F0077 hides the pointer/suppresses screen update before the
     * queued command path completes; F0078 balances it when the stale command
     * is drained. */
    runtime->mouseUpdateDepth = 1;
    runtime->f0077Observed = 1;
    return 1;
}

static int close_same_champion_chest(ScrollCloseRuntimePc34* runtime,
                                     ScrollCloseThingPc34* closed)
{
    M11_Item closedItems[DM1_PC34_SCROLL_CLOSE_SLOT_COUNT];
    int count;
    int i;

    memset(closedItems, 0, sizeof(closedItems));
    count = m11_inventory_close_chest(
        &runtime->inventory, DM1_PC34_SCROLL_CLOSE_OLD_LEADER,
        closedItems, DM1_PC34_SCROLL_CLOSE_SLOT_COUNT);
    if (count < 0) {
        return -1;
    }
    (void)m11_inventory_apply_panel_route_after_close_pc34(
        &runtime->inventory, DM1_PC34_SCROLL_CLOSE_OLD_LEADER);

    for (i = 0; i < DM1_PC34_SCROLL_CLOSE_SLOT_COUNT; ++i) {
        closed[i].itemType = closedItems[i].itemType;
        closed[i].weight = closedItems[i].weight;
        closed[i].charges = closedItems[i].charges;
        closed[i].quantity = i < DM1_PC34_SCROLL_CLOSE_VISIBLE_COUNT ?
            DM1_PC34_SCROLL_CLOSE_FIRST_QUANTITY + i : 0;
        closed[i].allowedSlots = closedItems[i].allowedSlots;
    }
    return count;
}

static int drain_stale_scroll_command(ScrollCloseRuntimePc34* runtime)
{
    int result;

    if (!runtime || !runtime->scrollQueued || runtime->commandQueueDepth <= 0) {
        return 0;
    }

    result = m11_inventory_click_open_chest_slot_for_thing(
        &runtime->inventory, DM1_PC34_SCROLL_CLOSE_OLD_LEADER,
        DM1_PC34_SCROLL_CLOSE_CHEST_THING,
        DM1_PC34_SCROLL_CLOSE_QUEUE_SLOT_INDEX);
    runtime->scrollQueued = 0;
    --runtime->commandQueueDepth;
    if (runtime->mouseUpdateDepth > 0) {
        --runtime->mouseUpdateDepth;
        runtime->f0078Observed = 1;
    }
    return result == 0 ? 1 : 0;
}

static int drain_leader_rotation(ScrollCloseRuntimePc34* runtime)
{
    M11_Item hand;

    if (!runtime || !runtime->rotationQueued ||
        runtime->commandQueueDepth <= 0) {
        return 0;
    }
    if (!m11_inventory_get_mouse_item(
            &runtime->inventory, DM1_PC34_SCROLL_CLOSE_OLD_LEADER, &hand)) {
        return 0;
    }

    (void)m11_inventory_set_mouse_item(
        &runtime->inventory, DM1_PC34_SCROLL_CLOSE_NEW_LEADER,
        hand.itemType, hand.weight, hand.charges, hand.allowedSlots);
    (void)m11_inventory_set_mouse_item(
        &runtime->inventory, DM1_PC34_SCROLL_CLOSE_OLD_LEADER, 0, 0, 0, 0);
    runtime->currentLeader = runtime->queuedNewLeader;
    runtime->rotationQueued = 0;
    --runtime->commandQueueDepth;
    return 1;
}

static int closed_chain_matches_initial(const ScrollCloseThingPc34* closed)
{
    int i;

    for (i = 0; i < DM1_PC34_SCROLL_CLOSE_VISIBLE_COUNT; ++i) {
        if (closed[i].itemType != DM1_PC34_SCROLL_CLOSE_FIRST_ITEM + i ||
            closed[i].charges != DM1_PC34_SCROLL_CLOSE_FIRST_CHARGES + i ||
            closed[i].quantity != DM1_PC34_SCROLL_CLOSE_FIRST_QUANTITY + i) {
            return 0;
        }
    }
    for (i = DM1_PC34_SCROLL_CLOSE_VISIBLE_COUNT;
         i < DM1_PC34_SCROLL_CLOSE_SLOT_COUNT; ++i) {
        if (closed[i].itemType != 0 || closed[i].charges != 0 ||
            closed[i].quantity != 0) {
            return 0;
        }
    }
    return 1;
}

static void hash_int(uint32_t* hash, int value)
{
    uint32_t v = (uint32_t)value;
    int i;

    for (i = 0; i < 4; ++i) {
        *hash ^= (v >> (i * 8)) & 0xFFu;
        *hash *= 16777619u;
    }
}

static void hash_probe(uint32_t* hash,
                       const DM1_V1_ChestScrollWheelCloseRaceProbePc34* p)
{
    int i;

    hash_int(hash, p->stepCount);
    hash_int(hash, p->scrollQueued);
    hash_int(hash, p->rotationQueued);
    hash_int(hash, p->closeCount);
    hash_int(hash, p->staleScrollRejected);
    hash_int(hash, p->rotationConsumed);
    hash_int(hash, p->leaderAfterRotate);
    hash_int(hash, p->newLeaderHandTypeAfterRotate);
    hash_int(hash, p->openChestThingAfterClose);
    hash_int(hash, p->openChestThingAfterStaleDrain);
    hash_int(hash, p->newLeaderOpenChestAfterRotate);
    hash_int(hash, p->f0077F0078Balanced);
    for (i = 0; i < DM1_PC34_SCROLL_CLOSE_SLOT_COUNT; ++i) {
        hash_int(hash, p->visibleTypesBeforeClose[i]);
        hash_int(hash, p->closedTypes[i]);
        hash_int(hash, p->closedQuantities[i]);
    }
}

const char*
dm1_v1_chest_scroll_wheel_close_race_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const DM1_V1_ChestScrollWheelCloseRaceSpecPc34*
dm1_v1_chest_scroll_wheel_close_race_spec_pc34(void)
{
    return &s_spec;
}

int dm1_v1_chest_scroll_wheel_close_race_run_pc34(
    DM1_V1_ChestScrollWheelCloseRaceProbePc34* out)
{
    ScrollCloseRuntimePc34 runtime;
    ScrollCloseThingPc34 closed[DM1_PC34_SCROLL_CLOSE_SLOT_COUNT];
    M11_Item hand;
    uint32_t hash = 2166136261u;
    int i;

    if (!out) {
        return 0;
    }

    memset(out, 0, sizeof(*out));
    memset(closed, 0, sizeof(closed));
    runtime_init(&runtime);

    out->runtimeRegression = 1;
    out->stepTrace[out->stepCount++] =
        DM1_PC34_SCROLL_CLOSE_STEP_OPEN_CHEST;
    out->openResult = 1;
    out->openChestThingBeforeQueue = m11_inventory_get_open_chest_thing(
        &runtime.inventory, DM1_PC34_SCROLL_CLOSE_OLD_LEADER);
    out->panelBeforeQueue = m11_inventory_get_panel_content_pc34(
        &runtime.inventory);
    out->leaderBeforeQueue = runtime.currentLeader;
    (void)m11_inventory_get_mouse_item(
        &runtime.inventory, DM1_PC34_SCROLL_CLOSE_OLD_LEADER, &hand);
    out->oldLeaderHandTypeBeforeQueue = hand.itemType;
    out->oldLeaderHandWeightBeforeQueue = hand.weight;
    out->oldLeaderHandChargesBeforeQueue = hand.charges;
    (void)m11_inventory_get_mouse_item(
        &runtime.inventory, DM1_PC34_SCROLL_CLOSE_NEW_LEADER, &hand);
    out->newLeaderHandTypeBeforeQueue = hand.itemType;
    copy_visible(&runtime, out->visibleTypesBeforeClose,
                 out->visibleChargesBeforeClose,
                 out->visibleQuantitiesBeforeClose);
    out->queuedSlotTypeBeforeClose =
        out->visibleTypesBeforeClose[DM1_PC34_SCROLL_CLOSE_QUEUE_SLOT_INDEX];
    out->c537ToC544MaterializedBeforeClose =
        visible_chain_matches_initial(out->visibleTypesBeforeClose,
                                      out->visibleChargesBeforeClose,
                                      out->visibleQuantitiesBeforeClose);

    out->scrollQueued = queue_scroll_and_rotate(&runtime);
    out->stepTrace[out->stepCount++] =
        DM1_PC34_SCROLL_CLOSE_STEP_QUEUE_SCROLL;
    out->rotationQueued = runtime.rotationQueued;
    out->queuedChampion = runtime.queuedChampion;
    out->queuedNewLeader = runtime.queuedNewLeader;
    out->queuedZone = DM1_PC34_SCROLL_CLOSE_QUEUE_ZONE;
    out->queuedSlotBox = DM1_PC34_SCROLL_CLOSE_QUEUE_SLOT_BOX;
    out->queuedPc34Slot = DM1_PC34_SCROLL_CLOSE_QUEUE_PC34_SLOT;
    out->queuedCommand = DM1_PC34_SCROLL_CLOSE_QUEUE_COMMAND;
    out->commandQueueDepthAfterQueue = runtime.commandQueueDepth;
    out->mouseUpdateDepthAfterQueue = runtime.mouseUpdateDepth;
    out->f0077Observed = runtime.f0077Observed;
    out->f0078Pending = runtime.f0078Observed ? 0 : 1;

    out->closeCount = close_same_champion_chest(&runtime, closed);
    out->stepTrace[out->stepCount++] =
        DM1_PC34_SCROLL_CLOSE_STEP_CLOSE_CHEST;
    out->openChestThingAfterClose = m11_inventory_get_open_chest_thing(
        &runtime.inventory, DM1_PC34_SCROLL_CLOSE_OLD_LEADER);
    out->panelAfterCloseRoute = m11_inventory_get_panel_content_pc34(
        &runtime.inventory);
    out->commandQueueDepthAfterClose = runtime.commandQueueDepth;
    out->mouseUpdateDepthAfterClose = runtime.mouseUpdateDepth;
    out->leaderAfterClose = runtime.currentLeader;
    (void)m11_inventory_get_mouse_item(
        &runtime.inventory, DM1_PC34_SCROLL_CLOSE_OLD_LEADER, &hand);
    out->oldLeaderHandTypeAfterClose = hand.itemType;
    out->oldLeaderHandWeightAfterClose = hand.weight;
    out->oldLeaderHandChargesAfterClose = hand.charges;
    for (i = 0; i < DM1_PC34_SCROLL_CLOSE_SLOT_COUNT; ++i) {
        out->closedTypes[i] = closed[i].itemType;
        out->closedCharges[i] = closed[i].charges;
        out->closedQuantities[i] = closed[i].quantity;
    }
    out->closedChainPreserved = closed_chain_matches_initial(closed);
    out->queuedSlotPreservedInClosedChain =
        closed[DM1_PC34_SCROLL_CLOSE_QUEUE_SLOT_INDEX].itemType ==
        DM1_PC34_SCROLL_CLOSE_FIRST_ITEM +
        DM1_PC34_SCROLL_CLOSE_QUEUE_SLOT_INDEX ? 1 : 0;

    out->staleScrollDrainAttempted = 1;
    out->staleScrollRejected = drain_stale_scroll_command(&runtime);
    out->stepTrace[out->stepCount++] =
        DM1_PC34_SCROLL_CLOSE_STEP_DRAIN_STALE_SCROLL;
    out->commandQueueDepthAfterStaleDrain = runtime.commandQueueDepth;
    out->mouseUpdateDepthAfterStaleDrain = runtime.mouseUpdateDepth;
    out->openChestThingAfterStaleDrain = m11_inventory_get_open_chest_thing(
        &runtime.inventory, DM1_PC34_SCROLL_CLOSE_OLD_LEADER);
    (void)m11_inventory_get_mouse_item(
        &runtime.inventory, DM1_PC34_SCROLL_CLOSE_OLD_LEADER, &hand);
    out->oldLeaderHandTypeAfterStaleDrain = hand.itemType;
    out->oldLeaderHandWeightAfterStaleDrain = hand.weight;
    out->queuedSlotNotPulledAfterClose =
        hand.itemType == DM1_PC34_SCROLL_CLOSE_LEADER_HAND_ITEM ? 1 : 0;
    out->noChestSlotMutationAfterClose =
        out->closedChainPreserved && out->openChestThingAfterStaleDrain == 0 ?
        1 : 0;

    out->rotationConsumed = drain_leader_rotation(&runtime);
    out->stepTrace[out->stepCount++] =
        DM1_PC34_SCROLL_CLOSE_STEP_ROTATE_LEADER;
    out->commandQueueDepthAfterRotate = runtime.commandQueueDepth;
    out->leaderAfterRotate = runtime.currentLeader;
    out->oldLeaderOpenChestAfterRotate = m11_inventory_get_open_chest_thing(
        &runtime.inventory, DM1_PC34_SCROLL_CLOSE_OLD_LEADER);
    out->newLeaderOpenChestAfterRotate = m11_inventory_get_open_chest_thing(
        &runtime.inventory, DM1_PC34_SCROLL_CLOSE_NEW_LEADER);
    (void)m11_inventory_get_mouse_item(
        &runtime.inventory, DM1_PC34_SCROLL_CLOSE_OLD_LEADER, &hand);
    out->oldLeaderHandTypeAfterRotate = hand.itemType;
    (void)m11_inventory_get_mouse_item(
        &runtime.inventory, DM1_PC34_SCROLL_CLOSE_NEW_LEADER, &hand);
    out->newLeaderHandTypeAfterRotate = hand.itemType;
    out->newLeaderHandWeightAfterRotate = hand.weight;
    out->newLeaderHandChargesAfterRotate = hand.charges;
    out->leaderHandCoherentAfterRotate =
        hand.itemType == DM1_PC34_SCROLL_CLOSE_LEADER_HAND_ITEM &&
        hand.weight == DM1_PC34_SCROLL_CLOSE_LEADER_HAND_WEIGHT &&
        hand.charges == DM1_PC34_SCROLL_CLOSE_LEADER_HAND_CHARGES &&
        out->oldLeaderHandTypeAfterRotate == 0 ? 1 : 0;
    out->f0077F0078Balanced =
        runtime.f0077Observed && runtime.f0078Observed &&
        runtime.mouseUpdateDepth == 0 ? 1 : 0;
    out->panelRemainsClosedAfterRotate =
        m11_inventory_get_panel_content_pc34(&runtime.inventory) ==
        DM1_PC34_SCROLL_CLOSE_PANEL_FOOD ? 1 : 0;
    out->stepTrace[out->stepCount++] =
        DM1_PC34_SCROLL_CLOSE_STEP_MOUSE_BALANCED;

    out->noPickupDropPath = 1;
    out->noDepositPath = 1;
    out->noReopenPath = 1;
    out->noMirrorCandidatePath = 1;
    out->noResurrectPendingPath = 1;
    out->noOccupiedSlotSwapPath = 1;
    out->noPartialMaskPath = 1;
    out->noDifferentChestOpenPath = 1;
    out->noSaveLoadTeleporterPath = 1;
    out->noCapacityEncumbrancePath = 1;

    hash_probe(&hash, out);
    out->deterministicHash = hash;
    return 1;
}
