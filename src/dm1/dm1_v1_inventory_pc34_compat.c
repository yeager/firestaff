#include "dm1_v1_inventory_pc34_compat.h"
#include <string.h>

static const int kPc34SlotMasks[DM1_PC34_SLOT_COUNT] = {
    DM1_PC34_ALLOWED_ANY_SLOT,   /* Ready Hand */
    DM1_PC34_ALLOWED_ANY_SLOT,   /* Action Hand */
    DM1_PC34_ALLOWED_HEAD,       /* Head */
    DM1_PC34_ALLOWED_TORSO,      /* Torso */
    DM1_PC34_ALLOWED_LEGS,       /* Legs */
    DM1_PC34_ALLOWED_FEET,       /* Feet */
    DM1_PC34_ALLOWED_POUCH,      /* Pouch 2 */
    DM1_PC34_ALLOWED_QUIVER_LINE2,
    DM1_PC34_ALLOWED_QUIVER_LINE2,
    DM1_PC34_ALLOWED_QUIVER_LINE2,
    DM1_PC34_ALLOWED_NECK,
    DM1_PC34_ALLOWED_POUCH,
    DM1_PC34_ALLOWED_QUIVER_LINE1,
    DM1_PC34_ALLOWED_ANY_SLOT,
    DM1_PC34_ALLOWED_ANY_SLOT,
    DM1_PC34_ALLOWED_ANY_SLOT,
    DM1_PC34_ALLOWED_ANY_SLOT,
    DM1_PC34_ALLOWED_ANY_SLOT,
    DM1_PC34_ALLOWED_ANY_SLOT,
    DM1_PC34_ALLOWED_ANY_SLOT,
    DM1_PC34_ALLOWED_ANY_SLOT,
    DM1_PC34_ALLOWED_ANY_SLOT,
    DM1_PC34_ALLOWED_ANY_SLOT,
    DM1_PC34_ALLOWED_ANY_SLOT,
    DM1_PC34_ALLOWED_ANY_SLOT,
    DM1_PC34_ALLOWED_ANY_SLOT,
    DM1_PC34_ALLOWED_ANY_SLOT,
    DM1_PC34_ALLOWED_ANY_SLOT,
    DM1_PC34_ALLOWED_ANY_SLOT,
    DM1_PC34_ALLOWED_ANY_SLOT,
    DM1_PC34_ALLOWED_CONTAINER,
    DM1_PC34_ALLOWED_CONTAINER,
    DM1_PC34_ALLOWED_CONTAINER,
    DM1_PC34_ALLOWED_CONTAINER,
    DM1_PC34_ALLOWED_CONTAINER,
    DM1_PC34_ALLOWED_CONTAINER,
    DM1_PC34_ALLOWED_CONTAINER,
    DM1_PC34_ALLOWED_CONTAINER
};

static void dm1_v1_inventory_clear_item_pc34(DM1_V1_ItemPc34* item) {
    memset(item, 0, sizeof(*item));
}

void DM1_V1_Inventory_InitPc34Compat(DM1_V1_InventoryStatePc34* s, int championCount) {
    if (!s) return;
    memset(s, 0, sizeof(DM1_V1_InventoryStatePc34));
    s->championCount = championCount;
    s->panelContent = DM1_PC34_PANEL_INVENTORY;
}

int DM1_V1_Inventory_SetItemPc34Compat(DM1_V1_InventoryStatePc34* s, int champ, int slot, int itemType, int weight, int charges) {
    return DM1_V1_Inventory_SetItemWithAllowedSlotsPc34Compat(
        s, champ, slot, itemType, weight, charges, DM1_PC34_ALLOWED_ANY_SLOT);
}

int DM1_V1_Inventory_SetItemWithAllowedSlotsPc34Compat(DM1_V1_InventoryStatePc34* s, int champ, int slot,
                                              int itemType, int weight, int charges,
                                              int allowedSlots) {
    if (!s || champ < 0 || champ >= s->championCount || slot < 0 || slot >= DM1_SLOT_COUNT) {
        return 0;
    }
    DM1_V1_ChampionInventoryPc34* inv = &s->champions[champ];
    inv->slots[slot].itemType = itemType;
    inv->slots[slot].weight = weight;
    inv->slots[slot].charges = charges;
    inv->slots[slot].cursed = 0;
    inv->slots[slot].identified = 0;
    inv->slots[slot].allowedSlots = allowedSlots;
    DM1_V1_Inventory_RecalcLoadPc34Compat(s, champ);
    return 1;
}

int DM1_V1_Inventory_GetItemPc34Compat(const DM1_V1_InventoryStatePc34* s, int champ, int slot, DM1_V1_ItemPc34* out) {
    if (!s || !out || champ < 0 || champ >= s->championCount || slot < 0 || slot >= DM1_SLOT_COUNT) {
        return 0;
    }
    const DM1_V1_ChampionInventoryPc34* inv = &s->champions[champ];
    *out = inv->slots[slot];
    return 1;
}

