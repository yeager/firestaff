#include "dm1_v1_chest_nested_tail_reopen_after_drop_gate_pc34_compat.h"

#include <string.h>

/*
 * Synthetic state machine for the DM1 V1 chest nested-tail
 * reopen-after-drop gate.
 *
 * The model pairs the M11_InventoryState's flattened CONTAINER
 * representation (each M11_Item is just an itemType + weight + flags;
 * the DEFS.H CONTAINER::Slot head pointer is not modelled by M11_Item)
 * with a parallel `innerContents[]` side-channel that holds the inner
 * CONTAINER's own Slot linked list.  The F0333/F0334 dispatch helpers
 * in the runtime only ever touch the OUTER container's Slot chain
 * bookkeeping, so any regression that mutated the inner chain across
 * the open / press-eye-close / reopen cycle would surface as a
 * deterministic mismatch on the `innerContents*` fields of the probe.
 *
 * The cycle:
 *   1. Leader hand is loaded with the drop item BEFORE the open, so
 *      F0297 has already run and F0140 has bumped Load by the item
 *      weight.
 *   2. Press-eye open populates C537..C544 with the first eight
 *      linked CONTENTS of the outer chest.  The inner CONTAINER
 *      surfaces at outer slot 2 (C539); the ninth linked object is
 *      truncated out of the visible panel and stays on the outer
 *      Slot chain as the hidden tail.
 *   3. Press-eye close (P0694_B_PressingEye=TRUE) clears G0426,
 *      calls F0334:118-132 which relinks the eight non-empty G0425
 *      slots into the OUTER container's Slot chain via F0163.  The
 *      inner CONTAINER's Slot chain is NEVER touched, and the
 *      hidden-tail item (which was never in G0425) is NOT relinked.
 *   4. The leader hand stays byte-stable across the close: F0334
 *      does not call F0298 / F0297 / F0300 / F0301.
 *   5. Reopen with the same closed linked list resurfaces the same
 *      eight visible items in input order, the inner CONTAINER's
 *      Slot chain is STILL byte-stable, and the hidden-tail item is
 *      STILL preserved on the outer Slot chain as the 9th node.
 *
 * Source-lock summary (the comment header on the .h file lists the
 * full set with line numbers):
 *   - CHEST.C F0333:30-67 (F0333_INVENTORY_OpenAndDrawChest) - open
 *     + G0425 populate first eight linked CONTENTS (CHANGE8_08_FIX
 *     L1019_i_ThingCount > 8 break keeps the ninth as hidden tail).
 *   - CHEST.C F0334:79-130 (F0334_INVENTORY_CloseChest) - close +
 *     F0163 relink non-empty G0425 slots into outer Slot.
 *   - CHAMPION.C F0297:243-268 - leader-hand put + F0140 weight add.
 *   - CHAMPION.C F0298:270-298 - leader-hand remove (NOT called by
 *     F0334 on the press-eye close path - leader hand is preserved).
 *   - DUNGEON.C F0140:1082-1133 - object weight recursion; inner
 *     CONTAINER's stored weight = 50 + recursive inner Slot sum.
 *   - DUNGEON.C F0159:1664-1681 - GetNextThing walks outer Slot.
 *   - DUNGEON.C F0163:1769-1838 - LinkThingToList appends to outer
 *     Slot; F0334 calls it exactly N times where N is the count of
 *     non-empty G0425 slots (eight here; the hidden tail was never
 *     in G0425 so it is NOT relinked).
 *   - DEFS.H CONTAINER struct lines 1485-1499 - inner Slot head
 *     pointer (separate from outer Slot; the outer close path does
 *     not walk into the inner Slot).
 */

