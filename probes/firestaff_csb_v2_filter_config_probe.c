/**
 * firestaff_csb_v2_filter_config_probe.c
 *
 * CSB V2 filter config headless probe. Verifies that the per-frame
 * filter toggles (CRT scanlines, palette correction, dither cleanup)
 * are correctly stored in the live global and that the M11 launch
 * wire-up (csb_v2_settings_apply_to_runtime) pushes the right
 * values into both the upscale config and the filter config.
 */
#include "csb_v2_filter_config_pc34.h"
#include "csb_v2_settings_pc34.h"
#include "csb_v2_texture_upscale_pc34.h"
#include <stdio.h>
#include <string.h>
static int g_total = 0, g_failed = 0;
static void check(int cond, const char* name) {
    g_total++;
    if (!cond) { g_failed++; fprintf(stderr, "[FAIL] %s\n", name); }
    else printf("[PASS] %s\n", name);
}

static void p_defaults(void) {
    csb_v2_filter_config_reset();
    const CSB_V2_FilterConfig* fc = csb_v2_filter_config_get();
    check(fc->crtScanlinesEnabled == 0, "default scanlines=0");
    check(fc->crtScanlineStrength == 35, "default strength=35");
    check(fc->paletteCorrectionEnabled == 0, "default palette=0");
    check(fc->ditherCleanupEnabled == 0, "default dither=0");
}

static void p_apply_get(void) {
    CSB_V2_FilterConfig fc;
    csb_v2_filter_config_defaults(&fc);
    fc.crtScanlinesEnabled = 1;
    fc.crtScanlineStrength = 50;
    fc.paletteCorrectionEnabled = 1;
    fc.ditherCleanupEnabled = 1;
    csb_v2_filter_config_apply(&fc);
    const CSB_V2_FilterConfig* live = csb_v2_filter_config_get();
    check(live->crtScanlinesEnabled == 1, "live scanlines=1");
    check(live->crtScanlineStrength == 50, "live strength=50");
    check(live->paletteCorrectionEnabled == 1, "live palette=1");
    check(live->ditherCleanupEnabled == 1, "live dither=1");
}

static void p_wireup(void) {
    csb_v2_filter_config_reset();
    CSB_V2_Settings s;
    csb_v2_settings_defaults(&s);
    s.scalePercent = 400;
    s.bilinearEnabled = 1;
    s.crtScanlinesEnabled = 1;
    s.crtScanlineStrength = 80;
    s.paletteCorrectionEnabled = 1;
    s.ditherCleanupEnabled = 0;
    csb_v2_settings_apply_to_runtime(&s);
    /* Upscale: scale=4, bilinear=1 */
    check(csb_v2_upscale_get_scale() == 4, "wireup: upscale scale=4");
    check(csb_v2_upscale_get_bilinear() == 1, "wireup: bilinear=1");
    /* Filter config: scanlines=1, strength=80, palette=1, dither=0 */
    const CSB_V2_FilterConfig* fc = csb_v2_filter_config_get();
    check(fc->crtScanlinesEnabled == 1, "wireup: scanlines=1");
    check(fc->crtScanlineStrength == 80, "wireup: strength=80");
    check(fc->paletteCorrectionEnabled == 1, "wireup: palette=1");
    check(fc->ditherCleanupEnabled == 0, "wireup: dither=0");
}

static void p_clamp(void) {
    CSB_V2_FilterConfig fc;
    csb_v2_filter_config_defaults(&fc);
    fc.crtScanlineStrength = 999;
    csb_v2_filter_config_apply(&fc);
    check(csb_v2_filter_config_get()->crtScanlineStrength == 100, "strength=999 clamps to 100");
    fc.crtScanlineStrength = -5;
    csb_v2_filter_config_apply(&fc);
    check(csb_v2_filter_config_get()->crtScanlineStrength == 0, "strength=-5 clamps to 0");
}

static void p_independent(void) {
    /* Two consecutive applies don't leak: only the latest survives. */
    CSB_V2_FilterConfig fc1;
    csb_v2_filter_config_defaults(&fc1);
    fc1.crtScanlinesEnabled = 1;
    csb_v2_filter_config_apply(&fc1);
    CSB_V2_FilterConfig fc2;
    csb_v2_filter_config_defaults(&fc2);
    fc2.crtScanlinesEnabled = 0;
    fc2.ditherCleanupEnabled = 1;
    csb_v2_filter_config_apply(&fc2);
    const CSB_V2_FilterConfig* live = csb_v2_filter_config_get();
    check(live->crtScanlinesEnabled == 0, "scanlines reset to 0");
    check(live->ditherCleanupEnabled == 1, "dither set to 1");
}

static void p_evidence(void) {
    const char* ev = csb_v2_filter_config_source_evidence();
    check(ev != NULL && strlen(ev) > 50, "ev non-trivial");
    check(strstr(ev, "CSB") != NULL, "ev CSB");
    check(strstr(ev, "filter") != NULL, "ev filter");
}

int main(void) {
    printf("=== CSB V2 filter config probe ===\n");
    p_defaults();
    p_apply_get();
    p_wireup();
    p_clamp();
    p_independent();
    p_evidence();
    printf("--- %d / %d passed ---\n", g_total - g_failed, g_total);
    return g_failed == 0 ? 0 : 1;
}
