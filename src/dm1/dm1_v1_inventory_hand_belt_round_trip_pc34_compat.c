#include "dm1_v1_inventory_hand_belt_round_trip_pc34_compat.h"

#include <string.h>

static const DM1_V1_InventoryHandBeltRoundTripSpecPc34 s_spec = {
    1,
    DM1_PC34_SLOT_READY_HAND,
    DM1_PC34_SLOT_ACTION_HAND,
    DM1_PC34_SLOT_BACKPACK_LINE2_7,
    DM1_PC34_SLOT_BACKPACK_LINE2_8,
    DM1_PC34_SLOT_BACKPACK_LINE2_9,
    DM1_PC34_SLOT_BACKPACK_LINE1_2,
    "DEFS.H:1874-1878 defines C08/C09 inventory hand slot boxes and "
        "M070_HAND_SLOT_INDEX for status hand boxes.",
    "CHAMPION.C F0302:684-710 resolves inventory slot boxes, checks "
        "AllowedSlots against G0038 masks, removes the occupied slot, then "
        "adds the previous leader-hand thing.",
    "CHAMPION.C F0297/F0298:243-298 owns leader-hand put/remove load state; "
        "F0300/F0301:511-615 owns slot remove/add load state.",
    "DATA.C G0038_ai_Graphic562_SlotMasks:1049-1079 marks ready/action hands "
        "and backpack C19-C22 storage slots as MASK0xFFFF_ANY_SLOT.",
    "DATA.C inventory source order:1128-1142 maps backpack C19-C22 into "
        "ordinary champion storage after ready/action hand slots.",
    "contract_only=1; synthetic DM1 V1 hand-to-belt non-empty round-trip "
        "runtime gate, no real-asset pixel or original-DOS parity claim."
};

const DM1_V1_InventoryHandBeltRoundTripSpecPc34*
dm1_v1_inventory_hand_belt_round_trip_spec_pc34(void)
{
    return &s_spec;
}

const char*
dm1_v1_inventory_hand_belt_round_trip_evidence_pc34(void)
{
    return
        "DEFS.H:1874-1878 C08/C09 and M070 hand slot box routing\n"
        "CHAMPION.C F0302:684-710 leader-hand/slot swap sequence\n"
        "CHAMPION.C F0297/F0298:243-298 and F0300/F0301:511-615 load ownership\n"
        "DATA.C G0038_ai_Graphic562_SlotMasks:1049-1079 hand/backpack any-slot masks\n"
        "DATA.C:1128-1142 C19-C22 backpack storage order";
}

static void fill_slot(DM1_V1_InventoryHandBeltRoundTripSlotPc34* out,
                      int pc34Slot,
                      int itemType,
                      int weight)
{
    out->pc34Slot = pc34Slot;
    out->storageSlot =
        m11_inventory_pc34_source_slot_to_storage_slot(pc34Slot);
    out->slotMask = m11_inventory_pc34_slot_mask(pc34Slot);
    out->itemType = itemType;
    out->weight = weight;
}

static int set_slot(M11_InventoryState* state,
                    int pc34Slot,
                    int itemType,
                    int weight)
{
    return m11_inventory_set_item_in_pc34_source_slot(
        state, 0, pc34Slot, itemType, weight, 0, DM1_PC34_ALLOWED_ANY_SLOT);
}

static int get_type(const M11_InventoryState* state, int pc34Slot)
{
    M11_Item item;

    if (!m11_inventory_get_item_in_pc34_source_slot(
            state, 0, pc34Slot, &item)) {
        return -1;
    }
    return item.itemType;
}

static int get_mouse_type(const M11_InventoryState* state)
{
    M11_Item item;

    if (!m11_inventory_get_mouse_item(state, 0, &item)) {
        return -1;
    }
    return item.itemType;
}

