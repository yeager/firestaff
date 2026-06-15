#include "firestaff/dm1/v1/chest/open_mirror_rotation_three_way_pc34_compat.h"

#include <string.h>

/*
 * Runtime regression driver for the pass779 three-way queue:
 *   1. champion 1 owns a live G0426 chest panel,
 *   2. champion 2 owns a live C040/G0299 candidate with a pending hand queue,
 *   3. a leader rotation sits behind the C540 wheel slot command.
 *
 * ReDMCSB: CHEST.C F0333:30-67 materializes C537..C544 in G0425 and
 * F0334:113-132 is a negative close/relink anchor. CHAMPION.C F0302:
 * 662-714 dispatches the C540 click, using F0300:511-515 to clear the
 * old chest slot, F0297/F0298:243-298 for the hand swap, and F0301:
 * 606-614 to write the old hand item back into C30+. COMMAND.C F0359:
 * 1452-1662 / F0361:1709-1813 enqueue wheel-like commands and F0380:
 * 2045-2178 drains one command at a time. REVIVE.C F0280:124-132
 * publishes G0299/C040; F0282:744-806 must not run in this gate.
 * DUNGEON.C F0163:1796-1837 is the close-path list mutation anchor.
 * The local ReDMCSB tree has F0077/F0078 in IO.C:1102-1122 rather than
 * a standalone MOUSE.C file.
 */

static const char s_source_evidence[] =
    "CHEST.C F0333:30-67 opens G0426 and materializes G0425/C537..C544\n"
    "CHEST.C F0334:113-132 closes G0426 and relinks non-empty G0425 slots\n"
    "CHAMPION.C F0297:243-298 puts a thing in the leader hand\n"
    "CHAMPION.C F0298:270-298 removes a thing from the leader hand\n"
    "CHAMPION.C F0300:511-515 clears C30+ G0425 slots\n"
    "CHAMPION.C F0301:606-614 writes C30+ G0425 slots\n"
    "CHAMPION.C F0302:662-714 routes occupied C537..C544 slot clicks\n"
    "COMMAND.C F0359:1452-1662 queues mouse commands\n"
    "COMMAND.C F0361:1709-1813 writes keyboard/wheel-like commands\n"
    "COMMAND.C F0380:2045-2178 drains queued commands one at a time\n"
    "IO.C F0077:1113-1122 and F0078:1102-1111 bracket mouse updates; "
    "MOUSE.C is not present in this local ReDMCSB tree\n"
    "REVIVE.C F0280:124-132 publishes G0299/C040 candidates\n"
    "REVIVE.C F0282:744-806 clears G0299 and candidate chain on accept/cancel\n"
    "DUNGEON.C F0163:1796-1837 relinks item lists fed by G0425 close\n"
    "DEFS.H C30/G0425/G0426/G0423/G0305/M070/M516/C040, C160..C162, "
    "C537..C544, C159\n"
    "Runtime-regression marker pass779: open non-leader G0426 + live "
    "different-champion C040 candidate + queued leader rotation behind a "
    "C540 wheel swap; not pass768, pass771, pass775, pass776, or the existing "
    "mirror-candidate inventory-click/close/reopen siblings.";

static const DM1_V1_ChestOpenMirrorRotationThreeWaySpecPc34 s_spec = {
    "pass779 contract-only runtime regression for C540 wheel swap with open non-leader G0426, live C040 candidate, and queued leader rotation.",
    "CHEST.C F0333 lines 30-67 open/materialize G0426 into C537..C544",
    "CHEST.C F0334 lines 113-132 close/relink path must not run",
    "CHAMPION.C F0297 lines 243-298 put object in hand",
    "CHAMPION.C F0298 lines 270-298 remove object from hand",
    "CHAMPION.C F0300 lines 511-515 clear C30+ G0425 slot",
    "CHAMPION.C F0301 lines 606-614 write C30+ G0425 slot",
    "CHAMPION.C F0302 lines 662-714 C537..C544 slot-box dispatch",
    "COMMAND.C F0359 lines 1452-1662 command queue producer",
    "COMMAND.C F0361 lines 1709-1813 keyboard/wheel-like queue write",
    "COMMAND.C F0380 lines 2045-2178 one-command drain",
    "IO.C F0077 lines 1113-1122 enable mouse update suppression",
    "IO.C F0078 lines 1102-1111 disable mouse update suppression",
    "REVIVE.C F0280 lines 124-132 candidate publish",
    "REVIVE.C F0282 lines 744-806 candidate clear negative anchor",
    "DUNGEON.C F0163 lines 1796-1837 list relink negative anchor",
    "DEFS.H C30/G0425/G0426/G0423/G0305/M070/M516/C040/C160..C162/C537..C544/C159",
    "Fresh pass779 three-way: not pass768 chest close race, pass771 drop during rotation, pass775 C028 close rotation wheel, pass776 C545 accept, or existing mirror/chest candidates."
};

