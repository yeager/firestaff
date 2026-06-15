#include "dm1_v1_chest_pickup_skip_leader_full_hand_to_non_leader_pc34_compat.h"

#include "dm1_v1_inventory_chest_load_pc34_compat.h"

#include <string.h>

static const char s_source_evidence[] =
    "CHEST.C F0333:53-76 materializes the first eight links as visible C537..C544/G0425 chest slots\n"
    "CHEST.C F0334:117-132 rewrites only non-empty visible G0425 slots, clearing each processed slot and compacting the list\n"
    "CHAMPION.C F0284:117-130 keeps champion hand/status redraw separate from party direction changes, so this gate treats the targeted champion hand as explicit state\n"
    "CHAMPION.C F0297:243-268 and F0298:270-298 own leader-hand put/remove; this regression proves they are not applied to the already-full leader hand for a non-leader targeted pickup\n"
    "CHAMPION.C F0302:677-710 resolves the clicked champion/slot, reads the leader hand and selected C30+ slot, and then performs the accepted slot exchange\n"
    "DEFS.H C30 and C537..C544 define the open chest source slots used here";

const DM1_V1_ChestPickupSkipLeaderFullHandToNonLeaderSpecPc34
    dm1_v1_chest_pickup_skip_leader_full_hand_to_non_leader_pc34_spec = {
        "Runtime gate: full leader hand is skipped when the non-leader champion is the pickup target.",
        DM1_PC34_CHEST_SKIP_LEADER_PARTY_COUNT,
        DM1_PC34_CHEST_SKIP_LEADER_LEADER_INDEX,
        DM1_PC34_CHEST_SKIP_LEADER_TARGET_INDEX,
        DM1_PC34_CHEST_SKIP_LEADER_PICKED_INDEX,
        DM1_PC34_CHEST_SKIP_LEADER_PICKED_PC34_SLOT,
        DM1_PC34_SLOT_CHEST_1,
        DM1_PC34_SLOT_CHEST_8,
        DM1_PC34_CHEST_SKIP_LEADER_SLOT_COUNT - 1,
        1,
        "CHEST.C F0333 lines 53-76 C537..C544 materialization",
        "CHEST.C F0334 lines 117-132 non-empty G0425 close compaction",
        "CHAMPION.C F0284 lines 117-130 champion hand/status redraw boundary",
        "CHAMPION.C F0297 lines 243-268 and F0298 lines 270-298 leader hand state",
        "CHAMPION.C F0302 lines 677-710 target slot dispatch",
        "DEFS.H C30 and C537..C544 slot definitions"
    };

static M11_Item make_item(int itemType, int weight, int charges,
                          int allowedSlots)
{
    M11_Item item;

    memset(&item, 0, sizeof(item));
    item.itemType = itemType;
    item.weight = weight;
    item.charges = charges;
    item.identified = 1;
    item.allowedSlots = allowedSlots;
    return item;
}

static void seed_chest(M11_Item* items)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_SKIP_LEADER_SLOT_COUNT; ++i) {
        items[i] = make_item(DM1_PC34_CHEST_SKIP_LEADER_FIRST_ITEM + i,
                             3 + i,
                             10 + i,
                             DM1_PC34_ALLOWED_CONTAINER);
    }
}

static int copy_open_fields(const M11_InventoryState* state,
                            int champ,
                            int* types,
                            int* weights,
                            int* charges,
                            int* allowedSlots)
{
    int i;

    if (!state || !types || !weights || !charges || !allowedSlots) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_SKIP_LEADER_SLOT_COUNT; ++i) {
        M11_Item item;

        if (!m11_inventory_get_item_in_chest_slot(state, champ, i, &item)) {
            return 0;
        }
        types[i] = item.itemType;
        weights[i] = item.weight;
        charges[i] = item.charges;
        allowedSlots[i] = item.allowedSlots;
    }
    return 1;
}

static void copy_item_fields(const M11_Item* items,
                             int count,
                             int* types,
                             int* weights,
                             int* charges,
                             int* allowedSlots)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_SKIP_LEADER_SLOT_COUNT; ++i) {
        if (items && i < count) {
            types[i] = items[i].itemType;
            weights[i] = items[i].weight;
            charges[i] = items[i].charges;
            allowedSlots[i] = items[i].allowedSlots;
        } else {
            types[i] = 0;
            weights[i] = 0;
            charges[i] = 0;
            allowedSlots[i] = 0;
        }
    }
}

