/*
 * firestaff_nexus_v2_lighting_probe.c
 *
 * Nexus V2.2 dynamic lighting headless probe.
 *
 * Headless probe: verifies nexus_v2_lighting.c without requiring
 * live game asset files or a running SDL renderer.
 *
 * This probe validates:
 *
 *   1. nexus_v2_lighting_init() zeroes state and sets ambient defaults
 *
 *   2. nexus_v2_light_add() inserts lights, returns valid index,
 *      NEXUS_MAX_LIGHTS = 16
 *
 *   3. nexus_v2_light_remove() clears active flag, OOB index is safe
 *
 *   4. nexus_v2_lighting_tick() advances torch_flicker_phase and
 *      updates flickering lights' intensity into [radius*0.5*0.85,
 *      radius*0.5*1.0] range
 *
 *   5. nexus_v2_apply_lighting() applies ambient + per-light contribution
 *      to RGBA framebuffer, in-place, deterministic for same inputs
 *
 *   6. Null-args are safe
 *
 *   7. Source evidence references NEXUS.BIN / VDP1 / VDP2 / ReDMCSB
 *
 *   8. NEXUS_MAX_LIGHTS constant pinned
 *
 * Exit codes:
 *   0  - all checks passed
 *   1  - one or more checks failed
 *
 * Usage:
 *   SDL_VIDEODRIVER=dummy ./firestaff_nexus_v2_lighting_probe
 *
 * Source references:
 *   Saturn NEXUS.BIN            VDP1 polygon lighting / VDP2 shadow layer
 *   DMDF level data             per-tile light emission values (DGN format)
 *   ReDMCSB LIGHT.C F0380       light radius, flicker timing
 *   ReDMCSB COMMAND.C F0209     spell-light colour binding
 *   ReDMCSB DUNGEON.C           torch position tracking in party state
 */

#include "nexus_v2_lighting.h"

#include <stdio.h>
#include <string.h>

static int g_total = 0;
static int g_failed = 0;

static void check(int cond, const char *name)
{
    ++g_total;
    if (!cond) {
        ++g_failed;
        printf("[FAIL] %s\n", name);
    } else {
        printf("[PASS] %s\n", name);
    }
}

static void check_init(void)
{
    Nexus_V2_LightingState ls;
    int all_inactive = 1;
    int i;

    memset(&ls, 0x55, sizeof(ls));
    nexus_v2_lighting_init(&ls);
    check(ls.light_count == 0, "init: light_count == 0");
    for (i = 0; i < NEXUS_MAX_LIGHTS; ++i) {
        if (ls.lights[i].active != 0) {
            all_inactive = 0;
            break;
        }
    }
    check(all_inactive, "init: all 16 lights inactive");
    check(ls.torch_flicker_phase == 0.0f,
          "init: torch_flicker_phase == 0");
    check(ls.ambient_r == 0.15f, "init: ambient_r == 0.15");
    check(ls.ambient_g == 0.12f, "init: ambient_g == 0.12");
    check(ls.ambient_b == 0.10f, "init: ambient_b == 0.10");
}

static void check_null_init(void)
{
    /* Null-arg init must not crash. */
    nexus_v2_lighting_init(0);
    check(1, "null init safe");
}

