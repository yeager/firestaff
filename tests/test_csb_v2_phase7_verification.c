#include "csb_v2_chaos_enhanced.h"
#include "csb_v2_lighting_dynamic.h"
#include "csb_v2_minimap.h"
#include "csb_v2_smooth_movement.h"
#include "csb_v2_viewport_renderer.h"

#include <math.h>
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

static int near_float(float a, float b, float epsilon) {
    return fabsf(a - b) <= epsilon;
}

static void test_boot_viewport_clock(void) {
    CSB_V2_ViewportState state;
    const char *evidence;

    memset(&state, 0x7f, sizeof(state));
    csb_v2_viewport_init(NULL, 2);
    csb_v2_viewport_init(&state, 4);
    CHECK(state.scale_factor == 4);
    CHECK(state.epx_enabled == 1);
    CHECK(csb_v2_viewport_sub_tick(&state) == 0.0f);

    csb_v2_viewport_v1_tick(&state, 1000u);
    CHECK(state.clock.last_v1_tick_ms == 1000u);
    csb_v2_viewport_render_frame(&state, 1027u);
    CHECK(csb_v2_viewport_sub_tick(&state) > 0.45f);
    CHECK(csb_v2_viewport_sub_tick(&state) < 0.55f);
    csb_v2_viewport_render_frame(&state, 1055u);
    CHECK(near_float(csb_v2_viewport_sub_tick(&state), 1.0f, 0.001f));

    evidence = csb_v2_viewport_source_evidence();
    CHECK(strstr(evidence, "V2_AnimClock") != NULL);
    CHECK(strstr(evidence, "PANEL.C:367-428") != NULL);
}

static void test_smooth_movement_verification(void) {
    V2_AnimClock clock;
    const char *evidence;

    csb_v2_smooth_init();
    CHECK(!csb_v2_smooth_is_moving());
    CHECK(CSB_V2_WALK_EASE == V2_EASE_OUT_CUBIC);
    CHECK(CSB_V2_TURN_EASE == V2_EASE_OUT_QUAD);

    csb_v2_smooth_start_walk(0.0f, 0.0f, 1.0f, 1.0f);
    CHECK(csb_v2_smooth_is_moving());
    memset(&clock, 0, sizeof(clock));
    clock.dt_ms = 27.5f;
    csb_v2_smooth_update_from_clock(&clock);
    CHECK(csb_v2_smooth_get_x() > 0.5f);
    CHECK(csb_v2_smooth_get_x() < 1.0f);
    CHECK(near_float(csb_v2_smooth_get_x(), csb_v2_smooth_get_y(), 0.001f));
    clock.dt_ms = 27.5f;
    csb_v2_smooth_update_from_clock(&clock);
    CHECK(near_float(csb_v2_smooth_get_x(), 1.0f, 0.001f));
    CHECK(!csb_v2_smooth_is_moving());

    csb_v2_smooth_start_turn(0.0f, 90.0f);
    clock.dt_ms = 55.0f;
    csb_v2_smooth_update_from_clock(&clock);
    CHECK(near_float(csb_v2_smooth_get_angle(), 90.0f, 0.001f));

    evidence = csb_v2_smooth_source_evidence();
    CHECK(strstr(evidence, "COMMAND.C") != NULL);
    CHECK(strstr(evidence, "CLIKMENU.C") != NULL);
    CHECK(strstr(evidence, "55ms") != NULL);
}

static void test_minimap_verification(void) {
    const char *evidence;

    CHECK(csb_v2_minimap_square_color(0, 0, 0) == 0xFF000000u);
    CHECK(csb_v2_minimap_square_color(0, 1, 0) == 0xFF000000u);
    CHECK(csb_v2_minimap_square_color(0, 1, 1) == 0xFFFF00FFu);
    CHECK(csb_v2_minimap_square_color(0, 0, 1) == 0xFF404040u);
    CHECK(csb_v2_minimap_square_color(1, 0, 1) == 0xFFA0A0A0u);
    CHECK(csb_v2_minimap_square_color(2, 0, 1) == 0xFF0000FFu);
    CHECK(csb_v2_minimap_square_color(3, 0, 1) == 0xFF0000CCu);
    CHECK(csb_v2_minimap_square_color(4, 0, 1) == 0xFFCC0000u);
    CHECK(csb_v2_minimap_square_color(16, 0, 1) == 0xFF806030u);
    CHECK(csb_v2_minimap_square_color(-1, 0, 1) == 0xFF606060u);

    evidence = csb_v2_minimap_source_evidence();
    CHECK(strstr(evidence, "DSA") != NULL);
}

static void test_chaos_lighting_verification(void) {
    CSB_V2_ViewportState state;
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

    csb_v2_viewport_init(&state, 2);
    CHECK(csb_v2_chaos_active_count() == 0);

    csb_v2_viewport_set_ambient_light(0.0f);
    csb_v2_viewport_set_dungeon_light(&state, 0);
    CHECK(csb_v2_viewport_add_torch(10.0f, 10.0f, 255, 120, 80, 40, 1) == 0);
    csb_v2_viewport_compute_light_map();
    csb_v2_viewport_get_tile_light(10, 10, &r0, &g0, &b0);
    CHECK(r0 == 120 && g0 == 80 && b0 == 40);

    csb_v2_viewport_render_frame(&state, 1016u);
    csb_v2_viewport_compute_light_map();
    csb_v2_viewport_get_tile_light(10, 10, &r1, &g1, &b1);
    CHECK(r1 <= r0 && g1 <= g0 && b1 <= b0);

    csb_v2_chaos_on_trigger(0x80, 0);
    CHECK(csb_v2_chaos_active_count() == 1);
    CHECK(csb_v2_light_event_current_type() == CSB_V2_LIGHT_EVENT_CHAOS_SURGE);
    csb_v2_chaos_render_overlay(&glowR, &glowG, &glowB, &glowA);
    CHECK(glowA > 0.0f);
    CHECK(glowR > 0.0f || glowG > 0.0f || glowB > 0.0f);

    csb_v2_chaos_tick(3.0f);
    CHECK(csb_v2_chaos_active_count() == 0);
    csb_v2_light_event_tick(1.1f);
    CHECK(csb_v2_light_event_current_type() == CSB_V2_LIGHT_EVENT_NORMAL);
}

static void test_evidence_summary(void) {
    const char *light = csb_v2_light_source_evidence();
    const char *chaos = csb_v2_chaos_source_evidence();

    CHECK(strstr(light, "PANEL.C:367-428") != NULL);
    CHECK(strstr(light, "PANEL.C:370-405") != NULL);
    CHECK(strstr(light, "DATA.C:359-360") != NULL);
    CHECK(strstr(chaos, "DSA") != NULL);
    CHECK(strstr(chaos, "PANEL.C:367-428") != NULL);
}

int main(void) {
    test_boot_viewport_clock();
    test_smooth_movement_verification();
    test_minimap_verification();
    test_chaos_lighting_verification();
    test_evidence_summary();

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("csb_v2_phase7_verification: ok");
    return 0;
}
