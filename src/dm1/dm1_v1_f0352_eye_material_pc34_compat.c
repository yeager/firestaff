#include "dm1_v1_f0352_eye_material_pc34_compat.h"

#include <string.h>

uint32_t dm1_v1_f0352_eye_material_fnv1a_pc34(
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

int dm1_v1_f0352_eye_material_receipt_pc34(
    const DM1_V1_F0352SourceSurfacePc34* surfaces, int surfaceCount,
    const DM1_V1_F0352GlyphSourcePc34* glyphs, int glyphCount,
    DM1_V1_F0352EyeMaterialReceiptPc34* outReceipt)
{
    uint32_t arrowHash = 0u;
    uint32_t eyeHash = 0u;
    uint32_t glyphHash = 0u;
    uint32_t fingerprint = 2166136261u;
    int arrowOk = 0;
    int eyeOk = 0;
    int glyphIndex = -1;
    int i;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    for (i = 0; surfaces && i < surfaceCount; ++i) {
        const DM1_V1_F0352SourceSurfacePc34* source = &surfaces[i];
        uint32_t hash;
        if (!source->graphicsDatOwned || !source->indexedPixels ||
            source->width != 16 || source->height != 9 ||
            source->indexedPixelCount < 16 * 9 ||
            (source->graphicIndex != DM1_V1_F0352_C018_ARROW_PC34 &&
             source->graphicIndex != DM1_V1_F0352_C019_EYE_PC34)) {
            continue;
        }
        hash = dm1_v1_f0352_eye_material_fnv1a_pc34(
            source->indexedPixels, source->indexedPixelCount);
        if (!hash || hash != source->pixelsFNV1a) continue;
        if (source->graphicIndex == DM1_V1_F0352_C018_ARROW_PC34) {
            arrowHash = hash;
            arrowOk = 1;
        } else {
            eyeHash = hash;
            eyeOk = 1;
        }
    }
    for (i = 0; glyphs && i < glyphCount; ++i) {
        const DM1_V1_F0352GlyphSourcePc34* glyph = &glyphs[i];
        if (!glyph->graphicsDatOwned || !glyph->bits ||
            glyph->byteCount != DM1_V1_F0352_M653_BYTES_PC34 ||
            (glyph->graphicIndex != DM1_V1_F0352_M653_PC34 &&
             glyph->graphicIndex != DM1_V1_F0352_M653_LEGACY_PC34)) {
            continue;
        }
        glyphHash = dm1_v1_f0352_eye_material_fnv1a_pc34(
            glyph->bits, glyph->byteCount);
        if (glyphHash && glyphHash == glyph->bitsFNV1a) {
            glyphIndex = glyph->graphicIndex;
        }
    }
    if (!arrowOk || !eyeOk || glyphIndex < 0) return 0;
    fingerprint ^= arrowHash; fingerprint *= 16777619u;
    fingerprint ^= eyeHash; fingerprint *= 16777619u;
    fingerprint ^= glyphHash; fingerprint *= 16777619u;
    if (!fingerprint) return 0;

    outReceipt->valid = 1;
    outReceipt->suppressSyntheticFallback = 1;
    outReceipt->arrowGraphicIndex = DM1_V1_F0352_C018_ARROW_PC34;
    outReceipt->eyeGraphicIndex = DM1_V1_F0352_C019_EYE_PC34;
    outReceipt->sourceWidth = 16;
    outReceipt->sourceHeight = 9;
    outReceipt->transparentColor = 8;
    outReceipt->panelZoneIndex = DM1_V1_F0352_C503_ZONE_PC34;
    outReceipt->panelX = 83;
    outReceipt->panelY = 57;
    outReceipt->m653GraphicIndex = glyphIndex;
    outReceipt->materialFingerprint = fingerprint;
    return 1;
}
