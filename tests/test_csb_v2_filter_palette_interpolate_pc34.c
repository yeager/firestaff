/*
 * test_csb_v2_filter_palette_interpolate_pc34.c
 *
 * CSB V2.0 palette interpolation per-frame filter test. The filter
 * smooths per-pixel brightness on an indexed framebuffer. The
 * high-4-bit level field is interpolated between adjacent canonical
 * levels. The low-4-bit palette index is preserved.
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

static uint8_t pack(int level, int index) {
    return (uint8_t)((level << 4) | (index & 0x0F));
}

static void t_invalid(void) {
    int r = csb_v2_filter_palette_interpolate_indexed(NULL, 4, 4, 50);
    check(r == -1, "NULL fb returns -1");
    uint8_t dummy[16];
    r = csb_v2_filter_palette_interpolate_indexed(dummy, 0, 4, 50);
    check(r == -1, "w=0 returns -1");
    r = csb_v2_filter_palette_interpolate_indexed(dummy, 4, 0, 50);
    check(r == -1, "h=0 returns -1");
    r = csb_v2_filter_palette_interpolate_indexed(dummy, 4, 1, 50);
    check(r == -1, "h=1 returns -1 (needs h > 2)");
}

static void t_index_preserved(void) {
    /* The low-4-bit palette index should be unchanged for any strength.
     * 4x4 surface (4 pixels wide × 4 pixels tall). */
    uint8_t fb[16] = { 0 };
    for (int i = 0; i < 16; i++) fb[i] = pack(0, 7);
    int r = csb_v2_filter_palette_interpolate_indexed(fb, 4, 4, 100);
    check(r == 0, "returns 0");
    for (int i = 0; i < 16; i++) {
        check((fb[i] & 0x0F) == 7, "index preserved");
    }
}

static void t_strength_zero(void) {
    /* strength=0 means no interpolation; pixel levels unchanged.
     * 4x4 surface. */
    uint8_t fb[16] = { 0 };
    fb[0] = pack(0, 1); fb[5] = pack(7, 2); fb[10] = pack(15, 3);
    /* Snapshot before */
    uint8_t before[16];
    memcpy(before, fb, 16);
    int r = csb_v2_filter_palette_interpolate_indexed(fb, 4, 4, 0);
    check(r == 0, "strength=0 returns 0");
    check(memcmp(fb, before, 16) == 0, "strength=0: pixels unchanged");
}

int main(void) {
    printf("=== CSB V2 filter palette interpolate test ===\n");
    t_invalid();
    t_index_preserved();
    t_strength_zero();
    printf("--- %d / %d passed ---\n", total - failures, total);
    return failures == 0 ? 0 : 1;
}