int DM1_V1_Inventory_RemoveItemPc34Compat(DM1_V1_InventoryStatePc34* s, int champ, int slot) {
    if (!s || champ < 0 || champ >= s->championCount || slot < 0 || slot >= DM1_SLOT_COUNT) {
        return 0;
    }
    DM1_V1_ChampionInventoryPc34* inv = &s->champions[champ];
    dm1_v1_inventory_clear_item_pc34(&inv->slots[slot]);
    DM1_V1_Inventory_RecalcLoadPc34Compat(s, champ);
    return 1;
}

int DM1_V1_Inventory_SwapHandPc34Compat(DM1_V1_InventoryStatePc34* s, int champ) {
    if (!s || champ < 0 || champ >= s->championCount) {
        return 0;
    }
    DM1_V1_ChampionInventoryPc34* inv = &s->champions[champ];
    DM1_V1_ItemPc34 temp = inv->slots[DM1_SLOT_HAND_RIGHT];
    inv->slots[DM1_SLOT_HAND_RIGHT] = inv->slots[DM1_SLOT_HAND_LEFT];
    inv->slots[DM1_SLOT_HAND_LEFT] = temp;
    return 1;
}

int DM1_V1_Inventory_PickupMousePc34Compat(DM1_V1_InventoryStatePc34* s, int champ, int slot) {
    if (!s || champ < 0 || champ >= s->championCount || slot < 0 || slot >= DM1_SLOT_COUNT) {
        return 0;
    }
    DM1_V1_ChampionInventoryPc34* inv = &s->champions[champ];

    if (inv->slots[slot].itemType == 0) {
        return 0;
    }

    if (inv->mouseItem.itemType != 0) {
        return 0;
    }

    inv->mouseItem = inv->slots[slot];
    dm1_v1_inventory_clear_item_pc34(&inv->slots[slot]);

    DM1_V1_Inventory_RecalcLoadPc34Compat(s, champ);
    return 1;
}

int DM1_V1_Inventory_DropMousePc34Compat(DM1_V1_InventoryStatePc34* s, int champ, int slot) {
    if (!s || champ < 0 || champ >= s->championCount || slot < 0 || slot >= DM1_SLOT_COUNT) {
        return 0;
    }
    DM1_V1_ChampionInventoryPc34* inv = &s->champions[champ];

    if (inv->mouseItem.itemType == 0) {
        return 0;
    }

    if (inv->slots[slot].itemType != 0) {
        return 0;
    }

    inv->slots[slot] = inv->mouseItem;
    dm1_v1_inventory_clear_item_pc34(&inv->mouseItem);

    DM1_V1_Inventory_RecalcLoadPc34Compat(s, champ);
    return 1;
}

void DM1_V1_Inventory_RecalcLoadPc34Compat(DM1_V1_InventoryStatePc34* s, int champ) {
    if (!s || champ < 0 || champ >= s->championCount) {
        return;
    }
    DM1_V1_ChampionInventoryPc34* inv = &s->champions[champ];
    int totalLoad = 0;
    for (int i = 0; i < DM1_SLOT_COUNT; i++) {
        totalLoad += inv->slots[i].weight;
    }
    if (inv->openChestThing != 0) {
        for (int i = 0; i < DM1_PC34_CHEST_SLOT_COUNT; i++) {
            totalLoad += inv->chestSlots[i].weight;
        }
    }
    inv->load = totalLoad;
}

int DM1_V1_Inventory_Pc34IsBackpackSourceSlotCompat(int pc34Slot) {
    return pc34Slot >= DM1_PC34_SLOT_BACKPACK_LINE1_1 &&
           pc34Slot <= DM1_PC34_SLOT_BACKPACK_LINE1_9;
}

int DM1_V1_Inventory_Pc34IsChestSourceSlotCompat(int pc34Slot) {
    return pc34Slot >= DM1_PC34_SLOT_CHEST_1 && pc34Slot <= DM1_PC34_SLOT_CHEST_8;
}

static DM1_V1_ItemPc34* dm1_v1_inventory_pc34_mutable_slot(DM1_V1_InventoryStatePc34* s, int champ, int pc34Slot) {
    int storageSlot;
    if (!s || champ < 0 || champ >= s->championCount) {
        return NULL;
    }
    if (DM1_V1_Inventory_Pc34IsChestSourceSlotCompat(pc34Slot)) {
        DM1_V1_ChampionInventoryPc34* inv = &s->champions[champ];
        if (inv->openChestThing == 0) {
            return NULL;
        }
        return &inv->chestSlots[pc34Slot - DM1_PC34_SLOT_CHEST_1];
    }
    storageSlot = DM1_V1_Inventory_Pc34SourceSlotToStorageSlotCompat(pc34Slot);
    if (storageSlot < 0) {
        return NULL;
    }
    return &s->champions[champ].slots[storageSlot];
}

