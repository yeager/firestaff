#ifndef REDMCSB_INVENTORY_PANEL_SLOT_ROUTES_PC34_COMPAT_H
#define REDMCSB_INVENTORY_PANEL_SLOT_ROUTES_PC34_COMPAT_H

typedef enum InventoryPanelRoutePc34Compat {
    INVENTORY_PANEL_ROUTE_FOOD_WATER_POISONED_PC34_COMPAT = 1,
    INVENTORY_PANEL_ROUTE_CHEST_OBJECT_PC34_COMPAT = 2,
    INVENTORY_PANEL_ROUTE_SCROLL_OBJECT_PC34_COMPAT = 3,
    INVENTORY_PANEL_ROUTE_RESURRECT_REINCARNATE_PC34_COMPAT = 4,
    INVENTORY_PANEL_ROUTE_SKILLS_STATISTICS_PC34_COMPAT = 5,
    INVENTORY_PANEL_ROUTE_LEADER_HAND_OBJECT_PC34_COMPAT = 6
} InventoryPanelRoutePc34Compat;

typedef struct InventorySlotRoutePc34Compat {
    unsigned int inventorySlotIndex;
    unsigned int slotBoxIndex;
    unsigned int commandId;
    unsigned int zoneIndex;
    const char* sourceName;
    const char* sourceEvidence;
} InventorySlotRoutePc34Compat;

typedef struct InventoryPanelInputPc34Compat {
    unsigned int candidateChampionOpen;
    unsigned int pressingMouth;
    unsigned int pressingEye;
    unsigned int leaderEmptyHanded;
    unsigned int actionHandThingType;
} InventoryPanelInputPc34Compat;

unsigned int INVENTORY_Compat_GetInventorySlotRouteCount(void);
int INVENTORY_Compat_GetInventorySlotRoute(unsigned int inventorySlotIndex,
                                           InventorySlotRoutePc34Compat* outRoute);
InventoryPanelRoutePc34Compat INVENTORY_Compat_EvaluatePanelRoute(InventoryPanelInputPc34Compat input);
const char* INVENTORY_Compat_GetPanelSlotRouteEvidence(void);

#endif
