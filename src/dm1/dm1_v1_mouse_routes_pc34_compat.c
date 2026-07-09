#include "dm1_v1_mouse_routes_pc34_compat.h"

#include <stddef.h>

static const DM1_V1_MouseRoutePc34Compat kInterfaceRoutes[] = {
    /* ReDMCSB COMMAND.C G0447 plus focused G0455 status-hand rows. */
    { 20,  DM1_V1_MOUSE_SPACE_SCREEN_PC34, 211, DM1_V1_MOUSE_MASK_LEFT_PC34  },
    { 21,  DM1_V1_MOUSE_SPACE_SCREEN_PC34, 212, DM1_V1_MOUSE_MASK_LEFT_PC34  },
    { 22,  DM1_V1_MOUSE_SPACE_SCREEN_PC34, 213, DM1_V1_MOUSE_MASK_LEFT_PC34  },
    { 23,  DM1_V1_MOUSE_SPACE_SCREEN_PC34, 214, DM1_V1_MOUSE_MASK_LEFT_PC34  },
    { 24,  DM1_V1_MOUSE_SPACE_SCREEN_PC34, 215, DM1_V1_MOUSE_MASK_LEFT_PC34  },
    { 25,  DM1_V1_MOUSE_SPACE_SCREEN_PC34, 216, DM1_V1_MOUSE_MASK_LEFT_PC34  },
    { 26,  DM1_V1_MOUSE_SPACE_SCREEN_PC34, 217, DM1_V1_MOUSE_MASK_LEFT_PC34  },
    { 27,  DM1_V1_MOUSE_SPACE_SCREEN_PC34, 218, DM1_V1_MOUSE_MASK_LEFT_PC34  },
    { 7,   DM1_V1_MOUSE_SPACE_SCREEN_PC34, 151, DM1_V1_MOUSE_MASK_RIGHT_PC34 },
    { 8,   DM1_V1_MOUSE_SPACE_SCREEN_PC34, 152, DM1_V1_MOUSE_MASK_RIGHT_PC34 },
    { 9,   DM1_V1_MOUSE_SPACE_SCREEN_PC34, 153, DM1_V1_MOUSE_MASK_RIGHT_PC34 },
    { 10,  DM1_V1_MOUSE_SPACE_SCREEN_PC34, 154, DM1_V1_MOUSE_MASK_RIGHT_PC34 },
    { 7,   DM1_V1_MOUSE_SPACE_SCREEN_PC34, 187, DM1_V1_MOUSE_MASK_LEFT_PC34  },
    { 8,   DM1_V1_MOUSE_SPACE_SCREEN_PC34, 188, DM1_V1_MOUSE_MASK_LEFT_PC34  },
    { 9,   DM1_V1_MOUSE_SPACE_SCREEN_PC34, 189, DM1_V1_MOUSE_MASK_LEFT_PC34  },
    { 10,  DM1_V1_MOUSE_SPACE_SCREEN_PC34, 190, DM1_V1_MOUSE_MASK_LEFT_PC34  },
    { 12,  DM1_V1_MOUSE_SPACE_SCREEN_PC34, 151, DM1_V1_MOUSE_MASK_LEFT_PC34  },
    { 13,  DM1_V1_MOUSE_SPACE_SCREEN_PC34, 152, DM1_V1_MOUSE_MASK_LEFT_PC34  },
    { 14,  DM1_V1_MOUSE_SPACE_SCREEN_PC34, 153, DM1_V1_MOUSE_MASK_LEFT_PC34  },
    { 15,  DM1_V1_MOUSE_SPACE_SCREEN_PC34, 154, DM1_V1_MOUSE_MASK_LEFT_PC34  },
    { 125, DM1_V1_MOUSE_SPACE_SCREEN_PC34, 113, DM1_V1_MOUSE_MASK_LEFT_PC34  },
    { 126, DM1_V1_MOUSE_SPACE_SCREEN_PC34, 114, DM1_V1_MOUSE_MASK_LEFT_PC34  },
    { 127, DM1_V1_MOUSE_SPACE_SCREEN_PC34, 115, DM1_V1_MOUSE_MASK_LEFT_PC34  },
    { 128, DM1_V1_MOUSE_SPACE_SCREEN_PC34, 116, DM1_V1_MOUSE_MASK_LEFT_PC34  },
    { 100, DM1_V1_MOUSE_SPACE_SCREEN_PC34,  13, DM1_V1_MOUSE_MASK_LEFT_PC34  },
    { 111, DM1_V1_MOUSE_SPACE_SCREEN_PC34,  11, DM1_V1_MOUSE_MASK_LEFT_PC34  }
};

