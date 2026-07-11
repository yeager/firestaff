#ifndef FIRESTAFF_DM1_V1_INVENTORY_CHEST_DROP_TO_FLOOR_FULL_LEADER_HAND_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_INVENTORY_CHEST_DROP_TO_FLOOR_FULL_LEADER_HAND_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_CHEST_DROP_TO_FLOOR_FULL_LEADER_HAND_SLOT_COUNT = 8
};

typedef struct DM1_V1_InventoryChestDropToFloorFullLeaderHandProbePc34_ {
    unsigned short leaderHandBefore;
    unsigned short leaderHandBeforeEye;
    unsigned short leaderHandAfterEye;
    unsigned short chestContentsBefore[
        DM1_V1_CHEST_DROP_TO_FLOOR_FULL_LEADER_HAND_SLOT_COUNT];
    int actionType;
    unsigned short expectedLeaderHandAfter;
    int expectedFloorState;
    int expectedChestVisibleSlotCount;
    int floorSlotCountAfterDrop;
    int chestVisibleSlotCountAfterClose;
    int hiddenNinthTailPresentAfterClose;
    const char* anchor;
} DM1_V1_InventoryChestDropToFloorFullLeaderHandProbePc34;

int DM1_V1_InventoryChestDropToFloorFullLeaderHand_RunPc34(
    DM1_V1_InventoryChestDropToFloorFullLeaderHandProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_INVENTORY_CHEST_DROP_TO_FLOOR_FULL_LEADER_HAND_PC34_COMPAT_H */
