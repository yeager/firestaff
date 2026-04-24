/*
 * Pass 46 bounded probe — DM1 PC 3.4 VGA palette lookup tables.
 *
 * Scope: V1_BLOCKERS.md §10 base palette + brightness lookup only.
 * Verifies that vga_palette_pc34_compat.c exposes the recovered
 * VIDEODRV.C palette values instead of the old EGA/linear model.
 */

#include "vga_palette_pc34_compat.h"

#include <stdio.h>

static int g_pass = 0;
static int g_fail = 0;

static void record(const char* id, int ok, const char* msg) {
    if (ok) {
        ++g_pass;
        printf("PASS %s %s\n", id, msg);
    } else {
        ++g_fail;
        printf("FAIL %s %s\n", id, msg);
    }
}

static int rgb_eq(const unsigned char* rgb,
                  unsigned char r,
                  unsigned char g,
                  unsigned char b) {
    return rgb && rgb[0] == r && rgb[1] == g && rgb[2] == b;
}

static int palette_eq(unsigned int level,
                      unsigned char color,
                      unsigned char r,
                      unsigned char g,
                      unsigned char b) {
    return rgb_eq(F9010_VGA_GetColorRgb_Compat(color, level), r, g, b);
}

int main(void) {
    int level;
    int cyanInvariant = 1;
    int light5NonBlack = 0;
    const unsigned char* badColor;
    const unsigned char* badLevel;

    record("INV_P46_01",
           VGA_PALETTE_PC34_COLOR_COUNT == 16 &&
           VGA_PALETTE_PC34_BRIGHTNESS_LEVELS == 6,
           "palette exposes 16 colors and 6 original brightness levels");

    record("INV_P46_02",
           palette_eq(0, 4, 0, 219, 219),
           "base palette index 4 is original VGA cyan, not EGA dark red");

    record("INV_P46_03",
           palette_eq(0, 3, 109, 36, 0) &&
           palette_eq(0, 10, 219, 146, 109) &&
           palette_eq(0, 14, 0, 0, 255),
           "base palette carries source-backed brown/tan/blue VGA values");

    for (level = 0; level < VGA_PALETTE_PC34_BRIGHTNESS_LEVELS; ++level) {
        if (!palette_eq((unsigned int)level, 4, 0, 219, 219)) {
            cyanInvariant = 0;
        }
    }
    record("INV_P46_04",
           cyanInvariant,
           "cyan index 4 is invariant across all 6 brightness tables");

    for (level = 0; level < 16; ++level) {
        const unsigned char* rgb = F9010_VGA_GetColorRgb_Compat((unsigned char)level, 5);
        if (rgb && (rgb[0] || rgb[1] || rgb[2])) {
            ++light5NonBlack;
        }
    }
    record("INV_P46_05",
           light5NonBlack == 8,
           "LIGHT5 is source lookup with 8 residual non-black colors, not all-black linear attenuation");

    record("INV_P46_06",
           palette_eq(1, 1, 73, 73, 73) &&
           palette_eq(2, 11, 255, 182, 0) &&
           palette_eq(3, 14, 0, 0, 146) &&
           palette_eq(4, 15, 109, 109, 109) &&
           palette_eq(5, 11, 109, 73, 0),
           "brightness levels use recovered per-level VIDEODRV.C lookup values");

    badColor = F9010_VGA_GetColorRgb_Compat(16, 0);
    badLevel = F9010_VGA_GetColorRgb_Compat(0, 6);
    record("INV_P46_07",
           badColor == 0 && badLevel == 0,
           "out-of-range color and palette level remain rejected");

    printf("# summary: %d/%d invariants passed\n", g_pass, g_pass + g_fail);
    return g_fail ? 1 : 0;
}
