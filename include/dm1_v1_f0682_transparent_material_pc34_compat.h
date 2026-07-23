#ifndef FIRESTAFF_DM1_V1_F0682_TRANSPARENT_MATERIAL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0682_TRANSPARENT_MATERIAL_PC34_COMPAT_H

#include <stdint.h>

typedef struct DM1_V1_F0682TransparentSurfacePc34 {
    int graphicsDatOwned;
    int graphicIndex;
    int width;
    int height;
    int indexedPixelCount;
    const unsigned char* indexedPixels;
    uint32_t pixelsFNV1a;
} DM1_V1_F0682TransparentSurfacePc34;

typedef struct DM1_V1_F0682TransparentMaterialReceiptPc34 {
    int valid;
    int suppressSyntheticFallback;
    int graphicIndex;
    int transparentColor;
    int paletteChangeCount;
    uint32_t sourceFingerprint;
    uint32_t paletteFingerprint;
} DM1_V1_F0682TransparentMaterialReceiptPc34;

uint32_t dm1_v1_f0682_transparent_material_fnv1a_pc34(
    const unsigned char* bytes, int byteCount);

/* ReDMCSB VIDEODRV.C F0682 omits only the original transparent index while
 * copying real indexed pixels. The current C486..C488 live route permits
 * G0212 palette bytes solely for the C488 smoke source. */
int dm1_v1_f0682_transparent_material_receipt_pc34(
    const DM1_V1_F0682TransparentSurfacePc34* surface,
    int transparentColor,
    const unsigned char* paletteChanges,
    int paletteChangeCount,
    DM1_V1_F0682TransparentMaterialReceiptPc34* outReceipt);

#endif
