#include <stdio.h>
#include <string.h>
#include "touch_click_zone_matrix_pc34_compat.h"

static int expect_zone(unsigned int ordinal, unsigned int commandId, unsigned int zoneIndex,
                       int x, int y, int w, int h, const char* groupName) {
    TouchClickZonePc34Compat zone;
    if (!TOUCHCLICK_Compat_GetZone(ordinal, &zone)) return 0;
    return zone.commandId == commandId && zone.zoneIndex == zoneIndex &&
           zone.x == x && zone.y == y && zone.w == w && zone.h == h &&
           strcmp(zone.groupName, groupName) == 0;
}

int main(void) {
    TouchClickZonePc34Compat hit;
    int ok = 1;
    printf("probe=firestaff_touch_click_zone_matrix\n");
    printf("sourceEvidence=%s\n", TOUCHCLICK_Compat_GetSourceEvidence());
    printf("zoneCount=%u\n", TOUCHCLICK_Compat_GetZoneCount());
    if (TOUCHCLICK_Compat_GetZoneCount() != 10u) ok = 0;
    if (!expect_zone(1u, 1u, 68u, 234, 125, 28, 21, "movement")) ok = 0;
    if (!expect_zone(3u, 111u, 11u, 233, 77, 87, 45, "action")) ok = 0;
    if (!expect_zone(7u, 101u, 245u, 235, 51, 13, 11, "spell")) ok = 0;
    if (!expect_zone(9u, 28u, 507u, 6, 53, 16, 16, "inventory")) ok = 0;
    if (!TOUCHCLICK_Compat_HitTest(264, 126, &hit) || hit.zoneIndex != 70u) ok = 0;
    if (!TOUCHCLICK_Compat_HitTest(286, 78, &hit) || hit.zoneIndex != 98u) ok = 0;
    if (TOUCHCLICK_Compat_HitTest(6, 53, &hit) && hit.zoneIndex == 507u) ok = 0;
    printf("touchClickZoneMatrixInvariantOk=%u\n", ok ? 1u : 0u);
    return ok ? 0 : 1;
}
