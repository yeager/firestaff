#ifndef FIRESTAFF_DM1_V1_CHEST_PARTIAL_MASK_SWAP_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_PARTIAL_MASK_SWAP_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_CHEST_PARTIAL_MASK_SLOT_COUNT = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_CHEST_PARTIAL_MASK_SLOT_INDEX = 5,
    DM1_PC34_CHEST_PARTIAL_MASK_PC34_SLOT = DM1_PC34_SLOT_CHEST_6,
    DM1_PC34_CHEST_PARTIAL_MASK_CHEST_THING = 0x7A30,
    DM1_PC34_CHEST_PARTIAL_MASK_FIRST_ITEM = 3100,
    DM1_PC34_CHEST_PARTIAL_MASK_LEADER_ITEM = 3900,
    DM1_PC34_CHEST_PARTIAL_MASK_ALLOWED = 0x7F00,
    DM1_PC34_CHEST_PARTIAL_MASK_THING_NONE = 0xFFFF
};

typedef struct {
    const char* contractMarker;
    int c537Pc34Slot;
    int c544Pc34Slot;
    int targetPc34Slot;
    int targetSlotIndex;
    int chestSlotMask;
    int partialAllowedSlots;
    int expectedMaskOverlap;
    int thingNoneSentinel;
} DM1_V1_ChestPartialMaskSwapSpecPc34;

typedef struct {
    int sourceLockedContractOnly;
    int chestThing;
    int openResult;
    int openThing;
    int targetPc34Slot;
    int targetSlotIndex;
    int slotMask;
    int leaderAllowedSlots;
    int maskExactMatch;
    int maskOverlap;
    int maskOverlapNonZero;
    int leaderCanEquip;
    int leaderHandBefore;
    int slotBefore;
    int clickResult;
    int leaderHandAfter;
    int slotAfter;
    int slotAfterAllowedSlots;
    int leaderReleasedToChestSlot;
    int slotOccupantPreservedInLeaderHand;
    int closeCount;
    int closedTypes[DM1_PC34_CHEST_PARTIAL_MASK_SLOT_COUNT];
    int closedAllowedSlots[DM1_PC34_CHEST_PARTIAL_MASK_SLOT_COUNT];
    int closedTargetIsPartialLeaderItem;
    int closedChainValid;
    int openChestClearedAfterClose;
} DM1_V1_ChestPartialMaskSwapProbePc34;

extern const DM1_V1_ChestPartialMaskSwapSpecPc34
    dm1_v1_chest_partial_mask_swap_pc34_spec;

const char* dm1_v1_chest_partial_mask_swap_source_evidence_pc34(void);
const DM1_V1_ChestPartialMaskSwapSpecPc34*
dm1_v1_chest_partial_mask_swap_spec_pc34(void);
int dm1_v1_chest_partial_mask_swap_run_pc34(
    DM1_V1_ChestPartialMaskSwapProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_PARTIAL_MASK_SWAP_PC34_COMPAT_H */
