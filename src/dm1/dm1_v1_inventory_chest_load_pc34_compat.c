#include "dm1_v1_inventory_chest_load_pc34_compat.h"

const char* DM1_V1_InventoryChestLoad_SourceEvidencePc34Compat(void)
{
    return
        "DUNGEON.C:1082-1133 F0140_DUNGEON_GetObjectWeight\n"
        "DUNGEON.C:1114-1120 F0140 gives containers weight 50 plus linked CONTENTS\n"
        "CHEST.C:53-76 F0333 copies only first eight links into G0425_aT_ChestSlots\n"
        "CHEST.C:112-133 F0334 compacts non-empty G0425 slots and clears the open chest\n"
        "CHAMPION.C:263-265 F0297 adds F0140 object weight to leader load\n"
        "CHAMPION.C:582-615 F0300/F0301 remove/add F0140 object weight from slot load";
}

int DM1_V1_InventoryChestLoad_OpenChestVisibleContentsWeightPc34Compat(const DM1_V1_InventoryStatePc34* s,
                                                          int champ)
{
    int total = 0;
    int slotIndex;

    if (!s || champ < 0 || champ >= s->championCount ||
        s->champions[champ].openChestThing == 0) {
        return 0;
    }

    /* ReDMCSB CHEST.C F0333 lines 53-76 materializes only the first eight
     * linked CONTENTS entries into G0425_aT_ChestSlots for the open panel.
     * DUNGEON.C F0140 lines 1117-1119 then sums object weights by following
     * the current linked CONTENTS chain; in this synthetic panel helper the
     * visible G0425 snapshot is the source-locked chain under test. */
    for (slotIndex = 0; slotIndex < DM1_PC34_CHEST_SLOT_COUNT; ++slotIndex) {
        const DM1_V1_ItemPc34* item = &s->champions[champ].chestSlots[slotIndex];
        if (item->itemType != 0) {
            total += item->weight;
        }
    }
    return total;
}

int DM1_V1_InventoryChestLoad_OpenChestContainerWeightPc34Compat(const DM1_V1_InventoryStatePc34* s,
                                                   int champ)
{
    if (!s || champ < 0 || champ >= s->championCount ||
        s->champions[champ].openChestThing == 0) {
        return 0;
    }

    /* ReDMCSB DUNGEON.C F0140 lines 1114-1120 gives every container a base
     * weight of 50 before adding each linked CONTENTS object weight. */
    return DM1_PC34_CHEST_EMPTY_THING_WEIGHT +
           DM1_V1_InventoryChestLoad_OpenChestVisibleContentsWeightPc34Compat(s, champ);
}

int DM1_V1_InventoryChestLoad_CloseChestWithWeightSnapshotPc34Compat(DM1_V1_InventoryStatePc34* s,
                                                        int champ,
                                                        DM1_V1_ItemPc34* linkedItemsOut,
                                                        int maxItemsOut,
                                                        int* outContainerWeightBeforeClose)
{
    int weightBeforeClose;

    if (outContainerWeightBeforeClose) {
        *outContainerWeightBeforeClose = 0;
    }
    if (!s || champ < 0 || champ >= s->championCount || maxItemsOut < 0 ||
        (maxItemsOut > 0 && !linkedItemsOut)) {
        return -1;
    }
    if (s->champions[champ].openChestThing == 0) {
        return 0;
    }

    /* ReDMCSB CHAMPION.C F0297/F0300/F0301 lines 263-265 and 582-615 adjust
     * champion Load through F0140 object weight.  CHEST.C F0334 lines 112-133
     * closes the open G0426 chest by compacting G0425 first, so capture the
     * source F0140 container weight before the transient slots are erased. */
    weightBeforeClose = DM1_V1_InventoryChestLoad_OpenChestContainerWeightPc34Compat(s, champ);
    if (outContainerWeightBeforeClose) {
        *outContainerWeightBeforeClose = weightBeforeClose;
    }

    /* ReDMCSB CHEST.C F0334 lines 117-132 rewrites the CONTENTS list from
     * non-empty G0425 slots and clears those slots, after which the champion
     * load must be recomputed without the transient open-chest contents. */
    return DM1_V1_Inventory_CloseChestPc34Compat(s, champ, linkedItemsOut, maxItemsOut);
}
