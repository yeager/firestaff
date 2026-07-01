#ifndef FIRESTAFF_DM1_V1_CHEST_NESTED_TAIL_REOPEN_AFTER_DROP_GATE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_NESTED_TAIL_REOPEN_AFTER_DROP_GATE_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * DM1 V1 chest nested-tail reopen-after-drop gate.
 *
 * Lane:
 *   "auto chest nested tail reopen after drop" - the unique source-locked
 *   combination that NONE of the existing chest gates cover in one slice:
 *
 *     1. an outer chest is open (G0426_T_OpenChest live, eight C537..C544
 *        visible slots populated from the outer container's first eight
 *        linked CONTENTS),
 *     2. one of those eight visible slots holds an INNER CONTAINER
 *        (DEFS.H C09_THING_TYPE_CONTAINER) whose own Slot chain carries
 *        inner CONTENTS that the outer Slot chain does NOT touch,
 *     3. the chest also carries a NINTH (hidden-tail) linked object
 *        that the F0333:53-76 visible-materialization loop truncates
 *        out of the panel (only eight links fit in C537..C544 per
 *        L1019_i_ThingCount > 8 early-break in CHANGE8_08_FIX),
 *     4. the leader hand is loaded BEFORE opening (F0297 has placed an
 *        item in the mouseItem slot; F0140 has bumped the leader Load
 *        by that item's weight), so the panel shows the chest with
 *        an occupied action hand,
 *     5. the chest auto-closes via the leader-hand-full-then-press-eye
 *        path: a second press-eye on a chest container while the
 *        action hand already holds an item triggers F0334
 *        (P0694_B_PressingEye=TRUE on the close side) WITHOUT taking
 *        the leader-hand item, leaving the leader hand stable, and
 *     6. the chest is REOPENED with the SAME closed linked list, and
 *        the visible slot order, the inner Slot chain head pointer,
 *        the inner Slot item types + weights, AND the hidden-tail item
 *        must all resurface byte-stable.
 *
 * Why this lane is non-duplicative:
 *
 *   - dm1_v1_chest_nested_container_weight_gate_pc34_compat pins the
 *     inner-container weight + slot chain across a take + drop on
 *     a NON-TAIL chest (eight visible items, no ninth tail).  It
 *     does NOT model the press-eye auto-close gate and it does NOT
 *     exercise a leader-hand-loaded-before-open + auto-close + reopen
 *     cycle.
 *   - dm1_v1_chest_ninth_item_hidden_tail_pc34_compat pins the
 *     hidden-tail truncation on a NON-NESTED chest (nine flat
 *     non-container linked objects).  It does NOT carry an inner
 *     CONTAINER at any visible slot, so the inner Slot chain is
 *     not exercised.
 *   - dm1_v1_auto_chest_action_hand_swap_during_close_gate_pc34_compat
 *     pins the press-eye + C09 action-hand swap on a NON-TAIL
 *     non-nested chest.  It does NOT carry a hidden tail.
 *   - dm1_v1_chest_close_rewire_runtime_pc34_compat /
 *     dm1_v1_chest_mid_close_hand_swap_pc34_compat / the scroll-wheel
 *     close-race gates pin the close-handler rewire under various
 *     slot-click and rotation races on FLAT chests, not nested+tail.
 *
 * What the contract pins:
 *
 *   A. Press-eye open on a chest with eight visible + one hidden tail
 *      + one visible slot occupied by an inner CONTAINER populates
 *      C537..C544 in input order, the inner Slot head + item types
 *      are byte-stable, the hidden-tail item is preserved on the
 *      outer container's Slot chain as the 9th node, and the panel
 *      content becomes M569_PANEL_CHEST.
 *   B. Setting the leader mouse item (the "drop" the lane name
 *      references - the leader hand is loaded with the same item
 *      that will be deposited when the chest_8 swap path fires
 *      F0302:700-710) does NOT mutate the inner Slot chain and does
 *      NOT touch the hidden tail.
 *   C. A press-eye CLOSE on a loaded leader hand (the auto-close
 *      path: P0694_B_PressingEye=TRUE) clears G0426, rewrites the
 *      OUTER Slot chain to F0334's 0..N compact order via F0163, and
 *      KEEPS the leader hand byte-stable (F0334 does not call
 *      F0298/F0297 - it leaves the mouse item untouched).
 *   D. The reopened chest (with the same closed linked list as input)
 *      resurfaces the SAME eight visible items in the SAME order
 *      (input-order preserved across F0334/F0333), the inner Slot
 *      head + item types are STILL byte-stable, and the hidden-tail
 *      item is STILL preserved on the outer container's Slot chain
 *      as the 9th node.
 *   E. F0163_DUNGEON_LinkThingToList is called exactly eight times
 *      across the close step (the ninth item was never in G0425 so
 *      it is NOT relinked, matching the F0334:118-132 source path),
 *      and F0333 reopen fires exactly twice across the cycle
 *      (initial open + reopen).
 *
 * Source-locked contract-only marker; no real-asset / original-DOS
 * pixel parity claim.  The M11_Item representation flattens the
 * DEFS.H CONTAINER::Slot head pointer into a single stored weight
 * value, so the inner Slot chain is modeled as a parallel side-
 * channel (`innerContentsType[]` + `innerContentsWeight[]`) that the
 * F0333/F0334/F0302 dispatch helpers never rewrite.  Any regression
 * in the outer close / reopen cycle that perturbed the inner chain
 * would surface as a deterministic mismatch on the `innerContents*`
 * fields of the probe.
 *
 * ReDMCSB anchors:
 *   CHEST.C F0333 lines 30-67 (F0333_INVENTORY_OpenAndDrawChest)
 *                              - open panel + G0426 set + G0425
 *                                populate first eight linked CONTENTS;
 *                                L1019_i_ThingCount > 8 break
 *                                (CHANGE8_08_FIX) keeps the ninth
 *                                item on the outer chain only.
 *   CHEST.C F0333 line 32      - P0694_B_PressingEye=TRUE skips the
 *                                C145_ICON_CONTAINER_CHEST_OPEN blit
 *                                (the auto-close-then-reopen path).
 *   CHEST.C F0333 lines 53-76  - visible slot materialization loop
 *                                (eight links max, then C0xFFFF fill).
 *   CHEST.C F0334 lines 79-130 - close panel + G0426 clear + F0163
 *                                relink the non-empty G0425 slots
 *                                into the outer container's Slot.
 *   CHAMPION.C F0297 lines 243-268 - leader-hand put + F0140 weight
 *                                add to Load (the pre-open step).
 *   CHAMPION.C F0298 lines 270-298 - leader-hand remove + F0140
 *                                weight subtract (F0334 does NOT call
 *                                this on the auto-close path - the
 *                                leader hand is preserved).
 *   CHAMPION.C F0302 lines 662-714 - C537..C544 slot-box click
 *                                dispatch (F0302:700-710 swap path).
 *   DUNGEON.C F0140 lines 1082-1133 - object weight recursion
 *                                (50 + recursive Slot sum for
 *                                CONTAINER - inner Slot chain stays
 *                                untouched across outer close).
 *   DUNGEON.C F0159 lines 1664-1681 - GetNextThing walks
 *                                CONTAINER::Next (the outer chain).
 *   DUNGEON.C F0163 lines 1769-1838 - LinkThingToList appends to the
 *                                outer container's Slot (eight calls
 *                                on close; the ninth item was never
 *                                in G0425, so it is NOT relinked).
 *   DEFS.H CONTAINER struct lines 1485-1499 - inner Slot head
 *                                pointer (separate from outer Slot).
 *   DEFS.H C09_THING_TYPE_CONTAINER - the inner item type.
 *   DEFS.H C0xFFFF_THING_NONE / C0xFFFE_THING_ENDOFLIST - panel
 *                                sentinel + chain terminator.
 *
 * Disjointness (covered elsewhere):
 *   - chest_nested_container_weight_gate (nested weight, no tail,
 *     no press-eye, no reopen-after-drop cycle)
 *   - chest_ninth_item_hidden_tail (hidden tail, no nested inner
 *     CONTAINER, no reopen-after-drop cycle)
 *   - chest_auto_close_on_leader_death (press-eye auto-close on
 *     death, no nested, no tail, no drop)
 *   - chest_hand_swap_no_duplicate_gate (C538/C539/C540/C542/C543
 *     click matrix on a flat chest, no nested, no tail)
 *   - chest_scroll_wheel_close_race (queued slot command vs close
 *     race, no nested, no tail)
 *   - chest_reopen_after_leader_rotation (leader rotation cycle, no
 *     nested, no tail)
 *   - chest_inventory_chest_close_recompaction (close-only path,
 *     no nested + tail + reopen cycle)
 *   - chest_open_stack_split / chest_open_stack_split_press_eye
 *     (stack split on flat chests, no nested, no tail)
 */

