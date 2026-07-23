#include "dm1_v1_f0661_damage_material_pc34_compat.h"

#include <string.h>

uint32_t dm1_v1_f0661_damage_material_fnv1a_pc34(
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

int dm1_v1_f0661_damage_material_receipt_pc34(
    const DM1_V1_F0661SourceSurfacePc34* damageSurface,
    const DM1_V1_F0661GlyphSourcePc34* glyph,
    const unsigned char* paletteChanges,
    DM1_V1_F0661DamageMaterialReceiptPc34* outReceipt)
{
    uint32_t surfaceHash;
    uint32_t glyphHash;
    uint32_t fingerprint = 2166136261u;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!damageSurface || !glyph || paletteChanges ||
        !damageSurface->graphicsDatOwned || !damageSurface->indexedPixels ||
        damageSurface->graphicIndex != DM1_V1_F0661_C014_DAMAGE_TO_CREATURE_PC34 ||
        damageSurface->width != DM1_V1_F0661_C014_WIDTH_PC34 ||
        damageSurface->height != DM1_V1_F0661_C014_HEIGHT_PC34 ||
        damageSurface->indexedPixelCount != DM1_V1_F0661_C014_WIDTH_PC34 *
                                           DM1_V1_F0661_C014_HEIGHT_PC34 ||
        !glyph->graphicsDatOwned || !glyph->bits ||
        glyph->byteCount != DM1_V1_F0661_M653_BYTES_PC34 ||
        (glyph->graphicIndex != DM1_V1_F0661_M653_PC34 &&
         glyph->graphicIndex != DM1_V1_F0661_M653_LEGACY_PC34)) {
        return 0;
    }
    surfaceHash = dm1_v1_f0661_damage_material_fnv1a_pc34(
        damageSurface->indexedPixels, damageSurface->indexedPixelCount);
    glyphHash = dm1_v1_f0661_damage_material_fnv1a_pc34(
        glyph->bits, glyph->byteCount);
    if (!surfaceHash || surfaceHash != damageSurface->pixelsFNV1a ||
        !glyphHash || glyphHash != glyph->bitsFNV1a) {
        return 0;
    }
    fingerprint ^= surfaceHash;
    fingerprint *= 16777619u;
    fingerprint ^= glyphHash;
    fingerprint *= 16777619u;
    if (!fingerprint) return 0;

    outReceipt->valid = 1;
    outReceipt->suppressSyntheticFallback = 1;
    outReceipt->damageGraphicIndex = damageSurface->graphicIndex;
    outReceipt->sourceWidth = damageSurface->width;
    outReceipt->sourceHeight = damageSurface->height;
    outReceipt->mediumWidth = DM1_V1_F0661_MEDIUM_WIDTH_PC34;
    outReceipt->mediumHeight = DM1_V1_F0661_MEDIUM_HEIGHT_PC34;
    outReceipt->smallWidth = DM1_V1_F0661_SMALL_WIDTH_PC34;
    outReceipt->smallHeight = DM1_V1_F0661_SMALL_HEIGHT_PC34;
    outReceipt->m653GraphicIndex = glyph->graphicIndex;
    outReceipt->originalPaletteUnchanged = 1;
    outReceipt->materialFingerprint = fingerprint;
    return 1;
}