static int count_type_in_slots(const int* types, int itemType)
{
    int i;
    int count = 0;

    if (!types || itemType == 0) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_SKIP_LEADER_SLOT_COUNT; ++i) {
        if (types[i] == itemType) {
            ++count;
        }
    }
    return count;
}

static int count_type_in_hands_and_slots(int leaderHandType,
                                         int nonLeaderHandType,
                                         const int* types,
                                         int itemType)
{
    int count = count_type_in_slots(types, itemType);

    if (leaderHandType == itemType) {
        ++count;
    }
    if (nonLeaderHandType == itemType) {
        ++count;
    }
    return count;
}

static int open_preserves_other_slots(
    const DM1_V1_ChestPickupSkipLeaderFullHandToNonLeaderProbePc34* out)
{
    int i;

    if (!out) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_SKIP_LEADER_SLOT_COUNT; ++i) {
        if (i == DM1_PC34_CHEST_SKIP_LEADER_PICKED_INDEX) {
            if (out->openAfterPickupTypes[i] != 0 ||
                out->openAfterPickupWeights[i] != 0 ||
                out->openAfterPickupCharges[i] != 0 ||
                out->openAfterPickupAllowedSlots[i] != 0) {
                return 0;
            }
            continue;
        }
        if (out->openAfterPickupTypes[i] != out->initialTypes[i] ||
            out->openAfterPickupWeights[i] != out->initialWeights[i] ||
            out->openAfterPickupCharges[i] != out->initialCharges[i] ||
            out->openAfterPickupAllowedSlots[i] !=
                out->initialAllowedSlots[i]) {
            return 0;
        }
    }
    return 1;
}

static int compacted_minus_picked(
    const DM1_V1_ChestPickupSkipLeaderFullHandToNonLeaderProbePc34* out,
    const int* types,
    const int* weights,
    const int* charges,
    const int* allowedSlots)
{
    int sourceIndex = 0;
    int compactedIndex = 0;

    if (!out || !types || !weights || !charges || !allowedSlots) {
        return 0;
    }
    for (sourceIndex = 0;
         sourceIndex < DM1_PC34_CHEST_SKIP_LEADER_SLOT_COUNT;
         ++sourceIndex) {
        if (sourceIndex == DM1_PC34_CHEST_SKIP_LEADER_PICKED_INDEX) {
            continue;
        }
        if (types[compactedIndex] != out->initialTypes[sourceIndex] ||
            weights[compactedIndex] != out->initialWeights[sourceIndex] ||
            charges[compactedIndex] != out->initialCharges[sourceIndex] ||
            allowedSlots[compactedIndex] !=
                out->initialAllowedSlots[sourceIndex]) {
            return 0;
        }
        ++compactedIndex;
    }
    return types[compactedIndex] == 0 &&
           weights[compactedIndex] == 0 &&
           charges[compactedIndex] == 0 &&
           allowedSlots[compactedIndex] == 0;
}

const char*
dm1_v1_chest_pickup_skip_leader_full_hand_to_non_leader_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const DM1_V1_ChestPickupSkipLeaderFullHandToNonLeaderSpecPc34*
dm1_v1_chest_pickup_skip_leader_full_hand_to_non_leader_spec_pc34(void)
{
    return &dm1_v1_chest_pickup_skip_leader_full_hand_to_non_leader_pc34_spec;
}

