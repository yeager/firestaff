#include "dm1_v1_chest_pickup_stack_failover_pc34_compat.h"

#include <string.h>

/*
 * Source-locked runtime probe for the hand-busy stackable pickup path.
 *
 * ReDMCSB CHEST.C F0333:30-32 keeps the active G0425 chest view stable,
 * F0333:53-67 makes C537..C544 the visible pickup dispatch source, and
 * F0334:117-132 rewires only the non-empty visible slots after this probe
 * removes or preserves the picked item.  OBJECT.C F0032/F0033:121-212 and
 * AMMO.C F0294:54-79 are represented by deterministic stack-kind sentinels
 * for bolts and scroll/quiver-like stackables.  BLITMASK.C F0133:30-33 is
 * the redraw anchor for leader-hand icon preservation and merge changes.
 */

static const char s_f0333_guard_anchor[] =
    "ReDMCSB CHEST.C F0333:30-32 returns early when G0426_T_OpenChest "
    "already equals the requested chest, so the open G0425 view remains the "
    "pickup source.";
static const char s_f0333_dispatch_anchor[] =
    "ReDMCSB CHEST.C F0333:53-67 copies linked objects into visible "
    "C537..C544/G0425_aT_ChestSlots for later in-hand pickup dispatch.";
static const char s_f0334_rewire_anchor[] =
    "ReDMCSB CHEST.C F0334:117-132 rewrites the container link list from "
    "non-empty visible G0425 slots and skips emptied pickup slots.";
static const char s_object_classify_anchor[] =
    "ReDMCSB OBJECT.C F0032/F0033:121-212 classifies object type/icons, "
    "including scroll state and charge-bearing weapon variants; this gate "
    "maps bolt and scroll sentinels to stack kinds.";
static const char s_ammo_quiver_anchor[] =
    "ReDMCSB AMMO.C F0294:54-79 dispatches bow/sling ammunition classes for "
    "quiver-compatible stackable objects.";
static const char s_blitmask_anchor[] =
    "ReDMCSB BLITMASK.C F0133:30-33 redraws masked bitmap boxes used by "
    "leader-hand and object icon refresh after pickup/merge changes.";
static const char s_source_summary[] =
    "runtime=1; contract_only=0; open chest pickup of a stackable object "
    "covers hand-busy non-stackable failover, same-stack merge, different "
    "stackable failover to a free inventory slot, and empty-hand pickup; "
    "hand preservation and F0334 close rewire are checked after each case.";

static const DM1_V1_ChestPickupStackFailoverSpecPc34 s_spec = {
    s_f0333_guard_anchor,
    s_f0333_dispatch_anchor,
    s_f0334_rewire_anchor,
    s_object_classify_anchor,
    s_ammo_quiver_anchor,
    s_blitmask_anchor,
    s_source_summary,
    4,
    2,
    DM1_PC34_SLOT_CHEST_1,
    DM1_PC34_SLOT_CHEST_2
};

static DM1_V1_ChestPickupStackFailoverProbePc34 s_last_probe;

