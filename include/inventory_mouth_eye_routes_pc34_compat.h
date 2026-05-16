#ifndef INVENTORY_MOUTH_EYE_ROUTES_PC34_COMPAT_H
#define INVENTORY_MOUTH_EYE_ROUTES_PC34_COMPAT_H

typedef enum InventoryMouthEyePanelResultPc34Compat {
    INVENTORY_MOUTH_EYE_PANEL_RESULT_NONE_PC34_COMPAT = 0,
    INVENTORY_MOUTH_EYE_PANEL_RESULT_FOOD_WATER_POISONED_PC34_COMPAT = 1,
    INVENTORY_MOUTH_EYE_PANEL_RESULT_SKILLS_STATISTICS_PC34_COMPAT = 2,
    INVENTORY_MOUTH_EYE_PANEL_RESULT_LEADER_HAND_OBJECT_DESCRIPTION_PC34_COMPAT = 3
} InventoryMouthEyePanelResultPc34Compat;

typedef struct InventoryMouthEyeRoutePc34Compat {
    unsigned int ordinal;
    unsigned int commandId;
    unsigned int zoneIndex;
    const char* name;
    const char* handler;
    const char* evidence;
} InventoryMouthEyeRoutePc34Compat;

typedef struct InventoryMouthEyeInputPc34Compat {
    unsigned int commandId;
    unsigned int leaderEmptyHanded;
    unsigned int panelAlreadyFoodWater;
} InventoryMouthEyeInputPc34Compat;

unsigned int inventory_mouth_eye_routes_GetRouteCount(void);
int inventory_mouth_eye_routes_GetRoute(unsigned int ordinal, InventoryMouthEyeRoutePc34Compat* outRoute);
InventoryMouthEyePanelResultPc34Compat inventory_mouth_eye_routes_EvaluatePress(InventoryMouthEyeInputPc34Compat input);
const char* inventory_mouth_eye_routes_GetEvidence(void);
unsigned int inventory_mouth_eye_routes_GetInvariant(void);

#endif
