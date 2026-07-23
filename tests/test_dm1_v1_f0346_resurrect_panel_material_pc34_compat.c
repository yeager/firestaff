#include "dm1_v1_f0346_resurrect_panel_material_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int check(int value, const char *message)
{
    if (!value) fprintf(stderr, "FAIL: %s\n", message);
    return value;
}

int main(void)
{
    static unsigned char pixels[224 * 136];
    DM1_V1_F0355SourceSurfacePc34 surface;
    DM1_V1_F0346ResurrectPanelMaterialReceiptPc34 receipt;
    int ok = 1;

    memset(&surface, 0, sizeof(surface));
    pixels[0] = 0x12u;
    pixels[sizeof(pixels) - 1u] = 0x6au;
    surface.graphicsDatOwned = 1;
    surface.graphicIndex = DM1_V1_F0346_C040_RESURRECT_PANEL_PC34;
    surface.width = DM1_V1_F0346_PANEL_WIDTH_PC34;
    surface.height = DM1_V1_F0346_PANEL_HEIGHT_PC34;
    surface.indexedPixelCount = (int)sizeof(pixels);
    surface.indexedPixels = pixels;
    surface.pixelsFNV1a = dm1_v1_f0355_inventory_material_fnv1a_pc34(
        pixels, (int)sizeof(pixels));

    ok &= check(dm1_v1_f0346_resurrect_panel_material_receipt_pc34(
                    &surface, 1, &receipt) && receipt.valid &&
                    receipt.suppressSyntheticFallback &&
                    receipt.graphicIndex == DM1_V1_F0346_C040_RESURRECT_PANEL_PC34 &&
                    receipt.materialFingerprint == surface.pixelsFNV1a,
                "F0346 accepts only hash-matched original C040 material");
    surface.pixelsFNV1a ^= 1u;
    ok &= check(!dm1_v1_f0346_resurrect_panel_material_receipt_pc34(
                    &surface, 1, &receipt) && !receipt.valid,
                "F0346 rejects altered GRAPHICS.DAT provenance");
    surface.pixelsFNV1a ^= 1u;
    surface.graphicsDatOwned = 0;
    ok &= check(!dm1_v1_f0346_resurrect_panel_material_receipt_pc34(
                    &surface, 1, &receipt) && !receipt.valid,
                "F0346 rejects non-source C040 surfaces");
    if (!ok) return 1;
    puts("PASS: DM1 F0346 C040 GRAPHICS.DAT material admission");
    return 0;
}
