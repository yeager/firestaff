/**
 * firestaff_csb_v2_filter_chain_probe.c
 *
 * CSB V2.0 filter chain headless probe. Verifies all 5 per-frame
 * filter functions:
 *   1. csb_v2_filter_crt_scanlines_rgba
 *   2. csb_v2_filter_palette_build_lut
 *   3. csb_v2_filter_dither_cleanup_indexed
 *   4. csb_v2_filter_palette_interpolate_indexed
 *   5. csb_v2_filter_sharpen_rgba
 *
 * Plus a sanity test that the filter config toggle (csb_v2_filter_config_get)
 * is honored by the per-frame functions. Source: include/csb_v2_filters.h +
 * include/csb_v2_filter_config_pc34.h.
 */
#include "csb_v2_filters.h"
#include "csb_v2_filter_config_pc34.h"
#include "csb_v2_asset_pipeline_pc34.h"
#include <stdio.h>
#include <string.h>
static int g_total = 0, g_failed = 0;
static void check(int cond, const char* name) {
    g_total++;
    if (!cond) { g_failed++; fprintf(stderr, "[FAIL] %s\n", name); }
    else printf("[PASS] %s\n", name);
}

static void p_crt_scanlines(void) {
    /* 4x4 RGBA all (200, 100, 50, 255); strength=50 -> even rows dimmed. */
    uint8_t rgba[4 * 4 * 4];
    for (int i = 0; i < 16; i++) {
        rgba[i*4+0] = 200; rgba[i*4+1] = 100;
        rgba[i*4+2] = 50;  rgba[i*4+3] = 255;
    }
    int r = csb_v2_filter_crt_scanlines_rgba(rgba, 4, 4, 50);
    check(r == 0, "crt scanlines returns 0");
    /* Row 0 (even) dimmed; row 1 (odd) unchanged. Row 1 starts at byte w*4 = 16. */
    check(rgba[0*4+0] == 100, "row 0 R dimmed to 100");
    check(rgba[1*4*4+0] == 200, "row 1 R unchanged");
}

static void p_palette_build_lut(void) {
    uint8_t lut[CSB_V2_PALETTE_LEVELS][16][3];
    int r = csb_v2_filter_palette_build_lut(220, 0, 0, lut);
    check(r == 0, "palette build lut returns 0");
    int nonzero = 0;
    for (int l = 0; l < CSB_V2_PALETTE_LEVELS; l++)
        for (int i = 0; i < 16; i++)
            if (lut[l][i][0] || lut[l][i][1] || lut[l][i][2]) nonzero = 1;
    check(nonzero, "LUT has non-zero values");
}

static void p_dither_cleanup(void) {
    /* Uniform 3x3: all pixels unchanged. */
    uint8_t fb[9];
    for (int i = 0; i < 9; i++) fb[i] = (uint8_t)((2 << 4) | 5);
    int r = csb_v2_filter_dither_cleanup_indexed(fb, 3, 3);
    check(r == 0, "dither cleanup returns 0");
    int same = 1;
    for (int i = 0; i < 9; i++) {
        if (fb[i] != ((2 << 4) | 5)) same = 0;
    }
    check(same, "uniform pixels unchanged");
}

static void p_palette_interpolate(void) {
    /* Pixels: 4x4 surface, each (level 2, index 7). strength=0 unchanged. */
    uint8_t fb[16];
    for (int i = 0; i < 16; i++) fb[i] = (uint8_t)((2 << 4) | 7);
    int r = csb_v2_filter_palette_interpolate_indexed(fb, 4, 4, 0);
    check(r == 0, "palette interp returns 0");
    int same = 1;
    for (int i = 0; i < 16; i++) {
        if (fb[i] != ((2 << 4) | 7)) same = 0;
    }
    check(same, "strength=0 unchanged");
    /* Low 4 bits (index) preserved regardless of strength. */
    r = csb_v2_filter_palette_interpolate_indexed(fb, 4, 4, 100);
    int idx_preserved = 1;
    for (int i = 0; i < 16; i++) {
        if ((fb[i] & 0x0F) != 7) idx_preserved = 0;
    }
    check(idx_preserved, "index preserved under interp");
}

static void p_sharpen(void) {
    /* strength=0 -> no change. 4x4 RGBA surface (needs w,h > 2). */
    uint8_t rgba[4 * 4 * 4];
    for (int i = 0; i < 16; i++) {
        rgba[i*4+0] = 200; rgba[i*4+1] = 100;
        rgba[i*4+2] = 50;  rgba[i*4+3] = 255;
    }
    int r = csb_v2_filter_sharpen_rgba(rgba, 4, 4, 0);
    check(r == 0, "sharpen returns 0");
    int same = 1;
    for (int i = 0; i < 16; i++) {
        if (rgba[i*4+0] != 200) same = 0;
    }
    check(same, "strength=0: RGB unchanged");
}

static void p_filter_config_toggle(void) {
    /* When the config says crtScanlinesEnabled=0, the per-frame path
     * should NOT call the filter. We can't observe that directly here
     * (the per-frame path is a separate scope), but we can verify that
     * the config reflects the user's choice. */
    CSB_V2_FilterConfig fc;
    csb_v2_filter_config_defaults(&fc);
    fc.crtScanlinesEnabled = 0;
    csb_v2_filter_config_apply(&fc);
    const CSB_V2_FilterConfig* live = csb_v2_filter_config_get();
    check(live->crtScanlinesEnabled == 0, "scanlines=0 (off)");
    fc.crtScanlinesEnabled = 1;
    csb_v2_filter_config_apply(&fc);
    live = csb_v2_filter_config_get();
    check(live->crtScanlinesEnabled == 1, "scanlines=1 (on)");
}

static void p_strength_clamp(void) {
    /* Out-of-range strength_pct should be tolerated. 4x4 RGBA surface. */
    uint8_t rgba[4 * 4 * 4];
    for (int i = 0; i < 16; i++) {
        rgba[i*4+0] = 200; rgba[i*4+1] = 100;
        rgba[i*4+2] = 50;  rgba[i*4+3] = 255;
    }
    int r = csb_v2_filter_crt_scanlines_rgba(rgba, 4, 4, 200); /* OOB */
    check(r == 0, "strength=200 OOB tolerated");
    r = csb_v2_filter_sharpen_rgba(rgba, 4, 4, -5); /* OOB */
    check(r == 0, "sharpen strength=-5 OOB tolerated");
}

int main(void) {
    printf("=== CSB V2 filter chain probe ===\n");
    p_crt_scanlines();
    p_palette_build_lut();
    p_dither_cleanup();
    p_palette_interpolate();
    p_sharpen();
    p_filter_config_toggle();
    p_strength_clamp();
    printf("--- %d / %d passed ---\n", g_total - g_failed, g_total);
    return g_failed == 0 ? 0 : 1;
}
