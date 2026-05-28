#include "csb_v2_lighting_dynamic.h"
#include "csb_v2_chaos_enhanced.h"
#include "csb_v2_viewport_renderer.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int failures = 0;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        ++failures; \
    } \
} while (0)

static void get_rgb(int x, int y, uint8_t *r, uint8_t *g, uint8_t *b) {
    *r = 99;
    *g = 99;
    *b = 99;
    csb_v2_light_get_tile(x, y, r, g, b);
}

static void test_source_palette_plan(void) {
    CSB_V2_SourcePaletteLighting plan =
        csb_v2_light_build_source_palette_lighting(0, true);
    CHECK(plan.sourcePaletteIndex == 0);
    CHECK(plan.sourceLightAmountFloor == 99);
    CHECK(plan.darknessPercent == 1);
    CHECK(plan.shadowAlpha == 2);
    CHECK(plan.enhancedEffectsEnabled);
    CHECK(!plan.deterministicFallback);

    plan = csb_v2_light_build_source_palette_lighting(5, true);
    CHECK(plan.sourcePaletteIndex == 5);
    CHECK(plan.sourceLightAmountFloor == 0);
    CHECK(plan.darknessPercent == 100);
    CHECK(plan.shadowAlpha == 192);
    CHECK(plan.enhancedEffectsEnabled);
    CHECK(!plan.deterministicFallback);

    plan = csb_v2_light_build_source_palette_lighting(-1, true);
    CHECK(plan.sourcePaletteIndex == 5);
    CHECK(plan.sourceLightAmountFloor == 0);
    CHECK(!plan.enhancedEffectsEnabled);
    CHECK(plan.deterministicFallback);
}

static void test_light_map_sources(void) {
    uint8_t r;
    uint8_t g;
    uint8_t b;

    csb_v2_light_init();
    csb_v2_light_set_ambient(0.0f);
    csb_v2_light_set_dungeon_level(0);
    csb_v2_light_compute_map();
    get_rgb(16, 16, &r, &g, &b);
    CHECK(r == 0 && g == 0 && b == 0);
    get_rgb(-1, 16, &r, &g, &b);
    CHECK(r == 0 && g == 0 && b == 0);

    CHECK(csb_v2_light_add_source(16.0f, 16.0f, 4.0f, 255, 100, 50, 25, 0) == 0);
    csb_v2_light_compute_map();
    get_rgb(16, 16, &r, &g, &b);
    CHECK(r == 100 && g == 50 && b == 25);
    get_rgb(18, 16, &r, &g, &b);
    CHECK(r == 56 && g == 28 && b == 14); /* squared distance falloff */
    get_rgb(20, 16, &r, &g, &b);
    CHECK(r == 0 && g == 0 && b == 0); /* radius boundary is exclusive */

    CHECK(csb_v2_light_add_source(16.0f, 16.0f, 2.0f, 255, 240, 240, 240, 0) == 1);
    csb_v2_light_compute_map();
    get_rgb(16, 16, &r, &g, &b);
    CHECK(r == 255 && g == 255 && b == 255);

    csb_v2_light_remove_source(1);
    csb_v2_light_compute_map();
    get_rgb(16, 16, &r, &g, &b);
    CHECK(r == 100 && g == 50 && b == 25);
}

static void test_limits_ambient_and_flicker(void) {
    uint8_t r0;
    uint8_t g0;
    uint8_t b0;
    uint8_t r1;
    uint8_t g1;
    uint8_t b1;
    int i;

    csb_v2_light_init();
    for (i = 0; i < CSB_V2_LIGHT_MAX_SOURCES; ++i) {
        CHECK(csb_v2_light_add_source(0.0f, 0.0f, 1.0f, 1, 1, 1, 1, 0) == i);
    }
    CHECK(csb_v2_light_add_source(0.0f, 0.0f, 1.0f, 1, 1, 1, 1, 0) == -1);
    CHECK(csb_v2_light_add_source(0.0f, 0.0f, 1.0f, 0, 1, 1, 1, 0) == -1);

    csb_v2_light_init();
    csb_v2_light_set_ambient(0.10f);
    csb_v2_light_set_dungeon_level(0);
    csb_v2_light_compute_map();
    get_rgb(4, 4, &r0, &g0, &b0);
    CHECK(r0 == 25 && g0 == 25 && b0 == 25);
    csb_v2_light_set_dungeon_level(5);
    csb_v2_light_compute_map();
    get_rgb(4, 4, &r1, &g1, &b1);
    CHECK(r1 < r0 && g1 < g0 && b1 < b0);
    CHECK(csb_v2_light_get_dungeon_level() == 5);

    csb_v2_light_init();
    csb_v2_light_set_ambient(0.0f);
    csb_v2_light_set_dungeon_level(0);
    CHECK(csb_v2_light_add_source(8.0f, 8.0f, 4.0f, 200, 200, 100, 50, 1) == 0);
    csb_v2_light_compute_map();
    get_rgb(8, 8, &r0, &g0, &b0);
    csb_v2_light_update_flicker(0.25f);
    csb_v2_light_compute_map();
    get_rgb(8, 8, &r1, &g1, &b1);
    CHECK(r1 != r0 || g1 != g0 || b1 != b0);
    CHECK(r1 <= r0 && g1 <= g0 && b1 <= b0);
}

