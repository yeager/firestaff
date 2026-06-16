/**
 * firestaff_theron_v2_filter_config_probe.c
 *
 * Theron V2 filter config headless probe. Parallel to
 * firestaff_csb_v2_filter_config_probe but for the PC Engine CD
 * (HuC6260 VDC + HuC6270 VCE) Theron pipeline.
 */
#include "theron_v2_filter_config_pc34.h"
#include "theron_v2_settings_pc34.h"
#include "theron_v2_texture_upscale_pc34.h"
#include <stdio.h>
#include <string.h>
static int g_total = 0, g_failed = 0;
static void check(int cond, const char* name) {
    g_total++;
    if (!cond) { g_failed++; fprintf(stderr, "[FAIL] %s\n", name); }
    else printf("[PASS] %s\n", name);
}

static void p_defaults(void) {
    theron_v2_filter_config_reset();
    const Theron_V2_FilterConfig* fc = theron_v2_filter_config_get();
    check(fc->crtScanlinesEnabled == 0, "default scanlines=0");
    check(fc->crtScanlineStrength == 35, "default strength=35");
    check(fc->paletteCorrectionEnabled == 0, "default palette=0");
    check(fc->ditherCleanupEnabled == 0, "default dither=0");
}

static void p_apply_get(void) {
    Theron_V2_FilterConfig fc;
    theron_v2_filter_config_defaults(&fc);
    fc.crtScanlinesEnabled = 1;
    fc.crtScanlineStrength = 70;
    theron_v2_filter_config_apply(&fc);
    const Theron_V2_FilterConfig* live = theron_v2_filter_config_get();
    check(live->crtScanlinesEnabled == 1, "live scanlines=1");
    check(live->crtScanlineStrength == 70, "live strength=70");
}

static void p_wireup(void) {
    theron_v2_filter_config_reset();
    Theron_V2_Settings s;
    theron_v2_settings_defaults(&s);
    s.scalePercent = 400;
    s.bilinearEnabled = 0;
    s.crtScanlinesEnabled = 1;
    s.crtScanlineStrength = 90;
    s.paletteCorrectionEnabled = 1;
    s.ditherCleanupEnabled = 1;
    theron_v2_settings_apply_to_runtime(&s);
    check(theron_v2_upscale_get_scale() == 4, "wireup: upscale scale=4");
    check(theron_v2_upscale_get_bilinear() == 0, "wireup: bilinear=0");
    const Theron_V2_FilterConfig* fc = theron_v2_filter_config_get();
    check(fc->crtScanlinesEnabled == 1, "wireup: scanlines=1");
    check(fc->crtScanlineStrength == 90, "wireup: strength=90");
    check(fc->paletteCorrectionEnabled == 1, "wireup: palette=1");
    check(fc->ditherCleanupEnabled == 1, "wireup: dither=1");
}

static void p_clamp(void) {
    Theron_V2_FilterConfig fc;
    theron_v2_filter_config_defaults(&fc);
    fc.crtScanlineStrength = 999;
    theron_v2_filter_config_apply(&fc);
    check(theron_v2_filter_config_get()->crtScanlineStrength == 100, "strength=999 clamps to 100");
}

static void p_independent(void) {
    Theron_V2_FilterConfig fc;
    theron_v2_filter_config_defaults(&fc);
    fc.crtScanlinesEnabled = 1;
    theron_v2_filter_config_apply(&fc);
    fc.crtScanlinesEnabled = 0;
    fc.ditherCleanupEnabled = 1;
    theron_v2_filter_config_apply(&fc);
    const Theron_V2_FilterConfig* live = theron_v2_filter_config_get();
    check(live->crtScanlinesEnabled == 0, "scanlines reset to 0");
    check(live->ditherCleanupEnabled == 1, "dither set to 1");
}

static void p_evidence(void) {
    const char* ev = theron_v2_filter_config_source_evidence();
    check(ev != NULL && strlen(ev) > 50, "ev non-trivial");
    check(strstr(ev, "Theron") != NULL, "ev Theron");
    check(strstr(ev, "HuC") != NULL, "ev HuC");
}

int main(void) {
    printf("=== Theron V2 filter config probe ===\n");
    p_defaults();
    p_apply_get();
    p_wireup();
    p_clamp();
    p_independent();
    p_evidence();
    printf("--- %d / %d passed ---\n", g_total - g_failed, g_total);
    return g_failed == 0 ? 0 : 1;
}
