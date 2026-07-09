#include "dm1_v1_chest_close_with_full_leader_hand_pc34_compat.h"

#include "dm1_v1_inventory_chest_load_pc34_compat.h"

#include <string.h>

enum {
    DM1_PC34_FULL_HAND_CHEST_A_THING = 0x6A10,
    DM1_PC34_FULL_HAND_CHEST_B_THING = 0x6B20,
    DM1_PC34_FULL_HAND_CHEST_A_CELL_X = 5,
    DM1_PC34_FULL_HAND_CHEST_A_CELL_Y = 9,
    DM1_PC34_FULL_HAND_CHEST_B_CELL_X = 12,
    DM1_PC34_FULL_HAND_CHEST_B_CELL_Y = 3,
    DM1_PC34_FULL_HAND_CHEST_A_FIRST_ITEM = 800,
    DM1_PC34_FULL_HAND_CHEST_A_HIDDEN_TAIL = 808,
    DM1_PC34_FULL_HAND_CHEST_B_FIRST_ITEM = 900,
    DM1_PC34_FULL_HAND_CHEST_A_C537_WEIGHT = 2,
    DM1_PC34_FULL_HAND_CHEST_B_C537_WEIGHT = 11
};

static const char s_source_evidence[] =
    "CHEST.C F0333:53-67 materializes the first eight linked chest objects into G0425_aT_ChestSlots\n"
    "CHEST.C F0334:117-132 rewrites the open container link array from non-empty G0425_aT_ChestSlots\n"
    "CHAMPION.C F0302:688-710 reads leader hand/C30+ slot, validates AllowedSlots, and swaps hand with slot\n"
    "DUNGEON.C F0140:1114-1120 gives containers base weight 50 plus linked contents\n"
    "CHAMPION.C F0297/F0300/F0301:263-265,582-615 adjust champion load through F0140 object weight";

const DM1_V1_ChestCloseFullLeaderHandSpecPc34
    dm1_v1_chest_close_with_full_leader_hand_pc34_spec = {
        "Source-locked contract gate only; not full real-asset chest runtime parity.",
        DM1_PC34_CHEST_CLOSE_FULL_HAND_C537_SLOT,
        DM1_PC34_CHEST_CLOSE_FULL_HAND_C538_SLOT,
        DM1_PC34_CHEST_CLOSE_FULL_HAND_C539_SLOT,
        DM1_PC34_CHEST_CLOSE_FULL_HAND_C540_SLOT,
        DM1_PC34_CHEST_CLOSE_FULL_HAND_C541_SLOT,
        DM1_PC34_CHEST_CLOSE_FULL_HAND_C542_SLOT,
        DM1_PC34_CHEST_CLOSE_FULL_HAND_C543_SLOT,
        DM1_PC34_CHEST_CLOSE_FULL_HAND_C544_SLOT,
        DM1_PC34_CHEST_CLOSE_FULL_HAND_SLOT_COUNT,
        DM1_PC34_CHEST_CLOSE_FULL_HAND_CONTAINER_BASE_WEIGHT,
        DM1_PC34_CHEST_CLOSE_FULL_HAND_LEADER_MAX_LOAD,
        DM1_PC34_CHEST_CLOSE_FULL_HAND_BASE_BACKPACK_WEIGHT,
        DM1_PC34_FULL_HAND_CHEST_A_CELL_X,
        DM1_PC34_FULL_HAND_CHEST_A_CELL_Y,
        DM1_PC34_FULL_HAND_CHEST_B_CELL_X,
        DM1_PC34_FULL_HAND_CHEST_B_CELL_Y,
        { DM1_PC34_CHEST_CLOSE_FULL_HAND_HELMET_ITEM_TYPE,
          DM1_PC34_CHEST_CLOSE_FULL_HAND_HELMET_WEIGHT,
          0,
          0,
          1,
          DM1_PC34_ALLOWED_HEAD | DM1_PC34_ALLOWED_CONTAINER },
        { DM1_PC34_FULL_HAND_CHEST_B_FIRST_ITEM + 1,
          DM1_PC34_FULL_HAND_CHEST_B_C537_WEIGHT + 1,
          0,
          0,
          1,
          DM1_PC34_ALLOWED_CONTAINER }
    };

static DM1_V1_ItemPc34 make_item(int itemType, int weight, int allowedSlots)
{
    DM1_V1_ItemPc34 item;

    memset(&item, 0, sizeof(item));
    item.itemType = itemType;
    item.weight = weight;
    item.charges = 0;
    item.cursed = 0;
    item.identified = 1;
    item.allowedSlots = allowedSlots;
    return item;
}

