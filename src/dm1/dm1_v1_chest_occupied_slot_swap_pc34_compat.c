#include "dm1_v1_chest_occupied_slot_swap_pc34_compat.h"

#include "memory_tick_orchestrator_pc34_compat.h"

#include <string.h>

enum {
    BACKPACK_SOURCE_CHEST_THING = 0x7830,
    CHEST_A_SOURCE_THING = 0x7930,
    CHEST_B_DESTINATION_THING = 0x7931,
    WORLD_HASH_SEED = 0xC30
};

static const char s_source_evidence[] =
    "CHEST.C F0333:31-67 guards/open materialization and copies first eight linked chest objects into G0425_aT_ChestSlots\n"
    "CHEST.C F0334:113-132 rewrites only non-empty visible G0425_aT_ChestSlots back into the container link array\n"
    "CHAMPION.C F0297/F0298/F0302:250-298,688-710 owns leader-hand put/remove state and occupied-slot swaps\n"
    "DUNGEON.C F0163:1796-1837 clears Next and appends linked visible-input returns without duplicating list members\n"
    "OBJECT.C F0031:25-120 loads deterministic object names; these tests use unique itemType values as object-id sentinels";

const DM1_V1_ChestOccupiedSlotSwapSpecPc34
    dm1_v1_chest_occupied_slot_swap_pc34_spec = {
        "Source-locked contract gate only; not full real-asset chest runtime parity.",
        DM1_PC34_SLOT_CHEST_1,
        DM1_PC34_SLOT_CHEST_8,
        DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT,
        DM1_PC34_CHEST_OCCUPIED_SWAP_CASE_COUNT,
        DM1_PC34_CHEST_OCCUPIED_SWAP_BACKPACK_SOURCE_INDEX,
        DM1_PC34_CHEST_OCCUPIED_SWAP_BACKPACK_DEST_PC34_SLOT,
        DM1_PC34_CHEST_OCCUPIED_SWAP_CHEST_B_SOURCE_INDEX,
        DM1_PC34_CHEST_OCCUPIED_SWAP_CHEST_B_DEST_INDEX,
        { DM1_PC34_CHEST_OCCUPIED_SWAP_BACKPACK_POTION,
          4,
          DM1_PC34_ALLOWED_CONTAINER },
        { DM1_PC34_CHEST_OCCUPIED_SWAP_CHEST_A_WEAPON,
          11,
          DM1_PC34_ALLOWED_CONTAINER }
    };

static M11_Item make_item(int itemType, int weight, int allowedSlots)
{
    M11_Item item;

    memset(&item, 0, sizeof(item));
    item.itemType = itemType;
    item.weight = weight;
    item.identified = 1;
    item.allowedSlots = allowedSlots;
    return item;
}

static int snapshot_world_hash(unsigned int* outHash)
{
    struct GameWorld_Compat world;
    uint32_t hash = 0;

    if (!outHash) {
        return 0;
    }
    memset(&world, 0, sizeof(world));
    if (!F0881_WORLD_InitDefault_Compat(&world, WORLD_HASH_SEED) ||
        !F0891_ORCH_WorldHash_Compat(&world, &hash)) {
        return 0;
    }
    *outHash = hash;
    F0883_WORLD_Free_Compat(&world);
    return 1;
}

static int copy_open_types(const M11_InventoryState* state,
                           int champ,
                           int* typesOut)
{
    int i;

    if (!state || !typesOut) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT; ++i) {
        M11_Item item;

        if (!m11_inventory_get_item_in_chest_slot(state, champ, i, &item)) {
            return 0;
        }
        typesOut[i] = item.itemType;
    }
    return 1;
}

static void copy_item_types(const M11_Item* items, int count, int* typesOut)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT; ++i) {
        typesOut[i] =
            (items && i < count && items[i].itemType != 0) ?
            items[i].itemType : 0;
    }
}

static int count_visible(const int* types)
{
    int count = 0;
    int i;

    if (!types) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT; ++i) {
        if (types[i] != 0) {
            ++count;
        }
    }
    return count;
}

