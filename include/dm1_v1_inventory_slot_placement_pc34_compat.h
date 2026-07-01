#ifndef DM1_V1_INVENTORY_SLOT_PLACEMENT_PC34_COMPAT_H
#define DM1_V1_INVENTORY_SLOT_PLACEMENT_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * DM1 V1 inventory hand/sheathe/backpack placement rules.
 *
 * Firestaff-only contract-only gate that pins the ReDMCSB-derived
 * placement rules that decide where a leader-hand object may be
 * auto-placed when the player clicks "drop" / "put away" or when a
 * chest pickup needs a target slot.  The rules are documented in
 *
 *   DATA.C:1049-1087 G0038_ai_Graphic562_SlotMasks
 *   DATA.C:436-466  G0057_ai_Graphic562_SlotDropOrder
 *   CHAMPION.C:677-687 F0302 status hand slot routing
 *   CHAMPION.C:684-710 F0302 empty-slot no-op + AllowedSlots mask
 *                    rejection + leader-hand/slot swap order
 *   CHAMPION.C:1546  F0300 forced-drop uses G0057 priority table
 *
 * The six rules are:
 *
 *   1. HAND:    AllowedSlots & MASK0x0200_HANDS  -> Ready Hand (C00)
 *               else Action Hand (C01).
 *   2. BODY:    AllowedSlots & MASK0x000E body parts -> the matching
 *               head/torso/legs/feet/neck slot only.
 *   3. POUCH:   AllowedSlots & MASK0x0100_POUCH -> Pouch 1 (C11)
 *               else Pouch 2 (C06).
 *   4. QUIVER:  AllowedSlots & MASK0x0040_QUIVER_LINE1 -> Quiver
 *               Line1 1 (C12); AllowedSlots & MASK0x0080_QUIVER_LINE2
 *               -> Quiver Line2 1 (C07) -> Line1 2 (C08) -> Line2 2
 *               (C09) priority.
 *   5. BACKPACK: AllowedSlots == MASK0xFFFF_ANY_SLOT -> first free
 *               backpack slot per G0057 drop-order priority
 *               (C13 Line1_1 -> C14..C21 Line2_2..9 -> C22..C29
 *               Line1_2..9).
 *   6. CONTAINER REJECT: AllowedSlots & MASK0x0400_CONTAINER-only ->
 *               never auto-place; require explicit chest open.
 *
 * The placement helper consumes the champion slot occupancy and a
 * leader-hand item's allowedSlots mask and returns either the chosen
 * PC34 inventory slot (>=0) or -1 if no placement fits.  The probe
 * verifies each rule against the source-locked slot masks and against
 * the existing m11_inventory_can_equip/click gate so the gate is
 * composable with F0302:684-710.
 */

enum {
    DM1_V1_ISP_RULE_NONE = 0,
    DM1_V1_ISP_RULE_HAND = 1,
    DM1_V1_ISP_RULE_BODY = 2,
    DM1_V1_ISP_RULE_POUCH = 3,
    DM1_V1_ISP_RULE_QUIVER = 4,
    DM1_V1_ISP_RULE_BACKPACK = 5,
    DM1_V1_ISP_RULE_CONTAINER_REJECT = 6,
    DM1_V1_ISP_RULE_NO_FIT = 7
};

enum {
    DM1_V1_ISP_HAND_ITEM = 0x9101,
    DM1_V1_ISP_HEAD_ITEM = 0x9102,
    DM1_V1_ISP_TORSO_ITEM = 0x9103,
    DM1_V1_ISP_LEGS_ITEM = 0x9104,
    DM1_V1_ISP_FEET_ITEM = 0x9105,
    DM1_V1_ISP_NECK_ITEM = 0x9106,
    DM1_V1_ISP_POUCH_ITEM = 0x9107,
    DM1_V1_ISP_QUIVER_LINE1_ITEM = 0x9108,
    DM1_V1_ISP_QUIVER_LINE2_ITEM = 0x9109,
    DM1_V1_ISP_BACKPACK_ITEM = 0x910A,
    DM1_V1_ISP_CONTAINER_ITEM = 0x910B,
    DM1_V1_ISP_BODY_AND_POUCH_ITEM = 0x910C,
    DM1_V1_ISP_HANDS_AND_POUCH_ITEM = 0x910D
};

typedef struct {
    int contractOnly;
    int readyHandPc34Slot;
    int actionHandPc34Slot;
    int headPc34Slot;
    int torsoPc34Slot;
    int legsPc34Slot;
    int feetPc34Slot;
    int neckPc34Slot;
    int pouch1Pc34Slot;
    int pouch2Pc34Slot;
    int quiverLine1Pc34Slot;
    int quiverLine2FirstPc34Slot;
    int quiverLine1SecondPc34Slot;
    int quiverLine2SecondPc34Slot;
    int backpackFirstPc34Slot;
    int backpackLastPc34Slot;
    const char* dataSlotMaskAnchor;
    const char* dataDropOrderAnchor;
    const char* championF0302Anchor;
    const char* championF0300Anchor;
    const char* scope;
} DM1_V1_InventorySlotPlacementSpecPc34;

