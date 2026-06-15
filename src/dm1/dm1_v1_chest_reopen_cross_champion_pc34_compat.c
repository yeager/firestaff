#include "dm1_v1_chest_reopen_cross_champion_pc34_compat.h"

#include "dm1_v1_champion_leader_pc34_compat.h"
#include "dm1_v1_inventory_chest_load_pc34_compat.h"

#include <string.h>

static const char s_source_evidence[] =
    "CHEST.C F0333:31-67 opens/reopens a chest by setting G0426 and rematerializing the requested container links into G0425\n"
    "CHEST.C F0334:113-132 closes the current G0426 chest by compacting non-empty G0425 entries back into the container\n"
    "CHAMPION.C F0297/F0298:250-298 owns leader-hand put/remove and leader load deltas; F0300/F0301:511-515,606-615 route C30+ slots through G0425 and update load\n"
    "CHAMPION.C F0302:688-710 swaps the global leader hand with the currently routed slot; F0309/F0310:1157-1205 consume Load for encumbrance\n"
    "CLIKCHAM.C F0368:54-72 removes leader-hand weight from the old leader and adds it to the new leader during leader changes\n"
    "DUNGEON.C F0140:1114-1120 gives containers base weight plus linked CONTENTS; F0163:1796-1837 clears Next and appends relinked chest contents\n"
    "non-overlap: chest_reopen_contents_order covers same-leader reopen; chest_reopen_then_swap_leader_hand covers single-link swap; this gate covers full cross-champion reopen state machine";

const DM1_V1_ChestReopenCrossChampionSpecPc34
    dm1_v1_chest_reopen_cross_champion_pc34_spec = {
        "Source-locked contract gate only; not full real-asset chest runtime parity.",
        DM1_PC34_CHEST_REOPEN_CROSS_LEADER_A,
        DM1_PC34_CHEST_REOPEN_CROSS_LEADER_B,
        DM1_PC34_SLOT_CHEST_1,
        DM1_PC34_SLOT_CHEST_8,
        DM1_PC34_CHEST_REOPEN_CROSS_SLOT_COUNT,
        DM1_PC34_CHEST_REOPEN_CROSS_A_FIRST,
        DM1_PC34_CHEST_REOPEN_CROSS_B_FIRST,
        DM1_PC34_CHEST_REOPEN_CROSS_HAND_ITEM,
        DM1_PC34_CHEST_REOPEN_CROSS_HAND_WEIGHT
    };

static M11_Item make_item(int itemType, int weight)
{
    M11_Item item;

    memset(&item, 0, sizeof(item));
    item.itemType = itemType;
    item.weight = weight;
    item.identified = 1;
    item.allowedSlots = DM1_PC34_ALLOWED_CONTAINER;
    return item;
}

static void seed_chest(M11_Item* items, int count, int firstType, int firstWeight)
{
    int i;

    for (i = 0; i < count; ++i) {
        items[i] = make_item(firstType + i, firstWeight + i);
    }
}

static int copy_open_types(const M11_InventoryState* state, int champ, int* outTypes)
{
    int i;

    if (!state || !outTypes) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_REOPEN_CROSS_SLOT_COUNT; ++i) {
        M11_Item item;

        if (!m11_inventory_get_item_in_chest_slot(state, champ, i, &item)) {
            return 0;
        }
        outTypes[i] = item.itemType;
    }
    return 1;
}

static void copy_closed_types(const M11_Item* items, int count, int* outTypes)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_REOPEN_CROSS_SLOT_COUNT; ++i) {
        outTypes[i] = (items && i < count) ? items[i].itemType : 0;
    }
}

static int count_visible_types(const int* types)
{
    int count = 0;
    int i;

    if (!types) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_REOPEN_CROSS_SLOT_COUNT; ++i) {
        if (types[i] != 0) {
            ++count;
        }
    }
    return count;
}

