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

enum {
    DM1_PC34_SLOT_READY_HAND = 0,
    DM1_PC34_SLOT_ACTION_HAND = 1,
    DM1_PC34_SLOT_HEAD = 2,
    DM1_PC34_SLOT_TORSO = 3,
    DM1_PC34_SLOT_LEGS = 4,
    DM1_PC34_SLOT_FEET = 5,
    DM1_PC34_SLOT_POUCH_2 = 6,
    DM1_PC34_SLOT_QUIVER_LINE2_1 = 7,
    DM1_PC34_SLOT_QUIVER_LINE1_2 = 8,
    DM1_PC34_SLOT_QUIVER_LINE2_2 = 9,
    DM1_PC34_SLOT_NECK = 10,
    DM1_PC34_SLOT_POUCH_1 = 11,
    DM1_PC34_SLOT_QUIVER_LINE1_1 = 12,
    DM1_PC34_SLOT_BACKPACK_LINE1_1 = 13,
    DM1_PC34_SLOT_BACKPACK_LINE2_2 = 14,
    DM1_PC34_SLOT_BACKPACK_LINE2_3 = 15,
    DM1_PC34_SLOT_BACKPACK_LINE2_4 = 16,
    DM1_PC34_SLOT_BACKPACK_LINE2_5 = 17,
    DM1_PC34_SLOT_BACKPACK_LINE2_6 = 18,
    DM1_PC34_SLOT_BACKPACK_LINE2_7 = 19,
    DM1_PC34_SLOT_BACKPACK_LINE2_8 = 20,
    DM1_PC34_SLOT_BACKPACK_LINE2_9 = 21,
    DM1_PC34_SLOT_BACKPACK_LINE1_2 = 22,
    DM1_PC34_SLOT_BACKPACK_LINE1_3 = 23,
    DM1_PC34_SLOT_BACKPACK_LINE1_4 = 24,
    DM1_PC34_SLOT_BACKPACK_LINE1_5 = 25,
    DM1_PC34_SLOT_BACKPACK_LINE1_6 = 26,
    DM1_PC34_SLOT_BACKPACK_LINE1_7 = 27,
    DM1_PC34_SLOT_BACKPACK_LINE1_8 = 28,
    DM1_PC34_SLOT_BACKPACK_LINE1_9 = 29,
    DM1_PC34_SLOT_CHEST_1 = 30,
    DM1_PC34_SLOT_COUNT = 38
};

enum {
    DM1_PC34_ALLOWED_MOUTH = 0x0001,
    DM1_PC34_ALLOWED_HEAD = 0x0002,
    DM1_PC34_ALLOWED_NECK = 0x0004,
    DM1_PC34_ALLOWED_TORSO = 0x0008,
    DM1_PC34_ALLOWED_LEGS = 0x0010,
    DM1_PC34_ALLOWED_FEET = 0x0020,
    DM1_PC34_ALLOWED_QUIVER_LINE1 = 0x0040,
    DM1_PC34_ALLOWED_QUIVER_LINE2 = 0x0080,
    DM1_PC34_ALLOWED_POUCH = 0x0100,
    DM1_PC34_ALLOWED_HANDS = 0x0200,
    DM1_PC34_ALLOWED_CONTAINER = 0x0400,
    DM1_PC34_ALLOWED_ANY_SLOT = 0xFFFF
};

typedef struct {
    int itemType;
    int weight;
    int charges;
    int cursed;
    int identified;
    int allowedSlots;
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
int m11_inventory_set_item_with_allowed_slots(M11_InventoryState* s, int champ, int slot,
                                              int itemType, int weight, int charges,
                                              int allowedSlots);
int m11_inventory_get_item(const M11_InventoryState* s, int champ, int slot, M11_Item* out);
int m11_inventory_remove_item(M11_InventoryState* s, int champ, int slot);
int m11_inventory_swap_hand(M11_InventoryState* s, int champ);
int m11_inventory_pickup_mouse(M11_InventoryState* s, int champ, int slot);
int m11_inventory_drop_mouse(M11_InventoryState* s, int champ, int slot);
void m11_inventory_recalc_load(M11_InventoryState* s, int champ);
int m11_inventory_get_load(const M11_InventoryState* s, int champ);
int m11_inventory_pc34_slot_mask(int pc34Slot);
int m11_inventory_pc34_source_slot_to_storage_slot(int pc34Slot);
int m11_inventory_set_mouse_item(M11_InventoryState* s, int champ, int itemType,
                                 int weight, int charges, int allowedSlots);
int m11_inventory_get_mouse_item(const M11_InventoryState* s, int champ, M11_Item* out);
int m11_inventory_set_item_in_pc34_source_slot(M11_InventoryState* s, int champ,
                                               int pc34Slot, int itemType, int weight,
                                               int charges, int allowedSlots);
int m11_inventory_get_item_in_pc34_source_slot(const M11_InventoryState* s, int champ,
                                               int pc34Slot, M11_Item* out);
int m11_inventory_click_pc34_source_slot(M11_InventoryState* s, int champ, int pc34Slot);
const char *dm1_inventory_pass601_inventory_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_INVENTORY_PC34_COMPAT_H */
