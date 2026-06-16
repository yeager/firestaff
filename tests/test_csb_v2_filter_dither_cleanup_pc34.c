/*
 * test_csb_v2_filter_dither_cleanup_pc34.c
 *
 * CSB V2.0 dither cleanup per-frame filter test. The filter applies
 * a 3x3 mode filter on an indexed framebuffer: the low-4-bit palette
 * index is replaced by the statistical mode of the 3x3 neighborhood
 * only when the mode is strictly more common than the center pixel.
 * The high-4-bit brightness level is preserved unchanged.
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

/* Pack a pixel: high 4 bits = level (0..15), low 4 bits = palette index. */
static uint8_t pack(int level, int index) {
    return (uint8_t)((level << 4) | (index & 0x0F));
}

static void t_invalid(void) {
    int r = csb_v2_filter_dither_cleanup_indexed(NULL, 4, 4);
    check(r == -1, "NULL fb returns -1");
    uint8_t dummy[16];
    r = csb_v2_filter_dither_cleanup_indexed(dummy, 0, 4);
    check(r == -1, "w=0 returns -1");
    r = csb_v2_filter_dither_cleanup_indexed(dummy, 4, 0);
    check(r == -1, "h=0 returns -1");
}

static void t_uniform(void) {
    /* Uniform 3x3: all pixels same. Mode == center, no replacement. */
    uint8_t fb[9];
    for (int i = 0; i < 9; i++) fb[i] = pack(2, 5);
    int r = csb_v2_filter_dither_cleanup_indexed(fb, 3, 3);
    check(r == 0, "uniform returns 0");
    /* All pixels still pack(2, 5) (level 2, index 5). */
    int same = 1;
    for (int i = 0; i < 9; i++) {
        if (fb[i] != pack(2, 5)) same = 0;
    }
    check(same, "uniform: all pixels unchanged");
}

static void t_level_preserved(void) {
    /* The high-4-bit level must NEVER be changed. */
    uint8_t fb[9];
    for (int i = 0; i < 9; i++) fb[i] = pack(7, 1);
    int r = csb_v2_filter_dither_cleanup_indexed(fb, 3, 3);
    check(r == 0, "level preserved test returns 0");
    int level_ok = 1;
    for (int i = 0; i < 9; i++) {
        if ((fb[i] >> 4) != 7) level_ok = 0;
    }
    check(level_ok, "level (high 4 bits) preserved across all pixels");
}

static void t_oversized(void) {
    /* 4096x4096 is too large; function should return -1. */
    uint8_t dummy[16];
    int r = csb_v2_filter_dither_cleanup_indexed(dummy, 4096, 4096);
    check(r == -1, "oversized surface returns -1");
}

int main(void) {
    printf("=== CSB V2 filter dither cleanup test ===\n");
    t_invalid();
    t_uniform();
    t_level_preserved();
    t_oversized();
    printf("--- %d / %d passed ---\n", total - failures, total);
    return failures == 0 ? 0 : 1;
}
