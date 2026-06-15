#ifndef FIRESTAFF_DM1_V1_INVENTORY_CHEST_DROP_TO_FLOOR_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_INVENTORY_CHEST_DROP_TO_FLOOR_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_CHEST_DROP_FLOOR_SLOT_COUNT = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_CHEST_DROP_FLOOR_CASE_COUNT = 5,
    DM1_PC34_CHEST_DROP_FLOOR_CELL_CAPACITY = 8,
    DM1_PC34_CHEST_DROP_FLOOR_THING_NONE = 0xFFFF,
    DM1_PC34_CHEST_DROP_FLOOR_THING_ENDOFLIST = 0xFFFE,
    DM1_PC34_CHEST_DROP_FLOOR_CHEST_THING = 0x7D40,
    DM1_PC34_CHEST_DROP_FLOOR_MAP_X = 14,
    DM1_PC34_CHEST_DROP_FLOOR_MAP_Y = 22
};

typedef struct {
    int result;
    int championIndex;
    int pc34Slot;
    int chestSlotIndex;
    int sourceIsChestSlot;
    int slotTypeBefore;
    int slotWeightBefore;
    int loadBefore;
    int loadAfterPickup;
    int loadAfterDrop;
    int expectedLoadAfterDrop;
    int mouseTypeAfterPickup;
    int mouseTypeAfterDrop;
    int chestSlotCleared;
    int floorCountBefore;
    int floorCountAfter;
    int floorCellIndex;
    int floorCellType;
    int floorCellWeight;
    int g0189FirstThing;
    int g0189Terminator;
    int adjacentLeftTypeBefore;
    int adjacentLeftTypeAfter;
    int adjacentRightTypeBefore;
    int adjacentRightTypeAfter;
    int nonEmptyBefore;
    int nonEmptyAfter;
    int chestLoadBefore;
    int chestLoadAfter;
    int droppedThingPreserved;
    int noAdjacentOrphan;
    int rabbitFootRefreshNotUsed;
    int slotZeroOrphanBugReproduced;
    int closedCountAfterDrop;
    int closedTypes[DM1_PC34_CHEST_DROP_FLOOR_SLOT_COUNT];
    int closedWeights[DM1_PC34_CHEST_DROP_FLOOR_SLOT_COUNT];
} DM1_V1_InventoryChestDropToFloorEventPc34;

typedef struct {
    M11_InventoryState inventory;
    M11_Item g0189_aT_Dungeon[DM1_PC34_CHEST_DROP_FLOOR_CELL_CAPACITY];
    int floorCount;
    int mapX;
    int mapY;
} DM1_V1_InventoryChestDropToFloorStatePc34;

typedef struct {
    DM1_V1_InventoryChestDropToFloorEventPc34 singleDrop;
    DM1_V1_InventoryChestDropToFloorEventPc34 fullChestDrop;
    DM1_V1_InventoryChestDropToFloorEventPc34 weightedDrop;
    DM1_V1_InventoryChestDropToFloorEventPc34 lastItemDrop;
    DM1_V1_InventoryChestDropToFloorEventPc34 indexedDrop;
    int sourceLockedContractOnly;
    int caseCount;
    int firstChestSlot;
    int lastChestSlot;
    int floorMapX;
    int floorMapY;
    int floorCellCapacity;
    int floorDropsTotal;
    int allDroppedItemsOnG0189Floor;
    int allPostDropLoadsMatch;
    int allAdjacentItemsPreserved;
} DM1_V1_InventoryChestDropToFloorProbePc34;

const char*
dm1_v1_inventory_chest_drop_to_floor_source_evidence_pc34(void);
void dm1_v1_inventory_chest_drop_to_floor_init_pc34(
    DM1_V1_InventoryChestDropToFloorStatePc34* state);
int dm1_v1_inventory_chest_drop_to_floor_open_pc34(
    DM1_V1_InventoryChestDropToFloorStatePc34* state,
    const M11_Item* linkedItems,
    int linkedItemCount);
int dm1_v1_inventory_chest_drop_to_floor_run_case_pc34(
    DM1_V1_InventoryChestDropToFloorStatePc34* state,
    int chestSlotIndex,
    DM1_V1_InventoryChestDropToFloorEventPc34* outEvent);
int m11_inventory_pc34_probe_chest_drop_to_floor(
    DM1_V1_InventoryChestDropToFloorProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_INVENTORY_CHEST_DROP_TO_FLOOR_PC34_COMPAT_H */
