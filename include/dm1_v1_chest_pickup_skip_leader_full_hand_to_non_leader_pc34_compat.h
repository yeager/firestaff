#ifndef FIRESTAFF_DM1_V1_CHEST_PICKUP_SKIP_LEADER_FULL_HAND_TO_NON_LEADER_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_PICKUP_SKIP_LEADER_FULL_HAND_TO_NON_LEADER_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_CHEST_SKIP_LEADER_SLOT_COUNT = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_CHEST_SKIP_LEADER_PARTY_COUNT = 2,
    DM1_PC34_CHEST_SKIP_LEADER_LEADER_INDEX = 0,
    DM1_PC34_CHEST_SKIP_LEADER_TARGET_INDEX = 1,
    DM1_PC34_CHEST_SKIP_LEADER_THING = 0x7D10,
    DM1_PC34_CHEST_SKIP_LEADER_REOPEN_THING = 0x7D11,
    DM1_PC34_CHEST_SKIP_LEADER_FIRST_ITEM = 0x6710,
    DM1_PC34_CHEST_SKIP_LEADER_LEADER_HAND_ITEM = 0x67F0,
    DM1_PC34_CHEST_SKIP_LEADER_PICKED_INDEX = 2,
    DM1_PC34_CHEST_SKIP_LEADER_PICKED_PC34_SLOT = DM1_PC34_SLOT_CHEST_3
};

typedef struct {
    const char* contractMarker;
    int partyCount;
    int leaderIndex;
    int targetChampionIndex;
    int pickedChestIndex;
    int pickedPc34Slot;
    int c537Pc34Slot;
    int c544Pc34Slot;
    int expectedClosedCount;
    int pickupLandsInNonLeaderHand;
    const char* chestOpenAnchor;
    const char* chestCloseAnchor;
    const char* championDirectionAnchor;
    const char* leaderHandAnchor;
    const char* slotDispatchAnchor;
    const char* defsAnchor;
} DM1_V1_ChestPickupSkipLeaderFullHandToNonLeaderSpecPc34;

typedef struct {
    int sourceLockedContractOnly;
    int leaderIndex;
    int targetChampionIndex;
    int pickedChestIndex;
    int pickedPc34Slot;
    int leaderHandSetupResult;
    int nonLeaderHandStartsEmpty;
    int openResult;
    int openThing;
    int pickupClickResult;
    int closeCount;
    int reopenResult;
    int reopenThing;

    int leaderHandBeforeType;
    int leaderHandBeforeWeight;
    int leaderHandBeforeCharges;
    int leaderHandBeforeAllowedSlots;
    int leaderHandAfterType;
    int leaderHandAfterWeight;
    int leaderHandAfterCharges;
    int leaderHandAfterAllowedSlots;

    int nonLeaderHandBeforeType;
    int nonLeaderHandAfterType;
    int nonLeaderHandAfterWeight;
    int nonLeaderHandAfterCharges;
    int nonLeaderHandAfterAllowedSlots;

    int pickedItemType;
    int pickedItemWeight;
    int pickedItemCharges;
    int pickedItemAllowedSlots;
    int leaderHandUnchanged;
    int nonLeaderReceivedPickedItem;
    int pickedSlotClearedInOpenView;
    int openViewPreservesOtherSlots;
    int closeCompactsMinusPicked;
    int reopenCompactsMinusPicked;
    int leaderCollisionCountAfterPickup;
    int pickedItemCountAfterPickup;
    int pickedItemCountAfterReopen;
    int leaderItemCountAfterPickup;
    int leaderItemCountAfterReopen;

    int initialTypes[DM1_PC34_CHEST_SKIP_LEADER_SLOT_COUNT];
    int initialWeights[DM1_PC34_CHEST_SKIP_LEADER_SLOT_COUNT];
    int initialCharges[DM1_PC34_CHEST_SKIP_LEADER_SLOT_COUNT];
    int initialAllowedSlots[DM1_PC34_CHEST_SKIP_LEADER_SLOT_COUNT];
    int openAfterPickupTypes[DM1_PC34_CHEST_SKIP_LEADER_SLOT_COUNT];
    int openAfterPickupWeights[DM1_PC34_CHEST_SKIP_LEADER_SLOT_COUNT];
    int openAfterPickupCharges[DM1_PC34_CHEST_SKIP_LEADER_SLOT_COUNT];
    int openAfterPickupAllowedSlots[DM1_PC34_CHEST_SKIP_LEADER_SLOT_COUNT];
    int closedTypes[DM1_PC34_CHEST_SKIP_LEADER_SLOT_COUNT];
    int closedWeights[DM1_PC34_CHEST_SKIP_LEADER_SLOT_COUNT];
    int closedCharges[DM1_PC34_CHEST_SKIP_LEADER_SLOT_COUNT];
    int closedAllowedSlots[DM1_PC34_CHEST_SKIP_LEADER_SLOT_COUNT];
    int reopenedTypes[DM1_PC34_CHEST_SKIP_LEADER_SLOT_COUNT];
    int reopenedWeights[DM1_PC34_CHEST_SKIP_LEADER_SLOT_COUNT];
    int reopenedCharges[DM1_PC34_CHEST_SKIP_LEADER_SLOT_COUNT];
    int reopenedAllowedSlots[DM1_PC34_CHEST_SKIP_LEADER_SLOT_COUNT];
} DM1_V1_ChestPickupSkipLeaderFullHandToNonLeaderProbePc34;

extern const DM1_V1_ChestPickupSkipLeaderFullHandToNonLeaderSpecPc34
    dm1_v1_chest_pickup_skip_leader_full_hand_to_non_leader_pc34_spec;

const char*
dm1_v1_chest_pickup_skip_leader_full_hand_to_non_leader_source_evidence_pc34(void);
const DM1_V1_ChestPickupSkipLeaderFullHandToNonLeaderSpecPc34*
dm1_v1_chest_pickup_skip_leader_full_hand_to_non_leader_spec_pc34(void);
int dm1_v1_chest_pickup_skip_leader_full_hand_to_non_leader_pc34(
    DM1_V1_ChestPickupSkipLeaderFullHandToNonLeaderProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_PICKUP_SKIP_LEADER_FULL_HAND_TO_NON_LEADER_PC34_COMPAT_H */