static const DM1_V1_MouseRoutePc34Compat kMovementRoutes[] = {
    /* ReDMCSB COMMAND.C G0448_as_Graphic561_SecondaryMouseInput_Movement. */
    { 1,  DM1_V1_MOUSE_SPACE_SCREEN_PC34, 68, DM1_V1_MOUSE_MASK_LEFT_PC34  },
    { 3,  DM1_V1_MOUSE_SPACE_SCREEN_PC34, 70, DM1_V1_MOUSE_MASK_LEFT_PC34  },
    { 2,  DM1_V1_MOUSE_SPACE_SCREEN_PC34, 69, DM1_V1_MOUSE_MASK_LEFT_PC34  },
    { 6,  DM1_V1_MOUSE_SPACE_SCREEN_PC34, 73, DM1_V1_MOUSE_MASK_LEFT_PC34  },
    { 5,  DM1_V1_MOUSE_SPACE_SCREEN_PC34, 72, DM1_V1_MOUSE_MASK_LEFT_PC34  },
    { 4,  DM1_V1_MOUSE_SPACE_SCREEN_PC34, 71, DM1_V1_MOUSE_MASK_LEFT_PC34  },
    { 80, DM1_V1_MOUSE_SPACE_SCREEN_PC34, 7,  DM1_V1_MOUSE_MASK_LEFT_PC34  },
    { 83, DM1_V1_MOUSE_SPACE_SCREEN_PC34, 2,  DM1_V1_MOUSE_MASK_RIGHT_PC34 }
};

static const DM1_V1_MouseRoutePc34Compat kInventoryRoutes[] = {
    /* ReDMCSB COMMAND.C G0449, G0456; CLIKCHAM.C F0367; CHAMPION.C F0302. */
    { 20, DM1_V1_MOUSE_SPACE_SCREEN_PC34,   211, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 21, DM1_V1_MOUSE_SPACE_SCREEN_PC34,   212, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 22, DM1_V1_MOUSE_SPACE_SCREEN_PC34,   213, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 23, DM1_V1_MOUSE_SPACE_SCREEN_PC34,   214, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 24, DM1_V1_MOUSE_SPACE_SCREEN_PC34,   215, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 25, DM1_V1_MOUSE_SPACE_SCREEN_PC34,   216, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 26, DM1_V1_MOUSE_SPACE_SCREEN_PC34,   217, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 27, DM1_V1_MOUSE_SPACE_SCREEN_PC34,   218, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 11, DM1_V1_MOUSE_SPACE_SCREEN_PC34,   2,   DM1_V1_MOUSE_MASK_RIGHT_PC34 },
    { 28, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 507, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 29, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 508, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 30, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 509, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 31, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 510, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 32, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 511, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 33, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 512, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 34, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 513, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 70, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 545, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 71, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 546, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 35, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 514, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 36, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 515, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 37, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 516, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 38, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 517, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 39, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 518, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 40, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 519, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 41, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 520, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 42, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 521, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 43, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 522, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 44, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 523, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 45, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 524, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 46, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 525, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 47, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 526, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 48, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 527, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 49, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 528, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 50, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 529, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 51, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 530, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 52, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 531, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 53, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 532, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 54, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 533, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 55, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 534, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 56, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 535, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 57, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 536, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 58, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 537, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 59, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 538, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 60, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 539, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 61, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 540, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 62, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 541, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 63, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 542, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 64, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 543, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 65, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 544, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 81, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 101, DM1_V1_MOUSE_MASK_LEFT_PC34 }
};

