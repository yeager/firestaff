#ifndef FIRESTAFF_DM1_V1_INVENTORY_CHEST_INCOMPATIBLE_SWAP_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_INVENTORY_CHEST_INCOMPATIBLE_SWAP_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_TEST_STAFF_OF_CLAWS_OBJECT_INFO = 27,
    DM1_PC34_TEST_CHEST_SWAP_SLOT_COUNT = DM1_PC34_CHEST_SLOT_COUNT
};

typedef struct {
    int c538SlotMask;
    int c538StaffCanEquip;
    int c538ClickResult;
    int c538LeaderHandBefore;
    int c538LeaderHandAfter;
    int c538SlotBefore;
    int c538SlotAfter;
    int c538ClosedCount;
    int c538ClosedTypes[DM1_PC34_TEST_CHEST_SWAP_SLOT_COUNT];

    int c544ReplacementCanEquip;
    int c544ClickResult;
    int c544LeaderHandAfter;
    int c544SlotAfter;
    int c544HiddenTailInput;
    int c544HiddenTailClosed;
    int c544ClosedCount;
    int c544ClosedTypes[DM1_PC34_TEST_CHEST_SWAP_SLOT_COUNT];
} DM1_V1_InventoryChestIncompatibleSwapProbePc34;

const char* dm1_inventory_chest_incompatible_swap_source_evidence_pc34(void);
int m11_inventory_pc34_probe_chest_incompatible_swap(
    DM1_V1_InventoryChestIncompatibleSwapProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_INVENTORY_CHEST_INCOMPATIBLE_SWAP_PC34_COMPAT_H */
