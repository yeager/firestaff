#include "dm1_v1_chest_round_trip_hand_swap_pc34_compat.h"

#include <string.h>

static const char s_f0333_anchor[] =
    "CHEST.C F0333:53-67 materializes visible linked chest objects into "
    "G0425_aT_ChestSlots/C537..C544.";
static const char s_f0334_anchor[] =
    "CHEST.C F0334:117-132 rewrites non-empty visible G0425 slots into "
    "the container link array and clears the open chest.";
static const char s_f0297_anchor[] =
    "CHAMPION.C F0297:263-265 adds the leader-hand object's F0140 weight "
    "to leader Load.";
static const char s_f0300_anchor[] =
    "CHAMPION.C F0300:582-615 removes selected slot weight from leader "
    "Load after C30+ G0425 removal.";
static const char s_f0301_anchor[] =
    "CHAMPION.C F0301:582-615 adds accepted slot weight to leader Load "
    "after C30+ G0425 insertion.";
static const char s_source_summary[] =
    "contract_only=1; round-trip hand swap preservation gate covers "
    "C537/C538 visible slots only and does not claim the hidden-tail "
    "ninth-object path.";

static const DM1_V1_ChestRoundTripHandSwapSpecPc34 s_spec = {
    1,
    DM1_PC34_SLOT_CHEST_1,
    DM1_PC34_SLOT_CHEST_2,
    DM1_PC34_SLOT_CHEST_8,
    DM1_PC34_CHEST_ROUND_TRIP_SLOT_COUNT,
    DM1_PC34_CHEST_ROUND_TRIP_LINKED_INPUT_COUNT,
    0,
    { DM1_PC34_CHEST_ROUND_TRIP_DAGGER,
      DM1_PC34_CHEST_ROUND_TRIP_DAGGER_WEIGHT,
      DM1_PC34_ALLOWED_CONTAINER },
    { DM1_PC34_CHEST_ROUND_TRIP_TORCH,
      DM1_PC34_CHEST_ROUND_TRIP_TORCH_WEIGHT,
      DM1_PC34_ALLOWED_CONTAINER },
    { DM1_PC34_CHEST_ROUND_TRIP_BASE_ITEM,
      DM1_PC34_CHEST_ROUND_TRIP_BASE_WEIGHT,
      DM1_PC34_ALLOWED_ANY_SLOT },
    s_f0333_anchor,
    s_f0334_anchor,
    s_f0297_anchor,
    s_f0300_anchor,
    s_f0301_anchor,
    s_source_summary
};

static DM1_V1_ChestRoundTripHandSwapProbePc34 s_last_probe;

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

static int copy_open_slots(const M11_InventoryState* state,
                           int* typesOut,
                           int* weightsOut)
{
    int i;

    if (!state || !typesOut || !weightsOut) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_ROUND_TRIP_SLOT_COUNT; ++i) {
        M11_Item item;

        if (!m11_inventory_get_item_in_chest_slot(state, 0, i, &item)) {
            return 0;
        }
        typesOut[i] = item.itemType;
        weightsOut[i] = item.weight;
    }
    return 1;
}

static void copy_closed_slots(const M11_Item* closed,
                              int count,
                              int* typesOut,
                              int* weightsOut)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_ROUND_TRIP_SLOT_COUNT; ++i) {
        if (closed && i < count) {
            typesOut[i] = closed[i].itemType;
            weightsOut[i] = closed[i].weight;
        } else {
            typesOut[i] = 0;
            weightsOut[i] = 0;
        }
    }
}

static int count_visible(const int* types)
{
    int count = 0;
    int i;

    if (!types) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_ROUND_TRIP_SLOT_COUNT; ++i) {
        if (types[i] != 0) {
            ++count;
        }
    }
    return count;
}

static int contains_type(const int* types, int itemType)
{
    int i;

    if (!types) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_ROUND_TRIP_SLOT_COUNT; ++i) {
        if (types[i] == itemType) {
            return 1;
        }
    }
    return 0;
}