enum {
    DM1_PC34_CHEST_NESTED_TAIL_REOPEN_SLOT_COUNT =
        DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_LINKED_COUNT = 9,
    DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_SLOT_COUNT = 3,
    DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_SLOT_INDEX = 2,

    DM1_PC34_CHEST_NESTED_TAIL_REOPEN_THING_NONE = 0,
    DM1_PC34_CHEST_NESTED_TAIL_REOPEN_ENDOFLIST = 0xFFFE,
    DM1_PC34_CHEST_NESTED_TAIL_REOPEN_CONTAINER_SHELL_WEIGHT = 50,

    /* Outer container (the chest on the dungeon floor). */
    DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_CHEST_THING = 0x6F01,
    DM1_PC34_CHEST_NESTED_TAIL_REOPEN_REOPENED_CHEST_THING = 0x6F02,

    /* Inner CONTAINER occupying outer chest slot 2 (visible C539). */
    DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_CONTAINER_THING = 0x7101,

    /* Inner Slot items (F0140 weight values). */
    DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_ITEM_A = 0x7301,
    DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_WEIGHT_A = 3,
    DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_ITEM_B = 0x7302,
    DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_WEIGHT_B = 5,
    DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_ITEM_C = 0x7303,
    DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_WEIGHT_C = 7,

