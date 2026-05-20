#include <stdio.h>
#include <string.h>
#include "entrance_mouse_routes_pc34_compat.h"

static int expect_route(const char* label,
                        unsigned int ordinal,
                        unsigned int commandId,
                        unsigned int zoneIndex,
                        unsigned int buttonMask,
                        int x,
                        int y,
                        int w,
                        int h,
                        const char* name) {
    EntranceMouseRouteCompat route;
    if (!ENTRANCE_Compat_GetMouseRoute(ordinal, &route)) {
        fprintf(stderr, "FAIL %s missing route\n", label);
        return 0;
    }
    if (route.commandId != commandId || route.zoneIndex != zoneIndex ||
        route.buttonMask != buttonMask || route.x != x || route.y != y ||
        route.w != w || route.h != h || strcmp(route.name, name) != 0) {
        fprintf(stderr,
                "FAIL %s got command=%u zone=%u mask=0x%04x box=%d,%d,%d,%d name=%s\n",
                label, route.commandId, route.zoneIndex, route.buttonMask,
                route.x, route.y, route.w, route.h, route.name);
        return 0;
    }
    return 1;
}

static int expect_hit(const char* label,
                      int x,
                      int y,
                      unsigned int buttonMask,
                      unsigned int commandId,
                      unsigned int zoneIndex) {
    EntranceMouseRouteCompat route;
    if (!ENTRANCE_Compat_HitTestMouseRoute(x, y, buttonMask, &route)) {
        fprintf(stderr, "FAIL %s missed\n", label);
        return 0;
    }
    if (route.commandId != commandId || route.zoneIndex != zoneIndex) {
        fprintf(stderr, "FAIL %s got command=%u zone=%u\n", label, route.commandId, route.zoneIndex);
        return 0;
    }
    return 1;
}

int main(void) {
    unsigned int i;
    int ok = 1;
    EntranceMouseRouteCompat route;

    printf("probe=firestaff_entrance_mouse_routes_source\n");
    printf("entranceMouseRouteEvidence=%s\n", ENTRANCE_Compat_GetMouseRouteEvidence());
    printf("entranceMouseRouteCount=%u\n", ENTRANCE_Compat_GetMouseRouteCount());

    ok &= ENTRANCE_Compat_GetMouseRouteCount() == 5u;
    ok &= expect_route("enter", 1u, 200u, 407u, ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT, 244, 45, 55, 14, "enter_dungeon");
    ok &= expect_route("bonus", 2u, 201u, 407u, ENTRANCE_MOUSE_BUTTON_BONUS_DUNGEON_COMPAT, 244, 45, 55, 14, "enter_bonus_dungeon");
    ok &= expect_route("resume", 3u, 202u, 409u, ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT, 244, 76, 55, 18, "resume");
    ok &= expect_route("quit", 4u, 216u, 434u, ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT, 243, 110, 50, 15, "quit");
    ok &= expect_route("credits", 5u, 203u, 411u, ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT, 248, 186, 46, 14, "draw_credits");

    ok &= expect_hit("enter top-left", 244, 45, ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT, 200u, 407u);
    ok &= expect_hit("enter bottom-right inclusive", 298, 58, ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT, 200u, 407u);
    ok &= expect_hit("bonus same box different mask", 244, 45, ENTRANCE_MOUSE_BUTTON_BONUS_DUNGEON_COMPAT, 201u, 407u);
    ok &= expect_hit("resume", 298, 93, ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT, 202u, 409u);
    ok &= expect_hit("quit", 292, 124, ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT, 216u, 434u);
    ok &= expect_hit("credits", 293, 199, ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT, 203u, 411u);

    if (ENTRANCE_Compat_HitTestMouseRoute(299, 58, ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT, &route)) ok = 0;
    if (ENTRANCE_Compat_HitTestMouseRoute(298, 59, ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT, &route)) ok = 0;
    if (ENTRANCE_Compat_HitTestMouseRoute(244, 45, 0u, &route)) ok = 0;
    if (ENTRANCE_Compat_GetMouseRoute(0u, &route)) ok = 0;
    if (ENTRANCE_Compat_GetMouseRoute(6u, &route)) ok = 0;

    for (i = 1u; i <= ENTRANCE_Compat_GetMouseRouteCount(); ++i) {
        if (ENTRANCE_Compat_GetMouseRoute(i, &route)) {
            printf("entranceMouseRoute[%u]=command:%u zone:%u mask:0x%04x box:%d,%d,%d,%d name:%s evidence:%s\n",
                   i, route.commandId, route.zoneIndex, route.buttonMask,
                   route.x, route.y, route.w, route.h, route.name, route.evidence);
        }
    }
    printf("entranceMouseRoutesInvariantOk=%d\n", ok);
    return ok ? 0 : 1;
}
