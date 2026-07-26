#include "asset_loader_m11.h"
#include "dm1_v1_f0355_inventory_material_pc34_compat.h"
#include "font_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const char* home = getenv("HOME");
    const char* root = getenv("FIRESTAFF_DM1_DATA_DIR");
    char path[2048];
    M11_AssetLoader loader;
    M11_FontState font;
    DM1_V1_F0355SourceSurfacePc34 surfaces[2];
    DM1_V1_F0355GlyphSourcePc34 glyph;
    DM1_V1_F0355InventoryMaterialReceiptPc34 receipt;
    const M11_AssetSlot* inventory;
    const M11_AssetSlot* slot;

    if (root && root[0]) snprintf(path, sizeof(path), "%s/GRAPHICS.DAT", root);
    else if (home && home[0]) snprintf(path, sizeof(path), "%s/.firestaff/data/dm1/GRAPHICS.DAT", home);
    else return 0;
    if (!M11_AssetLoader_Init(&loader, path)) {
        if (root && root[0]) return 1;
        puts("SKIP: PC34 GRAPHICS.DAT not installed");
        return 0;
    }
    memset(surfaces, 0, sizeof(surfaces));
    memset(&glyph, 0, sizeof(glyph));
    inventory = M11_AssetLoader_Load(&loader, 17u);
    slot = M11_AssetLoader_Load(&loader, 33u);
    if (!inventory || !slot || !inventory->loaded || !slot->loaded ||
        !inventory->pixels || !slot->pixels || inventory->width != 224 ||
        inventory->height != 136 || slot->width != 18 || slot->height != 18) goto fail;
    surfaces[0].graphicsDatOwned = 1;
    surfaces[0].graphicIndex = 17;
    surfaces[0].width = inventory->width;
    surfaces[0].height = inventory->height;
    surfaces[0].indexedPixelCount = 224 * 136;
    surfaces[0].indexedPixels = inventory->pixels;
    surfaces[0].pixelsFNV1a = dm1_v1_f0355_inventory_material_fnv1a_pc34(
        inventory->pixels, surfaces[0].indexedPixelCount);
    surfaces[1].graphicsDatOwned = 1;
    surfaces[1].graphicIndex = 33;
    surfaces[1].width = slot->width;
    surfaces[1].height = slot->height;
    surfaces[1].indexedPixelCount = 18 * 18;
    surfaces[1].indexedPixels = slot->pixels;
    surfaces[1].pixelsFNV1a = dm1_v1_f0355_inventory_material_fnv1a_pc34(
        slot->pixels, surfaces[1].indexedPixelCount);
    M11_Font_Init(&font);
    if (!M11_Font_LoadFromGraphicsDat(&font, loader.fileState, loader.runtimeState) ||
        !M11_Font_IsLoaded(&font)) goto fail;
    glyph.graphicsDatOwned = 1;
    glyph.graphicIndex = M11_Font_ResolvedGraphicIndex(&font);
    glyph.bits = font.bitmap;
    glyph.byteCount = M11_FONT_BITMAP_BYTES;
    glyph.bitsFNV1a = dm1_v1_f0355_inventory_material_fnv1a_pc34(
        glyph.bits, glyph.byteCount);
    if (!dm1_v1_f0355_inventory_material_receipt_pc34(surfaces, 2, &glyph, 1,
                                                       &receipt) ||
        !receipt.valid || receipt.inventoryWidth != 224 ||
        receipt.inventoryHeight != 136 || receipt.slotWidth != 18 ||
        receipt.slotHeight != 18 || receipt.m653GraphicIndex != glyph.graphicIndex) goto fail;
    ++surfaces[0].pixelsFNV1a;
    if (dm1_v1_f0355_inventory_material_receipt_pc34(surfaces, 2, &glyph, 1,
                                                      &receipt)) goto fail;
    M11_AssetLoader_Shutdown(&loader);
    puts("ok: real PC34 F0355 inventory material gate");
    return 0;
fail:
    M11_AssetLoader_Shutdown(&loader);
    return 1;
}
