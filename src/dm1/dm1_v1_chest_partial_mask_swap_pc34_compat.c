#include "dm1_v1_chest_partial_mask_swap_pc34_compat.h"

#include <string.h>

static const char s_source_evidence[] =
    "CHEST.C F0333:53-67 materializes the first eight linked container objects into G0425_aT_ChestSlots\n"
    "CHEST.C F0334:113-132 rewrites the open container from non-empty G0425_aT_ChestSlots\n"
    "CHAMPION.C F0297/F0298/F0300/F0301/F0302:243-298,511-515,606-610,688-710 owns leader-hand put/remove and C30+ chest-slot swap routing\n"
    "DATA.C G0038_ai_Graphic562_SlotMasks:1050-1087 supplies C30..C37 chest slot masks; F0302 accepts when AllowedSlots & SlotMasks is non-zero\n"
    "DUNGEON.C F0163:1796-1837 clears Next and appends linked visible-input returns\n"
    "CHAMDRAW.C F0291/F0296:551-552,1249-1252 draws visible open-chest icons from G0425_aT_ChestSlots\n"
    "DEFS.H:434,810-817 C0xFFFF_THING_NONE and C30_SLOT_CHEST_1..C37_SLOT_CHEST_8 identify empty things and C30+ chest slots\n"
    "BLITMASK.C F0133:30-33 documents the masked bitmap blit route used by C30+ item icons";

const DM1_V1_ChestPartialMaskSwapSpecPc34
    dm1_v1_chest_partial_mask_swap_pc34_spec = {
        "Source-locked contract gate only; not full real-asset chest runtime parity.",
        DM1_PC34_SLOT_CHEST_1,
        DM1_PC34_SLOT_CHEST_8,
        DM1_PC34_CHEST_PARTIAL_MASK_PC34_SLOT,
        DM1_PC34_CHEST_PARTIAL_MASK_SLOT_INDEX,
        DM1_PC34_ALLOWED_CONTAINER,
        DM1_PC34_CHEST_PARTIAL_MASK_ALLOWED,
        DM1_PC34_ALLOWED_CONTAINER,
        DM1_PC34_CHEST_PARTIAL_MASK_THING_NONE
    };

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

static int copy_closed_chain(const M11_Item* closed,
                             int count,
                             int* typesOut,
                             int* allowedOut)
{
    int i;

    if (!closed || !typesOut || !allowedOut || count < 0) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_PARTIAL_MASK_SLOT_COUNT; ++i) {
        typesOut[i] = i < count ? closed[i].itemType : 0;
        allowedOut[i] = i < count ? closed[i].allowedSlots : 0;
    }
    return 1;
}

static int closed_chain_valid(const DM1_V1_ChestPartialMaskSwapProbePc34* out)
{
    int i;

    if (!out || out->closeCount != DM1_PC34_CHEST_PARTIAL_MASK_SLOT_COUNT) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_PARTIAL_MASK_SLOT_COUNT; ++i) {
        const int expected =
            (i == DM1_PC34_CHEST_PARTIAL_MASK_SLOT_INDEX) ?
            DM1_PC34_CHEST_PARTIAL_MASK_LEADER_ITEM :
            DM1_PC34_CHEST_PARTIAL_MASK_FIRST_ITEM + i;
        if (out->closedTypes[i] != expected) {
            return 0;
        }
    }
    return 1;
}

const char* dm1_v1_chest_partial_mask_swap_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const DM1_V1_ChestPartialMaskSwapSpecPc34*
dm1_v1_chest_partial_mask_swap_spec_pc34(void)
{
    return &dm1_v1_chest_partial_mask_swap_pc34_spec;
}

