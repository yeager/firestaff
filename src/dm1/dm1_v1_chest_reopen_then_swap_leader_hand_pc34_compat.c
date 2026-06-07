#include "dm1_v1_chest_reopen_then_swap_leader_hand_pc34_compat.h"

#include "memory_tick_orchestrator_pc34_compat.h"

#include <string.h>

enum {
    WORLD_HASH_SEED = 0xC30
};

static const char s_source_evidence[] =
    "CHEST.C F0333:31-67 opens/reopens a chest and materializes only the first eight linked objects into G0425_aT_ChestSlots\n"
    "CHEST.C F0334:113-132 closes by rewriting only non-empty visible G0425_aT_ChestSlots back into the container link array\n"
    "CHAMPION.C F0297:250-298 puts an object into the leader hand and maintains leader load state\n"
    "CHAMPION.C F0298:263-285 removes the current leader-hand object before storage routing\n"
    "CHAMPION.C F0302:688-710 routes C30+ chest/backpack slots through G0425 and swaps or places the leader-hand object\n"
    "DUNGEON.C F0163:1796-1837 clears Next and appends visible-input returns without relinking the hidden tail";

const DM1_V1_ChestReopenThenSwapLeaderHandSpecPc34
    dm1_v1_chest_reopen_then_swap_leader_hand_pc34_spec = {
        "Source-locked contract gate only; not full real-asset chest runtime parity.",
        DM1_PC34_SLOT_CHEST_1,
        DM1_PC34_SLOT_CHEST_8,
        DM1_PC34_CHEST_REOPEN_SWAP_SLOT_COUNT,
        DM1_PC34_CHEST_REOPEN_SWAP_MAX_LINKED,
        DM1_PC34_CHEST_REOPEN_SWAP_B_DEST_INDEX,
        DM1_PC34_CHEST_REOPEN_SWAP_A_FIRST,
        DM1_PC34_CHEST_REOPEN_SWAP_A_HIDDEN_TAIL
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

static int copy_open_types(const M11_InventoryState* state, int* typesOut)
{
    int i;

    if (!state || !typesOut) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_REOPEN_SWAP_SLOT_COUNT; ++i) {
        M11_Item item;

        if (!m11_inventory_get_item_in_chest_slot(state, 0, i, &item)) {
            return 0;
        }
        typesOut[i] = item.itemType;
    }
    return 1;
}

static void copy_item_types(const M11_Item* items, int count, int* typesOut)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_REOPEN_SWAP_SLOT_COUNT; ++i) {
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
    for (i = 0; i < DM1_PC34_CHEST_REOPEN_SWAP_SLOT_COUNT; ++i) {
        if (types[i] != 0) {
            ++count;
        }
    }
    return count;
}

static int contains_type(const int* types, int count, int itemType)
{
    int i;

    if (!types || count < 0) {
        return 0;
    }
    for (i = 0;
         i < count && i < DM1_PC34_CHEST_REOPEN_SWAP_SLOT_COUNT;
         ++i) {
        if (types[i] == itemType) {
            return 1;
        }
    }
    return 0;
}

static int order_matches_visible_input(const int* types)
{
    int i;

    if (!types) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_REOPEN_SWAP_SLOT_COUNT; ++i) {
        if (types[i] != DM1_PC34_CHEST_REOPEN_SWAP_A_FIRST + i) {
            return 0;
        }
    }
    return 1;
}

static int orders_match(const int* left, const int* right)
{
    int i;

    if (!left || !right) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_REOPEN_SWAP_SLOT_COUNT; ++i) {
        if (left[i] != right[i]) {
            return 0;
        }
    }
    return 1;
}

const char*
M11_GameView_ChestReopenThenSwapLeaderHandSourceEvidencePc34(void)
{
    return s_source_evidence;
}

const DM1_V1_ChestReopenThenSwapLeaderHandSpecPc34*
M11_GameView_ChestReopenThenSwapLeaderHandSpecPc34(void)
{
    return &dm1_v1_chest_reopen_then_swap_leader_hand_pc34_spec;
}

