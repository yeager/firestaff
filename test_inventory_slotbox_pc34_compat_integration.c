#include <stdio.h>
#include <string.h>
#include "inventory_slotbox_pc34_compat.h"

int main(void) {
    unsigned int i;
    int ok = 1;
    InventoryCompatSlotBox box;
    InventoryCompatSlotRoute route;
    printf("probe=firestaff_inventory_slotbox_source_audit\n");
    printf("slotBoxSourceEvidence=%s\n", INVENTORY_Compat_GetSlotBoxSourceEvidence());
    printf("inventorySlotBoxCount=%u\n", INVENTORY_Compat_GetInventorySlotBoxCount());
    printf("chestSlotBoxCount=%u\n", INVENTORY_Compat_GetChestSlotBoxCount());
    printf("totalSlotBoxCount=%u\n", INVENTORY_Compat_GetTotalSlotBoxCount());
    if (INVENTORY_Compat_GetInventorySlotBoxCount() != 30u) ok = 0;
    if (INVENTORY_Compat_GetChestSlotBoxCount() != 8u) ok = 0;
    if (INVENTORY_Compat_GetTotalSlotBoxCount() != 46u) ok = 0;
    for (i = 8u; i <= 37u; ++i) {
        if (!INVENTORY_Compat_GetInventorySlotBox(i, &box)) { ok = 0; continue; }
        printf("inventorySlotBox[%u]=slot:%u command:%u zone:%u name:%s mask:%s evidence:%s\n",
               i, box.slotIndex, box.commandId, box.zoneIndex, box.slotName, box.allowedMaskName, box.sourceLineEvidence);
        if (box.commandId != 20u + i || box.zoneIndex != 499u + i || box.slotIndex != i - 8u) ok = 0;
    }
    for (i = 38u; i <= 45u; ++i) {
        if (!INVENTORY_Compat_GetChestSlotBox(i, &box)) { ok = 0; continue; }
        printf("chestSlotBox[%u]=slot:%u command:%u zone:%u name:%s mask:%s evidence:%s\n",
               i, box.slotIndex, box.commandId, box.zoneIndex, box.slotName, box.allowedMaskName, box.sourceLineEvidence);
        if (box.commandId != 20u + i || box.zoneIndex != 499u + i || box.slotIndex != i - 8u) ok = 0;
    }
    if (!INVENTORY_Compat_GetInventorySlotBox(8u, &box) || strcmp(box.slotName, "READY_HAND") != 0 || box.zoneIndex != 507u) ok = 0;
    if (!INVENTORY_Compat_GetInventorySlotBox(37u, &box) || strcmp(box.slotName, "BACKPACK_LINE1_9") != 0 || box.zoneIndex != 536u) ok = 0;
    if (!INVENTORY_Compat_GetChestSlotBox(45u, &box) || strcmp(box.slotName, "CHEST_8") != 0 || box.zoneIndex != 544u) ok = 0;
    for (i = 20u; i <= 65u; ++i) {
        if (!INVENTORY_Compat_GetSlotRouteFromCommand(i, &route)) { ok = 0; continue; }
        if (i == 20u || i == 21u || i == 28u || i == 57u || i == 65u) {
            printf("slotRouteFromCommand[%u]=slotBox:%u championFromStatus:%u slot:%u inventoryChampion:%u chest:%u evidence:%s\n",
                   i, route.slotBoxIndex, route.championIndexFromStatusBox, route.slotIndex,
                   route.usesInventoryChampion, route.usesChestSlots, route.sourceLineEvidence);
        }
    }
    if (!INVENTORY_Compat_GetSlotRouteFromCommand(28u, &route) || route.slotBoxIndex != 8u || route.slotIndex != 0u || !route.usesInventoryChampion) ok = 0;
    if (!INVENTORY_Compat_GetSlotRouteFromCommand(65u, &route) || route.slotBoxIndex != 45u || route.slotIndex != 37u || !route.usesChestSlots) ok = 0;
    printf("inventorySlotBoxSourceAuditInvariantOk=%d\n", ok);
    return ok ? 0 : 1;
}