int dm1_v1_chest_partial_mask_swap_run_pc34(
    DM1_V1_ChestPartialMaskSwapProbePc34* out)
{
    M11_InventoryState state;
    M11_Item linked[DM1_PC34_CHEST_PARTIAL_MASK_SLOT_COUNT];
    M11_Item closed[DM1_PC34_CHEST_PARTIAL_MASK_SLOT_COUNT];
    M11_Item item;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    memset(closed, 0, sizeof(closed));
    out->sourceLockedContractOnly = 1;
    out->chestThing = DM1_PC34_CHEST_PARTIAL_MASK_CHEST_THING;
    out->targetPc34Slot = DM1_PC34_CHEST_PARTIAL_MASK_PC34_SLOT;
    out->targetSlotIndex = DM1_PC34_CHEST_PARTIAL_MASK_SLOT_INDEX;

    m11_inventory_init(&state, 1);
    for (i = 0; i < DM1_PC34_CHEST_PARTIAL_MASK_SLOT_COUNT; ++i) {
        linked[i] = make_item(DM1_PC34_CHEST_PARTIAL_MASK_FIRST_ITEM + i,
                              2 + i,
                              DM1_PC34_ALLOWED_CONTAINER);
    }

    /* ReDMCSB CHEST.C F0333 lines 53-67 copies the visible C537..C544 link
     * window into G0425 before CHAMPION.C F0302 can route a C30+ slot click. */
    out->openResult = m11_inventory_open_chest(
        &state, 0, out->chestThing, linked,
        DM1_PC34_CHEST_PARTIAL_MASK_SLOT_COUNT);
    out->openThing = m11_inventory_get_open_chest_thing(&state, 0);
    if (!out->openResult ||
        !m11_inventory_get_item_in_chest_slot(
            &state, 0, out->targetSlotIndex, &item)) {
        return 0;
    }
    out->slotBefore = item.itemType;

    out->slotMask = m11_inventory_pc34_slot_mask(out->targetPc34Slot);
    out->leaderAllowedSlots = DM1_PC34_CHEST_PARTIAL_MASK_ALLOWED;
    out->maskExactMatch =
        out->leaderAllowedSlots == out->slotMask ? 1 : 0;
    out->maskOverlap = out->leaderAllowedSlots & out->slotMask;
    out->maskOverlapNonZero = out->maskOverlap != 0 ? 1 : 0;
    item = make_item(DM1_PC34_CHEST_PARTIAL_MASK_LEADER_ITEM, 13,
                     out->leaderAllowedSlots);
    out->leaderCanEquip =
        m11_inventory_can_equip(&item, out->targetPc34Slot);

    if (!m11_inventory_set_mouse_item(
            &state, 0, DM1_PC34_CHEST_PARTIAL_MASK_LEADER_ITEM,
            13, 0, out->leaderAllowedSlots) ||
        !m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    out->leaderHandBefore = item.itemType;

    /* ReDMCSB CHAMPION.C F0302 lines 688-710 accepts a leader-hand object
     * when DATA.C G0038 AllowedSlots & SlotMasks is non-zero, removes the
     * leader hand through F0298, removes C30+ G0425 through F0300, then stores
     * the previous leader object through F0301's C30+ path. */
    out->clickResult = m11_inventory_click_pc34_source_slot(
        &state, 0, out->targetPc34Slot);
    if (!m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    out->leaderHandAfter = item.itemType;
    if (!m11_inventory_get_item_in_chest_slot(
            &state, 0, out->targetSlotIndex, &item)) {
        return 0;
    }
    out->slotAfter = item.itemType;
    out->slotAfterAllowedSlots = item.allowedSlots;
    out->leaderReleasedToChestSlot =
        out->slotAfter == DM1_PC34_CHEST_PARTIAL_MASK_LEADER_ITEM &&
        out->slotAfterAllowedSlots == out->leaderAllowedSlots ? 1 : 0;
    out->slotOccupantPreservedInLeaderHand =
        out->leaderHandAfter == out->slotBefore ? 1 : 0;

    /* ReDMCSB CHEST.C F0334 lines 113-132 and DUNGEON.C F0163 lines
     * 1796-1837 rewrite the non-empty C537..C544 G0425 window as a valid
     * linked visible chain after the partial-mask C30+ swap. */
    out->closeCount = m11_inventory_close_chest(
        &state, 0, closed, DM1_PC34_CHEST_PARTIAL_MASK_SLOT_COUNT);
    if (out->closeCount < 0 ||
        !copy_closed_chain(closed, out->closeCount, out->closedTypes,
                           out->closedAllowedSlots)) {
        return 0;
    }
    out->closedTargetIsPartialLeaderItem =
        out->closedTypes[out->targetSlotIndex] ==
        DM1_PC34_CHEST_PARTIAL_MASK_LEADER_ITEM ? 1 : 0;
    out->closedChainValid = closed_chain_valid(out);
    out->openChestClearedAfterClose =
        m11_inventory_get_open_chest_thing(&state, 0) == 0 ? 1 : 0;

    return 1;
}
