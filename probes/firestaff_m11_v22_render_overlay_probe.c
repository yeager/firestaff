/**
 * firestaff_m11_v22_render_overlay_probe.c
 *
 * DM1 V2.2 GPU render path: V22 modern-art overlay headless probe.
 * Verifies the overlay end-to-end on a 320x200 V1 framebuffer.
 */
#include "m11_v22_render_overlay_pc34.h"
#include "m11_v22_shape_cache_pc34.h"
#include "dm1_v2_presentation_mode_pc34.h"

#include <stdio.h>
#include <string.h>

static int g_total = 0, g_failed = 0;
static void check(int cond, const char* name) {
    g_total++;
    if (!cond) { g_failed++; fprintf(stderr, "[FAIL] %s\n", name); }
    else printf("[PASS] %s\n", name);
}

static void p_v1_inactive(void) {
    unsigned char fb[320 * 200];
    memset(fb, 0x00, sizeof(fb));
    dm1_v2_presentation_mode_reset();
    m11_v22_shape_cache_update(0, (const unsigned char[3][3]){0});
    int n = m11_v22_render_overlay(fb, 320, 200);
    check(n == 0, "V1 inactive -> 0 cells painted");
    int all_zero = 1;
    for (int i = 0; i < 320 * 200; ++i) {
        if (fb[i] != 0x00) { all_zero = 0; break; }
    }
    check(all_zero, "V1 inactive -> pixels unchanged");
}

static void p_v22_9_cells(void) {
    unsigned char fb[320 * 200];
    memset(fb, 0x00, sizeof(fb));
    dm1_v2_presentation_mode_reset();
    dm1_v2_presentation_mode_set_modern_pack_available(1);
    dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
    m11_v22_shape_cache_update(0, (const unsigned char[3][3]){0});
    int n = m11_v22_render_overlay(fb, 320, 200);
    check(n == 9, "V22: 9 cells painted");
}

static void p_v22_transition(void) {
    unsigned char fb[320 * 200];
    /* V1 -> 0 cells. */
    dm1_v2_presentation_mode_reset();
    m11_v22_shape_cache_update(0, (const unsigned char[3][3]){0});
    int n1 = m11_v22_render_overlay(fb, 320, 200);
    /* V22 -> 9 cells. */
    dm1_v2_presentation_mode_set_modern_pack_available(1);
    dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
    m11_v22_shape_cache_update(0, (const unsigned char[3][3]){0});
    int n2 = m11_v22_render_overlay(fb, 320, 200);
    /* V1 -> 0 cells (back). */
    dm1_v2_presentation_mode_reset();
    m11_v22_shape_cache_update(0, (const unsigned char[3][3]){0});
    int n3 = m11_v22_render_overlay(fb, 320, 200);
    check(n1 == 0, "V1 -> 0 cells");
    check(n2 == 9, "V22 -> 9 cells");
    check(n3 == 0, "V1 (back) -> 0 cells");
}

static void p_null_safe(void) {
    int n = m11_v22_render_overlay(NULL, 320, 200);
    check(n == 0, "NULL framebuffer -> 0 cells");
    unsigned char fb[1] = { 0 };
    n = m11_v22_render_overlay(fb, 0, 0);
    check(n == 0, "zero-sized framebuffer -> 0 cells");
}

static void p_border(void) {
    /* The 1-pixel border uses the placeholder index. */
    unsigned char fb[320 * 200];
    memset(fb, 0x00, sizeof(fb));
    dm1_v2_presentation_mode_reset();
    dm1_v2_presentation_mode_set_modern_pack_available(1);
    dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
    m11_v22_shape_cache_update(0, (const unsigned char[3][3]){0});
    m11_v22_render_overlay(fb, 320, 200);
    /* D1L top-left border = (8, 103). The overlay's 1px border
     * should be M11_V22_OVERLAY_PLACEHOLDER_INDEX. */
    int idx = 103 * 320 + 8;
    check(fb[idx] == M11_V22_OVERLAY_PLACEHOLDER_INDEX,
          "D1L top-left border = placeholder index");
    /* The center of the D1L rect (8+35, 103+15) should also be
     * filled (with the color derived from color_tint). */
    int cidx = (103 + 15) * 320 + (8 + 35);
    check(fb[cidx] != 0x00, "D1L center filled (non-zero)");
}

static void p_source_palette_shadow(void) {
    unsigned char bright[320 * 200];
    unsigned char dark[320 * 200];
    int center = (103 + 15) * 320 + (8 + 35);
    memset(bright, 0x00, sizeof(bright));
    memset(dark, 0x00, sizeof(dark));
    dm1_v2_presentation_mode_reset();
    dm1_v2_presentation_mode_set_modern_pack_available(1);
    dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
    m11_v22_shape_cache_update(0, (const unsigned char[3][3]){0});
    check(m11_v22_render_overlay_with_palette(bright, 320, 200, 0) == 9,
          "palette shadow: bright source paints 9 cells");
    check(m11_v22_render_overlay_with_palette(dark, 320, 200, 5) == 9,
          "palette shadow: dark source paints 9 cells");
    check(dark[center] < bright[center],
          "palette shadow: source palette 5 darkens D1L center");
}

static void p_evidence(void) {
    const char* ev = m11_v22_render_overlay_source_evidence();
    check(ev != NULL && strlen(ev) > 50, "ev non-trivial");
    check(strstr(ev, "V22") != NULL, "ev V22");
    check(strstr(ev, "DUNVIEW") != NULL, "ev DUNVIEW");
}

int main(void) {
    printf("=== M11 V22 render overlay probe ===\n");
    p_v1_inactive();
    p_v22_9_cells();
    p_v22_transition();
    p_null_safe();
    p_border();
    p_source_palette_shadow();
    p_evidence();
    printf("--- %d / %d passed ---\n", g_total - g_failed, g_total);
    return g_failed == 0 ? 0 : 1;
}