static int order_matches_sequence(const int* types, int count, int firstType)
{
    int i;

    if (!types || count < 0 || count > DM1_PC34_CHEST_REOPEN_CROSS_SLOT_COUNT) {
        return 0;
    }
    for (i = 0; i < count; ++i) {
        if (types[i] != firstType + i) {
            return 0;
        }
    }
    for (; i < DM1_PC34_CHEST_REOPEN_CROSS_SLOT_COUNT; ++i) {
        if (types[i] != 0) {
            return 0;
        }
    }
    return 1;
}

static int contains_sequence_item(const int* types, int count, int firstType)
{
    int i;
    int j;

    if (!types || count < 0) {
        return 0;
    }
    for (i = 0; i < count; ++i) {
        for (j = 0; j < DM1_PC34_CHEST_REOPEN_CROSS_SLOT_COUNT; ++j) {
            if (types[j] == firstType + i) {
                return 1;
            }
        }
    }
    return 0;
}

static void seed_leader_switch_state(
    Dm1V1ChampionLeaderStatePc34Compat* leaderState,
    const M11_InventoryState* inventory,
    int handWeight)
{
    int i;

    DM1_V1_ChampionLeader_InitPc34Compat(leaderState);
    leaderState->leaderIndex = DM1_PC34_CHEST_REOPEN_CROSS_LEADER_A;
    leaderState->partyDirection = 1;
    leaderState->leaderHandWeight = handWeight;
    for (i = 0; i < DM1_PC34_CHEST_REOPEN_CROSS_PARTY_COUNT; ++i) {
        leaderState->champions[i].currentHealth = 100;
        leaderState->champions[i].load = m11_inventory_get_load(inventory, i);
    }
    leaderState->champions[DM1_PC34_CHEST_REOPEN_CROSS_LEADER_A].load +=
        handWeight;
}

const char* M11_GameView_ChestReopenCrossChampionSourceEvidencePc34(void)
{
    return s_source_evidence;
}

const DM1_V1_ChestReopenCrossChampionSpecPc34*
M11_GameView_ChestReopenCrossChampionSpecPc34(void)
{
    return &dm1_v1_chest_reopen_cross_champion_pc34_spec;
}

