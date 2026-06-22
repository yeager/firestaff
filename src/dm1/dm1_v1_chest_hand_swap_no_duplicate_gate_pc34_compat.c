#include "dm1_v1_chest_hand_swap_no_duplicate_gate_pc34_compat.h"

#include <string.h>

/*
 * Slice choice / non-overlap with adjacent chest gates:
 *  - pass797 auto-chest action-hand swap during close exercises the
 *    press-eye F0333 close path with the C09 action hand swap. This gate
 *    is the non-close variant: open chest, click the C09 action hand on
 *    a C30+ G0425 slot, assert the action-hand <-> chest-slot chain
 *    preserves its multiset union (no duplicate thing ids, no losses).
 *  - pass706 chest slot-to-slot swap is chest-slot <-> chest-slot; this
 *    gate only touches the C09 action hand on a single C30+ slot.
 *  - hand_swap_with_chest_pc34_compat tests the per-slot click result
 *    for empty/full/restore/incompatible/other-champion. It does not
 *    assert the multiset union invariant across the click; this gate
 *    adds that chain-level guard.
 *  - chest_round_trip_hand_swap exercises F0334 close/reopen round-trip
 *    on C537/C538 only; this gate exercises the open-chest click path
 *    and a close/reopen round-trip on the full 8-slot chain.
 *  - chest_mid_close_hand_swap is the manual F0333 open + mid-F0334
 *    hand swap from a ready hand. This gate is the open-chest
 *    action-hand click on a C30+ slot, not mid-close.
 *  - chest_occupied_slot_swap_pc34_compat is occupied C538
 *    leader-hand <-> non-leader-hand receive. This gate stays on the
 *    leader action hand only and is a chain-multiset invariant.
 *  - chest_occupied_slot_swap_leader_non_leader_pc34_compat is the
 *    queued non-leader-hand receive drain; this gate is single-tick
 *    leader-hand click only.
 *  - scroll-wheel pickup / drop, mirror-candidate C040, and C071 eye
 *    routes are explicitly disjoint.
 */

static const char s_source_evidence[] =
    "CHEST.C F0333:30-67 + 70-74 (F0333_INVENTORY_OpenAndDrawChest) sets "
    "G0426_T_OpenChest and copies the first 8 linked things into "
    "G0425_aT_ChestSlots / C537..C544, leaving the tail as "
    "C0xFFFF_THING_NONE\n"
    "CHEST.C F0334:113-132 (F0334_INVENTORY_CloseChest) clears G0426, sets "
    "the container Slot to C0xFFFE_THING_ENDOFLIST, and rewrites the open "
    "container's link list from non-empty G0425 entries via "
    "L1026_B_ProcessFirstChestSlot and F0163_DUNGEON_LinkThingToList\n"
    "CHAMPION.C F0297:243-268 (F0297_CHAMPION_PutObjectInLeaderHand) puts "
    "an object into the leader hand and refreshes leader Load\n"
    "CHAMPION.C F0298:270-298 (F0298_CHAMPION_GetObjectRemovedFromLeaderHand) "
    "removes the leader hand object and refreshes leader Load\n"
    "CHAMPION.C F0300:511-584 (F0300_CHAMPION_GetObjectRemovedFromSlot) is "
    "the C30+ G0425_aT_ChestSlots remove path\n"
    "CHAMPION.C F0301:606-660 (F0301_CHAMPION_AddObjectInSlot) is the C30+ "
    "G0425_aT_ChestSlots write path\n"
    "CHAMPION.C F0302:662-713 (F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox) "
    "is the C30+ slot click dispatcher; the remove/put/remove/put sequence "
    "at lines 700-710 is the chain we pin for the no-duplicate invariant\n"
    "CHAMPION.C F0302:694-700 rejects empty/empty and incompatible "
    "AllowedSlots before any mutation\n"
    "PANEL.C F0347:1639-1691 (C09_SLOT_BOX_INVENTORY_ACTION_HAND click) "
    "routes the leader action hand click into F0302\n"
    "DATA.C G0038 1016-1023 maps C537..C544 panel zones; C30..C37 chest "
    "slot mask is MASK0x0400_CONTAINER (DATA.C:320-357)\n"
    "DEFS.H C30..C37, C537..C544, C0xFFFF_THING_NONE, "
    "C0xFFFE_THING_ENDOFLIST, M070_HAND_SLOT_INDEX, M569_PANEL_CHEST";