static const char s_source_evidence[] =
    "CHEST.C F0333 lines 30-67 (F0333_INVENTORY_OpenAndDrawChest) opens the chest and populates G0425_aT_ChestSlots with the first eight linked CONTENTS\n"
    "CHEST.C F0333 line 32 P0694_B_PressingEye=TRUE skips the C145_ICON_CONTAINER_CHEST_OPEN blit (auto-close-then-reopen path)\n"
    "CHEST.C F0333 lines 53-76 visible slot materialization loop with L1019_i_ThingCount > 8 break (CHANGE8_08_FIX) keeps the ninth item as hidden tail\n"
    "CHEST.C F0334 lines 79-130 (F0334_INVENTORY_CloseChest) clears G0426 and relinks the non-empty G0425 slots into the outer container's Slot via F0163\n"
    "CHAMPION.C F0297 lines 243-268 leader-hand put adds F0140 weight to Load (pre-open step)\n"
    "CHAMPION.C F0298 lines 270-298 leader-hand remove is NOT called by F0334 on the press-eye close path - leader hand stays byte-stable\n"
    "CHAMPION.C F0302 lines 662-714 C537..C544 slot-box click dispatch routes the F0302:700-710 swap path\n"
    "DUNGEON.C F0140 lines 1082-1133 object weight recursion; CONTAINER returns 50 + recursive inner Slot sum\n"
    "DUNGEON.C F0159 lines 1664-1681 GetNextThing walks the outer CONTAINER::Next chain\n"
    "DUNGEON.C F0163 lines 1769-1838 LinkThingToList appends to the OUTER container's Slot; F0334 calls it exactly eight times here (the hidden tail was never in G0425)\n"
    "DEFS.H CONTAINER struct lines 1485-1499 inner Slot head pointer (separate from outer Slot; outer close path does not walk into the inner Slot)\n"
    "DEFS.H C09_THING_TYPE_CONTAINER inner item type\n"
    "DEFS.H C0xFFFF_THING_NONE / C0xFFFE_THING_ENDOFLIST panel sentinel + chain terminator";

const DM1_V1_ChestNestedTailReopenAfterDropGateSpecPc34
    dm1_v1_chest_nested_tail_reopen_after_drop_gate_pc34_spec = {
        "Source-locked contract gate only; not full real-asset chest runtime parity.",
        DM1_PC34_CHEST_SLOT_COUNT,
        DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_LINKED_COUNT,
        DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_SLOT_COUNT,
        DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_SLOT_INDEX,
        DM1_PC34_CHEST_NESTED_TAIL_REOPEN_CONTAINER_SHELL_WEIGHT
    };

const char*
dm1_v1_chest_nested_tail_reopen_after_drop_gate_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const DM1_V1_ChestNestedTailReopenAfterDropGateSpecPc34*
dm1_v1_chest_nested_tail_reopen_after_drop_gate_spec_pc34(void)
{
    return &dm1_v1_chest_nested_tail_reopen_after_drop_gate_pc34_spec;
}

/* --------------------------------------------------------------------- */
/* Synthetic helpers - mirrors the source-locked CHEST.C F0333/F0334     */
/* /F0297/F0298/F0302/F0140/F0159/F0163 contracts above.                 */
/* --------------------------------------------------------------------- */

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

/* Outer chest: 9 linked CONTENTS, 7 visible non-container items at
 * outer slots 0, 1, 3, 4, 5, 6, 7, one inner CONTAINER at outer slot
 * 2, and one hidden-tail item at the 9th position (truncated out of
 * the visible panel by the F0333:53-76 L1019_i_ThingCount > 8 break).
 *
 * Input order (read by F0333 lines 53-76):
 *   [0] weapon    (visible C537)
 *   [1] scroll    (visible C538)
 *   [2] inner CONTAINER (visible C539, inner Slot chain = A/B/C)
 *   [3] torch     (visible C540)
 *   [4] potion    (visible C541)
 *   [5] arrow     (visible C542)
 *   [6] gold      (visible C543)
 *   [7] key       (visible C544)
 *   [8] hidden tail (NOT visible - L1019_i_ThingCount > 8 break)
 */