static int only_visible_slots_used(const int* types, int linkedInputCount)
{
    int i;

    if (!types || linkedInputCount < 0 ||
        linkedInputCount > DM1_PC34_CHEST_ROUND_TRIP_SLOT_COUNT) {
        return 0;
    }
    for (i = linkedInputCount;
         i < DM1_PC34_CHEST_ROUND_TRIP_SLOT_COUNT;
         ++i) {
        if (types[i] != 0) {
            return 0;
        }
    }
    return 1;
}

static int leader_hand_weight(const M11_InventoryState* state)
{
    M11_Item hand;

    if (!m11_inventory_get_mouse_item(state, 0, &hand)) {
        return 0;
    }
    return hand.weight;
}

static void record_check(int condition, int* passed, int* failed)
{
    if (condition) {
        ++*passed;
    } else {
        ++*failed;
    }
}

const char* dm1_v1_chest_round_trip_hand_swap_source_evidence_pc34(void)
{
    return s_source_summary;
}

const DM1_V1_ChestRoundTripHandSwapSpecPc34*
dm1_v1_chest_round_trip_hand_swap_spec_pc34(void)
{
    return &s_spec;
}

const DM1_V1_ChestRoundTripHandSwapProbePc34*
dm1_v1_chest_round_trip_hand_swap_last_probe_pc34(void)
{
    return &s_last_probe;
}

