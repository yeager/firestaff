/**
 * firestaff_theron_v2_texture_upscale_probe.c
 *
 * Theron V2.1 texture upscale headless probe.
 */
#include "theron_v2_texture_upscale_pc34.h"
#include "theron_v2_presentation_mode_pc34.h"
#include <stdio.h>
#include <string.h>
static int g_total = 0, g_failed = 0;
static void check(int cond, const char* name) {
    g_total++;
    if (!cond) { g_failed++; fprintf(stderr, "[FAIL] %s\n", name); }
    else printf("[PASS] %s\n", name);
}

static void p_nearest(void) {
    uint8_t src[4] = { 10, 20, 30, 40 };
    uint8_t dst[16] = { 0 };
    theron_v2_upscale_nearest(src, 2, 2, dst, 4, 4);
    check(dst[0] == 10 && dst[15] == 40, "nearest corners");
}

static void p_bilinear(void) {
    uint8_t src[4] = { 0, 100, 0, 0 };
    uint8_t dst[16];
    theron_v2_upscale_bilinear(src, 2, 2, dst, 4, 4);
    check(dst[0] == 0 && dst[3] >= 90, "bilinear gradient");
}

static void p_epx(void) {
    /* Horizontal-edge input preserves the edge. */
    uint8_t src[4] = { 10, 10, 20, 20 };
    uint8_t dst[16] = { 0 };
    theron_v2_upscale_epx(src, 2, 2, dst, 4, 4);
    check(dst[0] == 10 && dst[3] == 10, "epx top = 10");
    check(dst[12] == 20 && dst[15] == 20, "epx bottom = 20");
}

static void p_palette(void) {
    uint8_t idx[4] = { 0, 1, 2, 5 };
    uint32_t pal[6] = { 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF,
                       0xFFFFFFFF, 0x00FFFF00 };
    uint32_t out[4];
    theron_v2_upscale_palette_to_rgba(idx, 2, 2, pal, 6, out);
    check(out[0] == 0xFF000000 && out[3] == 0x00FFFF00, "palette lookup");
}

static void p_scale_roundtrip(void) {
    theron_v2_upscale_set_scale(1);
    theron_v2_upscale_set_scale(2);
    theron_v2_upscale_set_scale(4);
    theron_v2_upscale_set_scale(99);
    check(1, "set_scale valid + invalid");
}

static void p_ntsc_fullscreen(void) {
    uint8_t src[256 * 224];
    uint8_t epx[512 * 448];
    uint32_t rgba[512 * 448];
    uint32_t pal[4] = { 0, 0xFF, 0xFF00, 0xFF0000 };
    memset(src, 1, 256 * 224);
    memset(epx, 0xCC, sizeof(epx));
    memset(rgba, 0xCC, sizeof(rgba));
    theron_v2_upscale_set_scale(2);
    theron_v2_upscale_ntsc_fullscreen(src, pal, 4, epx, rgba);
    check(epx[0] != 0xCC, "NTSC EPX wrote");
    theron_v2_upscale_set_scale(2);
}

static void p_dungeon_viewport(void) {
    uint8_t src[192 * 160];
    uint8_t epx[384 * 320];
    uint32_t rgba[384 * 320];
    uint32_t pal[4] = { 0, 0xFF, 0xFF00, 0xFF0000 };
    memset(src, 1, 192 * 160);
    memset(epx, 0xCC, sizeof(epx));
    memset(rgba, 0xCC, sizeof(rgba));
    theron_v2_upscale_set_scale(2);
    theron_v2_upscale_dungeon_viewport(src, 192, 160, pal, 4, epx, rgba);
    check(epx[0] != 0xCC, "dungeon viewport EPX wrote");
    theron_v2_upscale_set_scale(2);
}

static void p_present_mode_v22(void) {
    uint8_t src[4];
    uint8_t epx[16];
    uint32_t rgba[16];
    uint32_t pal[4] = { 0xAA, 0xBB, 0xCC, 0xDD };
    memset(src, 0, 4);
    memset(epx, 0xCC, 16);
    memset(rgba, 0xCC, 16 * sizeof(uint32_t));
    theron_v2_upscale_set_scale(2);
    theron_v2_presentation_mode_reset();
    theron_v2_presentation_mode_set_modern_pack_available(1);
    theron_v2_presentation_mode_set(THERON_V2_PM_V22_MODERN);
    theron_v2_upscale_dungeon_viewport(src, 2, 2, pal, 4, epx, rgba);
    check(theron_v2_presentation_mode_is_v22() == 1, "V22 active");
    check(epx[0] != 0xCC, "EPX ran under V22 (sentinel)");
    theron_v2_presentation_mode_reset();
}

static void p_evidence(void) {
    const char* ev = theron_v2_upscale_v21_source_evidence();
    check(ev != NULL && strlen(ev) > 50, "ev non-trivial");
    check(strstr(ev, "EPX") != NULL, "ev EPX");
    check(strstr(ev, "Theron") != NULL, "ev Theron");
    check(strstr(ev, "THQUEST") != NULL, "ev THQUEST");
}

int main(void) {
    printf("=== Theron V2.1 texture upscale probe ===\n");
    p_nearest(); p_bilinear(); p_epx(); p_palette();
    p_scale_roundtrip(); p_ntsc_fullscreen(); p_dungeon_viewport();
    p_present_mode_v22(); p_evidence();
    printf("--- %d / %d passed ---\n", g_total - g_failed, g_total);
    return g_failed == 0 ? 0 : 1;
}
