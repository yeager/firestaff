#ifndef FIRESTAFF_DM1_V1_F0661_DAMAGE_MATERIAL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0661_DAMAGE_MATERIAL_PC34_COMPAT_H

#include <stdint.h>

enum {
    DM1_V1_F0661_C014_DAMAGE_TO_CREATURE_PC34 = 14,
    DM1_V1_F0661_C014_WIDTH_PC34 = 88,
    DM1_V1_F0661_C014_HEIGHT_PC34 = 45,
    DM1_V1_F0661_MEDIUM_WIDTH_PC34 = 64,
    DM1_V1_F0661_MEDIUM_HEIGHT_PC34 = 37,
    DM1_V1_F0661_SMALL_WIDTH_PC34 = 42,
    DM1_V1_F0661_SMALL_HEIGHT_PC34 = 37,
    DM1_V1_F0661_M653_PC34 = 695,
    DM1_V1_F0661_M653_LEGACY_PC34 = 557,
    DM1_V1_F0661_M653_BYTES_PC34 = 768
};

typedef struct DM1_V1_F0661SourceSurfacePc34 {
    int graphicsDatOwned;
    int graphicIndex;
    int width;
    int height;
    int indexedPixelCount;
    const unsigned char* indexedPixels;
    uint32_t pixelsFNV1a;
} DM1_V1_F0661SourceSurfacePc34;

typedef struct DM1_V1_F0661GlyphSourcePc34 {
    int graphicsDatOwned;
    int graphicIndex;
    const unsigned char* bits;
    int byteCount;
    uint32_t bitsFNV1a;
} DM1_V1_F0661GlyphSourcePc34;

typedef struct DM1_V1_F0661DamageMaterialReceiptPc34 {
    int valid;
    int suppressSyntheticFallback;
    int damageGraphicIndex;
    int sourceWidth;
    int sourceHeight;
    int mediumWidth;
    int mediumHeight;
    int smallWidth;
    int smallHeight;
    int m653GraphicIndex;
    int originalPaletteUnchanged;
    uint32_t materialFingerprint;
} DM1_V1_F0661DamageMaterialReceiptPc34;

uint32_t dm1_v1_f0661_damage_material_fnv1a_pc34(
    const unsigned char* bytes, int byteCount);

/* ReDMCSB BASE.C F0661 and ACTIDRAW.C use C014 with a NULL palette-change
 * pointer: indexed GRAPHICS.DAT pixels keep their original palette. */
int dm1_v1_f0661_damage_material_receipt_pc34(
    const DM1_V1_F0661SourceSurfacePc34* damageSurface,
    const DM1_V1_F0661GlyphSourcePc34* glyph,
    const unsigned char* paletteChanges,
    DM1_V1_F0661DamageMaterialReceiptPc34* outReceipt);

#endif
