#include "asset_loader_m11.h"
#include "dm1_v1_f0341_scroll_material_pc34_compat.h"
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
    DM1_V1_F0341SourceSurfacePc34 panel;
    DM1_V1_F0341GlyphSourcePc34 glyph;
    DM1_V1_F0341ScrollMaterialReceiptPc34 receipt;
    const M11_AssetSlot* slot;

    if (root && root[0]) snprintf(path, sizeof(path), "%s/GRAPHICS.DAT", root);
    else if (home && home[0]) snprintf(path, sizeof(path), "%s/.firestaff/data/dm1/GRAPHICS.DAT", home);
    else return 0;
    if (!M11_AssetLoader_Init(&loader, path)) {
        if (root && root[0]) return 1;
        puts("SKIP: PC34 GRAPHICS.DAT not installed");
        return 0;
    }
    memset(&panel, 0, sizeof(panel));
    memset(&glyph, 0, sizeof(glyph));
    slot = M11_AssetLoader_Load(&loader, 23u);
    if (!slot || !slot->loaded || !slot->pixels || slot->width != 144 || slot->height != 73) goto fail;
    panel.graphicsDatOwned = 1;
    panel.graphicIndex = 23;
    panel.width = slot->width;
    panel.height = slot->height;
    panel.indexedPixelCount = panel.width * panel.height;
    panel.indexedPixels = slot->pixels;
    panel.pixelsFNV1a = dm1_v1_f0341_scroll_material_fnv1a_pc34(slot->pixels, panel.indexedPixelCount);
    M11_Font_Init(&font);
    if (!M11_Font_LoadFromGraphicsDat(&font, loader.fileState, loader.runtimeState) || !M11_Font_IsLoaded(&font)) goto fail;
    glyph.graphicsDatOwned = 1;
    glyph.graphicIndex = M11_Font_ResolvedGraphicIndex(&font);
    glyph.bits = font.bitmap;
    glyph.byteCount = M11_FONT_BITMAP_BYTES;
    glyph.bitsFNV1a = dm1_v1_f0341_scroll_material_fnv1a_pc34(glyph.bits, glyph.byteCount);
    if (!dm1_v1_f0341_scroll_material_receipt_pc34(&panel, 1, &glyph, 1, &receipt) ||
        !receipt.valid || receipt.panelZoneIndex != 101 ||
        receipt.panelTransparentColor != 8 || receipt.textForeground != 0 ||
        receipt.m653GraphicIndex != glyph.graphicIndex) goto fail;
    ++panel.pixelsFNV1a;
    if (dm1_v1_f0341_scroll_material_receipt_pc34(&panel, 1, &glyph, 1, &receipt)) goto fail;
    M11_AssetLoader_Shutdown(&loader);
    puts("ok: real PC34 F0341 scroll material gate");
    return 0;
fail:
    M11_AssetLoader_Shutdown(&loader);
    return 1;
}
