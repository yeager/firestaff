#ifndef REDMCSB_ENTRANCE_MOUSE_ROUTES_PC34_COMPAT_H
#define REDMCSB_ENTRANCE_MOUSE_ROUTES_PC34_COMPAT_H

typedef enum EntranceMouseButtonMaskCompat {
    ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT = 0x0002u,
    ENTRANCE_MOUSE_BUTTON_BONUS_DUNGEON_COMPAT = 0x0010u
} EntranceMouseButtonMaskCompat;

typedef struct EntranceMouseRouteCompat {
    unsigned int ordinal;
    unsigned int commandId;
    unsigned int zoneIndex;
    unsigned int buttonMask;
    int x;
    int y;
    int w;
    int h;
    const char* name;
    const char* evidence;
} EntranceMouseRouteCompat;

unsigned int ENTRANCE_Compat_GetMouseRouteCount(void);
int ENTRANCE_Compat_GetMouseRoute(unsigned int ordinal, EntranceMouseRouteCompat* outRoute);
int ENTRANCE_Compat_HitTestMouseRoute(int screenX, int screenY, unsigned int buttonMask, EntranceMouseRouteCompat* outRoute);
const char* ENTRANCE_Compat_GetMouseRouteEvidence(void);

#endif
