#include "asset_loader_m11.h"
#include "dm1_v1_f0661_damage_material_pc34_compat.h"
#include "font_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const char* home = getenv("HOME");
    const char* root = getenv("FIRESTAFF_DM1_DATA_DIR");
    const unsigned char bogusPalette[16] = {0};
    char path[1024];
    M11_AssetLoader loader;
    M11_FontState font;
    const M11_AssetSlot* damage;
    DM1_V1_F0661SourceSurfacePc34 surface;
    DM1_V1_F0661GlyphSourcePc34 glyph;
    DM1_V1_F0661DamageMaterialReceiptPc34 receipt;

    if (root && root[0]) snprintf(path, sizeof(path), "%s/GRAPHICS.DAT", root);
    else if (home && home[0]) snprintf(path, sizeof(path), "%s/.firestaff/data/dm1/GRAPHICS.DAT", home);
    else return 0;
    if (!M11_AssetLoader_Init(&loader, path)) {
        if (root && root[0]) return 1;
        puts("SKIP: PC34 GRAPHICS.DAT not installed");
        return 0;
    }
    damage = M11_AssetLoader_Load(&loader, DM1_V1_F0661_C014_DAMAGE_TO_CREATURE_PC34);
    if (!damage || !damage->loaded || !damage->pixels ||
        damage->width != DM1_V1_F0661_C014_WIDTH_PC34 ||
        damage->height != DM1_V1_F0661_C014_HEIGHT_PC34) goto fail;
    M11_Font_Init(&font);
    if (!M11_Font_LoadFromGraphicsDat(&font, loader.fileState, loader.runtimeState) ||
        !M11_Font_IsLoaded(&font)) goto fail;
    memset(&surface, 0, sizeof(surface));
    surface.graphicsDatOwned = 1;
    surface.graphicIndex = (int)damage->graphicIndex;
    surface.width = (int)damage->width;
    surface.height = (int)damage->height;
    surface.indexedPixelCount = surface.width * surface.height;
    surface.indexedPixels = damage->pixels;
    surface.pixelsFNV1a = dm1_v1_f0661_damage_material_fnv1a_pc34(
        surface.indexedPixels, surface.indexedPixelCount);
    memset(&glyph, 0, sizeof(glyph));
    glyph.graphicsDatOwned = 1;
    glyph.graphicIndex = M11_Font_ResolvedGraphicIndex(&font);
    glyph.bits = font.bitmap;
    glyph.byteCount = M11_FONT_BITMAP_BYTES;
    glyph.bitsFNV1a = dm1_v1_f0661_damage_material_fnv1a_pc34(
        glyph.bits, glyph.byteCount);
    if (!surface.pixelsFNV1a || !glyph.bitsFNV1a ||
        !dm1_v1_f0661_damage_material_receipt_pc34(
            &surface, &glyph, NULL, &receipt) || !receipt.valid ||
        !receipt.suppressSyntheticFallback || !receipt.originalPaletteUnchanged ||
        receipt.mediumWidth != 64 || receipt.mediumHeight != 37 ||
        receipt.smallWidth != 42 || receipt.smallHeight != 37) goto fail;
    if (dm1_v1_f0661_damage_material_receipt_pc34(
            &surface, &glyph, bogusPalette, &receipt)) goto fail;
    ++glyph.bitsFNV1a;
    if (dm1_v1_f0661_damage_material_receipt_pc34(
            &surface, &glyph, NULL, &receipt)) goto fail;
    --glyph.bitsFNV1a;
    ++surface.pixelsFNV1a;
    if (dm1_v1_f0661_damage_material_receipt_pc34(
            &surface, &glyph, NULL, &receipt)) goto fail;
    M11_AssetLoader_Shutdown(&loader);
    puts("ok: real PC34 F0661 damage material gate");
    return 0;
fail:
    M11_AssetLoader_Shutdown(&loader);
    return 1;
}