    /* Outer chest items (one per visible slot 0..7 except slot 2 which
     * holds the inner container, and slot 7 which is the "drop
     * target" we will route the leader-hand drop into during the
     * reopen cycle). */
    DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_WEAPON = 0x7401,
    DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_WEAPON_WEIGHT = 9,
    DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_SCROLL = 0x7402,
    DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_SCROLL_WEIGHT = 1,
    DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_TORCH = 0x7403,
    DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_TORCH_WEIGHT = 4,
    DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_POTION = 0x7404,
    DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_POTION_WEIGHT = 2,
    DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_ARROW = 0x7405,
    DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_ARROW_WEIGHT = 1,
    DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_GOLD = 0x7406,
    DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_GOLD_WEIGHT = 0,
    DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_KEY = 0x7407,
    DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_KEY_WEIGHT = 0,

    /* Hidden-tail item: the ninth linked object that F0333 truncates
     * out of the visible C537..C544 panel.  It must stay on the outer
     * container's Slot chain across F0334 close and F0333 reopen. */
    DM1_PC34_CHEST_NESTED_TAIL_REOPEN_HIDDEN_TAIL_THING = 0x7501,
    DM1_PC34_CHEST_NESTED_TAIL_REOPEN_HIDDEN_TAIL_WEIGHT = 2,

    /* Leader-hand drop item (the "drop" the lane name references -
     * loaded BEFORE the open so F0297/F0140 already ran; the F0334
     * close path does NOT touch the leader hand on the auto-close
     * branch). */
    DM1_PC34_CHEST_NESTED_TAIL_REOPEN_LEADER_HAND_DROP_THING = 0x7601,
    DM1_PC34_CHEST_NESTED_TAIL_REOPEN_LEADER_HAND_DROP_WEIGHT = 6
};

typedef struct {
    const char* contractMarker;
    int visibleSlotCount;
    int outerLinkedCount;
    int innerSlotCount;
    int innerSlotIndex;
    int containerShellWeight;
} DM1_V1_ChestNestedTailReopenAfterDropGateSpecPc34;