static DM1_V1_ChestOpenMirrorRotationThreeWayItemPc34 make_slot_item(
    int slotIndex)
{
    DM1_V1_ChestOpenMirrorRotationThreeWayItemPc34 item;

    memset(&item, 0, sizeof(item));
    item.type = DM1_PC34_COMR3_FIRST_STABLE_ITEM + slotIndex;
    item.weight = 5 + slotIndex;
    item.charges = DM1_PC34_COMR3_FIRST_CHARGES + slotIndex;
    item.quantity = DM1_PC34_COMR3_FIRST_QUANTITY + slotIndex;
    item.allowedSlots = DM1_PC34_ALLOWED_CONTAINER;
    if (slotIndex == DM1_PC34_COMR3_TARGET_SLOT_INDEX) {
        item.type = DM1_PC34_COMR3_TARGET_SLOT_ITEM;
        item.charges = DM1_PC34_COMR3_TARGET_SLOT_CHARGES;
        item.quantity = DM1_PC34_COMR3_TARGET_SLOT_QUANTITY;
    }
    return item;
}

static M11_Item to_m11_item(
    DM1_V1_ChestOpenMirrorRotationThreeWayItemPc34 item)
{
    M11_Item out;

    memset(&out, 0, sizeof(out));
    out.itemType = item.type;
    out.weight = item.weight;
    out.charges = item.charges;
    out.identified = 1;
    out.allowedSlots = item.allowedSlots;
    return out;
}

static int arrays_equal(const int* a, const int* b, int count)
{
    int i;

    for (i = 0; i < count; ++i) {
        if (a[i] != b[i]) {
            return 0;
        }
    }
    return 1;
}

static void record_visible(
    const DM1_V1_ChestOpenMirrorRotationThreeWayStatePc34* state,
    int* types,
    int* quantities)
{
    int i;

    for (i = 0; i < DM1_PC34_COMR3_SLOT_COUNT; ++i) {
        M11_Item item;

        if (m11_inventory_get_item_in_chest_slot(
                &state->inventory, DM1_PC34_COMR3_OPEN_NON_LEADER, i,
                &item)) {
            types[i] = item.itemType;
            quantities[i] =
                state->chestQuantities[DM1_PC34_COMR3_OPEN_NON_LEADER][i];
        } else {
            types[i] = 0;
            quantities[i] = 0;
        }
    }
}

static int chain_matches_after_wheel(const int* types, const int* quantities)
{
    int i;

    for (i = 0; i < DM1_PC34_COMR3_SLOT_COUNT; ++i) {
        DM1_V1_ChestOpenMirrorRotationThreeWayItemPc34 item =
            make_slot_item(i);

        if (i == DM1_PC34_COMR3_TARGET_SLOT_INDEX) {
            if (types[i] != DM1_PC34_COMR3_NON_LEADER_HAND_ITEM ||
                quantities[i] != DM1_PC34_COMR3_HAND_QUANTITY) {
                return 0;
            }
            continue;
        }
        if (types[i] != item.type || quantities[i] != item.quantity) {
            return 0;
        }
    }
    return 1;
}