int M11_GameView_ChestReopenThenSwapLeaderHandRunPc34(
    DM1_V1_ChestReopenThenSwapLeaderHandProbePc34* out)
{
    M11_InventoryState state;
    M11_Item chestAInput[DM1_PC34_CHEST_REOPEN_SWAP_MAX_LINKED];
    M11_Item chestAClosed[DM1_PC34_CHEST_REOPEN_SWAP_SLOT_COUNT];
    M11_Item chestAClosedWhileOpeningB[DM1_PC34_CHEST_REOPEN_SWAP_SLOT_COUNT];
    M11_Item item;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    memset(chestAClosed, 0, sizeof(chestAClosed));
    memset(chestAClosedWhileOpeningB, 0, sizeof(chestAClosedWhileOpeningB));

    out->contract_only = 1;
    out->chestAThing = DM1_PC34_CHEST_REOPEN_SWAP_A_THING;
    out->chestAReopenThing = DM1_PC34_CHEST_REOPEN_SWAP_A_REOPEN_THING;
    out->chestBThing = DM1_PC34_CHEST_REOPEN_SWAP_B_THING;

    out->worldHashBeforeResult = snapshot_world_hash(&out->worldHashBefore);
    if (!out->worldHashBeforeResult) {
        return 0;
    }

    m11_inventory_init(&state, 1);
    for (i = 0; i < DM1_PC34_CHEST_REOPEN_SWAP_MAX_LINKED; ++i) {
        chestAInput[i] =
            make_item(DM1_PC34_CHEST_REOPEN_SWAP_A_FIRST + i,
                      2 + i,
                      DM1_PC34_ALLOWED_CONTAINER);
    }

    /* ReDMCSB CHEST.C F0333 lines 31-67 materializes chest A's linked list
     * into C537..C544/G0425 and stops before the ninth hidden-tail object. */
    out->chestAOpenResult = m11_inventory_open_chest(
        &state, 0, out->chestAThing, chestAInput,
        DM1_PC34_CHEST_REOPEN_SWAP_MAX_LINKED);
    out->chestAOpenThing = m11_inventory_get_open_chest_thing(&state, 0);
    if (!out->chestAOpenResult ||
        !copy_open_types(&state, out->chestAOpenedTypes)) {
        return 0;
    }
    out->chestAOpenedVisibleCount = count_visible(out->chestAOpenedTypes);
    out->chestAOpenedOrderMatchesInput =
        order_matches_visible_input(out->chestAOpenedTypes);

    /* ReDMCSB CHAMPION.C F0297 lines 250-298 records the ninth linked object
     * in the leader hand; F0302 lines 688-710 later permits it in C30+. */
    item = chestAInput[DM1_PC34_CHEST_REOPEN_SWAP_SLOT_COUNT];
    out->leaderHandSetupResult =
        m11_inventory_set_mouse_item(&state, 0, item.itemType, item.weight,
                                     item.charges, item.allowedSlots);
    if (!out->leaderHandSetupResult ||
        !m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    out->leaderHandBeforeClose = item.itemType;
    out->leaderHandCanEnterChestB =
        m11_inventory_can_equip(&item, DM1_PC34_SLOT_CHEST_1);

    /* ReDMCSB CHEST.C F0334 lines 113-132 rewrites only visible G0425
     * members; DUNGEON.C F0163 lines 1796-1837 relinks those visible returns,
     * leaving the ninth object solely in the leader hand. */
    out->chestACloseCount = m11_inventory_close_chest(
        &state, 0, chestAClosed, DM1_PC34_CHEST_REOPEN_SWAP_SLOT_COUNT);
    if (out->chestACloseCount < 0 ||
        !m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    copy_item_types(chestAClosed, out->chestACloseCount,
                    out->chestAClosedTypes);
    out->chestAHiddenTailReturnedByClose =
        contains_type(out->chestAClosedTypes, out->chestACloseCount,
                      DM1_PC34_CHEST_REOPEN_SWAP_A_HIDDEN_TAIL);
    out->leaderHandAfterClose = item.itemType;
    out->leaderHandStableAcrossClose =
        item.itemType == DM1_PC34_CHEST_REOPEN_SWAP_A_HIDDEN_TAIL ? 1 : 0;

    out->worldHashAfterCloseResult =
        snapshot_world_hash(&out->worldHashAfterClose);
    if (!out->worldHashAfterCloseResult) {
        return 0;
    }
    out->chestAWorldHashStableAfterClose =
        out->worldHashBefore == out->worldHashAfterClose ? 1 : 0;

    /* ReDMCSB CHEST.C F0333 lines 31-67 reopens chest A from the F0334
     * visible-only rewrite, proving the hidden tail is not rematerialized. */
    out->chestAReopenResult = m11_inventory_open_chest(
        &state, 0, out->chestAReopenThing, chestAClosed,
        out->chestACloseCount);
    out->chestAReopenThing = m11_inventory_get_open_chest_thing(&state, 0);
    if (!out->chestAReopenResult ||
        !copy_open_types(&state, out->chestAReopenedTypes) ||
        !m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    out->chestAReopenedVisibleCount = count_visible(out->chestAReopenedTypes);
    out->chestAReopenedOrderMatchesClose =
        orders_match(out->chestAClosedTypes, out->chestAReopenedTypes);
    out->chestAHiddenTailVisibleAfterReopen =
        contains_type(out->chestAReopenedTypes,
                      out->chestAReopenedVisibleCount,
                      DM1_PC34_CHEST_REOPEN_SWAP_A_HIDDEN_TAIL);
    out->leaderHandAfterReopen = item.itemType;
    out->leaderHandStableAcrossReopen =
        item.itemType == DM1_PC34_CHEST_REOPEN_SWAP_A_HIDDEN_TAIL ? 1 : 0;

    /* ReDMCSB CHEST.C F0333 lines 34-39 closes reopened chest A through
     * F0334 lines 113-132 before opening different chest B for C30 routing. */
    out->chestACloseWhileOpeningBCount =
        m11_inventory_open_chest_replacing_current(
            &state, 0, out->chestBThing, NULL, 0,
            chestAClosedWhileOpeningB,
            DM1_PC34_CHEST_REOPEN_SWAP_SLOT_COUNT);
    if (out->chestACloseWhileOpeningBCount < 0 ||
        !m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    copy_item_types(chestAClosedWhileOpeningB,
                    out->chestACloseWhileOpeningBCount,
                    out->chestAClosedWhileOpeningBTypes);
    out->chestAHiddenTailReturnedWhileOpeningB =
        contains_type(out->chestAClosedWhileOpeningBTypes,
                      out->chestACloseWhileOpeningBCount,
                      DM1_PC34_CHEST_REOPEN_SWAP_A_HIDDEN_TAIL);
    out->chestBOpenThing = m11_inventory_get_open_chest_thing(&state, 0);

    /* ReDMCSB CHAMPION.C F0298 lines 263-285 removes the leader-hand object;
     * F0302 lines 688-710 adds it to B's empty C537/G0425 destination. */
    out->chestBPlaceClickResult = m11_inventory_click_pc34_source_slot(
        &state, 0,
        DM1_PC34_SLOT_CHEST_1 + DM1_PC34_CHEST_REOPEN_SWAP_B_DEST_INDEX);
    if (!out->chestBPlaceClickResult ||
        !m11_inventory_get_item_in_chest_slot(
            &state, 0, DM1_PC34_CHEST_REOPEN_SWAP_B_DEST_INDEX, &item)) {
        return 0;
    }
    out->chestBDestinationAfterPlace = item.itemType;
    out->hiddenTailStoredInChestB =
        item.itemType == DM1_PC34_CHEST_REOPEN_SWAP_A_HIDDEN_TAIL ? 1 : 0;
    if (!m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    out->leaderHandAfterPlace = item.itemType;
    out->leaderHandEmptyAfterPlace = item.itemType == 0 ? 1 : 0;
    out->chestAHiddenTailStillEmpty =
        !out->chestAHiddenTailReturnedByClose &&
        !out->chestAHiddenTailVisibleAfterReopen &&
        !out->chestAHiddenTailReturnedWhileOpeningB ? 1 : 0;

    out->worldHashAfterFinalResult =
        snapshot_world_hash(&out->worldHashAfterFinal);
    if (!out->worldHashAfterFinalResult) {
        return 0;
    }
    out->chestAWorldHashStableFinal =
        out->worldHashBefore == out->worldHashAfterFinal ? 1 : 0;
    return 1;
}
