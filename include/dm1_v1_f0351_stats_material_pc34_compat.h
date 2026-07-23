#ifndef FIRESTAFF_DM1_V1_F0351_STATS_MATERIAL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0351_STATS_MATERIAL_PC34_COMPAT_H

#include <stdint.h>

typedef struct DM1_V1_F0351SourceSurfacePc34 {
    int graphicsDatOwned, graphicIndex, width, height, indexedPixelCount;
    const unsigned char* indexedPixels;
    uint32_t pixelsFNV1a;
} DM1_V1_F0351SourceSurfacePc34;

typedef struct DM1_V1_F0351GlyphSourcePc34 {
    int graphicsDatOwned, graphicIndex, byteCount;
    const unsigned char* bits;
    uint32_t bitsFNV1a;
} DM1_V1_F0351GlyphSourcePc34;

typedef struct DM1_V1_F0351StatsMaterialReceiptPc34 {
    int valid, suppressSyntheticFallback;
    int panelGraphicIndex, panelSourceW, panelSourceH, panelTransparentColor;
    int panelZoneIndex, skillZoneIndex, statisticZoneIndex;
    int m653GraphicIndex;
    uint32_t materialFingerprint;
} DM1_V1_F0351StatsMaterialReceiptPc34;

uint32_t dm1_v1_f0351_stats_material_fnv1a_pc34(const unsigned char* bytes,
                                                 int byteCount);
int dm1_v1_f0351_stats_material_receipt_pc34(
    const DM1_V1_F0351SourceSurfacePc34* surfaces, int surfaceCount,
    const DM1_V1_F0351GlyphSourcePc34* glyphs, int glyphCount,
    DM1_V1_F0351StatsMaterialReceiptPc34* outReceipt);

#endif
