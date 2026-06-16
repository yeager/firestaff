/*
 * test_csb_v2_filter_palette_build_lut_pc34.c
 *
 * CSB V2.0 palette correction LUT builder test. Builds a corrected
 * [levels][16][3] RGB LUT from the VGA palette with gamma/brightness/
 * contrast adjustments. Source: include/csb_v2_filters.h
 */
#include "csb_v2_filters.h"
#include "csb_v2_asset_pipeline_pc34.h"
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
    uint8_t lut[CSB_V2_PALETTE_LEVELS][16][3];
    int r = csb_v2_filter_palette_build_lut(100, 0, 0, NULL);
    check(r == -1, "NULL out_lut returns -1");
    /* Now with a valid pointer. */
    r = csb_v2_filter_palette_build_lut(100, 0, 0, lut);
    check(r == 0, "valid out_lut returns 0");
}

static void t_lut_dimensions(void) {
    /* The LUT has CSB_V2_PALETTE_LEVELS entries, each with 16 indices,
     * each with 3 channels. */
    uint8_t lut[CSB_V2_PALETTE_LEVELS][16][3];
    int r = csb_v2_filter_palette_build_lut(220, 0, 0, lut);
    check(r == 0, "LUT build with gamma=220 returns 0");
    /* Spot-check that the LUT is non-zero (some valid output). */
    int nonzero = 0;
    for (int level = 0; level < CSB_V2_PALETTE_LEVELS; level++) {
        for (int idx = 0; idx < 16; idx++) {
            if (lut[level][idx][0] || lut[level][idx][1] || lut[level][idx][2]) {
                nonzero = 1;
            }
        }
    }
    check(nonzero, "LUT has non-zero entries");
}

static void t_brightness_offset(void) {
    /* brightness=+50 should produce higher channel values than
     * brightness=0 for at least one entry. */
    uint8_t lutA[CSB_V2_PALETTE_LEVELS][16][3];
    uint8_t lutB[CSB_V2_PALETTE_LEVELS][16][3];
    csb_v2_filter_palette_build_lut(100, 0, 0, lutA);
    csb_v2_filter_palette_build_lut(100, 50, 0, lutB);
    int differ = 0;
    for (int level = 0; level < CSB_V2_PALETTE_LEVELS; level++) {
        for (int idx = 0; idx < 16; idx++) {
            for (int c = 0; c < 3; c++) {
                if (lutB[level][idx][c] != lutA[level][idx][c]) {
                    differ = 1;
                }
            }
        }
    }
    check(differ, "brightness=+50 changes LUT values");
}

int main(void) {
    printf("=== CSB V2 filter palette build LUT test ===\n");
    t_invalid();
    t_lut_dimensions();
    t_brightness_offset();
    printf("--- %d / %d passed ---\n", total - failures, total);
    return failures == 0 ? 0 : 1;
}
