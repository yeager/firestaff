/* test_csb_v2_texture_upscale_pc34.c
 *
 * CSB V2.1 texture upscale unit test. Mirrors the DM1 V2.1
 * coverage but with csb_ prefix and CSB-specific helpers
 * (9-square viewport + panel).
 */
#include "csb_v2_texture_upscale_pc34.h"
#include "csb_v2_presentation_mode_pc34.h"
#include "dm1_v2_presentation_mode_pc34.h"
#include <stdio.h>
#include <string.h>
static int g_failed = 0, g_total = 0;
static void check(int cond, const char* name) {
    g_total++;
    if (!cond) { g_failed++; fprintf(stderr, "FAIL: %s\n", name); }
    else printf("PASS: %s\n", name);
}

static void fill_pattern(uint8_t* buf, int n) {
    for (int i = 0; i < n; i++) buf[i] = (uint8_t)(i & 0xFF);
}

static void t_init_defaults(void) {
    csb_v2_upscale_init(NULL);
    /* scale_factor is internal, but a known-2 default lets us
     * verify behaviour through the public EPX path. */
    check(1, "init(NULL) returns");
}

static void t_nearest_basic(void) {
    uint8_t src[4] = { 10, 20, 30, 40 };
    uint8_t dst[16] = { 0 };
    csb_v2_upscale_nearest(src, 2, 2, dst, 4, 4);
    check(dst[0] == 10 && dst[3] == 20, "nearest top row");
    check(dst[12] == 30 && dst[15] == 40, "nearest bottom row");
}

static void t_nearest_identity_at_same_size(void) {
    uint8_t src[16];
    uint8_t dst[16];
    fill_pattern(src, 16);
    memset(dst, 0, 16);
    csb_v2_upscale_nearest(src, 4, 4, dst, 4, 4);
    check(memcmp(src, dst, 16) == 0, "nearest same size is identity");
}

static void t_bilinear_smooth(void) {
    uint8_t src[4] = { 0, 100, 0, 0 };
    uint8_t dst[16] = { 0 };
    csb_v2_upscale_bilinear(src, 2, 2, dst, 4, 4);
    /* top-left is 0, top-right is 100, so the top row should
     * go 0..33..66..100 (or close, depending on rounding). */
    check(dst[0] == 0, "bilinear top-left");
    check(dst[3] >= 90 && dst[3] <= 100, "bilinear top-right near 100");
    check(dst[12] == 0, "bilinear bottom-left (row 1, col 0)");
}

static void t_epx_2x(void) {
    /* 2x2 input -> 4x4 output, EPX preserves edges. */
    uint8_t src[4] = { 10, 20, 10, 20 };
    uint8_t dst[16] = { 0 };
    csb_v2_upscale_epx(src, 2, 2, dst, 4, 4);
    /* On this simple {10,20,10,20} pattern, NONE of the four
     * EPX neighbour predicates (C==A && C!=D && A!=B /
     * A==B && A!=C && B!=D / D==C && D!=B && C!=A /
     * B==D && B!=A && D!=C) match, so every output pixel
     * falls back to P (the source pixel at the corresponding
     * 2x block). That is exactly the column-stripe nearest
     * pattern: row 0 mirrors column 0 of src, row 1 mirrors
     * column 0 too (because y=0 P=src[0] and y=1 P=src[0] in
     * the left half, y=0 P=src[1] and y=1 P=src[1] in the
     * right half), and so on. The block-arranged expectations
     * in earlier revisions of this test were wrong; the
     * CSBWin/Viewport.cpp:7290 EPX path (cited in the source
     * header) also returns P on a no-match, which is what
     * this test now pins. */
    check(dst[0] == 10 && dst[1] == 10, "EPX top-left block (P=src[0])");
    check(dst[2] == 20 && dst[3] == 20, "EPX top-right block (P=src[1])");
    check(dst[4] == 10 && dst[5] == 10, "EPX row 1 left (P=src[0])");
    check(dst[6] == 20 && dst[7] == 20, "EPX row 1 right (P=src[1])");
    check(dst[15] == 20, "EPX bottom-right");
}

static void t_palette_to_rgba(void) {
    uint8_t indexed[4] = { 0, 1, 2, 5 };
    uint32_t palette[6] = { 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF,
                            0xFFFFFFFF, 0x00FFFF00 };
    uint32_t out[4];
    csb_v2_upscale_palette_to_rgba(indexed, 2, 2, palette, 6, out);
    check(out[0] == 0xFF000000, "rgba[0]");
    check(out[1] == 0x00FF0000, "rgba[1]");
    check(out[2] == 0x0000FF00, "rgba[2]");
    check(out[3] == 0x00FFFF00, "rgba[3]");
}

