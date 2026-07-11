#ifndef FIRESTAFF_DM1_V1_INVENTORY_CHEST_LOAD_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_INVENTORY_CHEST_LOAD_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_CHEST_EMPTY_THING_WEIGHT = 50
};

const char* DM1_V1_InventoryChestLoad_SourceEvidencePc34Compat(void);
int DM1_V1_InventoryChestLoad_OpenChestVisibleContentsWeightPc34Compat(const DM1_V1_InventoryStatePc34* s,
                                                          int champ);
int DM1_V1_InventoryChestLoad_OpenChestContainerWeightPc34Compat(const DM1_V1_InventoryStatePc34* s,
                                                   int champ);
int DM1_V1_InventoryChestLoad_CloseChestWithWeightSnapshotPc34Compat(DM1_V1_InventoryStatePc34* s,
                                                        int champ,
                                                        DM1_V1_ItemPc34* linkedItemsOut,
                                                        int maxItemsOut,
                                                        int* outContainerWeightBeforeClose);

#define dm1_inventory_chest_load_source_evidence_pc34 \
    DM1_V1_InventoryChestLoad_SourceEvidencePc34Compat

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_INVENTORY_CHEST_LOAD_PC34_COMPAT_H */
