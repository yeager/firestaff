#ifndef DM1_V1_INVENTORY_CHAMPION_SWITCH_HAND_CARRY_PC34_COMPAT_H
#define DM1_V1_INVENTORY_CHAMPION_SWITCH_HAND_CARRY_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_ICSWHC_MAX_CHAMPIONS_PC34 = 4,
    DM1_V1_ICSWHC_NO_INVENTORY_ORDINAL_PC34 = 0,
    DM1_V1_ICSWHC_CLOSE_INVENTORY_PC34 = 4,
    DM1_V1_ICSWHC_SPECIAL_INVENTORY_PC34 = 5,
    DM1_V1_ICSWHC_SLOT_DRAW_COUNT_PC34 = 30,
    DM1_V1_ICSWHC_THING_NONE_PC34 = 0xFFFF,
    DM1_V1_ICSWHC_LEADER_HAND_THING_PC34 = 0x4101,
    DM1_V1_ICSWHC_OPEN_CHEST_THING_PC34 = 0x5301,
    DM1_V1_ICSWHC_CHEST_SLOT_COUNT_PC34 = 8,
    DM1_V1_ICSWHC_CHEST_ITEM_FIRST_PC34 = 0x6101,
    DM1_V1_ICSWHC_CHEST_ITEM_THIRD_PC34 = 0x6103,
    DM1_V1_ICSWHC_CHEST_ITEM_EIGHTH_PC34 = 0x6108
};

typedef struct {
    const char* f0354EntryGuard;
    const char* f0354OldInventoryClose;
    const char* f0354CloseBranch;
    const char* f0354NewInventoryDraw;
    const char* f0354MouseInputRefresh;
    const char* f0334ChestClose;
    const char* f0352F0353LeaderHandDraw;
    const char* nonOverlapScope;
} DM1_V1_InventoryChampionSwitchHandCarryEvidencePc34;

typedef struct {
    int championCount;
    int championHealth[DM1_V1_ICSWHC_MAX_CHAMPIONS_PC34];
    int g0423InventoryChampionOrdinal;
    int g4055LeaderHandThing;
    int leaderHandWeight;
    int g0426OpenChestThing;
    int g0425ChestSlots[DM1_V1_ICSWHC_CHEST_SLOT_COUNT_PC34];
    int f0334ClosedTypes[DM1_V1_ICSWHC_CHEST_SLOT_COUNT_PC34];
    int f0334ClosedCount;
    int pressingMouthOrEye;
    int stopWaitingForPlayerInput;
    int oldStatusDrawCount;
    int newStatusDrawCount;
    int slotDrawCount;
    int f0334CloseChestCount;
    int movementArrowsDrawCount;
    int floorCeilingDrawCount;
    int mousePointerBitmapUpdated;
    int refreshMousePointerInMainLoop;
    int secondaryInputInventory;
    int secondaryInputMovement;
    int discardInputCount;
} DM1_V1_InventoryChampionSwitchHandCarryStatePc34;

typedef struct {
    const DM1_V1_InventoryChampionSwitchHandCarryEvidencePc34* evidence;
    int requestedChampionIndex;
    int oldInventoryOrdinalBefore;
    int oldInventoryOrdinalAfter;
    int targetOrdinalAfter;
    int leaderHandThingBefore;
    int leaderHandThingAfter;
    int leaderHandPreserved;
    int openChestThingBefore;
    int openChestThingAfter;
    int chestClosed;
    int chestCloseCountAfter;
    int closedChestTypes[DM1_V1_ICSWHC_CHEST_SLOT_COUNT_PC34];
    int chestSlotsClearedAfterClose;
    int acceptedSwitch;
    int acceptedClose;
    int rejectedDeadChampion;
    int rejectedMouthEyePress;
    int oldStatusDrawDelta;
    int newStatusDrawDelta;
    int slotDrawDelta;
    int movementArrowsDrawDelta;
    int floorCeilingDrawDelta;
    int mousePointerBitmapUpdatedDelta;
    int refreshMousePointerDelta;
    int secondaryInputInventoryAfter;
    int secondaryInputMovementAfter;
    int discardInputDelta;
    int stopWaitingForPlayerInputAfter;
} DM1_V1_InventoryChampionSwitchHandCarryResultPc34;

const DM1_V1_InventoryChampionSwitchHandCarryEvidencePc34*
DM1_V1_InventoryChampionSwitchHandCarry_EvidencePc34(void);

void DM1_V1_InventoryChampionSwitchHandCarry_InitPc34(
    DM1_V1_InventoryChampionSwitchHandCarryStatePc34* state,
    int openInventoryChampionIndex,
    int openChestThing);

int DM1_V1_InventoryChampionSwitchHandCarry_OpenPc34(
    DM1_V1_InventoryChampionSwitchHandCarryStatePc34* state,
    int requestedChampionIndex,
    DM1_V1_InventoryChampionSwitchHandCarryResultPc34* outResult);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_INVENTORY_CHAMPION_SWITCH_HAND_CARRY_PC34_COMPAT_H */
