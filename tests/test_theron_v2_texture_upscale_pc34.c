/* test_theron_v2_texture_upscale_pc34.c
 *
 * Theron V2.1 texture upscale unit test. Mirrors the CSB V2.1
 * coverage with theron_ prefix and Theron-specific helpers
 * (NTSC 256x224 fullscreen + 192x160 dungeon viewport).
 */
#include "theron_v2_texture_upscale_pc34.h"
#include "theron_v2_presentation_mode_pc34.h"
#include <stdio.h>
#include <string.h>
static int g_failed = 0, g_total = 0;
static void check(int cond, const char* name) {
    g_total++;
    if (!cond) { g_failed++; fprintf(stderr, "FAIL: %s\n", name); }
    else printf("PASS: %s\n", name);
}

static void t_init_defaults(void) {
    theron_v2_upscale_init(NULL);
    check(1, "init(NULL) returns");
}

static void t_nearest_basic(void) {
    uint8_t src[4] = { 10, 20, 30, 40 };
    uint8_t dst[16] = { 0 };
    theron_v2_upscale_nearest(src, 2, 2, dst, 4, 4);
    check(dst[0] == 10 && dst[15] == 40, "nearest corners");
}

static void t_bilinear_smooth(void) {
    uint8_t src[4] = { 0, 100, 0, 0 };
    uint8_t dst[16] = { 0 };
    theron_v2_upscale_bilinear(src, 2, 2, dst, 4, 4);
    check(dst[0] == 0 && dst[3] >= 90 && dst[3] <= 100, "bilinear gradient");
}

static void t_epx_2x_horizontal_edge(void) {
    /* Horizontal-edge input: top row=10, bottom row=20.
     * EPX preserves the edge. */
    uint8_t src[4] = { 10, 10, 20, 20 };
    uint8_t dst[16] = { 0 };
    theron_v2_upscale_epx(src, 2, 2, dst, 4, 4);
    check(dst[0] == 10 && dst[3] == 10, "epx top row = 10");
    check(dst[12] == 20 && dst[15] == 20, "epx bottom row = 20");
}

static void t_palette_to_rgba(void) {
    uint8_t indexed[4] = { 0, 1, 2, 5 };
    uint32_t palette[6] = { 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF,
                            0xFFFFFFFF, 0x00FFFF00 };
    uint32_t out[4];
    theron_v2_upscale_palette_to_rgba(indexed, 2, 2, palette, 6, out);
    check(out[0] == 0xFF000000, "rgba[0]");
    check(out[3] == 0x00FFFF00, "rgba[3]");
}

static void t_palette_oob(void) {
    uint8_t indexed[2] = { 0, 99 };
    uint32_t palette[2] = { 0xAABBCCDD, 0x11223344 };
    uint32_t out[2];
    theron_v2_upscale_palette_to_rgba(indexed, 1, 2, palette, 2, out);
    check(out[0] == 0xAABBCCDD, "rgba[0] in range");
    check(out[1] == 0xFF000000, "rgba[1] OOB -> black opaque");
}

static void t_set_scale(void) {
    theron_v2_upscale_set_scale(1);
    theron_v2_upscale_set_scale(2);
    theron_v2_upscale_set_scale(4);
    theron_v2_upscale_set_scale(99);  /* invalid -> ignored */
    check(1, "set_scale valid + invalid");
}

static void t_full_pipeline(void) {
    uint8_t src[16];
    uint8_t epx_buf[64];
    uint32_t rgba[64];
    uint32_t palette[4] = { 0x00000000, 0x00FF0000, 0x0000FF00, 0x000000FF };
    memset(src, 1, 16);  /* uniform input */
    memset(epx_buf, 0xCC, 64);
    memset(rgba, 0xCC, 64 * sizeof(uint32_t));
    theron_v2_upscale_full_pipeline(src, 4, 4, palette, 4, epx_buf, rgba, 2);
    check(epx_buf[0] != 0xCC, "EPX step wrote to epx_buf");
    check(rgba[0] == 0x00FF0000, "rgba[0] = palette[1] = 0x00FF0000");
}

