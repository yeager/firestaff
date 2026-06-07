#include "dm1_v1_inventory_chest_pickup_encumbrance_pc34_compat.h"

#include <string.h>

static M11_Item make_item_pc34(int itemType, uint32_t weight)
{
    M11_Item item;

    memset(&item, 0, sizeof(item));
    item.itemType = itemType;
    item.weight = (int)dm1_v1_inventory_chest_pickup_encumbrance_weight_byte_pc34((int)weight);
    item.allowedSlots = DM1_PC34_ALLOWED_ANY_SLOT;
    return item;
}

const char* dm1_v1_inventory_chest_pickup_encumbrance_source_evidence_pc34(void)
{
    return
        "CHEST.C:53-76 F0333 copies the open chest links into G0425_aT_ChestSlots\n"
        "CHAMPION.C:688-710 F0302 routes C30+ chest slot clicks through F0300/F0297/F0301\n"
        "CHAMPION.C:263-266 F0297 adds picked leader-hand object weight immediately\n"
        "CHAMPION.C:609-615 F0301 stores C30+ slots and ordinary slots before adding F0140 weight\n"
        "CHAMPION.C:1198-1205 F0310 compares Load against F0309 maximum load for encumbrance\n"
        "DUNGEON.C:1102-1119 F0140 reads unsigned byte object-info weights and recursively accumulates container weight\n"
        "DEFS.H:1712-1738 declares WEAPON_INFO/ARMOUR_INFO Weight as unsigned char";
}

void dm1_v1_inventory_chest_pickup_encumbrance_init_pc34(
    DM1_V1_InventoryChestPickupEncumbranceStatePc34* state)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
}

int dm1_v1_inventory_chest_pickup_encumbrance_set_champion_pc34(
    DM1_V1_InventoryChestPickupEncumbranceStatePc34* state,
    int championIndex,
    uint32_t load,
    uint32_t maxLoad)
{
    DM1_V1_InventoryChestPickupChampionPc34* champion;

    if (!state || championIndex < 0 ||
        championIndex >= DM1_PC34_CHEST_PICKUP_ENCUMBRANCE_CHAMPION_COUNT) {
        return 0;
    }
    champion = &state->champions[championIndex];
    champion->load = load;
    champion->maxLoad = maxLoad;
    champion->encumbered =
        dm1_v1_inventory_chest_pickup_encumbrance_is_encumbered_pc34(
            load, maxLoad);
    return 1;
}

int dm1_v1_inventory_chest_pickup_encumbrance_set_chest_item_pc34(
    DM1_V1_InventoryChestPickupEncumbranceStatePc34* state,
    int championIndex,
    int chestSlotIndex,
    int itemType,
    uint32_t weight)
{
    if (!state || championIndex < 0 ||
        championIndex >= DM1_PC34_CHEST_PICKUP_ENCUMBRANCE_CHAMPION_COUNT ||
        chestSlotIndex < 0 ||
        chestSlotIndex >= DM1_PC34_CHEST_PICKUP_ENCUMBRANCE_SLOT_COUNT ||
        itemType == 0) {
        return 0;
    }
    state->chestSlots[championIndex][chestSlotIndex] =
        make_item_pc34(itemType, weight);
    return 1;
}

uint32_t dm1_v1_inventory_chest_pickup_encumbrance_weight_byte_pc34(int weight)
{
    if (weight <= 0) {
        return 0;
    }
    if (weight > 255) {
        return 255;
    }
    return (uint32_t)weight;
}

uint32_t dm1_v1_inventory_chest_pickup_encumbrance_add_weight_pc34(
    uint32_t load,
    uint32_t weight,
    int* saturated)
{
    if (saturated) {
        *saturated = 0;
    }
    if (UINT32_MAX - load < weight) {
        if (saturated) {
            *saturated = 1;
        }
        return UINT32_MAX;
    }
    return load + weight;
}

