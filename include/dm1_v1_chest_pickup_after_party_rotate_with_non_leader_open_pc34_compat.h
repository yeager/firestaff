#ifndef FIRESTAFF_DM1_V1_CHEST_PICKUP_AFTER_PARTY_ROTATE_WITH_NON_LEADER_OPEN_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_PICKUP_AFTER_PARTY_ROTATE_WITH_NON_LEADER_OPEN_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_SLOT_COUNT =
        DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_PARTY_COUNT = 2,
    DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_OLD_LEADER = 0,
    DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_NEW_LEADER = 1,
    DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_INVENTORY_CHAMPION = 1,
    DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_CHEST_THING = 0x7310,
    DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_FIRST_ITEM = 0x6810,
    DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_PICKED_INDEX = 3,
    DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_PICKED_PC34_SLOT =
        DM1_PC34_SLOT_CHEST_4,
    DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_OLD_HAND_ITEM = 0x68E0,
    DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_NEW_HAND_ITEM = 0x68F0
};

typedef enum {
    DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_MODE_DIRECTION = 0,
    DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_MODE_LEADER_SWAP = 1
} DM1_V1_ChestPickupAfterPartyRotateModePc34;

typedef struct {
    const char* contractMarker;
    int partyCount;
    int oldLeaderIndex;
    int newLeaderIndex;
    int inventoryChampionIndex;
    int pickedChestIndex;
    int pickedPc34Slot;
    int c537Pc34Slot;
    int c544Pc34Slot;
    int expectedVisibleCount;
    const char* chestSameOpenAnchor;
    const char* chestMaterializeAnchor;
    const char* partyDirectionAnchor;
    const char* leaderSwapAnchor;
    const char* slotDispatchAnchor;
    const char* defsAnchor;
} DM1_V1_ChestPickupAfterPartyRotateSpecPc34;

typedef struct {
    int mode;
    int sourceLockedContractOnly;
    int partyCount;
    int oldLeaderIndex;
    int newLeaderIndex;
    int inventoryChampionIndex;
    int leaderBeforeRotation;
    int leaderAfterRotation;
    int partyDirectionBefore;
    int partyDirectionAfter;
    int oldLeaderDirectionAfter;
    int newLeaderDirectionAfter;
    int oldLeaderCellAfter;
    int newLeaderCellAfter;

    int openResult;
    int openChestThing;
    int sameOpenResult;
    int sameOpenChestThing;
    int inFlightPickupResult;
    int refusedBecauseLeaderChanged;
    int refusedBecauseNewLeaderHandFull;
    int rejectedBeforeSlotSwap;
    int chestLinkStateIntact;
    int reopenOrderPreserved;
    int visibleCountAfterReject;

    int oldLeaderHandBeforeType;
    int oldLeaderHandAfterType;
    int oldLeaderHandDropped;
    int newLeaderHandBeforeType;
    int newLeaderHandAfterType;
    int newLeaderHandPreserved;
    int pickedItemType;
    int pickedItemCountAfterReject;
    int newLeaderHandItemCountAfterReject;

    int initialTypes[DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_SLOT_COUNT];
    int initialWeights[DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_SLOT_COUNT];
    int afterRejectTypes[DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_SLOT_COUNT];
    int afterRejectWeights[DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_SLOT_COUNT];
    int reopenedTypes[DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_SLOT_COUNT];
    int reopenedWeights[DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_SLOT_COUNT];
} DM1_V1_ChestPickupAfterPartyRotateCasePc34;

typedef struct {
    int caseCount;
    uint32_t deterministicHash;
    DM1_V1_ChestPickupAfterPartyRotateCasePc34 directionCase;
    DM1_V1_ChestPickupAfterPartyRotateCasePc34 leaderSwapCase;
} DM1_V1_ChestPickupAfterPartyRotateProbePc34;

extern const DM1_V1_ChestPickupAfterPartyRotateSpecPc34
    dm1_v1_chest_pickup_after_party_rotate_with_non_leader_open_pc34_spec;

const char*
dm1_v1_chest_pickup_after_party_rotate_with_non_leader_open_source_evidence_pc34(void);
const DM1_V1_ChestPickupAfterPartyRotateSpecPc34*
dm1_v1_chest_pickup_after_party_rotate_with_non_leader_open_spec_pc34(void);
int dm1_v1_chest_pickup_after_party_rotate_with_non_leader_open_run_pc34(
    DM1_V1_ChestPickupAfterPartyRotateProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_PICKUP_AFTER_PARTY_ROTATE_WITH_NON_LEADER_OPEN_PC34_COMPAT_H */
