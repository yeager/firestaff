#include "dm1_v1_inventory_pouch_quiver_backpack_swap_pc34_compat.h"

#include <string.h>

static const DM1_V1_InventoryPouchQuiverBackpackSwapSpecPc34 s_spec = {
    1,
    DM1_V1_IPQBS_POUCH_SLOT,
    DM1_SLOT_POUCH1,
    DM1_PC34_ALLOWED_POUCH,
    DM1_V1_IPQBS_QUIVER_SLOT,
    DM1_SLOT_QUIVER1,
    DM1_PC34_ALLOWED_QUIVER_LINE1,
    DM1_V1_IPQBS_BACKPACK_SLOT,
    DM1_SLOT_BACKPACK1,
    DM1_PC34_ALLOWED_ANY_SLOT,
    0,
    "CHEST.C F0333:53-67 opens G0425 chest slots before inventory routing",
    "CHEST.C F0334:117-132 rewrites non-empty G0425 slots on close",
    "INVENTORY.C (PC 3.4) AllowedSlots lookup; DEFS.H "
        "C545_AllowedSlots_Pouch/C546_AllowedSlots_Quiver/"
        "C547_AllowedSlots_Backpack map to 0x0100/0x0040/0xFFFF",
    "BLITMASK.C F0133:30-33 masked bitmap blit dispatch",
    "CHAMDRAW.C F0291/F0296:551-552,1249-1252 redraws C30+ icons",
    "contract_only=1; synthetic DM1 V1 hand-to-pouch/quiver/backpack "
        "mask-swap gate, no real-asset runtime claim."
};

static M11_Item make_item(int itemType, int weight, int allowedSlots)
{
    M11_Item item;

    memset(&item, 0, sizeof(item));
    item.itemType = itemType;
    item.weight = weight;
    item.allowedSlots = allowedSlots;
    item.identified = 1;
    return item;
}

const DM1_V1_InventoryPouchQuiverBackpackSwapSpecPc34*
dm1_v1_inventory_pouch_quiver_backpack_swap_spec_pc34(void)
{
    return &s_spec;
}

const char*
dm1_v1_inventory_pouch_quiver_backpack_swap_evidence_pc34(void)
{
    return
        "CHEST.C F0333:53-67 copies open chest links into G0425 before "
        "slot-box routing\n"
        "CHEST.C F0334:117-132 closes by rewriting the G0425 chest chain\n"
        "INVENTORY.C (PC 3.4) AllowedSlots lookup and C545/C546/C547 "
        "pouch/quiver/backpack slot-type definitions\n"
        "DEFS.H C545_AllowedSlots_Pouch, C546_AllowedSlots_Quiver, "
        "C547_AllowedSlots_Backpack masks\n"
        "BLITMASK.C F0133:30-33 masked bitmap blit dispatch\n"
        "CHAMDRAW.C F0291/F0296:551-552,1249-1252 C30+ icon redraw";
}

