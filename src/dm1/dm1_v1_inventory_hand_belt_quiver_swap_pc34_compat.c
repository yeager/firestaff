#include "dm1/dm1_v1_inventory_hand_belt_quiver_swap_pc34_compat.h"

#include <string.h>

static const DM1_V1_InventoryHandBeltQuiverSwapSpecPc34 s_spec = {
    1,
    DM1_PC34_SLOT_POUCH_1,
    DM1_PC34_SLOT_POUCH_2,
    DM1_PC34_SLOT_QUIVER_LINE1_1,
    DM1_PC34_SLOT_QUIVER_LINE2_1,
    DM1_PC34_SLOT_QUIVER_LINE1_2,
    DM1_PC34_SLOT_QUIVER_LINE2_2,
    DM1_PC34_SLOT_BACKPACK_LINE1_9,
    "CHAMPION.C F0302:684-710 resolves inventory slot boxes by subtracting "
        "C08_SLOT_BOX_INVENTORY_FIRST_SLOT, rejects incompatible leader-hand "
        "objects at 697-699, then runs the swap sequence.",
    "CHAMPION.C F0297/F0298:243-298 owns the leader-hand put/remove side of "
        "the slot-box transaction.",
    "CHAMPION.C F0300/F0301:511-518,606-615 removes and adds the resolved "
        "champion slot while updating slot-owned load.",
    "DATA.C G0038_ai_Graphic562_SlotMasks:1050-1087 gives pouch, quiver, "
        "backpack, and chest slot masks used by F0302.",
    "contract_only=1; synthetic DM1 V1 inventory hand-to-pouch/quiver/"
        "backpack swap gate, no real-asset pixel or original-DOS parity claim."
};

const DM1_V1_InventoryHandBeltQuiverSwapSpecPc34*
dm1_v1_inventory_hand_belt_quiver_swap_spec_pc34(void)
{
    return &s_spec;
}

const char*
dm1_v1_inventory_hand_belt_quiver_swap_evidence_pc34(void)
{
    return
        "CHAMPION.C F0302:684-710 inventory slot-box leader-hand/slot swap\n"
        "CHAMPION.C F0297/F0298:243-298 leader-hand put/remove ownership\n"
        "CHAMPION.C F0300/F0301:511-518,606-615 champion-slot remove/add load\n"
        "DATA.C G0038_ai_Graphic562_SlotMasks:1050-1087 pouch/quiver/backpack masks";
}

static int run_case(DM1_V1_InventoryHandBeltQuiverSwapCasePc34* row,
                    int pc34Slot,
                    int compatibleItemType,
                    int compatibleAllowedSlots,
                    int incompatibleAllowedSlots)
{
    M11_InventoryState accepted;
    M11_InventoryState rejected;
    M11_Item item;

    if (!row) {
        return 0;
    }
    memset(row, 0, sizeof(*row));
    row->pc34Slot = pc34Slot;
    row->expectedStorageSlot =
        m11_inventory_pc34_source_slot_to_storage_slot(pc34Slot);
    row->expectedSlotMask = m11_inventory_pc34_slot_mask(pc34Slot);
    row->compatibleAllowedSlots = compatibleAllowedSlots;
    row->compatibleItemType = compatibleItemType;
    row->incompatibleAllowedSlots = incompatibleAllowedSlots;
    row->slotItemBefore = DM1_V1_IHBQS_EXISTING_SLOT_ITEM;
    row->mouseItemBefore = compatibleItemType;

    m11_inventory_init(&accepted, 1);
    m11_inventory_set_item_in_pc34_source_slot(
        &accepted, 0, pc34Slot, row->slotItemBefore, 3, 0,
        DM1_PC34_ALLOWED_ANY_SLOT);
    m11_inventory_set_mouse_item(&accepted, 0, compatibleItemType, 5, 0,
                                 compatibleAllowedSlots);
    row->loadBefore = m11_inventory_get_load(&accepted, 0);
    row->acceptedClick =
        m11_inventory_click_pc34_source_slot(&accepted, 0, pc34Slot);
    m11_inventory_get_item_in_pc34_source_slot(&accepted, 0, pc34Slot, &item);
    row->slotItemAfterAccepted = item.itemType;
    m11_inventory_get_mouse_item(&accepted, 0, &item);
    row->mouseItemAfterAccepted = item.itemType;
    row->loadAfterAccepted = m11_inventory_get_load(&accepted, 0);

    m11_inventory_init(&rejected, 1);
    m11_inventory_set_item_in_pc34_source_slot(
        &rejected, 0, pc34Slot, row->slotItemBefore, 3, 0,
        DM1_PC34_ALLOWED_ANY_SLOT);
    m11_inventory_set_mouse_item(&rejected, 0, DM1_V1_IHBQS_HEAD_ONLY_ITEM, 7,
                                 0, incompatibleAllowedSlots);
    row->rejectedClick =
        m11_inventory_click_pc34_source_slot(&rejected, 0, pc34Slot);
    m11_inventory_get_item_in_pc34_source_slot(&rejected, 0, pc34Slot, &item);
    row->slotItemAfterRejected = item.itemType;
    m11_inventory_get_mouse_item(&rejected, 0, &item);
    row->mouseItemAfterRejected = item.itemType;
    row->loadAfterRejected = m11_inventory_get_load(&rejected, 0);
    return 1;
}

