#ifndef FIRESTAFF_DM1_V1_INVENTORY_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_INVENTORY_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_SLOT_HEAD = 0,
    DM1_SLOT_NECK,
    DM1_SLOT_TORSO,
    DM1_SLOT_LEGS,
    DM1_SLOT_FEET,
    DM1_SLOT_HAND_RIGHT,
    DM1_SLOT_HAND_LEFT,
    DM1_SLOT_POUCH1,
    DM1_SLOT_POUCH2,
    DM1_SLOT_QUIVER1,
    DM1_SLOT_QUIVER2,
    DM1_SLOT_QUIVER3,
    DM1_SLOT_QUIVER4,
    DM1_SLOT_BACKPACK1,
    DM1_SLOT_BACKPACK2,
    DM1_SLOT_BACKPACK3,
    DM1_SLOT_BACKPACK4,
    DM1_SLOT_BACKPACK5,
    DM1_SLOT_BACKPACK6,
    DM1_SLOT_BACKPACK7,
    DM1_SLOT_BACKPACK8,
    DM1_SLOT_COUNT
};

typedef struct {
    int itemType;
    int weight;
    int charges;
    int cursed;
    int identified;
} M11_Item;

typedef struct {
    M11_Item slots[DM1_SLOT_COUNT];
    int handItem;
    M11_Item mouseItem;
    int load;
    int maxLoad;
} M11_ChampionInventory;

#define M11_MAX_CHAMPIONS 4

typedef struct {
    M11_ChampionInventory champions[M11_MAX_CHAMPIONS];
    int championCount;
} M11_InventoryState;

void m11_inventory_init(M11_InventoryState* s, int championCount);
int m11_inventory_set_item(M11_InventoryState* s, int champ, int slot, int itemType, int weight, int charges);
int m11_inventory_get_item(const M11_InventoryState* s, int champ, int slot, M11_Item* out);
int m11_inventory_remove_item(M11_InventoryState* s, int champ, int slot);
int m11_inventory_swap_hand(M11_InventoryState* s, int champ);
int m11_inventory_pickup_mouse(M11_InventoryState* s, int champ, int slot);
int m11_inventory_drop_mouse(M11_InventoryState* s, int champ, int slot);
void m11_inventory_recalc_load(M11_InventoryState* s, int champ);
int m11_inventory_get_load(const M11_InventoryState* s, int champ);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_INVENTORY_PC34_COMPAT_H */