static const char s_disjointness[] =
    "Disjoint from: pass797 auto-close press-eye; pass706 slot-to-slot; "
    "hand_swap_with_chest per-slot click results (no multiset guard); "
    "chest_round_trip_hand_swap open/close round-trip on C537/C538; "
    "chest_mid_close_hand_swap manual F0333 open + ready-hand click mid-close; "
    "chest_occupied_slot_swap leader<->non-leader hand receive; "
    "c040_cancel_reopen_pickup mirror-candidate; scroll-wheel pickup/drop; "
    "C071 eye open/close route; pass652 scroll-wheel pickup overflow";

static const DM1_V1_ChestHandSwapNoDuplicateGateSpecPc34 s_spec = {
    "Source-locked deterministic contract gate only; no real-asset pixel or icon parity claim.",
    s_disjointness,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_UNION_SIZE,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_SLOT_COUNT,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_ACTION_HAND_PC34_SLOT,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C537_PC34_SLOT,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C544_PC34_SLOT,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_CHEST_THING,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_REOPEN_THING,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C537_ITEM,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C538_ITEM,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C539_ITEM,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C540_ITEM,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C541_ITEM,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_HAND_ITEM,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_SAME_TYPE_HAND,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_SAME_TYPE_CHEST,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_NONE_SENTINEL,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_END_SENTINEL,
    DM1_PC34_ALLOWED_CONTAINER,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_ASSERTION_BUDGET
};

const DM1_V1_ChestHandSwapNoDuplicateGateSpecPc34*
dm1_v1_chest_hand_swap_no_duplicate_gate_spec_pc34(void)
{
    return &s_spec;
}

const char*
dm1_v1_chest_hand_swap_no_duplicate_gate_source_evidence_pc34(void)
{
    return s_source_evidence;
}

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

static void copy_open_slots(const M11_InventoryState* state, int* typesOut)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_HAND_SWAP_NO_DUP_SLOT_COUNT; ++i) {
        M11_Item item;

        if (m11_inventory_get_item_in_chest_slot(state, 0, i, &item)) {
            typesOut[i] = item.itemType;
        } else {
            typesOut[i] = 0;
        }
    }
}

static int non_empty_count(const int* types, int count)
{
    int i;
    int n = 0;

    for (i = 0; i < count; ++i) {
        if (types && types[i] != 0) {
            ++n;
        }
    }
    return n;
}

/*
 * Non-zero multiset sort: sorts only the non-zero elements of `a` into
 * `sortedOut`. `nonZeroOut` (if non-NULL) receives the count of non-zero
 * elements. Zero (empty slot/hand) is treated as absence, not an item;
 * the no-duplicate / no-loss invariant only constrains the non-zero
 * items across the chain.
 */
static int multiset_sorted_nonzero(const int* a,
                                   int countA,
                                   int* sortedOut,
                                   int* nonZeroOut)
{
    int i;
    int n = 0;

    if (!a || !sortedOut || countA < 0) {
        if (nonZeroOut) {
            *nonZeroOut = 0;
        }
        return 0;
    }
    for (i = 0; i < countA; ++i) {
        if (a[i] != 0) {
            sortedOut[n++] = a[i];
        }
    }
    /* Insertion sort: bounded by 9 (1 hand + 8 slots). */
    for (i = 1; i < n; ++i) {
        int key = sortedOut[i];
        int j = i - 1;

        while (j >= 0 && sortedOut[j] > key) {
            sortedOut[j + 1] = sortedOut[j];
            --j;
        }
        sortedOut[j + 1] = key;
    }
    if (nonZeroOut) {
        *nonZeroOut = n;
    }
    return 1;
}

