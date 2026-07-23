#ifndef FIRESTAFF_DM1_V1_F0663_SMOKE_MATERIAL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0663_SMOKE_MATERIAL_PC34_COMPAT_H

#include <stdint.h>

enum {
    DM1_V1_F0663_C488_POISON_SOURCE_PC34 = 488,
    DM1_V1_F0663_C498_SMOKE_PATTERN_SMALL_PC34 = 498,
    DM1_V1_F0663_C499_SMOKE_PATTERN_MEDIUM_PC34 = 499,
    DM1_V1_F0663_C500_SMOKE_PATTERN_LARGE_PC34 = 500,
    DM1_V1_F0663_SURFACE_COUNT_PC34 = 4,
    DM1_V1_F0663_PALETTE_COUNT_PC34 = 16
};

typedef struct DM1_V1_F0663SourceSurfacePc34 {
    int graphicsDatOwned;
    int graphicIndex;
    int width;
    int height;
    int indexedPixelCount;
    const unsigned char* indexedPixels;
    uint32_t pixelsFNV1a;
} DM1_V1_F0663SourceSurfacePc34;

typedef struct DM1_V1_F0663SmokeMaterialReceiptPc34 {
    int valid;
    int suppressSyntheticFallback;
    int poisonGraphicIndex;
    int smokePatternFirstGraphicIndex;
    int smokePatternCount;
    int paletteChangeCount;
    int replacementSourceA;
    int replacementDestinationA;
    int replacementSourceB;
    int replacementDestinationB;
    uint32_t materialFingerprint;
} DM1_V1_F0663SmokeMaterialReceiptPc34;

uint32_t dm1_v1_f0663_smoke_material_fnv1a_pc34(
    const unsigned char* bytes, int byteCount);

/* ReDMCSB DUNVIEW.C G0212_auc_Graphic558_PaletteChanges_Smoke. */
const unsigned char* dm1_v1_f0663_smoke_palette_changes_pc34(void);

/* BASE.C F0663 receives C488 and the D0C C498..C500 pattern family through
 * the same original smoke palette copy. No generated bloom or palette is
 * permitted when one of those GRAPHICS.DAT inputs is unavailable. */
int dm1_v1_f0663_smoke_material_receipt_pc34(
    const DM1_V1_F0663SourceSurfacePc34* surfaces,
    int surfaceCount,
    const unsigned char* paletteChanges,
    int paletteChangeCount,
    DM1_V1_F0663SmokeMaterialReceiptPc34* outReceipt);

#endif