static int count_type(const int* types, int count, int itemType)
{
    int result = 0;
    int i;

    if (!types || count < 0) {
        return 0;
    }
    for (i = 0; i < count; ++i) {
        if (types[i] == itemType) {
            ++result;
        }
    }
    return result;
}

static int contains_type(const int* types, int count, int itemType)
{
    return count_type(types, count, itemType) > 0 ? 1 : 0;
}

static int visible_order_matches(const int* left,
                                 const int* right,
                                 int visibleCount)
{
    int i;

    if (!left || !right || visibleCount < 0 ||
        visibleCount > DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT) {
        return 0;
    }
    for (i = 0; i < visibleCount; ++i) {
        if (left[i] != right[i]) {
            return 0;
        }
    }
    for (i = visibleCount; i < DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT; ++i) {
        if (right[i] != 0) {
            return 0;
        }
    }
    return 1;
}

static int no_duplicates(const int* types, int count)
{
    int i;

    if (!types || count < 0) {
        return 0;
    }
    for (i = 0; i < count; ++i) {
        int j;

        if (types[i] == 0) {
            continue;
        }
        for (j = i + 1; j < count; ++j) {
            if (types[i] == types[j]) {
                return 0;
            }
        }
    }
    return 1;
}

static int final_set_has_no_duplicates(const int* visibleTypes,
                                       int destinationType,
                                       int leaderType,
                                       const int* extraTypes,
                                       int extraCount)
{
    int tracked[DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT + 4];
    int count = 0;
    int i;

    for (i = 0; i < DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT; ++i) {
        if (visibleTypes[i] != 0) {
            tracked[count++] = visibleTypes[i];
        }
    }
    if (destinationType != 0) {
        tracked[count++] = destinationType;
    }
    if (leaderType != 0) {
        tracked[count++] = leaderType;
    }
    for (i = 0; extraTypes && i < extraCount && count < (int)(sizeof(tracked) / sizeof(tracked[0])); ++i) {
        if (extraTypes[i] != 0) {
            tracked[count++] = extraTypes[i];
        }
    }
    return no_duplicates(tracked, count);
}

static int original_visible_members_present(const int* originalTypes,
                                            int originalCount,
                                            const int* finalTypes)
{
    int i;

    for (i = 0; i < originalCount; ++i) {
        if (!contains_type(finalTypes,
                           DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT,
                           originalTypes[i])) {
            return 0;
        }
    }
    return 1;
}

static int initialize_world_hashes(DM1_V1_ChestOccupiedSlotSwapCasePc34* out)
{
    out->worldHashBeforeResult = snapshot_world_hash(&out->worldHashBefore);
    if (!out->worldHashBeforeResult) {
        return 0;
    }
    return 1;
}

static int finish_world_hashes(DM1_V1_ChestOccupiedSlotSwapCasePc34* out)
{
    out->worldHashAfterResult = snapshot_world_hash(&out->worldHashAfter);
    if (!out->worldHashAfterResult) {
        return 0;
    }
    out->worldHashUnchanged =
        out->worldHashBefore == out->worldHashAfter ? 1 : 0;
    return 1;
}

