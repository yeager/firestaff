#include "asset_loader_m11.h"
#include "dm1_v1_f0662_invisibility_material_pc34_compat.h"
#include "font_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const char* home = getenv("HOME");
    const char* root = getenv("FIRESTAFF_DM1_DATA_DIR");
    const unsigned char* paletteChanges;
    unsigned char alteredPalette[DM1_V1_F0662_PALETTE_CHANGE_COUNT_PC34];
    char path[2048];
    M11_AssetLoader loader;
    M11_FontState font;
    const M11_AssetSlot* icon;
    DM1_V1_F0662SourceSurfacePc34 source;
    DM1_V1_F0662GlyphSourcePc34 glyph;
    DM1_V1_F0662InvisibilityMaterialReceiptPc34 receipt;

    if (root && root[0]) snprintf(path, sizeof(path), "%s/GRAPHICS.DAT", root);
    else if (home && home[0]) snprintf(path, sizeof(path), "%s/.firestaff/data/dm1/GRAPHICS.DAT", home);
    else return 0;
    if (!M11_AssetLoader_Init(&loader, path)) {
        if (root && root[0]) return 1;
        puts("SKIP: PC34 GRAPHICS.DAT not installed");
        return 0;
    }
    icon = M11_AssetLoader_Load(&loader, DM1_V1_F0662_C028_CHAMPION_ICONS_PC34);
    if (!icon || !icon->loaded || !icon->pixels ||
        icon->width != DM1_V1_F0662_C028_WIDTH_PC34 ||
        icon->height != DM1_V1_F0662_C028_HEIGHT_PC34) goto fail;
    M11_Font_Init(&font);
    if (!M11_Font_LoadFromGraphicsDat(&font, loader.fileState, loader.runtimeState) ||
        !M11_Font_IsLoaded(&font)) goto fail;
    memset(&source, 0, sizeof(source));
    source.graphicsDatOwned = 1;
    source.graphicIndex = (int)icon->graphicIndex;
    source.width = (int)icon->width;
    source.height = (int)icon->height;
    source.indexedPixelCount = source.width * source.height;
    source.indexedPixels = icon->pixels;
    source.pixelsFNV1a = dm1_v1_f0662_invisibility_material_fnv1a_pc34(
        source.indexedPixels, source.indexedPixelCount);
    memset(&glyph, 0, sizeof(glyph));
    glyph.graphicsDatOwned = 1;
    glyph.graphicIndex = M11_Font_ResolvedGraphicIndex(&font);
    glyph.bits = font.bitmap;
    glyph.byteCount = M11_FONT_BITMAP_BYTES;
    glyph.bitsFNV1a = dm1_v1_f0662_invisibility_material_fnv1a_pc34(
        glyph.bits, glyph.byteCount);
    paletteChanges = dm1_v1_f0662_invisibility_palette_changes_pc34();
    if (!source.pixelsFNV1a || !glyph.bitsFNV1a ||
        !dm1_v1_f0662_invisibility_material_receipt_pc34(
            &source, &glyph, paletteChanges,
            DM1_V1_F0662_PALETTE_CHANGE_COUNT_PC34, &receipt) ||
        !receipt.valid || !receipt.suppressSyntheticFallback ||
        receipt.championIconGraphicIndex != DM1_V1_F0662_C028_CHAMPION_ICONS_PC34 ||
        receipt.m653GraphicIndex != glyph.graphicIndex) goto fail;
    memcpy(alteredPalette, paletteChanges, sizeof(alteredPalette));
    alteredPalette[0] = 1;
    if (dm1_v1_f0662_invisibility_material_receipt_pc34(
            &source, &glyph, alteredPalette, sizeof(alteredPalette), &receipt)) goto fail;
    ++glyph.bitsFNV1a;
    if (dm1_v1_f0662_invisibility_material_receipt_pc34(
            &source, &glyph, paletteChanges,
            DM1_V1_F0662_PALETTE_CHANGE_COUNT_PC34, &receipt)) goto fail;
    --glyph.bitsFNV1a;
    ++source.pixelsFNV1a;
    if (dm1_v1_f0662_invisibility_material_receipt_pc34(
            &source, &glyph, paletteChanges,
            DM1_V1_F0662_PALETTE_CHANGE_COUNT_PC34, &receipt)) goto fail;
    M11_AssetLoader_Shutdown(&loader);
    puts("ok: real PC34 F0662 invisibility icon material gate");
    return 0;
fail:
    M11_AssetLoader_Shutdown(&loader);
    return 1;
}