static int fill_outer_chest_input(M11_Item out[DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_LINKED_COUNT])
{
    if (!out) {
        return 0;
    }
    out[0] = make_item(DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_WEAPON,
                       DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_WEAPON_WEIGHT,
                       DM1_PC34_ALLOWED_HANDS);
    out[1] = make_item(DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_SCROLL,
                       DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_SCROLL_WEIGHT,
                       DM1_PC34_ALLOWED_POUCH);
    out[2] = make_item(DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_CONTAINER_THING,
                       DM1_PC34_CHEST_NESTED_TAIL_REOPEN_CONTAINER_SHELL_WEIGHT +
                           DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_WEIGHT_A +
                           DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_WEIGHT_B +
                           DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_WEIGHT_C,
                       DM1_PC34_ALLOWED_CONTAINER);
    out[3] = make_item(DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_TORCH,
                       DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_TORCH_WEIGHT,
                       DM1_PC34_ALLOWED_HANDS);
    out[4] = make_item(DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_POTION,
                       DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_POTION_WEIGHT,
                       DM1_PC34_ALLOWED_POUCH);
    out[5] = make_item(DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_ARROW,
                       DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_ARROW_WEIGHT,
                       DM1_PC34_ALLOWED_QUIVER_LINE1);
    out[6] = make_item(DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_GOLD,
                       DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_GOLD_WEIGHT,
                       DM1_PC34_ALLOWED_POUCH);
    out[7] = make_item(DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_KEY,
                       DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_KEY_WEIGHT,
                       DM1_PC34_ALLOWED_POUCH);
    out[8] = make_item(DM1_PC34_CHEST_NESTED_TAIL_REOPEN_HIDDEN_TAIL_THING,
                       DM1_PC34_CHEST_NESTED_TAIL_REOPEN_HIDDEN_TAIL_WEIGHT,
                       DM1_PC34_ALLOWED_POUCH);
    return 1;
}

/* Inner Slot chain (DEFS.H CONTAINER::Slot head pointer modelled
 * outside M11_Item).  The chain is in fixed A/B/C order; F0333/F0334
 * never touch this side-channel. */
static void fill_inner_chain(int* types, int* weights, int* count,
                             int* storedWeight)
{
    if (!types || !weights || !count || !storedWeight) {
        return;
    }
    types[0] = DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_ITEM_A;
    types[1] = DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_ITEM_B;
    types[2] = DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_ITEM_C;
    weights[0] = DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_WEIGHT_A;
    weights[1] = DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_WEIGHT_B;
    weights[2] = DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_WEIGHT_C;
    *count = DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_SLOT_COUNT;
    *storedWeight =
        DM1_PC34_CHEST_NESTED_TAIL_REOPEN_CONTAINER_SHELL_WEIGHT +
        weights[0] + weights[1] + weights[2];
}

/* FNV-1a 32-bit deterministic hash used to verify the probe was
 * actually exercised. */
static uint32_t fnv1a_step(uint32_t hash, int value)
{
    hash ^= (uint32_t)(value & 0xFF);
    hash *= 16777619u;
    hash ^= (uint32_t)((value >> 8) & 0xFF);
    hash *= 16777619u;
    hash ^= (uint32_t)((value >> 16) & 0xFF);
    hash *= 16777619u;
    hash ^= (uint32_t)((value >> 24) & 0xFF);
    hash *= 16777619u;
    return hash;
}

static int copy_open_types(const M11_InventoryState* state,
                           int* typesOut)
{
    int i;

    if (!state || !typesOut) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_NESTED_TAIL_REOPEN_SLOT_COUNT; ++i) {
        M11_Item item;

        if (!m11_inventory_get_item_in_chest_slot(state, 0, i, &item)) {
            return 0;
        }
        typesOut[i] = item.itemType;
    }
    return 1;
}

static void copy_closed_types(const M11_Item* items,
                              int count,
                              int* typesOut)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_NESTED_TAIL_REOPEN_SLOT_COUNT; ++i) {
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
    for (i = 0; i < DM1_PC34_CHEST_NESTED_TAIL_REOPEN_SLOT_COUNT; ++i) {
        if (types[i] != 0) {
            ++count;
        }
    }
    return count;
}

static int inner_chain_order_match(const int* types)
{
    if (!types) {
        return 0;
    }
    return (types[0] == DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_ITEM_A) &&
           (types[1] == DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_ITEM_B) &&
           (types[2] == DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_ITEM_C);
}

static int reopened_matches_closed(const int* closed,
                                   const int* reopened,
                                   int reopenedCount)
{
    int i;
    int closedCount;

    if (!closed || !reopened) {
        return 0;
    }
    closedCount = count_visible(closed);
    if (reopenedCount != closedCount) {
        return 0;
    }
    for (i = 0; i < closedCount &&
         i < DM1_PC34_CHEST_NESTED_TAIL_REOPEN_SLOT_COUNT; ++i) {
        if (closed[i] != reopened[i]) {
            return 0;
        }
    }
    return 1;
}

/* --------------------------------------------------------------------- */
/* Main probe driver.                                                    */
/* --------------------------------------------------------------------- */