static const DM1_V1_ItemPc34* dm1_v1_inventory_pc34_const_slot(const DM1_V1_InventoryStatePc34* s, int champ, int pc34Slot) {
    int storageSlot;
    if (!s || champ < 0 || champ >= s->championCount) {
        return NULL;
    }
    if (DM1_V1_Inventory_Pc34IsChestSourceSlotCompat(pc34Slot)) {
        const DM1_V1_ChampionInventoryPc34* inv = &s->champions[champ];
        if (inv->openChestThing == 0) {
            return NULL;
        }
        return &inv->chestSlots[pc34Slot - DM1_PC34_SLOT_CHEST_1];
    }
    storageSlot = DM1_V1_Inventory_Pc34SourceSlotToStorageSlotCompat(pc34Slot);
    if (storageSlot < 0) {
        return NULL;
    }
    return &s->champions[champ].slots[storageSlot];
}

int DM1_V1_Inventory_GetLoadPc34Compat(const DM1_V1_InventoryStatePc34* s, int champ) {
    if (!s || champ < 0 || champ >= s->championCount) {
        return 0;
    }
    return s->champions[champ].load;
}

int DM1_V1_Inventory_Pc34SlotMaskCompat(int pc34Slot) {
    if (pc34Slot < 0 || pc34Slot >= DM1_PC34_SLOT_COUNT) {
        return 0;
    }
    return kPc34SlotMasks[pc34Slot];
}

int DM1_V1_Inventory_Pc34SourceSlotToStorageSlotCompat(int pc34Slot) {
    if (DM1_V1_Inventory_Pc34IsBackpackSourceSlotCompat(pc34Slot)) {
        return DM1_SLOT_BACKPACK1 + (pc34Slot - DM1_PC34_SLOT_BACKPACK_LINE1_1);
    }

    switch (pc34Slot) {
    case DM1_PC34_SLOT_READY_HAND:
        return DM1_SLOT_HAND_RIGHT;
    case DM1_PC34_SLOT_ACTION_HAND:
        return DM1_SLOT_HAND_LEFT;
    case DM1_PC34_SLOT_HEAD:
        return DM1_SLOT_HEAD;
    case DM1_PC34_SLOT_TORSO:
        return DM1_SLOT_TORSO;
    case DM1_PC34_SLOT_LEGS:
        return DM1_SLOT_LEGS;
    case DM1_PC34_SLOT_FEET:
        return DM1_SLOT_FEET;
    case DM1_PC34_SLOT_POUCH_2:
        return DM1_SLOT_POUCH2;
    case DM1_PC34_SLOT_QUIVER_LINE2_1:
        return DM1_SLOT_QUIVER2;
    case DM1_PC34_SLOT_QUIVER_LINE1_2:
        return DM1_SLOT_QUIVER3;
    case DM1_PC34_SLOT_QUIVER_LINE2_2:
        return DM1_SLOT_QUIVER4;
    case DM1_PC34_SLOT_NECK:
        return DM1_SLOT_NECK;
    case DM1_PC34_SLOT_POUCH_1:
        return DM1_SLOT_POUCH1;
    case DM1_PC34_SLOT_QUIVER_LINE1_1:
        return DM1_SLOT_QUIVER1;
    default:
        return -1;
    }
}

int DM1_V1_Inventory_SetMouseItemPc34Compat(DM1_V1_InventoryStatePc34* s, int champ, int itemType,
                                 int weight, int charges, int allowedSlots) {
    if (!s || champ < 0 || champ >= s->championCount) {
        return 0;
    }
    DM1_V1_ItemPc34* item = &s->champions[champ].mouseItem;
    item->itemType = itemType;
    item->weight = weight;
    item->charges = charges;
    item->cursed = 0;
    item->identified = 0;
    item->allowedSlots = allowedSlots;
    return 1;
}

int DM1_V1_Inventory_GetMouseItemPc34Compat(const DM1_V1_InventoryStatePc34* s, int champ, DM1_V1_ItemPc34* out) {
    if (!s || !out || champ < 0 || champ >= s->championCount) {
        return 0;
    }
    *out = s->champions[champ].mouseItem;
    return 1;
}

int DM1_V1_Inventory_SetItemInPc34SourceSlotCompat(DM1_V1_InventoryStatePc34* s, int champ,
                                               int pc34Slot, int itemType, int weight,
                                               int charges, int allowedSlots) {
    DM1_V1_ItemPc34* slot = dm1_v1_inventory_pc34_mutable_slot(s, champ, pc34Slot);
    if (!slot) {
        return 0;
    }
    slot->itemType = itemType;
    slot->weight = weight;
    slot->charges = charges;
    slot->cursed = 0;
    slot->identified = 0;
    slot->allowedSlots = allowedSlots;
    DM1_V1_Inventory_RecalcLoadPc34Compat(s, champ);
    return 1;
}

