#include <stdio.h>
#include <string.h>
#include "action_area_routes_pc34_compat.h"

static int expect_route(unsigned int ordinal,
                        unsigned int commandId,
                        unsigned int zoneIndex,
                        int actionListIndex,
                        int championIndex,
                        int x,
                        int y,
                        int w,
                        int h,
                        const char* name) {
    ActionAreaRoutePc34Compat route;
    if (!action_area_routes_GetRoute(ordinal, &route)) return 0;
    return route.commandId == commandId &&
           route.zoneIndex == zoneIndex &&
           route.actionListIndex == actionListIndex &&
           route.championIndex == championIndex &&
           route.x == x && route.y == y && route.w == w && route.h == h &&
           strcmp(route.name, name) == 0;
}

int main(void) {
    ActionAreaRoutePc34Compat route;
    unsigned int ok = action_area_routes_GetInvariant();

    printf("probe=firestaff_action_area_routes\n");
    printf("sourceEvidence=%s\n", action_area_routes_GetEvidence());
    printf("routeCount=%u\n", action_area_routes_GetRouteCount());

    if (action_area_routes_GetRouteCount() != 8u) ok = 0;
    if (!expect_route(0u, 112u, 98u, -1, -1, 285, 77, 35, 7, "action.pass")) ok = 0;
    if (!expect_route(1u, 113u, 82u, 0, -1, 234, 86, 85, 11, "action.row0")) ok = 0;
    if (!expect_route(2u, 114u, 83u, 1, -1, 234, 98, 85, 11, "action.row1")) ok = 0;
    if (!expect_route(3u, 115u, 84u, 2, -1, 234, 110, 85, 11, "action.row2")) ok = 0;
    if (!expect_route(4u, 116u, 89u, -1, 0, 233, 86, 20, 35, "action.icon0")) ok = 0;
    if (!expect_route(7u, 119u, 92u, -1, 3, 299, 86, 20, 35, "action.icon3")) ok = 0;

    if (!action_area_routes_ResolveNameMenuClick(285, 77, 0u, &route) || route.commandId != 112u) ok = 0;
    if (!action_area_routes_ResolveNameMenuClick(319, 83, 0u, &route) || route.commandId != 112u) ok = 0;
    if (action_area_routes_ResolveNameMenuClick(320, 83, 0u, &route)) ok = 0;
    if (action_area_routes_ResolveNameMenuClick(285, 84, 0u, &route)) ok = 0;

    if (!action_area_routes_ResolveNameMenuClick(234, 86, 1u, &route) || route.commandId != 113u || route.actionListIndex != 0) ok = 0;
    if (!action_area_routes_ResolveNameMenuClick(318, 96, 1u, &route) || route.commandId != 113u || route.actionListIndex != 0) ok = 0;
    if (action_area_routes_ResolveNameMenuClick(318, 96, 0u, &route)) ok = 0;
    if (action_area_routes_ResolveNameMenuClick(319, 96, 1u, &route)) ok = 0;
    if (action_area_routes_ResolveNameMenuClick(318, 97, 1u, &route)) ok = 0;

    if (!action_area_routes_ResolveNameMenuClick(234, 98, 2u, &route) || route.commandId != 114u || route.actionListIndex != 1) ok = 0;
    if (!action_area_routes_ResolveNameMenuClick(318, 108, 2u, &route) || route.commandId != 114u || route.actionListIndex != 1) ok = 0;
    if (action_area_routes_ResolveNameMenuClick(318, 108, 1u, &route)) ok = 0;

    if (!action_area_routes_ResolveNameMenuClick(234, 110, 3u, &route) || route.commandId != 115u || route.actionListIndex != 2) ok = 0;
    if (!action_area_routes_ResolveNameMenuClick(318, 120, 3u, &route) || route.commandId != 115u || route.actionListIndex != 2) ok = 0;
    if (action_area_routes_ResolveNameMenuClick(318, 120, 2u, &route)) ok = 0;

    if (!action_area_routes_ResolveIconClick(233, 86, 1u, &route) || route.commandId != 116u || route.championIndex != 0) ok = 0;
    if (!action_area_routes_ResolveIconClick(252, 120, 1u, &route) || route.commandId != 116u || route.championIndex != 0) ok = 0;
    if (action_area_routes_ResolveIconClick(253, 120, 1u, &route)) ok = 0;
    if (!action_area_routes_ResolveIconClick(318, 120, 4u, &route) || route.commandId != 119u || route.championIndex != 3) ok = 0;
    if (action_area_routes_ResolveIconClick(318, 120, 3u, &route)) ok = 0;
    if (action_area_routes_ResolveIconClick(319, 120, 4u, &route)) ok = 0;
    if (action_area_routes_ResolveIconClick(318, 121, 4u, &route)) ok = 0;

    printf("actionAreaRoutesInvariantOk=%u\n", action_area_routes_GetInvariant());
    printf("actionAreaTouchMatrixInvariantOk=%u\n", action_area_routes_GetTouchMatrixInvariant());
    printf("actionAreaInclusiveHitInvariantOk=%u\n", ok ? 1u : 0u);
    return ok ? 0 : 1;
}
