/* test_theron_v2_filter_config_pc34.c
 *
 * Theron V2 filter config test. Parallel to test_csb_v2_filter_config_pc34.c
 * but for the PC Engine CD (HuC6260 VDC + HuC6270 VCE) Theron pipeline.
 */
#include "theron_v2_filter_config_pc34.h"
#include "theron_v2_settings_pc34.h"
#include "theron_v2_texture_upscale_pc34.h"
#include <stdio.h>
#include <string.h>
static int failures = 0;
static int total = 0;
static void check(int cond, const char* name) {
    total++;
    if (!cond) { failures++; fprintf(stderr, "FAIL: %s\n", name); }
    else printf("PASS: %s\n", name);
}

static void t_defaults(void) {
    Theron_V2_FilterConfig fc;
    theron_v2_filter_config_defaults(&fc);
    check(fc.crtScanlinesEnabled == 0, "default scanlines=0");
    check(fc.crtScanlineStrength == 35, "default scanline strength=35");
    check(fc.paletteCorrectionEnabled == 0, "default palette=0");
    check(fc.ditherCleanupEnabled == 0, "default dither=0");
}

static void t_sanitize(void) {
    Theron_V2_FilterConfig fc;
    theron_v2_filter_config_defaults(&fc);
    fc.crtScanlinesEnabled = 2;
    fc.crtScanlineStrength = -5;
    fc.paletteCorrectionEnabled = 5;
    fc.ditherCleanupEnabled = 99;
    theron_v2_filter_config_sanitize(&fc);
    check(fc.crtScanlinesEnabled == 1, "scanlines clamp 1");
    check(fc.crtScanlineStrength == 0, "scanline strength clamp 0");
    check(fc.paletteCorrectionEnabled == 1, "palette clamp 1");
    check(fc.ditherCleanupEnabled == 1, "dither clamp 1");
}

static void t_apply_get_roundtrip(void) {
    Theron_V2_FilterConfig fc;
    theron_v2_filter_config_defaults(&fc);
    fc.crtScanlinesEnabled = 1;
    fc.crtScanlineStrength = 60;
    fc.paletteCorrectionEnabled = 1;
    fc.ditherCleanupEnabled = 0;
    theron_v2_filter_config_apply(&fc);
    const Theron_V2_FilterConfig* live = theron_v2_filter_config_get();
    check(live->crtScanlinesEnabled == 1, "live scanlines=1");
    check(live->crtScanlineStrength == 60, "live scanline strength=60");
    check(live->paletteCorrectionEnabled == 1, "live palette=1");
    check(live->ditherCleanupEnabled == 0, "live dither=0");
}

static void t_apply_null_uses_defaults(void) {
    theron_v2_filter_config_apply(NULL);
    const Theron_V2_FilterConfig* live = theron_v2_filter_config_get();
    check(live->crtScanlinesEnabled == 0, "null -> defaults: scanlines=0");
    check(live->crtScanlineStrength == 35, "null -> defaults: strength=35");
}

static void t_reset(void) {
    Theron_V2_FilterConfig fc;
    theron_v2_filter_config_defaults(&fc);
    fc.crtScanlinesEnabled = 1;
    theron_v2_filter_config_apply(&fc);
    theron_v2_filter_config_reset();
    check(theron_v2_filter_config_get()->crtScanlinesEnabled == 0, "reset");
}

static void t_wireup_from_settings(void) {
    Theron_V2_Settings s;
    theron_v2_settings_defaults(&s);
    s.crtScanlinesEnabled = 1;
    s.crtScanlineStrength = 90;
    s.paletteCorrectionEnabled = 0;
    s.ditherCleanupEnabled = 1;
    theron_v2_settings_apply_to_runtime(&s);
    const Theron_V2_FilterConfig* live = theron_v2_filter_config_get();
    check(live->crtScanlinesEnabled == 1, "wireup scanlines=1");
    check(live->crtScanlineStrength == 90, "wireup strength=90");
    check(live->paletteCorrectionEnabled == 0, "wireup palette=0");
    check(live->ditherCleanupEnabled == 1, "wireup dither=1");
}

static void t_independent_from_upscale(void) {
    Theron_V2_Settings s;
    theron_v2_settings_defaults(&s);
    s.crtScanlinesEnabled = 1;
    theron_v2_settings_apply_to_runtime(&s);
    check(theron_v2_upscale_get_scale() == 2, "upscale independent: scale=2");
    check(theron_v2_filter_config_get()->crtScanlinesEnabled == 1,
          "filter config reflects scanlines=1");
}

static void t_evidence(void) {
    const char* ev = theron_v2_filter_config_source_evidence();
    check(ev != NULL && strlen(ev) > 50, "ev non-trivial");
    check(strstr(ev, "Theron") != NULL, "ev Theron");
    check(strstr(ev, "HuC") != NULL, "ev HuC");
}

int main(void) {
    printf("=== Theron V2 filter config test ===\n");
    t_defaults();
    t_sanitize();
    t_apply_get_roundtrip();
    t_apply_null_uses_defaults();
    t_reset();
    t_wireup_from_settings();
    t_independent_from_upscale();
    t_evidence();
    printf("--- %d / %d passed ---\n", total - failures, total);
    return failures == 0 ? 0 : 1;
}
