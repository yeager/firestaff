#include "asset_loader_m11.h"
#include "dm1_v1_f0663_smoke_material_pc34_compat.h"
#include "dm1_v1_f0675_scaled_material_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const char* root = getenv("FIRESTAFF_DM1_DATA_DIR");
    const char* home = getenv("HOME");
    const int graphics[] = {486, 487, 488};
    char path[1024];
    M11_AssetLoader loader;
    int index;

    if (root && root[0]) snprintf(path, sizeof(path), "%s/GRAPHICS.DAT", root);
    else if (home && home[0]) {
        snprintf(path, sizeof(path), "%s/.firestaff/data/dm1/GRAPHICS.DAT", home);
    } else return 0;
    if (!M11_AssetLoader_Init(&loader, path)) {
        if (root && root[0]) return 1;
        puts("SKIP: PC34 GRAPHICS.DAT not installed");
        return 0;
    }
    for (index = 0; index < (int)(sizeof(graphics) / sizeof(graphics[0])); ++index) {
        const M11_AssetSlot* slot = M11_AssetLoader_Load(&loader, (unsigned int)graphics[index]);
        DM1_V1_F0675SourceSurfacePc34 surface;
        DM1_V1_F0675ScaledMaterialReceiptPc34 receipt;
        unsigned char alteredPalette[DM1_V1_F0663_PALETTE_COUNT_PC34];
        if (!slot || !slot->loaded || !slot->pixels || !slot->width || !slot->height) goto fail;
        memset(&surface, 0, sizeof(surface));
        surface.graphicsDatOwned = 1;
        surface.graphicIndex = (int)slot->graphicIndex;
        surface.width = (int)slot->width;
        surface.height = (int)slot->height;
        surface.indexedPixelCount = surface.width * surface.height;
        surface.indexedPixels = slot->pixels;
        surface.pixelsFNV1a = dm1_v1_f0675_scaled_material_fnv1a_pc34(
            surface.indexedPixels, surface.indexedPixelCount);
        if (!dm1_v1_f0675_scaled_material_receipt_pc34(
                &surface, surface.width + 1, surface.height + 1,
                NULL, 0, &receipt) || !receipt.valid ||
            !receipt.suppressSyntheticFallback ||
            receipt.graphicIndex != graphics[index]) goto fail;
        if (graphics[index] == 488 &&
            !dm1_v1_f0675_scaled_material_receipt_pc34(
                &surface, surface.width, surface.height,
                dm1_v1_f0663_smoke_palette_changes_pc34(),
                DM1_V1_F0663_PALETTE_COUNT_PC34, &receipt)) goto fail;
        if (graphics[index] == 488) {
            memcpy(alteredPalette, dm1_v1_f0663_smoke_palette_changes_pc34(),
                   sizeof(alteredPalette));
            alteredPalette[6] = 6;
            if (dm1_v1_f0675_scaled_material_receipt_pc34(
                    &surface, surface.width, surface.height, alteredPalette,
                    DM1_V1_F0663_PALETTE_COUNT_PC34, &receipt)) goto fail;
        }
        ++surface.pixelsFNV1a;
        if (dm1_v1_f0675_scaled_material_receipt_pc34(
                &surface, surface.width, surface.height, NULL, 0, &receipt)) goto fail;
    }
    M11_AssetLoader_Shutdown(&loader);
    puts("ok: real PC34 F0675 scaled material gate");
    return 0;
fail:
    M11_AssetLoader_Shutdown(&loader);
    return 1;
}
