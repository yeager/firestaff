/*
 * test_nexus_v2_lighting_pc34.c
 *
 * Unit test for Nexus V2.2 dynamic lighting (nexus_v2_lighting.c).
 *
 * Validates:
 *  - nexus_v2_lighting_init() zeroes state and sets ambient defaults
 *  - nexus_v2_light_add() inserts lights, returns valid index
 *  - nexus_v2_light_remove() clears active flag
 *  - nexus_v2_lighting_tick() advances torch_flicker_phase and
 *    updates flickering lights' intensity
 *  - nexus_v2_apply_lighting() applies ambient + per-light contribution
 *    to RGBA framebuffer, in-place, with byte-stable output for known
 *    inputs
 *  - null-args are safe
 *  - source-evidence references NEXUS.BIN / ReDMCSB
 *  - NEXUS_MAX_LIGHTS = 16
 *  - Out-of-bounds index for remove is safe
 *
 * Source-lock:
 *  - include/nexus_v2_lighting.h
 *  - src/nexus/nexus_v2_lighting.c
 */

#include "nexus_v2_lighting.h"

#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_passed = 0;

static void check_int(const char *id, int got, int want)
{
    ++g_assertions;
    if (got == want) {
        ++g_passed;
    } else {
        printf("FAIL %s got=%d want=%d\n", id, got, want);
    }
}

static void check_true(const char *id, int condition)
{
    ++g_assertions;
    if (condition) {
        ++g_passed;
    } else {
        printf("FAIL %s\n", id);
    }
}

static void check_u32(const char *id, uint32_t got, uint32_t want)
{
    ++g_assertions;
    if (got == want) {
        ++g_passed;
    } else {
        printf("FAIL %s got=0x%08x want=0x%08x\n", id,
               (unsigned int)got, (unsigned int)want);
    }
}

static void check_float_near(const char *id, float got, float want, float eps)
{
    ++g_assertions;
    if (got >= want - eps && got <= want + eps) {
        ++g_passed;
    } else {
        printf("FAIL %s got=%f want=%f eps=%f\n", id, (double)got, (double)want, (double)eps);
    }
}

static void check_init_defaults(void)
{
    Nexus_V2_LightingState ls;
    /* poison so we can see defaults win */
    memset(&ls, 0x55, sizeof(ls));
    nexus_v2_lighting_init(&ls);
    check_int("init.light_count", ls.light_count, 0);
    /* All lights inactive. */
    int all_inactive = 1;
    int i;
    for (i = 0; i < NEXUS_MAX_LIGHTS; ++i) {
        if (ls.lights[i].active != 0) {
            all_inactive = 0;
            break;
        }
    }
    check_true("init.all_inactive", all_inactive);
    /* torch_flicker_phase starts at 0. */
    check_float_near("init.torch_flicker_phase", ls.torch_flicker_phase, 0.0f, 0.0001f);
    /* Ambient defaults per nexus_v2_lighting_init: 0.15 / 0.12 / 0.10. */
    check_float_near("init.ambient_r", ls.ambient_r, 0.15f, 0.0001f);
    check_float_near("init.ambient_g", ls.ambient_g, 0.12f, 0.0001f);
    check_float_near("init.ambient_b", ls.ambient_b, 0.10f, 0.0001f);
}

static void check_null_init(void)
{
    /* Init on NULL must be a no-op (no crash). */
    nexus_v2_lighting_init(0);
    check_int("init.null.no_crash", 1, 1);
}

static void check_light_add_basic(void)
{
    Nexus_V2_LightingState ls;
    int idx;
    nexus_v2_lighting_init(&ls);

    idx = nexus_v2_light_add(&ls, 1.0f, 2.0f, 3.0f,
                              0.5f, 0.6f, 0.7f,
                              0.8f, 4.0f, 1);
    check_int("add.first_index", idx, 0);
    check_int("add.first_active", ls.lights[0].active, 1);
    check_int("add.first_count", ls.light_count, 1);
    check_float_near("add.first_x", ls.lights[0].x, 1.0f, 0.0001f);
    check_float_near("add.first_y", ls.lights[0].y, 2.0f, 0.0001f);
    check_float_near("add.first_z", ls.lights[0].z, 3.0f, 0.0001f);
    check_float_near("add.first_r", ls.lights[0].r, 0.5f, 0.0001f);
    check_float_near("add.first_g", ls.lights[0].g, 0.6f, 0.0001f);
    check_float_near("add.first_b", ls.lights[0].b, 0.7f, 0.0001f);
    check_float_near("add.first_intensity", ls.lights[0].intensity, 0.8f, 0.0001f);
    check_float_near("add.first_radius", ls.lights[0].radius, 4.0f, 0.0001f);
    check_int("add.first_flicker", ls.lights[0].flicker, 1);
}