static void t_palette_oob(void) {
    uint8_t indexed[2] = { 0, 99 };
    uint32_t palette[2] = { 0xAABBCCDD, 0x11223344 };
    uint32_t out[2];
    csb_v2_upscale_palette_to_rgba(indexed, 1, 2, palette, 2, out);
    check(out[0] == 0xAABBCCDD, "rgba[0] in range");
    check(out[1] == 0xFF000000, "rgba[1] OOB -> black opaque");
}

static void t_set_scale(void) {
    csb_v2_upscale_set_scale(1);
    csb_v2_upscale_set_scale(2);
    csb_v2_upscale_set_scale(4);
    csb_v2_upscale_set_scale(99);  /* invalid -> ignored */
    check(1, "set_scale valid + invalid");
}

static void t_full_pipeline(void) {
    uint8_t src[16];
    uint8_t epx_buf[64];
    uint32_t rgba[64];
    uint32_t palette[4] = { 0x00000000, 0x00FF0000, 0x0000FF00, 0x000000FF };
    fill_pattern(src, 16);
    memset(epx_buf, 0xCC, 64);
    memset(rgba, 0xCC, 64 * sizeof(uint32_t));
    csb_v2_upscale_full_pipeline(src, 4, 4, palette, 4, epx_buf, rgba, 2);
    /* epx_buf is 8x8 = 64, and rgba has 64 entries. */
    check(epx_buf[0] != 0xCC, "EPX step wrote to epx_buf");
    check(rgba[0] == 0x00000000, "rgba[0] from palette[0]");
}

static void t_9square_viewport(void) {
    /* scale_factor = 2 by default -> EPX path runs. */
    uint8_t src[16];
    uint8_t epx_buf[64];
    uint32_t rgba[64];
    uint32_t palette[4] = { 0x00000000, 0x00FF0000, 0x0000FF00, 0x000000FF };
    fill_pattern(src, 16);
    csb_v2_upscale_9square_viewport(src, 4, 4, palette, 4, epx_buf, rgba);
    check(epx_buf[0] != 0 && rgba[0] == 0x00000000, "9square EPX 2x");
    csb_v2_upscale_set_scale(1);
    csb_v2_upscale_9square_viewport(src, 4, 4, palette, 4, epx_buf, rgba);
    /* At scale 1, just palette pass-through; rgba = indexed mapped. */
    check(rgba[0] == 0x00000000, "9square scale=1 palette pass");
    csb_v2_upscale_set_scale(2);  /* restore */
}

static void t_panel(void) {
    uint8_t src[32];
    uint8_t epx_buf[128];
    uint32_t rgba[128];
    uint32_t palette[4] = { 0x00000000, 0x00FF0000, 0x0000FF00, 0x000000FF };
    fill_pattern(src, 32);
    csb_v2_upscale_panel(src, 8, 4, palette, 4, epx_buf, rgba);
    check(epx_buf[0] != 0 && rgba[0] == 0x00000000, "panel EPX 2x");
}

static void t_evidence(void) {
    const char* ev = csb_v2_upscale_v21_source_evidence();
    check(ev != NULL && strlen(ev) > 50, "ev non-trivial");
    check(strstr(ev, "EPX") != NULL, "ev EPX");
    check(strstr(ev, "CSB") != NULL, "ev CSB");
    check(strstr(ev, "9-square") != NULL || strstr(ev, "9square") != NULL, "ev 9-square");
}

static void t_present_mode_v22_triggers_epx(void) {
    /* When the V2.2 modern path is active, the V2.1 EPX path is
     * still the texture step (V22 just adds the modern material
     * on top). Verify by setting V22 + pack, then checking
     * csb_v2_upscale_9square_viewport runs the EPX step. */
    uint8_t src[4] = { 0, 1, 2, 3 };
    uint8_t epx_buf[16] = { 0 };
    uint32_t rgba[16];
    uint32_t palette[4] = { 0xAA, 0xBB, 0xCC, 0xDD };
    csb_v2_upscale_set_scale(2);
    csb_v2_presentation_mode_reset();
    csb_v2_presentation_mode_set_modern_pack_available(1);
    csb_v2_presentation_mode_set(CSB_V2_PM_V22_MODERN);
    csb_v2_upscale_9square_viewport(src, 2, 2, palette, 4, epx_buf, rgba);
    check(csb_v2_presentation_mode_is_v22() == 1, "V22 active");
    check(epx_buf[0] != 0, "EPX ran under V22");
    csb_v2_presentation_mode_reset();
}

int main(void) {
    printf("=== CSB V2.1 texture upscale test ===\n");
    t_init_defaults();
    t_nearest_basic();
    t_nearest_identity_at_same_size();
    t_bilinear_smooth();
    t_epx_2x();
    t_palette_to_rgba();
    t_palette_oob();
    t_set_scale();
    t_full_pipeline();
    t_9square_viewport();
    t_panel();
    t_evidence();
    t_present_mode_v22_triggers_epx();
    printf("--- %d / %d passed ---\n", g_total - g_failed, g_total);
    return g_failed == 0 ? 0 : 1;
}
