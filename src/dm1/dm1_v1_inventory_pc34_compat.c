#include "dm1_v1_inventory_pc34_compat.h"
#include <string.h>

static const int kPc34SlotMasks[DM1_PC34_SLOT_COUNT] = {
    DM1_PC34_ALLOWED_ANY_SLOT,   /* Ready Hand */
    DM1_PC34_ALLOWED_ANY_SLOT,   /* Action Hand */
    DM1_PC34_ALLOWED_HEAD,       /* Head */
    DM1_PC34_ALLOWED_TORSO,      /* Torso */
    DM1_PC34_ALLOWED_LEGS,       /* Legs */
    DM1_PC34_ALLOWED_FEET,       /* Feet */
    DM1_PC34_ALLOWED_POUCH,      /* Pouch 2 */
    DM1_PC34_ALLOWED_QUIVER_LINE2,
    DM1_PC34_ALLOWED_QUIVER_LINE2,
    DM1_PC34_ALLOWED_QUIVER_LINE2,
    DM1_PC34_ALLOWED_NECK,
    DM1_PC34_ALLOWED_POUCH,
    DM1_PC34_ALLOWED_QUIVER_LINE1,
    DM1_PC34_ALLOWED_ANY_SLOT,
    DM1_PC34_ALLOWED_ANY_SLOT,
    DM1_PC34_ALLOWED_ANY_SLOT,
    DM1_PC34_ALLOWED_ANY_SLOT,
    DM1_PC34_ALLOWED_ANY_SLOT,
    DM1_PC34_ALLOWED_ANY_SLOT,
    DM1_PC34_ALLOWED_ANY_SLOT,
    DM1_PC34_ALLOWED_ANY_SLOT,
    DM1_PC34_ALLOWED_ANY_SLOT,
    DM1_PC34_ALLOWED_ANY_SLOT,
    DM1_PC34_ALLOWED_ANY_SLOT,
    DM1_PC34_ALLOWED_ANY_SLOT,
    DM1_PC34_ALLOWED_ANY_SLOT,
    DM1_PC34_ALLOWED_ANY_SLOT,
    DM1_PC34_ALLOWED_ANY_SLOT,
    DM1_PC34_ALLOWED_ANY_SLOT,
    DM1_PC34_ALLOWED_ANY_SLOT,
    DM1_PC34_ALLOWED_CONTAINER,
    DM1_PC34_ALLOWED_CONTAINER,
    DM1_PC34_ALLOWED_CONTAINER,
    DM1_PC34_ALLOWED_CONTAINER,
    DM1_PC34_ALLOWED_CONTAINER,
    DM1_PC34_ALLOWED_CONTAINER,
    DM1_PC34_ALLOWED_CONTAINER,
    DM1_PC34_ALLOWED_CONTAINER
};

static void m11_inventory_clear_item(M11_Item* item) {
    memset(item, 0, sizeof(*item));
}

void m11_inventory_init(M11_InventoryState* s, int championCount) {
    if (!s) return;
    memset(s, 0, sizeof(M11_InventoryState));
    s->championCount = championCount;
}

int m11_inventory_set_item(M11_InventoryState* s, int champ, int slot, int itemType, int weight, int charges) {
    return m11_inventory_set_item_with_allowed_slots(
        s, champ, slot, itemType, weight, charges, DM1_PC34_ALLOWED_ANY_SLOT);
}

int m11_inventory_set_item_with_allowed_slots(M11_InventoryState* s, int champ, int slot,
                                              int itemType, int weight, int charges,
                                              int allowedSlots) {
    if (!s || champ < 0 || champ >= s->championCount || slot < 0 || slot >= DM1_SLOT_COUNT) {
        return 0;
    }
    M11_ChampionInventory* inv = &s->champions[champ];
    inv->slots[slot].itemType = itemType;
    inv->slots[slot].weight = weight;
    inv->slots[slot].charges = charges;
    inv->slots[slot].cursed = 0;
    inv->slots[slot].identified = 0;
    inv->slots[slot].allowedSlots = allowedSlots;
    m11_inventory_recalc_load(s, champ);
    return 1;
}

int m11_inventory_get_item(const M11_InventoryState* s, int champ, int slot, M11_Item* out) {
    if (!s || !out || champ < 0 || champ >= s->championCount || slot < 0 || slot >= DM1_SLOT_COUNT) {
        return 0;
    }
    const M11_ChampionInventory* inv = &s->champions[champ];
    *out = inv->slots[slot];
    return 1;
}

int m11_inventory_remove_item(M11_InventoryState* s, int champ, int slot) {
    if (!s || champ < 0 || champ >= s->championCount || slot < 0 || slot >= DM1_SLOT_COUNT) {
        return 0;
    }
    M11_ChampionInventory* inv = &s->champions[champ];
    m11_inventory_clear_item(&inv->slots[slot]);
    m11_inventory_recalc_load(s, champ);
    return 1;
}

