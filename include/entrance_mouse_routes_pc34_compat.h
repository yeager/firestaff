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
int ENTRANCE_Compat_DispatchMouseRouteCommand(int screenX, int screenY, unsigned int buttonMask);
/* Map a hit-tested point from the currently presented startup target back to
 * ReDMCSB's 320x200 source page.  V2/Custom may present that page through a
 * 640x400 or user-selected internal target; the entrance mouse table still
 * consumes the original source coordinates. */
int ENTRANCE_Compat_MapPresentedPointToSource(int presentedX,
                                              int presentedY,
                                              int presentedWidth,
                                              int presentedHeight,
                                              int sourceWidth,
                                              int sourceHeight,
                                              int* outSourceX,
                                              int* outSourceY);
const char* ENTRANCE_Compat_GetMouseRouteEvidence(void);

#endif