typedef struct {
    int itemType;
    int allowedSlots;
    int occupiedPouch1;
    int occupiedPouch2;
    int occupiedQuiverLine1;
    int occupiedQuiverLine2First;
    int occupiedQuiverLine1Second;
    int occupiedQuiverLine2Second;
    int occupiedBackpackCount;
    int expectedRule;
    int expectedPc34Slot;
    int placementResult;
    int placementMatchCanEquip;
    int fitsOpenChest;
} DM1_V1_InventorySlotPlacementCasePc34;

typedef struct {
    int contractOnly;
    int assertionBudget;
    DM1_V1_InventorySlotPlacementCasePc34 handEmptyReady;
    DM1_V1_InventorySlotPlacementCasePc34 handEmptyAction;
    DM1_V1_InventorySlotPlacementCasePc34 handBothOccupiedRejects;
    DM1_V1_InventorySlotPlacementCasePc34 head;
    DM1_V1_InventorySlotPlacementCasePc34 torso;
    DM1_V1_InventorySlotPlacementCasePc34 legs;
    DM1_V1_InventorySlotPlacementCasePc34 feet;
    DM1_V1_InventorySlotPlacementCasePc34 neck;
    DM1_V1_InventorySlotPlacementCasePc34 pouch1Empty;
    DM1_V1_InventorySlotPlacementCasePc34 pouch1OccupiedFallsToPouch2;
    DM1_V1_InventorySlotPlacementCasePc34 pouchBothOccupiedRejects;
    DM1_V1_InventorySlotPlacementCasePc34 quiverLine1Empty;
    DM1_V1_InventorySlotPlacementCasePc34 quiverLine2Empty;
    DM1_V1_InventorySlotPlacementCasePc34 quiverLine2FirstOccupiedFallsLine1Second;
    DM1_V1_InventorySlotPlacementCasePc34 quiverLine2BothSecondSlotsTakenRejects;
    DM1_V1_InventorySlotPlacementCasePc34 backpackFirstFree;
    DM1_V1_InventorySlotPlacementCasePc34 backpackLine1OccupiedFallsLine2;
    DM1_V1_InventorySlotPlacementCasePc34 containerOnlyRejected;
    DM1_V1_InventorySlotPlacementCasePc34 bodyAndPouchPrefersBody;
    DM1_V1_InventorySlotPlacementCasePc34 handsAndPouchPrefersHand;
    int handRuleHonored;
    int bodyRuleHonored;
    int pouchRuleHonored;
    int quiverRuleHonored;
    int backpackRuleHonored;
    int containerRejectHonored;
    int priorityOrderHonored;
    int fitsOpenChestHonored;
    int matchesCanEquipHonored;
} DM1_V1_InventorySlotPlacementProbePc34;

const DM1_V1_InventorySlotPlacementSpecPc34*
dm1_v1_inventory_slot_placement_spec_pc34(void);

const char*
dm1_v1_inventory_slot_placement_evidence_pc34(void);

/*
 * Compute the preferred placement PC34 slot for a leader-hand item with
 * the given allowedSlots mask, given the champion's current backpack /
 * pouch / quiver occupancy.  Returns the PC34 slot index (>=0) on a
 * fit, or -1 when no inventory slot can accept the item.  The
 * occupancy booleans map one-to-one to the source-locked C06/C07/C08/
 * C09/C11/C12/C13..C29 PC34 slots.
 *
 * The return value is the same as the existing m11_inventory_can_equip
 * positive side: passing the returned slot through
 * m11_inventory_click_pc34_source_slot() should accept the click
 * (m11_inventory_can_equip returns 1).
 */
int dm1_v1_inventory_slot_placement_pick_pc34(int allowedSlots,
                                              int occupiedPouch1,
                                              int occupiedPouch2,
                                              int occupiedQuiverLine1,
                                              int occupiedQuiverLine2First,
                                              int occupiedQuiverLine1Second,
                                              int occupiedQuiverLine2Second,
                                              int occupiedBackpackFirstFourG0057Slots);

int dm1_v1_inventory_slot_placement_rule_for_pc34(int allowedSlots);

int dm1_v1_inventory_slot_placement_probe_pc34(
    DM1_V1_InventorySlotPlacementProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_INVENTORY_SLOT_PLACEMENT_PC34_COMPAT_H */
