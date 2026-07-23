#include "dm1_v1_mouse_routes_pc34_compat.h"
#include "champion_status_slotbox_pc34_compat.h"
#include "dm1_v1_champion_status_layout_pc34_compat.h"
#include "dm1_v1_inventory_slot_placement_pc34_compat.h"
#include "dm1_v1_layout_zones_pc34_compat.h"
#include "firestaff/dm1/v1/box_action_area_pc34_compat.h"
#include "firestaff/dm1/v1/box_movement_arrows_pc34_compat.h"
#include "firestaff/dm1/v1/box_spell_area_pc34_compat.h"

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
    { 81, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 101, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    /* COMMAND.C:413-417 inventory control icons.  These are viewport
     * relative even though their source boxes sit above the dungeon view. */
    { 140, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 562, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 145, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 564, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 11,  DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 566, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 141, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 565, DM1_V1_MOUSE_MASK_LEFT_PC34 }
};

static const DM1_V1_MouseRoutePc34Compat kPanelChestRoutes[] = {
    /* ReDMCSB COMMAND.C G0456. F0378 reaches this list only after C081
     * has established that M569_PANEL_CHEST owns the C101 panel click. */
    { 58, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 537, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 59, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 538, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 60, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 539, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 61, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 540, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 62, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 541, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 63, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 542, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 64, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 543, DM1_V1_MOUSE_MASK_LEFT_PC34 },
    { 65, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 544, DM1_V1_MOUSE_MASK_LEFT_PC34 }
};

