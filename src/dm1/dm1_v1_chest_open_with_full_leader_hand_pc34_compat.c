#include "dm1_v1_chest_open_with_full_leader_hand_pc34_compat.h"

#include "dm1_v1_inventory_chest_load_pc34_compat.h"

#include <string.h>

enum {
    DM1_PC34_OPEN_FULL_HAND_CHEST_A_THING = 0x7A10,
    DM1_PC34_OPEN_FULL_HAND_CHEST_B_THING = 0x7B20,
    DM1_PC34_OPEN_FULL_HAND_CHEST_A_CELL_X = 6,
    DM1_PC34_OPEN_FULL_HAND_CHEST_A_CELL_Y = 10,
    DM1_PC34_OPEN_FULL_HAND_CHEST_B_CELL_X = 13,
    DM1_PC34_OPEN_FULL_HAND_CHEST_B_CELL_Y = 4,
    DM1_PC34_OPEN_FULL_HAND_CHEST_A_FIRST_ITEM = 810,
    DM1_PC34_OPEN_FULL_HAND_CHEST_B_FIRST_ITEM = 910,
    DM1_PC34_OPEN_FULL_HAND_CHEST_A_FIRST_WEIGHT = 7,
    DM1_PC34_OPEN_FULL_HAND_CHEST_B_FIRST_WEIGHT = 11
};

static const char s_source_evidence[] =
    "CHEST.C F0333:31-38 returns for the same G0426 chest and closes a different open chest before replacement\n"
    "CHEST.C F0333:43,53-67 writes G0426_T_OpenChest, then copies linked objects into G0425_aT_ChestSlots\n"
    "CHEST.C F0334:113-132 rewrites only the previously open container from non-empty G0425_aT_ChestSlots\n"
    "CHAMPION.C F0297/F0298:250-265,270-298 own leader-hand put/remove state\n"
    "CHAMPION.C F0302:688-710 swaps the leader hand only from explicit slot-box clicks, not from F0333 open";

const DM1_V1_ChestOpenFullLeaderHandSpecPc34
    dm1_v1_chest_open_with_full_leader_hand_pc34_spec = {
        "Source-locked contract gate only; not full real-asset chest runtime parity.",
        DM1_PC34_CHEST_OPEN_FULL_HAND_C537_SLOT,
        DM1_PC34_CHEST_OPEN_FULL_HAND_C538_SLOT,
        DM1_PC34_CHEST_OPEN_FULL_HAND_C539_SLOT,
        DM1_PC34_CHEST_OPEN_FULL_HAND_C540_SLOT,
        DM1_PC34_CHEST_OPEN_FULL_HAND_C541_SLOT,
        DM1_PC34_CHEST_OPEN_FULL_HAND_C542_SLOT,
        DM1_PC34_CHEST_OPEN_FULL_HAND_C543_SLOT,
        DM1_PC34_CHEST_OPEN_FULL_HAND_C544_SLOT,
        DM1_PC34_CHEST_OPEN_FULL_HAND_SLOT_COUNT,
        DM1_PC34_CHEST_OPEN_FULL_HAND_CONTAINER_BASE_WEIGHT,
        1,
        DM1_PC34_OPEN_FULL_HAND_CHEST_A_CELL_X,
        DM1_PC34_OPEN_FULL_HAND_CHEST_A_CELL_Y,
        DM1_PC34_OPEN_FULL_HAND_CHEST_B_CELL_X,
        DM1_PC34_OPEN_FULL_HAND_CHEST_B_CELL_Y,
        { DM1_PC34_CHEST_OPEN_FULL_HAND_HELMET_ITEM_TYPE,
          DM1_PC34_CHEST_OPEN_FULL_HAND_HELMET_WEIGHT,
          0,
          0,
          1,
          DM1_PC34_ALLOWED_HEAD | DM1_PC34_ALLOWED_CONTAINER },
        { DM1_PC34_OPEN_FULL_HAND_CHEST_A_FIRST_ITEM,
          DM1_PC34_OPEN_FULL_HAND_CHEST_A_FIRST_WEIGHT,
          0,
          0,
          1,
          DM1_PC34_ALLOWED_CONTAINER },
        { DM1_PC34_OPEN_FULL_HAND_CHEST_B_FIRST_ITEM,
          DM1_PC34_OPEN_FULL_HAND_CHEST_B_FIRST_WEIGHT,
          0,
          0,
          1,
          DM1_PC34_ALLOWED_CONTAINER }
    };