static int copy_open_chest_types_and_masks(const DM1_V1_InventoryStatePc34* state,
                                           int champ,
                                           int* typesOut,
                                           int* allowedSlotsOut)
{
    int i;

    if (!state || !typesOut || !allowedSlotsOut) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_CLOSE_FULL_HAND_SLOT_COUNT; ++i) {
        DM1_V1_ItemPc34 item;

        if (!DM1_V1_Inventory_GetItemInChestSlotPc34Compat(state, champ, i, &item)) {
            return 0;
        }
        typesOut[i] = item.itemType;
        allowedSlotsOut[i] = item.allowedSlots;
    }
    return 1;
}

static void copy_item_types_and_weights(const DM1_V1_ItemPc34* items,
                                        int count,
                                        int* typesOut,
                                        int* weightsOut)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_CLOSE_FULL_HAND_SLOT_COUNT; ++i) {
        if (i < count && items[i].itemType != 0) {
            typesOut[i] = items[i].itemType;
            weightsOut[i] = items[i].weight;
        } else {
            typesOut[i] = 0;
            weightsOut[i] = 0;
        }
    }
}

static void copy_int_array(const int* source, int* target)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_CLOSE_FULL_HAND_SLOT_COUNT; ++i) {
        target[i] = source[i];
    }
}

static int contains_type(const int* types, int count, int itemType)
{
    int i;

    if (!types || count < 0) {
        return 0;
    }
    for (i = 0; i < count && i < DM1_PC34_CHEST_CLOSE_FULL_HAND_SLOT_COUNT; ++i) {
        if (types[i] == itemType) {
            return 1;
        }
    }
    return 0;
}

static int arrays_differ(const int* left, const int* right)
{
    int i;

    if (!left || !right) {
        return 1;
    }
    for (i = 0; i < DM1_PC34_CHEST_CLOSE_FULL_HAND_SLOT_COUNT; ++i) {
        if (left[i] != right[i]) {
            return 1;
        }
    }
    return 0;
}

static int container_weight_from_closed_links(const DM1_V1_ItemPc34* items, int count)
{
    int total = DM1_PC34_CHEST_CLOSE_FULL_HAND_CONTAINER_BASE_WEIGHT;
    int i;

    for (i = 0; i < count && i < DM1_PC34_CHEST_CLOSE_FULL_HAND_SLOT_COUNT; ++i) {
        if (items[i].itemType != 0) {
            total += items[i].weight;
        }
    }
    return total;
}

const char* dm1_v1_chest_close_with_full_leader_hand_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const DM1_V1_ChestCloseFullLeaderHandSpecPc34*
dm1_v1_chest_close_with_full_leader_hand_spec_pc34(void)
{
    return &dm1_v1_chest_close_with_full_leader_hand_pc34_spec;
}

