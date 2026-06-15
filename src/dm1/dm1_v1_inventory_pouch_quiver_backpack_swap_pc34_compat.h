#ifndef DM1_V1_INVENTORY_POUCH_QUIVER_BACKPACK_SWAP_PC34_COMPAT_H
#define DM1_V1_INVENTORY_POUCH_QUIVER_BACKPACK_SWAP_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_IPQBS_POUCH_SLOT = DM1_PC34_SLOT_POUCH_1,
    DM1_V1_IPQBS_QUIVER_SLOT = DM1_PC34_SLOT_QUIVER_LINE1_1,
    DM1_V1_IPQBS_BACKPACK_SLOT = DM1_PC34_SLOT_BACKPACK_LINE1_1,
    DM1_V1_IPQBS_POUCH_ITEM = 0x7601,
    DM1_V1_IPQBS_QUIVER_ITEM = 0x7602,
    DM1_V1_IPQBS_BACKPACK_ITEM = 0x7603,
    DM1_V1_IPQBS_INCOMPATIBLE_ITEM = 0x7604,
    DM1_V1_IPQBS_CHEST_THING = 0x7610,
    DM1_V1_IPQBS_CHEST_FIRST_ITEM = 0x7620,
    DM1_V1_IPQBS_CHEST_ITEM_COUNT = 3
};

typedef struct {
    int contractOnly;
    int pouchPc34Slot;
    int pouchStorageSlot;
    int pouchMask;
    int quiverPc34Slot;
    int quiverStorageSlot;
    int quiverMask;
    int backpackPc34Slot;
    int backpackStorageSlot;
    int backpackMask;
    int zeroAllowedSlotsMask;
    const char* chestOpenAnchor;
    const char* chestCloseAnchor;
    const char* allowedSlotsAnchor;
    const char* blitMaskAnchor;
    const char* iconBlitAnchor;
    const char* scope;
} DM1_V1_InventoryPouchQuiverBackpackSwapSpecPc34;

typedef struct {
    int pc34Slot;
    int storageSlot;
    int slotMask;
    int sourceItemType;
    int sourceAllowedSlots;
    int sourceWeight;
    int slotBefore;
    int handBefore;
    int handAllowedBefore;
    int loadBefore;
    int maskOverlap;
    int canEquipBeforeClick;
    int acceptedClick;
    int handAfterAccepted;
    int handAllowedAfterAccepted;
    int slotAfterAccepted;
    int slotAllowedAfterAccepted;
    int slotWeightAfterAccepted;
    int loadAfterAccepted;
    int handEmptyAfterAccepted;
    int slotReceivedSource;
    int incompatibleAllowedSlots;
    int incompatibleMaskOverlap;
    int incompatibleCanEquip;
    int incompatibleClick;
    int incompatibleHandAfter;
    int incompatibleSlotAfter;
    int incompatibleRejected;
} DM1_V1_InventoryPouchQuiverBackpackSwapCasePc34;

typedef struct {
    int openResult;
    int openThingBeforeClose;
    int beltSwapResult;
    int handAfterBeltSwap;
    int beltSlotAfterSwap;
    int closeCount;
    int openThingAfterClose;
    int handAfterClose;
    int beltSlotAfterClose;
    int closedTypes[DM1_V1_IPQBS_CHEST_ITEM_COUNT];
    int chestKeptOriginalItems;
    int chestDidNotReceiveBeltItem;
    int handEmptyAfterClose;
    int c30IconBlitRerunAfterSwap;
    int maskBlitDispatchAcknowledged;
} DM1_V1_InventoryPouchQuiverBackpackChestClosePc34;

typedef struct {
    int contractOnly;
    int assertionBudget;
    DM1_V1_InventoryPouchQuiverBackpackSwapCasePc34 pouch;
    DM1_V1_InventoryPouchQuiverBackpackSwapCasePc34 quiver;
    DM1_V1_InventoryPouchQuiverBackpackSwapCasePc34 backpack;
    DM1_V1_InventoryPouchQuiverBackpackChestClosePc34 chestClose;
    int allDestinationMasksAllowSource;
    int allIncompatibleZeroMaskRoutesRejected;
    int allAcceptedHandsEmpty;
} DM1_V1_InventoryPouchQuiverBackpackSwapProbePc34;

const DM1_V1_InventoryPouchQuiverBackpackSwapSpecPc34*
dm1_v1_inventory_pouch_quiver_backpack_swap_spec_pc34(void);
const char*
dm1_v1_inventory_pouch_quiver_backpack_swap_evidence_pc34(void);
int dm1_v1_inventory_pouch_quiver_backpack_swap_probe_pc34(
    DM1_V1_InventoryPouchQuiverBackpackSwapProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif
