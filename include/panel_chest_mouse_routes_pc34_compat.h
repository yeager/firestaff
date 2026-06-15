#ifndef PANEL_CHEST_MOUSE_ROUTES_PC34_COMPAT_H
#define PANEL_CHEST_MOUSE_ROUTES_PC34_COMPAT_H

typedef struct PanelChestSlotRoutePc34Compat {
    unsigned int ordinal;
    unsigned int commandId;
    unsigned int zoneId;
    unsigned int slotBoxIndex;
    unsigned int chestSlotIndex;
    int viewportLeft;
    int viewportRight;
    int viewportTop;
    int viewportBottom;
    int panelLeft;
    int panelRight;
    int panelTop;
    int panelBottom;
    int width;
    int height;
} PanelChestSlotRoutePc34Compat;

const char* panel_chest_mouse_routes_GetEvidence(void);
unsigned int panel_chest_mouse_routes_GetSlotCount(void);
int panel_chest_mouse_routes_GetSlot(unsigned int ordinal,
                                     PanelChestSlotRoutePc34Compat* outSlot);
unsigned int panel_chest_mouse_routes_GetInvariant(void);
#endif
