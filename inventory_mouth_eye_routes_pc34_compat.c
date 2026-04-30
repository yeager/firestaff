#include "inventory_mouth_eye_routes_pc34_compat.h"

#define DM1_COMMAND_CLICK_ON_MOUTH_PC34_COMPAT 70u
#define DM1_COMMAND_CLICK_ON_EYE_PC34_COMPAT 71u

static const InventoryMouthEyeRoutePc34Compat kInventoryMouthEyeRoutes[] = {
    { 1u, DM1_COMMAND_CLICK_ON_MOUTH_PC34_COMPAT, 545u, "mouth", "F0349_INVENTORY_ProcessCommand70_ClickOnMouth", "COMMAND.C:426 maps C070 to C545_ZONE_MOUTH; COMMAND.C:2314-2316 dispatches C070 to F0349; PANEL.C:1788-1817 gates empty-hand mouth press and redraws food/water/poison panel." },
    { 2u, DM1_COMMAND_CLICK_ON_EYE_PC34_COMPAT, 546u, "eye", "F0352_INVENTORY_ProcessCommand71_ClickOnEye", "COMMAND.C:427 maps C071 to C546_ZONE_EYE; COMMAND.C:2318-2320 dispatches C071 to F0352; PANEL.C:2123-2159 draws looking-eye icon and routes to skills/statistics or leader-hand object description." }
};

unsigned int inventory_mouth_eye_routes_GetRouteCount(void) {
    return (unsigned int)(sizeof(kInventoryMouthEyeRoutes) / sizeof(kInventoryMouthEyeRoutes[0]));
}

int inventory_mouth_eye_routes_GetRoute(unsigned int ordinal, InventoryMouthEyeRoutePc34Compat* outRoute) {
    if (!outRoute || ordinal < 1u || ordinal > inventory_mouth_eye_routes_GetRouteCount()) return 0;
    *outRoute = kInventoryMouthEyeRoutes[ordinal - 1u];
    return 1;
}

InventoryMouthEyePanelResultPc34Compat inventory_mouth_eye_routes_EvaluatePress(InventoryMouthEyeInputPc34Compat input) {
    if (input.commandId == DM1_COMMAND_CLICK_ON_MOUTH_PC34_COMPAT) {
        if (!input.leaderEmptyHanded || input.panelAlreadyFoodWater) return INVENTORY_MOUTH_EYE_PANEL_RESULT_NONE_PC34_COMPAT;
        return INVENTORY_MOUTH_EYE_PANEL_RESULT_FOOD_WATER_POISONED_PC34_COMPAT;
    }
    if (input.commandId == DM1_COMMAND_CLICK_ON_EYE_PC34_COMPAT) {
        return input.leaderEmptyHanded ? INVENTORY_MOUTH_EYE_PANEL_RESULT_SKILLS_STATISTICS_PC34_COMPAT
                                       : INVENTORY_MOUTH_EYE_PANEL_RESULT_LEADER_HAND_OBJECT_DESCRIPTION_PC34_COMPAT;
    }
    return INVENTORY_MOUTH_EYE_PANEL_RESULT_NONE_PC34_COMPAT;
}

const char* inventory_mouth_eye_routes_GetEvidence(void) {
    return "ReDMCSB source lock: COMMAND.C:426-427 maps inventory secondary mouse input C070 mouth/C071 eye to viewport zones C545/C546; COMMAND.C:2314-2320 dispatches them to PANEL.C F0349/F0352; PANEL.C:1788-1817 and 2123-2159 define the resulting panel redraws.";
}

unsigned int inventory_mouth_eye_routes_GetInvariant(void) {
    InventoryMouthEyeRoutePc34Compat route;
    return inventory_mouth_eye_routes_GetRouteCount() == 2u &&
           inventory_mouth_eye_routes_GetRoute(1u, &route) && route.commandId == 70u && route.zoneIndex == 545u &&
           inventory_mouth_eye_routes_GetRoute(2u, &route) && route.commandId == 71u && route.zoneIndex == 546u &&
           inventory_mouth_eye_routes_EvaluatePress((InventoryMouthEyeInputPc34Compat){70u, 1u, 0u}) == INVENTORY_MOUTH_EYE_PANEL_RESULT_FOOD_WATER_POISONED_PC34_COMPAT &&
           inventory_mouth_eye_routes_EvaluatePress((InventoryMouthEyeInputPc34Compat){70u, 1u, 1u}) == INVENTORY_MOUTH_EYE_PANEL_RESULT_NONE_PC34_COMPAT &&
           inventory_mouth_eye_routes_EvaluatePress((InventoryMouthEyeInputPc34Compat){71u, 1u, 0u}) == INVENTORY_MOUTH_EYE_PANEL_RESULT_SKILLS_STATISTICS_PC34_COMPAT &&
           inventory_mouth_eye_routes_EvaluatePress((InventoryMouthEyeInputPc34Compat){71u, 0u, 0u}) == INVENTORY_MOUTH_EYE_PANEL_RESULT_LEADER_HAND_OBJECT_DESCRIPTION_PC34_COMPAT;
}
