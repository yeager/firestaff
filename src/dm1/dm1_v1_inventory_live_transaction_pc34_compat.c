#include "dm1_v1_inventory_live_transaction_pc34_compat.h"

#include "dm1_v1_dungeon_thing_data_pc34_compat.h"

#include <string.h>

static int inventory_live_is_carryable(uint16_t thing)
{
    switch (THING_GET_TYPE(thing)) {
    case THING_TYPE_WEAPON:
    case THING_TYPE_ARMOUR:
    case THING_TYPE_SCROLL:
    case THING_TYPE_POTION:
    case THING_TYPE_CONTAINER:
    case THING_TYPE_JUNK:
        return 1;
    default:
        return 0;
    }
}

static int inventory_live_record_size(int type)
{
    static const unsigned char sizes[DUNGEON_THING_TYPE_COUNT] = {
        4, 6, 4, 8, 16, 4, 4, 4, 4, 8, 4, 0, 0, 0, 8, 4
    };
    return type >= 0 && type < DUNGEON_THING_TYPE_COUNT ? sizes[type] : 0;
}

static unsigned char *inventory_live_raw_mutable(
    struct DungeonThings_Compat *things, uint16_t thing)
{
    int type = (int)THING_GET_TYPE(thing);
    int index = (int)THING_GET_INDEX(thing);
    int size = inventory_live_record_size(type);
    if (!things || !things->loaded || !size || index < 0 ||
        index >= things->thingCounts[type] || !things->rawThingData[type]) {
        return NULL;
    }
    return things->rawThingData[type] + index * size;
}

static int inventory_live_valid_thing(
    const DM1_V1_InventoryLiveTransactionPc34 *state, uint16_t thing)
{
    if (thing == THING_NONE) return 1;
    return state && state->things && inventory_live_is_carryable(thing) &&
        dm1_v1_dungeon_get_thing_data_pc34(state->things, thing) != NULL &&
        dm1_v1_dungeon_get_object_info_index_pc34(state->things, thing) >= 0 &&
        dm1_v1_dungeon_get_object_allowed_slots_pc34(state->things, thing) != 0u;
}

static int inventory_live_slot_accepts(
    const DM1_V1_InventoryLiveTransactionPc34 *state, int slot, uint16_t thing)
{
    unsigned int allowed;
    int mask;
    if (!inventory_live_valid_thing(state, thing) || slot < 0 ||
        slot >= DM1_PC34_SLOT_COUNT) return 0;
    if (slot >= DM1_PC34_SLOT_CHEST_1) return state->openChestThing != THING_NONE;
    allowed = dm1_v1_dungeon_get_object_allowed_slots_pc34(state->things, thing);
    mask = DM1_V1_Inventory_Pc34SlotMaskCompat(slot);
    return mask != 0 && (allowed & (unsigned int)mask) != 0u;
}

static int inventory_live_validate_panel(
    const DM1_V1_InventoryLiveTransactionPc34 *state)
{
    int slot;
    if (!state || !state->things || !state->things->loaded ||
        (state->openChestThing != THING_NONE &&
         !inventory_live_valid_thing(state, state->openChestThing))) return 0;
    for (slot = 0; slot < DM1_PC34_SLOT_COUNT; ++slot) {
        if (slot >= DM1_PC34_SLOT_CHEST_1 && state->openChestThing == THING_NONE &&
            state->slots[slot] != THING_NONE) return 0;
        if (!inventory_live_valid_thing(state, state->slots[slot])) return 0;
    }
    return inventory_live_valid_thing(state, state->mouseThing);
}

const char *dm1_v1_inventory_live_transaction_source_evidence_pc34(void)
{
    return "ReDMCSB CHAMPION.C F0297-F0303: raw C00-C29 slot ownership and "
           "G0237 AllowedSlots; CHEST.C F0333:30-75 and F0334:112-133: "
           "raw C09/G0425 container projection and rewrite; PANEL.C F0341 "
           "and F0349: scroll and mouth dispatch; DUNGEON.C F0141/F0156/F0159/F0163: "
           "object metadata, raw records, chain walk and rewrite.";
}

