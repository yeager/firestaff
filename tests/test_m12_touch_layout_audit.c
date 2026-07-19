/*
 * test_m12_touch_layout_audit — Launcher half of the "UI scaling and
 * touch-target audit across launcher and game views" TODO.
 *
 * Audits the three shipped M12 touch-layout presets (Classic, Compact,
 * One-handed) through the fs_gesture_navigation_gate zone-audit contract
 * (24 px source-space minimum floor / 44 px recommended, see
 * include/fs_gesture_navigation_gate.h and the launcher's
 * M12_TOUCH_MIN_ZONE_SIZE baseline). Shipped presets must never contain a
 * sub-floor or sub-recommended touch target; the editor clamp must land
 * user-resized zones exactly on the audit floor so a saved layout can
 * never fall below it either.
 */

#include "touch_layout_m12.h"
#include "fs_gesture_navigation_gate.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;

#define CHECK(cond_, msg_) do {                                            \
    if (!(cond_)) {                                                        \
        printf("  FAIL  %s:%d  %s\n", __FILE__, __LINE__, msg_);          \
        g_failures++;                                                      \
    }                                                                      \
} while (0)

static int audit_preset(M12_TouchPreset preset, const char* name) {
    M12_TouchLayout layout;
    FsGestureZone zones[M12_TOUCH_MAX_ZONES];
    FsGestureZoneAuditReport report;
    int i;
    int audited;

    M12_TouchLayout_LoadPreset(&layout, preset);
    CHECK(layout.zoneCount > 0, "preset must ship at least one zone");
    CHECK(layout.zoneCount <= M12_TOUCH_MAX_ZONES, "preset zone count within bounds");
    if (layout.zoneCount <= 0) return 1;

    for (i = 0; i < layout.zoneCount; ++i) {
        zones[i].x = layout.zones[i].x;
        zones[i].y = layout.zones[i].y;
        zones[i].w = layout.zones[i].w;
        zones[i].h = layout.zones[i].h;
        zones[i].groupName = layout.zones[i].label;
    }
    audited = fs_gesture_audit_zones(zones, layout.zoneCount,
                                     FS_GG_PLATFORM_MIN_TARGET_PX,
                                     FS_GG_PLATFORM_RECOMMENDED_PX,
                                     &report);
    CHECK(audited == layout.zoneCount, "every preset zone audited");
    printf("  preset %-10s zones=%2d  below-min=%d  below-recommended=%d\n",
           name, report.totalZones,
           report.zonesBelowMinimum, report.zonesBelowRecommended);
    /* Shipped launcher presets must never ship a sub-floor touch target. */
    CHECK(report.zonesBelowMinimum == 0,
          "preset contains a zone below the 24 px touch-target floor");
    /* ...and every shipped zone should meet the 44 px recommendation. */
    CHECK(report.zonesBelowRecommended == 0,
          "preset contains a zone below the 44 px recommended target");
    return 0;
}

static void test_editor_floor_consistency(void) {
    M12_TouchLayout layout;
    FsGestureZone shrunk[1];
    FsGestureZoneAuditReport report;

    printf("[editor:floor]\n");
    /* The layout editor's minimum zone size must not sit below the
     * gesture audit's platform floor; otherwise the editor would let a
     * user save a zone the audit flags as untouchable. */
    CHECK(M12_TOUCH_MIN_ZONE_SIZE >= FS_GG_PLATFORM_MIN_TARGET_PX,
          "editor minimum zone size must cover the audit platform floor");

    /* A user dragging a zone down to 10x10 must be clamped to the floor,
     * and the clamped result must pass the audit minimum (but is allowed
     * to sit below the 44 px recommendation, matching the deliberate
     * 24 px launcher baseline documented in fs_gesture_navigation_gate). */
    M12_TouchLayout_LoadPreset(&layout, M12_TOUCH_PRESET_CLASSIC);
    layout.zones[0].w = 10;
    layout.zones[0].h = 10;
    M12_TouchLayout_ClampZone(&layout.zones[0]);
    CHECK(layout.zones[0].w == M12_TOUCH_MIN_ZONE_SIZE &&
          layout.zones[0].h == M12_TOUCH_MIN_ZONE_SIZE,
          "clamp must raise an under-size zone to the editor floor");

    shrunk[0].x = layout.zones[0].x;
    shrunk[0].y = layout.zones[0].y;
    shrunk[0].w = layout.zones[0].w;
    shrunk[0].h = layout.zones[0].h;
    shrunk[0].groupName = "editor.clamped";
    (void)fs_gesture_audit_zones(shrunk, 1,
                                 FS_GG_PLATFORM_MIN_TARGET_PX,
                                 FS_GG_PLATFORM_RECOMMENDED_PX,
                                 &report);
    CHECK(report.zonesBelowMinimum == 0,
          "editor-clamped zone must pass the audit minimum");
    CHECK(report.audits[0].meetsMinimum == 1,
          "editor-clamped zone flagged below minimum");
}

int main(void) {
    printf("[presets]\n");
    audit_preset(M12_TOUCH_PRESET_CLASSIC, "Classic");
    audit_preset(M12_TOUCH_PRESET_COMPACT, "Compact");
    audit_preset(M12_TOUCH_PRESET_ONE_HANDED, "One-handed");
    test_editor_floor_consistency();

    printf("\nResult: %s (%d failure%s)\n",
           g_failures == 0 ? "PASS" : "FAIL",
           g_failures, g_failures == 1 ? "" : "s");
    return g_failures == 0 ? 0 : 1;
}