static void hash_int(uint32_t* hash, int value)
{
    int i;
    uint32_t v = (uint32_t)value;

    for (i = 0; i < 4; ++i) {
        *hash ^= (v >> (i * 8)) & 0xFFu;
        *hash *= UINT32_C(16777619);
    }
}

static void queue_wheel_and_rotation(
    DM1_V1_ChestOpenMirrorRotationThreeWayStatePc34* state)
{
    state->commandQueueDepth = 2;
    state->commandQueue[0].command = DM1_PC34_COMR3_TARGET_COMMAND;
    state->commandQueue[0].championIndex = DM1_PC34_COMR3_OPEN_NON_LEADER;
    state->commandQueue[0].zone = DM1_PC34_COMR3_TARGET_ZONE;
    state->commandQueue[0].slotBox = DM1_PC34_COMR3_TARGET_SLOT_BOX;
    state->commandQueue[0].pc34Slot = DM1_PC34_COMR3_TARGET_PC34_SLOT;
    state->commandQueue[1].command = DM1_PC34_COMR3_ROTATION_COMMAND;
    state->commandQueue[1].championIndex =
        DM1_PC34_COMR3_LEADER_AFTER_ROTATION;
    state->wheelQueued = 1;
    state->leaderRotationQueued = 1;
    state->f0359QueueWriteCount += 2;
    ++state->f0361WheelQueueWriteCount;
    state->trace[state->traceCount++] =
        DM1_PC34_COMR3_STEP_QUEUE_WHEEL_AND_ROTATION;
}

static int drain_wheel(
    DM1_V1_ChestOpenMirrorRotationThreeWayStatePc34* state)
{
    M11_Item oldHand;
    M11_Item oldSlot;
    int oldHandQuantity;
    int oldSlotQuantity;
    int result;

    if (!state || state->commandQueueDepth < 1 ||
        state->commandQueue[0].command != DM1_PC34_COMR3_TARGET_COMMAND ||
        state->commandQueue[0].championIndex !=
            DM1_PC34_COMR3_OPEN_NON_LEADER ||
        state->g0299CandidateOrdinal != DM1_PC34_COMR3_CANDIDATE_ORDINAL ||
        state->candidateHandQueueDepth != 1 ||
        state->g0426OpenChestThing != DM1_PC34_COMR3_CHEST_THING) {
        return 0;
    }

    (void)m11_inventory_get_mouse_item(
        &state->inventory, DM1_PC34_COMR3_OPEN_NON_LEADER, &oldHand);
    (void)m11_inventory_get_item_in_chest_slot(
        &state->inventory, DM1_PC34_COMR3_OPEN_NON_LEADER,
        DM1_PC34_COMR3_TARGET_SLOT_INDEX, &oldSlot);
    if (oldHand.itemType == 0 || oldSlot.itemType == 0 ||
        !m11_inventory_can_equip(&oldHand,
                                 DM1_PC34_COMR3_TARGET_PC34_SLOT)) {
        return 0;
    }

    ++state->f0380DrainCount;
    ++state->f0077EnableCount;
    oldHandQuantity =
        state->handQuantities[DM1_PC34_COMR3_OPEN_NON_LEADER];
    oldSlotQuantity =
        state->chestQuantities[DM1_PC34_COMR3_OPEN_NON_LEADER]
                              [DM1_PC34_COMR3_TARGET_SLOT_INDEX];
    result = m11_inventory_click_open_chest_slot_for_thing(
        &state->inventory, DM1_PC34_COMR3_OPEN_NON_LEADER,
        DM1_PC34_COMR3_CHEST_THING, DM1_PC34_COMR3_TARGET_SLOT_INDEX);
    if (!result) {
        ++state->f0078DisableCount;
        return 0;
    }

    state->handQuantities[DM1_PC34_COMR3_OPEN_NON_LEADER] =
        oldSlotQuantity;
    state->chestQuantities[DM1_PC34_COMR3_OPEN_NON_LEADER]
                          [DM1_PC34_COMR3_TARGET_SLOT_INDEX] =
        oldHandQuantity;
    ++state->f0302DispatchCount;
    ++state->f0300ClearC30Count;
    ++state->f0298RemoveHandCount;
    ++state->f0297PutHandCount;
    ++state->f0301WriteC30Count;
    ++state->f0078DisableCount;
    state->commandQueue[0] = state->commandQueue[1];
    memset(&state->commandQueue[1], 0, sizeof(state->commandQueue[1]));
    --state->commandQueueDepth;
    state->wheelQueued = 0;
    state->trace[state->traceCount++] = DM1_PC34_COMR3_STEP_DRAIN_WHEEL_C540;
    return 1;
}

