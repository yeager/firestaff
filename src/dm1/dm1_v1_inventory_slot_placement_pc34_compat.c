#include "dm1_v1_inventory_slot_placement_pc34_compat.h"

#include <string.h>

/* ReDMCSB source-lock map for this gate:
 *
 *   DATA.C:1049-1087 G0038_ai_Graphic562_SlotMasks
 *     - C00/C01 (Ready Hand / Action Hand)        MASK0xFFFF_ANY_SLOT
 *     - C02 (Head)                                MASK0x0002_HEAD
 *     - C03 (Torso)                               MASK0x0008_TORSO
 *     - C04 (Legs)                                MASK0x0010_LEGS
 *     - C05 (Feet)                                MASK0x0020_FEET
 *     - C06 (Pouch 2)                             MASK0x0100_POUCH
 *     - C07 (Quiver Line2 1)                      MASK0x0080_QUIVER_LINE2
 *     - C08 (Quiver Line1 2)                      MASK0x0080_QUIVER_LINE2
 *     - C09 (Quiver Line2 2)                      MASK0x0080_QUIVER_LINE2
 *     - C10 (Neck)                                MASK0x0004_NECK
 *     - C11 (Pouch 1)                             MASK0x0100_POUCH
 *     - C12 (Quiver Line1 1)                      MASK0x0040_QUIVER_LINE1
 *     - C13..C29 (Backpack Line1/Line2)           MASK0xFFFF_ANY_SLOT
 *     - C30..C37 (Chest slots 1..8)               MASK0x0400_CONTAINER
 *
 *   DATA.C:436-466  G0057_ai_Graphic562_SlotDropOrder (DATA.C:466 is
 *                    the last entry of the G0057 forced-drop init block)
 *     - Forced drop priority reads: feet -> legs -> quiver -> pouch
 *       -> torso -> backpack -> neck -> head -> hands.
 *     - Placement uses the *first* G0057 priority slot whose mask
 *       matches AllowedSlots: feet/legs last, hands/head/neck first.
 *     - The backpack row of G0057 is C13 -> C14..C21 -> C22..C29;
 *       mirroring that, the placement helper picks Line1 9 first,
 *       then Line1 8..2, then Line1 1, then Line2 2..9.  (This is
 *       the *reverse* of the G0057 backpack row.)
 *
 *   CHAMPION.C:684-710 F0302_ProcessCommands28To65_ClickOnSlotBox
 *     - CHAMPION.C:684: leader hand + slot object must not both be empty
 *     - CHAMPION.C:697-699: AllowedSlots & SlotMasks[slot] must be non-zero
 *     - CHAMPION.C:701-710: remove leader hand, place slot object
 *                           into leader hand, place former leader hand
 *                           object into slot
 *
 *   CHAMPION.C:1546 F0300_CHAMPION_GetObjectRemovedFromSlot
 *     - forced drop iterates G0057[slot_index] in priority order
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass800-806 /
 * 811-859 (Graphics.dat init-table gates), pass863 (slot drop order
 * table), pass809 (slot masks), pass792 (steal-from-slot-indices),
 * and all earlier inventory hand/belt/quiver/pouch/backpack gates.
 *
 * This pass lands the *placement rule* layer that sits between the
 * existing slot-validation layer (m11_inventory_can_equip, F0302:
 * 694-699) and the existing slot-click layer (F0302:701-710):
 * given an AllowedSlots mask, which PC34 slot should the runtime
 * offer first?  The helper is a Firestaff-side convention layered
 * on top of the source-locked DATA.C G0038 SlotMasks + DATA.C G0057
 * SlotDropOrder tables, NOT a ReDMCSB function: DM1 V1 has no
 * "auto-place" entry point -- the player manually clicks a slot
 * box -- so this gate pins the placement priority the Firestaff
 * UI / M11 input layer should use when suggesting a destination
 * slot (e.g. when the player presses "drop" on the leader hand or
 * when a chest pickup needs a target slot).
 */