int M11_GameView_ChestReopenCrossChampionRunPc34(
    DM1_V1_ChestReopenCrossChampionProbePc34* out)
{
    M11_InventoryState inventory;
    Dm1V1ChampionLeaderStatePc34Compat leaderState;
    Dm1V1ChampionLeaderSetResultPc34Compat switchResult;
    M11_Item chestA[DM1_PC34_CHEST_REOPEN_CROSS_A_COUNT];
    M11_Item chestB[DM1_PC34_CHEST_REOPEN_CROSS_B_COUNT];
    M11_Item closedA[DM1_PC34_CHEST_REOPEN_CROSS_SLOT_COUNT];
    M11_Item closedB[DM1_PC34_CHEST_REOPEN_CROSS_SLOT_COUNT];
    M11_Item hand;
    int leaderA;
    int leaderB;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    memset(closedA, 0, sizeof(closedA));
    memset(closedB, 0, sizeof(closedB));

    leaderA = DM1_PC34_CHEST_REOPEN_CROSS_LEADER_A;
    leaderB = DM1_PC34_CHEST_REOPEN_CROSS_LEADER_B;
    out->contractOnly = 1;
    out->leaderAIndex = leaderA;
    out->leaderBIndex = leaderB;
    out->chestAThing = DM1_PC34_CHEST_REOPEN_CROSS_A_THING;
    out->chestBThing = DM1_PC34_CHEST_REOPEN_CROSS_B_THING;

    m11_inventory_init(&inventory, DM1_PC34_CHEST_REOPEN_CROSS_PARTY_COUNT);
    seed_chest(chestA, DM1_PC34_CHEST_REOPEN_CROSS_A_COUNT,
               DM1_PC34_CHEST_REOPEN_CROSS_A_FIRST, 5);
    seed_chest(chestB, DM1_PC34_CHEST_REOPEN_CROSS_B_COUNT,
               DM1_PC34_CHEST_REOPEN_CROSS_B_FIRST, 11);

    out->leaderABaseSetResult = m11_inventory_set_item_in_pc34_source_slot(
        &inventory, leaderA, DM1_PC34_SLOT_BACKPACK_LINE1_1,
        DM1_PC34_CHEST_REOPEN_CROSS_A_BASE_ITEM, 17, 0,
        DM1_PC34_ALLOWED_ANY_SLOT);
    out->leaderBBaseSetResult = m11_inventory_set_item_in_pc34_source_slot(
        &inventory, leaderB, DM1_PC34_SLOT_BACKPACK_LINE1_1,
        DM1_PC34_CHEST_REOPEN_CROSS_B_BASE_ITEM, 23, 0,
        DM1_PC34_ALLOWED_ANY_SLOT);
    if (!out->leaderABaseSetResult || !out->leaderBBaseSetResult) {
        return 0;
    }
    out->leaderABaseLoad = m11_inventory_get_load(&inventory, leaderA);
    out->leaderBBaseLoad = m11_inventory_get_load(&inventory, leaderB);

    out->leaderAHandSetupResult = m11_inventory_set_mouse_item(
        &inventory, leaderA, DM1_PC34_CHEST_REOPEN_CROSS_HAND_ITEM,
        DM1_PC34_CHEST_REOPEN_CROSS_HAND_WEIGHT, 0,
        DM1_PC34_ALLOWED_CONTAINER);
    if (!out->leaderAHandSetupResult ||
        !m11_inventory_get_mouse_item(&inventory, leaderA, &hand)) {
        return 0;
    }
    out->leaderAHandBeforeSwitchType = hand.itemType;
    out->leaderAHandBeforeSwitchWeight = hand.weight;

    /* ReDMCSB CHEST.C F0333 lines 53-67 materializes leader A's action-hand
     * chest links into G0425 while the global leader hand remains separate. */
    out->chestAOpenResult = m11_inventory_open_chest(
        &inventory, leaderA, out->chestAThing, chestA,
        DM1_PC34_CHEST_REOPEN_CROSS_A_COUNT);
    out->chestAOpenThing = m11_inventory_get_open_chest_thing(
        &inventory, leaderA);
    if (!out->chestAOpenResult ||
        !copy_open_types(&inventory, leaderA, out->chestAOpenedTypes)) {
        return 0;
    }
    out->chestAOpenedVisibleWeight =
        m11_inventory_pc34_open_chest_visible_contents_weight(
            &inventory, leaderA);
    out->leaderAPanelLoadAfterOpen =
        m11_inventory_get_load(&inventory, leaderA) + hand.weight;

    /* ReDMCSB CHEST.C F0334 lines 113-132 closes A by compacting current
     * G0425 entries; DUNGEON.C F0140 lines 1114-1120 gives the snapshot weight. */
    out->chestACloseCount = m11_inventory_pc34_close_chest_with_weight_snapshot(
        &inventory, leaderA, closedA,
        DM1_PC34_CHEST_REOPEN_CROSS_SLOT_COUNT,
        &out->chestACloseContainerSnapshot);
    if (out->chestACloseCount < 0) {
        return 0;
    }
    copy_closed_types(closedA, out->chestACloseCount,
                      out->chestAClosedTypes);
    out->leaderAPanelLoadAfterClose =
        m11_inventory_get_load(&inventory, leaderA) + hand.weight;

    seed_leader_switch_state(&leaderState, &inventory, hand.weight);
    /* ReDMCSB CLIKCHAM.C F0368 lines 54-72 detaches the global leader hand
     * weight from leader A and attaches it to the new leader B. */
    out->leaderSwitchResult = DM1_V1_ChampionLeader_SetPc34Compat(
        &leaderState, leaderB, &switchResult);
    if (!out->leaderSwitchResult) {
        return 0;
    }
    out->leaderSwitchPrevious = switchResult.previousLeaderIndex;
    out->leaderSwitchNew = switchResult.newLeaderIndex;
    out->leaderAStateLoadAfterSwitch =
        leaderState.champions[leaderA].load;
    out->leaderBStateLoadAfterSwitch =
        leaderState.champions[leaderB].load;

    if (!m11_inventory_set_mouse_item(&inventory, leaderA, 0, 0, 0, 0) ||
        !m11_inventory_set_mouse_item(&inventory, leaderB, hand.itemType,
                                      hand.weight, hand.charges,
                                      hand.allowedSlots) ||
        !m11_inventory_get_mouse_item(&inventory, leaderA, &hand)) {
        return 0;
    }
    out->leaderAHandAfterSwitchType = hand.itemType;
    out->leaderAHandClearedAfterSwitch = hand.itemType == 0 ? 1 : 0;
    if (!m11_inventory_get_mouse_item(&inventory, leaderB, &hand)) {
        return 0;
    }
    out->leaderBHandAfterSwitchType = hand.itemType;
    out->leaderBHandAfterSwitchWeight = hand.weight;
    out->leaderBHandOccupiedAfterSwitch = hand.itemType != 0 ? 1 : 0;
    out->leaderBPanelLoadAfterSwitch =
        m11_inventory_get_load(&inventory, leaderB) + hand.weight;

    /* ReDMCSB CHEST.C F0333 lines 31-67 now rematerializes the new leader's
     * chest, so G0425 must reflect B's closed inventory snapshot rather than A's. */
    out->chestBReopenResult = m11_inventory_open_chest(
        &inventory, leaderB, out->chestBThing, chestB,
        DM1_PC34_CHEST_REOPEN_CROSS_B_COUNT);
    out->chestBOpenThing = m11_inventory_get_open_chest_thing(
        &inventory, leaderB);
    if (!out->chestBReopenResult ||
        !copy_open_types(&inventory, leaderB, out->chestBReopenedTypes)) {
        return 0;
    }
    out->chestBReopenedVisibleCount =
        count_visible_types(out->chestBReopenedTypes);
    out->chestBReopenedVisibleWeight =
        m11_inventory_pc34_open_chest_visible_contents_weight(
            &inventory, leaderB);
    out->chestBOrderMatchesLeaderB =
        order_matches_sequence(out->chestBReopenedTypes,
                               DM1_PC34_CHEST_REOPEN_CROSS_B_COUNT,
                               DM1_PC34_CHEST_REOPEN_CROSS_B_FIRST);
    out->chestBOrderLeaksLeaderA =
        contains_sequence_item(out->chestBReopenedTypes,
                               DM1_PC34_CHEST_REOPEN_CROSS_A_COUNT,
                               DM1_PC34_CHEST_REOPEN_CROSS_A_FIRST);
    out->leaderBPanelLoadAfterReopen =
        m11_inventory_get_load(&inventory, leaderB) + hand.weight;
    out->leaderBReopenLoadDelta =
        out->leaderBPanelLoadAfterReopen - out->leaderBPanelLoadAfterSwitch;
    out->reopenDoesNotDoubleCountLinkWeights =
        out->leaderBReopenLoadDelta == out->chestBReopenedVisibleWeight ? 1 : 0;

    /* ReDMCSB CHEST.C F0334 lines 117-132 and DUNGEON.C F0163 lines 1796-1837
     * rewrite the B snapshot once; closing proves the container weight matches
     * the four B links and is not counted twice. */
    out->chestBCloseCount = m11_inventory_pc34_close_chest_with_weight_snapshot(
        &inventory, leaderB, closedB,
        DM1_PC34_CHEST_REOPEN_CROSS_SLOT_COUNT,
        &out->chestBCloseContainerSnapshot);
    if (out->chestBCloseCount < 0) {
        return 0;
    }
    copy_closed_types(closedB, out->chestBCloseCount,
                      out->chestBClosedTypes);
    out->containerSnapshotWeightConsistent =
        out->chestBCloseContainerSnapshot ==
        DM1_PC34_CHEST_EMPTY_THING_WEIGHT + out->chestBReopenedVisibleWeight ?
        1 : 0;
    out->leaderBPanelLoadAfterClose =
        m11_inventory_get_load(&inventory, leaderB) + hand.weight;

    return 1;
}