typedef struct {
    /* Inner Slot chain side-channel (DEFS.H CONTAINER::Slot head
     * pointer; M11_Item does not encode the linked-list head). */
    int innerContentsType[DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_SLOT_COUNT];
    int innerContentsWeight[DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_SLOT_COUNT];
    int innerContentsCount;
    int innerContainerStoredWeight;

    /* Hidden-tail bookkeeping. */
    int hiddenTailItemType;
    int hiddenTailWeight;
    int hiddenTailPreservedBeforeClose;
    int hiddenTailPreservedAfterClose;
    int hiddenTailPreservedAfterReopen;

    /* Open / close / reopen return codes. */
    int openResult;
    int pressEyeCloseResult;
    int reopenResult;
    int f0333OpenCount;
    int f0334CloseCount;
    int f0163LinkCallCount;

    /* Visible slot state across the cycle. */
    int openedVisibleTypes[DM1_PC34_CHEST_NESTED_TAIL_REOPEN_SLOT_COUNT];
    int closedVisibleTypes[DM1_PC34_CHEST_NESTED_TAIL_REOPEN_SLOT_COUNT];
    int reopenedVisibleTypes[DM1_PC34_CHEST_NESTED_TAIL_REOPEN_SLOT_COUNT];
    int openedVisibleCount;
    int closedVisibleCount;
    int reopenedVisibleCount;
    int openedOrderMatchesInput;
    int reopenedOrderMatchesInput;

    /* Inner-slot position survives byte-stable. */
    int innerSlotIndexBefore;
    int innerSlotIndexAfterOpen;
    int innerSlotIndexAfterClose;
    int innerSlotIndexAfterReopen;
    int innerSlotWeightBefore;
    int innerSlotWeightAfterOpen;
    int innerSlotWeightAfterClose;
    int innerSlotWeightAfterReopen;
    int innerChainOrderMatchBefore;
    int innerChainOrderMatchAfterOpen;
    int innerChainOrderMatchAfterClose;
    int innerChainOrderMatchAfterReopen;
    int innerChainCountBefore;
    int innerChainCountAfterOpen;
    int innerChainCountAfterClose;
    int innerChainCountAfterReopen;
    int innerChainWeightSumBefore;
    int innerChainWeightSumAfterOpen;
    int innerChainWeightSumAfterClose;
    int innerChainWeightSumAfterReopen;

    /* Leader hand stays byte-stable across the press-eye close. */
    int leaderHandItemBeforeOpen;
    int leaderHandWeightBeforeOpen;
    int leaderHandItemAfterOpen;
    int leaderHandWeightAfterOpen;
    int leaderHandItemAfterClose;
    int leaderHandWeightAfterClose;
    int leaderHandItemAfterReopen;
    int leaderHandWeightAfterReopen;

    /* Outer visible contents weight (sum of 8 visible slots). */
    int outerVisibleContentsWeightAfterOpen;
    int outerVisibleContentsWeightAfterClose;
    int outerVisibleContentsWeightAfterReopen;
    int outerContainerWeightAfterOpen;
    int outerContainerWeightAfterClose;
    int outerContainerWeightAfterReopen;

    /* Open/close chest thing identity. */
    int openChestThingAfterOpen;
    int openChestThingAfterClose;
    int openChestThingAfterReopen;
    int openChestThingMatches;
    int reopenedChestThingMatches;

    /* Aggregate invariants. */
    int sourceLockedContractOnly;
    int innerChainUntouchedAcrossOpenCloseReopen;
    int hiddenTailUntouchedAcrossOpenCloseReopen;
    int leaderHandUntouchedAcrossPressEyeClose;
    int reopenedOrderMatchesClosedOrder;

    uint32_t deterministicHash;
} DM1_V1_ChestNestedTailReopenAfterDropGateProbePc34;

extern const DM1_V1_ChestNestedTailReopenAfterDropGateSpecPc34
    dm1_v1_chest_nested_tail_reopen_after_drop_gate_pc34_spec;

const char*
dm1_v1_chest_nested_tail_reopen_after_drop_gate_source_evidence_pc34(void);

const DM1_V1_ChestNestedTailReopenAfterDropGateSpecPc34*
dm1_v1_chest_nested_tail_reopen_after_drop_gate_spec_pc34(void);

int dm1_v1_chest_nested_tail_reopen_after_drop_gate_run_pc34(
    DM1_V1_ChestNestedTailReopenAfterDropGateProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_NESTED_TAIL_REOPEN_AFTER_DROP_GATE_PC34_COMPAT_H */