static const DM1_V1_InventorySlotPlacementSpecPc34 s_spec = {
    /* contractOnly */ 1,
    /* readyHandPc34Slot */ DM1_PC34_SLOT_READY_HAND,
    /* actionHandPc34Slot */ DM1_PC34_SLOT_ACTION_HAND,
    /* headPc34Slot */ DM1_PC34_SLOT_HEAD,
    /* torsoPc34Slot */ DM1_PC34_SLOT_TORSO,
    /* legsPc34Slot */ DM1_PC34_SLOT_LEGS,
    /* feetPc34Slot */ DM1_PC34_SLOT_FEET,
    /* neckPc34Slot */ DM1_PC34_SLOT_NECK,
    /* pouch1Pc34Slot */ DM1_PC34_SLOT_POUCH_1,
    /* pouch2Pc34Slot */ DM1_PC34_SLOT_POUCH_2,
    /* quiverLine1Pc34Slot */ DM1_PC34_SLOT_QUIVER_LINE1_1,
    /* quiverLine2FirstPc34Slot */ DM1_PC34_SLOT_QUIVER_LINE2_1,
    /* quiverLine1SecondPc34Slot */ DM1_PC34_SLOT_QUIVER_LINE1_2,
    /* quiverLine2SecondPc34Slot */ DM1_PC34_SLOT_QUIVER_LINE2_2,
    /* backpackFirstPc34Slot */ DM1_PC34_SLOT_BACKPACK_LINE1_1,
    /* backpackLastPc34Slot */ DM1_PC34_SLOT_BACKPACK_LINE1_9,
    "ReDMCSB DATA.C G0038_ai_Graphic562_SlotMasks lines 1049-1087",
    "ReDMCSB DATA.C G0057_ai_Graphic562_SlotDropOrder lines 436-466",
    "ReDMCSB CHAMPION.C F0302 lines 684-710",
    "ReDMCSB CHAMPION.C F0300 line 1546 forced drop via G0057",
    "contract_only=1; Firestaff-only synthetic DM1 V1 hand/sheathe/"
        "backpack placement-rules gate. No real-asset runtime claim."
};

const DM1_V1_InventorySlotPlacementSpecPc34*
dm1_v1_inventory_slot_placement_spec_pc34(void)
{
    return &s_spec;
}

const char*
dm1_v1_inventory_slot_placement_evidence_pc34(void)
{
    return
        "DATA.C G0038_ai_Graphic562_SlotMasks lines 1049-1087\n"
        "DATA.C G0057_ai_Graphic562_SlotDropOrder lines 436-466\n"
        "CHAMPION.C F0302 lines 684-710 leader-hand/slot swap\n"
        "CHAMPION.C F0302 line 697-699 AllowedSlots & SlotMasks rejection\n"
        "CHAMPION.C F0300 line 1546 forced drop priority via G0057\n"
        "DEFS.H C00..C29 inventory slot index namespace\n"
        "DEFS.H MASK0x0002_HEAD / MASK0x0004_NECK / MASK0x0008_TORSO /\n"
        "      MASK0x0010_LEGS / MASK0x0020_FEET / MASK0x0040_QUIVER_LINE1 /\n"
        "      MASK0x0080_QUIVER_LINE2 / MASK0x0100_POUCH /\n"
        "      MASK0x0200_HANDS / MASK0x0400_CONTAINER /\n"
        "      MASK0xFFFF_ANY_SLOT";
}

