/**
 * firestaff_csb_v2_texture_upscale_probe.c
 *
 * CSB V2.1 texture upscale headless probe.
 */
#include "csb_v2_texture_upscale_pc34.h"
#include "csb_v2_presentation_mode_pc34.h"
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
    csb_v2_upscale_nearest(src, 2, 2, dst, 4, 4);
    check(dst[0] == 10 && dst[15] == 40, "nearest corners");
}

static void p_bilinear(void) {
    uint8_t src[4] = { 0, 100, 0, 0 };
    uint8_t dst[16];
    csb_v2_upscale_bilinear(src, 2, 2, dst, 4, 4);
    check(dst[0] == 0 && dst[3] >= 90, "bilinear gradient");
}

static void p_epx(void) {
    uint8_t src[4] = { 10, 20, 10, 20 };
    uint8_t dst[16] = { 0 };
    csb_v2_upscale_epx(src, 2, 2, dst, 4, 4);
    check(dst[0] == 10 && dst[1] == 20, "epx 2x block");
}

static void p_palette(void) {
    uint8_t idx[4] = { 0, 1, 2, 5 };
    uint32_t pal[6] = { 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF,
                       0xFFFFFFFF, 0x00FFFF00 };
    uint32_t out[4];
    csb_v2_upscale_palette_to_rgba(idx, 2, 2, pal, 6, out);
    check(out[0] == 0xFF000000 && out[3] == 0x00FFFF00, "palette lookup");
}

static void p_scale_roundtrip(void) {
    csb_v2_upscale_set_scale(1);
    csb_v2_upscale_set_scale(2);
    csb_v2_upscale_set_scale(4);
    csb_v2_upscale_set_scale(99);  /* invalid, ignored */
    check(1, "set_scale valid + invalid accepted");
}

static void p_full_pipeline(void) {
    uint8_t src[16];
    uint8_t epx[64];
    uint32_t rgba[64];
    uint32_t pal[4] = { 0, 0xFF, 0xFF00, 0xFF0000 };
    for (int i = 0; i < 16; i++) src[i] = (uint8_t)i;
    csb_v2_upscale_full_pipeline(src, 4, 4, pal, 4, epx, rgba, 2);
    check(epx[0] != 0 && rgba[0] == 0, "full pipeline ran");
}

static void p_present_mode_v22_epx_runs(void) {
    uint8_t src[4] = { 0, 1, 2, 3 };
    uint8_t epx[16] = { 0 };
    uint32_t rgba[16];
    uint32_t pal[4] = { 0, 1, 2, 3 };
    csb_v2_upscale_set_scale(2);
    csb_v2_presentation_mode_reset();
    csb_v2_presentation_mode_set_modern_pack_available(1);
    csb_v2_presentation_mode_set(CSB_V2_PM_V22_MODERN);
    csb_v2_upscale_9square_viewport(src, 2, 2, pal, 4, epx, rgba);
    check(csb_v2_presentation_mode_is_v22() == 1, "V22 active");
    check(epx[0] != 0, "EPX ran under V22");
    csb_v2_presentation_mode_reset();
}

static void p_evidence(void) {
    const char* ev = csb_v2_upscale_v21_source_evidence();
    check(ev != NULL && strlen(ev) > 50, "ev non-trivial");
    check(strstr(ev, "EPX") != NULL, "ev EPX");
    check(strstr(ev, "CSB") != NULL, "ev CSB");
}

int main(void) {
    printf("=== CSB V2.1 texture upscale probe ===\n");
    p_nearest(); p_bilinear(); p_epx(); p_palette();
    p_scale_roundtrip(); p_full_pipeline();
    p_present_mode_v22_epx_runs(); p_evidence();
    printf("--- %d / %d passed ---\n", g_total - g_failed, g_total);
    return g_failed == 0 ? 0 : 1;
}