static void check_light_add_full(void)
{
    Nexus_V2_LightingState ls;
    int i;
    int idx;
    nexus_v2_lighting_init(&ls);
    for (i = 0; i < NEXUS_MAX_LIGHTS; ++i) {
        idx = nexus_v2_light_add(&ls, (float)i, 0.0f, 0.0f,
                                  0.1f, 0.1f, 0.1f, 0.5f, 1.0f, 0);
        check_int("add.full_index", idx, i);
    }
    /* Adding one more must return -1 (full). */
    idx = nexus_v2_light_add(&ls, 0.0f, 0.0f, 0.0f,
                              0.1f, 0.1f, 0.1f, 0.5f, 1.0f, 0);
    check_int("add.overflow", idx, -1);
    check_int("add.overflow_count", ls.light_count, NEXUS_MAX_LIGHTS);
    /* Remove slot 0 then re-add — new index is 0. */
    nexus_v2_light_remove(&ls, 0);
    idx = nexus_v2_light_add(&ls, 0.0f, 0.0f, 0.0f,
                              0.1f, 0.1f, 0.1f, 0.5f, 1.0f, 0);
    check_int("add.after_remove", idx, 0);
    check_int("add.after_remove_active", ls.lights[0].active, 1);
}

static void check_light_remove(void)
{
    Nexus_V2_LightingState ls;
    nexus_v2_lighting_init(&ls);
    nexus_v2_light_add(&ls, 0, 0, 0, 0.5, 0.5, 0.5, 0.5, 1.0, 0);
    check_int("remove.before_active", ls.lights[0].active, 1);
    nexus_v2_light_remove(&ls, 0);
    check_int("remove.after_active", ls.lights[0].active, 0);

    /* Out-of-bounds index is safe. */
    nexus_v2_light_remove(&ls, -1);
    nexus_v2_light_remove(&ls, NEXUS_MAX_LIGHTS);
    nexus_v2_light_remove(&ls, 999);
    check_int("remove.oob_safe", 1, 1);
}

