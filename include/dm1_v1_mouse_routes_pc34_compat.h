#ifndef FIRESTAFF_DM1_V1_MOUSE_ROUTES_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_MOUSE_ROUTES_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_MOUSE_MASK_RIGHT_PC34 = 0x0001,
    DM1_V1_MOUSE_MASK_LEFT_PC34 = 0x0002
};

enum {
    DM1_V1_MOUSE_SPACE_NONE_PC34 = 0,
    DM1_V1_MOUSE_SPACE_SCREEN_PC34 = 1,
    DM1_V1_MOUSE_SPACE_VIEWPORT_PC34 = 2
};

enum {
    DM1_V1_MOUSE_LIST_INTERFACE_PC34 = 1,
    DM1_V1_MOUSE_LIST_MOVEMENT_PC34 = 2,
    DM1_V1_MOUSE_LIST_INVENTORY_PC34 = 3,
    /* COMMAND.C G0450 replaces normal input while the party is resting. */
    DM1_V1_MOUSE_LIST_PARTY_RESTING_PC34 = 4
};

typedef struct {
    int command;
    int coordinateSpace;
    int zoneId;
    int buttonMask;
} DM1_V1_MouseRoutePc34Compat;

typedef int (*DM1_V1_MouseRouteZoneRectPc34CompatFn)(int zoneId,
                                                     void* user,
                                                     int* outX,
                                                     int* outY,
                                                     int* outW,
                                                     int* outH);

const char* DM1_V1_MouseRoutes_SourceEvidencePc34Compat(void);
int DM1_V1_MouseRoutes_GetRouteCountPc34Compat(int mouseInputList);
int DM1_V1_MouseRoutes_GetRoutePc34Compat(int mouseInputList,
                                          int index,
                                          DM1_V1_MouseRoutePc34Compat* outRoute);
int DM1_V1_MouseRoutes_CommandForPointPc34Compat(
    int mouseInputList,
    int screenX,
    int screenY,
    int buttonMask,
    int viewportX,
    int viewportY,
    DM1_V1_MouseRouteZoneRectPc34CompatFn zoneRectFn,
    void* user,
    int* outCoordinateSpace,
    int* outZoneId);
int DM1_V1_MouseRoutes_CommandForScreenPointPc34Compat(
    int mouseInputList,
    int screenX,
    int screenY,
    int buttonMask,
    int* outCoordinateSpace,
    int* outZoneId);

#ifdef __cplusplus
}
#endif

#endif
