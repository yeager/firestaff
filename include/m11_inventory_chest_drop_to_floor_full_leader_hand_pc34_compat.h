#ifndef FIRESTAFF_M11_INVENTORY_CHEST_DROP_TO_FLOOR_FULL_LEADER_HAND_PC34_COMPAT_H
#define FIRESTAFF_M11_INVENTORY_CHEST_DROP_TO_FLOOR_FULL_LEADER_HAND_PC34_COMPAT_H

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

enum {
    M11_DM1_V1_CHEST_DROP_TO_FLOOR_FULL_LEADER_HAND_SLOT_COUNT =
        DM1_V1_CHEST_DROP_TO_FLOOR_FULL_LEADER_HAND_SLOT_COUNT
};

typedef DM1_V1_InventoryChestDropToFloorFullLeaderHandProbePc34
    M11_InventoryChestDropToFloorFullLeaderHandProbePc34;

#define m11_inventory_chest_drop_to_floor_full_leader_hand_pc34_compat_run \
    DM1_V1_InventoryChestDropToFloorFullLeaderHand_RunPc34

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_M11_INVENTORY_CHEST_DROP_TO_FLOOR_FULL_LEADER_HAND_PC34_COMPAT_H */