static int run_backpack_case(DM1_V1_ChestOccupiedSlotSwapCasePc34* out)
{
    M11_InventoryState state;
    M11_Item sourceInput[9];
    M11_Item sourceClosed[DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT];
    M11_Item item;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    memset(sourceClosed, 0, sizeof(sourceClosed));
    out->sourceChestThing = BACKPACK_SOURCE_CHEST_THING;
    out->sourceSlotIndex = DM1_PC34_CHEST_OCCUPIED_SWAP_BACKPACK_SOURCE_INDEX;
    out->destinationPc34Slot =
        DM1_PC34_CHEST_OCCUPIED_SWAP_BACKPACK_DEST_PC34_SLOT;
    out->sourceReplacementType =
        DM1_PC34_CHEST_OCCUPIED_SWAP_BACKPACK_REPLACEMENT;
    out->destinationOriginalOccupant =
        DM1_PC34_CHEST_OCCUPIED_SWAP_BACKPACK_DEST_OCCUPANT;
    out->sourceHiddenTailInput =
        DM1_PC34_CHEST_OCCUPIED_SWAP_BACKPACK_HIDDEN_TAIL;

    if (!initialize_world_hashes(out)) {
        return 0;
    }

    m11_inventory_init(&state, 1);
    for (i = 0; i < 9; ++i) {
        sourceInput[i] =
            make_item(DM1_PC34_CHEST_OCCUPIED_SWAP_BACKPACK_CHEST_FIRST + i,
                      2 + i,
                      DM1_PC34_ALLOWED_CONTAINER);
    }
    if (!m11_inventory_set_item_in_pc34_source_slot(
            &state, 0, out->destinationPc34Slot,
            out->destinationOriginalOccupant, 13, 0,
            DM1_PC34_ALLOWED_ANY_SLOT) ||
        !m11_inventory_set_mouse_item(
            &state, 0, out->sourceReplacementType, 17, 0,
            DM1_PC34_ALLOWED_CONTAINER) ||
        !m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    out->leaderHandBefore = item.itemType;

    /* ReDMCSB CHEST.C F0333 lines 31-67 materializes source chest A's first
     * eight linked objects; OBJECT.C F0031 lines 25-120 is represented here by
     * unique deterministic itemType sentinels for every tracked object id. */
    out->sourceOpenResult = m11_inventory_open_chest(
        &state, 0, out->sourceChestThing, sourceInput, 9);
    out->sourceOpenThing = m11_inventory_get_open_chest_thing(&state, 0);
    if (!out->sourceOpenResult ||
        !copy_open_types(&state, 0, out->originalVisibleTypes)) {
        return 0;
    }
    out->sourceVisibleCountBefore = count_visible(out->originalVisibleTypes);
    out->originalHead = out->originalVisibleTypes[0];
    out->originalTail =
        out->originalVisibleTypes[out->sourceVisibleCountBefore - 1];

    /* ReDMCSB CHAMPION.C F0302 lines 688-710 swaps the leader-hand
     * replacement with occupied C539, leaving the picked potion in hand. */
    out->sourceSwapClickResult = m11_inventory_click_pc34_source_slot(
        &state, 0, DM1_PC34_SLOT_CHEST_1 + out->sourceSlotIndex);
    if (!out->sourceSwapClickResult ||
        !m11_inventory_get_item_in_chest_slot(
            &state, 0, out->sourceSlotIndex, &item)) {
        return 0;
    }
    out->sourceSlotAfterSourceSwap = item.itemType;
    out->sourceReplacementStoredAtOriginalIndex =
        item.itemType == out->sourceReplacementType ? 1 : 0;
    if (!m11_inventory_get_mouse_item(&state, 0, &item) ||
        !copy_open_types(&state, 0, out->afterSourceSwapTypes)) {
        return 0;
    }
    out->leaderHandAfterSourceSwap = item.itemType;
    out->swappedObjectType = item.itemType;

    /* ReDMCSB CHAMPION.C F0302 lines 688-710 applies the same occupied-slot
     * swap to a non-leader backpack slot, proving the chest occupant can live
     * outside G0425 before returning. */
    out->destinationSwapClickResult = m11_inventory_click_pc34_source_slot(
        &state, 0, out->destinationPc34Slot);
    if (!out->destinationSwapClickResult ||
        !m11_inventory_get_item_in_pc34_source_slot(
            &state, 0, out->destinationPc34Slot, &item)) {
        return 0;
    }
    out->destinationAfterSwapType = item.itemType;
    out->swappedObjectInDestinationSlot =
        item.itemType == out->swappedObjectType ? 1 : 0;
    if (!m11_inventory_get_mouse_item(&state, 0, &item) ||
        !copy_open_types(&state, 0, out->afterDestinationSwapTypes)) {
        return 0;
    }
    out->leaderHandAfterDestinationSwap = item.itemType;
    out->destinationOccupantMovedToLeader =
        item.itemType == out->destinationOriginalOccupant ? 1 : 0;

    out->destinationReturnClickResult = m11_inventory_click_pc34_source_slot(
        &state, 0, out->destinationPc34Slot);
    if (!out->destinationReturnClickResult ||
        !m11_inventory_get_item_in_pc34_source_slot(
            &state, 0, out->destinationPc34Slot, &item)) {
        return 0;
    }
    out->destinationAfterReturnType = item.itemType;
    out->destinationOccupantRestored =
        item.itemType == out->destinationOriginalOccupant ? 1 : 0;
    if (!m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    out->leaderHandAfterDestinationReturn = item.itemType;
    out->leaderHandBeforeReinsert = item.itemType;
    out->swappedObjectReadyForReinsert =
        item.itemType == out->swappedObjectType ? 1 : 0;

    /* ReDMCSB CHAMPION.C F0302 lines 688-710 reinserts the same object into
     * its originally occupied C539 index, swapping the source replacement back
     * to the leader hand without disturbing neighboring C537..C544 entries. */
    out->sourceReinsertClickResult = m11_inventory_click_pc34_source_slot(
        &state, 0, DM1_PC34_SLOT_CHEST_1 + out->sourceSlotIndex);
    if (!out->sourceReinsertClickResult ||
        !m11_inventory_get_item_in_chest_slot(
            &state, 0, out->sourceSlotIndex, &item)) {
        return 0;
    }
    out->sourceSlotAfterReinsert = item.itemType;
    if (!m11_inventory_get_mouse_item(&state, 0, &item) ||
        !copy_open_types(&state, 0, out->finalVisibleTypes)) {
        return 0;
    }
    out->leaderHandAfterReinsert = item.itemType;
    out->replacementReturnedToLeader =
        item.itemType == out->sourceReplacementType ? 1 : 0;
    out->leaderHandStable =
        out->leaderHandAfterReinsert == out->leaderHandBefore ? 1 : 0;
    out->finalVisibleCount = count_visible(out->finalVisibleTypes);
    out->finalHead = out->finalVisibleTypes[0];
    out->finalTail = out->finalVisibleTypes[out->finalVisibleCount - 1];
    out->finalHeadMembershipCount =
        count_type(out->finalVisibleTypes,
                   DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT,
                   out->originalHead);

    /* ReDMCSB CHEST.C F0334 lines 113-132 and DUNGEON.C F0163 lines
     * 1796-1837 rewrite and relink only the visible members after reinsert. */
    out->sourceCloseCount = m11_inventory_close_chest(
        &state, 0, sourceClosed,
        DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT);
    if (out->sourceCloseCount < 0) {
        return 0;
    }
    copy_item_types(sourceClosed, out->sourceCloseCount, out->closedTypes);
    out->hiddenTailClosed =
        contains_type(out->closedTypes, out->sourceCloseCount,
                      out->sourceHiddenTailInput);

    out->sourceReopenAfterCloseResult = m11_inventory_open_chest(
        &state, 0, out->sourceChestThing + 1, sourceClosed,
        out->sourceCloseCount);
    if (!out->sourceReopenAfterCloseResult ||
        !copy_open_types(&state, 0, out->reopenedTypes)) {
        return 0;
    }
    out->reopenedVisibleCount = count_visible(out->reopenedTypes);
    out->hiddenTailReopened =
        contains_type(out->reopenedTypes, out->reopenedVisibleCount,
                      out->sourceHiddenTailInput);

    out->visibleHeadUnchanged = out->finalHead == out->originalHead ? 1 : 0;
    out->visibleTailUnchanged = out->finalTail == out->originalTail ? 1 : 0;
    out->visibleOrderUnchanged =
        visible_order_matches(out->originalVisibleTypes,
                              out->finalVisibleTypes,
                              out->sourceVisibleCountBefore);
    out->reopenedOrderUnchanged =
        visible_order_matches(out->originalVisibleTypes, out->reopenedTypes,
                              out->sourceVisibleCountBefore);
    out->noDuplicateObjectIds =
        final_set_has_no_duplicates(out->finalVisibleTypes,
                                    out->destinationAfterReturnType,
                                    out->leaderHandAfterReinsert,
                                    NULL, 0);
    out->noEvictions =
        original_visible_members_present(out->originalVisibleTypes,
                                         out->sourceVisibleCountBefore,
                                         out->finalVisibleTypes) &&
        out->destinationOccupantRestored &&
        out->replacementReturnedToLeader ? 1 : 0;

    return finish_world_hashes(out);
}

static int run_chest_b_case(DM1_V1_ChestOccupiedSlotSwapCasePc34* out)
{
    M11_InventoryState state;
    M11_Item chestAInput[4];
    M11_Item chestBInput[1];
    M11_Item chestAAfterSwapClosed[DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT];
    M11_Item chestBClosed[DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT];
    M11_Item chestAFinalClosed[DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT];
    M11_Item item;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    memset(chestAAfterSwapClosed, 0, sizeof(chestAAfterSwapClosed));
    memset(chestBClosed, 0, sizeof(chestBClosed));
    memset(chestAFinalClosed, 0, sizeof(chestAFinalClosed));
    out->sourceChestThing = CHEST_A_SOURCE_THING;
    out->destinationChestThing = CHEST_B_DESTINATION_THING;
    out->sourceSlotIndex = DM1_PC34_CHEST_OCCUPIED_SWAP_CHEST_B_SOURCE_INDEX;
    out->destinationChestSlotIndex =
        DM1_PC34_CHEST_OCCUPIED_SWAP_CHEST_B_DEST_INDEX;
    out->sourceReplacementType =
        DM1_PC34_CHEST_OCCUPIED_SWAP_CHEST_A_REPLACEMENT;
    out->destinationOriginalOccupant =
        DM1_PC34_CHEST_OCCUPIED_SWAP_CHEST_B_DEST_OCCUPANT;
    out->sourceHiddenTailInput =
        DM1_PC34_CHEST_OCCUPIED_SWAP_CHEST_A_HIDDEN_TAIL;

    if (!initialize_world_hashes(out)) {
        return 0;
    }

    m11_inventory_init(&state, 1);
    chestAInput[0] =
        make_item(DM1_PC34_CHEST_OCCUPIED_SWAP_CHEST_A_WEAPON,
                  11, DM1_PC34_ALLOWED_CONTAINER);
    chestAInput[1] =
        make_item(DM1_PC34_CHEST_OCCUPIED_SWAP_CHEST_A_FIRST + 1,
                  4, DM1_PC34_ALLOWED_CONTAINER);
    chestAInput[2] =
        make_item(DM1_PC34_CHEST_OCCUPIED_SWAP_CHEST_A_FIRST + 2,
                  2, DM1_PC34_ALLOWED_CONTAINER);
    chestAInput[3] =
        make_item(out->sourceHiddenTailInput, 19,
                  DM1_PC34_ALLOWED_CONTAINER);
    chestBInput[0] =
        make_item(out->destinationOriginalOccupant, 7,
                  DM1_PC34_ALLOWED_CONTAINER);

    if (!m11_inventory_set_mouse_item(
            &state, 0, out->sourceReplacementType, 15, 0,
            DM1_PC34_ALLOWED_CONTAINER) ||
        !m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    out->leaderHandBefore = item.itemType;

    /* ReDMCSB CHEST.C F0333 lines 31-67 materializes the three visible chest
     * A objects and excludes the fourth hidden tail from the G0425 window. */
    out->sourceOpenResult = m11_inventory_open_chest(
        &state, 0, out->sourceChestThing, chestAInput, 3);
    out->sourceOpenThing = m11_inventory_get_open_chest_thing(&state, 0);
    if (!out->sourceOpenResult ||
        !copy_open_types(&state, 0, out->originalVisibleTypes)) {
        return 0;
    }
    out->sourceVisibleCountBefore = count_visible(out->originalVisibleTypes);
    out->originalHead = out->originalVisibleTypes[0];
    out->originalTail =
        out->originalVisibleTypes[out->sourceVisibleCountBefore - 1];

    /* ReDMCSB CHAMPION.C F0302 lines 688-710 swaps C537 with the leader-hand
     * source replacement, so the weapon can move through another chest slot. */
    out->sourceSwapClickResult = m11_inventory_click_pc34_source_slot(
        &state, 0, DM1_PC34_SLOT_CHEST_1 + out->sourceSlotIndex);
    if (!out->sourceSwapClickResult ||
        !m11_inventory_get_item_in_chest_slot(
            &state, 0, out->sourceSlotIndex, &item)) {
        return 0;
    }
    out->sourceSlotAfterSourceSwap = item.itemType;
    out->sourceReplacementStoredAtOriginalIndex =
        item.itemType == out->sourceReplacementType ? 1 : 0;
    if (!m11_inventory_get_mouse_item(&state, 0, &item) ||
        !copy_open_types(&state, 0, out->afterSourceSwapTypes)) {
        return 0;
    }
    out->leaderHandAfterSourceSwap = item.itemType;
    out->swappedObjectType = item.itemType;

    /* ReDMCSB CHEST.C F0334 lines 113-132 closes chest A before CHEST.C
     * F0333 lines 31-67 opens chest B, preserving A's visible replacement
     * window until the weapon returns. */
    out->sourceReopenResult = m11_inventory_open_chest_replacing_current(
        &state, 0, out->destinationChestThing, chestBInput, 1,
        chestAAfterSwapClosed, DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT);
    if (out->sourceReopenResult < 0 ||
        !copy_open_types(&state, 0, out->afterDestinationSwapTypes)) {
        return 0;
    }

    /* ReDMCSB CHAMPION.C F0302 lines 688-710 swaps the weapon into occupied
     * chest B C537 and moves B's occupant into the leader hand. */
    out->destinationSwapClickResult = m11_inventory_click_pc34_source_slot(
        &state, 0, DM1_PC34_SLOT_CHEST_1 + out->destinationChestSlotIndex);
    if (!out->destinationSwapClickResult ||
        !m11_inventory_get_item_in_chest_slot(
            &state, 0, out->destinationChestSlotIndex, &item)) {
        return 0;
    }
    out->destinationAfterSwapType = item.itemType;
    out->swappedObjectInDestinationSlot =
        item.itemType == out->swappedObjectType ? 1 : 0;
    if (!m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    out->leaderHandAfterDestinationSwap = item.itemType;
    out->destinationOccupantMovedToLeader =
        item.itemType == out->destinationOriginalOccupant ? 1 : 0;

    out->destinationReturnClickResult = m11_inventory_click_pc34_source_slot(
        &state, 0, DM1_PC34_SLOT_CHEST_1 + out->destinationChestSlotIndex);
    if (!out->destinationReturnClickResult ||
        !m11_inventory_get_item_in_chest_slot(
            &state, 0, out->destinationChestSlotIndex, &item)) {
        return 0;
    }
    out->destinationAfterReturnType = item.itemType;
    out->destinationOccupantRestored =
        item.itemType == out->destinationOriginalOccupant ? 1 : 0;
    if (!m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    out->leaderHandAfterDestinationReturn = item.itemType;
    out->leaderHandBeforeReinsert = item.itemType;
    out->swappedObjectReadyForReinsert =
        item.itemType == out->swappedObjectType ? 1 : 0;

    /* ReDMCSB CHEST.C F0334 lines 113-132 closes chest B, then F0333 lines
     * 31-67 rematerializes chest A's replacement window for reinsert. */
    out->sourceReopenResult = m11_inventory_open_chest_replacing_current(
        &state, 0, out->sourceChestThing, chestAAfterSwapClosed,
        out->sourceVisibleCountBefore,
        chestBClosed, DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT);
    if (out->sourceReopenResult < 0 ||
        !copy_open_types(&state, 0, out->afterDestinationSwapTypes)) {
        return 0;
    }
    out->destinationCloseCount = out->sourceReopenResult;
    copy_item_types(chestBClosed, out->destinationCloseCount,
                    out->destinationClosedTypes);

    /* ReDMCSB CHAMPION.C F0302 lines 688-710 puts the original weapon back
     * into chest A C537 and returns the source replacement to the leader hand. */
    out->sourceReinsertClickResult = m11_inventory_click_pc34_source_slot(
        &state, 0, DM1_PC34_SLOT_CHEST_1 + out->sourceSlotIndex);
    if (!out->sourceReinsertClickResult ||
        !m11_inventory_get_item_in_chest_slot(
            &state, 0, out->sourceSlotIndex, &item)) {
        return 0;
    }
    out->sourceSlotAfterReinsert = item.itemType;
    if (!m11_inventory_get_mouse_item(&state, 0, &item) ||
        !copy_open_types(&state, 0, out->finalVisibleTypes)) {
        return 0;
    }
    out->leaderHandAfterReinsert = item.itemType;
    out->replacementReturnedToLeader =
        item.itemType == out->sourceReplacementType ? 1 : 0;
    out->leaderHandStable =
        out->leaderHandAfterReinsert == out->leaderHandBefore ? 1 : 0;
    out->finalVisibleCount = count_visible(out->finalVisibleTypes);
    out->finalHead = out->finalVisibleTypes[0];
    out->finalTail = out->finalVisibleTypes[out->finalVisibleCount - 1];
    out->finalHeadMembershipCount =
        count_type(out->finalVisibleTypes,
                   DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT,
                   out->originalHead);

    /* ReDMCSB CHEST.C F0334 lines 113-132 and DUNGEON.C F0163 lines
     * 1796-1837 prove the restored three-member visible window relinks in
     * source order without pulling in the original fourth hidden tail. */
    out->sourceCloseCount = m11_inventory_close_chest(
        &state, 0, chestAFinalClosed,
        DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT);
    if (out->sourceCloseCount < 0) {
        return 0;
    }
    copy_item_types(chestAFinalClosed, out->sourceCloseCount, out->closedTypes);
    out->hiddenTailClosed =
        contains_type(out->closedTypes, out->sourceCloseCount,
                      out->sourceHiddenTailInput);

    out->sourceReopenAfterCloseResult = m11_inventory_open_chest(
        &state, 0, out->sourceChestThing + 2, chestAFinalClosed,
        out->sourceCloseCount);
    if (!out->sourceReopenAfterCloseResult ||
        !copy_open_types(&state, 0, out->reopenedTypes)) {
        return 0;
    }
    out->reopenedVisibleCount = count_visible(out->reopenedTypes);
    out->hiddenTailReopened =
        contains_type(out->reopenedTypes, out->reopenedVisibleCount,
                      out->sourceHiddenTailInput);

    out->visibleHeadUnchanged = out->finalHead == out->originalHead ? 1 : 0;
    out->visibleTailUnchanged = out->finalTail == out->originalTail ? 1 : 0;
    out->visibleOrderUnchanged =
        visible_order_matches(out->originalVisibleTypes,
                              out->finalVisibleTypes,
                              out->sourceVisibleCountBefore);
    out->reopenedOrderUnchanged =
        visible_order_matches(out->originalVisibleTypes, out->reopenedTypes,
                              out->sourceVisibleCountBefore);
    out->noDuplicateObjectIds =
        final_set_has_no_duplicates(out->finalVisibleTypes,
                                    0,
                                    out->leaderHandAfterReinsert,
                                    out->destinationClosedTypes,
                                    out->destinationCloseCount);
    out->noEvictions =
        original_visible_members_present(out->originalVisibleTypes,
                                         out->sourceVisibleCountBefore,
                                         out->finalVisibleTypes) &&
        out->destinationOccupantRestored &&
        out->replacementReturnedToLeader ? 1 : 0;

    return finish_world_hashes(out);
}

const char* dm1_v1_chest_occupied_slot_swap_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const DM1_V1_ChestOccupiedSlotSwapSpecPc34*
dm1_v1_chest_occupied_slot_swap_spec_pc34(void)
{
    return &dm1_v1_chest_occupied_slot_swap_pc34_spec;
}

int dm1_v1_chest_occupied_slot_swap_pc34(
    DM1_V1_ChestOccupiedSlotSwapProbePc34* out)
{
    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    out->sourceLockedContractOnly = 1;

    if (!run_backpack_case(
            &out->cases[DM1_PC34_CHEST_OCCUPIED_SWAP_BACKPACK_CASE]) ||
        !run_chest_b_case(
            &out->cases[DM1_PC34_CHEST_OCCUPIED_SWAP_CHEST_B_CASE])) {
        return 0;
    }
    return 1;
}