static void check_add(void)
{
    Nexus_V2_LightingState ls;
    int idx;
    int i;

    nexus_v2_lighting_init(&ls);
    idx = nexus_v2_light_add(&ls, 1, 2, 3, 0.5f, 0.6f, 0.7f, 0.8f, 4.0f, 1);
    check(idx == 0, "add: first light index == 0");
    check(ls.lights[0].active == 1, "add: first light active");
    check(ls.lights[0].x == 1.0f, "add: x pinned");
    check(ls.lights[0].y == 2.0f, "add: y pinned");
    check(ls.lights[0].z == 3.0f, "add: z pinned");
    check(ls.lights[0].r == 0.5f, "add: r pinned");
    check(ls.lights[0].g == 0.6f, "add: g pinned");
    check(ls.lights[0].b == 0.7f, "add: b pinned");
    check(ls.lights[0].intensity == 0.8f, "add: intensity pinned");
    check(ls.lights[0].radius == 4.0f, "add: radius pinned");
    check(ls.lights[0].flicker == 1, "add: flicker pinned");
    check(ls.light_count == 1, "add: light_count == 1");

    /* Fill the rest. */
    for (i = 1; i < NEXUS_MAX_LIGHTS; ++i) {
        idx = nexus_v2_light_add(&ls, 0, 0, 0, 0.1f, 0.1f, 0.1f, 0.5f, 1.0f, 0);
        check(idx == i, "add: sequential index after first");
    }
    check(ls.light_count == NEXUS_MAX_LIGHTS, "add: light_count == 16");

    /* Overflow. */
    idx = nexus_v2_light_add(&ls, 0, 0, 0, 0.1f, 0.1f, 0.1f, 0.5f, 1.0f, 0);
    check(idx == -1, "add: overflow returns -1");

    /* Add on NULL returns -1. */
    idx = nexus_v2_light_add(0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    check(idx == -1, "add: NULL state returns -1");
}

static void check_remove(void)
{
    Nexus_V2_LightingState ls;

    nexus_v2_lighting_init(&ls);
    nexus_v2_light_add(&ls, 0, 0, 0, 0.5f, 0.5f, 0.5f, 0.5f, 1.0f, 0);
    check(ls.lights[0].active == 1, "remove: pre active");
    nexus_v2_light_remove(&ls, 0);
    check(ls.lights[0].active == 0, "remove: post active");

    /* OOB indices are safe. */
    nexus_v2_light_remove(&ls, -1);
    nexus_v2_light_remove(&ls, NEXUS_MAX_LIGHTS);
    nexus_v2_light_remove(&ls, 9999);
    nexus_v2_light_remove(0, 0);
    check(1, "remove: OOB + NULL safe");
}

static void check_tick(void)
{
    Nexus_V2_LightingState ls;
    float before, after;

    nexus_v2_lighting_init(&ls);
    before = ls.torch_flicker_phase;
    nexus_v2_lighting_tick(&ls, 0.016f);
    after = ls.torch_flicker_phase;
    check(after > before, "tick: phase advances");
    /* dt*8.0 advance. */
    check(after - before >= 0.016f * 8.0f - 0.0001f &&
          after - before <= 0.016f * 8.0f + 0.0001f,
          "tick: phase advance is dt*8.0");

    /* Flickering light intensity range. */
    nexus_v2_light_add(&ls, 0, 0, 0, 1.0f, 0.5f, 0.2f, 1.0f, 2.0f, /*flicker=*/1);
    nexus_v2_lighting_tick(&ls, 0.016f);
    /* intensity = radius * 0.5 * flicker where flicker in [0.85, 1.0]. */
    check(ls.lights[0].intensity >= 2.0f * 0.5f * 0.85f - 0.0001f &&
          ls.lights[0].intensity <= 2.0f * 0.5f * 1.0f + 0.0001f,
          "tick: flickering intensity in expected range");

    /* Steady light intensity unchanged. */
    Nexus_V2_LightingState ls2;
    float before2, after2;
    nexus_v2_lighting_init(&ls2);
    nexus_v2_light_add(&ls2, 0, 0, 0, 1.0f, 0.5f, 0.2f, 0.42f, 2.0f, /*flicker=*/0);
    before2 = ls2.lights[0].intensity;
    nexus_v2_lighting_tick(&ls2, 0.016f);
    after2 = ls2.lights[0].intensity;
    check(before2 == after2, "tick: steady intensity unchanged");

    /* Tick on NULL is safe. */
    nexus_v2_lighting_tick(0, 0.016f);
    check(1, "tick: NULL safe");
}

static void check_apply(void)
{
    Nexus_V2_LightingState ls;
    uint32_t rgba1[16];
    uint32_t rgba2[16];
    int i;
    int any_changed = 0;

    /* Null-arg apply is safe. */
    nexus_v2_lighting_init(&ls);
    nexus_v2_apply_lighting(0, 4, 4, &ls, 0, 0, 0);
    for (i = 0; i < 16; ++i) rgba1[i] = 0xFF808080u;
    nexus_v2_apply_lighting(rgba1, 4, 4, 0, 0, 0, 0);
    check(rgba1[0] == 0xFF808080u, "apply: NULL state leaves rgba unchanged");

    /* Determinism. */
    for (i = 0; i < 16; ++i) {
        rgba1[i] = 0xFF808080u;
        rgba2[i] = 0xFF808080u;
    }
    nexus_v2_light_add(&ls, 0, 0, 0, 1.0f, 0.5f, 0.2f, 0.5f, 4.0f, 0);
    nexus_v2_apply_lighting(rgba1, 4, 4, &ls, 0.0f, 0.0f, 0.0f);
    nexus_v2_apply_lighting(rgba2, 4, 4, &ls, 0.0f, 0.0f, 0.0f);
    int same = 1;
    for (i = 0; i < 16; ++i) {
        if (rgba1[i] != rgba2[i]) {
            same = 0;
            break;
        }
    }
    check(same, "apply: deterministic for same inputs");

    /* Modification. */
    for (i = 0; i < 16; ++i) {
        rgba1[i] = 0xFF808080u;
    }
    nexus_v2_light_add(&ls, 0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 0.2f, 1.0f, 8.0f, 0);
    nexus_v2_apply_lighting(rgba1, 4, 4, &ls, 0.0f, 0.0f, 0.0f);
    for (i = 0; i < 16; ++i) {
        if (rgba1[i] != 0xFF808080u) {
            any_changed = 1;
            break;
        }
    }
    check(any_changed, "apply: modifies rgba with strong light");
}

static void check_max_lights_constant(void)
{
    check(NEXUS_MAX_LIGHTS == 16, "NEXUS_MAX_LIGHTS == 16");
}

static void check_source_evidence(void)
{
    const char *e = nexus_v2_lighting_source_evidence();
    check(e != 0 && e[0] != 0, "evidence: present + non-empty");
    check(e && strstr(e, "NEXUS.BIN") != 0, "evidence: NEXUS.BIN");
    check(e && strstr(e, "VDP1") != 0, "evidence: VDP1 (Saturn VDP1)");
    check(e && strstr(e, "VDP2") != 0, "evidence: VDP2 (Saturn VDP2)");
    check(e && strstr(e, "LIGHT.C") != 0, "evidence: ReDMCSB LIGHT.C");
    check(e && strstr(e, "F0380") != 0, "evidence: F0380 (light radius, flicker)");
    check(e && strstr(e, "COMMAND.C") != 0, "evidence: ReDMCSB COMMAND.C");
    check(e && strstr(e, "F0209") != 0, "evidence: F0209 (spell-light binding)");
    check(e && strstr(e, "DUNGEON.C") != 0, "evidence: ReDMCSB DUNGEON.C");
    check(e && strstr(e, "DMDF") != 0, "evidence: DMDF (DGN format)");
}

int main(void)
{
    printf("=== Nexus V2 Lighting Probe ===\n");
    check_init();
    check_null_init();
    check_add();
    check_remove();
    check_tick();
    check_apply();
    check_max_lights_constant();
    check_source_evidence();
    printf("--- %d / %d passed ---\n", g_total - g_failed, g_total);
    return g_failed == 0 ? 0 : 1;
}
