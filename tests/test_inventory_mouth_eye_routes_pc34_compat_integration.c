#include <stdio.h>
#include <string.h>
#include "inventory_mouth_eye_routes_pc34_compat.h"

static int expect_route(unsigned int ordinal, unsigned int commandId, unsigned int zoneIndex,
                        const char* name, const char* handler) {
    InventoryMouthEyeRoutePc34Compat route;
    if (!inventory_mouth_eye_routes_GetRoute(ordinal, &route)) return 0;
    return route.ordinal == ordinal && route.commandId == commandId && route.zoneIndex == zoneIndex &&
           strcmp(route.name, name) == 0 && strcmp(route.handler, handler) == 0;
}

static int expect_press(InventoryMouthEyeInputPc34Compat input,
                        InventoryMouthEyePanelResultPc34Compat expected) {
    return inventory_mouth_eye_routes_EvaluatePress(input) == expected;
}

int main(void) {
    InventoryMouthEyeRoutePc34Compat route;
    unsigned int i;
    int ok = 1;
    printf("probe=firestaff_inventory_mouth_eye_routes\n");
    printf("sourceEvidence=%s\n", inventory_mouth_eye_routes_GetEvidence());
    printf("inventoryMouthEyeRouteCount=%u\n", inventory_mouth_eye_routes_GetRouteCount());

    if (inventory_mouth_eye_routes_GetRouteCount() != 2u) ok = 0;
    if (!expect_route(1u, 70u, 545u, "mouth", "F0349_INVENTORY_ProcessCommand70_ClickOnMouth")) ok = 0;
    if (!expect_route(2u, 71u, 546u, "eye", "F0352_INVENTORY_ProcessCommand71_ClickOnEye")) ok = 0;
    if (inventory_mouth_eye_routes_GetRoute(0u, &route)) ok = 0;
    if (inventory_mouth_eye_routes_GetRoute(3u, &route)) ok = 0;

    if (!expect_press((InventoryMouthEyeInputPc34Compat){70u, 1u, 0u}, INVENTORY_MOUTH_EYE_PANEL_RESULT_FOOD_WATER_POISONED_PC34_COMPAT)) ok = 0;
    if (!expect_press((InventoryMouthEyeInputPc34Compat){70u, 1u, 1u}, INVENTORY_MOUTH_EYE_PANEL_RESULT_NONE_PC34_COMPAT)) ok = 0;
    if (!expect_press((InventoryMouthEyeInputPc34Compat){70u, 0u, 0u}, INVENTORY_MOUTH_EYE_PANEL_RESULT_NONE_PC34_COMPAT)) ok = 0;
    if (!expect_press((InventoryMouthEyeInputPc34Compat){71u, 1u, 0u}, INVENTORY_MOUTH_EYE_PANEL_RESULT_SKILLS_STATISTICS_PC34_COMPAT)) ok = 0;
    if (!expect_press((InventoryMouthEyeInputPc34Compat){71u, 0u, 0u}, INVENTORY_MOUTH_EYE_PANEL_RESULT_LEADER_HAND_OBJECT_DESCRIPTION_PC34_COMPAT)) ok = 0;

    for (i = 1u; i <= inventory_mouth_eye_routes_GetRouteCount(); ++i) {
        if (inventory_mouth_eye_routes_GetRoute(i, &route)) {
            printf("inventoryMouthEyeRoute[%u]=command:%u zone:%u name:%s handler:%s evidence:%s\n",
                   i, route.commandId, route.zoneIndex, route.name, route.handler, route.evidence);
        }
    }
    printf("inventoryMouthEyeRoutesInvariantOk=%u\n", (ok && inventory_mouth_eye_routes_GetInvariant()) ? 1u : 0u);
    return (ok && inventory_mouth_eye_routes_GetInvariant()) ? 0 : 1;
}
