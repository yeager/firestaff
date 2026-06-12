#ifndef FIRESTAFF_DM1_V1_CHEST_PICKUP_AT_MAX_STACK_COUNT_OVERFLOW_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_PICKUP_AT_MAX_STACK_COUNT_OVERFLOW_PC34_COMPAT_H

#include "dm1_v1_inventory_chest_load_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_CHEST_MAX_STACK_OVERFLOW_SLOT_COUNT = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_CHEST_MAX_STACK_OVERFLOW_LEADER = 0,
    DM1_PC34_CHEST_MAX_STACK_OVERFLOW_PARTY_COUNT = 1,
    DM1_PC34_CHEST_MAX_STACK_OVERFLOW_CHEST_THING = 0x7390,
    DM1_PC34_CHEST_MAX_STACK_OVERFLOW_STACK_ITEM = 0x7391,
    DM1_PC34_CHEST_MAX_STACK_OVERFLOW_GUARD_ITEM = 0x7392,
    DM1_PC34_CHEST_MAX_STACK_OVERFLOW_STACK_CAP = 15,
    DM1_PC34_CHEST_MAX_STACK_OVERFLOW_CAP_MINUS_ONE = 14,
    DM1_PC34_CHEST_MAX_STACK_OVERFLOW_SOURCE_SLOT = 0,
    DM1_PC34_CHEST_MAX_STACK_OVERFLOW_FREE_SLOT = 1,
    DM1_PC34_CHEST_MAX_STACK_OVERFLOW_PANEL_CHEST = DM1_PC34_PANEL_CHEST,
    DM1_PC34_CHEST_MAX_STACK_OVERFLOW_C537_ZONE = 537,
    DM1_PC34_CHEST_MAX_STACK_OVERFLOW_C544_ZONE = 544
};

typedef struct {
    int redrawGeneration;
    int panelContent;
    int sourceZone;
    int sourceSlotIndex;
    int handItemType;
    int handStackCount;
    int chestSlot0ItemType;
    int chestSlot0StackCount;
    int chestSlot1ItemType;
    int chestSlot1StackCount;
    int displayedSaturatedCount;
    int displayedNegativeCount;
    int displayedOverflowCount;
    int maskClipApplied;
    int screenUpdateBalanced;
} DM1_V1_ChestPickupAtMaxStackCountOverflowPanelPc34;

typedef struct {
    int result;
    int sourceSlotIndex;
    int sourceZone;
    int leaderHandTypeBefore;
    int leaderHandCountBefore;
    int chestTypeBefore;
    int chestCountBefore;
    int freeSlotTypeBefore;
    int freeSlotCountBefore;
    int candidateMergedCount;
    int stackCap;
    int saturatedCount;
    int rolloverCountIfUnguarded;
    int leaderHandTypeAfter;
    int leaderHandCountAfter;
    int chestTypeAfter;
    int chestCountAfter;
    int freeSlotTypeAfter;
    int freeSlotCountAfter;
    int mergedIntoExistingHand;
    int createdNewChestSlot;
    int overflowRemainderPreserved;
    int overflowPrevented;
    int negativeCountPrevented;
    int crashGuardOk;
    int totalCountBefore;
    int totalCountAfter;
    int totalCountPreserved;
    int panelRedrawRequested;
    int panelRedrawCountBefore;
    int panelRedrawCountAfter;
    int partyDirectionBefore;
    int partyDirectionAfter;
    int rotateTicksBefore;
    int rotateTicksAfter;
    int partyRotateStatePreserved;
    int commandQueueLockedBefore;
    int commandQueueLockedAfter;
    int handOverflowAttempted;
    int handOverflowPrevented;
    DM1_V1_ChestPickupAtMaxStackCountOverflowPanelPc34 panel;
} DM1_V1_ChestPickupAtMaxStackCountOverflowEventPc34;

typedef struct {
    int setupResult;
    int openResult;
    int openChestThing;
    int leaderIndex;
    int partyChampionCount;
    int stackCap;
    int stackCapFromDefsBits;
    int c160IsStackCap;
    int c160CommandLine;
    int c160ZoneLine;
    int chargeCountWeaponLine;
    int chargeCountArmourLine;
    int initialPanelContent;
    int initialPanelRedrawGeneration;
    int initialPartyDirection;
    int initialRotateTicks;
    int initialCommandQueueLocked;
    int sourcePc34Slot;
    int sourceZone;
    int freePc34Slot;
    DM1_V1_ChestPickupAtMaxStackCountOverflowEventPc34 capMinusOnePlusOne;
    DM1_V1_ChestPickupAtMaxStackCountOverflowEventPc34 alreadyCapPlusOne;
    unsigned int deterministicHash;
} DM1_V1_ChestPickupAtMaxStackCountOverflowProbePc34;

const char*
dm1_v1_chest_pickup_at_max_stack_count_overflow_source_evidence_pc34(void);
int dm1_v1_chest_pickup_at_max_stack_count_overflow_run_pc34(
    DM1_V1_ChestPickupAtMaxStackCountOverflowProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_PICKUP_AT_MAX_STACK_COUNT_OVERFLOW_PC34_COMPAT_H */
