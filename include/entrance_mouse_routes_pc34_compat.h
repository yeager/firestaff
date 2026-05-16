#ifndef REDMCSB_ENTRANCE_MOUSE_ROUTES_PC34_COMPAT_H
#define REDMCSB_ENTRANCE_MOUSE_ROUTES_PC34_COMPAT_H

typedef struct EntranceMouseRouteCompat { unsigned int ordinal; unsigned int commandId; unsigned int zoneIndex; const char* name; const char* evidence; } EntranceMouseRouteCompat;
unsigned int ENTRANCE_Compat_GetMouseRouteCount(void);
int ENTRANCE_Compat_GetMouseRoute(unsigned int ordinal, EntranceMouseRouteCompat* outRoute);
const char* ENTRANCE_Compat_GetMouseRouteEvidence(void);
#endif
