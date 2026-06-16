/*
 * test_csb_v2_filter_sharpen_rgba_pc34.c
 *
 * CSB V2.0 sharpen per-frame filter test. The filter applies a 3x3
 * unsharp mask on an RGBA surface: out = clamp(orig + (orig - blur) * strength).
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

static void t_invalid(void) {
    int r = csb_v2_filter_sharpen_rgba(NULL, 4, 4, 50);
    check(r == -1, "NULL rgba returns -1");
    uint8_t dummy[16];
    r = csb_v2_filter_sharpen_rgba(dummy, 0, 4, 50);
    check(r == -1, "w=0 returns -1");
    r = csb_v2_filter_sharpen_rgba(dummy, 4, 0, 50);
    check(r == -1, "h=0 returns -1");
    r = csb_v2_filter_sharpen_rgba(dummy, 2, 2, 50);
    check(r == -1, "2x2 returns -1 (needs w,h > 2)");
}

static void t_strength_zero(void) {
    /* strength=0 means no effect; all pixels unchanged.
     * 4x4 RGBA surface. */
    uint8_t rgba[4 * 4 * 4];
    for (int i = 0; i < 16; i++) {
        rgba[i*4+0] = 200; rgba[i*4+1] = 100;
        rgba[i*4+2] = 50;  rgba[i*4+3] = 255;
    }
    int r = csb_v2_filter_sharpen_rgba(rgba, 4, 4, 0);
    check(r == 0, "strength=0 returns 0");
    for (int i = 0; i < 16; i++) {
        check(rgba[i*4+0] == 200, "strength=0 R unchanged");
        check(rgba[i*4+1] == 100, "strength=0 G unchanged");
        check(rgba[i*4+2] == 50,  "strength=0 B unchanged");
        check(rgba[i*4+3] == 255, "strength=0 A unchanged");
    }
}

static void t_alpha_preserved(void) {
    /* The sharpen filter must not change the alpha channel.
     * 4x4 RGBA surface. */
    uint8_t rgba[4 * 4 * 4];
    /* Set each pixel to (200, 100, 50, alpha) where alpha varies. */
    for (int i = 0; i < 16; i++) {
        rgba[i*4+0] = 200; rgba[i*4+1] = 100;
        rgba[i*4+2] = 50;  rgba[i*4+3] = (uint8_t)((i * 17) & 0xFF);
    }
    uint8_t before_alphas[16];
    for (int i = 0; i < 16; i++) before_alphas[i] = rgba[i*4+3];
    int r = csb_v2_filter_sharpen_rgba(rgba, 4, 4, 50);
    check(r == 0, "sharpen returns 0");
    /* Each pixel's alpha byte should be unchanged. */
    int alphas_ok = 1;
    for (int i = 0; i < 16; i++) {
        if (rgba[i*4+3] != before_alphas[i]) alphas_ok = 0;
    }
    check(alphas_ok, "alpha preserved across all pixels");
}

int main(void) {
    printf("=== CSB V2 filter sharpen RGBA test ===\n");
    t_invalid();
    t_strength_zero();
    t_alpha_preserved();
    printf("--- %d / %d passed ---\n", total - failures, total);
    return failures == 0 ? 0 : 1;
}