static M11_Item make_item(int itemType, int weight, int allowedSlots)
{
    M11_Item item;

    memset(&item, 0, sizeof(item));
    item.itemType = itemType;
    item.weight = weight;
    item.charges = 0;
    item.cursed = 0;
    item.identified = 1;
    item.allowedSlots = allowedSlots;
    return item;
}

static int copy_open_chest_items(const M11_InventoryState* state,
                                 int champ,
                                 int* typesOut,
                                 int* weightsOut)
{
    int i;

    if (!state || !typesOut || !weightsOut) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_OPEN_FULL_HAND_SLOT_COUNT; ++i) {
        M11_Item item;

        if (!m11_inventory_get_item_in_chest_slot(state, champ, i, &item)) {
            return 0;
        }
        typesOut[i] = item.itemType;
        weightsOut[i] = item.weight;
    }
    return 1;
}

static int item_list_head_type(const M11_Item* items, int count)
{
    if (!items || count <= 0) {
        return 0;
    }
    return items[0].itemType;
}

static int item_list_tail_type(const M11_Item* items, int count)
{
    int i;

    if (!items || count <= 0) {
        return 0;
    }
    for (i = count - 1; i >= 0; --i) {
        if (items[i].itemType != 0) {
            return items[i].itemType;
        }
    }
    return 0;
}

static int container_weight_from_closed_links(const M11_Item* items, int count)
{
    int total = DM1_PC34_CHEST_OPEN_FULL_HAND_CONTAINER_BASE_WEIGHT;
    int i;

    if (!items || count <= 0) {
        return total;
    }
    for (i = 0; i < count; ++i) {
        if (items[i].itemType != 0) {
            total += items[i].weight;
        }
    }
    return total;
}

static int contains_type(const int* types, int count, int itemType)
{
    int i;

    if (!types || count < 0) {
        return 0;
    }
    for (i = 0; i < count && i < DM1_PC34_CHEST_OPEN_FULL_HAND_SLOT_COUNT; ++i) {
        if (types[i] == itemType) {
            return 1;
        }
    }
    return 0;
}

const char* M11_GameView_ChestOpenWithFullLeaderHandSourceEvidencePc34(void)
{
    return s_source_evidence;
}

const DM1_V1_ChestOpenFullLeaderHandSpecPc34*
M11_GameView_ChestOpenWithFullLeaderHandSpecPc34(void)
{
    return &dm1_v1_chest_open_with_full_leader_hand_pc34_spec;
}

