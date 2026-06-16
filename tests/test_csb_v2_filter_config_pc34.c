/* test_csb_v2_filter_config_pc34.c
 *
 * CSB V2 filter config test. Verifies that the per-frame filter
 * toggles (CRT scanlines, palette correction, dither cleanup) are
 * correctly stored in the global config and round-trip through
 * csb_v2_settings_apply_to_runtime (the M11 launch wire-up).
 */
#include "csb_v2_filter_config_pc34.h"
#include "csb_v2_settings_pc34.h"
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
    CSB_V2_FilterConfig fc;
    csb_v2_filter_config_defaults(&fc);
    check(fc.crtScanlinesEnabled == 0, "default scanlines=0");
    check(fc.crtScanlineStrength == 35, "default scanline strength=35");
    check(fc.paletteCorrectionEnabled == 0, "default palette=0");
    check(fc.ditherCleanupEnabled == 0, "default dither=0");
}

static void t_sanitize(void) {
    CSB_V2_FilterConfig fc;
    csb_v2_filter_config_defaults(&fc);
    fc.crtScanlinesEnabled = -1;
    fc.crtScanlineStrength = 999;
    fc.paletteCorrectionEnabled = 7;
    fc.ditherCleanupEnabled = -5;
    csb_v2_filter_config_sanitize(&fc);
    check(fc.crtScanlinesEnabled == 0, "scanlines clamp 0");
    check(fc.crtScanlineStrength == 100, "scanline strength clamp 100");
    check(fc.paletteCorrectionEnabled == 1, "palette clamp 1");
    check(fc.ditherCleanupEnabled == 0, "dither clamp 0");
}

static void t_apply_get_roundtrip(void) {
    CSB_V2_FilterConfig fc;
    csb_v2_filter_config_defaults(&fc);
    fc.crtScanlinesEnabled = 1;
    fc.crtScanlineStrength = 75;
    fc.paletteCorrectionEnabled = 1;
    fc.ditherCleanupEnabled = 1;
    csb_v2_filter_config_apply(&fc);
    const CSB_V2_FilterConfig* live = csb_v2_filter_config_get();
    check(live->crtScanlinesEnabled == 1, "live scanlines=1");
    check(live->crtScanlineStrength == 75, "live scanline strength=75");
    check(live->paletteCorrectionEnabled == 1, "live palette=1");
    check(live->ditherCleanupEnabled == 1, "live dither=1");
}

static void t_apply_null_uses_defaults(void) {
    csb_v2_filter_config_apply(NULL);
    const CSB_V2_FilterConfig* live = csb_v2_filter_config_get();
    check(live->crtScanlinesEnabled == 0, "null -> defaults: scanlines=0");
    check(live->crtScanlineStrength == 35, "null -> defaults: strength=35");
    check(live->paletteCorrectionEnabled == 0, "null -> defaults: palette=0");
    check(live->ditherCleanupEnabled == 0, "null -> defaults: dither=0");
}

static void t_reset(void) {
    CSB_V2_FilterConfig fc;
    csb_v2_filter_config_defaults(&fc);
    fc.crtScanlinesEnabled = 1;
    csb_v2_filter_config_apply(&fc);
    csb_v2_filter_config_reset();
    const CSB_V2_FilterConfig* live = csb_v2_filter_config_get();
    check(live->crtScanlinesEnabled == 0, "reset -> scanlines=0");
}

static void t_wireup_from_settings(void) {
    /* The M11 launch wire-up goes M12_StartupMenuState -> CSB_V2_Settings
     * -> csb_v2_settings_apply_to_runtime -> csb_v2_filter_config_apply.
     * Verify the end-to-end path. */
    CSB_V2_Settings s;
    csb_v2_settings_defaults(&s);
    s.crtScanlinesEnabled = 1;
    s.crtScanlineStrength = 80;
    s.paletteCorrectionEnabled = 1;
    s.ditherCleanupEnabled = 0;
    csb_v2_settings_apply_to_runtime(&s);
    const CSB_V2_FilterConfig* live = csb_v2_filter_config_get();
    check(live->crtScanlinesEnabled == 1, "wireup scanlines=1");
    check(live->crtScanlineStrength == 80, "wireup strength=80");
    check(live->paletteCorrectionEnabled == 1, "wireup palette=1");
    check(live->ditherCleanupEnabled == 0, "wireup dither=0");
}

static void t_independent_from_upscale(void) {
    /* Filter config and upscale config are separate global state. */
    CSB_V2_Settings s;
    csb_v2_settings_defaults(&s);
    s.crtScanlinesEnabled = 1;
    csb_v2_settings_apply_to_runtime(&s);
    /* Upscale config is still the default (200% -> 2x). */
    check(csb_v2_upscale_get_scale() == 2, "upscale independent: scale=2");
    /* Filter config reflects the new value. */
    check(csb_v2_filter_config_get()->crtScanlinesEnabled == 1,
          "filter config reflects scanlines=1");
}

static void t_evidence(void) {
    const char* ev = csb_v2_filter_config_source_evidence();
    check(ev != NULL && strlen(ev) > 50, "ev non-trivial");
    check(strstr(ev, "CSB") != NULL, "ev CSB");
    check(strstr(ev, "filter") != NULL, "ev filter");
}

int main(void) {
    printf("=== CSB V2 filter config test ===\n");
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