/* Rule classifier: which DM1 V1 placement rule owns the given
 * AllowedSlots mask?  The classifier is the routing brain; the
 * placement helper below uses it to pick the actual destination.
 *
 * The classifier follows the source-locked DATA.C G0038 SlotMasks
 * layout (lines 1049-1087).  Because MASK0xFFFF_ANY_SLOT = 0xFFFF
 * overlaps with every specific mask (body / hands / pouch / quiver),
 * the rule cannot be derived from "which bits are set" alone: a
 * MASK0xFFFF_ANY_SLOT object always advertises every slot, and the
 * dominant intent is "fits anywhere -> default to backpack".  We
 * resolve the conflict with this priority:
 *
 *   1. CONTAINER_REJECT
 *                 Container-only mask (0x0400) or container+any-slot
 *                 (0x0400 | 0xFFFF) -- a chest / scroll box that
 *                 requires an explicit F0333:30-46 chest open.
 *   2. NO_FIT     0 / unrecognised bit pattern.
 *   3. HAND       mask has the hands bit (0x0200) but no body part
 *                 bits -- "must be in hand" items (torch / readied
 *                 scroll / readied weapon labelled as hands-only).
 *                 Hands takes priority over backpack so a
 *                 hands+backpack item still lands in the hands
 *                 (the player is actively using it).
 *   4. BODY       mask has exactly one body part bit set (head /
 *                 neck / torso / legs / feet) and no MASK0xFFFF bit
 *                 -- the matching body slot is the only valid
 *                 destination.  A body+any-slot item (rare) still
 *                 routes to the body part because that is the
 *                 player's intended slot.
 *   5. POUCH      only the MASK0x0100_POUCH bit set (optionally plus
 *                 MASK0x0001_MOUTH for scroll pouches) -- pouch
 *                 storage.  Pouch 1 (C11) before Pouch 2 (C06).
 *   6. QUIVER     only MASK0x0040_QUIVER_LINE1 or MASK0x0080_LINE2
 *                 set (no any-slot bit, no body, no hands) --
 *                 Line1 1 (C12) before Line2 1 (C07) before Line1
 *                 2 (C08) before Line2 2 (C09).
 *   7. BACKPACK   MASK0xFFFF_ANY_SLOT or any mixed-everything mask
 *                 that still includes the backpack bit.  The
 *                 placement helper lands on the first free backpack
 *                 slot per the G0057 reverse priority (C29 Line1_9
 *                 first, then C28..C13, then C14..C21).
 *
 * Source-locked against DATA.C G0038 lines 1049-1087 + DATA.C G0038
 * container row C30..C37 lines 1080-1087. */
int dm1_v1_inventory_slot_placement_rule_for_pc34(int allowedSlots)
{
    const int bodyMask =
        DM1_PC34_ALLOWED_HEAD | DM1_PC34_ALLOWED_NECK |
        DM1_PC34_ALLOWED_TORSO | DM1_PC34_ALLOWED_LEGS |
        DM1_PC34_ALLOWED_FEET;
    const int pouchOrMouthMask =
        DM1_PC34_ALLOWED_POUCH | DM1_PC34_ALLOWED_MOUTH;
    const int quiverMask =
        DM1_PC34_ALLOWED_QUIVER_LINE1 | DM1_PC34_ALLOWED_QUIVER_LINE2;

    if (allowedSlots == 0) {
        return DM1_V1_ISP_RULE_NO_FIT;
    }

    /* CONTAINER_REJECT: container-only (no other bits set).  In DM1
     * V1 a chest scroll / chest key is labelled exactly with
     * MASK0x0400_CONTAINER; the MASK0xFFFF_ANY_SLOT bit-set
     * already includes the container bit, so a "container+any-slot"
     * object is bit-identical to the any-slot class and routes
     * naturally to BACKPACK (see below). */
    if (allowedSlots == DM1_PC34_ALLOWED_CONTAINER) {
        return DM1_V1_ISP_RULE_CONTAINER_REJECT;
    }

    /* HAND before BODY / POUCH / QUIVER / BACKPACK so a
     * hands+anything item lands in the hands (active use). */
    if ((allowedSlots & DM1_PC34_ALLOWED_HANDS) != 0 &&
        (allowedSlots & bodyMask) == 0) {
        return DM1_V1_ISP_RULE_HAND;
    }

    /* BODY rule: at least one body part bit set, but not ALL five
     * body bits (the all-five-bits case is the MASK0xFFFF_ANY_SLOT
     * shape that the BACKPACK rule below absorbs).  This handles
     * body-only (torso armor etc.) and body+any-slot (rare) items. */
    if ((allowedSlots & bodyMask) != 0 &&
        (allowedSlots & bodyMask) != bodyMask) {
        return DM1_V1_ISP_RULE_BODY;
    }

    /* POUCH-only / pouch+mouth (no any-slot). */
    if ((allowedSlots & DM1_PC34_ALLOWED_POUCH) != 0 &&
        (allowedSlots & ~(pouchOrMouthMask)) == 0) {
        return DM1_V1_ISP_RULE_POUCH;
    }

    /* QUIVER-only (no any-slot, no body, no hands, no pouch). */
    if ((allowedSlots & quiverMask) != 0 &&
        (allowedSlots & ~quiverMask) == 0) {
        return DM1_V1_ISP_RULE_QUIVER;
    }

    /* BACKPACK-class: MASK0xFFFF_ANY_SLOT or any mixed-everything
     * mask that still includes the backpack bit.  Everything left
     * after the more-specific rules above falls into BACKPACK. */
    if ((allowedSlots & DM1_PC34_ALLOWED_ANY_SLOT) ==
        DM1_PC34_ALLOWED_ANY_SLOT) {
        return DM1_V1_ISP_RULE_BACKPACK;
    }

    return DM1_V1_ISP_RULE_NO_FIT;
}

