#include "dm1_v1_f0351_stats_material_pc34_compat.h"

#include <string.h>

uint32_t dm1_v1_f0351_stats_material_fnv1a_pc34(const unsigned char* bytes,
                                                 int byteCount)
{
    uint32_t hash = 2166136261u;
    int i;
    if (!bytes || byteCount <= 0) return 0u;
    for (i = 0; i < byteCount; ++i) { hash ^= bytes[i]; hash *= 16777619u; }
    return hash;
}

int dm1_v1_f0351_stats_material_receipt_pc34(
    const DM1_V1_F0351SourceSurfacePc34* surfaces, int surfaceCount,
    const DM1_V1_F0351GlyphSourcePc34* glyphs, int glyphCount,
    DM1_V1_F0351StatsMaterialReceiptPc34* outReceipt)
{
    uint32_t panelHash = 0u, glyphHash = 0u, fingerprint = 2166136261u;
    int panelOk = 0, glyphIndex = -1, i;
    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    for (i = 0; surfaces && i < surfaceCount; ++i) {
        const DM1_V1_F0351SourceSurfacePc34* s = &surfaces[i];
        if (!s->graphicsDatOwned || s->graphicIndex != 20 || s->width != 144 ||
            s->height != 73 || !s->indexedPixels || s->indexedPixelCount < 10512) continue;
        panelHash = dm1_v1_f0351_stats_material_fnv1a_pc34(s->indexedPixels, s->indexedPixelCount);
        panelOk = panelHash && panelHash == s->pixelsFNV1a;
    }
    for (i = 0; glyphs && i < glyphCount; ++i) {
        const DM1_V1_F0351GlyphSourcePc34* g = &glyphs[i];
        if (!g->graphicsDatOwned || (g->graphicIndex != 695 && g->graphicIndex != 557) ||
            !g->bits || g->byteCount != 768) continue;
        glyphHash = dm1_v1_f0351_stats_material_fnv1a_pc34(g->bits, g->byteCount);
        if (glyphHash && glyphHash == g->bitsFNV1a) glyphIndex = g->graphicIndex;
    }
    if (!panelOk || glyphIndex < 0) return 0;
    fingerprint ^= panelHash; fingerprint *= 16777619u;
    fingerprint ^= glyphHash; fingerprint *= 16777619u;
    if (!fingerprint) return 0;
    outReceipt->valid = 1;
    outReceipt->suppressSyntheticFallback = 1;
    outReceipt->panelGraphicIndex = 20;
    outReceipt->panelSourceW = 144;
    outReceipt->panelSourceH = 73;
    outReceipt->panelTransparentColor = 8;
    outReceipt->panelZoneIndex = 101;
    outReceipt->skillZoneIndex = 557;
    outReceipt->statisticZoneIndex = 559;
    outReceipt->m653GraphicIndex = glyphIndex;
    outReceipt->materialFingerprint = fingerprint;
    return 1;
}
