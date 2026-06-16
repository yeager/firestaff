#ifndef FIRESTAFF_DM1_V1_CHEST_C040_CANCEL_REOPEN_PICKUP_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_C040_CANCEL_REOPEN_PICKUP_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_C040_CANCEL_REOPEN_PICKUP_SLOT_COUNT_PC34 8
#define DM1_V1_C040_CANCEL_REOPEN_PICKUP_TRACE_COUNT_PC34 8
#define DM1_V1_C040_CANCEL_REOPEN_PICKUP_NONE_PC34 (-1)

typedef struct {
    const char *chestOpenAnchor;
    const char *chestCloseAnchor;
    const char *panelToggleAnchor;
    const char *reviveCancelAnchor;
    const char *commandQueueAnchor;
    const char *championSlotAnchor;
    const char *defsAnchor;
    const char *contractScope;
    const char *nonOverlap;
} Dm1V1ChestC040CancelReopenPickupEvidencePc34;

typedef struct {
    int panelContent;
    int c040PanelOpen;
    int g0299CandidateOrdinal;
    int inventoryChampionOrdinal;
    int partyChampionCount;
    int leaderIndex;
    int leaderHandThing;
    int openChestThing;
    int containerHeadThing;
    int chestSlots[DM1_V1_C040_CANCEL_REOPEN_PICKUP_SLOT_COUNT_PC34];
    int containerSlots[DM1_V1_C040_CANCEL_REOPEN_PICKUP_SLOT_COUNT_PC34];
    int cancelQueued;
    int cancelMidF0355;
    int reopenQueued;
    int pickupQueued;
    int requestedPickupSlot;
    int f0282CancelCount;
    int f0355ToggleCount;
    int f0334CloseCount;
    int f0333OpenCount;
    int f0380DispatchCount;
    int f0302PickupCount;
    int unsafePickupRejectCount;
    int trace[DM1_V1_C040_CANCEL_REOPEN_PICKUP_TRACE_COUNT_PC34];
    uint32_t chestHash;
    uint32_t stateHash;
} Dm1V1ChestC040CancelReopenPickupStatePc34;

typedef struct {
    int accepted;
    int initialPanelContent;
    int finalPanelContent;
    int c040OpenBefore;
    int c040OpenAfter;
    int g0299Before;
    int g0299After;
    int inventoryOrdinalBefore;
    int inventoryOrdinalAfter;
    int partyCountBefore;
    int partyCountAfter;
    int leaderIndexBefore;
    int leaderIndexAfter;
    int leaderHandBefore;
    int leaderHandAfter;
    int openChestBefore;
    int openChestAfterCancelClose;
    int openChestAfterReopen;
    int openChestAfterPickup;
    int pickedThing;
    int requestedPickupSlot;
    int slotThingBefore;
    int slotThingAfterCancelClose;
    int slotThingAfterReopen;
    int slotThingAfterPickup;
    int containerHeadAfterClose;
    int f0282CancelCount;
    int f0355ToggleCount;
    int f0334CloseCount;
    int f0333OpenCount;
    int f0380DispatchCount;
    int f0302PickupCount;
    int unsafePickupRejectCount;
    int cancelClosedChestBeforeCandidateClear;
    int candidateClearedAfterF0355;
    int pickupRejectedWhileCancelMidF0355;
    int reopenRematerializedG0425;
    int pickupWaitedForReopen;
    int pickupRanAfterCandidateClear;
    int noDuplicateClose;
    int noDuplicateReopen;
    int noRotationPath;
    int noSaveLoadTeleporterPath;
    int sourceAnchorsPresent;
    int trace[DM1_V1_C040_CANCEL_REOPEN_PICKUP_TRACE_COUNT_PC34];
    uint32_t chestHashBefore;
    uint32_t chestHashAfterClose;
    uint32_t chestHashAfterReopen;
    uint32_t chestHashAfterPickup;
    uint32_t deterministicHash;
} Dm1V1ChestC040CancelReopenPickupResultPc34;

void dm1_v1_chest_c040_cancel_reopen_pickup_init_pc34(
    Dm1V1ChestC040CancelReopenPickupStatePc34 *state);

int dm1_v1_chest_c040_cancel_reopen_pickup_run_pc34(
    Dm1V1ChestC040CancelReopenPickupStatePc34 *state,
    Dm1V1ChestC040CancelReopenPickupResultPc34 *result);

const Dm1V1ChestC040CancelReopenPickupEvidencePc34 *
dm1_v1_chest_c040_cancel_reopen_pickup_evidence_pc34(void);

const char *
dm1_v1_chest_c040_cancel_reopen_pickup_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