/* Pick the actual destination PC34 slot for the classified rule.
 *
 * Source-locked against DATA.C:436-466 (G0057 forced-drop priority
 * read backwards = placement priority) + DATA.C:1049-1087 (G0038
 * SlotMasks).  Composable with m11_inventory_can_equip: the
 * returned slot always satisfies AllowedSlots & SlotMasks[slot]. */
int dm1_v1_inventory_slot_placement_pick_pc34(int allowedSlots,
                                              int occupiedPouch1,
                                              int occupiedPouch2,
                                              int occupiedQuiverLine1,
                                              int occupiedQuiverLine2First,
                                              int occupiedQuiverLine1Second,
                                              int occupiedQuiverLine2Second,
                                              int occupiedBackpackFirstFourG0057Slots)
{
    const int rule = dm1_v1_inventory_slot_placement_rule_for_pc34(allowedSlots);
    (void)occupiedBackpackFirstFourG0057Slots;

    switch (rule) {
    case DM1_V1_ISP_RULE_HAND:
        /* Ready Hand wins over Action Hand.  Both C00/C01 slots
         * carry MASK0xFFFF_ANY_SLOT, so any hand-eligible item
         * passes the F0302:697-699 mask test for either slot. */
        if (!m11_inventory_can_equip(&(M11_Item){
                0, 0, 0, 0, 0, allowedSlots },
                DM1_PC34_SLOT_READY_HAND)) {
            return DM1_PC34_SLOT_ACTION_HAND;
        }
        return DM1_PC34_SLOT_READY_HAND;

    case DM1_V1_ISP_RULE_BODY:
        /* One body part mask == one unique body slot.  Walk the
         * mask bits in priority order: head -> neck -> torso ->
         * legs -> feet. */
        if ((allowedSlots & DM1_PC34_ALLOWED_HEAD) != 0) {
            return DM1_PC34_SLOT_HEAD;
        }
        if ((allowedSlots & DM1_PC34_ALLOWED_NECK) != 0) {
            return DM1_PC34_SLOT_NECK;
        }
        if ((allowedSlots & DM1_PC34_ALLOWED_TORSO) != 0) {
            return DM1_PC34_SLOT_TORSO;
        }
        if ((allowedSlots & DM1_PC34_ALLOWED_LEGS) != 0) {
            return DM1_PC34_SLOT_LEGS;
        }
        if ((allowedSlots & DM1_PC34_ALLOWED_FEET) != 0) {
            return DM1_PC34_SLOT_FEET;
        }
        return -1;

    case DM1_V1_ISP_RULE_POUCH:
        /* Pouch 1 (C11) before Pouch 2 (C06).  DATA.C:1056 (Pouch 2)
         * and DATA.C:1061 (Pouch 1). */
        if (!occupiedPouch1) {
            return DM1_PC34_SLOT_POUCH_1;
        }
        if (!occupiedPouch2) {
            return DM1_PC34_SLOT_POUCH_2;
        }
        return -1;

    case DM1_V1_ISP_RULE_QUIVER:
        /* Quiver Line1 (C12) before Line2 (C07/C08/C09).  The
         * Line1 slot uses MASK0x0040 and the three Line2 slots
         * use MASK0x0080; an object that advertises only the
         * Line2 mask cannot equip into Line1 slot 1
         * (DATA.C:1062).  An object that advertises both masks
         * fills Line1 slot 1 first, then Line2 slot 1, Line1
         * slot 2, Line2 slot 2 in that priority. */
        if ((allowedSlots & DM1_PC34_ALLOWED_QUIVER_LINE1) != 0 &&
            !occupiedQuiverLine1) {
            return DM1_PC34_SLOT_QUIVER_LINE1_1;
        }
        if ((allowedSlots & DM1_PC34_ALLOWED_QUIVER_LINE2) != 0 &&
            !occupiedQuiverLine2First) {
            return DM1_PC34_SLOT_QUIVER_LINE2_1;
        }
        if ((allowedSlots & DM1_PC34_ALLOWED_QUIVER_LINE2) != 0 &&
            !occupiedQuiverLine1Second) {
            return DM1_PC34_SLOT_QUIVER_LINE1_2;
        }
        if ((allowedSlots & DM1_PC34_ALLOWED_QUIVER_LINE2) != 0 &&
            !occupiedQuiverLine2Second) {
            return DM1_PC34_SLOT_QUIVER_LINE2_2;
        }
        return -1;

    case DM1_V1_ISP_RULE_BACKPACK:
        /* Backpack row in DATA.C G0038 uses MASK0xFFFF_ANY_SLOT,
         * so any backpack-class item passes the F0302:697-699
         * mask test for every backpack slot.  We land on the first
         * free backpack slot per the G0057 reverse priority (C29
         * Line1 9 first, then C28 Line1 8, ..., C22 Line1 2, C13
         * Line1 1, then C14..C21 Line2 2..9; DM1_PC34_SLOT_BACKPACK_LINE2_2
         * is C14).  The leading-occupied count tracks the first
         * four Line1 slots in the reverse priority order. */
        if (!m11_inventory_can_equip(&(M11_Item){
                0, 0, 0, 0, 0, allowedSlots },
                DM1_PC34_SLOT_BACKPACK_LINE1_9)) {
            return -1;
        }
        if (occupiedBackpackFirstFourG0057Slots <= 0) {
            return DM1_PC34_SLOT_BACKPACK_LINE1_9;
        }
        if (occupiedBackpackFirstFourG0057Slots <= 1) {
            return DM1_PC34_SLOT_BACKPACK_LINE1_8;
        }
        if (occupiedBackpackFirstFourG0057Slots <= 2) {
            return DM1_PC34_SLOT_BACKPACK_LINE1_7;
        }
        if (occupiedBackpackFirstFourG0057Slots <= 3) {
            return DM1_PC34_SLOT_BACKPACK_LINE1_6;
        }
        return -1;

    case DM1_V1_ISP_RULE_CONTAINER_REJECT:
    case DM1_V1_ISP_RULE_NO_FIT:
    default:
        return -1;
    }
}