int M11_GameView_ChestOpenWithFullLeaderHandRuntimeGatePc34(
    DM1_V1_ChestOpenFullLeaderHandProbePc34* out)
{
    M11_InventoryState state;
    M11_InventoryState state2;
    M11_Item chestAClosed[1];
    M11_Item chestBClosed[1];
    M11_Item case2ChestAClosed[2];
    M11_Item case2ChestBClosed[1];
    M11_Item case2PreviousChestAClosed[DM1_PC34_CHEST_OPEN_FULL_HAND_SLOT_COUNT];
    M11_Item item;
    int case2Weights[DM1_PC34_CHEST_OPEN_FULL_HAND_SLOT_COUNT];

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    memset(case2PreviousChestAClosed, 0, sizeof(case2PreviousChestAClosed));

    out->sourceLockedContractOnly = 1;
    out->partyChampionCount = 1;
    out->chestAThing = DM1_PC34_OPEN_FULL_HAND_CHEST_A_THING;
    out->chestBThing = DM1_PC34_OPEN_FULL_HAND_CHEST_B_THING;
    out->chestACellX = DM1_PC34_OPEN_FULL_HAND_CHEST_A_CELL_X;
    out->chestACellY = DM1_PC34_OPEN_FULL_HAND_CHEST_A_CELL_Y;
    out->chestBCellX = DM1_PC34_OPEN_FULL_HAND_CHEST_B_CELL_X;
    out->chestBCellY = DM1_PC34_OPEN_FULL_HAND_CHEST_B_CELL_Y;

    chestAClosed[0] = make_item(DM1_PC34_OPEN_FULL_HAND_CHEST_A_FIRST_ITEM,
                                DM1_PC34_OPEN_FULL_HAND_CHEST_A_FIRST_WEIGHT,
                                DM1_PC34_ALLOWED_CONTAINER);
    chestBClosed[0] = make_item(DM1_PC34_OPEN_FULL_HAND_CHEST_B_FIRST_ITEM,
                                DM1_PC34_OPEN_FULL_HAND_CHEST_B_FIRST_WEIGHT,
                                DM1_PC34_ALLOWED_CONTAINER);
    out->chestAClosedHeadBefore = item_list_head_type(chestAClosed, 1);
    out->chestAClosedTailBefore = item_list_tail_type(chestAClosed, 1);
    out->chestAClosedWeightBefore =
        container_weight_from_closed_links(chestAClosed, 1);
    out->chestBClosedHeadBefore = item_list_head_type(chestBClosed, 1);
    out->chestBClosedTailBefore = item_list_tail_type(chestBClosed, 1);
    out->chestBClosedWeightBefore =
        container_weight_from_closed_links(chestBClosed, 1);

    m11_inventory_init(&state, out->partyChampionCount);
    out->leaderHandSetupResult = m11_inventory_set_mouse_item(
        &state, 0, DM1_PC34_CHEST_OPEN_FULL_HAND_HELMET_ITEM_TYPE,
        DM1_PC34_CHEST_OPEN_FULL_HAND_HELMET_WEIGHT, 0,
        DM1_PC34_ALLOWED_HEAD | DM1_PC34_ALLOWED_CONTAINER);
    if (!out->leaderHandSetupResult ||
        !m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    out->leaderHandBeforeOpenType = item.itemType;
    out->leaderHandBeforeOpenWeight = item.weight;
    out->leaderHandBeforeOpenAllowedSlots = item.allowedSlots;

    /* ReDMCSB CHEST.C F0333 lines 43 and 53-67 opens chest B by writing
     * G0426_T_OpenChest and copying its linked list into G0425. The leader
     * hand is outside that path; CHAMPION.C F0302 lines 688-710 swaps it only
     * after an explicit slot-box click. */
    out->chestBOpenResult = m11_inventory_open_chest(
        &state, 0, out->chestBThing, chestBClosed, 1);
    out->chestBOpenThing = m11_inventory_get_open_chest_thing(&state, 0);
    if (!out->chestBOpenResult ||
        !copy_open_chest_items(&state, 0, out->chestBOpenTypes,
                               out->chestBOpenWeights) ||
        !m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    out->leaderHandAfterChestBOpenType = item.itemType;
    out->leaderHandAfterChestBOpenWeight = item.weight;
    out->leaderHandAfterChestBOpenAllowedSlots = item.allowedSlots;
    out->leaderHandEvictedByChestBOpen = item.itemType == 0 ? 1 : 0;
    out->chestBVisibleWeightAfterOpen =
        m11_inventory_pc34_open_chest_visible_contents_weight(&state, 0);
    out->chestBContainerWeightAfterOpen =
        m11_inventory_pc34_open_chest_container_weight(&state, 0);
    out->chestBC537TypeAfterOpen =
        out->chestBOpenTypes[DM1_PC34_CHEST_OPEN_FULL_HAND_C537_INDEX];
    out->chestBC540TypeAfterOpen =
        out->chestBOpenTypes[DM1_PC34_CHEST_OPEN_FULL_HAND_C540_INDEX];
    out->chestBContainsLeaderHelmetAfterOpen =
        contains_type(out->chestBOpenTypes,
                      DM1_PC34_CHEST_OPEN_FULL_HAND_SLOT_COUNT,
                      DM1_PC34_CHEST_OPEN_FULL_HAND_HELMET_ITEM_TYPE);
    out->leaderHandDuplicatedIntoC540ByChestBOpen =
        out->chestBC540TypeAfterOpen ==
        DM1_PC34_CHEST_OPEN_FULL_HAND_HELMET_ITEM_TYPE ? 1 : 0;
    out->chestBFirstItemInLeaderHandAfterOpen =
        out->leaderHandAfterChestBOpenType ==
        DM1_PC34_OPEN_FULL_HAND_CHEST_B_FIRST_ITEM ? 1 : 0;

    out->chestAClosedHeadAfterChestBOpen = item_list_head_type(chestAClosed, 1);
    out->chestAClosedTailAfterChestBOpen = item_list_tail_type(chestAClosed, 1);
    out->chestAClosedWeightAfterChestBOpen =
        container_weight_from_closed_links(chestAClosed, 1);
    out->chestAClosedStateIntactAfterChestBOpen =
        out->chestAClosedHeadAfterChestBOpen == out->chestAClosedHeadBefore &&
        out->chestAClosedTailAfterChestBOpen == out->chestAClosedTailBefore &&
        out->chestAClosedWeightAfterChestBOpen == out->chestAClosedWeightBefore ?
        1 : 0;
    out->chestBClosedHeadAfterOpen = item_list_head_type(chestBClosed, 1);
    out->chestBClosedTailAfterOpen = item_list_tail_type(chestBClosed, 1);
    out->chestBClosedWeightAfterOpen =
        container_weight_from_closed_links(chestBClosed, 1);
    out->chestBClosedStateIntactAfterOpen =
        out->chestBClosedHeadAfterOpen == out->chestBClosedHeadBefore &&
        out->chestBClosedTailAfterOpen == out->chestBClosedTailBefore &&
        out->chestBClosedWeightAfterOpen == out->chestBClosedWeightBefore ?
        1 : 0;

    m11_inventory_init(&state2, out->partyChampionCount);
    out->case2LeaderHandSetupResult = m11_inventory_set_mouse_item(
        &state2, 0, DM1_PC34_CHEST_OPEN_FULL_HAND_HELMET_ITEM_TYPE,
        DM1_PC34_CHEST_OPEN_FULL_HAND_HELMET_WEIGHT, 0,
        DM1_PC34_ALLOWED_HEAD | DM1_PC34_ALLOWED_CONTAINER);
    if (!out->case2LeaderHandSetupResult ||
        !m11_inventory_get_mouse_item(&state2, 0, &item)) {
        return 0;
    }
    out->case2LeaderBeforeChestAOpen = item.itemType;
    case2ChestAClosed[0] = make_item(DM1_PC34_OPEN_FULL_HAND_CHEST_A_FIRST_ITEM,
                                     DM1_PC34_OPEN_FULL_HAND_CHEST_A_FIRST_WEIGHT,
                                     DM1_PC34_ALLOWED_CONTAINER);
    case2ChestAClosed[1] =
        make_item(DM1_PC34_OPEN_FULL_HAND_CHEST_A_FIRST_ITEM + 1,
                  DM1_PC34_OPEN_FULL_HAND_CHEST_A_FIRST_WEIGHT + 3,
                  DM1_PC34_ALLOWED_CONTAINER);
    case2ChestBClosed[0] = make_item(DM1_PC34_OPEN_FULL_HAND_CHEST_B_FIRST_ITEM,
                                     DM1_PC34_OPEN_FULL_HAND_CHEST_B_FIRST_WEIGHT,
                                     DM1_PC34_ALLOWED_CONTAINER);

    /* ReDMCSB CHEST.C F0333 lines 53-67 materializes chest A while the full
     * leader hand remains untouched because no F0302 slot-box click occurs. */
    out->case2ChestAOpenResult = m11_inventory_open_chest(
        &state2, 0, out->chestAThing, case2ChestAClosed, 2);
    out->case2ChestAOpenThing = m11_inventory_get_open_chest_thing(&state2, 0);
    if (!out->case2ChestAOpenResult ||
        !copy_open_chest_items(&state2, 0, out->case2ChestAOpenTypes,
                               case2Weights) ||
        !m11_inventory_get_mouse_item(&state2, 0, &item)) {
        return 0;
    }
    out->case2LeaderAfterChestAOpen = item.itemType;
    out->case2ChestAContainsLeaderHelmetAfterOpen =
        contains_type(out->case2ChestAOpenTypes,
                      DM1_PC34_CHEST_OPEN_FULL_HAND_SLOT_COUNT,
                      DM1_PC34_CHEST_OPEN_FULL_HAND_HELMET_ITEM_TYPE);
    out->case2ChestAC537TypeAfterOpen =
        out->case2ChestAOpenTypes[DM1_PC34_CHEST_OPEN_FULL_HAND_C537_INDEX];
    out->case2ChestAC538TypeAfterOpen =
        out->case2ChestAOpenTypes[DM1_PC34_CHEST_OPEN_FULL_HAND_C538_INDEX];
    out->case2ChestAVisibleWeightAfterOpen =
        m11_inventory_pc34_open_chest_visible_contents_weight(&state2, 0);
    out->case2ChestAContainerWeightAfterOpen =
        m11_inventory_pc34_open_chest_container_weight(&state2, 0);

    /* ReDMCSB CHEST.C F0333 lines 34-38 closes a different current chest
     * before lines 53-67 repopulate G0425 from chest B. */
    out->case2OpenChestBReplacingReturn =
        m11_inventory_open_chest_replacing_current(
            &state2, 0, out->chestBThing, case2ChestBClosed, 1,
            case2PreviousChestAClosed,
            DM1_PC34_CHEST_OPEN_FULL_HAND_SLOT_COUNT);
    if (out->case2OpenChestBReplacingReturn < 0 ||
        !copy_open_chest_items(&state2, 0, out->case2ChestBOpenTypes,
                               case2Weights) ||
        !m11_inventory_get_mouse_item(&state2, 0, &item)) {
        return 0;
    }
    out->case2ChestBOpenThing = m11_inventory_get_open_chest_thing(&state2, 0);
    out->case2LeaderAfterChestBOpen = item.itemType;
    out->case2PreviousChestAClosedCount =
        out->case2OpenChestBReplacingReturn;
    out->case2PreviousChestAClosedHead =
        item_list_head_type(case2PreviousChestAClosed,
                            out->case2PreviousChestAClosedCount);
    out->case2PreviousChestAClosedTail =
        item_list_tail_type(case2PreviousChestAClosed,
                            out->case2PreviousChestAClosedCount);
    out->case2PreviousChestAClosedWeight =
        container_weight_from_closed_links(
            case2PreviousChestAClosed,
            out->case2PreviousChestAClosedCount);
    out->case2ChestBVisibleWeightAfterOpen =
        m11_inventory_pc34_open_chest_visible_contents_weight(&state2, 0);
    out->case2ChestBContainerWeightAfterOpen =
        m11_inventory_pc34_open_chest_container_weight(&state2, 0);
    out->case2ChestBC537TypeAfterOpen =
        out->case2ChestBOpenTypes[DM1_PC34_CHEST_OPEN_FULL_HAND_C537_INDEX];
    out->case2ChestBC538TypeAfterOpen =
        out->case2ChestBOpenTypes[DM1_PC34_CHEST_OPEN_FULL_HAND_C538_INDEX];
    out->case2ChestBC540TypeAfterOpen =
        out->case2ChestBOpenTypes[DM1_PC34_CHEST_OPEN_FULL_HAND_C540_INDEX];
    out->case2ChestBContainsChestAItem =
        contains_type(out->case2ChestBOpenTypes,
                      DM1_PC34_CHEST_OPEN_FULL_HAND_SLOT_COUNT,
                      DM1_PC34_OPEN_FULL_HAND_CHEST_A_FIRST_ITEM);
    out->case2ChestBContainsLeaderHelmet =
        contains_type(out->case2ChestBOpenTypes,
                      DM1_PC34_CHEST_OPEN_FULL_HAND_SLOT_COUNT,
                      DM1_PC34_CHEST_OPEN_FULL_HAND_HELMET_ITEM_TYPE);
    out->case2ChestBFirstItemInLeaderHand =
        out->case2LeaderAfterChestBOpen ==
        DM1_PC34_OPEN_FULL_HAND_CHEST_B_FIRST_ITEM ? 1 : 0;
    out->case2LeaderDuplicatedIntoC540 =
        out->case2ChestBC540TypeAfterOpen ==
        DM1_PC34_CHEST_OPEN_FULL_HAND_HELMET_ITEM_TYPE ? 1 : 0;

    return 1;
}
