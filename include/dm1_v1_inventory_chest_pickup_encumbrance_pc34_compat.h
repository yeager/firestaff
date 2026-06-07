#ifndef FIRESTAFF_DM1_V1_INVENTORY_CHEST_PICKUP_ENCUMBRANCE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_INVENTORY_CHEST_PICKUP_ENCUMBRANCE_PC34_COMPAT_H

#include <stdint.h>

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_CHEST_PICKUP_ENCUMBRANCE_CHAMPION_COUNT = 4,
    DM1_PC34_CHEST_PICKUP_ENCUMBRANCE_SLOT_COUNT = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_CHEST_PICKUP_ENCUMBRANCE_WEIGHT_BYTE_CASE_COUNT = 4
};

typedef struct {
    uint32_t load;
    uint32_t maxLoad;
    int encumbered;
    M11_Item storage[DM1_SLOT_COUNT];
} DM1_V1_InventoryChestPickupChampionPc34;

typedef struct {
    DM1_V1_InventoryChestPickupChampionPc34 champions[DM1_PC34_CHEST_PICKUP_ENCUMBRANCE_CHAMPION_COUNT];
    M11_Item chestSlots[DM1_PC34_CHEST_PICKUP_ENCUMBRANCE_CHAMPION_COUNT][DM1_PC34_CHEST_PICKUP_ENCUMBRANCE_SLOT_COUNT];
} DM1_V1_InventoryChestPickupEncumbranceStatePc34;

typedef struct {
    int result;
    int championIndex;
    int chestSlotIndex;
    int storageSlotIndex;
    uint32_t itemWeight;
    uint32_t loadBefore;
    uint32_t loadAfter;
    uint32_t maxLoad;
    int encumberedBefore;
    int encumberedAfter;
    int storedInChampionSlot;
    int clearedChestSlot;
    int saturated;
} DM1_V1_InventoryChestPickupEncumbranceEventPc34;

typedef struct {
    DM1_V1_InventoryChestPickupEncumbranceEventPc34 firstPickup;
    DM1_V1_InventoryChestPickupEncumbranceEventPc34 thresholdPickup;
    DM1_V1_InventoryChestPickupEncumbranceEventPc34 saturatingPickup;
    DM1_V1_InventoryChestPickupEncumbranceEventPc34 doublePickup;
    DM1_V1_InventoryChestPickupEncumbranceEventPc34 championPickups[DM1_PC34_CHEST_PICKUP_ENCUMBRANCE_CHAMPION_COUNT];
    uint32_t championLoads[DM1_PC34_CHEST_PICKUP_ENCUMBRANCE_CHAMPION_COUNT];
    uint32_t championMaxLoads[DM1_PC34_CHEST_PICKUP_ENCUMBRANCE_CHAMPION_COUNT];
    int championEncumbered[DM1_PC34_CHEST_PICKUP_ENCUMBRANCE_CHAMPION_COUNT];
    int independentChampionStorage[DM1_PC34_CHEST_PICKUP_ENCUMBRANCE_CHAMPION_COUNT];
    uint32_t weightByteCases[DM1_PC34_CHEST_PICKUP_ENCUMBRANCE_WEIGHT_BYTE_CASE_COUNT];
    uint32_t weightByteCaseSum;
    uint32_t weightByteCaseSaturatedSum;
    int sourceLockedContractOnly;
} DM1_V1_InventoryChestPickupEncumbranceProbePc34;

const char* dm1_v1_inventory_chest_pickup_encumbrance_source_evidence_pc34(void);
void dm1_v1_inventory_chest_pickup_encumbrance_init_pc34(
    DM1_V1_InventoryChestPickupEncumbranceStatePc34* state);
int dm1_v1_inventory_chest_pickup_encumbrance_set_champion_pc34(
    DM1_V1_InventoryChestPickupEncumbranceStatePc34* state,
    int championIndex,
    uint32_t load,
    uint32_t maxLoad);
int dm1_v1_inventory_chest_pickup_encumbrance_set_chest_item_pc34(
    DM1_V1_InventoryChestPickupEncumbranceStatePc34* state,
    int championIndex,
    int chestSlotIndex,
    int itemType,
    uint32_t weight);
uint32_t dm1_v1_inventory_chest_pickup_encumbrance_weight_byte_pc34(int weight);
uint32_t dm1_v1_inventory_chest_pickup_encumbrance_add_weight_pc34(
    uint32_t load,
    uint32_t weight,
    int* saturated);
int dm1_v1_inventory_chest_pickup_encumbrance_is_encumbered_pc34(
    uint32_t load,
    uint32_t maxLoad);
int dm1_v1_inventory_chest_pickup_encumbrance_pickup_pc34(
    DM1_V1_InventoryChestPickupEncumbranceStatePc34* state,
    int championIndex,
    int chestSlotIndex,
    int storageSlotIndex,
    DM1_V1_InventoryChestPickupEncumbranceEventPc34* outEvent);
int m11_inventory_pc34_probe_chest_pickup_encumbrance(
    DM1_V1_InventoryChestPickupEncumbranceProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_INVENTORY_CHEST_PICKUP_ENCUMBRANCE_PC34_COMPAT_H */