int m11_inventory_swap_hand(M11_InventoryState* s, int champ) {
    if (!s || champ < 0 || champ >= s->championCount) {
        return 0;
    }
    M11_ChampionInventory* inv = &s->champions[champ];
    M11_Item temp = inv->slots[DM1_SLOT_HAND_RIGHT];
    inv->slots[DM1_SLOT_HAND_RIGHT] = inv->slots[DM1_SLOT_HAND_LEFT];
    inv->slots[DM1_SLOT_HAND_LEFT] = temp;
    return 1;
}

int m11_inventory_pickup_mouse(M11_InventoryState* s, int champ, int slot) {
    if (!s || champ < 0 || champ >= s->championCount || slot < 0 || slot >= DM1_SLOT_COUNT) {
        return 0;
    }
    M11_ChampionInventory* inv = &s->champions[champ];
    
    if (inv->slots[slot].itemType == 0) {
        return 0;
    }

    if (inv->mouseItem.itemType != 0) {
        return 0;
    }

    inv->mouseItem = inv->slots[slot];
    m11_inventory_clear_item(&inv->slots[slot]);

    m11_inventory_recalc_load(s, champ);
    return 1;
}

int m11_inventory_drop_mouse(M11_InventoryState* s, int champ, int slot) {
    if (!s || champ < 0 || champ >= s->championCount || slot < 0 || slot >= DM1_SLOT_COUNT) {
        return 0;
    }
    M11_ChampionInventory* inv = &s->champions[champ];
    
    if (inv->mouseItem.itemType == 0) {
        return 0;
    }

    if (inv->slots[slot].itemType != 0) {
        return 0;
    }

    inv->slots[slot] = inv->mouseItem;
    m11_inventory_clear_item(&inv->mouseItem);

    m11_inventory_recalc_load(s, champ);
    return 1;
}

void m11_inventory_recalc_load(M11_InventoryState* s, int champ) {
    if (!s || champ < 0 || champ >= s->championCount) {
        return;
    }
    M11_ChampionInventory* inv = &s->champions[champ];
    int totalLoad = 0;
    for (int i = 0; i < DM1_SLOT_COUNT; i++) {
        totalLoad += inv->slots[i].weight;
    }
    inv->load = totalLoad;
}

int m11_inventory_get_load(const M11_InventoryState* s, int champ) {
    if (!s || champ < 0 || champ >= s->championCount) {
        return 0;
    }
    return s->champions[champ].load;
}

int m11_inventory_pc34_slot_mask(int pc34Slot) {
    if (pc34Slot < 0 || pc34Slot >= DM1_PC34_SLOT_COUNT) {
        return 0;
    }
    return kPc34SlotMasks[pc34Slot];
}

int m11_inventory_pc34_source_slot_to_storage_slot(int pc34Slot) {
    if (pc34Slot >= DM1_PC34_SLOT_BACKPACK_LINE1_1 &&
        pc34Slot <= DM1_PC34_SLOT_BACKPACK_LINE2_8) {
        return DM1_SLOT_BACKPACK1 + (pc34Slot - DM1_PC34_SLOT_BACKPACK_LINE1_1);
    }

    switch (pc34Slot) {
    case DM1_PC34_SLOT_READY_HAND:
        return DM1_SLOT_HAND_RIGHT;
    case DM1_PC34_SLOT_ACTION_HAND:
        return DM1_SLOT_HAND_LEFT;
    case DM1_PC34_SLOT_HEAD:
        return DM1_SLOT_HEAD;
    case DM1_PC34_SLOT_TORSO:
        return DM1_SLOT_TORSO;
    case DM1_PC34_SLOT_LEGS:
        return DM1_SLOT_LEGS;
    case DM1_PC34_SLOT_FEET:
        return DM1_SLOT_FEET;
    case DM1_PC34_SLOT_POUCH_2:
        return DM1_SLOT_POUCH2;
    case DM1_PC34_SLOT_QUIVER_LINE2_1:
        return DM1_SLOT_QUIVER2;
    case DM1_PC34_SLOT_QUIVER_LINE1_2:
        return DM1_SLOT_QUIVER3;
    case DM1_PC34_SLOT_QUIVER_LINE2_2:
        return DM1_SLOT_QUIVER4;
    case DM1_PC34_SLOT_NECK:
        return DM1_SLOT_NECK;
    case DM1_PC34_SLOT_POUCH_1:
        return DM1_SLOT_POUCH1;
    case DM1_PC34_SLOT_QUIVER_LINE1_1:
        return DM1_SLOT_QUIVER1;
    default:
        return -1;
    }
}

int m11_inventory_set_mouse_item(M11_InventoryState* s, int champ, int itemType,
                                 int weight, int charges, int allowedSlots) {
    if (!s || champ < 0 || champ >= s->championCount) {
        return 0;
    }
    M11_Item* item = &s->champions[champ].mouseItem;
    item->itemType = itemType;
    item->weight = weight;
    item->charges = charges;
    item->cursed = 0;
    item->identified = 0;
    item->allowedSlots = allowedSlots;
    return 1;
}

