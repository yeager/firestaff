#include "spell_area_routes_pc34_compat.h"
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

const char* spell_area_routes_GetEvidence(void) {
    return "COMMAND.C:473-483 defines G0454_as_Graphic561_MouseInput_SpellArea with C109 set caster, C101..C106 rune symbols, C108 cast, and C107 recant; COMMAND.C:2302-2307 dispatches C100 click-in-spell-area to F0370 only when no candidate champion and a magic caster exists; CLIKMENU.C:475-512 routes caster tab/symbol/cast/recant effects; SYMBOL.C:16-30 and 85-100 mutate champion symbol state.";
}

unsigned int spell_area_routes_GetInvariant(void) { return spell_area_routes_GetTouchMatrixInvariant(); }

unsigned int spell_area_routes_GetTouchMatrixInvariant(void) {
    return has_touch_matrix_route(100u, 13u) &&
           has_touch_matrix_route(109u, 221u) &&
           has_touch_matrix_route(101u, 245u) &&
           has_touch_matrix_route(102u, 246u) &&
           has_touch_matrix_route(103u, 247u) &&
           has_touch_matrix_route(104u, 248u) &&
           has_touch_matrix_route(105u, 249u) &&
           has_touch_matrix_route(106u, 250u) &&
           has_touch_matrix_route(108u, 252u) &&
           has_touch_matrix_route(107u, 254u);
}
