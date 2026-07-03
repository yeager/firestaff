#include "dm1_v2_enhanced_effects_runtime_pc34.h"
#include "dm1_v2_lighting_dynamic_pc34.h"
#include "dm1_v2_particle_system_pc34.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

static int failures = 0;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        failures++; \
    } \
} while (0)

static int approxf(float a, float b) {
    return fabsf(a - b) < 0.0001f;
}

static void init_particle_with_short_life(void) {
    int emitter;
    v2_particle_init();
    v2_particle_set_seed(1u);
    emitter = v2_particle_emitter_create(
        10.0f, 12.0f, 1.0f, 0.0f, 0.25f, 1.0f, 0.0f, 0x00FF00FFu, 8);
    CHECK(emitter == 0);
    v2_particle_emit(emitter, 10.0f, 12.0f);
    CHECK(v2_particle_active_count() == 1);
}

static void test_default_gate_is_noop(void) {
    DM1_V2_PhaseGateConfig gate;
    DM1_V2_Settings settings;
    float beforeLight;

    dm1_v2_phase_gate_defaults(&gate);
    memset(&settings, 0, sizeof(settings));
    settings.dynamicLightingEnabled = 1;
    init_particle_with_short_life();
    v22_light_set_ambient(0.37f);
    beforeLight = v22_light_get(4, 4);

    CHECK(dm1_v2_enhanced_effects_runtime_tick(&gate, &settings, 1.0f) == 0);
    CHECK(v2_particle_active_count() == 1);
    CHECK(approxf(v22_light_get(4, 4), beforeLight));
}

static void test_presentation_gate_ticks_particles_without_lighting(void) {
    DM1_V2_PhaseGateConfig gate;
    DM1_V2_Settings settings;
    float beforeLight;

    dm1_v2_phase_gate_defaults(&gate);
    gate.v2PresentationEnabled = 1;
    memset(&settings, 0, sizeof(settings));
    settings.dynamicLightingEnabled = 0;
    init_particle_with_short_life();
    v22_light_set_ambient(0.44f);
    beforeLight = v22_light_get(5, 5);

    CHECK(dm1_v2_enhanced_effects_runtime_tick(&gate, &settings, 1.0f) == 1);
    CHECK(v2_particle_active_count() == 0);
    CHECK(approxf(v22_light_get(5, 5), beforeLight));
}

static void test_presentation_gate_and_dynamic_lighting_tick_light_map(void) {
    DM1_V2_PhaseGateConfig gate;
    DM1_V2_Settings settings;

    dm1_v2_phase_gate_defaults(&gate);
    gate.v2PresentationEnabled = 1;
    memset(&settings, 0, sizeof(settings));
    settings.dynamicLightingEnabled = 1;
    v2_light_init();
    v22_light_clear();
    v22_light_set_ambient(0.52f);

    CHECK(dm1_v2_enhanced_effects_runtime_tick(&gate, &settings, 0.016f) == 1);
    CHECK(approxf(v22_light_get(6, 6), 0.52f));
}

static void test_indexed_render_gate_paints_particles(void) {
    DM1_V2_PhaseGateConfig gate;
    DM1_V2_Settings settings;
    unsigned char framebuffer[64 * 64];

    dm1_v2_phase_gate_defaults(&gate);
    dm1_v2_settings_defaults(&settings);
    memset(framebuffer, 0, sizeof(framebuffer));
    init_particle_with_short_life();

    CHECK(dm1_v2_enhanced_effects_runtime_render_indexed(
              &gate, &settings, framebuffer, 64, 64, 4, 5) == 0);
    CHECK(framebuffer[(5 + 12) * 64 + (4 + 10)] == 0);

    gate.v2PresentationEnabled = 1;
    CHECK(dm1_v2_enhanced_effects_runtime_render_indexed(
              &gate, &settings, framebuffer, 64, 64, 4, 5) > 0);
    CHECK(framebuffer[(5 + 12) * 64 + (4 + 10)] != 0);
}

static void test_direct_particle_seed_is_bounded_and_visible(void) {
    unsigned char framebuffer[32 * 32];
    int i;

    v2_particle_init();
    memset(framebuffer, 0, sizeof(framebuffer));
    CHECK(v2_particle_add_direct(4.0f, 5.0f, 0.25f, 1.0f, 0.0f,
                                 0x00ff00ffu) == 0);
    CHECK(v2_particle_active_count() == 1);
    CHECK(v2_particle_blit_indexed(framebuffer, 32, 32, 2, 3, 12u) > 0);
    CHECK(framebuffer[(3 + 5) * 32 + (2 + 4)] == 12u);

    v2_particle_init();
    for (i = 0; i < M11_V2_MAX_PARTICLES; ++i) {
        CHECK(v2_particle_add_direct((float)i, 1.0f, 0.25f, 1.0f, 0.0f,
                                     0x00ff00ffu) == i);
    }
    CHECK(v2_particle_add_direct(1.0f, 1.0f, 0.25f, 1.0f, 0.0f,
                                 0x00ff00ffu) == -1);
    CHECK(v2_particle_active_count() == M11_V2_MAX_PARTICLES);

    v2_particle_init();
    CHECK(v2_particle_add_direct(1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
                                 0x00ff00ffu) == -1);
    CHECK(v2_particle_add_direct(1.0f, 1.0f, 0.25f, 0.0f, 0.0f,
                                 0x00ff00ffu) == -1);
    CHECK(v2_particle_active_count() == 0);
}

static void test_null_inputs_fail_safe(void) {
    DM1_V2_PhaseGateConfig gate;
    float beforeLight;

    init_particle_with_short_life();
    beforeLight = v22_light_get(7, 7);
    CHECK(dm1_v2_enhanced_effects_runtime_tick(NULL, NULL, 1.0f) == 0);
    CHECK(v2_particle_active_count() == 1);
    CHECK(approxf(v22_light_get(7, 7), beforeLight));

    dm1_v2_phase_gate_defaults(&gate);
    gate.v2PresentationEnabled = 1;
    init_particle_with_short_life();
    beforeLight = v22_light_get(8, 8);
    CHECK(dm1_v2_enhanced_effects_runtime_tick(&gate, NULL, 1.0f) == 1);
    CHECK(v2_particle_active_count() == 0);
    CHECK(approxf(v22_light_get(8, 8), beforeLight));
}

int main(void) {
    const char* evidence = dm1_v2_enhanced_effects_runtime_source_evidence();
    CHECK(evidence != NULL);
    CHECK(strstr(evidence, "RENDER_PRESENTATION") != NULL);
    CHECK(strstr(evidence, "PROJEXPL.C") != NULL);

    test_default_gate_is_noop();
    test_presentation_gate_ticks_particles_without_lighting();
    test_presentation_gate_and_dynamic_lighting_tick_light_map();
    test_indexed_render_gate_paints_particles();
    test_direct_particle_seed_is_bounded_and_visible();
    test_null_inputs_fail_safe();

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("dm1_v2_enhanced_effects_runtime_pc34: ok");
    return 0;
}