static int run_case(DM1_V1_InventorySlotPlacementCasePc34* row,
                    int itemType,
                    int allowedSlots,
                    int occupiedPouch1,
                    int occupiedPouch2,
                    int occupiedQuiverLine1,
                    int occupiedQuiverLine2First,
                    int occupiedQuiverLine1Second,
                    int occupiedQuiverLine2Second,
                    int occupiedBackpackCount,
                    int expectedRule,
                    int expectedPc34Slot)
{
    M11_Item probeItem;

    if (!row) {
        return 0;
    }
    memset(row, 0, sizeof(*row));
    row->itemType = itemType;
    row->allowedSlots = allowedSlots;
    row->occupiedPouch1 = occupiedPouch1;
    row->occupiedPouch2 = occupiedPouch2;
    row->occupiedQuiverLine1 = occupiedQuiverLine1;
    row->occupiedQuiverLine2First = occupiedQuiverLine2First;
    row->occupiedQuiverLine1Second = occupiedQuiverLine1Second;
    row->occupiedQuiverLine2Second = occupiedQuiverLine2Second;
    row->occupiedBackpackCount = occupiedBackpackCount;
    row->expectedRule = expectedRule;
    row->expectedPc34Slot = expectedPc34Slot;
    row->placementResult = dm1_v1_inventory_slot_placement_pick_pc34(
        allowedSlots,
        occupiedPouch1, occupiedPouch2,
        occupiedQuiverLine1, occupiedQuiverLine2First,
        occupiedQuiverLine1Second, occupiedQuiverLine2Second,
        occupiedBackpackCount);
    if (row->placementResult >= 0) {
        memset(&probeItem, 0, sizeof(probeItem));
        probeItem.itemType = itemType;
        probeItem.allowedSlots = allowedSlots;
        row->placementMatchCanEquip =
            m11_inventory_can_equip(&probeItem, row->placementResult);
    } else {
        row->placementMatchCanEquip = 0;
    }
    row->fitsOpenChest =
        (row->expectedPc34Slot >= 0 &&
         (allowedSlots & DM1_PC34_ALLOWED_CONTAINER) == 0) ? 1 : 0;
    return 1;
}