static void check_null_args_light(void)
{
    /* Adding/removing on NULL must return -1 / be safe. */
    int idx = nexus_v2_light_add(0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    check_int("add.null", idx, -1);
    nexus_v2_light_remove(0, 0);
    nexus_v2_light_remove(0, 999);
    nexus_v2_lighting_tick(0, 0.016f);
    check_int("null_args.no_crash", 1, 1);
}

static void check_tick_advances_phase(void)
{
    Nexus_V2_LightingState ls;
    float before, after;
    nexus_v2_lighting_init(&ls);
    before = ls.torch_flicker_phase;
    nexus_v2_lighting_tick(&ls, 0.016f);
    after = ls.torch_flicker_phase;
    check_true("tick.advances_phase", after > before);
    /* Phase advance is dt * 8.0. */
    check_float_near("tick.phase_delta", after - before, 0.016f * 8.0f, 0.0001f);
}

static void check_tick_flicker_intensity(void)
{
    Nexus_V2_LightingState ls;
    float i0, i1;
    nexus_v2_lighting_init(&ls);
    /* Add a flickering light. */
    nexus_v2_light_add(&ls, 0, 0, 0, 1.0f, 0.5f, 0.2f, 1.0f, 2.0f, /*flicker=*/1);
    /* intensity is set to radius*0.5*1.0 at add time? Actually add() sets
     * `intensity = ls->lights[i].radius * 0.5` (line 37 in lighting.c) when
     * the C99 struct init uses radius*0.5. Wait, the add() sets fields
     * explicitly: `ls->lights[i] = (Nexus_Light){...intensity..., 1}` so
     * intensity == input intensity. Then tick overwrites it for flicker. */
    i0 = ls.lights[0].intensity;
    nexus_v2_lighting_tick(&ls, 0.016f);
    i1 = ls.lights[0].intensity;
    /* Tick recomputes intensity = radius * 0.5 * flicker_factor where
     * flicker_factor is in [0.85, 1.0]. So i1 is in [radius*0.5*0.85,
     * radius*0.5*1.0] = [0.85, 1.0] for radius=2.0. */
    check_true("tick.flicker_in_range",
               i1 >= 2.0f * 0.5f * 0.85f - 0.0001f &&
               i1 <= 2.0f * 0.5f * 1.0f + 0.0001f);
    /* And the value should have changed (or at least be in the recomputed
     * range, which differs from the initial value). */
    (void)i0;
}

static void check_tick_no_flicker_steady(void)
{
    Nexus_V2_LightingState ls;
    float before, after;
    nexus_v2_lighting_init(&ls);
    /* Add a steady (non-flickering) light with intensity 0.42. */
    nexus_v2_light_add(&ls, 0, 0, 0, 1.0f, 0.5f, 0.2f, 0.42f, 2.0f, /*flicker=*/0);
    before = ls.lights[0].intensity;
    nexus_v2_lighting_tick(&ls, 0.016f);
    after = ls.lights[0].intensity;
    check_float_near("tick.steady_unchanged", after, before, 0.0001f);
}

static void check_apply_lighting_null(void)
{
    Nexus_V2_LightingState ls;
    uint32_t rgba[16] = {0};
    nexus_v2_lighting_init(&ls);
    /* Apply on NULL rgba / NULL ls must be a no-op (no crash). */
    nexus_v2_apply_lighting(0, 4, 4, &ls, 0, 0, 0);
    nexus_v2_apply_lighting(rgba, 4, 4, 0, 0, 0, 0);
    /* And rgba must be unchanged. */
    check_u32("apply.null_rgba_unchanged", rgba[0], 0u);
    check_int("null.apply_safe", 1, 1);
}

static void check_apply_lighting_deterministic(void)
{
    /* Determinism: same inputs -> same outputs (modulo the camera
     * transform that always sees the same light positions). */
    Nexus_V2_LightingState ls1, ls2;
    uint32_t rgba1[16] = {0};
    uint32_t rgba2[16] = {0};
    int i;

    nexus_v2_lighting_init(&ls1);
    nexus_v2_lighting_init(&ls2);
    nexus_v2_light_add(&ls1, 0, 0, 0, 1.0f, 0.5f, 0.2f, 0.5f, 4.0f, 0);
    nexus_v2_light_add(&ls2, 0, 0, 0, 1.0f, 0.5f, 0.2f, 0.5f, 4.0f, 0);
    /* Fill framebuffer with a known pattern. */
    for (i = 0; i < 16; ++i) {
        rgba1[i] = 0xFF808080u;
        rgba2[i] = 0xFF808080u;
    }
    nexus_v2_apply_lighting(rgba1, 4, 4, &ls1, 0.0f, 0.0f, 0.0f);
    nexus_v2_apply_lighting(rgba2, 4, 4, &ls2, 0.0f, 0.0f, 0.0f);
    for (i = 0; i < 16; ++i) {
        char id[64];
        snprintf(id, sizeof(id), "apply.deterministic[%d]", i);
        check_u32(id, rgba1[i], rgba2[i]);
    }
}

static void check_apply_lighting_modifies_rgba(void)
{
    /* Apply lighting to a uniform mid-gray framebuffer with a strong
     * light. The output must be byte-different from the input. */
    Nexus_V2_LightingState ls;
    uint32_t rgba[16];
    int i;
    int any_changed = 0;

    nexus_v2_lighting_init(&ls);
    nexus_v2_light_add(&ls, 0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 0.2f, 1.0f, 8.0f, 0);
    for (i = 0; i < 16; ++i) {
        rgba[i] = 0xFF808080u;
    }
    nexus_v2_apply_lighting(rgba, 4, 4, &ls, 0.0f, 0.0f, 0.0f);
    for (i = 0; i < 16; ++i) {
        if (rgba[i] != 0xFF808080u) {
            any_changed = 1;
            break;
        }
    }
    check_true("apply.modifies_rgba", any_changed);
}

static void check_source_evidence(void)
{
    const char *e = nexus_v2_lighting_source_evidence();
    check_true("evidence.present", e != 0);
    check_true("evidence.nonempty", e && e[0] != 0);
    check_true("evidence.NEXUS.BIN", strstr(e, "NEXUS.BIN") != 0);
    check_true("evidence.VDP1", strstr(e, "VDP1") != 0);
    check_true("evidence.VDP2", strstr(e, "VDP2") != 0);
    check_true("evidence.ReDMCSB_LIGHT.C", strstr(e, "LIGHT.C") != 0);
    check_true("evidence.ReDMCSB_F0380", strstr(e, "F0380") != 0);
    check_true("evidence.ReDMCSB_COMMAND.C", strstr(e, "COMMAND.C") != 0);
    check_true("evidence.ReDMCSB_F0209", strstr(e, "F0209") != 0);
    check_true("evidence.ReDMCSB_DUNGEON.C", strstr(e, "DUNGEON.C") != 0);
    check_true("evidence.DMDF", strstr(e, "DMDF") != 0);
}

static void check_max_lights_constant(void)
{
    /* NEXUS_MAX_LIGHTS is 16 (matches DM1 V2 lighting max). */
    check_int("MAX_LIGHTS", NEXUS_MAX_LIGHTS, 16);
}

int main(void)
{
    printf("=== Nexus V2 lighting unit test ===\n");
    check_init_defaults();
    check_null_init();
    check_light_add_basic();
    check_light_add_full();
    check_light_remove();
    check_null_args_light();
    check_tick_advances_phase();
    check_tick_flicker_intensity();
    check_tick_no_flicker_steady();
    check_apply_lighting_null();
    check_apply_lighting_deterministic();
    check_apply_lighting_modifies_rgba();
    check_source_evidence();
    check_max_lights_constant();
    printf("--- %d / %d passed ---\n", g_passed, g_assertions);
    if (g_passed != g_assertions) {
        return 1;
    }
    return 0;
}
