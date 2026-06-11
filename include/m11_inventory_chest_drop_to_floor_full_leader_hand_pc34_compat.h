#ifndef FIRESTAFF_M11_INVENTORY_CHEST_DROP_TO_FLOOR_FULL_LEADER_HAND_PC34_COMPAT_H
#define FIRESTAFF_M11_INVENTORY_CHEST_DROP_TO_FLOOR_FULL_LEADER_HAND_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
    M11_DM1_V1_CHEST_DROP_TO_FLOOR_FULL_LEADER_HAND_SLOT_COUNT = 8
};

typedef struct M11_InventoryChestDropToFloorFullLeaderHandProbePc34_ {
    unsigned short leaderHandBefore;
    unsigned short leaderHandBeforeEye;
    unsigned short leaderHandAfterEye;
    unsigned short chestContentsBefore[
        M11_DM1_V1_CHEST_DROP_TO_FLOOR_FULL_LEADER_HAND_SLOT_COUNT];
    int actionType;
    unsigned short expectedLeaderHandAfter;
    int expectedFloorState;
    int expectedChestVisibleSlotCount;
    int floorSlotCountAfterDrop;
    int chestVisibleSlotCountAfterClose;
    int hiddenNinthTailPresentAfterClose;
    const char* anchor;
} M11_InventoryChestDropToFloorFullLeaderHandProbePc34;

int m11_inventory_chest_drop_to_floor_full_leader_hand_pc34_compat_run(
    M11_InventoryChestDropToFloorFullLeaderHandProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_M11_INVENTORY_CHEST_DROP_TO_FLOOR_FULL_LEADER_HAND_PC34_COMPAT_H */