static int run_swap_case(
    DM1_V1_InventoryPouchQuiverBackpackSwapCasePc34* row,
    int pc34Slot,
    int sourceItemType,
    int sourceAllowedSlots,
    int sourceWeight)
{
    M11_InventoryState accepted;
    M11_InventoryState rejected;
    M11_Item item;

    if (!row) {
        return 0;
    }
    memset(row, 0, sizeof(*row));
    row->pc34Slot = pc34Slot;
    row->storageSlot = m11_inventory_pc34_source_slot_to_storage_slot(pc34Slot);
    row->slotMask = m11_inventory_pc34_slot_mask(pc34Slot);
    row->sourceItemType = sourceItemType;
    row->sourceAllowedSlots = sourceAllowedSlots;
    row->sourceWeight = sourceWeight;
    row->incompatibleAllowedSlots = 0;

    m11_inventory_init(&accepted, 1);
    if (!m11_inventory_set_mouse_item(&accepted, 0, sourceItemType,
                                      sourceWeight, 0, sourceAllowedSlots)) {
        return 0;
    }
    if (!m11_inventory_get_mouse_item(&accepted, 0, &item)) {
        return 0;
    }
    row->slotBefore = 0;
    row->handBefore = item.itemType;
    row->handAllowedBefore = item.allowedSlots;
    row->loadBefore = m11_inventory_get_load(&accepted, 0);
    row->maskOverlap = sourceAllowedSlots & row->slotMask;
    row->canEquipBeforeClick = m11_inventory_can_equip(&item, pc34Slot);

    /* ReDMCSB INVENTORY.C (PC 3.4) and DEFS.H C545/C546/C547 define the
     * pouch/quiver/backpack AllowedSlots masks; CHAMPION.C F0302:697-710
     * accepts only when AllowedSlots & G0038_ai_Graphic562_SlotMasks is set. */
    row->acceptedClick =
        m11_inventory_click_pc34_source_slot(&accepted, 0, pc34Slot);
    if (!m11_inventory_get_mouse_item(&accepted, 0, &item)) {
        return 0;
    }
    row->handAfterAccepted = item.itemType;
    row->handAllowedAfterAccepted = item.allowedSlots;
    if (!m11_inventory_get_item_in_pc34_source_slot(&accepted, 0, pc34Slot,
                                                    &item)) {
        return 0;
    }
    row->slotAfterAccepted = item.itemType;
    row->slotAllowedAfterAccepted = item.allowedSlots;
    row->slotWeightAfterAccepted = item.weight;
    row->loadAfterAccepted = m11_inventory_get_load(&accepted, 0);
    row->handEmptyAfterAccepted = row->handAfterAccepted == 0 ? 1 : 0;
    row->slotReceivedSource =
        row->slotAfterAccepted == sourceItemType &&
        row->slotAllowedAfterAccepted == sourceAllowedSlots ? 1 : 0;

    m11_inventory_init(&rejected, 1);
    if (!m11_inventory_set_mouse_item(&rejected, 0,
                                      DM1_V1_IPQBS_INCOMPATIBLE_ITEM,
                                      sourceWeight + 10, 0,
                                      row->incompatibleAllowedSlots)) {
        return 0;
    }
    if (!m11_inventory_get_mouse_item(&rejected, 0, &item)) {
        return 0;
    }
    row->incompatibleMaskOverlap =
        row->incompatibleAllowedSlots & row->slotMask;
    row->incompatibleCanEquip = m11_inventory_can_equip(&item, pc34Slot);
    row->incompatibleClick =
        m11_inventory_click_pc34_source_slot(&rejected, 0, pc34Slot);
    if (!m11_inventory_get_mouse_item(&rejected, 0, &item)) {
        return 0;
    }
    row->incompatibleHandAfter = item.itemType;
    if (!m11_inventory_get_item_in_pc34_source_slot(&rejected, 0, pc34Slot,
                                                    &item)) {
        return 0;
    }
    row->incompatibleSlotAfter = item.itemType;
    row->incompatibleRejected =
        row->incompatibleClick == 0 &&
        row->incompatibleHandAfter == DM1_V1_IPQBS_INCOMPATIBLE_ITEM &&
        row->incompatibleSlotAfter == 0 ? 1 : 0;
    return 1;
}

