#ifndef FIRESTAFF_DM1_V1_CHEST_NESTED_CONTAINER_WEIGHT_GATE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_NESTED_CONTAINER_WEIGHT_GATE_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * DM1 V1 chest nested-container weight gate.
 *
 * Lane:
 *   "Nested container weight" - when an open chest (CHEST.C F0333,
 *   G0426_T_OpenChest live, G0425_aT_ChestSlots[0..7] populated) contains
 *   an inner CONTAINER (DEFS.H C09_THING_TYPE_CONTAINER) at one of the
 *   visible slots, taking or dropping ONE non-container item from a
 *   different outer-chest slot must:
 *     1. leave the inner CONTAINER's own Slot (DEFS.H CONTAINER::Slot,
 *        the head of its inner linked CONTENTS chain) byte-stable,
 *     2. preserve the inner CONTAINER's stored weight value (F0140
 *        recursion: 50 + sum of inner Slot weights) at the outer
 *        visible slot,
 *     3. shift the outer chest's visible contents weight by EXACTLY
 *        the weight of the taken/dropped item, no more and no less,
 *     4. update leader-hand Load through F0297/F0298 (CHAMPION.C
 *        F0297:243-268 add, F0298:270-298 subtract) by the same
 *        weight, and
 *     5. round-trip cleanly through F0334 close + F0333 reopen: the
 *        inner CONTAINER must resurface at the same outer slot with
 *        the same itemType and the same stored weight, and the
 *        F0163/F0159 chain bookkeeping that touches the OUTER
 *        container's Slot must not perturb the INNER Slot head.
 *
 * The gate is intentionally a single take + a single drop, scoped to
 * one champion's inventory. The chain-mechanics invariants exercised
 * here are disjoint from:
 *   - the chest scroll-wheel pickup overflow (pass652 / pass1062)
 *   - the chest ninth-item hidden tail (CHEST.C F0333:53-76, only
 *     eight visible links; the inner Slot is OUTSIDE the visible
 *     panel so its first eight items would themselves be truncated
 *     if the inner container were popped open)
 *   - the chest close-while-candidate-live-non-leader pass822
 *   - the chest action-hand owner-change close-chest-first redraw
 *   - any mirror-candidate / C040 / resurrect lanes
 *
 * The "nesting" is the DEFS.H CONTAINER::Slot head pointer; the
 * synthetic M11 model represents the inner Slot as a private
 * side-channel of `innerContainerContents[]` that the
 * F0333/F0334/F0302 dispatch never rewrites, so any regression in
 * the outer take/drop that touched the inner chain would surface
 * as a deterministic mismatch on the `innerContentsPreservedBefore`
 * vs `innerContentsPreservedAfter` assertions.
 *
 * ReDMCSB anchors:
 *   CHEST.C F0333:53-76    - copies the first eight linked things
 *                             into G0425_aT_ChestSlots for the open
 *                             panel.
 *   CHEST.C F0334:113-132  - close path: clears G0426, compacts
 *                             non-empty G0425 slots through F0163
 *                             into the outer container's Slot list.
 *   CHAMPION.C F0297:243-268 - leader-hand put, F0140 weight add.
 *   CHAMPION.C F0298:270-298 - leader-hand remove, F0140 weight
 *                             subtract.
 *   CHAMPION.C F0300:511-515 - slot remove through F0299.
 *   CHAMPION.C F0301:606-614 - slot add through F0299.
 *   CHAMPION.C F0302:662-714 - C537..C544 slot-box click dispatch.
 *   DUNGEON.C F0140:1082-1133 - object weight recursion; CONTAINER
 *                             type returns 50 + recursive Slot sum.
 *   DUNGEON.C F0159:1664-1681 - GetNextThing walks CONTAINER::Next.
 *   DUNGEON.C F0163:1769-1838 - LinkThingToList appends a thing to
 *                             the outer container's Slot list.
 *   DEFS.H C09_THING_TYPE_CONTAINER - the inner item type.
 *   DEFS.H CONTAINER struct         - Next + Slot linked-list
 *                             members (lines 1485-1499).
 *
 * Source-locked contract-only marker; no real-asset / original-DOS
 * pixel parity claim.
 */

enum {
    DM1_PC34_NESTED_CONTAINER_INNER_SLOT_COUNT = 3,
    DM1_PC34_NESTED_CONTAINER_OUTER_SLOT_COUNT = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_NESTED_CONTAINER_INNER_SLOT_INDEX = 1,

    DM1_PC34_NESTED_CONTAINER_THING_NONE = 0,
    DM1_PC34_NESTED_CONTAINER_ENDOFLIST = 0xFFFE,
    DM1_PC34_NESTED_CONTAINER_CONTAINER_SHELL_WEIGHT = 50,

    DM1_PC34_NESTED_CONTAINER_OUTER_CHEST_THING = 0x6C71,
    DM1_PC34_NESTED_CONTAINER_INNER_CONTAINER_THING = 0x6E91,

    /* Inner Slot items (F0140 weight values). */
    DM1_PC34_NESTED_CONTAINER_INNER_ITEM_A = 0x7701,
    DM1_PC34_NESTED_CONTAINER_INNER_WEIGHT_A = 3,
    DM1_PC34_NESTED_CONTAINER_INNER_ITEM_B = 0x7702,
    DM1_PC34_NESTED_CONTAINER_INNER_WEIGHT_B = 5,
    DM1_PC34_NESTED_CONTAINER_INNER_ITEM_C = 0x7703,
    DM1_PC34_NESTED_CONTAINER_INNER_WEIGHT_C = 7,

