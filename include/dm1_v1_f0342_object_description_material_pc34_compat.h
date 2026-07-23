#ifndef FIRESTAFF_DM1_V1_F0342_OBJECT_DESCRIPTION_MATERIAL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0342_OBJECT_DESCRIPTION_MATERIAL_PC34_COMPAT_H

/* ReDMCSB PANEL.C F0342 object-description transaction.  The receipt keeps
 * C020, C029 and M653 tied to their original PC34 pixels and layout zones. */

#include <stdint.h>

enum {
    DM1_V1_F0342_M653_BYTES_PC34 = 768,
    DM1_V1_F0342_C020_PANEL_PC34 = 20,
    DM1_V1_F0342_C029_CIRCLE_PC34 = 29,
    DM1_V1_F0342_M653_PC34 = 695,
    DM1_V1_F0342_M653_LEGACY_PC34 = 557
};

typedef struct DM1_V1_F0342SourceSurfacePc34 {
    int graphicsDatOwned;
    int graphicIndex;
    int width;
    int height;
    int indexedPixelCount;
    const unsigned char* indexedPixels;
    uint32_t pixelsFNV1a;
} DM1_V1_F0342SourceSurfacePc34;

typedef struct DM1_V1_F0342GlyphSourcePc34 {
    int graphicsDatOwned;
    int graphicIndex;
    const unsigned char* bits;
    int byteCount;
    uint32_t bitsFNV1a;
} DM1_V1_F0342GlyphSourcePc34;

typedef enum DM1_V1_F0342MaterialOperationKindPc34 {
    DM1_V1_F0342_PANEL_BACKGROUND_PC34 = 1,
    DM1_V1_F0342_OBJECT_CIRCLE_PC34,
    DM1_V1_F0342_OBJECT_NAME_PC34,
    DM1_V1_F0342_OBJECT_BODY_PC34
} DM1_V1_F0342MaterialOperationKindPc34;

typedef struct DM1_V1_F0342MaterialOperationPc34 {
    int kind;
    int graphicIndex;
    int zoneIndex;
    int sourceX;
    int sourceY;
    int sourceW;
    int sourceH;
    int paletteForeground;
    int paletteBackground;
    int transparentColor;
    int sourceLine;
} DM1_V1_F0342MaterialOperationPc34;

typedef struct DM1_V1_F0342ObjectDescriptionMaterialReceiptPc34 {
    int valid;
    int suppressSyntheticFallback;
    int m653GraphicIndex;
    uint32_t m653BitsFNV1a;
    int operationCount;
    DM1_V1_F0342MaterialOperationPc34 operations[4];
    uint32_t materialFingerprint;
} DM1_V1_F0342ObjectDescriptionMaterialReceiptPc34;

uint32_t dm1_v1_f0342_object_description_material_fnv1a_pc34(
    const unsigned char* bytes, int byteCount);

/* Requires the complete C020/C029/M653 source set.  Missing, altered, or
 * foreign pixels provide no partial receipt and cannot enable host drawing. */
int dm1_v1_f0342_object_description_material_receipt_pc34(
    const DM1_V1_F0342SourceSurfacePc34* surfaces,
    int surfaceCount,
    const DM1_V1_F0342GlyphSourcePc34* glyphs,
    int glyphCount,
    DM1_V1_F0342ObjectDescriptionMaterialReceiptPc34* outReceipt);

#endif /* FIRESTAFF_DM1_V1_F0342_OBJECT_DESCRIPTION_MATERIAL_PC34_COMPAT_H */