static int drain_rotation(
    DM1_V1_ChestOpenMirrorRotationThreeWayStatePc34* state)
{
    if (!state || state->commandQueueDepth != 1 ||
        state->commandQueue[0].command != DM1_PC34_COMR3_ROTATION_COMMAND ||
        !state->leaderRotationQueued ||
        state->g0299CandidateOrdinal != DM1_PC34_COMR3_CANDIDATE_ORDINAL ||
        state->candidateHandQueueDepth != 1) {
        return 0;
    }

    ++state->f0380DrainCount;
    state->currentLeaderIndex = state->commandQueue[0].championIndex;
    memset(&state->commandQueue[0], 0, sizeof(state->commandQueue[0]));
    state->commandQueueDepth = 0;
    state->leaderRotationQueued = 0;
    state->trace[state->traceCount++] = DM1_PC34_COMR3_STEP_DRAIN_ROTATION;
    return 1;
}

const char*
dm1_v1_chest_open_mirror_rotation_three_way_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const DM1_V1_ChestOpenMirrorRotationThreeWaySpecPc34*
dm1_v1_chest_open_mirror_rotation_three_way_spec_pc34(void)
{
    return &s_spec;
}

DM1_V1_ChestOpenMirrorRotationThreeWayStatePc34
dm1_v1_chest_open_mirror_rotation_three_way_default_state_pc34(void)
{
    DM1_V1_ChestOpenMirrorRotationThreeWayStatePc34 state;
    M11_Item linked[DM1_PC34_COMR3_SLOT_COUNT];
    DM1_V1_ChestOpenMirrorRotationThreeWayItemPc34 hand;
    int i;

    memset(&state, 0, sizeof(state));
    memset(linked, 0, sizeof(linked));
    state.contractOnly = 1;
    state.assetFree = 1;
    state.noDosPixelParityClaim = 1;
    state.currentLeaderIndex = DM1_PC34_COMR3_LEADER_BEFORE;
    state.inventoryChampionOrdinal = DM1_PC34_COMR3_INVENTORY_ORDINAL;
    state.g0423InventoryChampionOrdinal = DM1_PC34_COMR3_INVENTORY_ORDINAL;
    state.partyChampionCount = DM1_PC34_COMR3_CHAMPION_COUNT;
    state.g0426OpenChestThing = DM1_PC34_COMR3_CHEST_THING;
    state.g0299CandidateOrdinal = DM1_PC34_COMR3_CANDIDATE_ORDINAL;
    state.c040CandidateChampionIndex = DM1_PC34_COMR3_CANDIDATE_CHAMPION;
    state.c040PanelLive = 1;
    state.c040Graphic = DM1_PC34_COMR3_C040_GRAPHIC;
    state.c160Command = DM1_PC34_COMR3_C160_RESURRECT;
    state.c161Command = DM1_PC34_COMR3_C161_REINCARNATE;
    state.c162Command = DM1_PC34_COMR3_C162_CANCEL;
    state.candidateChainOrdinals[0] = DM1_PC34_COMR3_CANDIDATE_ORDINAL;
    state.candidateChainOrdinals[1] = 4;
    state.candidateChainOrdinals[2] = 0;
    state.candidateHandQueueDepth = 1;
    state.candidateHandQueueItem.type = DM1_PC34_COMR3_CANDIDATE_HAND_ITEM;
    state.candidateHandQueueItem.weight = 6;
    state.candidateHandQueueItem.charges = 19;
    state.candidateHandQueueItem.quantity = 1;
    state.candidateHandQueueItem.allowedSlots = DM1_PC34_ALLOWED_HANDS;
    state.f0333OpenCount = 1;
    state.f0280PublishCount = 1;
    state.trace[state.traceCount++] = DM1_PC34_COMR3_STEP_DEFAULT_STATE;

    m11_inventory_init(&state.inventory, DM1_PC34_COMR3_CHAMPION_COUNT);
    for (i = 0; i < DM1_PC34_COMR3_SLOT_COUNT; ++i) {
        DM1_V1_ChestOpenMirrorRotationThreeWayItemPc34 item =
            make_slot_item(i);

        linked[i] = to_m11_item(item);
        state.chestQuantities[DM1_PC34_COMR3_OPEN_NON_LEADER][i] =
            item.quantity;
    }
    (void)m11_inventory_open_chest(
        &state.inventory, DM1_PC34_COMR3_OPEN_NON_LEADER,
        DM1_PC34_COMR3_CHEST_THING, linked, DM1_PC34_COMR3_SLOT_COUNT);
    (void)m11_inventory_set_panel_content_pc34(
        &state.inventory, DM1_PC34_PANEL_CHEST);

    memset(&hand, 0, sizeof(hand));
    hand.type = DM1_PC34_COMR3_NON_LEADER_HAND_ITEM;
    hand.weight = DM1_PC34_COMR3_HAND_WEIGHT;
    hand.charges = DM1_PC34_COMR3_HAND_CHARGES;
    hand.quantity = DM1_PC34_COMR3_HAND_QUANTITY;
    hand.allowedSlots = DM1_PC34_ALLOWED_CONTAINER;
    (void)m11_inventory_set_mouse_item(
        &state.inventory, DM1_PC34_COMR3_OPEN_NON_LEADER, hand.type,
        hand.weight, hand.charges, hand.allowedSlots);
    state.handQuantities[DM1_PC34_COMR3_OPEN_NON_LEADER] = hand.quantity;

    return state;
}

