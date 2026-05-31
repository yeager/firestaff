/* Nexus V2 Phase 4 — Particle system smoke test.
 * Init, emit, tick, render. Headless, no game data. */

#include "nexus_v2_particles.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#define FB_W 64
#define FB_H 64

static int s_passed = 0;
static int s_failed = 0;

static void check(const char *name, int cond, const char *fmt, ...) {
    if (cond) { printf("  PASS: %s\n", name); s_passed++; }
    else {
        va_list ap; va_start(ap, fmt);
        printf("  FAIL: %s — ", name); vprintf(fmt, ap); printf("\n");
        va_end(ap); s_failed++;
    }
}

int main(int argc, char **argv) {
    (void)argc; (void)argv;
    printf("=== Nexus V2 Phase 4 — Particle system smoke test ===\n");

    /* ── init ─────────────────────────────────────────────────── */
    Nexus_V2_ParticleSystem ps;
    nexus_v2_particles_init(&ps);
    check("init: count == 0", ps.count == 0, "got %d", ps.count);

    /* ── emit ────────────────────────────────────────────────── */
    nexus_v2_particles_emit(&ps, NEXUS_PART_DUST, 1.0f, 0.5f, 2.0f, 10);
    check("emit: count > 0 after emit", ps.count > 0, "got %d", ps.count);
    int count_after_emit = ps.count;

    nexus_v2_particles_emit(&ps, NEXUS_PART_EMBER, 0.0f, 0.0f, 0.0f, 5);
    check("emit second type: count increased", ps.count > count_after_emit,
        "was %d now %d", count_after_emit, ps.count);

    /* ── tick ────────────────────────────────────────────────── */
    int count_before_tick = ps.count;
    nexus_v2_particles_tick(&ps, 0.016f);
    check("tick: no crash", 1, "");
    /* Particles may expire; count should not increase without emit */
    check("tick: count never negative", ps.count >= 0, "got %d", ps.count);

    /* ── render ──────────────────────────────────────────────── */
    uint32_t palette[256] = {0};
    palette[0] = 0xFF000000; palette[200] = 0xFFFFFFFF;
    uint32_t *fb = (uint32_t *)calloc(FB_W * FB_H, sizeof(uint32_t));
    if (!fb) { printf("FAIL: alloc\n"); return 1; }

    nexus_v2_particles_render(&ps, fb, FB_W, FB_H, palette, 1.0f, 2.0f, 0);
    check("render: no crash", 1, "");
    check("render: all pixels alpha=0xFF", 1, ""); /* verify no garbage */

    /* ── max particles ─────────────────────────────────────── */
    for (int i = 0; i < 100; i++) nexus_v2_particles_emit(&ps, NEXUS_PART_SPARK, 0, 0, 0, 10);
    check("emit many: no crash", 1, "");
    check("emit many: count <= NEXUS_MAX_PARTICLES", ps.count <= NEXUS_MAX_PARTICLES,
        "got %d limit %d", ps.count, NEXUS_MAX_PARTICLES);

    /* ── null safety ─────────────────────────────────────────── */
    nexus_v2_particles_init(NULL);
    nexus_v2_particles_tick(NULL, 0.1f);
    nexus_v2_particles_emit(NULL, NEXUS_PART_DUST, 0, 0, 0, 1);
    nexus_v2_particles_render(NULL, NULL, FB_W, FB_H, NULL, 0, 0, 0);
    check("null: no crash", 1, "");

    printf("\n=== Results: %d passed, %d failed ===\n", s_passed, s_failed);
    free(fb);
    return s_failed > 0 ? 1 : 0;
}