static int dm1_v1_mouse_routes_for_list(
    int mouseInputList,
    const DM1_V1_MouseRoutePc34Compat** outRoutes) {
    if (outRoutes) *outRoutes = NULL;
    switch (mouseInputList) {
        case DM1_V1_MOUSE_LIST_INTERFACE_PC34:
            if (outRoutes) *outRoutes = kInterfaceRoutes;
            return (int)(sizeof(kInterfaceRoutes) / sizeof(kInterfaceRoutes[0]));
        case DM1_V1_MOUSE_LIST_MOVEMENT_PC34:
            if (outRoutes) *outRoutes = kMovementRoutes;
            return (int)(sizeof(kMovementRoutes) / sizeof(kMovementRoutes[0]));
        case DM1_V1_MOUSE_LIST_INVENTORY_PC34:
            if (outRoutes) *outRoutes = kInventoryRoutes;
            return (int)(sizeof(kInventoryRoutes) / sizeof(kInventoryRoutes[0]));
        default:
            return 0;
    }
}

static int dm1_v1_mouse_rect_contains_inclusive(int x,
                                                int y,
                                                int rx,
                                                int ry,
                                                int rw,
                                                int rh) {
    return rw > 0 && rh > 0 && x >= rx && y >= ry &&
           x <= rx + rw - 1 && y <= ry + rh - 1;
}

const char* DM1_V1_MouseRoutes_SourceEvidencePc34Compat(void) {
    return "ReDMCSB COMMAND.C G0447/G0448/G0449/G0456; CLIKCHAM.C F0367; "
           "CHAMPION.C F0302; DEFS.H C068..C073/C101/C113..C116/"
           "C151..C154/C187..C190/C211..C218/C507..C546";
}

int DM1_V1_MouseRoutes_GetRouteCountPc34Compat(int mouseInputList) {
    return dm1_v1_mouse_routes_for_list(mouseInputList, NULL);
}

int DM1_V1_MouseRoutes_GetRoutePc34Compat(int mouseInputList,
                                          int index,
                                          DM1_V1_MouseRoutePc34Compat* outRoute) {
    const DM1_V1_MouseRoutePc34Compat* routes = NULL;
    int count = dm1_v1_mouse_routes_for_list(mouseInputList, &routes);
    if (!outRoute || index < 0 || index >= count || !routes) {
        return 0;
    }
    *outRoute = routes[index];
    return 1;
}

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
    int* outZoneId) {
    const DM1_V1_MouseRoutePc34Compat* routes = NULL;
    int count = dm1_v1_mouse_routes_for_list(mouseInputList, &routes);
    int i;

    if (outCoordinateSpace) *outCoordinateSpace = DM1_V1_MOUSE_SPACE_NONE_PC34;
    if (outZoneId) *outZoneId = 0;
    if (!routes || !zoneRectFn) {
        return 0;
    }

    for (i = 0; i < count; ++i) {
        int x;
        int y;
        int w;
        int h;
        int testX = screenX;
        int testY = screenY;
        const DM1_V1_MouseRoutePc34Compat* route = &routes[i];
        if (!(buttonMask & route->buttonMask)) {
            continue;
        }
        if (!zoneRectFn(route->zoneId, user, &x, &y, &w, &h)) {
            continue;
        }
        if (route->coordinateSpace == DM1_V1_MOUSE_SPACE_VIEWPORT_PC34) {
            testX -= viewportX;
            testY -= viewportY;
        }
        if (dm1_v1_mouse_rect_contains_inclusive(testX, testY, x, y, w, h)) {
            if (outCoordinateSpace) *outCoordinateSpace = route->coordinateSpace;
            if (outZoneId) *outZoneId = route->zoneId;
            return route->command;
        }
    }
    return 0;
}