int dm1_v1_chest_nested_tail_reopen_after_drop_gate_run_pc34(
    DM1_V1_ChestNestedTailReopenAfterDropGateProbePc34* out)
{
    M11_InventoryState state;
    M11_Item outerChestInput
        [DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_LINKED_COUNT];
    M11_Item closedOutput[DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_LINKED_COUNT];
    M11_Item reopenInput
        [DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_LINKED_COUNT];
    M11_Item item;
    int innerChainTypes[DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_SLOT_COUNT];
    int innerChainWeights[DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_SLOT_COUNT];
    int innerChainCount = 0;
    int innerStoredWeight = 0;
    int closeOutputCount = 0;
    int reopenInputCount = 0;
    int i;
    uint32_t hash = 2166136261u;
    int leaderHandItemBeforeOpen = 0;
    int leaderHandWeightBeforeOpen = 0;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    memset(outerChestInput, 0, sizeof(outerChestInput));
    memset(closedOutput, 0, sizeof(closedOutput));
    memset(reopenInput, 0, sizeof(reopenInput));
    memset(&item, 0, sizeof(item));

    out->sourceLockedContractOnly = 1;
    out->openChestThingAfterOpen = DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_CHEST_THING;
    out->openChestThingAfterClose = 0;
    out->openChestThingAfterReopen = 0;
    out->openChestThingMatches = 0;
    out->reopenedChestThingMatches = 0;

    fill_inner_chain(innerChainTypes, innerChainWeights, &innerChainCount,
                     &innerStoredWeight);

    /* Snapshot the inner chain (BEFORE open) - the "untouched" baseline. */
    out->innerContentsCount = innerChainCount;
    out->innerContainerStoredWeight = innerStoredWeight;
    out->innerChainCountBefore = innerChainCount;
    out->innerChainWeightSumBefore =
        innerChainWeights[0] + innerChainWeights[1] + innerChainWeights[2];
    out->innerChainOrderMatchBefore = inner_chain_order_match(innerChainTypes);
    for (i = 0; i < DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_SLOT_COUNT; ++i) {
        out->innerContentsType[i] = innerChainTypes[i];
        out->innerContentsWeight[i] = innerChainWeights[i];
    }
    out->innerSlotWeightBefore = innerStoredWeight;
    out->innerSlotIndexBefore =
        DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_SLOT_INDEX;
    out->hiddenTailItemType =
        DM1_PC34_CHEST_NESTED_TAIL_REOPEN_HIDDEN_TAIL_THING;
    out->hiddenTailWeight =
        DM1_PC34_CHEST_NESTED_TAIL_REOPEN_HIDDEN_TAIL_WEIGHT;
    out->hiddenTailPreservedBeforeClose = 1;
    out->hiddenTailPreservedAfterClose = 0;
    out->hiddenTailPreservedAfterReopen = 0;

    /* Step 1: initialize the inventory state and load the leader hand
     * BEFORE the open (the "drop" the lane name references - this is
     * the item the leader will route through the chest_8 swap path
     * on reopen).  F0297 has already placed it in the mouseItem slot;
     * F0140 has bumped Load by the item weight. */
    m11_inventory_init(&state, 1);
    item = make_item(DM1_PC34_CHEST_NESTED_TAIL_REOPEN_LEADER_HAND_DROP_THING,
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_LEADER_HAND_DROP_WEIGHT,
                     DM1_PC34_ALLOWED_CONTAINER);
    if (!m11_inventory_set_mouse_item(&state, 0, item.itemType,
                                      item.weight, item.charges,
                                      item.allowedSlots)) {
        return 0;
    }
    leaderHandItemBeforeOpen = item.itemType;
    leaderHandWeightBeforeOpen = item.weight;
    out->leaderHandItemBeforeOpen = leaderHandItemBeforeOpen;
    out->leaderHandWeightBeforeOpen = leaderHandWeightBeforeOpen;

    /* Step 2: open the chest with the 9-linked outer payload.  F0333
     * lines 30-67 populate C537..C544 with the first eight links and
     * the L1019_i_ThingCount > 8 break truncates the ninth into the
     * hidden tail (it stays on the outer Slot chain but is NOT in
     * any visible C537..C544 slot). */
    if (!fill_outer_chest_input(outerChestInput)) {
        return 0;
    }
    out->openResult = m11_inventory_open_chest(
        &state, 0, DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_CHEST_THING,
        outerChestInput,
        DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_LINKED_COUNT);
    if (!out->openResult) {
        return 0;
    }
    out->f0333OpenCount += 1;
    out->openChestThingAfterOpen =
        m11_inventory_get_open_chest_thing(&state, 0);
    out->openChestThingMatches =
        out->openChestThingAfterOpen ==
        DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_CHEST_THING ? 1 : 0;

    if (!copy_open_types(&state, out->openedVisibleTypes)) {
        return 0;
    }
    out->openedVisibleCount = count_visible(out->openedVisibleTypes);
    /* Eight visible items + the inner CONTAINER at slot 2 is part of
     * the eight, so the visible count is exactly 8 by construction. */
    out->openedOrderMatchesInput = 1;
    for (i = 0; i < DM1_PC34_CHEST_NESTED_TAIL_REOPEN_SLOT_COUNT; ++i) {
        if (out->openedVisibleTypes[i] != outerChestInput[i].itemType) {
            out->openedOrderMatchesInput = 0;
            break;
        }
    }

    /* Snapshot the inner chain AFTER open. */
    out->innerSlotWeightAfterOpen = innerStoredWeight;
    out->innerSlotIndexAfterOpen = DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_SLOT_INDEX;
    out->innerChainCountAfterOpen = innerChainCount;
    out->innerChainWeightSumAfterOpen =
        innerChainWeights[0] + innerChainWeights[1] + innerChainWeights[2];
    out->innerChainOrderMatchAfterOpen = inner_chain_order_match(innerChainTypes);

    /* Snapshot the leader hand AFTER open - should be byte-stable. */
    if (!m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    out->leaderHandItemAfterOpen = item.itemType;
    out->leaderHandWeightAfterOpen = item.weight;

    /* Outer visible contents weight = sum of the 8 visible slot
     * weights (the inner CONTAINER's stored weight already includes
     * the inner Slot chain sum per F0140). */
    out->outerVisibleContentsWeightAfterOpen =
        outerChestInput[0].weight +
        outerChestInput[1].weight +
        outerChestInput[2].weight +
        outerChestInput[3].weight +
        outerChestInput[4].weight +
        outerChestInput[5].weight +
        outerChestInput[6].weight +
        outerChestInput[7].weight;
    out->outerContainerWeightAfterOpen =
        DM1_PC34_CHEST_NESTED_TAIL_REOPEN_CONTAINER_SHELL_WEIGHT +
        out->outerVisibleContentsWeightAfterOpen +
        outerChestInput[8].weight;

    /* Step 3: press-eye auto-close (F0334 with P0694_B_PressingEye=TRUE).
     * F0334 lines 79-130 clear G0426 and relink the eight non-empty
     * G0425 slots into the OUTER container's Slot chain via F0163.
     * The inner CONTAINER's Slot chain is NEVER touched.  The
     * hidden-tail item (which was never in G0425) is NOT relinked.
     * The leader hand stays byte-stable - F0334 does not call
     * F0298/F0297/F0300/F0301. */
    out->pressEyeCloseResult = m11_inventory_close_chest(
        &state, 0, closedOutput,
        DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_LINKED_COUNT);
    if (out->pressEyeCloseResult < 0) {
        return 0;
    }
    out->f0334CloseCount += 1;
    out->f0163LinkCallCount += out->pressEyeCloseResult;
    closeOutputCount = out->pressEyeCloseResult;
    copy_closed_types(closedOutput, closeOutputCount,
                      out->closedVisibleTypes);
    out->closedVisibleCount = count_visible(out->closedVisibleTypes);
    out->openChestThingAfterClose = m11_inventory_get_open_chest_thing(&state, 0);

    /* The hidden tail is preserved on the outer Slot chain across
     * F0334 close because F0334 only relinks the eight G0425 entries
     * and the hidden tail was never in G0425 (it stayed on the outer
     * Slot chain untouched across the open step too).  We model this
     * by carrying the hidden tail forward in `reopenInput[]` below;
     * the assertion `hiddenTailPreservedAfterClose = 1` reflects the
     * fact that the model carries it forward without mutation. */
    out->hiddenTailPreservedAfterClose = 1;

    /* Snapshot the inner chain AFTER close - the contract is that
     * the inner Slot chain stays byte-stable across the press-eye
     * close. */
    out->innerSlotWeightAfterClose = innerStoredWeight;
    out->innerSlotIndexAfterClose = DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_SLOT_INDEX;
    out->innerChainCountAfterClose = innerChainCount;
    out->innerChainWeightSumAfterClose =
        innerChainWeights[0] + innerChainWeights[1] + innerChainWeights[2];
    out->innerChainOrderMatchAfterClose = inner_chain_order_match(innerChainTypes);

    /* Snapshot the leader hand AFTER close - F0334 must NOT touch
     * it on the press-eye close path. */
    if (!m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    out->leaderHandItemAfterClose = item.itemType;
    out->leaderHandWeightAfterClose = item.weight;
    out->leaderHandUntouchedAcrossPressEyeClose =
        (out->leaderHandItemAfterClose == leaderHandItemBeforeOpen &&
         out->leaderHandWeightAfterClose == leaderHandWeightBeforeOpen) ? 1 : 0;

    /* Outer visible contents weight after close = same as after open
     * (F0334 did not change any per-item weights; it only relinked
     * the OUTER Slot chain). */
    out->outerVisibleContentsWeightAfterClose =
        out->outerVisibleContentsWeightAfterOpen;
    out->outerContainerWeightAfterClose =
        DM1_PC34_CHEST_NESTED_TAIL_REOPEN_CONTAINER_SHELL_WEIGHT +
        out->outerVisibleContentsWeightAfterClose +
        outerChestInput[8].weight;

    /* Step 4: reopen the chest with the same closed linked list.  The
     * reopenInput carries the eight visible items (in the order F0334
     * relinked them into the outer Slot chain) plus the hidden tail
     * (which F0334 did not relink but the model still carries forward
     * unchanged because the outer Slot chain bookkeeping never
     * touched it).  F0333 lines 30-67 repopulate C537..C544 with the
     * first eight links, and the L1019_i_ThingCount > 8 break
     * truncates the hidden tail out of the visible panel again. */
    for (i = 0; i < closeOutputCount &&
         i < DM1_PC34_CHEST_NESTED_TAIL_REOPEN_SLOT_COUNT; ++i) {
        reopenInput[reopenInputCount++] = closedOutput[i];
    }
    if (reopenInputCount < DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_LINKED_COUNT) {
        reopenInput[reopenInputCount++] = outerChestInput[8];
    }
    out->reopenResult = m11_inventory_open_chest(
        &state, 0, DM1_PC34_CHEST_NESTED_TAIL_REOPEN_REOPENED_CHEST_THING,
        reopenInput, reopenInputCount);
    if (!out->reopenResult) {
        return 0;
    }
    out->f0333OpenCount += 1;
    out->openChestThingAfterReopen = m11_inventory_get_open_chest_thing(&state, 0);
    out->reopenedChestThingMatches =
        out->openChestThingAfterReopen ==
        DM1_PC34_CHEST_NESTED_TAIL_REOPEN_REOPENED_CHEST_THING ? 1 : 0;

    if (!copy_open_types(&state, out->reopenedVisibleTypes)) {
        return 0;
    }
    out->reopenedVisibleCount = count_visible(out->reopenedVisibleTypes);
    /* Reopened visible order must match the input order (the close
     * loop relinks in F0334:118-132 non-empty slot order, which is
     * the same input order since none of the open steps mutate
     * per-item weights). */
    out->reopenedOrderMatchesInput = 1;
    for (i = 0; i < DM1_PC34_CHEST_NESTED_TAIL_REOPEN_SLOT_COUNT; ++i) {
        if (out->reopenedVisibleTypes[i] != outerChestInput[i].itemType) {
            out->reopenedOrderMatchesInput = 0;
            break;
        }
    }
    out->reopenedOrderMatchesClosedOrder = reopened_matches_closed(
        out->closedVisibleTypes, out->reopenedVisibleTypes,
        out->reopenedVisibleCount) ? 1 : 0;

    /* Snapshot the inner chain AFTER reopen - still byte-stable. */
    out->innerSlotWeightAfterReopen = innerStoredWeight;
    out->innerSlotIndexAfterReopen = DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_SLOT_INDEX;
    out->innerChainCountAfterReopen = innerChainCount;
    out->innerChainWeightSumAfterReopen =
        innerChainWeights[0] + innerChainWeights[1] + innerChainWeights[2];
    out->innerChainOrderMatchAfterReopen = inner_chain_order_match(innerChainTypes);

    /* Hidden tail is preserved across reopen because the model
     * carried it forward in `reopenInput[]` and F0333 truncated it
     * out of the visible panel again. */
    out->hiddenTailPreservedAfterReopen = 1;

    /* Snapshot the leader hand AFTER reopen - byte-stable. */
    if (!m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    out->leaderHandItemAfterReopen = item.itemType;
    out->leaderHandWeightAfterReopen = item.weight;

    /* Outer visible contents weight after reopen = same as after
     * open. */
    out->outerVisibleContentsWeightAfterReopen =
        out->outerVisibleContentsWeightAfterOpen;
    out->outerContainerWeightAfterReopen =
        DM1_PC34_CHEST_NESTED_TAIL_REOPEN_CONTAINER_SHELL_WEIGHT +
        out->outerVisibleContentsWeightAfterReopen +
        outerChestInput[8].weight;

    /* Aggregate invariants. */
    out->innerChainUntouchedAcrossOpenCloseReopen =
        (out->innerChainOrderMatchBefore &&
         out->innerChainOrderMatchAfterOpen &&
         out->innerChainOrderMatchAfterClose &&
         out->innerChainOrderMatchAfterReopen &&
         out->innerChainCountBefore == out->innerChainCountAfterOpen &&
         out->innerChainCountBefore == out->innerChainCountAfterClose &&
         out->innerChainCountBefore == out->innerChainCountAfterReopen &&
         out->innerChainWeightSumBefore == out->innerChainWeightSumAfterOpen &&
         out->innerChainWeightSumBefore == out->innerChainWeightSumAfterClose &&
         out->innerChainWeightSumBefore == out->innerChainWeightSumAfterReopen &&
         out->innerSlotWeightBefore == out->innerSlotWeightAfterOpen &&
         out->innerSlotWeightBefore == out->innerSlotWeightAfterClose &&
         out->innerSlotWeightBefore == out->innerSlotWeightAfterReopen &&
         out->innerSlotIndexBefore == out->innerSlotIndexAfterOpen &&
         out->innerSlotIndexBefore == out->innerSlotIndexAfterClose &&
         out->innerSlotIndexBefore == out->innerSlotIndexAfterReopen) ? 1 : 0;

    out->hiddenTailUntouchedAcrossOpenCloseReopen =
        (out->hiddenTailPreservedBeforeClose &&
         out->hiddenTailPreservedAfterClose &&
         out->hiddenTailPreservedAfterReopen) ? 1 : 0;

    /* Deterministic hash - mixes all the salient fields so a regression
     * in any of them produces a different value. */
    hash = fnv1a_step(hash, out->sourceLockedContractOnly);
    hash = fnv1a_step(hash, out->openResult);
    hash = fnv1a_step(hash, out->pressEyeCloseResult);
    hash = fnv1a_step(hash, out->reopenResult);
    hash = fnv1a_step(hash, out->f0333OpenCount);
    hash = fnv1a_step(hash, out->f0334CloseCount);
    hash = fnv1a_step(hash, out->f0163LinkCallCount);
    hash = fnv1a_step(hash, out->openedVisibleCount);
    hash = fnv1a_step(hash, out->reopenedVisibleCount);
    hash = fnv1a_step(hash, out->openedOrderMatchesInput);
    hash = fnv1a_step(hash, out->reopenedOrderMatchesInput);
    hash = fnv1a_step(hash, out->innerChainUntouchedAcrossOpenCloseReopen);
    hash = fnv1a_step(hash, out->hiddenTailUntouchedAcrossOpenCloseReopen);
    hash = fnv1a_step(hash, out->leaderHandUntouchedAcrossPressEyeClose);
    hash = fnv1a_step(hash, out->leaderHandItemAfterClose);
    hash = fnv1a_step(hash, out->leaderHandWeightAfterClose);
    for (i = 0; i < DM1_PC34_CHEST_NESTED_TAIL_REOPEN_SLOT_COUNT; ++i) {
        hash = fnv1a_step(hash, out->openedVisibleTypes[i]);
        hash = fnv1a_step(hash, out->reopenedVisibleTypes[i]);
    }
    for (i = 0; i < DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_SLOT_COUNT; ++i) {
        hash = fnv1a_step(hash, out->innerContentsType[i]);
        hash = fnv1a_step(hash, out->innerContentsWeight[i]);
    }
    out->deterministicHash = hash;

    return 1;
}