int DM1_V1_Inventory_GetItemInPc34SourceSlotCompat(const DM1_V1_InventoryStatePc34* s, int champ,
                                               int pc34Slot, DM1_V1_ItemPc34* out) {
    const DM1_V1_ItemPc34* slot = dm1_v1_inventory_pc34_const_slot(s, champ, pc34Slot);
    if (!slot || !out) {
        return 0;
    }
    *out = *slot;
    return 1;
}

int DM1_V1_Inventory_ClickPc34SourceSlotCompat(DM1_V1_InventoryStatePc34* s, int champ, int pc34Slot) {
    if (!s || champ < 0 || champ >= s->championCount) {
        return 0;
    }
    const int slotMask = DM1_V1_Inventory_Pc34SlotMaskCompat(pc34Slot);
    DM1_V1_ItemPc34* slot = dm1_v1_inventory_pc34_mutable_slot(s, champ, pc34Slot);
    if (!slot || slotMask == 0) {
        return 0;
    }
    DM1_V1_ChampionInventoryPc34* inv = &s->champions[champ];
    DM1_V1_ItemPc34 leaderHandObject = inv->mouseItem;
    DM1_V1_ItemPc34 slotObject = *slot;
    if (leaderHandObject.itemType == 0 && slotObject.itemType == 0) {
        return 0;
    }

    if (leaderHandObject.itemType != 0 &&
        !DM1_V1_Inventory_CanEquipPc34Compat(&leaderHandObject, pc34Slot)) {
        return 0;
    }

    /* ReDMCSB CHAMPION.C F0302 lines 688-710: remember leader hand and
     * selected slot, remove the occupied slot into leader hand, then add the
     * previous leader-hand object back to the same slot.  For C30+ chest slots
     * F0300 lines 511-515 and F0301 lines 606-610 route through
     * G0425_aT_ChestSlots rather than champion Slots[]. */
    if (slotObject.itemType != 0) {
        inv->mouseItem = slotObject;
    } else {
        dm1_v1_inventory_clear_item_pc34(&inv->mouseItem);
    }
    if (leaderHandObject.itemType != 0) {
        *slot = leaderHandObject;
    } else {
        dm1_v1_inventory_clear_item_pc34(slot);
    }
    DM1_V1_Inventory_RecalcLoadPc34Compat(s, champ);
    return 1;
}

const char *dm1_inventory_chest_stale_click_source_evidence_pc34(void)
{
    return
        "CHEST.C:31-43 F0333 ignores same chest, closes a different open chest, then writes G0426_T_OpenChest\n"
        "CHEST.C:113-121 F0334 rejects close when no G0426 chest is open, clears G0426_T_OpenChest, and erases G0425 slots\n"
        "CHAMPION.C:689-690 F0302 routes C30+ slot boxes through the current G0425_aT_ChestSlots entry only\n"
        "CHAMPION.C:694-710 F0302 rejects empty hand/empty slot, then performs the leader-hand/chest-slot swap";
}

int DM1_V1_Inventory_ClickOpenChestSlotForThingPc34Compat(DM1_V1_InventoryStatePc34* s, int champ,
                                                  int expectedOpenChestThing,
                                                  int chestSlotIndex) {
    if (!s || champ < 0 || champ >= s->championCount ||
        expectedOpenChestThing == 0 || chestSlotIndex < 0 ||
        chestSlotIndex >= DM1_PC34_CHEST_SLOT_COUNT) {
        return 0;
    }

    /* ReDMCSB CHEST.C F0333 lines 31-43 and F0334 lines 113-121 make
     * G0426_T_OpenChest the authority for the currently routed chest panel.
     * Reject stale C537..C544 clicks from a dismissed or replaced chest before
     * they can operate on the current G0425_aT_ChestSlots view. */
    if (s->champions[champ].openChestThing != expectedOpenChestThing) {
        return 0;
    }

    return DM1_V1_Inventory_ClickPc34SourceSlotCompat(
        s, champ, DM1_PC34_SLOT_CHEST_1 + chestSlotIndex);
}

int DM1_V1_Inventory_ResolveStatusHandSlotBoxPc34Compat(int slotBoxIndex,
                                               int partyChampionCount,
                                               int inventoryChampionOrdinal,
                                               int candidateChampionOrdinal,
                                               const int* championCurrentHealth,
                                               int* outChampionIndex,
                                               int* outPc34SourceSlot) {
    if (outChampionIndex) {
        *outChampionIndex = -1;
    }
    if (outPc34SourceSlot) {
        *outPc34SourceSlot = -1;
    }

    if (slotBoxIndex < 0 || slotBoxIndex >= 8 || partyChampionCount < 0 ||
        partyChampionCount > DM1_V1_MAX_CHAMPIONS_PC34 || !championCurrentHealth) {
        return 0;
    }

    if (candidateChampionOrdinal != 0) {
        return 0;
    }

    const int championIndex = slotBoxIndex >> 1;
    if (championIndex >= partyChampionCount) {
        return 0;
    }

    if (inventoryChampionOrdinal == championIndex + 1) {
        return 0;
    }

    if (championCurrentHealth[championIndex] <= 0) {
        return 0;
    }

    const int pc34SourceSlot = (slotBoxIndex & 1) ?
        DM1_PC34_SLOT_ACTION_HAND : DM1_PC34_SLOT_READY_HAND;

    if (outChampionIndex) {
        *outChampionIndex = championIndex;
    }
    if (outPc34SourceSlot) {
        *outPc34SourceSlot = pc34SourceSlot;
    }
    return 1;
}

