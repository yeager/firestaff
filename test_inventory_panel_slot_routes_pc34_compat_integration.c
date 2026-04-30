#include <stdio.h>
#include <string.h>
#include "inventory_panel_slot_routes_pc34_compat.h"

static int expect_slot(unsigned int index, unsigned int slotBox, unsigned int command,
                       unsigned int zone, const char* sourceName) {
    InventorySlotRoutePc34Compat route;
    if (!INVENTORY_Compat_GetInventorySlotRoute(index, &route)) return 0;
    return route.inventorySlotIndex == index && route.slotBoxIndex == slotBox &&
           route.commandId == command && route.zoneIndex == zone &&
           strcmp(route.sourceName, sourceName) == 0;
}

static int expect_panel(InventoryPanelInputPc34Compat input,
                        InventoryPanelRoutePc34Compat route) {
    return INVENTORY_Compat_EvaluatePanelRoute(input) == route;
}

int main(void) {
    int ok = 1;
    printf("probe=firestaff_inventory_panel_slot_routes\n");
    printf("sourceEvidence=%s\n", INVENTORY_Compat_GetPanelSlotRouteEvidence());
    printf("inventorySlotRouteCount=%u\n", INVENTORY_Compat_GetInventorySlotRouteCount());

    if (INVENTORY_Compat_GetInventorySlotRouteCount() != 30u) ok = 0;
    if (!expect_slot(0u, 8u, 28u, 507u, "ready hand")) ok = 0;
    if (!expect_slot(1u, 9u, 29u, 508u, "action hand")) ok = 0;
    if (!expect_slot(10u, 18u, 38u, 517u, "neck")) ok = 0;
    if (!expect_slot(29u, 37u, 57u, 536u, "backpack line1 9")) ok = 0;

    if (!expect_panel((InventoryPanelInputPc34Compat){0u, 1u, 0u, 0u, 9u},
                      INVENTORY_PANEL_ROUTE_FOOD_WATER_POISONED_PC34_COMPAT)) ok = 0;
    if (!expect_panel((InventoryPanelInputPc34Compat){0u, 0u, 1u, 1u, 0u},
                      INVENTORY_PANEL_ROUTE_SKILLS_STATISTICS_PC34_COMPAT)) ok = 0;
    if (!expect_panel((InventoryPanelInputPc34Compat){0u, 0u, 1u, 0u, 0u},
                      INVENTORY_PANEL_ROUTE_LEADER_HAND_OBJECT_PC34_COMPAT)) ok = 0;
    if (!expect_panel((InventoryPanelInputPc34Compat){1u, 0u, 0u, 0u, 0u},
                      INVENTORY_PANEL_ROUTE_RESURRECT_REINCARNATE_PC34_COMPAT)) ok = 0;
    if (!expect_panel((InventoryPanelInputPc34Compat){0u, 0u, 0u, 0u, 9u},
                      INVENTORY_PANEL_ROUTE_CHEST_OBJECT_PC34_COMPAT)) ok = 0;
    if (!expect_panel((InventoryPanelInputPc34Compat){0u, 0u, 0u, 0u, 7u},
                      INVENTORY_PANEL_ROUTE_SCROLL_OBJECT_PC34_COMPAT)) ok = 0;

    printf("inventoryPanelSlotRoutesInvariantOk=%u\n", ok ? 1u : 0u);
    return ok ? 0 : 1;
}