static M11_Item make_item(int itemType,
                          int weight,
                          int charges,
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

static void record_check(int condition, int* passed, int* failed)
{
    if (condition) {
        ++*passed;
    } else {
        ++*failed;
    }
}

int dm1_v1_chest_pickup_stack_failover_stack_kind_pc34(
    const M11_Item* item)
{
    if (!item || item->itemType == 0) {
        return DM1_PC34_CHEST_STACK_KIND_NONE;
    }
    if (item->itemType == DM1_PC34_CHEST_STACK_FAILOVER_BOLT) {
        return DM1_PC34_CHEST_STACK_KIND_BOLT;
    }
    if (item->itemType == DM1_PC34_CHEST_STACK_FAILOVER_SCROLL) {
        return DM1_PC34_CHEST_STACK_KIND_SCROLL;
    }
    return DM1_PC34_CHEST_STACK_KIND_NONE;
}

static int get_item_in_source_slot(const M11_InventoryState* state,
                                   int champ,
                                   int pc34Slot,
                                   M11_Item* out)
{
    if (!state || !out) {
        return 0;
    }
    return m11_inventory_get_item_in_pc34_source_slot(
        state, champ, pc34Slot, out);
}

static int clear_chest_slot(M11_InventoryState* state,
                            int champ,
                            int chestSlotIndex)
{
    return m11_inventory_set_item_in_chest_slot(
        state, champ, chestSlotIndex, 0, 0, 0, 0);
}

static int find_first_free_backpack_slot(const M11_InventoryState* state,
                                         int champ,
                                         int* firstOut,
                                         int* secondOut,
                                         int* countOut)
{
    int first = -1;
    int second = -1;
    int count = 0;
    int pc34Slot;

    if (!state) {
        return 0;
    }
    for (pc34Slot = DM1_PC34_SLOT_BACKPACK_LINE1_1;
         pc34Slot <= DM1_PC34_SLOT_BACKPACK_LINE1_9;
         ++pc34Slot) {
        M11_Item item;

        if (!get_item_in_source_slot(state, champ, pc34Slot, &item)) {
            return 0;
        }
        if (item.itemType == 0) {
            if (first < 0) {
                first = pc34Slot;
            } else if (second < 0) {
                second = pc34Slot;
            }
            ++count;
        }
    }
    if (firstOut) {
        *firstOut = first;
    }
    if (secondOut) {
        *secondOut = second;
    }
    if (countOut) {
        *countOut = count;
    }
    return first >= 0 ? 1 : 0;
}

int dm1_v1_chest_pickup_stack_failover_click_pc34(
    M11_InventoryState* state,
    int champ,
    int chestSlotIndex,
    int* routedPc34SlotOut)
{
    M11_Item chestItem;
    M11_Item handItem;
    int chestKind;
    int handKind;
    int routedPc34Slot = -1;

    if (routedPc34SlotOut) {
        *routedPc34SlotOut = -1;
    }
    if (!state || champ < 0 || champ >= state->championCount ||
        chestSlotIndex < 0 ||
        chestSlotIndex >= DM1_PC34_CHEST_STACK_FAILOVER_SLOT_COUNT ||
        m11_inventory_get_open_chest_thing(state, champ) == 0 ||
        !m11_inventory_get_item_in_chest_slot(
            state, champ, chestSlotIndex, &chestItem) ||
        !m11_inventory_get_mouse_item(state, champ, &handItem) ||
        chestItem.itemType == 0) {
        return DM1_PC34_CHEST_STACK_OUTCOME_REJECTED;
    }

    chestKind = dm1_v1_chest_pickup_stack_failover_stack_kind_pc34(&chestItem);
    handKind = dm1_v1_chest_pickup_stack_failover_stack_kind_pc34(&handItem);

    /* ReDMCSB CHEST.C F0333:53-67 exposes the selected C537 slot, while
     * BLITMASK.C F0133:30-33 anchors the leader-hand icon redraw after the
     * default empty-hand pickup. */
    if (handItem.itemType == 0) {
        if (!m11_inventory_set_mouse_item(
                state, champ, chestItem.itemType, chestItem.weight,
                chestItem.charges, chestItem.allowedSlots) ||
            !clear_chest_slot(state, champ, chestSlotIndex)) {
            return DM1_PC34_CHEST_STACK_OUTCOME_REJECTED;
        }
        return DM1_PC34_CHEST_STACK_OUTCOME_TO_HAND;
    }

    /* ReDMCSB OBJECT.C F0032/F0033:121-212 and AMMO.C F0294:54-79 classify
     * scroll/bolt-like stackables before a same-kind hand stack absorbs the
     * visible chest stack without disturbing the previously held object id. */
    if (chestKind != DM1_PC34_CHEST_STACK_KIND_NONE &&
        handKind == chestKind &&
        handItem.itemType == chestItem.itemType) {
        if (!m11_inventory_set_mouse_item(
                state, champ, handItem.itemType,
                handItem.weight + chestItem.weight,
                handItem.charges + chestItem.charges,
                handItem.allowedSlots) ||
            !clear_chest_slot(state, champ, chestSlotIndex)) {
            return DM1_PC34_CHEST_STACK_OUTCOME_REJECTED;
        }
        return DM1_PC34_CHEST_STACK_OUTCOME_MERGED_HAND;
    }

    /* ReDMCSB CHEST.C F0333:30-32 keeps the already-open chest panel stable
     * while an incompatible occupied hand sends the picked stackable into
     * inventory instead of overwriting the held object; F0334:117-132 later
     * rewires the remaining visible chest sentinel around the empty C537. */
    if (chestKind != DM1_PC34_CHEST_STACK_KIND_NONE &&
        find_first_free_backpack_slot(state, champ, &routedPc34Slot,
                                      NULL, NULL)) {
        if (!m11_inventory_set_item_in_pc34_source_slot(
                state, champ, routedPc34Slot, chestItem.itemType,
                chestItem.weight, chestItem.charges,
                chestItem.allowedSlots) ||
            !clear_chest_slot(state, champ, chestSlotIndex)) {
            return DM1_PC34_CHEST_STACK_OUTCOME_REJECTED;
        }
        if (routedPc34SlotOut) {
            *routedPc34SlotOut = routedPc34Slot;
        }
        return DM1_PC34_CHEST_STACK_OUTCOME_ROUTED_FREE_SLOT;
    }

    return DM1_PC34_CHEST_STACK_OUTCOME_REJECTED;
}

static int fill_backpack_except_two_free(M11_InventoryState* state,
                                         int champ)
{
    int pc34Slot;

    for (pc34Slot = DM1_PC34_SLOT_BACKPACK_LINE1_1;
         pc34Slot <= DM1_PC34_SLOT_BACKPACK_LINE1_9;
         ++pc34Slot) {
        if (pc34Slot == DM1_PC34_CHEST_STACK_FAILOVER_FREE_A ||
            pc34Slot == DM1_PC34_CHEST_STACK_FAILOVER_FREE_B) {
            continue;
        }
        if (!m11_inventory_set_item_in_pc34_source_slot(
                state, champ, pc34Slot,
                DM1_PC34_CHEST_STACK_FAILOVER_PACKED + pc34Slot,
                1, 0, DM1_PC34_ALLOWED_ANY_SLOT)) {
            return 0;
        }
    }
    return 1;
}

static int closed_contains(const M11_Item* closed, int count, int itemType)
{
    int i;

    if (!closed || count < 0 || itemType == 0) {
        return 0;
    }
    for (i = 0; i < count; ++i) {
        if (closed[i].itemType == itemType) {
            return 1;
        }
    }
    return 0;
}

static int run_case(DM1_V1_ChestPickupStackFailoverCasePc34* out,
                    int handItemType,
                    int handWeight,
                    int handCharges,
                    int wantOutcome)
{
    M11_InventoryState state;
    M11_Item linked[2];
    M11_Item handBefore;
    M11_Item chestBefore;
    M11_Item sentinelBefore;
    M11_Item routedBefore;
    M11_Item routedAfter;
    M11_Item secondFreeAfter;
    M11_Item handAfter;
    M11_Item chestAfter;
    M11_Item sentinelAfter;
    M11_Item closed[DM1_PC34_CHEST_STACK_FAILOVER_SLOT_COUNT];
    int routedSlot = -1;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    memset(closed, 0, sizeof(closed));

    linked[0] = make_item(DM1_PC34_CHEST_STACK_FAILOVER_BOLT, 3, 3,
                          DM1_PC34_ALLOWED_ANY_SLOT);
    linked[1] = make_item(DM1_PC34_CHEST_STACK_FAILOVER_SENTINEL, 5, 0,
                          DM1_PC34_ALLOWED_CONTAINER);

    m11_inventory_init(&state, 1);
    out->setupResult = fill_backpack_except_two_free(&state, 0);
    if (!out->setupResult) {
        return 0;
    }
    if (handItemType != 0 &&
        !m11_inventory_set_mouse_item(&state, 0, handItemType, handWeight,
                                      handCharges,
                                      DM1_PC34_ALLOWED_ANY_SLOT)) {
        return 0;
    }

    out->openResult = m11_inventory_open_chest(
        &state, 0, DM1_PC34_CHEST_STACK_FAILOVER_CHEST_THING, linked, 2);
    out->openThing = m11_inventory_get_open_chest_thing(&state, 0);
    if (!out->openResult ||
        !m11_inventory_get_mouse_item(&state, 0, &handBefore) ||
        !m11_inventory_get_item_in_chest_slot(&state, 0, 0, &chestBefore) ||
        !m11_inventory_get_item_in_chest_slot(&state, 0, 1,
                                              &sentinelBefore) ||
        !find_first_free_backpack_slot(&state, 0, &out->firstFreeSlot,
                                       &out->secondFreeSlot,
                                       &out->freeSlotCountBefore) ||
        !get_item_in_source_slot(&state, 0, out->firstFreeSlot,
                                 &routedBefore)) {
        return 0;
    }

    out->handBusyBefore = handBefore.itemType != 0 ? 1 : 0;
    out->handBeforeType = handBefore.itemType;
    out->handBeforeWeight = handBefore.weight;
    out->handBeforeCharges = handBefore.charges;
    out->handBeforeStackKind =
        dm1_v1_chest_pickup_stack_failover_stack_kind_pc34(&handBefore);
    out->chestBeforeType = chestBefore.itemType;
    out->chestBeforeWeight = chestBefore.weight;
    out->chestBeforeCharges = chestBefore.charges;
    out->chestBeforeStackKind =
        dm1_v1_chest_pickup_stack_failover_stack_kind_pc34(&chestBefore);
    out->sameStackBefore =
        out->handBeforeStackKind != DM1_PC34_CHEST_STACK_KIND_NONE &&
        out->handBeforeStackKind == out->chestBeforeStackKind &&
        out->handBeforeType == out->chestBeforeType ? 1 : 0;
    out->routedSlotBeforeType = routedBefore.itemType;
    out->sentinelBeforeType = sentinelBefore.itemType;
    out->loadBeforePickup = m11_inventory_get_load(&state, 0);

    out->pickupResult =
        dm1_v1_chest_pickup_stack_failover_click_pc34(
            &state, 0, 0, &routedSlot);
    out->outcome = out->pickupResult;
    out->routedSlot = routedSlot;

    if (!m11_inventory_get_mouse_item(&state, 0, &handAfter) ||
        !m11_inventory_get_item_in_chest_slot(&state, 0, 0, &chestAfter) ||
        !m11_inventory_get_item_in_chest_slot(&state, 0, 1,
                                              &sentinelAfter) ||
        !get_item_in_source_slot(&state, 0, out->firstFreeSlot,
                                 &routedAfter) ||
        !get_item_in_source_slot(&state, 0, out->secondFreeSlot,
                                 &secondFreeAfter)) {
        return 0;
    }

    out->routedSlotAfterType = routedAfter.itemType;
    out->routedSlotAfterWeight = routedAfter.weight;
    out->routedSlotAfterCharges = routedAfter.charges;
    out->secondFreeSlotAfterType = secondFreeAfter.itemType;
    out->handAfterType = handAfter.itemType;
    out->handAfterWeight = handAfter.weight;
    out->handAfterCharges = handAfter.charges;
    out->handAfterStackKind =
        dm1_v1_chest_pickup_stack_failover_stack_kind_pc34(&handAfter);
    out->chestAfterType = chestAfter.itemType;
    out->chestAfterWeight = chestAfter.weight;
    out->chestAfterCharges = chestAfter.charges;
    out->sentinelAfterType = sentinelAfter.itemType;
    out->visibleWeightAfterPickup =
        m11_inventory_pc34_open_chest_visible_contents_weight(&state, 0);
    out->loadAfterPickup = m11_inventory_get_load(&state, 0);

    out->closeCount = m11_inventory_pc34_close_chest_with_weight_snapshot(
        &state, 0, closed,
        DM1_PC34_CHEST_STACK_FAILOVER_SLOT_COUNT, NULL);
    if (out->closeCount < 0) {
        return 0;
    }
    out->closedFirstType =
        out->closeCount > 0 ? closed[0].itemType : 0;
    out->closedContainsPickedType =
        closed_contains(closed, out->closeCount,
                        DM1_PC34_CHEST_STACK_FAILOVER_BOLT);
    out->closedContainsHeldType =
        closed_contains(closed, out->closeCount, handBefore.itemType);
    out->handPreserved =
        handItemType == 0 ? 1 :
        handAfter.itemType == handBefore.itemType ? 1 : 0;
    out->pickedRemovedFromChest =
        chestAfter.itemType == 0 && !out->closedContainsPickedType ? 1 : 0;
    out->pickedPreservedSomewhere =
        (wantOutcome == DM1_PC34_CHEST_STACK_OUTCOME_TO_HAND &&
         handAfter.itemType == DM1_PC34_CHEST_STACK_FAILOVER_BOLT) ||
        (wantOutcome == DM1_PC34_CHEST_STACK_OUTCOME_MERGED_HAND &&
         handAfter.itemType == DM1_PC34_CHEST_STACK_FAILOVER_BOLT &&
         handAfter.charges == handCharges + linked[0].charges) ||
        (wantOutcome == DM1_PC34_CHEST_STACK_OUTCOME_ROUTED_FREE_SLOT &&
         routedAfter.itemType == DM1_PC34_CHEST_STACK_FAILOVER_BOLT);
    out->sentinelPreserved =
        sentinelAfter.itemType == DM1_PC34_CHEST_STACK_FAILOVER_SENTINEL &&
        out->closedFirstType == DM1_PC34_CHEST_STACK_FAILOVER_SENTINEL;
    out->secondFreeSlotStayedFree =
        secondFreeAfter.itemType == 0 ? 1 : 0;

    return out->pickupResult == wantOutcome ? 1 : 0;
}

const char*
dm1_v1_chest_pickup_stack_failover_source_evidence_pc34(void)
{
    return s_source_summary;
}

const DM1_V1_ChestPickupStackFailoverSpecPc34*
dm1_v1_chest_pickup_stack_failover_spec_pc34(void)
{
    return &s_spec;
}

const DM1_V1_ChestPickupStackFailoverProbePc34*
dm1_v1_chest_pickup_stack_failover_last_probe_pc34(void)
{
    return &s_last_probe;
}

int dm1_v1_chest_pickup_stack_failover_run(int* passed, int* failed)
{
    int localPassed = 0;
    int localFailed = 0;
    int ok;

    if (passed) {
        *passed = 0;
    }
    if (failed) {
        *failed = 0;
    }
    memset(&s_last_probe, 0, sizeof(s_last_probe));

    ok = run_case(&s_last_probe.nonStackHand,
                  DM1_PC34_CHEST_STACK_FAILOVER_NON_STACK, 12, 0,
                  DM1_PC34_CHEST_STACK_OUTCOME_ROUTED_FREE_SLOT);
    record_check(ok, &localPassed, &localFailed);
    record_check(s_last_probe.nonStackHand.handPreserved,
                 &localPassed, &localFailed);
    record_check(s_last_probe.nonStackHand.pickedPreservedSomewhere,
                 &localPassed, &localFailed);

    ok = run_case(&s_last_probe.sameStackHand,
                  DM1_PC34_CHEST_STACK_FAILOVER_BOLT, 4, 4,
                  DM1_PC34_CHEST_STACK_OUTCOME_MERGED_HAND);
    record_check(ok, &localPassed, &localFailed);
    record_check(s_last_probe.sameStackHand.sameStackBefore,
                 &localPassed, &localFailed);
    record_check(s_last_probe.sameStackHand.handAfterCharges == 7,
                 &localPassed, &localFailed);

    ok = run_case(&s_last_probe.differentStackHand,
                  DM1_PC34_CHEST_STACK_FAILOVER_SCROLL, 2, 1,
                  DM1_PC34_CHEST_STACK_OUTCOME_ROUTED_FREE_SLOT);
    record_check(ok, &localPassed, &localFailed);
    record_check(s_last_probe.differentStackHand.handPreserved,
                 &localPassed, &localFailed);
    record_check(s_last_probe.differentStackHand.pickedPreservedSomewhere,
                 &localPassed, &localFailed);

    ok = run_case(&s_last_probe.emptyHand, 0, 0, 0,
                  DM1_PC34_CHEST_STACK_OUTCOME_TO_HAND);
    record_check(ok, &localPassed, &localFailed);
    record_check(s_last_probe.emptyHand.handAfterType ==
                 DM1_PC34_CHEST_STACK_FAILOVER_BOLT,
                 &localPassed, &localFailed);
    record_check(s_last_probe.emptyHand.pickedPreservedSomewhere,
                 &localPassed, &localFailed);

    record_check(s_last_probe.nonStackHand.freeSlotCountBefore >= 2,
                 &localPassed, &localFailed);
    record_check(s_last_probe.differentStackHand.secondFreeSlotStayedFree,
                 &localPassed, &localFailed);
    record_check(s_last_probe.emptyHand.sentinelPreserved,
                 &localPassed, &localFailed);

    if (passed) {
        *passed = localPassed;
    }
    if (failed) {
        *failed = localFailed;
    }
    return localFailed == 0 ? 1 : 0;
}