int DM1_V1_Inventory_OpenChestPc34Compat(DM1_V1_InventoryStatePc34* s, int champ, int openChestThing,
                             const DM1_V1_ItemPc34* linkedItems, int linkedItemCount) {
    if (!s || champ < 0 || champ >= s->championCount || openChestThing == 0 ||
        linkedItemCount < 0 || (linkedItemCount > 0 && !linkedItems)) {
        return 0;
    }
    DM1_V1_ChampionInventoryPc34* inv = &s->champions[champ];
    /* ReDMCSB: CHEST.C F0333 line 28 sets G0424_i_PanelContent to
     * M569_PANEL_CHEST before the F0333 lines 31-32 same-open return.  For
     * PC 3.4, DEFS.H lines 3005-3008 define M569_PANEL_CHEST as 6. */
    s->panelContent = DM1_PC34_PANEL_CHEST;
    if (inv->openChestThing == openChestThing) {
        return 1;
    }
    inv->openChestThing = openChestThing;
    for (int i = 0; i < DM1_PC34_CHEST_SLOT_COUNT; i++) {
        dm1_v1_inventory_clear_item_pc34(&inv->chestSlots[i]);
    }
    const int limit = linkedItemCount < DM1_PC34_CHEST_SLOT_COUNT ?
        linkedItemCount : DM1_PC34_CHEST_SLOT_COUNT;
    for (int i = 0; i < limit; i++) {
        inv->chestSlots[i] = linkedItems[i];
    }
    DM1_V1_Inventory_RecalcLoadPc34Compat(s, champ);
    return 1;
}

int DM1_V1_Inventory_GetPanelContentPc34Compat(const DM1_V1_InventoryStatePc34* s) {
    if (!s) {
        return DM1_PC34_PANEL_INVENTORY;
    }
    return s->panelContent;
}

int DM1_V1_Inventory_ApplyPanelRouteAfterClosePc34Compat(DM1_V1_InventoryStatePc34* s,
                                                   int champ)
{
    if (!s || champ < 0 || champ >= s->championCount) {
        return 0;
    }

    /* ReDMCSB PANEL.C F0347 lines 1651-1691 redraws the inventory action-hand
     * route when close/click flows leave the chest panel: container action hands
     * keep CHEST panel content, otherwise status panel content returns to
     * food/water/poison fallback. */
    const DM1_V1_ItemPc34* actionHand =
        &s->champions[champ].slots[DM1_SLOT_HAND_LEFT];
    if (!actionHand->itemType ||
        (actionHand->allowedSlots & DM1_PC34_ALLOWED_CONTAINER) == 0) {
        s->panelContent = DM1_PC34_PANEL_FOOD_WATER_POISONED;
        return 1;
    }

    s->panelContent = DM1_PC34_PANEL_CHEST;
    return 1;
}

int DM1_V1_Inventory_SetPanelContentPc34Compat(DM1_V1_InventoryStatePc34* s,
                                         int panelContent) {
    if (!s) {
        return 0;
    }
    s->panelContent = panelContent;
    return 1;
}

int DM1_V1_Inventory_OpenChestReplacingCurrentPc34Compat(DM1_V1_InventoryStatePc34* s, int champ,
                                               int openChestThing,
                                               const DM1_V1_ItemPc34* linkedItems,
                                               int linkedItemCount,
                                               DM1_V1_ItemPc34* previousItemsOut,
                                               int maxPreviousItemsOut) {
    if (!s || champ < 0 || champ >= s->championCount || openChestThing == 0 ||
        linkedItemCount < 0 || (linkedItemCount > 0 && !linkedItems) ||
        maxPreviousItemsOut < 0 || (maxPreviousItemsOut > 0 && !previousItemsOut)) {
        return -1;
    }

    DM1_V1_ChampionInventoryPc34* inv = &s->champions[champ];
    /* ReDMCSB CHEST.C F0333 line 28 writes M569_PANEL_CHEST before the
     * same-open guard at lines 31-32 and before the different-chest close at
     * lines 35-38. */
    s->panelContent = DM1_PC34_PANEL_CHEST;
    if (inv->openChestThing == openChestThing) {
        return 0;
    }

    int previousCount = 0;
    /* ReDMCSB CHEST.C F0333 lines 34-39 closes a different G0426_T_OpenChest
     * through F0334 before F0333 lines 53-76 copies the requested container's
     * first eight links into G0425_aT_ChestSlots. */
    if (inv->openChestThing != 0) {
        previousCount = DM1_V1_Inventory_CloseChestPc34Compat(s, champ, previousItemsOut,
                                                  maxPreviousItemsOut);
        if (previousCount < 0) {
            return -1;
        }
    }

    if (!DM1_V1_Inventory_OpenChestPc34Compat(s, champ, openChestThing, linkedItems,
                                  linkedItemCount)) {
        return -1;
    }
    return previousCount;
}