static void test_events_and_viewport_wrappers(void) {
    CSB_V2_ViewportState viewport;
    uint8_t r0;
    uint8_t g0;
    uint8_t b0;
    uint8_t r1;
    uint8_t g1;
    uint8_t b1;
    float glowR;
    float glowG;
    float glowB;
    float glowA;
    const char *evidence;

    csb_v2_light_init();
    csb_v2_light_set_ambient(0.0f);
    csb_v2_light_set_dungeon_level(0);
    CHECK(csb_v2_light_add_source(10.0f, 10.0f, 4.0f, 255, 120, 120, 120, 0) == 0);
    csb_v2_light_compute_map();
    get_rgb(10, 10, &r0, &g0, &b0);
    csb_v2_light_event_trigger(CSB_V2_LIGHT_EVENT_DARKNESS_BURST, 1.0f, 1.0f);
    CHECK(csb_v2_light_event_is_active());
    CHECK(csb_v2_light_event_current_type() == CSB_V2_LIGHT_EVENT_DARKNESS_BURST);
    csb_v2_light_event_tick(0.5f);
    csb_v2_light_compute_map();
    get_rgb(10, 10, &r1, &g1, &b1);
    CHECK(r1 < r0 && g1 < g0 && b1 < b0);
    csb_v2_light_event_tick(0.6f);
    CHECK(!csb_v2_light_event_is_active());
    CHECK(csb_v2_light_event_current_type() == CSB_V2_LIGHT_EVENT_NORMAL);

    csb_v2_viewport_init(&viewport, 4);
    csb_v2_viewport_set_ambient_light(0.0f);
    csb_v2_viewport_set_dungeon_light(&viewport, 0);
    CHECK(csb_v2_viewport_add_torch(12.0f, 12.0f, 255, 80, 40, 20, 0) == 0);
    csb_v2_viewport_compute_light_map();
    csb_v2_viewport_get_tile_light(12, 12, &r0, &g0, &b0);
    CHECK(r0 == 80 && g0 == 40 && b0 == 20);
    csb_v2_viewport_trigger_dsa_light_event(CSB_V2_LIGHT_EVENT_MAGICAL_PULSE, 0.8f, 0.5f);
    CHECK(csb_v2_viewport_light_event_type() == CSB_V2_LIGHT_EVENT_MAGICAL_PULSE);

    csb_v2_chaos_on_trigger(0x80, 0);
    CHECK(csb_v2_viewport_chaos_active_count() == 1);
    csb_v2_viewport_get_chaos_overlay(&glowR, &glowG, &glowB, &glowA);
    CHECK(glowR > 0.0f || glowG > 0.0f || glowB > 0.0f);
    CHECK(glowA > 0.0f);

    evidence = csb_v2_light_source_evidence();
    CHECK(strstr(evidence, "PANEL.C:367-428") != NULL);
    CHECK(strstr(evidence, "PANEL.C:370-405") != NULL);
    CHECK(strstr(evidence, "DATA.C:359-360") != NULL);
    CHECK(strstr(csb_v2_viewport_source_evidence(), "PANEL.C:367-428") != NULL);
}

int main(void) {
    test_source_palette_plan();
    test_light_map_sources();
    test_limits_ambient_and_flicker();
    test_events_and_viewport_wrappers();

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("csb_v2_lighting_dynamic: ok");
    return 0;
}