int m11_inventory_get_mouse_item(const M11_InventoryState* s, int champ, M11_Item* out) {
    if (!s || !out || champ < 0 || champ >= s->championCount) {
        return 0;
    }
    *out = s->champions[champ].mouseItem;
    return 1;
}

int m11_inventory_set_item_in_pc34_source_slot(M11_InventoryState* s, int champ,
                                               int pc34Slot, int itemType, int weight,
                                               int charges, int allowedSlots) {
    const int storageSlot = m11_inventory_pc34_source_slot_to_storage_slot(pc34Slot);
    if (storageSlot < 0) {
        return 0;
    }
    return m11_inventory_set_item_with_allowed_slots(
        s, champ, storageSlot, itemType, weight, charges, allowedSlots);
}

int m11_inventory_get_item_in_pc34_source_slot(const M11_InventoryState* s, int champ,
                                               int pc34Slot, M11_Item* out) {
    const int storageSlot = m11_inventory_pc34_source_slot_to_storage_slot(pc34Slot);
    if (storageSlot < 0) {
        return 0;
    }
    return m11_inventory_get_item(s, champ, storageSlot, out);
}

int m11_inventory_click_pc34_source_slot(M11_InventoryState* s, int champ, int pc34Slot) {
    int slotMask;
    int storageSlot;
    M11_ChampionInventory* inv;
    M11_Item leaderHandObject;
    M11_Item slotObject;

    if (!s || champ < 0 || champ >= s->championCount) {
        return 0;
    }
    storageSlot = m11_inventory_pc34_source_slot_to_storage_slot(pc34Slot);
    if (storageSlot < 0) {
        return 0;
    }
    inv = &s->champions[champ];
    leaderHandObject = inv->mouseItem;
    slotObject = inv->slots[storageSlot];
    if (leaderHandObject.itemType == 0 && slotObject.itemType == 0) {
        return 0;
    }

    slotMask = m11_inventory_pc34_slot_mask(pc34Slot);
    if (leaderHandObject.itemType != 0 &&
        ((leaderHandObject.allowedSlots & slotMask) == 0)) {
        return 0;
    }

    if (slotObject.itemType != 0) {
        inv->mouseItem = slotObject;
    } else {
        m11_inventory_clear_item(&inv->mouseItem);
    }
    if (leaderHandObject.itemType != 0) {
        inv->slots[storageSlot] = leaderHandObject;
    } else {
        m11_inventory_clear_item(&inv->slots[storageSlot]);
    }
    m11_inventory_recalc_load(s, champ);
    return 1;
}

/* ══════════════════════════════════════════════════════════════════════
 * Pass601 — Inventory system source-lock extensions
 *
 * CHAMPION.C:243-268  F0297_CHAMPION_PutObjectInLeaderHand
 * CHAMPION.C:270-298  F0298_CHAMPION_GetObjectRemovedFromLeaderHand
 * CHAMPION.C:301-487  F0299_CHAMPION_ApplyObjectModifiersToStatistics
 * CHAMPION.C:489-560  F0300_CHAMPION_GetObjectRemovedFromSlot
 * CHAMPION.C:587-660  F0301_CHAMPION_AddObjectInSlot
 * CHAMPION.C:662-712  F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox
 *   BUG0_39: Food/Water panel flash when swapping scroll/chest in leader hand
 *   (F0300 sets MASK0x0800, F0297 triggers F0292 which draws panel prematurely)
 *
 * OBJECT.C:121-200    F0032_OBJECT_GetType (thing type extraction)
 * OBJECT.C:25-120     F0031_OBJECT_LoadNames (object name table)
 * ══════════════════════════════════════════════════════════════════════ */

const char *dm1_inventory_pass601_inventory_source_evidence(void)
{
    return
        "CHAMPION.C:243-268 F0297_PutObjectInLeaderHand\n"
        "CHAMPION.C:270-298 F0298_GetObjectRemovedFromLeaderHand\n"
        "CHAMPION.C:301-487 F0299_ApplyObjectModifiersToStatistics\n"
        "CHAMPION.C:489-560 F0300_GetObjectRemovedFromSlot\n"
        "CHAMPION.C:587-660 F0301_AddObjectInSlot\n"
        "CHAMPION.C:694-699 F0302 empty-slot no-op and AllowedSlots/SlotMasks rejection\n"
        "CHAMPION.C:701-710 F0302 leader-hand/slot swap order\n"
        "DATA.C:1049-1087 G0038_ai_Graphic562_SlotMasks\n"
        "DEFS.H:778-805 C00..C25 slot index namespace\n"
        "DEFS.H:1698-1710 object allowed-slot masks\n"
        "CHAMPION.C:662-712 F0302_ProcessCommands28To65_ClickOnSlotBox BUG0_39\n"
        "OBJECT.C:121-200 F0032_OBJECT_GetType\n"
        "OBJECT.C:25-120 F0031_OBJECT_LoadNames\n";
}
