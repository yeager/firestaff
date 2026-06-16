/* test_csb_v2_filter_chain_pc34.c
 *
 * CSB V2.0 per-frame filter chain dispatch test. Verifies that
 * the csb_v2_filter_chain_apply_indexed and _rgba functions:
 *   - Skip disabled filters (count=0).
 *   - Apply enabled filters (count >= 1).
 *   - Are null-safe and zero-sized-safe.
 *   - Toggle between indexed and RGBA paths independently.
 */
#include "csb_v2_filter_config_pc34.h"
#include "csb_v2_settings_pc34.h"
#include <stdio.h>
#include <string.h>

static int g_failed = 0, g_total = 0;
static void check(int cond, const char* name) {
    g_total++;
    if (!cond) { g_failed++; fprintf(stderr, "FAIL: %s\n", name); }
    else printf("PASS: %s\n", name);
}

static void t_indexed_all_off(void) {
    uint8_t fb[16] = {0};
    csb_v2_filter_config_reset();
    int n = csb_v2_filter_chain_apply_indexed(fb, 4, 4);
    check(n == 0, "indexed: all off -> 0 filters applied");
}

static void t_indexed_dither_only(void) {
    uint8_t fb[16] = {0};
    csb_v2_filter_config_reset();
    CSB_V2_FilterConfig fc;
    csb_v2_filter_config_defaults(&fc);
    fc.ditherCleanupEnabled = 1;
    fc.paletteCorrectionEnabled = 0;
    csb_v2_filter_config_apply(&fc);
    int n = csb_v2_filter_chain_apply_indexed(fb, 4, 4);
    check(n == 1, "indexed: dither only -> 1 filter applied");
}

static void t_indexed_palette_only(void) {
    uint8_t fb[16] = {0};
    csb_v2_filter_config_reset();
    CSB_V2_FilterConfig fc;
    csb_v2_filter_config_defaults(&fc);
    fc.ditherCleanupEnabled = 0;
    fc.paletteCorrectionEnabled = 1;
    fc.crtScanlineStrength = 75;
    csb_v2_filter_config_apply(&fc);
    int n = csb_v2_filter_chain_apply_indexed(fb, 4, 4);
    check(n == 1, "indexed: palette only -> 1 filter applied");
}

static void t_indexed_both_on(void) {
    uint8_t fb[16] = {0};
    csb_v2_filter_config_reset();
    CSB_V2_FilterConfig fc;
    csb_v2_filter_config_defaults(&fc);
    fc.ditherCleanupEnabled = 1;
    fc.paletteCorrectionEnabled = 1;
    csb_v2_filter_config_apply(&fc);
    int n = csb_v2_filter_chain_apply_indexed(fb, 4, 4);
    check(n == 2, "indexed: both on -> 2 filters applied");
}

static void t_rgba_all_off(void) {
    /* 4x4 RGBA. */
    uint8_t rgba[4 * 4 * 4] = {0};
    csb_v2_filter_config_reset();
    int n = csb_v2_filter_chain_apply_rgba(rgba, 4, 4);
    check(n == 0, "rgba: all off -> 0 filters applied");
}

static void t_rgba_scanlines_on(void) {
    uint8_t rgba[4 * 4 * 4] = {0};
    csb_v2_filter_config_reset();
    CSB_V2_FilterConfig fc;
    csb_v2_filter_config_defaults(&fc);
    fc.crtScanlinesEnabled = 1;
    fc.crtScanlineStrength = 60;
    csb_v2_filter_config_apply(&fc);
    int n = csb_v2_filter_chain_apply_rgba(rgba, 4, 4);
    check(n == 1, "rgba: scanlines on -> 1 filter applied");
}

static void t_null_safe(void) {
    csb_v2_filter_config_reset();
    int n1 = csb_v2_filter_chain_apply_indexed(NULL, 4, 4);
    int n2 = csb_v2_filter_chain_apply_rgba(NULL, 4, 4);
    check(n1 == 0, "NULL fb -> 0");
    check(n2 == 0, "NULL rgba -> 0");
}

static void t_zero_sized(void) {
    csb_v2_filter_config_reset();
    uint8_t fb[1] = {0};
    int n1 = csb_v2_filter_chain_apply_indexed(fb, 0, 0);
    int n2 = csb_v2_filter_chain_apply_indexed(fb, 1, 0);
    int n3 = csb_v2_filter_chain_apply_indexed(fb, 0, 1);
    /* 1x1: indexed filters need w>2, h>2 so both should be skipped. */
    check(n1 == 0, "0x0 -> 0");
    check(n2 == 0, "1x0 -> 0");
    check(n3 == 0, "0x1 -> 0");
}

static void t_too_small_for_indexed(void) {
    /* The indexed filters require w>2, h>2. 2x2 should be skipped
     * (count=0). 3x3 is the minimum that the underlying filter
     * functions accept (w>2 means w>=3). 4x4 works fully. */
    uint8_t fb[16] = {0};
    CSB_V2_FilterConfig fc;
    csb_v2_filter_config_defaults(&fc);
    fc.ditherCleanupEnabled = 1;
    fc.paletteCorrectionEnabled = 1;
    csb_v2_filter_config_apply(&fc);
    int n1 = csb_v2_filter_chain_apply_indexed(fb, 2, 2);
    int n2 = csb_v2_filter_chain_apply_indexed(fb, 3, 3);
    int n3 = csb_v2_filter_chain_apply_indexed(fb, 4, 4);
    check(n1 == 0, "2x2 indexed -> 0 (too small, w<=2)");
    check(n2 == 2, "3x3 indexed -> 2 (both filters apply, w>2 h>2)");
    check(n3 == 2, "4x4 indexed -> 2 (both filters apply)");
}

static void t_wireup_from_settings(void) {
    /* End-to-end: M12 menu settings -> bridge -> config -> chain. */
    CSB_V2_Settings s;
    csb_v2_settings_defaults(&s);
    s.ditherCleanupEnabled = 1;
    s.paletteCorrectionEnabled = 1;
    s.crtScanlinesEnabled = 1;
    s.crtScanlineStrength = 70;
    csb_v2_settings_apply_to_runtime(&s);
    uint8_t fb[16] = {0};
    uint8_t rgba[4 * 4 * 4] = {0};
    int n1 = csb_v2_filter_chain_apply_indexed(fb, 4, 4);
    int n2 = csb_v2_filter_chain_apply_rgba(rgba, 4, 4);
    check(n1 == 2, "wireup: indexed = 2");
    check(n2 == 1, "wireup: rgba = 1");
}

int main(void) {
    printf("=== CSB V2.0 per-frame filter chain test ===\n");
    t_indexed_all_off();
    t_indexed_dither_only();
    t_indexed_palette_only();
    t_indexed_both_on();
    t_rgba_all_off();
    t_rgba_scanlines_on();
    t_null_safe();
    t_zero_sized();
    t_too_small_for_indexed();
    t_wireup_from_settings();
    printf("--- %d / %d passed ---\n", g_total - g_failed, g_total);
    return g_failed == 0 ? 0 : 1;
}
