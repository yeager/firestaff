#ifndef FIRESTAFF_DM1_V1_INVENTORY_CHEST_LOAD_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_INVENTORY_CHEST_LOAD_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_CHEST_EMPTY_THING_WEIGHT = 50
};

const char* dm1_inventory_chest_load_source_evidence_pc34(void);
int m11_inventory_pc34_open_chest_visible_contents_weight(const M11_InventoryState* s,
                                                          int champ);
int m11_inventory_pc34_open_chest_container_weight(const M11_InventoryState* s,
                                                   int champ);
int m11_inventory_pc34_close_chest_with_weight_snapshot(M11_InventoryState* s,
                                                        int champ,
                                                        M11_Item* linkedItemsOut,
                                                        int maxItemsOut,
                                                        int* outContainerWeightBeforeClose);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_INVENTORY_CHEST_LOAD_PC34_COMPAT_H */