int dm1_v1_chest_close_with_full_leader_hand_pc34(
    DM1_V1_ChestCloseFullLeaderHandProbePc34* out)
{
    DM1_V1_InventoryStatePc34 state;
    DM1_V1_ItemPc34 chestAInput[9];
    DM1_V1_ItemPc34 chestBInput[DM1_PC34_CHEST_CLOSE_FULL_HAND_SLOT_COUNT];
    DM1_V1_ItemPc34 chestAClosed[DM1_PC34_CHEST_CLOSE_FULL_HAND_SLOT_COUNT];
    DM1_V1_ItemPc34 chestBClosed[DM1_PC34_CHEST_CLOSE_FULL_HAND_SLOT_COUNT];
    DM1_V1_ItemPc34 item;
    int chestAClosedSnapshot[DM1_PC34_CHEST_CLOSE_FULL_HAND_SLOT_COUNT];
    int chestBAllowedSlots[DM1_PC34_CHEST_CLOSE_FULL_HAND_SLOT_COUNT];
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    memset(chestAClosed, 0, sizeof(chestAClosed));
    memset(chestBClosed, 0, sizeof(chestBClosed));

    out->sourceLockedContractOnly = 1;
    out->chestAThing = DM1_PC34_FULL_HAND_CHEST_A_THING;
    out->chestBThing = DM1_PC34_FULL_HAND_CHEST_B_THING;
    out->chestACellX = DM1_PC34_FULL_HAND_CHEST_A_CELL_X;
    out->chestACellY = DM1_PC34_FULL_HAND_CHEST_A_CELL_Y;
    out->chestBCellX = DM1_PC34_FULL_HAND_CHEST_B_CELL_X;
    out->chestBCellY = DM1_PC34_FULL_HAND_CHEST_B_CELL_Y;

    DM1_V1_Inventory_InitPc34Compat(&state, 1);
    out->setupBaseLoadResult = DM1_V1_Inventory_SetItemInPc34SourceSlotCompat(
        &state, 0, DM1_PC34_SLOT_BACKPACK_LINE1_1,
        DM1_PC34_CHEST_CLOSE_FULL_HAND_BACKPACK_ITEM,
        DM1_PC34_CHEST_CLOSE_FULL_HAND_BASE_BACKPACK_WEIGHT,
        0, DM1_PC34_ALLOWED_ANY_SLOT);
    if (!out->setupBaseLoadResult) {
        return 0;
    }
    out->baseBackpackLoad = DM1_V1_Inventory_GetLoadPc34Compat(&state, 0);

    for (i = 0; i < DM1_PC34_CHEST_CLOSE_FULL_HAND_SLOT_COUNT; ++i) {
        chestAInput[i] = make_item(DM1_PC34_FULL_HAND_CHEST_A_FIRST_ITEM + i,
                                   DM1_PC34_FULL_HAND_CHEST_A_C537_WEIGHT + i,
                                   DM1_PC34_ALLOWED_CONTAINER);
    }
    chestAInput[DM1_PC34_CHEST_CLOSE_FULL_HAND_C544_INDEX] =
        make_item(DM1_PC34_CHEST_CLOSE_FULL_HAND_HELMET_ITEM_TYPE,
                  DM1_PC34_CHEST_CLOSE_FULL_HAND_HELMET_WEIGHT,
                  DM1_PC34_ALLOWED_HEAD | DM1_PC34_ALLOWED_CONTAINER);
    chestAInput[8] = make_item(DM1_PC34_FULL_HAND_CHEST_A_HIDDEN_TAIL,
                               19, DM1_PC34_ALLOWED_CONTAINER);
    out->chestAHiddenTailInputType = chestAInput[8].itemType;

    /* ReDMCSB CHEST.C F0333 lines 53-67 copies only the first eight linked
     * objects into G0425_aT_ChestSlots for chest A. */
    out->chestAOpenResult = DM1_V1_Inventory_OpenChestPc34Compat(
        &state, 0, out->chestAThing, chestAInput, 9);
    out->chestAOpenThing = DM1_V1_Inventory_GetOpenChestThingPc34Compat(&state, 0);
    if (!out->chestAOpenResult ||
        !copy_open_chest_types_and_masks(&state, 0, out->chestAOpenTypes,
                                         out->chestAOpenAllowedSlots)) {
        return 0;
    }
    out->chestAVisibleWeightAfterOpen =
        m11_inventory_pc34_open_chest_visible_contents_weight(&state, 0);
    out->chestAContainerWeightAfterOpen =
        m11_inventory_pc34_open_chest_container_weight(&state, 0);

    out->leaderHandBeforeC544Click = 0;
    if (!DM1_V1_Inventory_GetItemInChestSlotPc34Compat(
            &state, 0, DM1_PC34_CHEST_CLOSE_FULL_HAND_C544_INDEX, &item)) {
        return 0;
    }
    out->c544BeforePickupType = item.itemType;
    out->c544BeforePickupAllowedSlots = item.allowedSlots;
    out->c544HelmetCanLeaveChest =
        DM1_V1_Inventory_CanEquipPc34Compat(&item, DM1_PC34_CHEST_CLOSE_FULL_HAND_C544_SLOT);

    /* ReDMCSB CHAMPION.C F0302 lines 688-710 reads the empty leader hand and
     * C544, removes the slot object, and puts that chest-compatible helmet in
     * the leader hand while C544 becomes empty. */
    out->c544ClickResult = DM1_V1_Inventory_ClickPc34SourceSlotCompat(
        &state, 0, DM1_PC34_CHEST_CLOSE_FULL_HAND_C544_SLOT);
    if (!DM1_V1_Inventory_GetMouseItemPc34Compat(&state, 0, &item)) {
        return 0;
    }
    out->leaderHandAfterC544Click = item.itemType;
    out->leaderHandAfterC544ClickAllowedSlots = item.allowedSlots;
    out->leaderHandFullAfterC544Click = item.itemType != 0 ? 1 : 0;
    if (!DM1_V1_Inventory_GetItemInChestSlotPc34Compat(
            &state, 0, DM1_PC34_CHEST_CLOSE_FULL_HAND_C544_INDEX, &item)) {
        return 0;
    }
    out->c544AfterPickupType = item.itemType;
    out->chestAVisibleWeightAfterPickup =
        m11_inventory_pc34_open_chest_visible_contents_weight(&state, 0);
    out->loadAfterC544Pickup = DM1_V1_Inventory_GetLoadPc34Compat(&state, 0);

    /* ReDMCSB CHEST.C F0334 lines 117-132 closes chest A from the visible
     * G0425 slots while the leader hand remains full. */
    out->leaderHandFullDuringChestAClose = out->leaderHandFullAfterC544Click;
    out->chestACloseCount = m11_inventory_pc34_close_chest_with_weight_snapshot(
        &state, 0, chestAClosed, DM1_PC34_CHEST_CLOSE_FULL_HAND_SLOT_COUNT,
        &out->chestAContainerWeightSnapshotAtClose);
    if (out->chestACloseCount < 0) {
        return 0;
    }
    copy_item_types_and_weights(chestAClosed, out->chestACloseCount,
                                out->chestAClosedTypes,
                                out->chestAClosedWeights);
    out->chestAHiddenTailExcludedOnClose =
        contains_type(out->chestAClosedTypes, out->chestACloseCount,
                      out->chestAHiddenTailInputType) ? 0 : 1;
    if (!DM1_V1_Inventory_GetMouseItemPc34Compat(&state, 0, &item)) {
        return 0;
    }
    out->leaderHandAfterChestAClose = item.itemType;
    out->leaderHandWeightAfterChestAClose = item.weight;
    out->leaderHandAllowedSlotsAfterChestAClose = item.allowedSlots;
    if (!DM1_V1_Inventory_GetItemInPc34SourceSlotCompat(
            &state, 0, DM1_PC34_SLOT_READY_HAND, &item)) {
        return 0;
    }
    out->readyHandAfterChestAClose = item.itemType;
    if (!DM1_V1_Inventory_GetItemInPc34SourceSlotCompat(
            &state, 0, DM1_PC34_SLOT_ACTION_HAND, &item)) {
        return 0;
    }
    out->actionHandAfterChestAClose = item.itemType;
    if (!DM1_V1_Inventory_GetItemInPc34SourceSlotCompat(
            &state, 0, DM1_PC34_SLOT_BACKPACK_LINE1_1, &item)) {
        return 0;
    }
    out->backpackAfterChestAClose = item.itemType;
    out->loadAfterChestAClose = DM1_V1_Inventory_GetLoadPc34Compat(&state, 0);
    out->chestAReadSlotAfterCloseResult =
        DM1_V1_Inventory_GetItemInChestSlotPc34Compat(
            &state, 0, DM1_PC34_CHEST_CLOSE_FULL_HAND_C537_INDEX, &item);
    out->chestAOpenThingAfterClose = DM1_V1_Inventory_GetOpenChestThingPc34Compat(&state, 0);
    copy_int_array(out->chestAClosedTypes, chestAClosedSnapshot);
    out->containerAWeightAfterFirstClose =
        container_weight_from_closed_links(chestAClosed, out->chestACloseCount);

    for (i = 0; i < DM1_PC34_CHEST_CLOSE_FULL_HAND_SLOT_COUNT; ++i) {
        chestBInput[i] = make_item(DM1_PC34_FULL_HAND_CHEST_B_FIRST_ITEM + i,
                                   DM1_PC34_FULL_HAND_CHEST_B_C537_WEIGHT + i,
                                   DM1_PC34_ALLOWED_CONTAINER);
    }
    out->leaderHandBeforeChestBOpen = out->leaderHandAfterChestAClose;

    /* ReDMCSB CHEST.C F0333 lines 53-67 opens a different chest on a different
     * cell and repopulates G0425 from chest B, independent of the full hand. */
    out->chestBOpenResult = DM1_V1_Inventory_OpenChestPc34Compat(
        &state, 0, out->chestBThing, chestBInput,
        DM1_PC34_CHEST_CLOSE_FULL_HAND_SLOT_COUNT);
    out->chestBOpenThing = DM1_V1_Inventory_GetOpenChestThingPc34Compat(&state, 0);
    if (!out->chestBOpenResult ||
        !copy_open_chest_types_and_masks(&state, 0, out->chestBOpenTypes,
                                         chestBAllowedSlots)) {
        return 0;
    }
    if (!DM1_V1_Inventory_GetMouseItemPc34Compat(&state, 0, &item)) {
        return 0;
    }
    out->leaderHandAfterChestBOpen = item.itemType;
    out->chestBContainsLeaderHelmet =
        contains_type(out->chestBOpenTypes,
                      DM1_PC34_CHEST_CLOSE_FULL_HAND_SLOT_COUNT,
                      out->leaderHandAfterChestBOpen);
    out->chestBVisibleWeightAfterOpen =
        m11_inventory_pc34_open_chest_visible_contents_weight(&state, 0);
    out->chestBContainerWeightAfterOpen =
        m11_inventory_pc34_open_chest_container_weight(&state, 0);
    out->chestBC538Type =
        out->chestBOpenTypes[DM1_PC34_CHEST_CLOSE_FULL_HAND_C538_INDEX];
    out->chestBC538IsOwnItem =
        out->chestBC538Type == DM1_PC34_FULL_HAND_CHEST_B_FIRST_ITEM + 1 ? 1 : 0;

    /* ReDMCSB CHEST.C F0334 lines 117-132 rewrites only the current G0426
     * chest B link array; chest A's closed snapshot must remain untouched. */
    out->leaderHandFullDuringChestBClose =
        out->leaderHandAfterChestBOpen != 0 ? 1 : 0;
    out->chestBCloseCount = m11_inventory_pc34_close_chest_with_weight_snapshot(
        &state, 0, chestBClosed, DM1_PC34_CHEST_CLOSE_FULL_HAND_SLOT_COUNT,
        &out->chestBContainerWeightSnapshotAtClose);
    if (out->chestBCloseCount < 0) {
        return 0;
    }
    copy_item_types_and_weights(chestBClosed, out->chestBCloseCount,
                                out->chestBClosedTypes,
                                out->chestBClosedWeights);
    if (!DM1_V1_Inventory_GetMouseItemPc34Compat(&state, 0, &item)) {
        return 0;
    }
    out->leaderHandAfterChestBClose = item.itemType;
    out->leaderHandWeightAfterChestBClose = item.weight;
    out->leaderHandAllowedSlotsAfterChestBClose = item.allowedSlots;
    if (!DM1_V1_Inventory_GetItemInPc34SourceSlotCompat(
            &state, 0, DM1_PC34_SLOT_READY_HAND, &item)) {
        return 0;
    }
    out->readyHandAfterChestBClose = item.itemType;
    if (!DM1_V1_Inventory_GetItemInPc34SourceSlotCompat(
            &state, 0, DM1_PC34_SLOT_ACTION_HAND, &item)) {
        return 0;
    }
    out->actionHandAfterChestBClose = item.itemType;
    if (!DM1_V1_Inventory_GetItemInPc34SourceSlotCompat(
            &state, 0, DM1_PC34_SLOT_BACKPACK_LINE1_1, &item)) {
        return 0;
    }
    out->backpackAfterChestBClose = item.itemType;
    out->loadAfterChestBClose = DM1_V1_Inventory_GetLoadPc34Compat(&state, 0);
    out->chestBReadSlotAfterCloseResult =
        DM1_V1_Inventory_GetItemInChestSlotPc34Compat(
            &state, 0, DM1_PC34_CHEST_CLOSE_FULL_HAND_C537_INDEX, &item);
    out->chestBOpenThingAfterClose = DM1_V1_Inventory_GetOpenChestThingPc34Compat(&state, 0);

    copy_int_array(out->chestAClosedTypes, out->chestAAfterChestBCloseTypes);
    out->chestAChangedByChestBClose =
        arrays_differ(chestAClosedSnapshot, out->chestAAfterChestBCloseTypes);
    out->chestAStateIntactAfterChestBClose =
        out->chestAChangedByChestBClose ? 0 : 1;
    out->containerAWeightAfterChestBClose =
        container_weight_from_closed_links(chestAClosed, out->chestACloseCount);
    out->containerAContainerBaseWeightPreserved =
        out->containerAWeightAfterChestBClose ==
        out->containerAWeightAfterFirstClose ? 1 : 0;
    out->containerBWeightFromClosedLinks =
        container_weight_from_closed_links(chestBClosed, out->chestBCloseCount);
    out->containerBContainerBaseWeightComputed =
        out->containerBWeightFromClosedLinks ==
        out->chestBContainerWeightSnapshotAtClose ? 1 : 0;

    return 1;
}