int dm1_v1_inventory_live_begin_pc34(
    DM1_V1_InventoryLiveTransactionPc34 *state,
    struct DungeonThings_Compat *things,
    const uint16_t championSlots[DM1_PC34_INVENTORY_SLOT_COUNT])
{
    DM1_V1_InventoryLiveTransactionPc34 next;
    int slot;
    if (!state || !things || !championSlots) return 0;
    memset(&next, 0, sizeof(next));
    next.things = things;
    next.mouseThing = THING_NONE;
    next.openChestThing = THING_NONE;
    for (slot = 0; slot < DM1_PC34_SLOT_COUNT; ++slot) next.slots[slot] = THING_NONE;
    for (slot = 0; slot < DM1_PC34_INVENTORY_SLOT_COUNT; ++slot) {
        next.slots[slot] = championSlots[slot];
        if (!inventory_live_valid_thing(&next, next.slots[slot])) return 0;
    }
    next.generation = state->generation + 1u;
    *state = next;
    return 1;
}

int dm1_v1_inventory_live_click_slot_pc34(
    DM1_V1_InventoryLiveTransactionPc34 *state, int pc34Slot)
{
    DM1_V1_InventoryLiveTransactionPc34 next;
    uint16_t target;
    if (!state || pc34Slot < 0 || pc34Slot >= DM1_PC34_SLOT_COUNT ||
        !inventory_live_validate_panel(state)) return 0;
    if (pc34Slot >= DM1_PC34_SLOT_CHEST_1 && state->openChestThing == THING_NONE) return 0;
    next = *state;
    target = next.slots[pc34Slot];
    if (next.mouseThing == THING_NONE) {
        if (target == THING_NONE) return 0;
        next.mouseThing = target;
        next.slots[pc34Slot] = THING_NONE;
    } else {
        if (!inventory_live_slot_accepts(&next, pc34Slot, next.mouseThing)) return 0;
        next.slots[pc34Slot] = next.mouseThing;
        next.mouseThing = target;
    }
    if (!inventory_live_validate_panel(&next)) return 0;
    next.generation++;
    *state = next;
    return 1;
}

int dm1_v1_inventory_live_open_chest_pc34(
    DM1_V1_InventoryLiveTransactionPc34 *state, uint16_t containerThing,
    DM1_ChestAdmissionReceiptF0333F0334Pc34 *outReceipt)
{
    DM1_V1_InventoryLiveTransactionPc34 next;
    DM1_ChestAdmissionReceiptF0333F0334Pc34 receipt;
    int i;
    if (!state || !outReceipt || !inventory_live_validate_panel(state) ||
        THING_GET_TYPE(containerThing) != THING_TYPE_CONTAINER ||
        !dm1_v1_chest_open_admit_f0333_pc34(state->things, containerThing, &receipt) ||
        !receipt.valid) return 0;
    next = *state;
    next.openChestThing = containerThing;
    for (i = 0; i < DM1_PC34_CHEST_SLOT_COUNT; ++i)
        next.slots[DM1_PC34_SLOT_CHEST_1 + i] = receipt.slots[i];
    if (!inventory_live_validate_panel(&next)) return 0;
    next.generation++;
    *state = next;
    *outReceipt = receipt;
    return 1;
}