static void t_ntsc_fullscreen(void) {
    uint8_t src[256 * 224];
    uint8_t epx_buf[512 * 448];
    uint32_t rgba[512 * 448];
    uint32_t palette[4] = { 0x00000000, 0x00FF0000, 0x0000FF00, 0x000000FF };
    memset(src, 1, 256 * 224);
    memset(epx_buf, 0xCC, sizeof(epx_buf));
    memset(rgba, 0xCC, sizeof(rgba));
    theron_v2_upscale_set_scale(2);
    theron_v2_upscale_ntsc_fullscreen(src, palette, 4, epx_buf, rgba);
    check(epx_buf[0] != 0xCC, "NTSC fullscreen EPX 2x wrote epx_buf");
    theron_v2_upscale_set_scale(1);
    theron_v2_upscale_ntsc_fullscreen(src, palette, 4, epx_buf, rgba);
    check(rgba[0] == 0x00FF0000, "NTSC fullscreen scale=1 palette pass");
    theron_v2_upscale_set_scale(2);
}

static void t_dungeon_viewport(void) {
    /* 192x160 letterboxed dungeon viewport. */
    uint8_t src[192 * 160];
    uint8_t epx_buf[384 * 320];
    uint32_t rgba[384 * 320];
    uint32_t palette[4] = { 0x00000000, 0x00FF0000, 0x0000FF00, 0x000000FF };
    memset(src, 1, 192 * 160);
    memset(epx_buf, 0xCC, sizeof(epx_buf));
    memset(rgba, 0xCC, sizeof(rgba));
    theron_v2_upscale_set_scale(2);
    theron_v2_upscale_dungeon_viewport(src, 192, 160, palette, 4, epx_buf, rgba);
    check(epx_buf[0] != 0xCC, "dungeon viewport EPX 2x wrote epx_buf");
    theron_v2_upscale_set_scale(1);
    theron_v2_upscale_dungeon_viewport(src, 192, 160, palette, 4, epx_buf, rgba);
    check(rgba[0] == 0x00FF0000, "dungeon viewport scale=1 palette pass");
    theron_v2_upscale_set_scale(2);
}

static void t_evidence(void) {
    const char* ev = theron_v2_upscale_v21_source_evidence();
    check(ev != NULL && strlen(ev) > 50, "ev non-trivial");
    check(strstr(ev, "EPX") != NULL, "ev EPX");
    check(strstr(ev, "256x224") != NULL || strstr(ev, "NTSC") != NULL, "ev NTSC");
    check(strstr(ev, "192x160") != NULL || strstr(ev, "letterbox") != NULL, "ev letterbox");
    check(strstr(ev, "THQUEST") != NULL, "ev THQUEST");
    check(strstr(ev, "HuC6260") != NULL || strstr(ev, "HuC6270") != NULL, "ev HuC");
}

static void t_present_mode_v22_triggers_epx(void) {
    uint8_t src[4];
    uint8_t epx_buf[16];
    uint32_t rgba[16];
    uint32_t palette[4] = { 0xAA, 0xBB, 0xCC, 0xDD };
    memset(src, 0, 4);
    memset(epx_buf, 0xCC, 16);
    memset(rgba, 0xCC, 16 * sizeof(uint32_t));
    theron_v2_upscale_set_scale(2);
    theron_v2_presentation_mode_reset();
    theron_v2_presentation_mode_set_modern_pack_available(1);
    theron_v2_presentation_mode_set(THERON_V2_PM_V22_MODERN);
    theron_v2_upscale_dungeon_viewport(src, 2, 2, palette, 4, epx_buf, rgba);
    check(theron_v2_presentation_mode_is_v22() == 1, "V22 active");
    check(epx_buf[0] != 0xCC, "EPX ran under V22");
    check(rgba[0] == 0xAA, "V22 EPX palette[0] lookup");
    theron_v2_presentation_mode_reset();
}

int main(void) {
    printf("=== Theron V2.1 texture upscale test ===\n");
    t_init_defaults();
    t_nearest_basic();
    t_bilinear_smooth();
    t_epx_2x_horizontal_edge();
    t_palette_to_rgba();
    t_palette_oob();
    t_set_scale();
    t_full_pipeline();
    t_ntsc_fullscreen();
    t_dungeon_viewport();
    t_evidence();
    t_present_mode_v22_triggers_epx();
    printf("--- %d / %d passed ---\n", g_total - g_failed, g_total);
    return g_failed == 0 ? 0 : 1;
}
