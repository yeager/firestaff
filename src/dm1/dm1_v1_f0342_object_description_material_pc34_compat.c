#include "dm1_v1_f0342_object_description_material_pc34_compat.h"

#include <string.h>

uint32_t dm1_v1_f0342_object_description_material_fnv1a_pc34(
    const unsigned char* bytes, int byteCount)
{
    uint32_t hash = 2166136261u;
    int i;
    if (!bytes || byteCount <= 0) return 0u;
    for (i = 0; i < byteCount; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

static const DM1_V1_F0342SourceSurfacePc34* find_surface(
    const DM1_V1_F0342SourceSurfacePc34* surfaces, int surfaceCount,
    int graphicIndex, int width, int height, uint32_t* outHash)
{
    int i;
    if (outHash) *outHash = 0u;
    if (!surfaces || surfaceCount <= 0) return 0;
    for (i = 0; i < surfaceCount; ++i) {
        const DM1_V1_F0342SourceSurfacePc34* surface = &surfaces[i];
        uint32_t hash;
        if (!surface->graphicsDatOwned || surface->graphicIndex != graphicIndex ||
            surface->width != width || surface->height != height ||
            !surface->indexedPixels ||
            surface->indexedPixelCount < width * height) continue;
        hash = dm1_v1_f0342_object_description_material_fnv1a_pc34(
            surface->indexedPixels, surface->indexedPixelCount);
        if (!hash || hash != surface->pixelsFNV1a) continue;
        if (outHash) *outHash = hash;
        return surface;
    }
    return 0;
}

static const DM1_V1_F0342GlyphSourcePc34* find_m653(
    const DM1_V1_F0342GlyphSourcePc34* glyphs, int glyphCount,
    uint32_t* outHash)
{
    int i;
    if (outHash) *outHash = 0u;
    if (!glyphs || glyphCount <= 0) return 0;
    for (i = 0; i < glyphCount; ++i) {
        const DM1_V1_F0342GlyphSourcePc34* glyph = &glyphs[i];
        uint32_t hash;
        if (!glyph->graphicsDatOwned ||
            (glyph->graphicIndex != DM1_V1_F0342_M653_PC34 &&
             glyph->graphicIndex != DM1_V1_F0342_M653_LEGACY_PC34) ||
            !glyph->bits || glyph->byteCount != DM1_V1_F0342_M653_BYTES_PC34) {
            continue;
        }
        hash = dm1_v1_f0342_object_description_material_fnv1a_pc34(
            glyph->bits, glyph->byteCount);
        if (!hash || hash != glyph->bitsFNV1a) continue;
        if (outHash) *outHash = hash;
        return glyph;
    }
    return 0;
}

static void add_operation(
    DM1_V1_F0342ObjectDescriptionMaterialReceiptPc34* receipt, int kind,
    int graphic, int zone, int x, int y, int w, int h, int fg, int bg,
    int transparent, int sourceLine)
{
    DM1_V1_F0342MaterialOperationPc34* operation =
        &receipt->operations[receipt->operationCount++];
    operation->kind = kind;
    operation->graphicIndex = graphic;
    operation->zoneIndex = zone;
    operation->sourceX = x;
    operation->sourceY = y;
    operation->sourceW = w;
    operation->sourceH = h;
    operation->paletteForeground = fg;
    operation->paletteBackground = bg;
    operation->transparentColor = transparent;
    operation->sourceLine = sourceLine;
}

int dm1_v1_f0342_object_description_material_receipt_pc34(
    const DM1_V1_F0342SourceSurfacePc34* surfaces,
    int surfaceCount,
    const DM1_V1_F0342GlyphSourcePc34* glyphs,
    int glyphCount,
    DM1_V1_F0342ObjectDescriptionMaterialReceiptPc34* outReceipt)
{
    DM1_V1_F0342ObjectDescriptionMaterialReceiptPc34 receipt;
    const DM1_V1_F0342GlyphSourcePc34* glyph;
    uint32_t hashes[3];
    uint32_t fingerprint = 2166136261u;
    int i;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!find_surface(surfaces, surfaceCount, DM1_V1_F0342_C020_PANEL_PC34,
                      144, 73, &hashes[0]) ||
        !find_surface(surfaces, surfaceCount, DM1_V1_F0342_C029_CIRCLE_PC34,
                      26, 26, &hashes[1]) ||
        !(glyph = find_m653(glyphs, glyphCount, &hashes[2]))) {
        return 0;
    }

    memset(&receipt, 0, sizeof(receipt));
    receipt.suppressSyntheticFallback = 1;
    receipt.m653GraphicIndex = glyph->graphicIndex;
    receipt.m653BitsFNV1a = hashes[2];
    /* PANEL.C F0342:1140-1145 resolves C101/C504, then TEXT.C F0648
     * writes C506 and continuation C556 through M653. */
    add_operation(&receipt, DM1_V1_F0342_PANEL_BACKGROUND_PC34,
                  20, 101, 0, 0, 144, 73, 4, 0, -1, 1140);
    add_operation(&receipt, DM1_V1_F0342_OBJECT_CIRCLE_PC34,
                  29, 504, 0, 0, 26, 26, 13, -1, 12, 1141);
    add_operation(&receipt, DM1_V1_F0342_OBJECT_NAME_PC34,
                  glyph->graphicIndex, 506, 0, 0, 0, 0, 13, -1, -1, 1198);
    add_operation(&receipt, DM1_V1_F0342_OBJECT_BODY_PC34,
                  glyph->graphicIndex, 556, 0, 0, 0, 0, 13, -1, -1, 1200);
    for (i = 0; i < 3; ++i) {
        fingerprint ^= hashes[i];
        fingerprint *= 16777619u;
    }
    if (!fingerprint || receipt.operationCount != 4) return 0;
    receipt.materialFingerprint = fingerprint;
    receipt.valid = 1;
    *outReceipt = receipt;
    return 1;
}
