#ifndef DM1_V1_INVENTORY_HAND_BELT_ROUND_TRIP_PC34_COMPAT_H
#define DM1_V1_INVENTORY_HAND_BELT_ROUND_TRIP_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_IHBRT_READY_ITEM = 0x8101,
    DM1_V1_IHBRT_ACTION_ITEM = 0x8102,
    DM1_V1_IHBRT_BELT_C19_ITEM = 0x8201,
    DM1_V1_IHBRT_BELT_C20_ITEM = 0x8202,
    DM1_V1_IHBRT_LEADER_HAND_ITEM = 0x8301,
    DM1_V1_IHBRT_HEAD_ONLY_ITEM = 0x8401,
    DM1_V1_IHBRT_POUCH_OCCUPANT_ITEM = 0x8501
};

typedef struct {
    int contractOnly;
    int readyHandPc34Slot;
    int actionHandPc34Slot;
    int beltC19Pc34Slot;
    int beltC20Pc34Slot;
    int beltC21Pc34Slot;
    int beltC22Pc34Slot;
    const char* defsAnchor;
    const char* championSwapAnchor;
    const char* championHandAnchor;
    const char* dataMaskAnchor;
    const char* dataStorageAnchor;
    const char* scope;
} DM1_V1_InventoryHandBeltRoundTripSpecPc34;

typedef struct {
    int pc34Slot;
    int storageSlot;
    int slotMask;
    int itemType;
    int weight;
} DM1_V1_InventoryHandBeltRoundTripSlotPc34;

typedef struct {
    int contractOnly;
    int assertionBudget;
    DM1_V1_InventoryHandBeltRoundTripSlotPc34 readySlot;
    DM1_V1_InventoryHandBeltRoundTripSlotPc34 actionSlot;
    DM1_V1_InventoryHandBeltRoundTripSlotPc34 beltC19Slot;
    DM1_V1_InventoryHandBeltRoundTripSlotPc34 beltC20Slot;
    DM1_V1_InventoryHandBeltRoundTripSlotPc34 beltC21Slot;
    DM1_V1_InventoryHandBeltRoundTripSlotPc34 beltC22Slot;
    int initialLoad;
    int leaderHandBeforeReadyClick;
    int readyClickResult;
    int readyAfterReadyClick;
    int leaderAfterReadyClick;
    int loadAfterReadyClick;
    int beltC19ClickResult;
    int beltC19AfterClick;
    int leaderAfterBeltC19Click;
    int loadAfterBeltC19Click;
    int actionClickResult;
    int actionAfterClick;
    int leaderAfterActionClick;
    int loadAfterActionClick;
    int beltC20ClickResult;
    int beltC20AfterClick;
    int leaderAfterBeltC20Click;
    int loadAfterBeltC20Click;
    int beltC21EmptyClickResult;
    int beltC21AfterEmptyClick;
    int leaderAfterBeltC21EmptyClick;
    int loadAfterBeltC21EmptyClick;
    int readyPickupResult;
    int readyAfterPickup;
    int leaderAfterReadyPickup;
    int loadAfterReadyPickup;
    int beltC22ReinsertResult;
    int beltC22AfterReinsert;
    int leaderAfterBeltC22Reinsert;
    int loadAfterBeltC22Reinsert;
    int actionPickupResult;
    int actionAfterPickup;
    int leaderAfterActionPickup;
    int loadAfterActionPickup;
    int pouchRejectResult;
    int pouchAfterReject;
    int leaderAfterPouchReject;
    int loadAfterPouchReject;
} DM1_V1_InventoryHandBeltRoundTripProbePc34;

const DM1_V1_InventoryHandBeltRoundTripSpecPc34*
dm1_v1_inventory_hand_belt_round_trip_spec_pc34(void);
const char*
dm1_v1_inventory_hand_belt_round_trip_evidence_pc34(void);
int dm1_v1_inventory_hand_belt_round_trip_probe_pc34(
    DM1_V1_InventoryHandBeltRoundTripProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_INVENTORY_HAND_BELT_ROUND_TRIP_PC34_COMPAT_H */