int dm1_v1_chest_pickup_skip_leader_full_hand_to_non_leader_pc34(
    DM1_V1_ChestPickupSkipLeaderFullHandToNonLeaderProbePc34* out)
{
    M11_InventoryState state;
    M11_Item linked[DM1_PC34_CHEST_SKIP_LEADER_SLOT_COUNT];
    M11_Item closed[DM1_PC34_CHEST_SKIP_LEADER_SLOT_COUNT];
    M11_Item leaderHandBefore;
    M11_Item leaderHandAfter;
    M11_Item targetHandBefore;
    M11_Item targetHandAfter;
    int leader = DM1_PC34_CHEST_SKIP_LEADER_LEADER_INDEX;
    int target = DM1_PC34_CHEST_SKIP_LEADER_TARGET_INDEX;
    int picked = DM1_PC34_CHEST_SKIP_LEADER_PICKED_INDEX;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    memset(closed, 0, sizeof(closed));
    seed_chest(linked);

    out->sourceLockedContractOnly = 0;
    out->leaderIndex = leader;
    out->targetChampionIndex = target;
    out->pickedChestIndex = picked;
    out->pickedPc34Slot = DM1_PC34_CHEST_SKIP_LEADER_PICKED_PC34_SLOT;
    out->pickedItemType = linked[picked].itemType;
    out->pickedItemWeight = linked[picked].weight;
    out->pickedItemCharges = linked[picked].charges;
    out->pickedItemAllowedSlots = linked[picked].allowedSlots;

    m11_inventory_init(&state, DM1_PC34_CHEST_SKIP_LEADER_PARTY_COUNT);
    out->leaderHandSetupResult = m11_inventory_set_mouse_item(
        &state, leader, DM1_PC34_CHEST_SKIP_LEADER_LEADER_HAND_ITEM,
        21, 4, DM1_PC34_ALLOWED_CONTAINER);
    if (!out->leaderHandSetupResult ||
        !m11_inventory_get_mouse_item(&state, leader, &leaderHandBefore) ||
        !m11_inventory_get_mouse_item(&state, target, &targetHandBefore)) {
        return 0;
    }

    out->leaderHandBeforeType = leaderHandBefore.itemType;
    out->leaderHandBeforeWeight = leaderHandBefore.weight;
    out->leaderHandBeforeCharges = leaderHandBefore.charges;
    out->leaderHandBeforeAllowedSlots = leaderHandBefore.allowedSlots;
    out->nonLeaderHandBeforeType = targetHandBefore.itemType;
    out->nonLeaderHandStartsEmpty =
        targetHandBefore.itemType == 0 ? 1 : 0;

    /* ReDMCSB CHEST.C F0333 lines 53-76 exposes C537..C544 as G0425.
     * This probe opens the chest for the explicitly targeted non-leader,
     * while CHAMPION.C F0297/F0298 lines 243-298 leave the already-full
     * leader hand as separate state. */
    out->openResult = m11_inventory_open_chest(
        &state, target, DM1_PC34_CHEST_SKIP_LEADER_THING,
        linked, DM1_PC34_CHEST_SKIP_LEADER_SLOT_COUNT);
    out->openThing = m11_inventory_get_open_chest_thing(&state, target);
    if (!out->openResult ||
        !copy_open_fields(&state, target,
                          out->initialTypes,
                          out->initialWeights,
                          out->initialCharges,
                          out->initialAllowedSlots)) {
        return 0;
    }

    /* ReDMCSB CHAMPION.C F0302 lines 677-710 routes the selected C30+
     * chest slot through the current target. Firestaff's PC34 runtime state
     * stores the hand by champion, so a non-leader targeted empty hand picks
     * up the visible C539 object without colliding with the full leader hand. */
    out->pickupClickResult = m11_inventory_click_open_chest_slot_for_thing(
        &state, target, DM1_PC34_CHEST_SKIP_LEADER_THING, picked);
    if (!m11_inventory_get_mouse_item(&state, leader, &leaderHandAfter) ||
        !m11_inventory_get_mouse_item(&state, target, &targetHandAfter) ||
        !copy_open_fields(&state, target,
                          out->openAfterPickupTypes,
                          out->openAfterPickupWeights,
                          out->openAfterPickupCharges,
                          out->openAfterPickupAllowedSlots)) {
        return 0;
    }

    out->leaderHandAfterType = leaderHandAfter.itemType;
    out->leaderHandAfterWeight = leaderHandAfter.weight;
    out->leaderHandAfterCharges = leaderHandAfter.charges;
    out->leaderHandAfterAllowedSlots = leaderHandAfter.allowedSlots;
    out->nonLeaderHandAfterType = targetHandAfter.itemType;
    out->nonLeaderHandAfterWeight = targetHandAfter.weight;
    out->nonLeaderHandAfterCharges = targetHandAfter.charges;
    out->nonLeaderHandAfterAllowedSlots = targetHandAfter.allowedSlots;
    out->leaderHandUnchanged =
        memcmp(&leaderHandBefore, &leaderHandAfter,
               sizeof(leaderHandBefore)) == 0 ? 1 : 0;
    out->nonLeaderReceivedPickedItem =
        targetHandAfter.itemType == out->pickedItemType &&
        targetHandAfter.weight == out->pickedItemWeight &&
        targetHandAfter.charges == out->pickedItemCharges &&
        targetHandAfter.allowedSlots == out->pickedItemAllowedSlots ? 1 : 0;
    out->pickedSlotClearedInOpenView =
        out->openAfterPickupTypes[picked] == 0 ? 1 : 0;
    out->openViewPreservesOtherSlots = open_preserves_other_slots(out);
    out->leaderCollisionCountAfterPickup =
        count_type_in_slots(out->openAfterPickupTypes,
                            out->leaderHandAfterType) +
        (out->nonLeaderHandAfterType == out->leaderHandAfterType ? 1 : 0);
    out->pickedItemCountAfterPickup =
        count_type_in_hands_and_slots(out->leaderHandAfterType,
                                      out->nonLeaderHandAfterType,
                                      out->openAfterPickupTypes,
                                      out->pickedItemType);
    out->leaderItemCountAfterPickup =
        count_type_in_hands_and_slots(out->leaderHandAfterType,
                                      out->nonLeaderHandAfterType,
                                      out->openAfterPickupTypes,
                                      DM1_PC34_CHEST_SKIP_LEADER_LEADER_HAND_ITEM);

    /* ReDMCSB CHEST.C F0334 lines 117-132 compacts the still-visible G0425
     * slots. Reopening from that snapshot pins the exact C537..C544 behavior:
     * the original order is preserved with the picked C539 item omitted. */
    out->closeCount = m11_inventory_close_chest(
        &state, target, closed, DM1_PC34_CHEST_SKIP_LEADER_SLOT_COUNT);
    if (out->closeCount < 0) {
        return 0;
    }
    copy_item_fields(closed, out->closeCount,
                     out->closedTypes,
                     out->closedWeights,
                     out->closedCharges,
                     out->closedAllowedSlots);
    out->closeCompactsMinusPicked =
        compacted_minus_picked(out,
                               out->closedTypes,
                               out->closedWeights,
                               out->closedCharges,
                               out->closedAllowedSlots);

    out->reopenResult = m11_inventory_open_chest(
        &state, target, DM1_PC34_CHEST_SKIP_LEADER_REOPEN_THING,
        closed, out->closeCount);
    out->reopenThing = m11_inventory_get_open_chest_thing(&state, target);
    if (!out->reopenResult ||
        !copy_open_fields(&state, target,
                          out->reopenedTypes,
                          out->reopenedWeights,
                          out->reopenedCharges,
                          out->reopenedAllowedSlots)) {
        return 0;
    }
    out->reopenCompactsMinusPicked =
        compacted_minus_picked(out,
                               out->reopenedTypes,
                               out->reopenedWeights,
                               out->reopenedCharges,
                               out->reopenedAllowedSlots);
    out->pickedItemCountAfterReopen =
        count_type_in_hands_and_slots(out->leaderHandAfterType,
                                      out->nonLeaderHandAfterType,
                                      out->reopenedTypes,
                                      out->pickedItemType);
    out->leaderItemCountAfterReopen =
        count_type_in_hands_and_slots(out->leaderHandAfterType,
                                      out->nonLeaderHandAfterType,
                                      out->reopenedTypes,
                                      DM1_PC34_CHEST_SKIP_LEADER_LEADER_HAND_ITEM);

    return out->pickupClickResult &&
           out->leaderHandUnchanged &&
           out->nonLeaderReceivedPickedItem &&
           out->pickedSlotClearedInOpenView &&
           out->openViewPreservesOtherSlots &&
           out->closeCompactsMinusPicked &&
           out->reopenCompactsMinusPicked;
}
