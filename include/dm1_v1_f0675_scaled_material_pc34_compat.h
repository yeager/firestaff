#ifndef FIRESTAFF_DM1_V1_F0675_SCALED_MATERIAL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0675_SCALED_MATERIAL_PC34_COMPAT_H

#include <stdint.h>

typedef struct DM1_V1_F0675SourceSurfacePc34 {
    int graphicsDatOwned;
    int graphicIndex;
    int width;
    int height;
    int indexedPixelCount;
    const unsigned char* indexedPixels;
    uint32_t pixelsFNV1a;
} DM1_V1_F0675SourceSurfacePc34;

typedef struct DM1_V1_F0675ScaledMaterialReceiptPc34 {
    int valid;
    int suppressSyntheticFallback;
    int graphicIndex;
    int sourceWidth;
    int sourceHeight;
    int scaledWidth;
    int scaledHeight;
    int paletteChangeCount;
    uint32_t sourceFingerprint;
    uint32_t paletteFingerprint;
} DM1_V1_F0675ScaledMaterialReceiptPc34;

uint32_t dm1_v1_f0675_scaled_material_fnv1a_pc34(
    const unsigned char* bytes, int byteCount);

/* ReDMCSB DUNVIEW.C F0675 selects a native GRAPHICS.DAT surface before it
 * calls F0129 to derive the scaled copy. This receipt keeps the loader-owned
 * indexed source and the only caller-provided palette bytes source-bound. */
int dm1_v1_f0675_scaled_material_receipt_pc34(
    const DM1_V1_F0675SourceSurfacePc34* surface,
    int scaledWidth,
    int scaledHeight,
    const unsigned char* paletteChanges,
    int paletteChangeCount,
    DM1_V1_F0675ScaledMaterialReceiptPc34* outReceipt);

#endif
