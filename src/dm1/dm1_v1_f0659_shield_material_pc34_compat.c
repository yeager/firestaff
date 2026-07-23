#include "dm1_v1_f0659_shield_material_pc34_compat.h"

#include <string.h>

uint32_t dm1_v1_f0659_shield_material_fnv1a_pc34(
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

int dm1_v1_f0659_shield_material_receipt_pc34(
    const DM1_V1_F0659SourceSurfacePc34* surfaces, int surfaceCount,
    const DM1_V1_F0659GlyphSourcePc34* glyphs, int glyphCount,
    DM1_V1_F0659ShieldMaterialReceiptPc34* outReceipt)
{
    uint32_t hashes[3] = {0u, 0u, 0u};
    uint32_t glyphHash = 0u;
    uint32_t fingerprint = 2166136261u;
    int present[3] = {0, 0, 0};
    int glyphIndex = -1;
    int i;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    for (i = 0; surfaces && i < surfaceCount; ++i) {
        const DM1_V1_F0659SourceSurfacePc34* source = &surfaces[i];
        uint32_t hash;
        int index;
        if (!source->graphicsDatOwned || !source->indexedPixels ||
            source->graphicIndex < DM1_V1_F0659_C037_SHIELD_PC34 ||
            source->graphicIndex > DM1_V1_F0659_C039_SPELL_SHIELD_PC34 ||
            source->width != 67 || source->height != 29 ||
            source->indexedPixelCount < 67 * 29) continue;
        hash = dm1_v1_f0659_shield_material_fnv1a_pc34(
            source->indexedPixels, source->indexedPixelCount);
        if (!hash || hash != source->pixelsFNV1a) continue;
        index = source->graphicIndex - DM1_V1_F0659_C037_SHIELD_PC34;
        hashes[index] = hash;
        present[index] = 1;
    }
    for (i = 0; glyphs && i < glyphCount; ++i) {
        const DM1_V1_F0659GlyphSourcePc34* glyph = &glyphs[i];
        if (!glyph->graphicsDatOwned || !glyph->bits ||
            glyph->byteCount != DM1_V1_F0659_M653_BYTES_PC34 ||
            (glyph->graphicIndex != DM1_V1_F0659_M653_PC34 &&
             glyph->graphicIndex != DM1_V1_F0659_M653_LEGACY_PC34)) continue;
        glyphHash = dm1_v1_f0659_shield_material_fnv1a_pc34(
            glyph->bits, glyph->byteCount);
        if (glyphHash && glyphHash == glyph->bitsFNV1a) glyphIndex = glyph->graphicIndex;
    }
    if (!present[0] || !present[1] || !present[2] || glyphIndex < 0) return 0;
    for (i = 0; i < 3; ++i) {
        fingerprint ^= hashes[i];
        fingerprint *= 16777619u;
    }
    fingerprint ^= glyphHash;
    fingerprint *= 16777619u;
    if (!fingerprint) return 0;
    outReceipt->valid = 1;
    outReceipt->suppressSyntheticFallback = 1;
    outReceipt->shieldGraphicIndex = DM1_V1_F0659_C037_SHIELD_PC34;
    outReceipt->fireShieldGraphicIndex = DM1_V1_F0659_C038_FIRE_SHIELD_PC34;
    outReceipt->spellShieldGraphicIndex = DM1_V1_F0659_C039_SPELL_SHIELD_PC34;
    outReceipt->sourceWidth = 67;
    outReceipt->sourceHeight = 29;
    outReceipt->transparentColor = 10;
    outReceipt->m653GraphicIndex = glyphIndex;
    outReceipt->materialFingerprint = fingerprint;
    return 1;
}
