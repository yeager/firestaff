#ifndef FIRESTAFF_DM1_V1_F0659_SHIELD_MATERIAL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0659_SHIELD_MATERIAL_PC34_COMPAT_H

#include <stdint.h>

enum {
    DM1_V1_F0659_C037_SHIELD_PC34 = 37,
    DM1_V1_F0659_C038_FIRE_SHIELD_PC34 = 38,
    DM1_V1_F0659_C039_SPELL_SHIELD_PC34 = 39,
    DM1_V1_F0659_M653_PC34 = 695,
    DM1_V1_F0659_M653_LEGACY_PC34 = 557,
    DM1_V1_F0659_M653_BYTES_PC34 = 768
};

typedef struct DM1_V1_F0659SourceSurfacePc34 {
    int graphicsDatOwned;
    int graphicIndex;
    int width;
    int height;
    int indexedPixelCount;
    const unsigned char* indexedPixels;
    uint32_t pixelsFNV1a;
} DM1_V1_F0659SourceSurfacePc34;

typedef struct DM1_V1_F0659GlyphSourcePc34 {
    int graphicsDatOwned;
    int graphicIndex;
    const unsigned char* bits;
    int byteCount;
    uint32_t bitsFNV1a;
} DM1_V1_F0659GlyphSourcePc34;

typedef struct DM1_V1_F0659ShieldMaterialReceiptPc34 {
    int valid;
    int suppressSyntheticFallback;
    int shieldGraphicIndex;
    int fireShieldGraphicIndex;
    int spellShieldGraphicIndex;
    int sourceWidth;
    int sourceHeight;
    int transparentColor;
    int m653GraphicIndex;
    uint32_t materialFingerprint;
} DM1_V1_F0659ShieldMaterialReceiptPc34;

uint32_t dm1_v1_f0659_shield_material_fnv1a_pc34(
    const unsigned char* bytes, int byteCount);

/* CHAMDRAW.C F0292 -> BASE.C F0659: ordered C037/C038/C039 status-border
 * blits are allowed only with the original GRAPHICS.DAT trio and M653. */
int dm1_v1_f0659_shield_material_receipt_pc34(
    const DM1_V1_F0659SourceSurfacePc34* surfaces, int surfaceCount,
    const DM1_V1_F0659GlyphSourcePc34* glyphs, int glyphCount,
    DM1_V1_F0659ShieldMaterialReceiptPc34* outReceipt);

#endif