int DM1_V1_Inventory_CloseChestPc34Compat(DM1_V1_InventoryStatePc34* s, int champ,
                              DM1_V1_ItemPc34* linkedItemsOut, int maxItemsOut) {
    if (!s || champ < 0 || champ >= s->championCount || maxItemsOut < 0 ||
        (maxItemsOut > 0 && !linkedItemsOut)) {
        return -1;
    }
    DM1_V1_ChampionInventoryPc34* inv = &s->champions[champ];
    if (inv->openChestThing == 0) {
        return 0;
    }
    int count = 0;
    for (int i = 0; i < DM1_PC34_CHEST_SLOT_COUNT; i++) {
        if (inv->chestSlots[i].itemType != 0) {
            if (count < maxItemsOut) {
                linkedItemsOut[count] = inv->chestSlots[i];
            }
            count++;
        }
        dm1_v1_inventory_clear_item_pc34(&inv->chestSlots[i]);
    }
    inv->openChestThing = 0;
    DM1_V1_Inventory_RecalcLoadPc34Compat(s, champ);
    return count;
}

int DM1_V1_Inventory_GetOpenChestThingPc34Compat(const DM1_V1_InventoryStatePc34* s, int champ) {
    if (!s || champ < 0 || champ >= s->championCount) {
        return 0;
    }
    return s->champions[champ].openChestThing;
}

int DM1_V1_Inventory_SetItemInChestSlotPc34Compat(DM1_V1_InventoryStatePc34* s, int champ, int chestSlotIndex,
                                         int itemType, int weight, int charges, int allowedSlots) {
    if (!s || champ < 0 || champ >= s->championCount || chestSlotIndex < 0 ||
        chestSlotIndex >= DM1_PC34_CHEST_SLOT_COUNT || s->champions[champ].openChestThing == 0) {
        return 0;
    }
    DM1_V1_ItemPc34* item = &s->champions[champ].chestSlots[chestSlotIndex];
    item->itemType = itemType;
    item->weight = weight;
    item->charges = charges;
    item->cursed = 0;
    item->identified = 0;
    item->allowedSlots = allowedSlots;
    DM1_V1_Inventory_RecalcLoadPc34Compat(s, champ);
    return 1;
}

int DM1_V1_Inventory_GetItemInChestSlotPc34Compat(const DM1_V1_InventoryStatePc34* s, int champ,
                                         int chestSlotIndex, DM1_V1_ItemPc34* out) {
    if (!s || !out || champ < 0 || champ >= s->championCount || chestSlotIndex < 0 ||
        chestSlotIndex >= DM1_PC34_CHEST_SLOT_COUNT || s->champions[champ].openChestThing == 0) {
        return 0;
    }
    *out = s->champions[champ].chestSlots[chestSlotIndex];
    return 1;
}

/* ══════════════════════════════════════════════════════════════════════
 * DM1 V1 equip/unequip slot validation
 *
 * DM1_V1_Inventory_CanEquipPc34Compat  — CHAMPION.C:694-699 F0302 AllowedSlots/SlotMasks rejection
 * DM1_V1_Inventory_EquipPc34Compat      — CHAMPION.C:587-660 F0301_AddObjectInSlot
 * DM1_V1_Inventory_UnequipPc34Compat   — CHAMPION.C:489-560 F0300_GetObjectRemovedFromSlot
 * ══════════════════════════════════════════════════════════════════════ */

/* dm1_v1_inventory_pc34_compat.c:DM1_V1_Inventory_CanEquipPc34Compat:1
 * Returns 1 if item->allowedSlots overlaps the slot mask for pc34Slot.
 * dm1_v1_inventory_pc34_compat.c:DM1_V1_Inventory_Pc34SlotMaskCompat:217
 * dm1_v1_inventory_pc34_compat.c:DM1_V1_Inventory_ClickPc34SourceSlotCompat:327 */
int DM1_V1_Inventory_CanEquipPc34Compat(const DM1_V1_ItemPc34* item, int pc34Slot) {
    if (!item || pc34Slot < 0 || pc34Slot >= DM1_PC34_SLOT_COUNT) {
        return 0;
    }
    const int slotMask = DM1_V1_Inventory_Pc34SlotMaskCompat(pc34Slot);
    if (slotMask == 0) {
        return 0;
    }
    /* ReDMCSB CHAMPION.C F0302 lines 694-699 rejects the leader-hand object
     * when AllowedSlots & DATA.C G0038_ai_Graphic562_SlotMasks[slot] is zero;
     * there is no special unrestricted case for an AllowedSlots value of 0. */
    return (item->allowedSlots & slotMask) != 0 ? 1 : 0;
}

