#include "action_area_routes_pc34_compat.h"
#include "touch_click_zone_matrix_pc34_compat.h"
#include <string.h>

static const ActionAreaRoutePc34Compat kActionAreaRoutes[] = {
    {112u, 98u, -1, -1, 285,  77, 35,  7, "action.pass",
     "COMMAND.C:461-466 maps C112 pass to C098; CLIKMENU.C:529-539 highlights x=285..319 y=77..83 and calls F0391(-1)" },
    {113u, 82u,  0, -1, 234,  86, 85, 11, "action.row0",
     "COMMAND.C:463 maps C113 action row 0 to C082; CLIKMENU.C:541-548 gates row0 with actionCount and calls F0391(0)" },
    {114u, 83u,  1, -1, 234,  98, 85, 11, "action.row1",
     "COMMAND.C:464 maps C114 action row 1 to C083; CLIKMENU.C:550-573 gates row1 with actionCount and calls F0391(1)" },
    {115u, 84u,  2, -1, 234, 110, 85, 11, "action.row2",
     "COMMAND.C:465 maps C115 action row 2 to C084; CLIKMENU.C:564-573 gates row2 with actionCount and calls F0391(2)" },
    {116u, 89u, -1,  0, 233,  86, 20, 35, "action.icon0",
     "COMMAND.C:467-471 maps C116 champion 0 action icon to C089; CLIKMENU.C:578-582 routes to F0389 when champion index < party count" },
    {117u, 90u, -1,  1, 255,  86, 20, 35, "action.icon1",
     "COMMAND.C:469 maps C117 champion 1 action icon to C090; CLIKMENU.C:578-582 routes to F0389 when champion index < party count" },
    {118u, 91u, -1,  2, 277,  86, 20, 35, "action.icon2",
     "COMMAND.C:470 maps C118 champion 2 action icon to C091; CLIKMENU.C:578-582 routes to F0389 when champion index < party count" },
    {119u, 92u, -1,  3, 299,  86, 20, 35, "action.icon3",
     "COMMAND.C:471 maps C119 champion 3 action icon to C092; CLIKMENU.C:578-582 routes to F0389 when champion index < party count" }
};

static int has_touch_matrix_route(unsigned int commandId, unsigned int zoneIndex) {
    unsigned int i;
    for (i = 0; i < TOUCHCLICK_Compat_GetZoneCount(); ++i) {
        TouchClickZonePc34Compat zone;
        if (!TOUCHCLICK_Compat_GetZone(i, &zone)) return 0;
        if (zone.commandId == commandId && zone.zoneIndex == zoneIndex) return 1;
    }
    return 0;
}

static int point_in_route(const ActionAreaRoutePc34Compat* route, int screenX, int screenY) {
    return route &&
           screenX >= route->x && screenX < route->x + route->w &&
           screenY >= route->y && screenY < route->y + route->h;
}

const char* action_area_routes_GetEvidence(void) {
    return "COMMAND.C:461-471 defines action-area name/icon routes: C112 pass, C113..C115 action slots, and C116..C119 champion action icons; COMMAND.C:2308-2312 dispatches C111 click-in-action-area to F0371 only when no candidate champion is active; CLIKMENU.C:529-573 dispatches pass/action rows through F0391 using inclusive source boxes represented as x/y plus width/height; CLIKMENU.C:578-582 routes champion action icons to F0389; MENU.C:635-670 builds G0713_s_ActionList and G0507_ui_ActionCount; MENU.C:820-837 maps selected ActionIndices to champion action state.";
}

unsigned int action_area_routes_GetRouteCount(void) {
    return (unsigned int)(sizeof(kActionAreaRoutes) / sizeof(kActionAreaRoutes[0]));
}

int action_area_routes_GetRoute(unsigned int ordinal, ActionAreaRoutePc34Compat* outRoute) {
    if (!outRoute || ordinal >= action_area_routes_GetRouteCount()) return 0;
    *outRoute = kActionAreaRoutes[ordinal];
    return 1;
}

int action_area_routes_ResolveNameMenuClick(int screenX,
                                            int screenY,
                                            unsigned int actionCount,
                                            ActionAreaRoutePc34Compat* outRoute) {
    unsigned int i;
    for (i = 0; i < 4u; ++i) {
        const ActionAreaRoutePc34Compat* route = &kActionAreaRoutes[i];
        if (!point_in_route(route, screenX, screenY)) continue;
        if (route->commandId == 112u || (unsigned int)(route->actionListIndex + 1) <= actionCount) {
            if (outRoute) *outRoute = *route;
            return 1;
        }
        break;
    }
    if (outRoute) memset(outRoute, 0, sizeof(*outRoute));
    return 0;
}

int action_area_routes_ResolveIconClick(int screenX,
                                        int screenY,
                                        unsigned int partyChampionCount,
                                        ActionAreaRoutePc34Compat* outRoute) {
    unsigned int i;
    for (i = 4u; i < action_area_routes_GetRouteCount(); ++i) {
        const ActionAreaRoutePc34Compat* route = &kActionAreaRoutes[i];
        if (!point_in_route(route, screenX, screenY)) continue;
        if (route->championIndex >= 0 && (unsigned int)route->championIndex < partyChampionCount) {
            if (outRoute) *outRoute = *route;
            return 1;
        }
        break;
    }
    if (outRoute) memset(outRoute, 0, sizeof(*outRoute));
    return 0;
}

unsigned int action_area_routes_GetInvariant(void) {
    ActionAreaRoutePc34Compat route;
    return action_area_routes_GetTouchMatrixInvariant() &&
           action_area_routes_GetRouteCount() == 8u &&
           action_area_routes_ResolveNameMenuClick(285, 77, 0u, &route) &&
           route.commandId == 112u && route.actionListIndex == -1 &&
           action_area_routes_ResolveNameMenuClick(318, 96, 1u, &route) &&
           route.commandId == 113u && route.actionListIndex == 0 &&
           !action_area_routes_ResolveNameMenuClick(318, 96, 0u, &route) &&
           action_area_routes_ResolveNameMenuClick(318, 120, 3u, &route) &&
           route.commandId == 115u && route.actionListIndex == 2 &&
           !action_area_routes_ResolveNameMenuClick(318, 120, 2u, &route) &&
           action_area_routes_ResolveIconClick(318, 120, 4u, &route) &&
           route.commandId == 119u && route.championIndex == 3 &&
           !action_area_routes_ResolveIconClick(318, 120, 3u, &route);
}

unsigned int action_area_routes_GetTouchMatrixInvariant(void) {
    return has_touch_matrix_route(111u, 11u) &&
           has_touch_matrix_route(112u, 98u) &&
           has_touch_matrix_route(113u, 82u) &&
           has_touch_matrix_route(114u, 83u) &&
           has_touch_matrix_route(115u, 84u) &&
           has_touch_matrix_route(116u, 89u) &&
           has_touch_matrix_route(117u, 90u) &&
           has_touch_matrix_route(118u, 91u) &&
           has_touch_matrix_route(119u, 92u);
}