int dm1_v1_inventory_chest_pickup_encumbrance_is_encumbered_pc34(
    uint32_t load,
    uint32_t maxLoad)
{
    if (maxLoad == 0) {
        return load != 0 ? 1 : 0;
    }
    /* ReDMCSB: CHAMPION.C F0310 lines 1198-1205 uses maxLoad > Load for
     * the non-overloaded branch, so equal load already enters the encumbered
     * movement path even though the display color note calls out BUG0_72. */
    return load >= maxLoad ? 1 : 0;
}

int dm1_v1_inventory_chest_pickup_encumbrance_pickup_pc34(
    DM1_V1_InventoryChestPickupEncumbranceStatePc34* state,
    int championIndex,
    int chestSlotIndex,
    int storageSlotIndex,
    DM1_V1_InventoryChestPickupEncumbranceEventPc34* outEvent)
{
    DM1_V1_InventoryChestPickupChampionPc34* champion;
    M11_Item* chestSlot;
    M11_Item item;
    int saturated = 0;

    if (outEvent) {
        memset(outEvent, 0, sizeof(*outEvent));
        outEvent->championIndex = championIndex;
        outEvent->chestSlotIndex = chestSlotIndex;
        outEvent->storageSlotIndex = storageSlotIndex;
    }
    if (!state || championIndex < 0 ||
        championIndex >= DM1_PC34_CHEST_PICKUP_ENCUMBRANCE_CHAMPION_COUNT ||
        chestSlotIndex < 0 ||
        chestSlotIndex >= DM1_PC34_CHEST_PICKUP_ENCUMBRANCE_SLOT_COUNT ||
        storageSlotIndex < 0 || storageSlotIndex >= DM1_SLOT_COUNT) {
        return 0;
    }

    champion = &state->champions[championIndex];
    chestSlot = &state->chestSlots[championIndex][chestSlotIndex];
    if (chestSlot->itemType == 0 || champion->storage[storageSlotIndex].itemType != 0) {
        return 0;
    }

    item = *chestSlot;
    if (outEvent) {
        outEvent->itemWeight = (uint32_t)item.weight;
        outEvent->loadBefore = champion->load;
        outEvent->maxLoad = champion->maxLoad;
        outEvent->encumberedBefore = champion->encumbered;
    }

    /* ReDMCSB: CHAMPION.C F0302 lines 688-710 removes the C30+ G0425 slot
     * object, then F0297 lines 263-266 / F0301 lines 609-615 add the same
     * F0140 weight to the champion-owned load before any chest close pass. */
    champion->storage[storageSlotIndex] = item;
    memset(chestSlot, 0, sizeof(*chestSlot));
    champion->load = dm1_v1_inventory_chest_pickup_encumbrance_add_weight_pc34(
        champion->load, (uint32_t)item.weight, &saturated);
    champion->encumbered =
        dm1_v1_inventory_chest_pickup_encumbrance_is_encumbered_pc34(
            champion->load, champion->maxLoad);

    if (outEvent) {
        outEvent->result = 1;
        outEvent->loadAfter = champion->load;
        outEvent->encumberedAfter = champion->encumbered;
        outEvent->storedInChampionSlot =
            champion->storage[storageSlotIndex].itemType == item.itemType ? 1 : 0;
        outEvent->clearedChestSlot = chestSlot->itemType == 0 ? 1 : 0;
        outEvent->saturated = saturated;
    }
    return 1;
}

