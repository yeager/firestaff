#ifndef REDMCSB_INVENTORY_PANEL_ROUTES_PC34_COMPAT_H
#define REDMCSB_INVENTORY_PANEL_ROUTES_PC34_COMPAT_H

typedef struct InventoryPanelRouteCompat { unsigned int ordinal; unsigned int commandId; unsigned int zoneIndex; const char* name; const char* evidence; } InventoryPanelRouteCompat;
unsigned int INVENTORY_Compat_GetPanelRouteCount(void);
int INVENTORY_Compat_GetPanelRoute(unsigned int ordinal, InventoryPanelRouteCompat* outRoute);
const char* INVENTORY_Compat_GetPanelRouteEvidence(void);
#endif
