#ifndef FIRESTAFF_DM1_V1_F0352_EYE_MATERIAL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0352_EYE_MATERIAL_PC34_COMPAT_H

#include <stdint.h>

enum {
    DM1_V1_F0352_C018_ARROW_PC34 = 18,
    DM1_V1_F0352_C019_EYE_PC34 = 19,
    DM1_V1_F0352_M653_PC34 = 695,
    DM1_V1_F0352_M653_LEGACY_PC34 = 557,
    DM1_V1_F0352_M653_BYTES_PC34 = 768,
    DM1_V1_F0352_C503_ZONE_PC34 = 503
};

typedef struct DM1_V1_F0352SourceSurfacePc34 {
    int graphicsDatOwned;
    int graphicIndex;
    int width;
    int height;
    int indexedPixelCount;
    const unsigned char* indexedPixels;
    uint32_t pixelsFNV1a;
} DM1_V1_F0352SourceSurfacePc34;

typedef struct DM1_V1_F0352GlyphSourcePc34 {
    int graphicsDatOwned;
    int graphicIndex;
    const unsigned char* bits;
    int byteCount;
    uint32_t bitsFNV1a;
} DM1_V1_F0352GlyphSourcePc34;

typedef struct DM1_V1_F0352EyeMaterialReceiptPc34 {
    int valid;
    int suppressSyntheticFallback;
    int arrowGraphicIndex;
    int eyeGraphicIndex;
    int sourceWidth;
    int sourceHeight;
    int transparentColor;
    int panelZoneIndex;
    int panelX;
    int panelY;
    int m653GraphicIndex;
    uint32_t materialFingerprint;
} DM1_V1_F0352EyeMaterialReceiptPc34;

uint32_t dm1_v1_f0352_eye_material_fnv1a_pc34(
    const unsigned char* bytes, int byteCount);

/* PANEL.C F0339/F0352/F0353: C503's arrow/eye state is publishable only
 * when both raw C018/C019 surfaces and the session M653 are authenticated. */
int dm1_v1_f0352_eye_material_receipt_pc34(
    const DM1_V1_F0352SourceSurfacePc34* surfaces, int surfaceCount,
    const DM1_V1_F0352GlyphSourcePc34* glyphs, int glyphCount,
    DM1_V1_F0352EyeMaterialReceiptPc34* outReceipt);

#endif