int m11_inventory_pc34_probe_chest_pickup_encumbrance(
    DM1_V1_InventoryChestPickupEncumbranceProbePc34* out)
{
    DM1_V1_InventoryChestPickupEncumbranceStatePc34 state;
    static const uint32_t weights[DM1_PC34_CHEST_PICKUP_ENCUMBRANCE_WEIGHT_BYTE_CASE_COUNT] = {
        0, 1, 254, 255
    };
    int i;
    int saturated;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    out->sourceLockedContractOnly = 1;
    dm1_v1_inventory_chest_pickup_encumbrance_init_pc34(&state);

    for (i = 0; i < DM1_PC34_CHEST_PICKUP_ENCUMBRANCE_CHAMPION_COUNT; ++i) {
        dm1_v1_inventory_chest_pickup_encumbrance_set_champion_pc34(
            &state, i, (uint32_t)(i * 10), (uint32_t)(40 + (i * 25)));
        dm1_v1_inventory_chest_pickup_encumbrance_set_chest_item_pc34(
            &state, i, i, 1000 + i, (uint32_t)(11 + (i * 7)));
        dm1_v1_inventory_chest_pickup_encumbrance_pickup_pc34(
            &state, i, i, DM1_SLOT_BACKPACK1 + i, &out->championPickups[i]);
        out->championLoads[i] = state.champions[i].load;
        out->championMaxLoads[i] = state.champions[i].maxLoad;
        out->championEncumbered[i] = state.champions[i].encumbered;
        out->independentChampionStorage[i] =
            state.champions[i].storage[DM1_SLOT_BACKPACK1 + i].itemType;
    }

    dm1_v1_inventory_chest_pickup_encumbrance_init_pc34(&state);
    dm1_v1_inventory_chest_pickup_encumbrance_set_champion_pc34(&state, 0, 12, 200);
    dm1_v1_inventory_chest_pickup_encumbrance_set_chest_item_pc34(&state, 0, 0, 2000, 23);
    dm1_v1_inventory_chest_pickup_encumbrance_pickup_pc34(
        &state, 0, 0, DM1_SLOT_BACKPACK1, &out->firstPickup);

    dm1_v1_inventory_chest_pickup_encumbrance_init_pc34(&state);
    dm1_v1_inventory_chest_pickup_encumbrance_set_champion_pc34(&state, 1, 38, 50);
    dm1_v1_inventory_chest_pickup_encumbrance_set_chest_item_pc34(&state, 1, 1, 2100, 13);
    dm1_v1_inventory_chest_pickup_encumbrance_pickup_pc34(
        &state, 1, 1, DM1_SLOT_BACKPACK2, &out->thresholdPickup);

    dm1_v1_inventory_chest_pickup_encumbrance_init_pc34(&state);
    dm1_v1_inventory_chest_pickup_encumbrance_set_champion_pc34(
        &state, 2, UINT32_MAX - 3U, 100);
    dm1_v1_inventory_chest_pickup_encumbrance_set_chest_item_pc34(&state, 2, 2, 2200, 9);
    dm1_v1_inventory_chest_pickup_encumbrance_pickup_pc34(
        &state, 2, 2, DM1_SLOT_BACKPACK3, &out->saturatingPickup);

    dm1_v1_inventory_chest_pickup_encumbrance_init_pc34(&state);
    dm1_v1_inventory_chest_pickup_encumbrance_set_champion_pc34(&state, 3, 42, 100);
    state.champions[3].storage[DM1_SLOT_BACKPACK1] = make_item_pc34(2300, 42);
    dm1_v1_inventory_chest_pickup_encumbrance_set_chest_item_pc34(&state, 3, 3, 2301, 42);
    dm1_v1_inventory_chest_pickup_encumbrance_pickup_pc34(
        &state, 3, 3, DM1_SLOT_BACKPACK4, &out->doublePickup);

    out->weightByteCaseSum = 0;
    for (i = 0; i < DM1_PC34_CHEST_PICKUP_ENCUMBRANCE_WEIGHT_BYTE_CASE_COUNT; ++i) {
        out->weightByteCases[i] =
            dm1_v1_inventory_chest_pickup_encumbrance_weight_byte_pc34((int)weights[i]);
        out->weightByteCaseSum =
            dm1_v1_inventory_chest_pickup_encumbrance_add_weight_pc34(
                out->weightByteCaseSum, out->weightByteCases[i], 0);
    }
    out->weightByteCaseSaturatedSum =
        dm1_v1_inventory_chest_pickup_encumbrance_add_weight_pc34(
            UINT32_MAX - 1U, out->weightByteCases[3], &saturated);
    (void)saturated;

    return 1;
}
