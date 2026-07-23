#ifndef FIRESTAFF_DM1_V1_F0662_INVISIBILITY_MATERIAL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0662_INVISIBILITY_MATERIAL_PC34_COMPAT_H

#include <stdint.h>

enum {
    DM1_V1_F0662_C028_CHAMPION_ICONS_PC34 = 28,
    DM1_V1_F0662_C028_WIDTH_PC34 = 76,
    DM1_V1_F0662_C028_HEIGHT_PC34 = 14,
    DM1_V1_F0662_M653_PC34 = 695,
    DM1_V1_F0662_M653_LEGACY_PC34 = 557,
    DM1_V1_F0662_M653_BYTES_PC34 = 768,
    DM1_V1_F0662_PALETTE_CHANGE_COUNT_PC34 = 16
};

typedef struct DM1_V1_F0662SourceSurfacePc34 {
    int graphicsDatOwned;
    int graphicIndex;
    int width;
    int height;
    int indexedPixelCount;
    const unsigned char* indexedPixels;
    uint32_t pixelsFNV1a;
} DM1_V1_F0662SourceSurfacePc34;

typedef struct DM1_V1_F0662GlyphSourcePc34 {
    int graphicsDatOwned;
    int graphicIndex;
    const unsigned char* bits;
    int byteCount;
    uint32_t bitsFNV1a;
} DM1_V1_F0662GlyphSourcePc34;

typedef struct DM1_V1_F0662InvisibilityMaterialReceiptPc34 {
    int valid;
    int suppressSyntheticFallback;
    int championIconGraphicIndex;
    int sourceWidth;
    int sourceHeight;
    int m653GraphicIndex;
    int paletteChangeCount;
    uint32_t materialFingerprint;
} DM1_V1_F0662InvisibilityMaterialReceiptPc34;

uint32_t dm1_v1_f0662_invisibility_material_fnv1a_pc34(
    const unsigned char* bytes, int byteCount);

/* ReDMCSB CHAMDRAW.C G2362_auc_PaletteChanges_Invisibility. */
const unsigned char* dm1_v1_f0662_invisibility_palette_changes_pc34(void);

/* CHAMDRAW.C F0622 -> BASE.C F0662: C028's party-position icon strip may
 * receive the invisibility palette transform only when its original pixels,
 * the raw M653 record, and the exact source palette table are all present. */
int dm1_v1_f0662_invisibility_material_receipt_pc34(
    const DM1_V1_F0662SourceSurfacePc34* icon,
    const DM1_V1_F0662GlyphSourcePc34* glyph,
    const unsigned char* paletteChanges,
    int paletteChangeCount,
    DM1_V1_F0662InvisibilityMaterialReceiptPc34* outReceipt);

#endif