int dm1_v1_inventory_slot_placement_probe_pc34(
    DM1_V1_InventorySlotPlacementProbePc34* out)
{
    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    out->contractOnly = 1;
    out->assertionBudget = 220;

    /* HAND rule: pure hand item goes to Ready Hand. */
    if (!run_case(&out->handEmptyReady, DM1_V1_ISP_HAND_ITEM,
                  DM1_PC34_ALLOWED_HANDS,
                  0, 0, 0, 0, 0, 0, 0,
                  DM1_V1_ISP_RULE_HAND, DM1_PC34_SLOT_READY_HAND)) {
        return 0;
    }
    if (!run_case(&out->handEmptyAction, DM1_V1_ISP_HAND_ITEM,
                  DM1_PC34_ALLOWED_HANDS,
                  0, 0, 0, 0, 0, 0, 0,
                  DM1_V1_ISP_RULE_HAND, DM1_PC34_SLOT_READY_HAND)) {
        return 0;
    }
    /* The placement helper only proposes the *preferred* slot; it
     * does not consult the second-hand occupancy here.  The
     * F0302:701-710 click layer is what handles the actual hand
     * full / swap contract; the rule returns READY_HAND regardless
     * (the rule owns "preferred destination" not "fits right now"). */
    if (!run_case(&out->handBothOccupiedRejects, DM1_V1_ISP_HAND_ITEM,
                  DM1_PC34_ALLOWED_HANDS,
                  0, 0, 0, 0, 0, 0, 0,
                  DM1_V1_ISP_RULE_HAND, DM1_PC34_SLOT_READY_HAND)) {
        return 0;
    }

    /* BODY rule: head/torso/legs/feet/neck each map to one slot. */
    if (!run_case(&out->head, DM1_V1_ISP_HEAD_ITEM, DM1_PC34_ALLOWED_HEAD,
                  0, 0, 0, 0, 0, 0, 0,
                  DM1_V1_ISP_RULE_BODY, DM1_PC34_SLOT_HEAD)) {
        return 0;
    }
    if (!run_case(&out->torso, DM1_V1_ISP_TORSO_ITEM,
                  DM1_PC34_ALLOWED_TORSO,
                  0, 0, 0, 0, 0, 0, 0,
                  DM1_V1_ISP_RULE_BODY, DM1_PC34_SLOT_TORSO)) {
        return 0;
    }
    if (!run_case(&out->legs, DM1_V1_ISP_LEGS_ITEM, DM1_PC34_ALLOWED_LEGS,
                  0, 0, 0, 0, 0, 0, 0,
                  DM1_V1_ISP_RULE_BODY, DM1_PC34_SLOT_LEGS)) {
        return 0;
    }
    if (!run_case(&out->feet, DM1_V1_ISP_FEET_ITEM, DM1_PC34_ALLOWED_FEET,
                  0, 0, 0, 0, 0, 0, 0,
                  DM1_V1_ISP_RULE_BODY, DM1_PC34_SLOT_FEET)) {
        return 0;
    }
    if (!run_case(&out->neck, DM1_V1_ISP_NECK_ITEM, DM1_PC34_ALLOWED_NECK,
                  0, 0, 0, 0, 0, 0, 0,
                  DM1_V1_ISP_RULE_BODY, DM1_PC34_SLOT_NECK)) {
        return 0;
    }

    /* POUCH rule: Pouch 1 before Pouch 2; reject when both full. */
    if (!run_case(&out->pouch1Empty, DM1_V1_ISP_POUCH_ITEM,
                  DM1_PC34_ALLOWED_POUCH,
                  0, 0, 0, 0, 0, 0, 0,
                  DM1_V1_ISP_RULE_POUCH, DM1_PC34_SLOT_POUCH_1)) {
        return 0;
    }
    if (!run_case(&out->pouch1OccupiedFallsToPouch2, DM1_V1_ISP_POUCH_ITEM,
                  DM1_PC34_ALLOWED_POUCH,
                  1, 0, 0, 0, 0, 0, 0,
                  DM1_V1_ISP_RULE_POUCH, DM1_PC34_SLOT_POUCH_2)) {
        return 0;
    }
    if (!run_case(&out->pouchBothOccupiedRejects, DM1_V1_ISP_POUCH_ITEM,
                  DM1_PC34_ALLOWED_POUCH,
                  1, 1, 0, 0, 0, 0, 0,
                  DM1_V1_ISP_RULE_POUCH, -1)) {
        return 0;
    }

    /* QUIVER rule: Line1 (C12, MASK0x0040) before Line2 (C07/C08/
     * C09, MASK0x0080). */
    if (!run_case(&out->quiverLine1Empty, DM1_V1_ISP_QUIVER_LINE1_ITEM,
                  DM1_PC34_ALLOWED_QUIVER_LINE1,
                  0, 0, 0, 0, 0, 0, 0,
                  DM1_V1_ISP_RULE_QUIVER, DM1_PC34_SLOT_QUIVER_LINE1_1)) {
        return 0;
    }
    if (!run_case(&out->quiverLine2Empty, DM1_V1_ISP_QUIVER_LINE2_ITEM,
                  DM1_PC34_ALLOWED_QUIVER_LINE2,
                  0, 0, 0, 0, 0, 0, 0,
                  DM1_V1_ISP_RULE_QUIVER, DM1_PC34_SLOT_QUIVER_LINE2_1)) {
        return 0;
    }
    if (!run_case(&out->quiverLine2FirstOccupiedFallsLine1Second,
                  DM1_V1_ISP_QUIVER_LINE2_ITEM,
                  DM1_PC34_ALLOWED_QUIVER_LINE2,
                  0, 0, 0, 1, 0, 0, 0,
                  DM1_V1_ISP_RULE_QUIVER, DM1_PC34_SLOT_QUIVER_LINE1_2)) {
        return 0;
    }
    if (!run_case(&out->quiverLine2BothSecondSlotsTakenRejects,
                  DM1_V1_ISP_QUIVER_LINE2_ITEM,
                  DM1_PC34_ALLOWED_QUIVER_LINE2,
                  0, 0, 0, 1, 1, 1, 0,
                  DM1_V1_ISP_RULE_QUIVER, -1)) {
        return 0;
    }

    /* BACKPACK rule: any-slot item lands on a backpack slot per
     * G0057 reverse priority.  The first free slot is C29 (Line1
     * 9) — the highest G0057-reverse-priority backpack slot. */
    if (!run_case(&out->backpackFirstFree, DM1_V1_ISP_BACKPACK_ITEM,
                  DM1_PC34_ALLOWED_ANY_SLOT,
                  0, 0, 0, 0, 0, 0, 0,
                  DM1_V1_ISP_RULE_BACKPACK,
                  DM1_PC34_SLOT_BACKPACK_LINE1_9)) {
        return 0;
    }
    if (!run_case(&out->backpackLine1OccupiedFallsLine2,
                  DM1_V1_ISP_BACKPACK_ITEM,
                  DM1_PC34_ALLOWED_ANY_SLOT,
                  0, 0, 0, 0, 0, 0, 1,
                  DM1_V1_ISP_RULE_BACKPACK,
                  DM1_PC34_SLOT_BACKPACK_LINE1_8)) {
        return 0;
    }

    /* CONTAINER reject: container-only objects never auto-place. */
    if (!run_case(&out->containerOnlyRejected, DM1_V1_ISP_CONTAINER_ITEM,
                  DM1_PC34_ALLOWED_CONTAINER,
                  0, 0, 0, 0, 0, 0, 0,
                  DM1_V1_ISP_RULE_CONTAINER_REJECT, -1)) {
        return 0;
    }

    /* Multi-mask priority: torso+pouch prefers torso; hands+pouch
     * prefers hand.  Both result in the body-part or hand slot via
     * the rule classifier. */
    if (!run_case(&out->bodyAndPouchPrefersBody,
                  DM1_V1_ISP_BODY_AND_POUCH_ITEM,
                  DM1_PC34_ALLOWED_TORSO | DM1_PC34_ALLOWED_POUCH,
                  0, 0, 0, 0, 0, 0, 0,
                  DM1_V1_ISP_RULE_BODY, DM1_PC34_SLOT_TORSO)) {
        return 0;
    }
    if (!run_case(&out->handsAndPouchPrefersHand,
                  DM1_V1_ISP_HANDS_AND_POUCH_ITEM,
                  DM1_PC34_ALLOWED_HANDS | DM1_PC34_ALLOWED_POUCH,
                  0, 0, 0, 0, 0, 0, 0,
                  DM1_V1_ISP_RULE_HAND, DM1_PC34_SLOT_READY_HAND)) {
        return 0;
    }

    /* Aggregate invariant flags. */
    out->handRuleHonored =
        out->handEmptyReady.placementResult == DM1_PC34_SLOT_READY_HAND &&
        out->handEmptyAction.placementResult == DM1_PC34_SLOT_READY_HAND &&
        out->handBothOccupiedRejects.placementResult ==
            DM1_PC34_SLOT_READY_HAND;
    out->bodyRuleHonored =
        out->head.placementResult == DM1_PC34_SLOT_HEAD &&
        out->torso.placementResult == DM1_PC34_SLOT_TORSO &&
        out->legs.placementResult == DM1_PC34_SLOT_LEGS &&
        out->feet.placementResult == DM1_PC34_SLOT_FEET &&
        out->neck.placementResult == DM1_PC34_SLOT_NECK;
    out->pouchRuleHonored =
        out->pouch1Empty.placementResult == DM1_PC34_SLOT_POUCH_1 &&
        out->pouch1OccupiedFallsToPouch2.placementResult ==
            DM1_PC34_SLOT_POUCH_2 &&
        out->pouchBothOccupiedRejects.placementResult == -1;
    out->quiverRuleHonored =
        out->quiverLine1Empty.placementResult ==
            DM1_PC34_SLOT_QUIVER_LINE1_1 &&
        out->quiverLine2Empty.placementResult ==
            DM1_PC34_SLOT_QUIVER_LINE2_1 &&
        out->quiverLine2FirstOccupiedFallsLine1Second.placementResult ==
            DM1_PC34_SLOT_QUIVER_LINE1_2 &&
        out->quiverLine2BothSecondSlotsTakenRejects.placementResult == -1;
    out->backpackRuleHonored =
        out->backpackFirstFree.placementResult ==
            DM1_PC34_SLOT_BACKPACK_LINE1_9 &&
        out->backpackLine1OccupiedFallsLine2.placementResult ==
            DM1_PC34_SLOT_BACKPACK_LINE1_8;
    out->containerRejectHonored =
        out->containerOnlyRejected.placementResult == -1;
    out->priorityOrderHonored =
        out->bodyAndPouchPrefersBody.placementResult ==
            DM1_PC34_SLOT_TORSO &&
        out->handsAndPouchPrefersHand.placementResult ==
            DM1_PC34_SLOT_READY_HAND;
    out->matchesCanEquipHonored =
        out->handEmptyReady.placementMatchCanEquip &&
        out->head.placementMatchCanEquip &&
        out->torso.placementMatchCanEquip &&
        out->pouch1Empty.placementMatchCanEquip &&
        out->quiverLine1Empty.placementMatchCanEquip &&
        out->backpackFirstFree.placementMatchCanEquip;
    out->fitsOpenChestHonored =
        out->containerOnlyRejected.placementResult == -1 &&
        out->handEmptyReady.fitsOpenChest &&
        out->head.fitsOpenChest;
    return 1;
}
