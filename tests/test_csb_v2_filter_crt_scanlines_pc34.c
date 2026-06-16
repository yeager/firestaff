/*
 * test_csb_v2_filter_crt_scanlines_pc34.c
 *
 * CSB V2.0 CRT scanlines per-frame filter test. The filter dims
 * even-numbered rows of an RGBA surface for a CRT effect.
 * Source: include/csb_v2_filters.h
 */
#include "csb_v2_filters.h"
#include <stdio.h>
#include <string.h>
static int failures = 0;
static int total = 0;
static void check(int cond, const char* name) {
    total++;
    if (!cond) { failures++; fprintf(stderr, "FAIL: %s\n", name); }
    else printf("PASS: %s\n", name);
}

static void t_basic(void) {
    /* 4x4 RGBA: each pixel is R=200, G=100, B=50, A=255.
     * With strength=50, even rows (0, 2) are dimmed to 50%
     * (R=100, G=50, B=25, A=255); odd rows (1, 3) are unchanged. */
    uint8_t rgba[4 * 4 * 4];
    for (int i = 0; i < 16; i++) {
        rgba[i*4+0] = 200; rgba[i*4+1] = 100; rgba[i*4+2] = 50; rgba[i*4+3] = 255;
    }
    int r = csb_v2_filter_crt_scanlines_rgba(rgba, 4, 4, 50);
    check(r == 0, "basic returns 0");
    /* Row 0 (even) dimmed */
    check(rgba[0*16+0] == 100, "R row 0 = 100");
    check(rgba[0*16+1] == 50, "G row 0 = 50");
    check(rgba[0*16+2] == 25, "B row 0 = 25");
    check(rgba[0*16+3] == 255, "A row 0 unchanged");
    /* Row 1 (odd) unchanged */
    check(rgba[1*16+0] == 200, "R row 1 = 200");
    check(rgba[1*16+3] == 255, "A row 1 unchanged");
    /* Row 2 (even) dimmed */
    check(rgba[2*16+0] == 100, "R row 2 = 100");
    /* Row 3 (odd) unchanged */
    check(rgba[3*16+0] == 200, "R row 3 = 200");
}

static void t_strength_zero(void) {
    /* strength=0 means no effect, all pixels unchanged. */
    uint8_t rgba[2 * 2 * 4] = { 200, 100, 50, 255, 200, 100, 50, 255,
                               200, 100, 50, 255, 200, 100, 50, 255 };
    int r = csb_v2_filter_crt_scanlines_rgba(rgba, 2, 2, 0);
    check(r == 0, "strength=0 returns 0");
    for (int i = 0; i < 4; i++) {
        check(rgba[i*4+0] == 200, "strength=0 R unchanged");
        check(rgba[i*4+3] == 255, "strength=0 A unchanged");
    }
}

static void t_strength_100(void) {
    /* strength=100 means maximum dimming: even rows are zero. */
    uint8_t rgba[2 * 2 * 4] = { 200, 100, 50, 255, 200, 100, 50, 255,
                               200, 100, 50, 255, 200, 100, 50, 255 };
    int r = csb_v2_filter_crt_scanlines_rgba(rgba, 2, 2, 100);
    check(r == 0, "strength=100 returns 0");
    /* Row 0 (even): all RGB channels should be 0, A preserved. */
    check(rgba[0*4+0] == 0, "R row 0 = 0");
    check(rgba[0*4+1] == 0, "G row 0 = 0");
    check(rgba[0*4+2] == 0, "B row 0 = 0");
    check(rgba[0*4+3] == 255, "A row 0 preserved");
    /* Row 1 (odd) unchanged. Row 1 starts at byte w*4 = 8. */
    check(rgba[1*2*4+0] == 200, "R row 1 unchanged");
}

static void t_invalid(void) {
    int r = csb_v2_filter_crt_scanlines_rgba(NULL, 4, 4, 50);
    check(r == -1, "NULL rgba returns -1");
    uint8_t dummy[16];
    r = csb_v2_filter_crt_scanlines_rgba(dummy, 0, 4, 50);
    check(r == -1, "w=0 returns -1");
    r = csb_v2_filter_crt_scanlines_rgba(dummy, 4, 0, 50);
    check(r == -1, "h=0 returns -1");
}

static void t_strength_clamps(void) {
    /* Out-of-range strength should be tolerated (clamped internally). */
    uint8_t rgba[2 * 2 * 4] = { 100, 100, 100, 255, 100, 100, 100, 255,
                               100, 100, 100, 255, 100, 100, 100, 255 };
    int r = csb_v2_filter_crt_scanlines_rgba(rgba, 2, 2, 200);
    check(r == 0, "strength=200 (OOB) returns 0 (clamped)");
    r = csb_v2_filter_crt_scanlines_rgba(rgba, 2, 2, -5);
    check(r == 0, "strength=-5 (OOB) returns 0 (clamped)");
}

int main(void) {
    printf("=== CSB V2 filter CRT scanlines test ===\n");
    t_basic();
    t_strength_zero();
    t_strength_100();
    t_invalid();
    t_strength_clamps();
    printf("--- %d / %d passed ---\n", total - failures, total);
    return failures == 0 ? 0 : 1;
}