int dm1_v1_inventory_live_close_chest_pc34(
    DM1_V1_InventoryLiveTransactionPc34 *state,
    DM1_ChestAdmissionReceiptF0333F0334Pc34 *outReceipt)
{
    DM1_V1_InventoryLiveTransactionPc34 next;
    uint16_t slots[DM1_PC34_CHEST_SLOT_COUNT];
    int i;
    if (!state || !outReceipt || !inventory_live_validate_panel(state) ||
        state->openChestThing == THING_NONE) return 0;
    for (i = 0; i < DM1_PC34_CHEST_SLOT_COUNT; ++i)
        slots[i] = state->slots[DM1_PC34_SLOT_CHEST_1 + i];
    if (!dm1_v1_chest_close_commit_f0334_pc34(state->things,
                                                state->openChestThing, slots,
                                                outReceipt) || !outReceipt->valid) return 0;
    next = *state;
    next.openChestThing = THING_NONE;
    for (i = 0; i < DM1_PC34_CHEST_SLOT_COUNT; ++i)
        next.slots[DM1_PC34_SLOT_CHEST_1 + i] = THING_NONE;
    next.generation++;
    *state = next;
    return 1;
}

int dm1_v1_inventory_live_refill_from_quiver_pc34(
    DM1_V1_InventoryLiveTransactionPc34 *state, int destinationPc34Slot,
    DM1_V1_InventoryLiveQuiverRefillReceiptPc34 *outReceipt)
{
    static const int sourceSlots[] = {
        /* ReDMCSB TIMELINE.C F0259: C12, C07, C08, then C09. */
        DM1_PC34_SLOT_QUIVER_LINE1_1,
        DM1_PC34_SLOT_QUIVER_LINE2_1,
        DM1_PC34_SLOT_QUIVER_LINE1_2,
        DM1_PC34_SLOT_QUIVER_LINE2_2
    };
    DM1_V1_InventoryLiveTransactionPc34 next;
    DM1_V1_InventoryLiveQuiverRefillReceiptPc34 receipt;
    int i;

    if (!outReceipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.destinationSlot = destinationPc34Slot;
    receipt.sourceSlot = -1;
    receipt.thing = THING_NONE;
    receipt.sourceEvidence =
        "ReDMCSB TIMELINE.C F0258/F0259: C12,C07,C08,C09 weapon priority; "
        "DUNGEON.C F0141/F0156: loaded raw C05 Thing ownership.";
    *outReceipt = receipt;
    if (!state || destinationPc34Slot < 0 ||
        destinationPc34Slot >= DM1_PC34_INVENTORY_SLOT_COUNT ||
        !inventory_live_validate_panel(state) ||
        state->mouseThing != THING_NONE ||
        state->slots[destinationPc34Slot] != THING_NONE) {
        return 0;
    }

    next = *state;
    receipt.generationBefore = state->generation;
    for (i = 0; i < (int)(sizeof(sourceSlots) / sizeof(sourceSlots[0])); ++i) {
        const int sourceSlot = sourceSlots[i];
        const uint16_t thing = next.slots[sourceSlot];

        if (THING_GET_TYPE(thing) != THING_TYPE_WEAPON ||
            !inventory_live_valid_thing(&next, thing) ||
            !inventory_live_slot_accepts(&next, destinationPc34Slot, thing)) {
            continue;
        }
        next.slots[destinationPc34Slot] = thing;
        next.slots[sourceSlot] = THING_NONE;
        if (!inventory_live_validate_panel(&next)) return 0;
        next.generation++;
        receipt.valid = 1;
        receipt.moved = 1;
        receipt.sourceSlot = sourceSlot;
        receipt.thing = thing;
        receipt.generationAfter = next.generation;
        *state = next;
        *outReceipt = receipt;
        return 1;
    }

    /* No usable raw weapon is a normal F0259 no-op, not an error. */
    receipt.valid = 1;
    receipt.generationAfter = state->generation;
    *outReceipt = receipt;
    return 1;
}

int dm1_v1_inventory_live_use_action_hand_pc34(
    DM1_V1_InventoryLiveTransactionPc34 *state,
    DM1ConsumableChampionPc34 *champion,
    const uint16_t *woundRandomMasks, int woundRandomMaskCount,
    DM1_V1_InventoryLiveUseReceiptPc34 *outReceipt)
{
    DM1_V1_InventoryLiveUseReceiptPc34 receipt;
    DM1_ChestAdmissionReceiptF0333F0334Pc34 chestReceipt;
    uint16_t thing;
    const unsigned char *raw;
    unsigned char *mutableRaw;
    int type;
    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceEvidence = dm1_v1_inventory_live_transaction_source_evidence_pc34();
    if (!state || !outReceipt || !champion || !inventory_live_validate_panel(state)) return 0;
    thing = state->slots[DM1_PC34_SLOT_ACTION_HAND];
    if (!inventory_live_valid_thing(state, thing)) return 0;
    raw = dm1_v1_dungeon_get_thing_data_pc34(state->things, thing);
    receipt.thing = thing;
    receipt.iconIndex = F7017_GetIconIndex(state->things, thing);
    receipt.objectInfoIndex = F7019_GetObjectInfoIndex(state->things, thing);
    type = (int)THING_GET_TYPE(thing);
    if (!raw || receipt.iconIndex < 0 || receipt.objectInfoIndex < 0) return 0;
    if (type == THING_TYPE_CONTAINER) {
        receipt.kind = DM1_V1_INVENTORY_LIVE_USE_CHEST_PC34;
        if (!dm1_v1_inventory_live_open_chest_pc34(state, thing, &chestReceipt)) return 0;
        receipt.valid = chestReceipt.valid;
        *outReceipt = receipt;
        return 1;
    }
    if (type == THING_TYPE_SCROLL) {
        /* C07 bitfield stores its real text index; presentation obtains the
         * glyph/panel material separately through F0341. */
        receipt.kind = DM1_V1_INVENTORY_LIVE_USE_SCROLL_PC34;
        receipt.scrollTextIndex = (int)((raw[3] >> 2) & 0x3fu);
        receipt.valid = 1;
        *outReceipt = receipt;
        return 1;
    }
    if (type == THING_TYPE_POTION) {
        DM1ConsumableChampionPc34 nextChampion = *champion;
        if (!dm1_inventory_consume_potion_pc34(&nextChampion, raw[3] & 0x7fu,
                                                raw[2], woundRandomMasks,
                                                woundRandomMaskCount,
                                                &receipt.consumable)) return 0;
        mutableRaw = inventory_live_raw_mutable(state->things, thing);
        if (!mutableRaw) return 0;
        /* PANEL.C F0349 preserves power and replaces potion Type with C20. */
        mutableRaw[3] = (unsigned char)((mutableRaw[3] & 0x80u) |
                                        (receipt.consumable.potionTypeAfter & 0x7fu));
        *champion = nextChampion;
        receipt.kind = DM1_V1_INVENTORY_LIVE_USE_CONSUMABLE_PC34;
        receipt.valid = 1;
        state->generation++;
        *outReceipt = receipt;
        return 1;
    }
    if (type == THING_TYPE_JUNK) {
        DM1ConsumableChampionPc34 nextChampion = *champion;
        int consumed = 0;
        if (receipt.iconIndex == 8 || receipt.iconIndex == 9)
            consumed = dm1_inventory_consume_water_junk_pc34(&nextChampion,
                receipt.iconIndex, (raw[3] >> 6) & 0x03u, &receipt.consumable);
        else
            consumed = dm1_inventory_consume_food_junk_pc34(&nextChampion,
                receipt.iconIndex, &receipt.consumable);
        if (!consumed) return 0;
        mutableRaw = inventory_live_raw_mutable(state->things, thing);
        if (!mutableRaw) return 0;
        if (receipt.consumable.chargeCountAfter >= 0)
            mutableRaw[3] = (unsigned char)((mutableRaw[3] & 0x3fu) |
                ((receipt.consumable.chargeCountAfter & 0x03) << 6));
        if (receipt.consumable.removeLeaderHandObject)
            state->slots[DM1_PC34_SLOT_ACTION_HAND] = THING_NONE;
        *champion = nextChampion;
        receipt.kind = DM1_V1_INVENTORY_LIVE_USE_CONSUMABLE_PC34;
        receipt.valid = 1;
        state->generation++;
        *outReceipt = receipt;
        return 1;
    }
    return 0;
}
