/**
 * firestaff_csb_v2_filter_chain_probe.c
 *
 * CSB V2.0 per-frame filter chain dispatch probe. Verifies that
 * the csb_v2_filter_chain_apply_indexed and _rgba functions honor
 * the live config and the toggle flags. End-to-end from M12
 * menu settings through the bridge into the per-frame chain.
 */
#include "csb_v2_filter_config_pc34.h"
#include "csb_v2_settings_pc34.h"
#include <stdio.h>
#include <string.h>

static int g_total = 0, g_failed = 0;
static void check(int cond, const char* name) {
    g_total++;
    if (!cond) { g_failed++; fprintf(stderr, "[FAIL] %s\n", name); }
    else printf("[PASS] %s\n", name);
}

static void p_indexed_all_off(void) {
    uint8_t fb[16] = {0};
    csb_v2_filter_config_reset();
    int n = csb_v2_filter_chain_apply_indexed(fb, 4, 4);
    check(n == 0, "indexed: all off -> 0");
}

static void p_indexed_dither_only(void) {
    uint8_t fb[16] = {0};
    CSB_V2_FilterConfig fc;
    csb_v2_filter_config_defaults(&fc);
    fc.ditherCleanupEnabled = 1;
    fc.paletteCorrectionEnabled = 0;
    csb_v2_filter_config_apply(&fc);
    int n = csb_v2_filter_chain_apply_indexed(fb, 4, 4);
    check(n == 1, "indexed: dither only -> 1");
}

static void p_indexed_palette_only(void) {
    uint8_t fb[16] = {0};
    CSB_V2_FilterConfig fc;
    csb_v2_filter_config_defaults(&fc);
    fc.ditherCleanupEnabled = 0;
    fc.paletteCorrectionEnabled = 1;
    csb_v2_filter_config_apply(&fc);
    int n = csb_v2_filter_chain_apply_indexed(fb, 4, 4);
    check(n == 1, "indexed: palette only -> 1");
}

static void p_indexed_both_on(void) {
    uint8_t fb[16] = {0};
    CSB_V2_FilterConfig fc;
    csb_v2_filter_config_defaults(&fc);
    fc.ditherCleanupEnabled = 1;
    fc.paletteCorrectionEnabled = 1;
    csb_v2_filter_config_apply(&fc);
    int n = csb_v2_filter_chain_apply_indexed(fb, 4, 4);
    check(n == 2, "indexed: both on -> 2");
}

static void p_rgba_scanlines(void) {
    uint8_t rgba[4 * 4 * 4] = {0};
    CSB_V2_FilterConfig fc;
    csb_v2_filter_config_defaults(&fc);
    fc.crtScanlinesEnabled = 1;
    fc.crtScanlineStrength = 50;
    csb_v2_filter_config_apply(&fc);
    int n = csb_v2_filter_chain_apply_rgba(rgba, 4, 4);
    check(n == 1, "rgba: scanlines on -> 1");
}

static void p_rgba_all_off(void) {
    uint8_t rgba[4 * 4 * 4] = {0};
    csb_v2_filter_config_reset();
    int n = csb_v2_filter_chain_apply_rgba(rgba, 4, 4);
    check(n == 0, "rgba: all off -> 0");
}

static void p_null_safe(void) {
    int n1 = csb_v2_filter_chain_apply_indexed(NULL, 4, 4);
    int n2 = csb_v2_filter_chain_apply_rgba(NULL, 4, 4);
    check(n1 == 0, "NULL fb -> 0");
    check(n2 == 0, "NULL rgba -> 0");
}

static void p_wireup_from_settings(void) {
    /* M12 menu settings -> bridge -> config -> chain. */
    CSB_V2_Settings s;
    csb_v2_settings_defaults(&s);
    s.ditherCleanupEnabled = 1;
    s.paletteCorrectionEnabled = 1;
    s.crtScanlinesEnabled = 1;
    s.crtScanlineStrength = 80;
    csb_v2_settings_apply_to_runtime(&s);
    uint8_t fb[16] = {0};
    uint8_t rgba[4 * 4 * 4] = {0};
    int n1 = csb_v2_filter_chain_apply_indexed(fb, 4, 4);
    int n2 = csb_v2_filter_chain_apply_rgba(rgba, 4, 4);
    check(n1 == 2, "wireup: indexed = 2");
    check(n2 == 1, "wireup: rgba = 1");
}

int main(void) {
    printf("=== CSB V2.0 per-frame filter chain probe ===\n");
    p_indexed_all_off();
    p_indexed_dither_only();
    p_indexed_palette_only();
    p_indexed_both_on();
    p_rgba_scanlines();
    p_rgba_all_off();
    p_null_safe();
    p_wireup_from_settings();
    printf("--- %d / %d passed ---\n", g_total - g_failed, g_total);
    return g_failed == 0 ? 0 : 1;
}
