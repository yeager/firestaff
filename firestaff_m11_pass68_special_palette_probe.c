/*
 * Pass 68 bounded probe — DM1 PC 3.4 special VGA palettes.
 *
 * Scope: V1 graphics blocker for credits/entrance palettes only.
 * Verifies the source-backed G8147_CREDITS and G8148_ENTRANCE palettes
 * are exposed through the compat palette layer instead of falling back
 * to the base dungeon/title palette.
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

static int special_eq(unsigned int palette,
                      unsigned char color,
                      unsigned char r,
                      unsigned char g,
                      unsigned char b) {
    return rgb_eq(F9011_VGA_GetSpecialColorRgb_Compat(color, palette), r, g, b);
}

int main(void) {
    const unsigned char* badColor;
    const unsigned char* badPalette;

    record("INV_P68_01",
           VGA_PALETTE_PC34_COLOR_COUNT == 16 &&
           VGA_PALETTE_PC34_SPECIAL_PALETTE_COUNT == 2 &&
           VGA_PALETTE_PC34_SPECIAL_CREDITS == 0 &&
           VGA_PALETTE_PC34_SPECIAL_ENTRANCE == 1,
           "special palette namespace exposes credits and entrance palettes");

    record("INV_P68_02",
           special_eq(VGA_PALETTE_PC34_SPECIAL_CREDITS, 0, 0, 0, 109) &&
           special_eq(VGA_PALETTE_PC34_SPECIAL_CREDITS, 1, 0, 182, 182) &&
           special_eq(VGA_PALETTE_PC34_SPECIAL_CREDITS, 2, 255, 255, 109) &&
           special_eq(VGA_PALETTE_PC34_SPECIAL_CREDITS, 15, 255, 255, 219),
           "credits palette matches G8147_CREDITS sampled COLOR_DEF rows");

    record("INV_P68_03",
           special_eq(VGA_PALETTE_PC34_SPECIAL_ENTRANCE, 4, 219, 182, 146) &&
           special_eq(VGA_PALETTE_PC34_SPECIAL_ENTRANCE, 5, 0, 219, 0) &&
           special_eq(VGA_PALETTE_PC34_SPECIAL_ENTRANCE, 9, 255, 0, 0) &&
           special_eq(VGA_PALETTE_PC34_SPECIAL_ENTRANCE, 15, 255, 255, 255),
           "entrance palette matches G8148_ENTRANCE sampled COLOR_DEF rows");

    record("INV_P68_04",
           !rgb_eq(F9011_VGA_GetSpecialColorRgb_Compat(4, VGA_PALETTE_PC34_SPECIAL_CREDITS),
                   0, 219, 219) &&
           !rgb_eq(F9011_VGA_GetSpecialColorRgb_Compat(5, VGA_PALETTE_PC34_SPECIAL_ENTRANCE),
                   146, 73, 0),
           "special palettes are distinct from the base dungeon/title palette");

    record("INV_P68_05",
           G9013_auc_VgaPaletteSpecial_Compat[VGA_PALETTE_PC34_SPECIAL_CREDITS][11][0] == 255 &&
           G9013_auc_VgaPaletteSpecial_Compat[VGA_PALETTE_PC34_SPECIAL_CREDITS][11][1] == 219 &&
           G9013_auc_VgaPaletteSpecial_Compat[VGA_PALETTE_PC34_SPECIAL_CREDITS][11][2] == 0 &&
           G9013_auc_VgaPaletteSpecial_Compat[VGA_PALETTE_PC34_SPECIAL_ENTRANCE][11][0] == 109 &&
           G9013_auc_VgaPaletteSpecial_Compat[VGA_PALETTE_PC34_SPECIAL_ENTRANCE][11][1] == 73 &&
           G9013_auc_VgaPaletteSpecial_Compat[VGA_PALETTE_PC34_SPECIAL_ENTRANCE][11][2] == 36,
           "indexed special-palette table maps both palettes without caller-side branching");

    badColor = F9011_VGA_GetSpecialColorRgb_Compat(16, VGA_PALETTE_PC34_SPECIAL_CREDITS);
    badPalette = F9011_VGA_GetSpecialColorRgb_Compat(0, VGA_PALETTE_PC34_SPECIAL_PALETTE_COUNT);
    record("INV_P68_06",
           badColor == 0 && badPalette == 0,
           "out-of-range color and special palette are rejected");

    printf("# summary: %d/%d invariants passed\n", g_pass, g_pass + g_fail);
    return g_fail ? 1 : 0;
}
