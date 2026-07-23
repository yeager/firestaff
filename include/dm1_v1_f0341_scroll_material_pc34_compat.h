#ifndef FIRESTAFF_DM1_V1_F0341_SCROLL_MATERIAL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0341_SCROLL_MATERIAL_PC34_COMPAT_H

#include <stdint.h>

enum {
    DM1_V1_F0341_C023_PANEL_PC34 = 23,
    DM1_V1_F0341_M653_PC34 = 695,
    DM1_V1_F0341_M653_LEGACY_PC34 = 557,
    DM1_V1_F0341_M653_BYTES_PC34 = 768
};

typedef struct DM1_V1_F0341SourceSurfacePc34 {
    int graphicsDatOwned;
    int graphicIndex;
    int width;
    int height;
    int indexedPixelCount;
    const unsigned char* indexedPixels;
    uint32_t pixelsFNV1a;
} DM1_V1_F0341SourceSurfacePc34;

typedef struct DM1_V1_F0341GlyphSourcePc34 {
    int graphicsDatOwned;
    int graphicIndex;
    const unsigned char* bits;
    int byteCount;
    uint32_t bitsFNV1a;
} DM1_V1_F0341GlyphSourcePc34;

typedef struct DM1_V1_F0341ScrollMaterialReceiptPc34 {
    int valid;
    int suppressSyntheticFallback;
    int panelGraphicIndex;
    int panelSourceW;
    int panelSourceH;
    int panelTransparentColor;
    int panelZoneIndex;
    int textForeground;
    int textBackground;
    int m653GraphicIndex;
    uint32_t materialFingerprint;
} DM1_V1_F0341ScrollMaterialReceiptPc34;

uint32_t dm1_v1_f0341_scroll_material_fnv1a_pc34(
    const unsigned char* bytes, int byteCount);

/* ReDMCSB PANEL.C F0341: only raw C023 plus an authenticated M653 may
 * produce C101 scroll chrome or text. */
int dm1_v1_f0341_scroll_material_receipt_pc34(
    const DM1_V1_F0341SourceSurfacePc34* surfaces, int surfaceCount,
    const DM1_V1_F0341GlyphSourcePc34* glyphs, int glyphCount,
    DM1_V1_F0341ScrollMaterialReceiptPc34* outReceipt);

#endif
