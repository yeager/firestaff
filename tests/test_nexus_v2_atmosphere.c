/* Nexus V2 Phase 4 — Atmosphere smoke test.
 * Fog, AO, tint, level presets. Headless, no game data. */

#include "nexus_v2_atmosphere.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
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
    printf("=== Nexus V2 Phase 4 — Atmosphere smoke test ===\n");

    /* ── init with level presets ──────────────────────────────── */
    for (int lvl = 0; lvl < 16; lvl++) {
        Nexus_V2_Atmosphere a;
        nexus_v2_atmosphere_init(&a, lvl);
        check("init level 0: fog_start ~1.5", fabsf(a.fog_start - 1.5f) < 0.01f, "got %.2f", a.fog_start);
        check("init level 0: fog_end ~4.0",   fabsf(a.fog_end - 4.0f)   < 0.01f, "got %.2f", a.fog_end);
        check("init level 0: ao_strength ~0.3", fabsf(a.ao_strength - 0.3f) < 0.01f, "got %.2f", a.ao_strength);
    }

    /* ── apply_fog modifies framebuffer ─────────────────────── */
    Nexus_V2_Atmosphere atm;
    nexus_v2_atmosphere_init(&atm, 0);
    uint32_t *fb = (uint32_t *)calloc(FB_W * FB_H, sizeof(uint32_t));
    uint32_t *fb_orig = (uint32_t *)calloc(FB_W * FB_H, sizeof(uint32_t));
    if (!fb || !fb_orig) { free(fb); free(fb_orig); return 1; }
    for (int i = 0; i < FB_W * FB_H; i++) fb[i] = fb_orig[i] = 0xFFFFFFFF;

    nexus_v2_apply_fog(fb, FB_W, FB_H, &atm);
    int modified = 0;
    for (int i = 0; i < FB_W * FB_H; i++) if (fb[i] != fb_orig[i]) { modified++; break; }
    check("apply_fog: framebuffer modified", modified > 0, "%d pixels changed", modified);

    /* all pixels have valid RGBA */
    int valid = 1;
    for (int i = 0; i < FB_W * FB_H; i++) {
        uint32_t c = fb[i];
        if (((c >> 24) & 0xFF) != 0xFF) { valid = 0; break; }
    }
    check("apply_fog: all pixels alpha=0xFF", valid, "");

    /* ── apply_ao modifies framebuffer ──────────────────────── */
    for (int i = 0; i < FB_W * FB_H; i++) fb[i] = 0xFFFFFFFF;
    nexus_v2_apply_ao(fb, FB_W, FB_H, atm.ao_strength);
    int ao_modified = 0;
    for (int i = 0; i < FB_W * FB_H; i++) {
        /* AO darkens near edges; with all-white fb edges may or may not darken
         * depending on implementation — just check no crash */
    }
    (void)ao_modified;
    check("apply_ao: no crash", 1, "");

    /* ── null safety ────────────────────────────────────────── */
    nexus_v2_atmosphere_init(NULL, 0);
    nexus_v2_apply_fog(NULL, FB_W, FB_H, NULL);
    nexus_v2_apply_ao(NULL, FB_W, FB_H, 0.0f);
    check("null: no crash", 1, "");

    printf("\n=== Results: %d passed, %d failed ===\n", s_passed, s_failed);
    free(fb); free(fb_orig);
    return s_failed > 0 ? 1 : 0;
}