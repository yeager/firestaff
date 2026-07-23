#include "asset_loader_m11.h"
#include "dm1_v1_f0663_smoke_material_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const char* home = getenv("HOME");
    const char* root = getenv("FIRESTAFF_DM1_DATA_DIR");
    const unsigned char* paletteChanges;
    unsigned char alteredPalette[DM1_V1_F0663_PALETTE_COUNT_PC34];
    const int graphics[DM1_V1_F0663_SURFACE_COUNT_PC34] = {
        DM1_V1_F0663_C488_POISON_SOURCE_PC34,
        DM1_V1_F0663_C498_SMOKE_PATTERN_SMALL_PC34,
        DM1_V1_F0663_C499_SMOKE_PATTERN_MEDIUM_PC34,
        DM1_V1_F0663_C500_SMOKE_PATTERN_LARGE_PC34
    };
    char path[1024];
    M11_AssetLoader loader;
    DM1_V1_F0663SourceSurfacePc34 surfaces[DM1_V1_F0663_SURFACE_COUNT_PC34];
    DM1_V1_F0663SmokeMaterialReceiptPc34 receipt;
    int i;

    if (root && root[0]) snprintf(path, sizeof(path), "%s/GRAPHICS.DAT", root);
    else if (home && home[0]) snprintf(path, sizeof(path), "%s/.firestaff/data/dm1/GRAPHICS.DAT", home);
    else return 0;
    if (!M11_AssetLoader_Init(&loader, path)) {
        if (root && root[0]) return 1;
        puts("SKIP: PC34 GRAPHICS.DAT not installed");
        return 0;
    }
    memset(surfaces, 0, sizeof(surfaces));
    for (i = 0; i < DM1_V1_F0663_SURFACE_COUNT_PC34; ++i) {
        const M11_AssetSlot* slot = M11_AssetLoader_Load(&loader, (unsigned int)graphics[i]);
        if (!slot || !slot->loaded || !slot->pixels ||
            slot->width == 0 || slot->height == 0) goto fail;
        surfaces[i].graphicsDatOwned = 1;
        surfaces[i].graphicIndex = (int)slot->graphicIndex;
        surfaces[i].width = (int)slot->width;
        surfaces[i].height = (int)slot->height;
        surfaces[i].indexedPixelCount = surfaces[i].width * surfaces[i].height;
        surfaces[i].indexedPixels = slot->pixels;
        surfaces[i].pixelsFNV1a = dm1_v1_f0663_smoke_material_fnv1a_pc34(
            slot->pixels, surfaces[i].indexedPixelCount);
    }
    paletteChanges = dm1_v1_f0663_smoke_palette_changes_pc34();
    if (!dm1_v1_f0663_smoke_material_receipt_pc34(
            surfaces, DM1_V1_F0663_SURFACE_COUNT_PC34, paletteChanges,
            DM1_V1_F0663_PALETTE_COUNT_PC34, &receipt) || !receipt.valid ||
        !receipt.suppressSyntheticFallback ||
        receipt.poisonGraphicIndex != DM1_V1_F0663_C488_POISON_SOURCE_PC34 ||
        receipt.smokePatternFirstGraphicIndex !=
            DM1_V1_F0663_C498_SMOKE_PATTERN_SMALL_PC34 ||
        receipt.replacementSourceA != 6 || receipt.replacementDestinationA != 12 ||
        receipt.replacementSourceB != 7 || receipt.replacementDestinationB != 1) goto fail;
    memcpy(alteredPalette, paletteChanges, sizeof(alteredPalette));
    alteredPalette[7] = 7;
    if (dm1_v1_f0663_smoke_material_receipt_pc34(
            surfaces, DM1_V1_F0663_SURFACE_COUNT_PC34, alteredPalette,
            sizeof(alteredPalette), &receipt)) goto fail;
    ++surfaces[3].pixelsFNV1a;
    if (dm1_v1_f0663_smoke_material_receipt_pc34(
            surfaces, DM1_V1_F0663_SURFACE_COUNT_PC34, paletteChanges,
            DM1_V1_F0663_PALETTE_COUNT_PC34, &receipt)) goto fail;
    M11_AssetLoader_Shutdown(&loader);
    puts("ok: real PC34 F0663 smoke material gate");
    return 0;
fail:
    M11_AssetLoader_Shutdown(&loader);
    return 1;
}
