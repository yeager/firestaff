/* Focused real-PC34 gate for ReDMCSB PANEL.C F0342 material ownership. */

#include "asset_loader_m11.h"
#include "dm1_v1_f0342_object_description_material_pc34_compat.h"
#include "font_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
                        DM1_V1_F0342SourceSurfacePc34* out)
{
    const M11_AssetSlot* slot = M11_AssetLoader_Load(loader, (unsigned int)graphic);
    if (!slot || !slot->loaded || !slot->pixels || !slot->width || !slot->height) {
        return 0;
    }
    out->graphicsDatOwned = 1;
    out->graphicIndex = graphic;
    out->width = slot->width;
    out->height = slot->height;
    out->indexedPixelCount = (int)slot->width * (int)slot->height;
    out->indexedPixels = slot->pixels;
    out->pixelsFNV1a = dm1_v1_f0342_object_description_material_fnv1a_pc34(
        slot->pixels, out->indexedPixelCount);
    return out->pixelsFNV1a != 0u;
}

int main(void)
{
    char path[1024];
    M11_AssetLoader loader;
    M11_FontState font;
    DM1_V1_F0342SourceSurfacePc34 surfaces[2];
    DM1_V1_F0342GlyphSourcePc34 glyph;
    DM1_V1_F0342ObjectDescriptionMaterialReceiptPc34 receipt;

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
    if (!load_surface(&loader, 20, &surfaces[0]) ||
        !load_surface(&loader, 29, &surfaces[1])) goto fail;
    if (surfaces[0].width != 144 || surfaces[0].height != 73 ||
        surfaces[1].width != 26 || surfaces[1].height != 26) {
        fprintf(stderr, "unexpected C020/C029 dimensions: %dx%d / %dx%d\n",
                surfaces[0].width, surfaces[0].height,
                surfaces[1].width, surfaces[1].height);
        goto fail;
    }
    M11_Font_Init(&font);
    if (!M11_Font_LoadFromGraphicsDat(&font, loader.fileState, loader.runtimeState) ||
        !M11_Font_IsLoaded(&font)) goto fail;
    memset(&glyph, 0, sizeof(glyph));
    glyph.graphicsDatOwned = 1;
    glyph.graphicIndex = M11_Font_ResolvedGraphicIndex(&font);
    glyph.bits = font.bitmap;
    glyph.byteCount = M11_FONT_BITMAP_BYTES;
    glyph.bitsFNV1a = dm1_v1_f0342_object_description_material_fnv1a_pc34(
        glyph.bits, glyph.byteCount);
    if (!dm1_v1_f0342_object_description_material_receipt_pc34(
            surfaces, 2, &glyph, 1, &receipt) || !receipt.valid ||
        !receipt.suppressSyntheticFallback || receipt.operationCount != 4 ||
        receipt.operations[0].graphicIndex != 20 ||
        receipt.operations[0].zoneIndex != 101 ||
        receipt.operations[1].graphicIndex != 29 ||
        receipt.operations[1].zoneIndex != 504 ||
        receipt.operations[1].transparentColor != 12 ||
        receipt.operations[2].zoneIndex != 506 ||
        receipt.operations[3].zoneIndex != 556 ||
        receipt.m653BitsFNV1a == 0u) {
        fprintf(stderr, "F0342 receipt rejected (font graphic %d)\n",
                glyph.graphicIndex);
        goto fail;
    }
    ++surfaces[1].pixelsFNV1a;
    if (dm1_v1_f0342_object_description_material_receipt_pc34(
            surfaces, 2, &glyph, 1, &receipt)) {
        fputs("tampered C029 surface was admitted\n", stderr);
        goto fail;
    }
    M11_AssetLoader_Shutdown(&loader);
    puts("ok: real PC34 F0342 object-description material gate");
    return 0;
fail:
    M11_AssetLoader_Shutdown(&loader);
    return 1;
}
