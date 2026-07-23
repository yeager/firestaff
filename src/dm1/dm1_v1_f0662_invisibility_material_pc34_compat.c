#include "dm1_v1_f0662_invisibility_material_pc34_compat.h"

#include <string.h>

static const unsigned char kDm1V1F0662InvisibilityPaletteChangesPc34[
    DM1_V1_F0662_PALETTE_CHANGE_COUNT_PC34] = {
    0, 1, 2, 0, 4, 0, 6, 7, 8, 9, 0, 11, 12, 13, 14, 15
};

uint32_t dm1_v1_f0662_invisibility_material_fnv1a_pc34(
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

const unsigned char* dm1_v1_f0662_invisibility_palette_changes_pc34(void)
{
    return kDm1V1F0662InvisibilityPaletteChangesPc34;
}

int dm1_v1_f0662_invisibility_material_receipt_pc34(
    const DM1_V1_F0662SourceSurfacePc34* icon,
    const DM1_V1_F0662GlyphSourcePc34* glyph,
    const unsigned char* paletteChanges,
    int paletteChangeCount,
    DM1_V1_F0662InvisibilityMaterialReceiptPc34* outReceipt)
{
    uint32_t iconHash;
    uint32_t glyphHash;
    uint32_t paletteHash;
    uint32_t fingerprint = 2166136261u;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!icon || !glyph || !paletteChanges ||
        !icon->graphicsDatOwned || !icon->indexedPixels ||
        icon->graphicIndex != DM1_V1_F0662_C028_CHAMPION_ICONS_PC34 ||
        icon->width != DM1_V1_F0662_C028_WIDTH_PC34 ||
        icon->height != DM1_V1_F0662_C028_HEIGHT_PC34 ||
        icon->indexedPixelCount != DM1_V1_F0662_C028_WIDTH_PC34 *
                                  DM1_V1_F0662_C028_HEIGHT_PC34 ||
        !glyph->graphicsDatOwned || !glyph->bits ||
        glyph->byteCount != DM1_V1_F0662_M653_BYTES_PC34 ||
        (glyph->graphicIndex != DM1_V1_F0662_M653_PC34 &&
         glyph->graphicIndex != DM1_V1_F0662_M653_LEGACY_PC34) ||
        paletteChangeCount != DM1_V1_F0662_PALETTE_CHANGE_COUNT_PC34 ||
        memcmp(paletteChanges, kDm1V1F0662InvisibilityPaletteChangesPc34,
               DM1_V1_F0662_PALETTE_CHANGE_COUNT_PC34) != 0) {
        return 0;
    }
    iconHash = dm1_v1_f0662_invisibility_material_fnv1a_pc34(
        icon->indexedPixels, icon->indexedPixelCount);
    glyphHash = dm1_v1_f0662_invisibility_material_fnv1a_pc34(
        glyph->bits, glyph->byteCount);
    paletteHash = dm1_v1_f0662_invisibility_material_fnv1a_pc34(
        paletteChanges, paletteChangeCount);
    if (!iconHash || iconHash != icon->pixelsFNV1a ||
        !glyphHash || glyphHash != glyph->bitsFNV1a || !paletteHash) {
        return 0;
    }
    fingerprint ^= iconHash;
    fingerprint *= 16777619u;
    fingerprint ^= glyphHash;
    fingerprint *= 16777619u;
    fingerprint ^= paletteHash;
    fingerprint *= 16777619u;
    if (!fingerprint) return 0;

    outReceipt->valid = 1;
    outReceipt->suppressSyntheticFallback = 1;
    outReceipt->championIconGraphicIndex = icon->graphicIndex;
    outReceipt->sourceWidth = icon->width;
    outReceipt->sourceHeight = icon->height;
    outReceipt->m653GraphicIndex = glyph->graphicIndex;
    outReceipt->paletteChangeCount = paletteChangeCount;
    outReceipt->materialFingerprint = fingerprint;
    return 1;
}