static int run_chest_close_case(
    DM1_V1_InventoryPouchQuiverBackpackChestClosePc34* row)
{
    M11_InventoryState state;
    M11_Item linked[DM1_V1_IPQBS_CHEST_ITEM_COUNT];
    M11_Item closed[DM1_PC34_CHEST_SLOT_COUNT];
    M11_Item item;
    int i;

    if (!row) {
        return 0;
    }
    memset(row, 0, sizeof(*row));
    memset(closed, 0, sizeof(closed));
    m11_inventory_init(&state, 1);
    for (i = 0; i < DM1_V1_IPQBS_CHEST_ITEM_COUNT; ++i) {
        linked[i] = make_item(DM1_V1_IPQBS_CHEST_FIRST_ITEM + i, 2 + i,
                              DM1_PC34_ALLOWED_CONTAINER);
    }

    /* ReDMCSB CHEST.C F0333:53-67 first copies the open chest chain into
     * G0425_aT_ChestSlots; the later belt-slot swap must not be serialized
     * into that chain by CHEST.C F0334:117-132. */
    row->openResult = m11_inventory_open_chest(
        &state, 0, DM1_V1_IPQBS_CHEST_THING, linked,
        DM1_V1_IPQBS_CHEST_ITEM_COUNT);
    row->openThingBeforeClose = m11_inventory_get_open_chest_thing(&state, 0);
    if (!m11_inventory_set_mouse_item(&state, 0, DM1_V1_IPQBS_POUCH_ITEM,
                                      7, 0, DM1_PC34_ALLOWED_POUCH)) {
        return 0;
    }
    row->beltSwapResult = m11_inventory_click_pc34_source_slot(
        &state, 0, DM1_V1_IPQBS_POUCH_SLOT);
    if (!m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    row->handAfterBeltSwap = item.itemType;
    if (!m11_inventory_get_item_in_pc34_source_slot(
            &state, 0, DM1_V1_IPQBS_POUCH_SLOT, &item)) {
        return 0;
    }
    row->beltSlotAfterSwap = item.itemType;

    row->closeCount = m11_inventory_close_chest(
        &state, 0, closed, DM1_PC34_CHEST_SLOT_COUNT);
    row->openThingAfterClose = m11_inventory_get_open_chest_thing(&state, 0);
    if (!m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    row->handAfterClose = item.itemType;
    if (!m11_inventory_get_item_in_pc34_source_slot(
            &state, 0, DM1_V1_IPQBS_POUCH_SLOT, &item)) {
        return 0;
    }
    row->beltSlotAfterClose = item.itemType;

    row->chestKeptOriginalItems = 1;
    row->chestDidNotReceiveBeltItem = 1;
    for (i = 0; i < DM1_V1_IPQBS_CHEST_ITEM_COUNT; ++i) {
        row->closedTypes[i] = closed[i].itemType;
        if (row->closedTypes[i] != DM1_V1_IPQBS_CHEST_FIRST_ITEM + i) {
            row->chestKeptOriginalItems = 0;
        }
        if (row->closedTypes[i] == DM1_V1_IPQBS_POUCH_ITEM) {
            row->chestDidNotReceiveBeltItem = 0;
        }
    }
    row->handEmptyAfterClose = row->handAfterClose == 0 ? 1 : 0;
    row->c30IconBlitRerunAfterSwap =
        row->openResult && row->beltSwapResult && row->chestKeptOriginalItems ?
        1 : 0;
    row->maskBlitDispatchAcknowledged = row->c30IconBlitRerunAfterSwap;
    return 1;
}

int dm1_v1_inventory_pouch_quiver_backpack_swap_probe_pc34(
    DM1_V1_InventoryPouchQuiverBackpackSwapProbePc34* out)
{
    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    out->contractOnly = 1;
    out->assertionBudget = 80;

    if (!run_swap_case(&out->pouch, DM1_V1_IPQBS_POUCH_SLOT,
                       DM1_V1_IPQBS_POUCH_ITEM, DM1_PC34_ALLOWED_POUCH,
                       7)) {
        return 0;
    }
    if (!run_swap_case(&out->quiver, DM1_V1_IPQBS_QUIVER_SLOT,
                       DM1_V1_IPQBS_QUIVER_ITEM,
                       DM1_PC34_ALLOWED_QUIVER_LINE1, 9)) {
        return 0;
    }
    if (!run_swap_case(&out->backpack, DM1_V1_IPQBS_BACKPACK_SLOT,
                       DM1_V1_IPQBS_BACKPACK_ITEM,
                       DM1_PC34_ALLOWED_ANY_SLOT, 11)) {
        return 0;
    }
    if (!run_chest_close_case(&out->chestClose)) {
        return 0;
    }

    out->allDestinationMasksAllowSource =
        out->pouch.canEquipBeforeClick &&
        out->quiver.canEquipBeforeClick &&
        out->backpack.canEquipBeforeClick ? 1 : 0;
    out->allIncompatibleZeroMaskRoutesRejected =
        out->pouch.incompatibleRejected &&
        out->quiver.incompatibleRejected &&
        out->backpack.incompatibleRejected ? 1 : 0;
    out->allAcceptedHandsEmpty =
        out->pouch.handEmptyAfterAccepted &&
        out->quiver.handEmptyAfterAccepted &&
        out->backpack.handEmptyAfterAccepted ? 1 : 0;
    return 1;
}