int dm1_v1_inventory_hand_belt_quiver_swap_probe_pc34(
    DM1_V1_InventoryHandBeltQuiverSwapProbePc34* out)
{
    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    out->contractOnly = 1;
    out->assertionBudget = 120;

    /* ReDMCSB: DATA.C:1056/1061 gives both pouch/belt slots the same
     * MASK0x0100 gate, and CHAMPION.C F0302:697-699 applies it before the
     * CHAMPION.C F0300/F0301:511-518,606-615 swap. */
    if (!run_case(&out->pouch1, DM1_PC34_SLOT_POUCH_1,
                  DM1_V1_IHBQS_POUCH_ITEM, DM1_PC34_ALLOWED_POUCH,
                  DM1_PC34_ALLOWED_HEAD)) {
        return 0;
    }
    if (!run_case(&out->pouch2, DM1_PC34_SLOT_POUCH_2,
                  DM1_V1_IHBQS_POUCH_ITEM, DM1_PC34_ALLOWED_POUCH,
                  DM1_PC34_ALLOWED_HEAD)) {
        return 0;
    }

    /* ReDMCSB: DATA.C:1057-1062 keeps quiver line 1 slot 1 on
     * MASK0x0040, while line 2 slots and line1_2 use MASK0x0080. */
    if (!run_case(&out->quiverLine1, DM1_PC34_SLOT_QUIVER_LINE1_1,
                  DM1_V1_IHBQS_QUIVER_LINE1_ITEM,
                  DM1_PC34_ALLOWED_QUIVER_LINE1,
                  DM1_PC34_ALLOWED_HEAD)) {
        return 0;
    }
    if (!run_case(&out->quiverLine2First, DM1_PC34_SLOT_QUIVER_LINE2_1,
                  DM1_V1_IHBQS_QUIVER_LINE2_ITEM,
                  DM1_PC34_ALLOWED_QUIVER_LINE2,
                  DM1_PC34_ALLOWED_HEAD)) {
        return 0;
    }
    if (!run_case(&out->quiverLine1Second, DM1_PC34_SLOT_QUIVER_LINE1_2,
                  DM1_V1_IHBQS_QUIVER_LINE2_ITEM,
                  DM1_PC34_ALLOWED_QUIVER_LINE2,
                  DM1_PC34_ALLOWED_QUIVER_LINE1)) {
        return 0;
    }
    if (!run_case(&out->quiverLine2Second, DM1_PC34_SLOT_QUIVER_LINE2_2,
                  DM1_V1_IHBQS_QUIVER_LINE2_ITEM,
                  DM1_PC34_ALLOWED_QUIVER_LINE2,
                  DM1_PC34_ALLOWED_HEAD)) {
        return 0;
    }

    /* ReDMCSB: DATA.C:1063-1079 marks backpack slots MASK0xFFFF_ANY_SLOT,
     * so the same head-only object rejected by pouch/quiver slots is accepted
     * here by the CHAMPION.C F0302:697-699 mask test. */
    if (!run_case(&out->backpackLast, DM1_PC34_SLOT_BACKPACK_LINE1_9,
                  DM1_V1_IHBQS_BACKPACK_ITEM,
                  DM1_PC34_ALLOWED_HEAD,
                  DM1_PC34_ALLOWED_HEAD)) {
        return 0;
    }

    out->backpackAcceptsHeadOnly = out->backpackLast.acceptedClick;
    out->pouchRejectsHeadOnly = !out->pouch1.rejectedClick &&
        out->pouch1.slotItemAfterRejected == out->pouch1.slotItemBefore;
    out->quiverRejectsHeadOnly = !out->quiverLine2First.rejectedClick &&
        out->quiverLine2First.slotItemAfterRejected ==
            out->quiverLine2First.slotItemBefore;
    out->quiverLine1RejectsLine2Only =
        !m11_inventory_can_equip(&(M11_Item){ DM1_V1_IHBQS_QUIVER_LINE2_ITEM,
                                             1, 0, 0, 0,
                                             DM1_PC34_ALLOWED_QUIVER_LINE2 },
                                 DM1_PC34_SLOT_QUIVER_LINE1_1);
    out->quiverLine1SecondUsesLine2Mask =
        m11_inventory_pc34_slot_mask(DM1_PC34_SLOT_QUIVER_LINE1_2) ==
        DM1_PC34_ALLOWED_QUIVER_LINE2;
    return 1;
}
