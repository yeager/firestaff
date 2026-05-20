#include "entrance_mouse_routes_pc34_compat.h"

/* ReDMCSB source lock:
 * - COMMAND.C:340-353 defines the later-media G0445 entrance mouse table.
 * - DEFS.H:375-384 fixes C200/C201/M566/M567 command IDs for I34E/I34M.
 * - DEFS.H:3824-3826,3845 names C407/C409/C411/C434 entrance zones.
 * - COMMAND.C:1113-1139 F0673 resolves zone-backed MOUSE_INPUT rows to
 *   inclusive screen boxes for MEDIA707_I34E/I34M.
 * - COORD.C:2490-2495 F0638_GetZone fetches the layout-696 zone records.
 */
static const EntranceMouseRouteCompat kRoutes[] = {
    { 1u, 200u, 407u, ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT,
      244, 45, 55, 14, "enter_dungeon",
      "COMMAND.C:346 maps C200 to C407_ZONE_ENTRANCE_ENTER; layout-696 records 406/407 resolve to x=244 y=45 w=55 h=14" },
    { 2u, 201u, 407u, ENTRANCE_MOUSE_BUTTON_BONUS_DUNGEON_COMPAT,
      244, 45, 55, 14, "enter_bonus_dungeon",
      "COMMAND.C:347 maps C201 to C407_ZONE_ENTRANCE_ENTER with bonus-dungeon mask 0x0010" },
    { 3u, 202u, 409u, ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT,
      244, 76, 55, 18, "resume",
      "COMMAND.C:348 maps I34E/I34M M566 command 202 to C409_ZONE_ENTRANCE_RESUME; layout-696 records 408/409 resolve to x=244 y=76 w=55 h=18" },
    { 4u, 216u, 434u, ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT,
      243, 110, 50, 15, "quit",
      "COMMAND.C:350 maps C216 to C434_ZONE_ENTRANCE_QUIT; layout-696 records 433/434 resolve to x=243 y=110 w=50 h=15" },
    { 5u, 203u, 411u, ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT,
      248, 186, 46, 14, "draw_credits",
      "COMMAND.C:352 maps I34E/I34M M567 command 203 to C411_ZONE_ENTRANCE_CREDITS; layout-696 records 410/411 resolve to x=248 y=186 w=46 h=14" }
};

static int in_route_box(const EntranceMouseRouteCompat* route, int screenX, int screenY) {
    return screenX >= route->x && screenX < route->x + route->w &&
           screenY >= route->y && screenY < route->y + route->h;
}

unsigned int ENTRANCE_Compat_GetMouseRouteCount(void) {
    return (unsigned int)(sizeof(kRoutes) / sizeof(kRoutes[0]));
}

int ENTRANCE_Compat_GetMouseRoute(unsigned int ordinal, EntranceMouseRouteCompat* outRoute) {
    if (!outRoute || ordinal < 1u || ordinal > ENTRANCE_Compat_GetMouseRouteCount()) return 0;
    *outRoute = kRoutes[ordinal - 1u];
    return 1;
}

int ENTRANCE_Compat_HitTestMouseRoute(int screenX,
                                      int screenY,
                                      unsigned int buttonMask,
                                      EntranceMouseRouteCompat* outRoute) {
    unsigned int i;
    if (buttonMask == 0u) return 0;
    for (i = 0; i < ENTRANCE_Compat_GetMouseRouteCount(); ++i) {
        const EntranceMouseRouteCompat* route = &kRoutes[i];
        if ((buttonMask & route->buttonMask) && in_route_box(route, screenX, screenY)) {
            if (outRoute) *outRoute = *route;
            return 1;
        }
    }
    return 0;
}

const char* ENTRANCE_Compat_GetMouseRouteEvidence(void) {
    return "ReDMCSB COMMAND.C:340-353 G0445 entrance mouse table; DEFS.H:375-384 I34E/I34M command IDs; DEFS.H:3824-3826,3845 entrance zones; COMMAND.C:1113-1139 F0673 zone-to-box expansion; COORD.C:2490-2495 F0638_GetZone; ENTRANCE.C:739-747 installs entrance input and ENTRANCE.C:850-883 waits for a fresh entrance command.";
}
