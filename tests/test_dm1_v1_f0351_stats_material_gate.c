#include "asset_loader_m11.h"
#include "dm1_v1_f0351_stats_material_pc34_compat.h"
#include "font_m11.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(void) {
    char path[2048]; const char* home = getenv("HOME"); const char* root = getenv("FIRESTAFF_DM1_DATA_DIR");
    M11_AssetLoader loader; M11_FontState font; const M11_AssetSlot* slot;
    DM1_V1_F0351SourceSurfacePc34 panel; DM1_V1_F0351GlyphSourcePc34 glyph;
    DM1_V1_F0351StatsMaterialReceiptPc34 receipt;
    if (root && root[0]) snprintf(path, sizeof(path), "%s/GRAPHICS.DAT", root);
    else if (home && home[0]) snprintf(path, sizeof(path), "%s/.firestaff/data/dm1/GRAPHICS.DAT", home); else return 0;
    if (!M11_AssetLoader_Init(&loader, path)) return root && root[0] ? 1 : 0;
    memset(&panel, 0, sizeof(panel)); memset(&glyph, 0, sizeof(glyph));
    slot = M11_AssetLoader_Load(&loader, 20u);
    if (!slot || !slot->pixels || slot->width != 144 || slot->height != 73) goto fail;
    panel.graphicsDatOwned=1; panel.graphicIndex=20; panel.width=slot->width; panel.height=slot->height;
    panel.indexedPixelCount=panel.width*panel.height; panel.indexedPixels=slot->pixels;
    panel.pixelsFNV1a=dm1_v1_f0351_stats_material_fnv1a_pc34(slot->pixels,panel.indexedPixelCount);
    M11_Font_Init(&font); if (!M11_Font_LoadFromGraphicsDat(&font,loader.fileState,loader.runtimeState)||!M11_Font_IsLoaded(&font)) goto fail;
    glyph.graphicsDatOwned=1; glyph.graphicIndex=M11_Font_ResolvedGraphicIndex(&font); glyph.bits=font.bitmap; glyph.byteCount=M11_FONT_BITMAP_BYTES;
    glyph.bitsFNV1a=dm1_v1_f0351_stats_material_fnv1a_pc34(glyph.bits,glyph.byteCount);
    if (!dm1_v1_f0351_stats_material_receipt_pc34(&panel,1,&glyph,1,&receipt)||!receipt.valid||receipt.skillZoneIndex!=557||receipt.statisticZoneIndex!=559||receipt.panelTransparentColor!=8) goto fail;
    ++glyph.bitsFNV1a; if (dm1_v1_f0351_stats_material_receipt_pc34(&panel,1,&glyph,1,&receipt)) goto fail;
    M11_AssetLoader_Shutdown(&loader); puts("ok: real PC34 F0351 stats material gate"); return 0;
fail: M11_AssetLoader_Shutdown(&loader); return 1;
}
