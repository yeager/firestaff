#include "dm1_v1_inventory_pc34_compat.h"
#include <string.h>

void m11_inventory_init(M11_InventoryState* s, int championCount) {
    if (!s) return;
    memset(s, 0, sizeof(M11_InventoryState));
    s->championCount = championCount;
}

int m11_inventory_set_item(M11_InventoryState* s, int champ, int slot, int itemType, int weight, int charges) {
    if (!s || champ < 0 || champ >= s->championCount || slot < 0 || slot >= DM1_SLOT_COUNT) {
        return 0;
    }
    M11_ChampionInventory* inv = &s->champions[champ];
    inv->slots[slot].itemType = itemType;
    inv->slots[slot].weight = weight;
    inv->slots[slot].charges = charges;
    inv->slots[slot].cursed = 0;
    inv->slots[slot].identified = 0;
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
    memset(&inv->slots[slot], 0, sizeof(M11_Item));
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
    
    // Check if slot has an item (itemType != 0)
    if (inv->slots[slot].itemType == 0) {
        return 0;
    }
    
    // Check if mouseItem is empty (itemType == 0)
    if (inv->mouseItem != 0) {
        return 0;
    }
    
    // Move slot item to mouseItem
    inv->mouseItem = inv->slots[slot].itemType;
    memset(&inv->slots[slot], 0, sizeof(M11_Item));
    
    m11_inventory_recalc_load(s, champ);
    return 1;
}

int m11_inventory_drop_mouse(M11_InventoryState* s, int champ, int slot) {
    if (!s || champ < 0 || champ >= s->championCount || slot < 0 || slot >= DM1_SLOT_COUNT) {
        return 0;
    }
    M11_ChampionInventory* inv = &s->champions[champ];
    
    // Check if mouseItem is not empty
    if (inv->mouseItem == 0) {
        return 0;
    }
    
    // Check if slot is empty
    if (inv->slots[slot].itemType != 0) {
        return 0;
    }
    
    // Move mouseItem to slot
    inv->slots[slot].itemType = inv->mouseItem;
    inv->slots[slot].weight = 0; // Weight needs to be determined by itemType, but spec says move mouseItem->slot
    // Note: The spec implies mouseItem holds just the itemType ID. 
    // In a real system, we'd need to look up weight/charges from a database.
    // Based on the struct M11_Item, mouseItem is just an int.
    // The prompt says "move mouseItem->slot". 
    // Since mouseItem is just an int (itemType), we set itemType.
    // We cannot restore weight/charges from mouseItem alone unless we have a lookup.
    // However, looking at the struct: int mouseItem;
    // And set_item takes weight/charges.
    // This implies mouseItem might just be a placeholder or the logic is simplified.
    // Let's assume for this compat layer that we just set the itemType and zero others, 
    // or that the caller handles the rest. 
    // But wait, pickup_mouse moved the whole struct? No, it moved itemType.
    // Let's re-read: "move slot->mouseItem". 
    // If mouseItem is just an int, we lose data. 
    // Let's assume the prompt implies a simplified model where mouseItem stores the ID.
    
    inv->mouseItem = 0;
    
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