    /* Outer chest items. */
    DM1_PC34_NESTED_CONTAINER_OUTER_WEAPON = 0x7801,
    DM1_PC34_NESTED_CONTAINER_OUTER_WEAPON_WEIGHT = 9,
    DM1_PC34_NESTED_CONTAINER_OUTER_TORCH = 0x7802,
    DM1_PC34_NESTED_CONTAINER_OUTER_TORCH_WEIGHT = 4,
    DM1_PC34_NESTED_CONTAINER_OUTER_SCROLL = 0x7803,
    DM1_PC34_NESTED_CONTAINER_OUTER_SCROLL_WEIGHT = 1,

    /* Slot indices inside the outer chest (C30..C37 layout). */
    DM1_PC34_NESTED_CONTAINER_OUTER_SLOT_0 = 0,
    DM1_PC34_NESTED_CONTAINER_OUTER_SLOT_1 = 1,
    DM1_PC34_NESTED_CONTAINER_OUTER_SLOT_2 = 2,
    DM1_PC34_NESTED_CONTAINER_OUTER_SLOT_3 = 3,
    DM1_PC34_NESTED_CONTAINER_OUTER_SLOT_4 = 4
};

typedef struct {
    const char* contractMarker;
    int c09ContainerThingType;
    int containerShellWeight;
    int innerSlotCount;
    int outerSlotCount;
    int innerSlotIndex;
} DM1_V1_ChestNestedContainerWeightGateSpecPc34;

typedef struct {
    /* Side-channel: the inner container's own Slot head chain.
     * Lives outside the M11_Item representation because M11_Item does
     * not encode the CONTAINER::Next linked-list pointer; the test
     * model treats this array as the inner Slot chain that the
     * F0333/F0334/F0302 dispatch must NOT perturb when the outer
     * take/drop sequence runs. */
    int innerContentsType[DM1_PC34_NESTED_CONTAINER_INNER_SLOT_COUNT];
    int innerContentsWeight[DM1_PC34_NESTED_CONTAINER_INNER_SLOT_COUNT];
    int innerContentsCount;
    int innerContainerStoredWeight;

    /* Outer chest shape. */
    int outerOpenResult;
    int outerCloseResult;
    int outerReopenResult;
    int outerVisibleContentsWeightBefore;
    int outerVisibleContentsWeightAfterTake;
    int outerVisibleContentsWeightAfterDrop;
    int outerVisibleContentsWeightAfterReopen;

    /* Inner container survives the outer take/drop. */
    int innerSlotItemBefore;
    int innerSlotItemAfterTake;
    int innerSlotItemAfterDrop;
    int innerSlotItemAfterReopen;
    int innerSlotWeightBefore;
    int innerSlotWeightAfterTake;
    int innerSlotWeightAfterDrop;
    int innerSlotWeightAfterReopen;

    /* Inner chain survives byte-stable. */
    int innerContentsCountBefore;
    int innerContentsCountAfterTake;
    int innerContentsCountAfterDrop;
    int innerContentsCountAfterReopen;
    int innerContentsOrderMatchBefore;
    int innerContentsOrderMatchAfterTake;
    int innerContentsOrderMatchAfterDrop;
    int innerContentsOrderMatchAfterReopen;
    int innerContentsWeightSumBefore;
    int innerContentsWeightSumAfterTake;
    int innerContentsWeightSumAfterDrop;
    int innerContentsWeightSumAfterReopen;

    /* Outer container weight (50 + visible contents) per F0140. */
    int outerContainerWeightBefore;
    int outerContainerWeightAfterTake;
    int outerContainerWeightAfterDrop;
    int outerContainerWeightAfterReopen;

    /* Leader-hand take/drop contract (F0297/F0298). */
    int leaderHandBeforeType;
    int leaderHandBeforeWeight;
    int leaderHandAfterTakeType;
    int leaderHandAfterTakeWeight;
    int leaderHandAfterDropType;
    int leaderHandAfterDropWeight;

    /* Load recompute contract (CHAMPION.C F0297+F0298 = no-op when
     * the same object is dropped back). */
    int loadBeforeOpen;
    int loadAfterOpen;
    int loadAfterTake;
    int loadAfterDrop;
    int loadAfterReopen;

    /* Counters. */
    int f0297PutLeaderHandCount;
    int f0298RemoveLeaderHandCount;
    int f0300SlotRemoveCount;
    int f0301SlotAddCount;
    int f0302SlotClickCount;
    int f0333OpenCount;
    int f0334CloseCount;
    int f0163LinkThingToListCalls;

    /* Bookkeeping sanity. */
    int sourceLockedContractOnly;
    int weightDeltaMatchesTakenItem;
    int leaderHandWeightMatchesTakenItem;
    int innerChainUntouchedAcrossTakeDropReopen;
    int reopenedSlot0Preserved;
    int reopenedSlot1Preserved;
    int reopenedSlot2Preserved;
    int reopenedSlot3Preserved;

    uint32_t deterministicHash;
} DM1_V1_ChestNestedContainerWeightGateProbePc34;

extern const DM1_V1_ChestNestedContainerWeightGateSpecPc34
    dm1_v1_chest_nested_container_weight_gate_pc34_spec;

const char*
dm1_v1_chest_nested_container_weight_gate_source_evidence_pc34(void);

const DM1_V1_ChestNestedContainerWeightGateSpecPc34*
dm1_v1_chest_nested_container_weight_gate_spec_pc34(void);

int dm1_v1_chest_nested_container_weight_gate_run_pc34(
    DM1_V1_ChestNestedContainerWeightGateProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_NESTED_CONTAINER_WEIGHT_GATE_PC34_COMPAT_H */
