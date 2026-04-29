#ifndef REDMCSB_INVENTORY_SLOTBOX_PC34_COMPAT_H
#define REDMCSB_INVENTORY_SLOTBOX_PC34_COMPAT_H

typedef struct InventoryCompatSlotBox {
    unsigned int slotBoxIndex;
    unsigned int slotIndex;
    unsigned int commandId;
    unsigned int zoneIndex;
    const char* slotName;
    const char* allowedMaskName;
    const char* sourceLineEvidence;
} InventoryCompatSlotBox;

typedef struct InventoryCompatSlotRoute {
    unsigned int commandId;
    unsigned int slotBoxIndex;
    unsigned int championIndexFromStatusBox;
    unsigned int slotIndex;
    unsigned int usesInventoryChampion;
    unsigned int usesChestSlots;
    const char* sourceLineEvidence;
} InventoryCompatSlotRoute;

unsigned int INVENTORY_Compat_GetInventorySlotBoxCount(void);
unsigned int INVENTORY_Compat_GetChestSlotBoxCount(void);
unsigned int INVENTORY_Compat_GetTotalSlotBoxCount(void);
int INVENTORY_Compat_GetInventorySlotBox(unsigned int slotBoxIndex, InventoryCompatSlotBox* outSlotBox);
int INVENTORY_Compat_GetChestSlotBox(unsigned int slotBoxIndex, InventoryCompatSlotBox* outSlotBox);
int INVENTORY_Compat_GetSlotRouteFromCommand(unsigned int commandId, InventoryCompatSlotRoute* outRoute);
int INVENTORY_Compat_IsMutableObjectIconIndex(unsigned int iconIndex);
const char* INVENTORY_Compat_GetSlotBoxSourceEvidence(void);
const char* INVENTORY_Compat_GetCarriedObjectIconEvidence(void);

#endif
