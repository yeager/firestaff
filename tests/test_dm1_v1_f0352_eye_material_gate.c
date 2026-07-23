#include "asset_loader_m11.h"
#include "dm1_v1_f0352_eye_material_pc34_compat.h"
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
    DM1_V1_F0352SourceSurfacePc34 surfaces[2];
    DM1_V1_F0352GlyphSourcePc34 glyph;
    DM1_V1_F0352EyeMaterialReceiptPc34 receipt;
    const M11_AssetSlot* arrow;
    const M11_AssetSlot* eye;

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
    arrow = M11_AssetLoader_Load(&loader, 18u);
    eye = M11_AssetLoader_Load(&loader, 19u);
    if (!arrow || !eye || !arrow->loaded || !eye->loaded ||
        !arrow->pixels || !eye->pixels || arrow->width != 16 ||
        arrow->height != 9 || eye->width != 16 || eye->height != 9) goto fail;
    surfaces[0].graphicsDatOwned = 1;
    surfaces[0].graphicIndex = 18;
    surfaces[0].width = arrow->width;
    surfaces[0].height = arrow->height;
    surfaces[0].indexedPixelCount = 16 * 9;
    surfaces[0].indexedPixels = arrow->pixels;
    surfaces[0].pixelsFNV1a = dm1_v1_f0352_eye_material_fnv1a_pc34(arrow->pixels, 16 * 9);
    surfaces[1].graphicsDatOwned = 1;
    surfaces[1].graphicIndex = 19;
    surfaces[1].width = eye->width;
    surfaces[1].height = eye->height;
    surfaces[1].indexedPixelCount = 16 * 9;
    surfaces[1].indexedPixels = eye->pixels;
    surfaces[1].pixelsFNV1a = dm1_v1_f0352_eye_material_fnv1a_pc34(eye->pixels, 16 * 9);
    M11_Font_Init(&font);
    if (!M11_Font_LoadFromGraphicsDat(&font, loader.fileState, loader.runtimeState) ||
        !M11_Font_IsLoaded(&font)) goto fail;
    glyph.graphicsDatOwned = 1;
    glyph.graphicIndex = M11_Font_ResolvedGraphicIndex(&font);
    glyph.bits = font.bitmap;
    glyph.byteCount = M11_FONT_BITMAP_BYTES;
    glyph.bitsFNV1a = dm1_v1_f0352_eye_material_fnv1a_pc34(glyph.bits, glyph.byteCount);
    if (!dm1_v1_f0352_eye_material_receipt_pc34(surfaces, 2, &glyph, 1, &receipt) ||
        !receipt.valid || receipt.panelZoneIndex != 503 || receipt.panelX != 83 ||
        receipt.panelY != 57 || receipt.transparentColor != 8 ||
        receipt.m653GraphicIndex != glyph.graphicIndex) goto fail;
    ++surfaces[1].pixelsFNV1a;
    if (dm1_v1_f0352_eye_material_receipt_pc34(surfaces, 2, &glyph, 1, &receipt)) goto fail;
    M11_AssetLoader_Shutdown(&loader);
    puts("ok: real PC34 F0352 eye material gate");
    return 0;
fail:
    M11_AssetLoader_Shutdown(&loader);
    return 1;
}
