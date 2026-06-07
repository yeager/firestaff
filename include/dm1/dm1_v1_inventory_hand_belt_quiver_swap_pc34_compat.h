#ifndef DM1_V1_INVENTORY_HAND_BELT_QUIVER_SWAP_PC34_COMPAT_H
#define DM1_V1_INVENTORY_HAND_BELT_QUIVER_SWAP_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_IHBQS_POUCH_ITEM = 0x7101,
    DM1_V1_IHBQS_QUIVER_LINE1_ITEM = 0x7201,
    DM1_V1_IHBQS_QUIVER_LINE2_ITEM = 0x7202,
    DM1_V1_IHBQS_BACKPACK_ITEM = 0x7301,
    DM1_V1_IHBQS_EXISTING_SLOT_ITEM = 0x7401,
    DM1_V1_IHBQS_HEAD_ONLY_ITEM = 0x7501
};

typedef struct {
    int contractOnly;
    int pouch1Pc34Slot;
    int pouch2Pc34Slot;
    int quiverLine1Pc34Slot;
    int quiverLine2FirstPc34Slot;
    int quiverLine1SecondPc34Slot;
    int quiverLine2SecondPc34Slot;
    int backpackLastPc34Slot;
    const char* f0302Anchor;
    const char* f0297F0298Anchor;
    const char* f0300F0301Anchor;
    const char* dataSlotMaskAnchor;
    const char* scope;
} DM1_V1_InventoryHandBeltQuiverSwapSpecPc34;

typedef struct {
    int pc34Slot;
    int expectedStorageSlot;
    int expectedSlotMask;
    int compatibleAllowedSlots;
    int compatibleItemType;
    int incompatibleAllowedSlots;
    int slotItemBefore;
    int mouseItemBefore;
    int loadBefore;
    int acceptedClick;
    int slotItemAfterAccepted;
    int mouseItemAfterAccepted;
    int loadAfterAccepted;
    int rejectedClick;
    int slotItemAfterRejected;
    int mouseItemAfterRejected;
    int loadAfterRejected;
} DM1_V1_InventoryHandBeltQuiverSwapCasePc34;

typedef struct {
    int contractOnly;
    int assertionBudget;
    DM1_V1_InventoryHandBeltQuiverSwapCasePc34 pouch1;
    DM1_V1_InventoryHandBeltQuiverSwapCasePc34 pouch2;
    DM1_V1_InventoryHandBeltQuiverSwapCasePc34 quiverLine1;
    DM1_V1_InventoryHandBeltQuiverSwapCasePc34 quiverLine2First;
    DM1_V1_InventoryHandBeltQuiverSwapCasePc34 quiverLine1Second;
    DM1_V1_InventoryHandBeltQuiverSwapCasePc34 quiverLine2Second;
    DM1_V1_InventoryHandBeltQuiverSwapCasePc34 backpackLast;
    int backpackAcceptsHeadOnly;
    int pouchRejectsHeadOnly;
    int quiverRejectsHeadOnly;
    int quiverLine1RejectsLine2Only;
    int quiverLine1SecondUsesLine2Mask;
} DM1_V1_InventoryHandBeltQuiverSwapProbePc34;

const DM1_V1_InventoryHandBeltQuiverSwapSpecPc34*
dm1_v1_inventory_hand_belt_quiver_swap_spec_pc34(void);
const char*
dm1_v1_inventory_hand_belt_quiver_swap_evidence_pc34(void);
int dm1_v1_inventory_hand_belt_quiver_swap_probe_pc34(
    DM1_V1_InventoryHandBeltQuiverSwapProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_INVENTORY_HAND_BELT_QUIVER_SWAP_PC34_COMPAT_H */
