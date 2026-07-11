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
    DM1_SLOT_BACKPACK9,
    DM1_SLOT_BACKPACK10,
    DM1_SLOT_BACKPACK11,
    DM1_SLOT_BACKPACK12,
    DM1_SLOT_BACKPACK13,
    DM1_SLOT_BACKPACK14,
    DM1_SLOT_BACKPACK15,
    DM1_SLOT_BACKPACK16,
    DM1_SLOT_BACKPACK17,
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
    DM1_PC34_SLOT_CHEST_2 = 31,
    DM1_PC34_SLOT_CHEST_3 = 32,
    DM1_PC34_SLOT_CHEST_4 = 33,
    DM1_PC34_SLOT_CHEST_5 = 34,
    DM1_PC34_SLOT_CHEST_6 = 35,
    DM1_PC34_SLOT_CHEST_7 = 36,
    DM1_PC34_SLOT_CHEST_8 = 37,
    DM1_PC34_SLOT_COUNT = 38,
    DM1_PC34_INVENTORY_SLOT_COUNT = 30,
    DM1_PC34_BACKPACK_SLOT_COUNT = 17,
    DM1_PC34_CHEST_SLOT_COUNT = 8
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

enum {
    DM1_PC34_ICON_JUNK_RABBITS_FOOT = 137,
    DM1_PC34_RABBITS_FOOT_LUCK_BONUS = 10
};

enum {
    DM1_PC34_PANEL_INVENTORY = 0,
    DM1_PC34_PANEL_FOOD_WATER_POISONED = 1,
    DM1_PC34_PANEL_SCROLL = 5,
    DM1_PC34_PANEL_CHEST = 6,
    DM1_PC34_PANEL_RESURRECT_REINCARNATE = 7
};

typedef struct {
    int itemType;
    int weight;
    int charges;
    int cursed;
    int identified;
    int allowedSlots;
} DM1_V1_ItemPc34;

typedef struct {
    DM1_V1_ItemPc34 slots[DM1_SLOT_COUNT];
    int handItem;
    DM1_V1_ItemPc34 mouseItem;
    int load;
    int maxLoad;
    DM1_V1_ItemPc34 chestSlots[DM1_PC34_CHEST_SLOT_COUNT];
    int openChestThing;
} DM1_V1_ChampionInventoryPc34;

#define DM1_V1_MAX_CHAMPIONS_PC34 4

typedef struct {
    DM1_V1_ChampionInventoryPc34 champions[DM1_V1_MAX_CHAMPIONS_PC34];
    int championCount;
    int panelContent;
} DM1_V1_InventoryStatePc34;

void DM1_V1_Inventory_InitPc34Compat(DM1_V1_InventoryStatePc34* s, int championCount);
int DM1_V1_Inventory_SetItemPc34Compat(DM1_V1_InventoryStatePc34* s, int champ, int slot, int itemType, int weight, int charges);
int DM1_V1_Inventory_SetItemWithAllowedSlotsPc34Compat(DM1_V1_InventoryStatePc34* s, int champ, int slot,
                                              int itemType, int weight, int charges,
                                              int allowedSlots);
int DM1_V1_Inventory_GetItemPc34Compat(const DM1_V1_InventoryStatePc34* s, int champ, int slot, DM1_V1_ItemPc34* out);
int DM1_V1_Inventory_RemoveItemPc34Compat(DM1_V1_InventoryStatePc34* s, int champ, int slot);
int DM1_V1_Inventory_SwapHandPc34Compat(DM1_V1_InventoryStatePc34* s, int champ);
int DM1_V1_Inventory_PickupMousePc34Compat(DM1_V1_InventoryStatePc34* s, int champ, int slot);
int DM1_V1_Inventory_DropMousePc34Compat(DM1_V1_InventoryStatePc34* s, int champ, int slot);
void DM1_V1_Inventory_RecalcLoadPc34Compat(DM1_V1_InventoryStatePc34* s, int champ);
int DM1_V1_Inventory_GetLoadPc34Compat(const DM1_V1_InventoryStatePc34* s, int champ);
int DM1_V1_Inventory_Pc34SlotMaskCompat(int pc34Slot);
int DM1_V1_Inventory_Pc34SourceSlotToStorageSlotCompat(int pc34Slot);
int DM1_V1_Inventory_SetMouseItemPc34Compat(DM1_V1_InventoryStatePc34* s, int champ, int itemType,
                                 int weight, int charges, int allowedSlots);
int DM1_V1_Inventory_GetMouseItemPc34Compat(const DM1_V1_InventoryStatePc34* s, int champ, DM1_V1_ItemPc34* out);
int DM1_V1_Inventory_SetItemInPc34SourceSlotCompat(DM1_V1_InventoryStatePc34* s, int champ,
                                               int pc34Slot, int itemType, int weight,
                                               int charges, int allowedSlots);
int DM1_V1_Inventory_GetItemInPc34SourceSlotCompat(const DM1_V1_InventoryStatePc34* s, int champ,
                                               int pc34Slot, DM1_V1_ItemPc34* out);
int DM1_V1_Inventory_ClickPc34SourceSlotCompat(DM1_V1_InventoryStatePc34* s, int champ, int pc34Slot);
const char *dm1_inventory_pass601_inventory_source_evidence(void);
const char *dm1_inventory_chest_stale_click_source_evidence_pc34(void);
int DM1_V1_Inventory_ClickOpenChestSlotForThingPc34Compat(DM1_V1_InventoryStatePc34* s, int champ,
                                                  int expectedOpenChestThing,
                                                  int chestSlotIndex);
int DM1_V1_Inventory_ResolveStatusHandSlotBoxPc34Compat(int slotBoxIndex,
                                               int partyChampionCount,
                                               int inventoryChampionOrdinal,
                                               int candidateChampionOrdinal,
                                               const int* championCurrentHealth,
                                               int* outChampionIndex,
                                               int* outPc34SourceSlot);
int DM1_V1_Inventory_Pc34IsBackpackSourceSlotCompat(int pc34Slot);
int DM1_V1_Inventory_Pc34IsChestSourceSlotCompat(int pc34Slot);
int DM1_V1_Inventory_OpenChestPc34Compat(DM1_V1_InventoryStatePc34* s, int champ, int openChestThing,
                             const DM1_V1_ItemPc34* linkedItems, int linkedItemCount);
int DM1_V1_Inventory_OpenChestReplacingCurrentPc34Compat(DM1_V1_InventoryStatePc34* s, int champ,
                                               int openChestThing,
                                               const DM1_V1_ItemPc34* linkedItems,
                                               int linkedItemCount,
                                               DM1_V1_ItemPc34* previousItemsOut,
                                               int maxPreviousItemsOut);
int DM1_V1_Inventory_GetPanelContentPc34Compat(const DM1_V1_InventoryStatePc34* s);
int DM1_V1_Inventory_SetPanelContentPc34Compat(DM1_V1_InventoryStatePc34* s,
                                         int panelContent);
/* ReDMCSB: PANEL.C F0347 redraws FOOD/WATER/POISONED when no container
 * remains in the action hand; keeps CHEST when the action hand is container. */
int DM1_V1_Inventory_ApplyPanelRouteAfterClosePc34Compat(DM1_V1_InventoryStatePc34* s,
                                                    int champ);
int DM1_V1_Inventory_CloseChestPc34Compat(DM1_V1_InventoryStatePc34* s, int champ,
                              DM1_V1_ItemPc34* linkedItemsOut, int maxItemsOut);
int DM1_V1_Inventory_GetOpenChestThingPc34Compat(const DM1_V1_InventoryStatePc34* s, int champ);
int DM1_V1_Inventory_SetItemInChestSlotPc34Compat(DM1_V1_InventoryStatePc34* s, int champ, int chestSlotIndex,
                                         int itemType, int weight, int charges, int allowedSlots);
int DM1_V1_Inventory_GetItemInChestSlotPc34Compat(const DM1_V1_InventoryStatePc34* s, int champ,
                                         int chestSlotIndex, DM1_V1_ItemPc34* out);
int DM1_V1_Inventory_CanEquipPc34Compat(const DM1_V1_ItemPc34* item, int pc34Slot);
int DM1_V1_Inventory_Pc34AppliesRabbitsFootLuckModifierCompat(const DM1_V1_ItemPc34* item,
                                                          int pc34Slot);
int DM1_V1_Inventory_Pc34GetRabbitsFootLuckBonusCompat(const DM1_V1_InventoryStatePc34* s,
                                                   int champ);
int DM1_V1_Inventory_EquipPc34Compat(DM1_V1_InventoryStatePc34* s, int champ, int pc34Slot, const DM1_V1_ItemPc34* item);
int DM1_V1_Inventory_UnequipPc34Compat(DM1_V1_InventoryStatePc34* s, int champ, int pc34Slot);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_INVENTORY_PC34_COMPAT_H */