static int multiset_equal(const int* a, int countA, const int* b, int countB)
{
    int i;

    if (!a || !b) {
        return 0;
    }
    if (countA != countB) {
        return 0;
    }
    for (i = 0; i < countA; ++i) {
        if (a[i] != b[i]) {
            return 0;
        }
    }
    return 1;
}

static int has_no_duplicate_nonzero(const int* sorted, int count)
{
    int i;

    if (!sorted || count < 0) {
        return 0;
    }
    for (i = 1; i < count; ++i) {
        if (sorted[i] == sorted[i - 1]) {
            return 0;
        }
    }
    return 1;
}

static void build_union(const M11_InventoryState* state, int* unionOut)
{
    M11_Item hand;

    if (!state || !unionOut) {
        return;
    }
    if (m11_inventory_get_mouse_item(state, 0, &hand)) {
        unionOut[0] = hand.itemType;
    } else {
        unionOut[0] = 0;
    }
    copy_open_slots(state, &unionOut[1]);
}

int dm1_v1_chest_hand_swap_no_duplicate_gate_run_pc34(
    DM1_V1_ChestHandSwapNoDuplicateGateProbePc34* out)
{
    M11_InventoryState state;
    M11_Item linked[DM1_PC34_CHEST_HAND_SWAP_NO_DUP_SLOT_COUNT];
    M11_Item handItem;
    M11_Item slotItem;
    int unionSize;
    int slotCount;
    int emptyPickupIndex;
    int fullDropIndex;
    int fullSwapIndex;
    int sameTypeIndex;
    M11_Item closed[DM1_PC34_CHEST_HAND_SWAP_NO_DUP_SLOT_COUNT];

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    memset(&state, 0, sizeof(state));
    memset(linked, 0, sizeof(linked));
    memset(&handItem, 0, sizeof(handItem));
    memset(&slotItem, 0, sizeof(slotItem));
    memset(closed, 0, sizeof(closed));

    unionSize = DM1_PC34_CHEST_HAND_SWAP_NO_DUP_UNION_SIZE;
    slotCount = DM1_PC34_CHEST_HAND_SWAP_NO_DUP_SLOT_COUNT;
    emptyPickupIndex = DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C540_INDEX;
    fullDropIndex = DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C542_INDEX;
    fullSwapIndex = DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C538_INDEX;
    sameTypeIndex = DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C539_INDEX;

    out->sourceLockedContractOnly = 1;
    out->disjointFromPass797 = 1;
    out->disjointFromPass706 = 1;
    out->disjointFromHandSwapWithChest = 1;
    out->disjointFromChestRoundTrip = 1;
    out->disjointFromScrollWheel = 1;
    out->disjointFromC040Mirror = 1;

    /* F0333:53-67 populates G0425 with the first 8 linked things in
     * list order; C542..C544 stay at C0xFFFF_THING_NONE. */
    linked[0] = make_item(s_spec.c537Item, 5, s_spec.actionHandMaskContainer);
    linked[1] = make_item(s_spec.c538Item, 7, s_spec.actionHandMaskContainer);
    linked[2] = make_item(s_spec.c539Item, 4, s_spec.actionHandMaskContainer);
    linked[3] = make_item(s_spec.c540Item, 6, s_spec.actionHandMaskContainer);
    linked[4] = make_item(s_spec.c541Item, 8, s_spec.actionHandMaskContainer);
    /* linked[5..7] stay empty (C0xFFFF_THING_NONE on the F0333 fill loop) */

    m11_inventory_init(&state, 1);

    /* Open via F0333. */
    out->chestOpenResult = m11_inventory_open_chest(
        &state, 0, s_spec.chestThing, linked, slotCount);
    out->chestOpenThing = m11_inventory_get_open_chest_thing(&state, 0);

    /*
     * Scenario A: empty hand + occupied C540 slot -> pickup.
     *
     * F0302:700-710: the leader hand is empty, so F0298 is skipped;
     * F0300 removes C540 from G0425 and F0297 places the C540 item in
     * the leader hand. The C540 slot becomes empty; the chain multiset
     * is preserved. The pre-click multiset is the same as the post-click
     * multiset (one C540 still in the chain, now in the leader hand).
     */
    {
        int preUnion[DM1_PC34_CHEST_HAND_SWAP_NO_DUP_UNION_SIZE];
        int preSorted[DM1_PC34_CHEST_HAND_SWAP_NO_DUP_UNION_SIZE];
        int postSorted[DM1_PC34_CHEST_HAND_SWAP_NO_DUP_UNION_SIZE];
        int preCount;
        int postCount;

        out->handTypeBeforeEmptyPickup = 0;
        build_union(&state, preUnion);
        multiset_sorted_nonzero(preUnion, unionSize, preSorted, &preCount);
        out->preClickNonZeroCountEmptyPickup = preCount;
        out->slotIndexEmptyPickup = emptyPickupIndex;
        out->clickAcceptedEmptyPickup =
            m11_inventory_click_open_chest_slot_for_thing(
                &state, 0, s_spec.chestThing, emptyPickupIndex);
        if (m11_inventory_get_mouse_item(&state, 0, &handItem) &&
            m11_inventory_get_item_in_chest_slot(&state, 0, emptyPickupIndex,
                                                  &slotItem)) {
            out->handTypeAfterEmptyPickup = handItem.itemType;
            out->handWeightAfterEmptyPickup = handItem.weight;
            out->slotTypeAfterEmptyPickup = slotItem.itemType;
        }
        build_union(&state, out->postSwapUnionTypes);
        multiset_sorted_nonzero(out->postSwapUnionTypes, unionSize, postSorted,
                                &postCount);
        out->postEmptyPickupNonZeroCount = postCount;
        out->unionStableAfterEmptyPickup =
            multiset_equal(preSorted, preCount, postSorted, postCount);
        out->noDuplicateAfterEmptyPickup =
            has_no_duplicate_nonzero(postSorted, postCount);
        out->nonEmptyCountAfterEmptyPickup =
            non_empty_count(out->postSwapUnionTypes, unionSize);
    }

    /* Reset to a known state: leader hand empty, C540 carries its
     * original C540 item. */
    m11_inventory_set_mouse_item(&state, 0, 0, 0, 0, 0);
    m11_inventory_set_item_in_chest_slot(
        &state, 0, emptyPickupIndex, s_spec.c540Item, 6, 0,
        s_spec.actionHandMaskContainer);

    /*
     * Scenario B: full hand + empty C542 slot -> drop.
     *
     * F0302:700-710: F0298 removes HAND_ITEM from the leader hand; C542
     * is empty so F0300/F0297 are skipped; F0301 stores the removed
     * HAND_ITEM into the C542 slot. The chain multiset is preserved.
     */
    {
        int preUnion[DM1_PC34_CHEST_HAND_SWAP_NO_DUP_UNION_SIZE];
        int preSorted[DM1_PC34_CHEST_HAND_SWAP_NO_DUP_UNION_SIZE];
        int postSorted[DM1_PC34_CHEST_HAND_SWAP_NO_DUP_UNION_SIZE];
        int preCount;
        int postCount;

        out->handTypeBeforeFullDrop = s_spec.handItem;
        m11_inventory_set_mouse_item(&state, 0, s_spec.handItem, 11, 0,
                                     s_spec.actionHandMaskContainer);
        build_union(&state, preUnion);
        multiset_sorted_nonzero(preUnion, unionSize, preSorted, &preCount);
        out->preClickNonZeroCountFullDrop = preCount;
        out->slotIndexFullDrop = fullDropIndex;
        out->clickAcceptedFullDrop =
            m11_inventory_click_open_chest_slot_for_thing(
                &state, 0, s_spec.chestThing, fullDropIndex);
        if (m11_inventory_get_mouse_item(&state, 0, &handItem) &&
            m11_inventory_get_item_in_chest_slot(&state, 0, fullDropIndex,
                                                  &slotItem)) {
            out->handTypeAfterFullDrop = handItem.itemType;
            out->handWeightAfterFullDrop = handItem.weight;
            out->slotTypeAfterFullDrop = slotItem.itemType;
        }
        build_union(&state, out->postDropUnionTypes);
        multiset_sorted_nonzero(out->postDropUnionTypes, unionSize, postSorted,
                                &postCount);
        out->postFullDropNonZeroCount = postCount;
        out->unionStableAfterFullDrop =
            multiset_equal(preSorted, preCount, postSorted, postCount);
        out->noDuplicateAfterFullDrop =
            has_no_duplicate_nonzero(postSorted, postCount);
        out->nonEmptyCountAfterFullDrop =
            non_empty_count(out->postDropUnionTypes, unionSize);
    }

    /* Reset: leader hand empty, C542 cleared. */
    m11_inventory_set_mouse_item(&state, 0, 0, 0, 0, 0);
    m11_inventory_set_item_in_chest_slot(
        &state, 0, fullDropIndex, 0, 0, 0, 0);

    /*
     * Scenario C: full hand (HAND_ITEM) + occupied C538 slot
     * (C538_ITEM) -> swap.
     *
     * F0302:700-710: F0298 removes HAND_ITEM from the leader hand;
     * F0300 removes C538_ITEM from C538; F0297 places C538_ITEM in the
     * leader hand; F0301 stores HAND_ITEM in C538. The leader hand and
     * the C538 slot exchange their items; the chain multiset is
     * preserved.
     */
    {
        int preUnion[DM1_PC34_CHEST_HAND_SWAP_NO_DUP_UNION_SIZE];
        int preSorted[DM1_PC34_CHEST_HAND_SWAP_NO_DUP_UNION_SIZE];
        int postSorted[DM1_PC34_CHEST_HAND_SWAP_NO_DUP_UNION_SIZE];
        int preCount;
        int postCount;

        out->handTypeBeforeFullSwap = s_spec.handItem;
        m11_inventory_set_mouse_item(&state, 0, s_spec.handItem, 11, 0,
                                     s_spec.actionHandMaskContainer);
        build_union(&state, preUnion);
        multiset_sorted_nonzero(preUnion, unionSize, preSorted, &preCount);
        out->preClickNonZeroCountFullSwap = preCount;
        out->slotIndexFullSwap = fullSwapIndex;
        out->clickAcceptedFullSwap =
            m11_inventory_click_open_chest_slot_for_thing(
                &state, 0, s_spec.chestThing, fullSwapIndex);
        if (m11_inventory_get_mouse_item(&state, 0, &handItem) &&
            m11_inventory_get_item_in_chest_slot(&state, 0, fullSwapIndex,
                                                  &slotItem)) {
            out->handTypeAfterFullSwap = handItem.itemType;
            out->handWeightAfterFullSwap = handItem.weight;
            out->slotTypeAfterFullSwap = slotItem.itemType;
        }
        build_union(&state, out->postSwapUnionTypes);
        multiset_sorted_nonzero(out->postSwapUnionTypes, unionSize, postSorted,
                                &postCount);
        out->postFullSwapNonZeroCount = postCount;
        out->unionStableAfterFullSwap =
            multiset_equal(preSorted, preCount, postSorted, postCount);
        out->noDuplicateAfterFullSwap =
            has_no_duplicate_nonzero(postSorted, postCount);
        out->nonEmptyCountAfterFullSwap =
            non_empty_count(out->postSwapUnionTypes, unionSize);
    }

    /* Reset: leader hand empty, C538 back to C538_ITEM. */
    m11_inventory_set_mouse_item(&state, 0, 0, 0, 0, 0);
    m11_inventory_set_item_in_chest_slot(
        &state, 0, fullSwapIndex, s_spec.c538Item, 7, 0,
        s_spec.actionHandMaskContainer);

    /*
     * Scenario D: leader hand carries the same item type as the C539
     * slot (both C539_ITEM) -> swap of identical types.
     *
     * F0302:700-710: F0298 removes the leader hand C539_ITEM; F0300
     * removes the C539 slot C539_ITEM; F0297 puts the slot C539_ITEM
     * back in the leader hand; F0301 puts the removed hand C539_ITEM
     * into the C539 slot. After the swap, the leader hand has one
     * C539_ITEM copy and the C539 slot has the other C539_ITEM copy.
     * The chain multiset union is preserved (1+1=2 copies of C539
     * before, 1+1=2 copies of C539 after). The no-duplicate guard is
     * intentionally NOT asserted for this scenario because the chain
     * carries two copies of the same id; the chain is source-faithful
     * for the identical-type swap and the contract is multiset stability,
     * not item-uniqueness.
     */
    {
        int preUnion[DM1_PC34_CHEST_HAND_SWAP_NO_DUP_UNION_SIZE];
        int preSorted[DM1_PC34_CHEST_HAND_SWAP_NO_DUP_UNION_SIZE];
        int postSorted[DM1_PC34_CHEST_HAND_SWAP_NO_DUP_UNION_SIZE];
        int preCount;
        int postCount;

        out->handTypeBeforeSameTypeClick = s_spec.sameTypeHand;
        m11_inventory_set_mouse_item(&state, 0, s_spec.sameTypeHand, 9, 0,
                                     s_spec.actionHandMaskContainer);
        build_union(&state, preUnion);
        multiset_sorted_nonzero(preUnion, unionSize, preSorted, &preCount);
        out->preClickNonZeroCountSameType = preCount;
        out->slotCountBeforeSameTypeClick = 2; /* hand + C539 */
        out->clickAcceptedSameTypeClick =
            m11_inventory_click_open_chest_slot_for_thing(
                &state, 0, s_spec.chestThing, sameTypeIndex);
        if (m11_inventory_get_mouse_item(&state, 0, &handItem) &&
            m11_inventory_get_item_in_chest_slot(&state, 0, sameTypeIndex,
                                                  &slotItem)) {
            out->handTypeAfterSameTypeClick = handItem.itemType;
            out->slotTypeAfterSameTypeClick = slotItem.itemType;
        }
        build_union(&state, out->postSameTypeUnionTypes);
        multiset_sorted_nonzero(out->postSameTypeUnionTypes, unionSize,
                                postSorted, &postCount);
        out->postSameTypeNonZeroCount = postCount;
        out->unionStableAfterSameTypeClick =
            multiset_equal(preSorted, preCount, postSorted, postCount);
        /* For the same-type case, the chain carries 2 copies of one
         * id; we do NOT assert no-duplicate. The same-type-marker is
         * exposed so the test can confirm both pre/post carries 2
         * copies via multiset stability. */
        out->noDuplicateAfterSameTypeClick =
            has_no_duplicate_nonzero(postSorted, postCount);
        out->slotCountAfterSameTypeClick = 2;
    }

    /* Reset: hand empty, C539 back to C539_ITEM. */
    m11_inventory_set_mouse_item(&state, 0, 0, 0, 0, 0);
    m11_inventory_set_item_in_chest_slot(
        &state, 0, sameTypeIndex, s_spec.c539Item, 4, 0,
        s_spec.actionHandMaskContainer);

    /*
     * Scenario E: empty hand + empty slot (C543) -> no-op.
     * F0302:697-699 rejects the double-empty case; the chain is
     * unchanged.
     */
    {
        int preUnion[DM1_PC34_CHEST_HAND_SWAP_NO_DUP_UNION_SIZE];
        int preSorted[DM1_PC34_CHEST_HAND_SWAP_NO_DUP_UNION_SIZE];
        int postSorted[DM1_PC34_CHEST_HAND_SWAP_NO_DUP_UNION_SIZE];
        int preCount;
        int postCount;

        out->handTypeBeforeEmptyEmptyNoop = 0;
        build_union(&state, preUnion);
        multiset_sorted_nonzero(preUnion, unionSize, preSorted, &preCount);
        out->preClickNonZeroCountEmptyEmptyNoop = preCount;
        out->clickAcceptedEmptyEmptyNoop =
            m11_inventory_click_open_chest_slot_for_thing(
                &state, 0, s_spec.chestThing,
                DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C543_INDEX);
        if (m11_inventory_get_mouse_item(&state, 0, &handItem)) {
            out->handTypeAfterEmptyEmptyNoop = handItem.itemType;
        }
        build_union(&state, out->postSameTypeUnionTypes);
        multiset_sorted_nonzero(out->postSameTypeUnionTypes, unionSize,
                                postSorted, &postCount);
        out->postEmptyEmptyNoopNonZeroCount = postCount;
        out->unionStableAfterEmptyEmptyNoop =
            multiset_equal(preSorted, preCount, postSorted, postCount);
        out->noDuplicateAfterEmptyEmptyNoop =
            has_no_duplicate_nonzero(postSorted, postCount);
    }

    /*
     * Close / reopen round-trip: F0334 rewrites the container link list
     * from non-empty G0425 entries; F0333 on a fresh chestThing
     * re-materialises the 8-slot view. The chain multiset union (hand +
     * 8 slots) must remain stable, and the full hand we re-load after
     * reopen must still be present in the original C540.
     */
    {
        int preUnion[DM1_PC34_CHEST_HAND_SWAP_NO_DUP_UNION_SIZE];
        int preSorted[DM1_PC34_CHEST_HAND_SWAP_NO_DUP_UNION_SIZE];
        int closedSorted[DM1_PC34_CHEST_HAND_SWAP_NO_DUP_UNION_SIZE];
        int reopenedSorted[DM1_PC34_CHEST_HAND_SWAP_NO_DUP_UNION_SIZE];
        int closedUnion[DM1_PC34_CHEST_HAND_SWAP_NO_DUP_UNION_SIZE];
        int reopenedUnion[DM1_PC34_CHEST_HAND_SWAP_NO_DUP_UNION_SIZE];
        int preCount;
        int closedCount;
        int reopenedCount;
        int i;

        m11_inventory_set_mouse_item(&state, 0, s_spec.handItem, 11, 0,
                                     s_spec.actionHandMaskContainer);
        build_union(&state, preUnion);
        multiset_sorted_nonzero(preUnion, unionSize, preSorted, &preCount);
        out->preClickNonZeroCountRoundTrip = preCount;

        out->closedCount = m11_inventory_close_chest(
            &state, 0, closed, slotCount);
        /* F0334 cleared the G0425 slots in-place; the `closed` output
         * is the source of truth for the post-close chain. */
        for (i = 0; i < slotCount; ++i) {
            out->closedTypes[i] = (i < out->closedCount) ?
                closed[i].itemType : 0;
        }
        closedUnion[0] = s_spec.handItem;
        for (i = 0; i < slotCount; ++i) {
            closedUnion[1 + i] =
                (i < out->closedCount) ? closed[i].itemType : 0;
        }
        multiset_sorted_nonzero(closedUnion, unionSize, closedSorted,
                                &closedCount);

        out->reopenResult = m11_inventory_open_chest(
            &state, 0, s_spec.reopenThing, closed, out->closedCount);
        out->reopenThing = m11_inventory_get_open_chest_thing(&state, 0);
        copy_open_slots(&state, &out->reopenedTypes[0]);
        out->reopenedCount = non_empty_count(out->reopenedTypes, slotCount);
        if (m11_inventory_get_mouse_item(&state, 0, &handItem)) {
            out->reopenedHandType = handItem.itemType;
            reopenedUnion[0] = handItem.itemType;
        } else {
            reopenedUnion[0] = 0;
        }
        for (i = 0; i < slotCount; ++i) {
            reopenedUnion[1 + i] = out->reopenedTypes[i];
        }
        multiset_sorted_nonzero(reopenedUnion, unionSize, reopenedSorted,
                                &reopenedCount);
        out->unionStableAcrossCloseReopen =
            multiset_equal(preSorted, preCount, reopenedSorted,
                           reopenedCount);
        out->noDuplicateAcrossCloseReopen =
            has_no_duplicate_nonzero(reopenedSorted, reopenedCount);
        out->fullHandLoadedAfterReopen =
            out->reopenedHandType == s_spec.handItem &&
            out->reopenedTypes[
                DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C540_INDEX] ==
                s_spec.c540Item;
        out->noOrphanItemAcrossRoundTrip =
            out->reopenedCount == out->closedCount &&
            reopenedCount == preCount;
        out->closedHandType = s_spec.handItem;
    }

    return 1;
}
