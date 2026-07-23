#include "dm1_v1_f0341_scroll_material_pc34_compat.h"

#include <string.h>

uint32_t dm1_v1_f0341_scroll_material_fnv1a_pc34(
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

int dm1_v1_f0341_scroll_material_receipt_pc34(
    const DM1_V1_F0341SourceSurfacePc34* surfaces, int surfaceCount,
    const DM1_V1_F0341GlyphSourcePc34* glyphs, int glyphCount,
    DM1_V1_F0341ScrollMaterialReceiptPc34* outReceipt)
{
    const DM1_V1_F0341SourceSurfacePc34* panel = NULL;
    const DM1_V1_F0341GlyphSourcePc34* glyph = NULL;
    uint32_t panelHash = 0u;
    uint32_t glyphHash = 0u;
    uint32_t fingerprint = 2166136261u;
    int i;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    for (i = 0; surfaces && i < surfaceCount; ++i) {
        const DM1_V1_F0341SourceSurfacePc34* candidate = &surfaces[i];
        if (!candidate->graphicsDatOwned ||
            candidate->graphicIndex != DM1_V1_F0341_C023_PANEL_PC34 ||
            candidate->width != 144 || candidate->height != 73 ||
            !candidate->indexedPixels || candidate->indexedPixelCount < 144 * 73) {
            continue;
        }
        panelHash = dm1_v1_f0341_scroll_material_fnv1a_pc34(
            candidate->indexedPixels, candidate->indexedPixelCount);
        if (panelHash && panelHash == candidate->pixelsFNV1a) panel = candidate;
    }
    for (i = 0; glyphs && i < glyphCount; ++i) {
        const DM1_V1_F0341GlyphSourcePc34* candidate = &glyphs[i];
        if (!candidate->graphicsDatOwned ||
            (candidate->graphicIndex != DM1_V1_F0341_M653_PC34 &&
             candidate->graphicIndex != DM1_V1_F0341_M653_LEGACY_PC34) ||
            !candidate->bits ||
            candidate->byteCount != DM1_V1_F0341_M653_BYTES_PC34) {
            continue;
        }
        glyphHash = dm1_v1_f0341_scroll_material_fnv1a_pc34(
            candidate->bits, candidate->byteCount);
        if (glyphHash && glyphHash == candidate->bitsFNV1a) glyph = candidate;
    }
    if (!panel || !glyph) return 0;
    fingerprint ^= panelHash;
    fingerprint *= 16777619u;
    fingerprint ^= glyphHash;
    fingerprint *= 16777619u;
    if (!fingerprint) return 0;
    outReceipt->valid = 1;
    outReceipt->suppressSyntheticFallback = 1;
    outReceipt->panelGraphicIndex = panel->graphicIndex;
    outReceipt->panelSourceW = 144;
    outReceipt->panelSourceH = 73;
    outReceipt->panelTransparentColor = 8; /* C08 red */
    outReceipt->panelZoneIndex = 101;
    outReceipt->textForeground = 0; /* F0340 scroll ink is C00 */
    outReceipt->textBackground = -1;
    outReceipt->m653GraphicIndex = glyph->graphicIndex;
    outReceipt->materialFingerprint = fingerprint;
    return 1;
}