static const DM1_V1_MouseRoutePc34Compat kPartyRestingRoutes[] = {
    /* ReDMCSB COMMAND.C G0450.  The active input tables are replaced while
     * G0300_B_PartyIsResting is set: only these real source surfaces wake it. */
    /* C007's COORD.C rectangle is in screen coordinates (0,33,224,136),
     * unlike the inventory sub-zones that are relative to the viewport. */
    { 146, DM1_V1_MOUSE_SPACE_SCREEN_PC34,   7, DM1_V1_MOUSE_MASK_LEFT_PC34  },
    { 146, DM1_V1_MOUSE_SPACE_SCREEN_PC34,   2, DM1_V1_MOUSE_MASK_RIGHT_PC34 }
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
        case DM1_V1_MOUSE_LIST_PARTY_RESTING_PC34:
            if (outRoutes) *outRoutes = kPartyRestingRoutes;
            return (int)(sizeof(kPartyRestingRoutes) /
                         sizeof(kPartyRestingRoutes[0]));
        case DM1_V1_MOUSE_LIST_PANEL_CHEST_PC34:
            if (outRoutes) *outRoutes = kPanelChestRoutes;
            return (int)(sizeof(kPanelChestRoutes) /
                         sizeof(kPanelChestRoutes[0]));
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
    return "ReDMCSB COMMAND.C G0447/G0448/G0449/G0450/G0456; F0378; CLIKCHAM.C F0367; "
           "CHAMPION.C F0302; DEFS.H C068..C073/C101/C113..C116/"
           "C146/C151..C154/C187..C190/C211..C218/C507..C546";
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

static int dm1_v1_mouse_route_zone_rect_pc34(int zoneId,
                                             void* user,
                                             int* outX,
                                             int* outY,
                                             int* outW,
                                             int* outH) {
    (void)user;
    switch (zoneId) {
        case 2:
        {
            DM1_V1_LayoutZoneRectPc34 rect = dm1_v1_screen_rect_pc34();
            if (!dm1_v1_screen_zone_id_pc34()) return 0;
            if (outX) *outX = rect.x;
            if (outY) *outY = rect.y;
            if (outW) *outW = rect.w;
            if (outH) *outH = rect.h;
            return 1;
        }
        case 7:
        {
            DM1_V1_LayoutZoneRectPc34 rect = dm1_v1_viewport_rect_pc34();
            if (!dm1_v1_viewport_zone_id_pc34()) return 0;
            if (outX) *outX = rect.x;
            if (outY) *outY = rect.y;
            if (outW) *outW = rect.w;
            if (outH) *outH = rect.h;
            return 1;
        }
        case 11:
        {
            DM1_V1_ActionAreaRectPc34 rect = dm1_v1_action_area_rect_pc34();
            if (!dm1_v1_action_area_zone_id_pc34()) return 0;
            if (outX) *outX = rect.x;
            if (outY) *outY = rect.y;
            if (outW) *outW = rect.w;
            if (outH) *outH = rect.h;
            return 1;
        }
        case 13:
        {
            DM1_V1_SpellAreaRectPc34 rect = dm1_v1_spell_area_click_rect_pc34();
            if (outX) *outX = rect.x;
            if (outY) *outY = rect.y;
            if (outW) *outW = rect.w;
            if (outH) *outH = rect.h;
            return 1;
        }
        default:
            break;
    }

    if (zoneId >= 151 && zoneId <= 154) {
        DM1_V1_ChampionStatusRectPc34 rect;
        if (!dm1_v1_champion_status_box_rect_pc34(zoneId - 151, &rect)) {
            return 0;
        }
        if (outX) *outX = rect.x;
        if (outY) *outY = rect.y;
        if (outW) *outW = rect.w;
        if (outH) *outH = rect.h;
        return 1;
    }
    if (zoneId >= 211 && zoneId <= 218) {
        const int slotBox = zoneId - 211;
        const int championSlot = slotBox >> 1;
        const int handSlot = slotBox & 1;
        DM1_V1_ChampionStatusRectPc34 rect;
        if (!dm1_v1_champion_status_hand_rect_pc34(championSlot, handSlot,
                                                   &rect)) {
            return 0;
        }
        if (outX) *outX = rect.x;
        if (outY) *outY = rect.y;
        if (outW) *outW = rect.w;
        if (outH) *outH = rect.h;
        return 1;
    }
    if (zoneId >= 187 && zoneId <= 190) {
        ChampionStatusRectCompat rect;
        if (!CHAMPION_Compat_StatusBarGraphRegionZone(zoneId - 187,
                                                      &rect)) return 0;
        if (outX) *outX = rect.x;
        if (outY) *outY = rect.y;
        if (outW) *outW = rect.w;
        if (outH) *outH = rect.h;
        return 1;
    }
    if (zoneId >= 68 && zoneId <= 73) {
        DM1_V1_MovementArrowRectPc34 rect;
        if (!dm1_v1_movement_arrow_rect_pc34(zoneId - 68, &rect)) return 0;
        if (outX) *outX = rect.x;
        if (outY) *outY = rect.y;
        if (outW) *outW = rect.w;
        if (outH) *outH = rect.h;
        return 1;
    }
    if (zoneId >= 113 && zoneId <= 116) {
        DM1_V1_LayoutZoneRectPc34 rect;
        if (!dm1_v1_champion_icon_rect_pc34(zoneId - 113, &rect)) return 0;
        if (outX) *outX = rect.x;
        if (outY) *outY = rect.y;
        if (outW) *outW = rect.w;
        if (outH) *outH = rect.h;
        return 1;
    }
    if (zoneId >= 507 && zoneId <= 536) {
        DM1_V1_InventorySlotBoxZonePc34 zone;
        if (!dm1_v1_inventory_source_slot_box_zone_pc34(zoneId - 499, &zone)) {
            return 0;
        }
        if (outX) *outX = zone.x;
        if (outY) *outY = zone.y;
        if (outW) *outW = zone.w;
        if (outH) *outH = zone.h;
        return 1;
    }
    if (zoneId >= 537 && zoneId <= 544) {
        DM1_V1_InventorySlotBoxZonePc34 zone;
        if (!dm1_v1_inventory_chest_slot_box_zone_pc34(zoneId - 537, &zone)) {
            return 0;
        }
        if (outX) *outX = zone.x;
        if (outY) *outY = zone.y;
        if (outW) *outW = zone.w;
        if (outH) *outH = zone.h;
        return 1;
    }
    if (zoneId == 101) {
        DM1_V1_LayoutZoneRectPc34 panelRect =
            dm1_v1_inventory_panel_rect_pc34();
        if (!dm1_v1_inventory_panel_zone_id_pc34()) return 0;
        if (outX) *outX = panelRect.x;
        if (outY) *outY = panelRect.y;
        if (outW) *outW = panelRect.w;
        if (outH) *outH = panelRect.h;
        return 1;
    }
    if (zoneId == 545 || zoneId == 546) {
        if (outX) *outX = (zoneId == 545) ? 56 : 12;
        if (outY) *outY = 13;
        if (outW) *outW = 16;
        if (outH) *outH = 16;
        return 1;
    }
    if (zoneId >= 562 && zoneId <= 566) {
        switch (zoneId) {
            case 562: /* save */
                if (outX) *outX = 179;
                if (outY) *outY = 2;
                if (outW) *outW = 11;
                if (outH) *outH = 11;
                return 1;
            case 564: /* rest */
                if (outX) *outX = 190;
                if (outY) *outY = 2;
                if (outW) *outW = 19;
                if (outH) *outH = 11;
                return 1;
            case 565: /* music */
                if (outX) *outX = 168;
                if (outY) *outY = 3;
                if (outW) *outW = 9;
                if (outH) *outH = 9;
                return 1;
            case 566: /* close */
                if (outX) *outX = 209;
                if (outY) *outY = 2;
                if (outW) *outW = 11;
                if (outH) *outH = 11;
                return 1;
            default:
                break;
        }
    }
    return 0;
}

int DM1_V1_MouseRoutes_CommandForScreenPointPc34Compat(
    int mouseInputList,
    int screenX,
    int screenY,
    int buttonMask,
    int* outCoordinateSpace,
    int* outZoneId) {
    DM1_V1_LayoutZoneRectPc34 viewport = dm1_v1_viewport_rect_pc34();
    return DM1_V1_MouseRoutes_CommandForPointPc34Compat(
        mouseInputList,
        screenX,
        screenY,
        buttonMask,
        viewport.x,
        viewport.y,
        dm1_v1_mouse_route_zone_rect_pc34,
        NULL,
        outCoordinateSpace,
        outZoneId);
}
