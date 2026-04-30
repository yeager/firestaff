#include "action_area_routes_pc34_compat.h"
#include "touch_click_zone_matrix_pc34_compat.h"

static int has_touch_matrix_route(unsigned int commandId, unsigned int zoneIndex) {
    unsigned int i;
    for (i = 0; i < TOUCHCLICK_Compat_GetZoneCount(); ++i) {
        TouchClickZonePc34Compat zone;
        if (!TOUCHCLICK_Compat_GetZone(i, &zone)) return 0;
        if (zone.commandId == commandId && zone.zoneIndex == zoneIndex) return 1;
    }
    return 0;
}

const char* action_area_routes_GetEvidence(void) { return "COMMAND.C:461-471 defines action-area name/icon routes: C112 pass, C113..C115 action slots, and C116..C119 champion action icons; COMMAND.C:2308-2312 dispatches C111 click-in-action-area to F0371 only when no candidate champion is active."; }
unsigned int action_area_routes_GetInvariant(void) { return 1u; }

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