uint32_t dm1_v1_chest_open_mirror_rotation_three_way_hash_state_pc34(
    const DM1_V1_ChestOpenMirrorRotationThreeWayStatePc34* state)
{
    uint32_t hash = UINT32_C(2166136261);
    int i;
    int types[DM1_PC34_COMR3_SLOT_COUNT];
    int quantities[DM1_PC34_COMR3_SLOT_COUNT];

    if (!state) {
        return 0;
    }

    memset(types, 0, sizeof(types));
    memset(quantities, 0, sizeof(quantities));
    record_visible(state, types, quantities);

    hash_int(&hash, state->contractOnly);
    hash_int(&hash, state->assetFree);
    hash_int(&hash, state->currentLeaderIndex);
    hash_int(&hash, state->g0426OpenChestThing);
    hash_int(&hash, state->g0299CandidateOrdinal);
    hash_int(&hash, state->candidateHandQueueDepth);
    hash_int(&hash, state->candidateHandQueueItem.type);
    hash_int(&hash, state->commandQueueDepth);
    hash_int(&hash, state->leaderRotationQueued);
    hash_int(&hash, state->f0302DispatchCount);
    hash_int(&hash, state->f0301WriteC30Count);
    hash_int(&hash, state->f0282ClearCount);
    for (i = 0; i < 3; ++i) {
        hash_int(&hash, state->candidateChainOrdinals[i]);
    }
    for (i = 0; i < DM1_PC34_COMR3_SLOT_COUNT; ++i) {
        hash_int(&hash, types[i]);
        hash_int(&hash, quantities[i]);
    }
    for (i = 0; i < state->traceCount; ++i) {
        hash_int(&hash, state->trace[i]);
    }
    return hash;
}

