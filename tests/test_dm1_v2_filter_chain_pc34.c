#include "dm1v2/dm1_v2_filters.h"
#include "vga_palette_pc34_compat.h"

#include <stdio.h>
#include <string.h>

int main(void) {
    unsigned char indexed[16] = {
        0x01, 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78,
        0x89, 0x9a, 0xab, 0xbc, 0xcd, 0xde, 0xef, 0xf0
    };
    unsigned char indexed_before[sizeof(indexed)];
    unsigned char rgba[64];
    unsigned char rgba_before[sizeof(rgba)];
    unsigned char lut[DM1_V2_PALETTE_LEVELS][16][3];
    int ok = 1;
    int i;

    for (i = 0; i < (int)sizeof(rgba); ++i) rgba[i] = (unsigned char)(i * 3 + 1);
    memcpy(indexed_before, indexed, sizeof(indexed));
    memcpy(rgba_before, rgba, sizeof(rgba));

    ok &= dm1_v2_filter_palette_interpolate_indexed(indexed, 4, 4, 100) == 0;
    ok &= dm1_v2_filter_dither_cleanup_indexed(indexed, 4, 4) == 0;
    ok &= memcmp(indexed, indexed_before, sizeof(indexed)) == 0;
    ok &= dm1_v2_filter_sharpen_rgba(rgba, 4, 4, 100) == 0;
    ok &= dm1_v2_filter_crt_scanlines_rgba(rgba, 4, 4, 100) == 0;
    ok &= memcmp(rgba, rgba_before, sizeof(rgba)) == 0;
    ok &= dm1_v2_filter_palette_build_lut(260, 50, -50, lut) == 0;
    ok &= memcmp(lut, G9010_auc_VgaPaletteAll_Compat, sizeof(lut)) == 0;
    ok &= dm1_v2_filter_dither_cleanup_indexed(NULL, 4, 4) == -1;
    ok &= dm1_v2_filter_crt_scanlines_rgba(NULL, 4, 4, 100) == -1;

    puts(ok ? "PASS dm1_v2_filter_chain_pc34" : "FAIL dm1_v2_filter_chain_pc34");
    return ok ? 0 : 1;
}