int DM1_V1_Inventory_Pc34AppliesRabbitsFootLuckModifierCompat(const DM1_V1_ItemPc34* item,
                                                          int pc34Slot) {
    if (!item || item->itemType != DM1_PC34_ICON_JUNK_RABBITS_FOOT ||
        pc34Slot < 0 || pc34Slot >= DM1_PC34_SLOT_COUNT) {
        return 0;
    }
    /* ReDMCSB CHAMPION.C F0299 lines 343-346 gates C137 Rabbit's Foot luck
     * through P0624_ui_SlotIndex < C30_SLOT_CHEST_1; DEFS.H line 810 makes
     * C30 the first chest slot, so C30..C37 never apply the luck modifier. */
    return pc34Slot < DM1_PC34_SLOT_CHEST_1 ? 1 : 0;
}

int DM1_V1_Inventory_Pc34GetRabbitsFootLuckBonusCompat(const DM1_V1_InventoryStatePc34* s,
                                                   int champ) {
    if (!s || champ < 0 || champ >= s->championCount) {
        return 0;
    }
    int bonus = 0;
    for (int pc34Slot = 0; pc34Slot < DM1_PC34_INVENTORY_SLOT_COUNT; pc34Slot++) {
        const DM1_V1_ItemPc34* item = dm1_v1_inventory_pc34_const_slot(s, champ, pc34Slot);
        if (DM1_V1_Inventory_Pc34AppliesRabbitsFootLuckModifierCompat(item, pc34Slot)) {
            bonus += DM1_PC34_RABBITS_FOOT_LUCK_BONUS;
        }
    }
    return bonus;
}

/* dm1_v1_inventory_pc34_compat.c:DM1_V1_Inventory_EquipPc34Compat:1
 * Moves item from the leader hand (mouseItem) into body slot pc34Slot.
 * Checks that the mouse item is non-empty and can_equip the target slot.
 * If the slot already holds an item, the existing item returns to the
 * leader hand (standard swap).  Recalculates champion load on success.
 * dm1_v1_inventory_pc34_compat.c:DM1_V1_Inventory_CanEquipPc34Compat:1
 * dm1_v1_inventory_pc34_compat.c:dm1_v1_inventory_pc34_mutable_slot:193
 * dm1_v1_inventory_pc34_compat.c:DM1_V1_Inventory_ClickPc34SourceSlotCompat:327
 * dm1_v1_inventory_pc34_compat.c:DM1_V1_Inventory_RecalcLoadPc34Compat:155
 * dm1_v1_inventory_pc34_compat.c:dm1_v1_inventory_clear_item_pc34:42 */
int DM1_V1_Inventory_EquipPc34Compat(DM1_V1_InventoryStatePc34* s, int champ, int pc34Slot, const DM1_V1_ItemPc34* item) {
    if (!s || !item || champ < 0 || champ >= s->championCount ||
        pc34Slot < 0 || pc34Slot >= DM1_PC34_SLOT_COUNT) {
        return 0;
    }
    if (item->itemType == 0) {
        return 0;
    }
    if (!DM1_V1_Inventory_CanEquipPc34Compat(item, pc34Slot)) {
        return 0;
    }
    DM1_V1_ItemPc34* slot = dm1_v1_inventory_pc34_mutable_slot(s, champ, pc34Slot);
    if (!slot) {
        return 0;
    }
    DM1_V1_ChampionInventoryPc34* inv = &s->champions[champ];
    DM1_V1_ItemPc34 existing = *slot;
    if (existing.itemType != 0) {
        inv->mouseItem = existing;
    } else {
        dm1_v1_inventory_clear_item_pc34(&inv->mouseItem);
    }
    slot->itemType = item->itemType;
    slot->weight   = item->weight;
    slot->charges  = item->charges;
    slot->cursed   = item->cursed;
    slot->identified = item->identified;
    slot->allowedSlots = item->allowedSlots;
    DM1_V1_Inventory_RecalcLoadPc34Compat(s, champ);
    return 1;
}

/* dm1_v1_inventory_pc34_compat.c:DM1_V1_Inventory_UnequipPc34Compat:1
 * Moves the item currently in body slot pc34Slot back to the leader hand.
 * If the leader hand already holds an item the unequip fails (return 0)
 * so no data is lost.  Clears the slot and recalculates champion load.
 * dm1_v1_inventory_pc34_compat.c:dm1_v1_inventory_pc34_mutable_slot:193
 * dm1_v1_inventory_pc34_compat.c:DM1_V1_Inventory_RecalcLoadPc34Compat:155
 * dm1_v1_inventory_pc34_compat.c:dm1_v1_inventory_clear_item_pc34:42 */