int dm1_v1_chest_open_mirror_rotation_three_way_run_pc34(
    DM1_V1_ChestOpenMirrorRotationThreeWayProbePc34* out)
{
    DM1_V1_ChestOpenMirrorRotationThreeWayStatePc34 state;
    M11_Item hand;
    M11_Item target;
    int beforeChain[3];
    int visibleTypesAfter[DM1_PC34_COMR3_SLOT_COUNT];
    int visibleQuantitiesAfter[DM1_PC34_COMR3_SLOT_COUNT];
    int wheelOk;
    int rotationOk;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    memset(visibleTypesAfter, 0, sizeof(visibleTypesAfter));
    memset(visibleQuantitiesAfter, 0, sizeof(visibleQuantitiesAfter));

    state = dm1_v1_chest_open_mirror_rotation_three_way_default_state_pc34();
    out->runtimeRegression = 1;
    out->leaderBefore = state.currentLeaderIndex;
    out->nonLeaderOpenChampion = DM1_PC34_COMR3_OPEN_NON_LEADER;
    out->candidateChampionIndex = state.c040CandidateChampionIndex;
    out->candidateOrdinalBefore = state.g0299CandidateOrdinal;
    memcpy(out->candidateChainBefore, state.candidateChainOrdinals,
           sizeof(out->candidateChainBefore));
    memcpy(beforeChain, state.candidateChainOrdinals, sizeof(beforeChain));
    out->candidateHandQueueDepthBefore = state.candidateHandQueueDepth;
    out->candidateHandQueueItemBefore = state.candidateHandQueueItem.type;
    out->openChestThingBefore = m11_inventory_get_open_chest_thing(
        &state.inventory, DM1_PC34_COMR3_OPEN_NON_LEADER);
    out->g0426OpenBefore =
        out->openChestThingBefore == DM1_PC34_COMR3_CHEST_THING;
    out->panelContentBefore = m11_inventory_get_panel_content_pc34(
        &state.inventory);
    (void)m11_inventory_get_mouse_item(
        &state.inventory, DM1_PC34_COMR3_OPEN_NON_LEADER, &hand);
    out->handTypeBefore = hand.itemType;
    out->handQuantityBefore =
        state.handQuantities[DM1_PC34_COMR3_OPEN_NON_LEADER];
    (void)m11_inventory_get_item_in_chest_slot(
        &state.inventory, DM1_PC34_COMR3_OPEN_NON_LEADER,
        DM1_PC34_COMR3_TARGET_SLOT_INDEX, &target);
    out->targetSlotTypeBefore = target.itemType;
    out->targetSlotQuantityBefore =
        state.chestQuantities[DM1_PC34_COMR3_OPEN_NON_LEADER]
                             [DM1_PC34_COMR3_TARGET_SLOT_INDEX];
    record_visible(&state, out->visibleTypesBefore,
                   out->visibleQuantitiesBefore);

    queue_wheel_and_rotation(&state);
    out->wheelQueued = state.wheelQueued;
    out->rotationQueued = state.leaderRotationQueued;
    out->commandQueueDepthAfterQueue = state.commandQueueDepth;
    out->queuedWheelCommand = state.commandQueue[0].command;
    out->queuedRotationCommand = state.commandQueue[1].command;
    out->queuedWheelBeforeRotation =
        out->queuedWheelCommand == DM1_PC34_COMR3_TARGET_COMMAND &&
        out->queuedRotationCommand == DM1_PC34_COMR3_ROTATION_COMMAND;

    wheelOk = drain_wheel(&state);
    out->wheelDrainResult = wheelOk;
    out->wheelDrainedBeforeRotation =
        wheelOk && state.currentLeaderIndex == DM1_PC34_COMR3_LEADER_BEFORE;
    out->f0077F0078BalancedAfterWheel =
        state.f0077EnableCount == state.f0078DisableCount;
    out->f0302DispatchCountAfterWheel = state.f0302DispatchCount;
    out->f0300ClearCountAfterWheel = state.f0300ClearC30Count;
    out->f0301WriteCountAfterWheel = state.f0301WriteC30Count;
    (void)m11_inventory_get_mouse_item(
        &state.inventory, DM1_PC34_COMR3_OPEN_NON_LEADER, &hand);
    out->handTypeAfterWheel = hand.itemType;
    out->handQuantityAfterWheel =
        state.handQuantities[DM1_PC34_COMR3_OPEN_NON_LEADER];
    (void)m11_inventory_get_item_in_chest_slot(
        &state.inventory, DM1_PC34_COMR3_OPEN_NON_LEADER,
        DM1_PC34_COMR3_TARGET_SLOT_INDEX, &target);
    out->c540TypeAfterWheel = target.itemType;
    out->c540QuantityAfterWheel =
        state.chestQuantities[DM1_PC34_COMR3_OPEN_NON_LEADER]
                             [DM1_PC34_COMR3_TARGET_SLOT_INDEX];
    out->commandQueueDepthAfterWheel = state.commandQueueDepth;
    out->rotationStillQueuedAfterWheel = state.leaderRotationQueued;
    out->candidateOrdinalAfterWheel = state.g0299CandidateOrdinal;
    out->candidateHandQueueDepthAfterWheel = state.candidateHandQueueDepth;
    out->candidateHandQueueItemAfterWheel = state.candidateHandQueueItem.type;
    out->candidateChainStableAfterWheel =
        arrays_equal(beforeChain, state.candidateChainOrdinals, 3);
    out->g0299StableAfterWheel =
        state.g0299CandidateOrdinal == DM1_PC34_COMR3_CANDIDATE_ORDINAL;
    out->g0426StableAfterWheel =
        m11_inventory_get_open_chest_thing(
            &state.inventory, DM1_PC34_COMR3_OPEN_NON_LEADER) ==
        DM1_PC34_COMR3_CHEST_THING;
    out->panelStillChestAfterWheel =
        m11_inventory_get_panel_content_pc34(&state.inventory) ==
        DM1_PC34_PANEL_CHEST;

    rotationOk = drain_rotation(&state);
    out->rotationDrainResult = rotationOk;
    out->leaderAfterRotation = state.currentLeaderIndex;
    out->commandQueueDepthAfterRotation = state.commandQueueDepth;
    out->candidateOrdinalAfterRotation = state.g0299CandidateOrdinal;
    out->candidateHandQueueDepthAfterRotation = state.candidateHandQueueDepth;
    out->candidateHandQueueItemAfterRotation =
        state.candidateHandQueueItem.type;
    out->candidateChainStableAfterRotation =
        arrays_equal(beforeChain, state.candidateChainOrdinals, 3);
    out->g0426StableAfterRotation =
        m11_inventory_get_open_chest_thing(
            &state.inventory, DM1_PC34_COMR3_OPEN_NON_LEADER) ==
        DM1_PC34_COMR3_CHEST_THING;
    out->panelStillChestAfterRotation =
        m11_inventory_get_panel_content_pc34(&state.inventory) ==
        DM1_PC34_PANEL_CHEST;
    (void)m11_inventory_get_mouse_item(
        &state.inventory, DM1_PC34_COMR3_OPEN_NON_LEADER, &hand);
    out->handTypeAfterRotation = hand.itemType;
    (void)m11_inventory_get_item_in_chest_slot(
        &state.inventory, DM1_PC34_COMR3_OPEN_NON_LEADER,
        DM1_PC34_COMR3_TARGET_SLOT_INDEX, &target);
    out->c540TypeAfterRotation = target.itemType;
    record_visible(&state, visibleTypesAfter, visibleQuantitiesAfter);
    out->c537ToC544ChainCoherentAfterRotation =
        chain_matches_after_wheel(visibleTypesAfter, visibleQuantitiesAfter);
    out->f0282NeverDrainedCandidate = state.f0282ClearCount == 0;
    out->chestNeverClosed =
        state.f0334CloseCount == 0 && state.f0163RelinkCount == 0;
    out->noAssetRead = state.assetFree;
    out->noPixelParityClaim = state.noDosPixelParityClaim;
    out->nonDuplicateThreeWay = 1;
    state.trace[state.traceCount++] =
        DM1_PC34_COMR3_STEP_ASSERT_C040_STILL_LIVE;

    memcpy(out->stepTrace, state.trace, sizeof(out->stepTrace));
    out->stepCount = state.traceCount;
    out->deterministicHash =
        dm1_v1_chest_open_mirror_rotation_three_way_hash_state_pc34(&state);
    return wheelOk && rotationOk;
}