int dm1_v1_inventory_hand_belt_round_trip_probe_pc34(
    DM1_V1_InventoryHandBeltRoundTripProbePc34* out)
{
    M11_InventoryState state;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    out->contractOnly = 1;
    out->assertionBudget = 80;

    fill_slot(&out->readySlot, DM1_PC34_SLOT_READY_HAND,
              DM1_V1_IHBRT_READY_ITEM, 11);
    fill_slot(&out->actionSlot, DM1_PC34_SLOT_ACTION_HAND,
              DM1_V1_IHBRT_ACTION_ITEM, 13);
    fill_slot(&out->beltC19Slot, DM1_PC34_SLOT_BACKPACK_LINE2_7,
              DM1_V1_IHBRT_BELT_C19_ITEM, 3);
    fill_slot(&out->beltC20Slot, DM1_PC34_SLOT_BACKPACK_LINE2_8,
              DM1_V1_IHBRT_BELT_C20_ITEM, 5);
    fill_slot(&out->beltC21Slot, DM1_PC34_SLOT_BACKPACK_LINE2_9, 0, 0);
    fill_slot(&out->beltC22Slot, DM1_PC34_SLOT_BACKPACK_LINE1_2, 0, 0);

    m11_inventory_init(&state, 1);
    if (!set_slot(&state, out->readySlot.pc34Slot, out->readySlot.itemType,
                  out->readySlot.weight) ||
        !set_slot(&state, out->actionSlot.pc34Slot, out->actionSlot.itemType,
                  out->actionSlot.weight) ||
        !set_slot(&state, out->beltC19Slot.pc34Slot,
                  out->beltC19Slot.itemType, out->beltC19Slot.weight) ||
        !set_slot(&state, out->beltC20Slot.pc34Slot,
                  out->beltC20Slot.itemType, out->beltC20Slot.weight)) {
        return 0;
    }
    out->initialLoad = m11_inventory_get_load(&state, 0);

    /* ReDMCSB CHAMPION.C F0302 lines 688-710 swaps the leader hand with the
     * clicked source slot; DATA.C lines 1050-1051 make ready/action hands
     * any-slot destinations, so a non-empty ready hand accepts the leader
     * object and returns the old ready item to the leader hand. */
    if (!m11_inventory_set_mouse_item(&state, 0,
                                      DM1_V1_IHBRT_LEADER_HAND_ITEM, 17, 0,
                                      DM1_PC34_ALLOWED_HANDS)) {
        return 0;
    }
    out->leaderHandBeforeReadyClick = get_mouse_type(&state);
    out->readyClickResult = m11_inventory_click_pc34_source_slot(
        &state, 0, out->readySlot.pc34Slot);
    out->readyAfterReadyClick = get_type(&state, out->readySlot.pc34Slot);
    out->leaderAfterReadyClick = get_mouse_type(&state);
    out->loadAfterReadyClick = m11_inventory_get_load(&state, 0);

    /* ReDMCSB DATA.C lines 1069-1072 and 1135-1138 keep C19-C22 backpack
     * source slots as ordinary inventory slots.  F0302 lines 704-709 must
     * move a non-empty belt occupant to the leader hand while inserting the
     * previous hand object into that belt slot. */
    out->beltC19ClickResult = m11_inventory_click_pc34_source_slot(
        &state, 0, out->beltC19Slot.pc34Slot);
    out->beltC19AfterClick = get_type(&state, out->beltC19Slot.pc34Slot);
    out->leaderAfterBeltC19Click = get_mouse_type(&state);
    out->loadAfterBeltC19Click = m11_inventory_get_load(&state, 0);

    out->actionClickResult = m11_inventory_click_pc34_source_slot(
        &state, 0, out->actionSlot.pc34Slot);
    out->actionAfterClick = get_type(&state, out->actionSlot.pc34Slot);
    out->leaderAfterActionClick = get_mouse_type(&state);
    out->loadAfterActionClick = m11_inventory_get_load(&state, 0);

    out->beltC20ClickResult = m11_inventory_click_pc34_source_slot(
        &state, 0, out->beltC20Slot.pc34Slot);
    out->beltC20AfterClick = get_type(&state, out->beltC20Slot.pc34Slot);
    out->leaderAfterBeltC20Click = get_mouse_type(&state);
    out->loadAfterBeltC20Click = m11_inventory_get_load(&state, 0);

    out->beltC21EmptyClickResult = m11_inventory_click_pc34_source_slot(
        &state, 0, out->beltC21Slot.pc34Slot);
    out->beltC21AfterEmptyClick = get_type(&state, out->beltC21Slot.pc34Slot);
    out->leaderAfterBeltC21EmptyClick = get_mouse_type(&state);
    out->loadAfterBeltC21EmptyClick = m11_inventory_get_load(&state, 0);

    out->readyPickupResult = m11_inventory_click_pc34_source_slot(
        &state, 0, out->readySlot.pc34Slot);
    out->readyAfterPickup = get_type(&state, out->readySlot.pc34Slot);
    out->leaderAfterReadyPickup = get_mouse_type(&state);
    out->loadAfterReadyPickup = m11_inventory_get_load(&state, 0);

    out->beltC22ReinsertResult = m11_inventory_click_pc34_source_slot(
        &state, 0, out->beltC22Slot.pc34Slot);
    out->beltC22AfterReinsert = get_type(&state, out->beltC22Slot.pc34Slot);
    out->leaderAfterBeltC22Reinsert = get_mouse_type(&state);
    out->loadAfterBeltC22Reinsert = m11_inventory_get_load(&state, 0);

    out->actionPickupResult = m11_inventory_click_pc34_source_slot(
        &state, 0, out->actionSlot.pc34Slot);
    out->actionAfterPickup = get_type(&state, out->actionSlot.pc34Slot);
    out->leaderAfterActionPickup = get_mouse_type(&state);
    out->loadAfterActionPickup = m11_inventory_get_load(&state, 0);

    if (!set_slot(&state, DM1_PC34_SLOT_POUCH_1,
                  DM1_V1_IHBRT_POUCH_OCCUPANT_ITEM, 7) ||
        !m11_inventory_set_mouse_item(&state, 0,
                                      DM1_V1_IHBRT_HEAD_ONLY_ITEM, 23, 0,
                                      DM1_PC34_ALLOWED_HEAD)) {
        return 0;
    }
    out->pouchRejectResult = m11_inventory_click_pc34_source_slot(
        &state, 0, DM1_PC34_SLOT_POUCH_1);
    out->pouchAfterReject = get_type(&state, DM1_PC34_SLOT_POUCH_1);
    out->leaderAfterPouchReject = get_mouse_type(&state);
    out->loadAfterPouchReject = m11_inventory_get_load(&state, 0);
    return 1;
}
