/* Focused real-PC34 asset test for the source-only F0344/F0658 HUD receipt. */

#include "asset_loader_m11.h"
#include "dm1_v1_f0344_f0658_hud_material_pc34_compat.h"
#include "font_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { kSurfaceCount = 7 };

static const char* graphics_path(char path[1024])
{
    const char* root = getenv("FIRESTAFF_DM1_DATA_DIR");
    const char* home;
    if (root && root[0]) {
        snprintf(path, 1024, "%s/GRAPHICS.DAT", root);
        return path;
    }
    home = getenv("HOME");
    if (!home || !home[0]) return 0;
    snprintf(path, 1024, "%s/.firestaff/data/dm1/GRAPHICS.DAT", home);
    return path;
}

static int load_surface(M11_AssetLoader* loader, int graphic,
                        DM1_V1_HudSourceSurfacePc34* out)
{
    const M11_AssetSlot* slot = M11_AssetLoader_Load(loader, (unsigned int)graphic);
    if (!slot || !slot->loaded || !slot->pixels || !slot->width || !slot->height) return 0;
    out->graphicsDatOwned = 1;
    out->graphicIndex = graphic;
    out->width = slot->width;
    out->height = slot->height;
    out->indexedPixelCount = (int)slot->width * (int)slot->height;
    out->indexedPixels = slot->pixels;
    out->pixelsFNV1a = dm1_v1_f0344_f0658_hud_material_fnv1a_pc34(
        slot->pixels, out->indexedPixelCount);
    return out->pixelsFNV1a != 0u;
}

int main(void)
{
    char path[1024];
    M11_AssetLoader loader;
    M11_FontState font;
    DM1_V1_HudSourceSurfacePc34 surfaces[kSurfaceCount];
    DM1_V1_HudGlyphSourcePc34 glyph;
    DM1_V1_F0344F0658HudMaterialReceiptPc34 receipt;
    int graphics[kSurfaceCount] = { 10, 9, 11, 20, 30, 31, 32 };
    int i;

    if (!graphics_path(path)) return 0;
    if (!M11_AssetLoader_Init(&loader, path)) {
        if (getenv("FIRESTAFF_DM1_DATA_DIR")) {
            fputs("configured PC34 GRAPHICS.DAT is unavailable\n", stderr);
            return 1;
        }
        puts("SKIP: PC34 GRAPHICS.DAT not installed");
        return 0;
    }
    memset(surfaces, 0, sizeof(surfaces));
    for (i = 0; i < kSurfaceCount; ++i) {
        if (!load_surface(&loader, graphics[i], &surfaces[i])) goto fail;
    }
    M11_Font_Init(&font);
    if (!M11_Font_LoadFromGraphicsDat(&font, loader.fileState, loader.runtimeState) ||
        !M11_Font_IsLoaded(&font)) goto fail;
    memset(&glyph, 0, sizeof(glyph));
    glyph.graphicsDatOwned = 1;
    glyph.graphicIndex = M11_Font_ResolvedGraphicIndex(&font);
    glyph.bits = font.bitmap;
    glyph.byteCount = M11_FONT_BITMAP_BYTES;
    glyph.bitsFNV1a = dm1_v1_f0344_f0658_hud_material_fnv1a_pc34(
        glyph.bits, glyph.byteCount);
    if (!dm1_v1_f0344_f0658_hud_material_receipt_pc34(
            surfaces, kSurfaceCount, &glyph, 1, &receipt) ||
        !receipt.valid || !receipt.suppressSyntheticFallback ||
        receipt.operationCount != 10 || receipt.m653BitsFNV1a == 0u ||
        receipt.operations[0].zoneIndex != 79 ||
        receipt.operations[1].zoneIndex != 77 ||
        receipt.operations[2].zoneIndex != 11 ||
        receipt.operations[4].sourceY != 13 ||
        receipt.operations[5].sourceY != 26 ||
        receipt.operations[7].transparentColor != 12 ||
        receipt.operations[9].sourceLine != 1606) goto fail;
    ++surfaces[0].pixelsFNV1a;
    if (dm1_v1_f0344_f0658_hud_material_receipt_pc34(
            surfaces, kSurfaceCount, &glyph, 1, &receipt)) {
        fputs("tampered C010 surface was admitted\n", stderr);
        goto fail;
    }
    M11_AssetLoader_Shutdown(&loader);
    puts("ok: real PC34 F0344/F0658 HUD material gate");
    return 0;
fail:
    M11_AssetLoader_Shutdown(&loader);
    return 1;
}
