#include "asset_loader_m11.h"
#include "dm1_v1_f0659_shield_material_pc34_compat.h"
#include "font_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const char* home = getenv("HOME");
    const char* root = getenv("FIRESTAFF_DM1_DATA_DIR");
    char path[1024];
    M11_AssetLoader loader;
    M11_FontState font;
    DM1_V1_F0659SourceSurfacePc34 surfaces[3];
    DM1_V1_F0659GlyphSourcePc34 glyph;
    DM1_V1_F0659ShieldMaterialReceiptPc34 receipt;
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
    memset(&glyph, 0, sizeof(glyph));
    for (i = 0; i < 3; ++i) {
        const M11_AssetSlot* slot = M11_AssetLoader_Load(&loader, (unsigned int)(37 + i));
        if (!slot || !slot->loaded || !slot->pixels ||
            slot->width != 67 || slot->height != 29) goto fail;
        surfaces[i].graphicsDatOwned = 1;
        surfaces[i].graphicIndex = 37 + i;
        surfaces[i].width = slot->width;
        surfaces[i].height = slot->height;
        surfaces[i].indexedPixelCount = 67 * 29;
        surfaces[i].indexedPixels = slot->pixels;
        surfaces[i].pixelsFNV1a = dm1_v1_f0659_shield_material_fnv1a_pc34(
            slot->pixels, surfaces[i].indexedPixelCount);
    }
    M11_Font_Init(&font);
    if (!M11_Font_LoadFromGraphicsDat(&font, loader.fileState, loader.runtimeState) ||
        !M11_Font_IsLoaded(&font)) goto fail;
    glyph.graphicsDatOwned = 1;
    glyph.graphicIndex = M11_Font_ResolvedGraphicIndex(&font);
    glyph.bits = font.bitmap;
    glyph.byteCount = M11_FONT_BITMAP_BYTES;
    glyph.bitsFNV1a = dm1_v1_f0659_shield_material_fnv1a_pc34(glyph.bits, glyph.byteCount);
    if (!dm1_v1_f0659_shield_material_receipt_pc34(surfaces, 3, &glyph, 1, &receipt) ||
        !receipt.valid || receipt.transparentColor != 10 ||
        receipt.sourceWidth != 67 || receipt.sourceHeight != 29 ||
        receipt.m653GraphicIndex != glyph.graphicIndex) goto fail;
    ++surfaces[2].pixelsFNV1a;
    if (dm1_v1_f0659_shield_material_receipt_pc34(surfaces, 3, &glyph, 1, &receipt)) goto fail;
    M11_AssetLoader_Shutdown(&loader);
    puts("ok: real PC34 F0659 shield material gate");
    return 0;
fail:
    M11_AssetLoader_Shutdown(&loader);
    return 1;
}