int DM1_V1_Inventory_UnequipPc34Compat(DM1_V1_InventoryStatePc34* s, int champ, int pc34Slot) {
    if (!s || champ < 0 || champ >= s->championCount ||
        pc34Slot < 0 || pc34Slot >= DM1_PC34_SLOT_COUNT) {
        return 0;
    }
    DM1_V1_ItemPc34* slot = dm1_v1_inventory_pc34_mutable_slot(s, champ, pc34Slot);
    if (!slot) {
        return 0;
    }
    if (slot->itemType == 0) {
        return 0;
    }
    DM1_V1_ChampionInventoryPc34* inv = &s->champions[champ];
    if (inv->mouseItem.itemType != 0) {
        return 0;
    }
    inv->mouseItem = *slot;
    dm1_v1_inventory_clear_item_pc34(slot);
    DM1_V1_Inventory_RecalcLoadPc34Compat(s, champ);
    return 1;
}

/* ══════════════════════════════════════════════════════════════════════
 * Pass601 — Inventory system source-lock extensions
 *
 * CHAMPION.C:243-268  F0297_CHAMPION_PutObjectInLeaderHand
 * CHAMPION.C:270-298  F0298_CHAMPION_GetObjectRemovedFromLeaderHand
 * CHAMPION.C:301-487  F0299_CHAMPION_ApplyObjectModifiersToStatistics
 * CHAMPION.C:489-560  F0300_CHAMPION_GetObjectRemovedFromSlot
 * CHAMPION.C:587-660  F0301_CHAMPION_AddObjectInSlot
 * CHAMPION.C:662-712  F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox
 *   677-687: status hand slot boxes resolve to championIndex=(slot>>1)
 *            and hand slot=(slot&1); inventory panel slots subtract C08.
 *   BUG0_39: Food/Water panel flash when swapping scroll/chest in leader hand
 *   (F0300 sets MASK0x0800, F0297 triggers F0292 which draws panel prematurely)
 *
 * OBJECT.C:121-200    F0032_OBJECT_GetType (thing type extraction)
 * OBJECT.C:25-120     F0031_OBJECT_LoadNames (object name table)
 * CHEST.C:30-46       F0333 open chest id guard, close-other, open icon
 * CLIKCHAM.C:31-32    status-hand commands dispatch F0302 with command offset
 * CHEST.C:53-76       F0333 copies first 8 linked container things into G0425
 * CHEST.C:112-133     F0334 closes by compacting non-empty G0425 slots back to links
 * DEFS.H:778-817      C00..C37 inventory/backpack/chest slot namespace
 * DEFS.H:1937         C137_ICON_JUNK_RABBITS_FOOT
 * DATA.C:1049-1087    30 inventory slot masks + 8 chest container masks
 * ══════════════════════════════════════════════════════════════════════ */

const char *dm1_inventory_pass601_inventory_source_evidence(void)
{
    return
        "CHAMPION.C:243-268 F0297_PutObjectInLeaderHand\n"
        "CHAMPION.C:270-298 F0298_GetObjectRemovedFromLeaderHand\n"
        "CHAMPION.C:301-487 F0299_ApplyObjectModifiersToStatistics\n"
        "CHAMPION.C:489-560 F0300_GetObjectRemovedFromSlot\n"
        "CHAMPION.C:587-660 F0301_AddObjectInSlot\n"
        "CHAMPION.C:343-346 F0299 Rabbit's Foot luck ignores C30+ chest slots\n"
        "CHAMPION.C:694-699 F0302 empty-slot no-op and AllowedSlots/SlotMasks rejection\n"
        "CHAMPION.C:701-710 F0302 leader-hand/slot swap order\n"
        "CHAMPION.C:677-687 F0302 status hand slot routing gates\n"
        "DATA.C:1049-1087 G0038_ai_Graphic562_SlotMasks\n"
        "DEFS.H:778-817 C00..C37 inventory/backpack/chest slot index namespace\n"
        "DEFS.H:1937 C137_ICON_JUNK_RABBITS_FOOT\n"
        "DEFS.H:1874-1878 C08 slot-box split and M070_HAND_SLOT_INDEX\n"
        "DEFS.H:1698-1710 object allowed-slot masks\n"
        "CHEST.C:30-46 F0333 open chest guard/open icon\n"
        "CHEST.C:34-39 F0333 closes different open chest before replacement\n"
        "CHEST.C:53-76 F0333 first-8 chest slot copy\n"
        "CHEST.C:112-133 F0334 non-empty slot compact close\n"
        "CLIKCHAM.C:31-32 status box hand click dispatch\n"
        "CHAMPION.C:662-712 F0302_ProcessCommands28To65_ClickOnSlotBox BUG0_39\n"
        "OBJECT.C:121-200 F0032_OBJECT_GetType\n"
        "OBJECT.C:25-120 F0031_OBJECT_LoadNames\n";
}