int dm1_v1_chest_round_trip_hand_swap_run(int* passed, int* failed)
{
    M11_InventoryState state;
    M11_Item linked[DM1_PC34_CHEST_ROUND_TRIP_LINKED_INPUT_COUNT];
    M11_Item closed[DM1_PC34_CHEST_ROUND_TRIP_SLOT_COUNT];
    M11_Item hand;
    int localPassed = 0;
    int localFailed = 0;

    if (passed) {
        *passed = 0;
    }
    if (failed) {
        *failed = 0;
    }
    memset(&s_last_probe, 0, sizeof(s_last_probe));
    memset(closed, 0, sizeof(closed));

    linked[0] = make_item(DM1_PC34_CHEST_ROUND_TRIP_DAGGER,
                          DM1_PC34_CHEST_ROUND_TRIP_DAGGER_WEIGHT,
                          DM1_PC34_ALLOWED_CONTAINER);
    linked[1] = make_item(DM1_PC34_CHEST_ROUND_TRIP_TORCH,
                          DM1_PC34_CHEST_ROUND_TRIP_TORCH_WEIGHT,
                          DM1_PC34_ALLOWED_CONTAINER);

    m11_inventory_init(&state, 1);
    s_last_probe.contractOnly = 1;
    s_last_probe.hiddenTailClaimed = 0;
    s_last_probe.originalDaggerId = linked[0].itemType;
    s_last_probe.originalTorchId = linked[1].itemType;
    s_last_probe.baseItemSetResult =
        m11_inventory_set_item_in_pc34_source_slot(
            &state, 0, DM1_PC34_SLOT_BACKPACK_LINE1_1,
            DM1_PC34_CHEST_ROUND_TRIP_BASE_ITEM,
            DM1_PC34_CHEST_ROUND_TRIP_BASE_WEIGHT, 0,
            DM1_PC34_ALLOWED_ANY_SLOT);
    if (!s_last_probe.baseItemSetResult) {
        return 0;
    }
    s_last_probe.open.loadBeforeOpen = m11_inventory_get_load(&state, 0);

    /* ReDMCSB CHEST.C F0333:53-67 copies the two linked entries into C537
     * and C538/G0425, with C539..C544 left empty. */
    s_last_probe.open.openResult = m11_inventory_open_chest(
        &state, 0, DM1_PC34_CHEST_ROUND_TRIP_CHEST_THING, linked,
        DM1_PC34_CHEST_ROUND_TRIP_LINKED_INPUT_COUNT);
    s_last_probe.open.openThing = m11_inventory_get_open_chest_thing(&state, 0);
    if (!s_last_probe.open.openResult ||
        !copy_open_slots(&state, s_last_probe.open.openedTypes,
                         s_last_probe.open.openedWeights)) {
        return 0;
    }
    s_last_probe.open.openedVisibleCount =
        count_visible(s_last_probe.open.openedTypes);
    s_last_probe.open.openedHasDaggerAtC537 =
        s_last_probe.open.openedTypes[0] ==
        DM1_PC34_CHEST_ROUND_TRIP_DAGGER;
    s_last_probe.open.openedHasTorchAtC538 =
        s_last_probe.open.openedTypes[1] ==
        DM1_PC34_CHEST_ROUND_TRIP_TORCH;
    s_last_probe.open.openedOnlyVisibleSlots =
        only_visible_slots_used(
            s_last_probe.open.openedTypes,
            DM1_PC34_CHEST_ROUND_TRIP_LINKED_INPUT_COUNT);
    s_last_probe.open.openVisibleWeight =
        m11_inventory_pc34_open_chest_visible_contents_weight(&state, 0);
    s_last_probe.open.openContainerWeight =
        m11_inventory_pc34_open_chest_container_weight(&state, 0);
    s_last_probe.open.openContainerBaseContribution =
        s_last_probe.open.openContainerWeight -
        s_last_probe.open.openVisibleWeight;
    s_last_probe.open.loadAfterOpen = m11_inventory_get_load(&state, 0);
    s_last_probe.open.loadDeltaAfterOpen =
        s_last_probe.open.loadAfterOpen -
        s_last_probe.open.loadBeforeOpen;

    /* ReDMCSB CHAMPION.C F0300:582-615 removes C538 from G0425 and
     * F0297:263-265 places that torch in the leader hand. */
    s_last_probe.firstSwap.firstClickResult =
        m11_inventory_click_pc34_source_slot(
            &state, 0, DM1_PC34_SLOT_CHEST_2);
    if (!s_last_probe.firstSwap.firstClickResult ||
        !m11_inventory_get_mouse_item(&state, 0, &hand) ||
        !copy_open_slots(&state, s_last_probe.firstSwap.afterFirstTypes,
                         s_last_probe.firstSwap.afterFirstWeights)) {
        return 0;
    }
    s_last_probe.firstSwap.leaderHandAfterFirstType = hand.itemType;
    s_last_probe.firstSwap.leaderHandAfterFirstWeight = hand.weight;
    s_last_probe.firstSwap.leaderHandAfterFirstCanEnterChest =
        m11_inventory_can_equip(&hand, DM1_PC34_SLOT_CHEST_1);
    s_last_probe.firstSwap.afterFirstVisibleCount =
        count_visible(s_last_probe.firstSwap.afterFirstTypes);
    s_last_probe.firstSwap.afterFirstC537StillDagger =
        s_last_probe.firstSwap.afterFirstTypes[0] ==
        DM1_PC34_CHEST_ROUND_TRIP_DAGGER;
    s_last_probe.firstSwap.afterFirstC538Empty =
        s_last_probe.firstSwap.afterFirstTypes[1] == 0;
    s_last_probe.firstSwap.afterFirstVisibleWeight =
        m11_inventory_pc34_open_chest_visible_contents_weight(&state, 0);
    s_last_probe.firstSwap.afterFirstContainerWeight =
        m11_inventory_pc34_open_chest_container_weight(&state, 0);
    s_last_probe.firstSwap.afterFirstContainerBaseContribution =
        s_last_probe.firstSwap.afterFirstContainerWeight -
        s_last_probe.firstSwap.afterFirstVisibleWeight;
    s_last_probe.firstSwap.loadAfterFirstSwap =
        m11_inventory_get_load(&state, 0);
    s_last_probe.firstSwap.loadDeltaAfterFirstSwap =
        s_last_probe.firstSwap.loadAfterFirstSwap -
        s_last_probe.open.loadAfterOpen;
    s_last_probe.firstSwap.effectiveLoadAfterFirstSwap =
        s_last_probe.firstSwap.loadAfterFirstSwap + leader_hand_weight(&state);
    s_last_probe.firstSwap.effectiveLoadDeltaAfterFirstSwap =
        s_last_probe.firstSwap.effectiveLoadAfterFirstSwap -
        s_last_probe.open.loadAfterOpen;

    /* ReDMCSB CHAMPION.C F0300/F0301:582-615 swaps C537 with the current
     * leader hand, preserving the torch in G0425 and the dagger in hand. */
    s_last_probe.secondSwap.secondClickResult =
        m11_inventory_click_pc34_source_slot(
            &state, 0, DM1_PC34_SLOT_CHEST_1);
    if (!s_last_probe.secondSwap.secondClickResult ||
        !m11_inventory_get_mouse_item(&state, 0, &hand) ||
        !copy_open_slots(&state, s_last_probe.secondSwap.afterSecondTypes,
                         s_last_probe.secondSwap.afterSecondWeights)) {
        return 0;
    }
    s_last_probe.secondSwap.leaderHandAfterSecondType = hand.itemType;
    s_last_probe.secondSwap.leaderHandAfterSecondWeight = hand.weight;
    s_last_probe.secondSwap.leaderHandAfterSecondCanEnterChest =
        m11_inventory_can_equip(&hand, DM1_PC34_SLOT_CHEST_1);
    s_last_probe.secondSwap.afterSecondVisibleCount =
        count_visible(s_last_probe.secondSwap.afterSecondTypes);
    s_last_probe.secondSwap.afterSecondC537Torch =
        s_last_probe.secondSwap.afterSecondTypes[0] ==
        DM1_PC34_CHEST_ROUND_TRIP_TORCH;
    s_last_probe.secondSwap.afterSecondC538Empty =
        s_last_probe.secondSwap.afterSecondTypes[1] == 0;
    s_last_probe.secondSwap.afterSecondDaggerOnlyInLeaderHand =
        hand.itemType == DM1_PC34_CHEST_ROUND_TRIP_DAGGER &&
        !contains_type(s_last_probe.secondSwap.afterSecondTypes,
                       DM1_PC34_CHEST_ROUND_TRIP_DAGGER);
    s_last_probe.secondSwap.afterSecondTorchOnlyInChest =
        hand.itemType != DM1_PC34_CHEST_ROUND_TRIP_TORCH &&
        contains_type(s_last_probe.secondSwap.afterSecondTypes,
                      DM1_PC34_CHEST_ROUND_TRIP_TORCH);
    s_last_probe.secondSwap.afterSecondVisibleWeight =
        m11_inventory_pc34_open_chest_visible_contents_weight(&state, 0);
    s_last_probe.secondSwap.afterSecondContainerWeight =
        m11_inventory_pc34_open_chest_container_weight(&state, 0);
    s_last_probe.secondSwap.afterSecondContainerBaseContribution =
        s_last_probe.secondSwap.afterSecondContainerWeight -
        s_last_probe.secondSwap.afterSecondVisibleWeight;
    s_last_probe.secondSwap.loadAfterSecondSwap =
        m11_inventory_get_load(&state, 0);
    s_last_probe.secondSwap.loadDeltaAfterSecondSwap =
        s_last_probe.secondSwap.loadAfterSecondSwap -
        s_last_probe.firstSwap.loadAfterFirstSwap;
    s_last_probe.secondSwap.effectiveLoadAfterSecondSwap =
        s_last_probe.secondSwap.loadAfterSecondSwap + leader_hand_weight(&state);
    s_last_probe.secondSwap.effectiveLoadDeltaAfterSecondSwap =
        s_last_probe.secondSwap.effectiveLoadAfterSecondSwap -
        s_last_probe.open.loadAfterOpen;

    /* ReDMCSB CHEST.C F0334:117-132 closes from non-empty visible slots only;
     * CHAMPION.C F0300/F0301:582-615 then leaves transient chest load back at
     * the pre-open value while F0297 keeps the leader hand object intact. */
    s_last_probe.close.closeCount =
        m11_inventory_pc34_close_chest_with_weight_snapshot(
            &state, 0, closed, DM1_PC34_CHEST_ROUND_TRIP_SLOT_COUNT,
            &s_last_probe.close.closeContainerWeightSnapshot);
    if (s_last_probe.close.closeCount < 0 ||
        !m11_inventory_get_mouse_item(&state, 0, &hand)) {
        return 0;
    }
    copy_closed_slots(closed, s_last_probe.close.closeCount,
                      s_last_probe.close.closedTypes,
                      s_last_probe.close.closedWeights);
    s_last_probe.close.closeClearsOpenChest =
        m11_inventory_get_open_chest_thing(&state, 0) == 0;
    s_last_probe.close.closedVisibleCount =
        count_visible(s_last_probe.close.closedTypes);
    s_last_probe.close.closedC537Torch =
        s_last_probe.close.closedTypes[0] ==
        DM1_PC34_CHEST_ROUND_TRIP_TORCH;
    s_last_probe.close.closedC538Empty =
        s_last_probe.close.closedTypes[1] == 0;
    s_last_probe.close.closedDaggerExcludedBecauseLeaderHand =
        !contains_type(s_last_probe.close.closedTypes,
                       DM1_PC34_CHEST_ROUND_TRIP_DAGGER);
    s_last_probe.close.closeContainerBaseContribution =
        s_last_probe.close.closeContainerWeightSnapshot -
        s_last_probe.secondSwap.afterSecondVisibleWeight;
    s_last_probe.close.closeContainerWeightAfter =
        m11_inventory_pc34_open_chest_container_weight(&state, 0);
    s_last_probe.close.loadAfterClose = m11_inventory_get_load(&state, 0);
    s_last_probe.close.loadDeltaAfterClose =
        s_last_probe.close.loadAfterClose -
        s_last_probe.open.loadBeforeOpen;
    s_last_probe.close.leaderHandAfterCloseType = hand.itemType;
    s_last_probe.close.leaderHandAfterCloseWeight = hand.weight;

    /* ReDMCSB CHEST.C F0333:53-67 rematerializes the F0334-produced link
     * array, proving C537 contains the original torch and the dagger stayed
     * in the leader hand rather than being duplicated into the chest. */
    s_last_probe.reopen.reopenResult = m11_inventory_open_chest(
        &state, 0, DM1_PC34_CHEST_ROUND_TRIP_REOPEN_THING, closed,
        s_last_probe.close.closeCount);
    s_last_probe.reopen.reopenThing =
        m11_inventory_get_open_chest_thing(&state, 0);
    if (!s_last_probe.reopen.reopenResult ||
        !m11_inventory_get_mouse_item(&state, 0, &hand) ||
        !copy_open_slots(&state, s_last_probe.reopen.reopenedTypes,
                         s_last_probe.reopen.reopenedWeights)) {
        return 0;
    }
    s_last_probe.reopen.reopenedVisibleCount =
        count_visible(s_last_probe.reopen.reopenedTypes);
    s_last_probe.reopen.reopenedC537Torch =
        s_last_probe.reopen.reopenedTypes[0] ==
        DM1_PC34_CHEST_ROUND_TRIP_TORCH;
    s_last_probe.reopen.reopenedC538Empty =
        s_last_probe.reopen.reopenedTypes[1] == 0;
    s_last_probe.reopen.reopenedDaggerStillLeaderHand =
        hand.itemType == DM1_PC34_CHEST_ROUND_TRIP_DAGGER;
    s_last_probe.reopen.reopenedTorchPreserved =
        contains_type(s_last_probe.reopen.reopenedTypes,
                      DM1_PC34_CHEST_ROUND_TRIP_TORCH);
    s_last_probe.reopen.reopenedOriginalObjectIdentitiesPreserved =
        s_last_probe.reopen.reopenedDaggerStillLeaderHand &&
        s_last_probe.reopen.reopenedTorchPreserved &&
        !contains_type(s_last_probe.reopen.reopenedTypes,
                       DM1_PC34_CHEST_ROUND_TRIP_DAGGER);
    s_last_probe.reopen.reopenedVisibleWeight =
        m11_inventory_pc34_open_chest_visible_contents_weight(&state, 0);
    s_last_probe.reopen.reopenedContainerWeight =
        m11_inventory_pc34_open_chest_container_weight(&state, 0);
    s_last_probe.reopen.reopenedContainerBaseContribution =
        s_last_probe.reopen.reopenedContainerWeight -
        s_last_probe.reopen.reopenedVisibleWeight;
    s_last_probe.reopen.loadAfterReopen = m11_inventory_get_load(&state, 0);
    s_last_probe.reopen.loadDeltaAfterReopen =
        s_last_probe.reopen.loadAfterReopen -
        s_last_probe.close.loadAfterClose;
    s_last_probe.reopen.leaderHandAfterReopenType = hand.itemType;
    s_last_probe.reopen.leaderHandAfterReopenWeight = hand.weight;

    record_check(s_last_probe.contractOnly == 1,
                 &localPassed, &localFailed);
    record_check(s_last_probe.hiddenTailClaimed == 0,
                 &localPassed, &localFailed);
    record_check(s_last_probe.open.openedHasDaggerAtC537,
                 &localPassed, &localFailed);
    record_check(s_last_probe.open.openedHasTorchAtC538,
                 &localPassed, &localFailed);
    record_check(s_last_probe.firstSwap.leaderHandAfterFirstType ==
                 DM1_PC34_CHEST_ROUND_TRIP_TORCH,
                 &localPassed, &localFailed);
    record_check(s_last_probe.secondSwap.leaderHandAfterSecondType ==
                 DM1_PC34_CHEST_ROUND_TRIP_DAGGER,
                 &localPassed, &localFailed);
    record_check(s_last_probe.secondSwap.afterSecondC537Torch,
                 &localPassed, &localFailed);
    record_check(s_last_probe.close.closeCount == 1,
                 &localPassed, &localFailed);
    record_check(s_last_probe.close.loadDeltaAfterClose == 0,
                 &localPassed, &localFailed);
    record_check(s_last_probe.close.leaderHandAfterCloseType ==
                 DM1_PC34_CHEST_ROUND_TRIP_DAGGER,
                 &localPassed, &localFailed);
    record_check(s_last_probe.reopen.reopenedC537Torch,
                 &localPassed, &localFailed);
    record_check(s_last_probe.reopen.reopenedC538Empty,
                 &localPassed, &localFailed);
    record_check(s_last_probe.reopen.reopenedOriginalObjectIdentitiesPreserved,
                 &localPassed, &localFailed);
    record_check(s_last_probe.reopen.leaderHandAfterReopenType ==
                 DM1_PC34_CHEST_ROUND_TRIP_DAGGER,
                 &localPassed, &localFailed);
    record_check(s_last_probe.open.openContainerBaseContribution ==
                 DM1_PC34_CHEST_EMPTY_THING_WEIGHT,
                 &localPassed, &localFailed);
    record_check(s_last_probe.close.closeContainerBaseContribution ==
                 DM1_PC34_CHEST_EMPTY_THING_WEIGHT,
                 &localPassed, &localFailed);

    if (passed) {
        *passed = localPassed;
    }
    if (failed) {
        *failed = localFailed;
    }
    return localFailed == 0 ? 1 : 0;
}
