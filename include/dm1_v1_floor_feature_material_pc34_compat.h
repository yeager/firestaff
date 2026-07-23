#ifndef FIRESTAFF_DM1_V1_FLOOR_FEATURE_MATERIAL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_FLOOR_FEATURE_MATERIAL_PC34_COMPAT_H

#include <stdint.h>

/* Original PC34 GRAPHICS.DAT surface supplied by the active decoder. */
typedef struct DM1_V1_FloorFeatureSourceMaterialPc34 {
    int graphicsDatOwned;
    int graphicIndex;
    int width;
    int height;
    const unsigned char* indexedPixels;
    int indexedPixelCount;
    uint32_t pixelsFNV1a;
} DM1_V1_FloorFeatureSourceMaterialPc34;

typedef enum DM1_V1_FloorFeaturePaletteRoutePc34 {
    DM1_V1_FLOOR_FEATURE_PALETTE_NATIVE_PC34 = 0,
    DM1_V1_FLOOR_FEATURE_PALETTE_FLOOR_ORNAMENT_D3_PC34 = 1,
    DM1_V1_FLOOR_FEATURE_PALETTE_FLOOR_ORNAMENT_D2_PC34 = 2
} DM1_V1_FloorFeaturePaletteRoutePc34;

typedef struct DM1_V1_FloorFeatureMaterialReceiptPc34 {
    int valid;
    int graphicIndex;
    int srcX;
    int srcY;
    int dstX;
    int dstY;
    int width;
    int height;
    int paletteRoute;
    unsigned char paletteMap[16];
    uint32_t sourcePixelsFNV1a;
    uint32_t paletteFNV1a;
} DM1_V1_FloorFeatureMaterialReceiptPc34;

static inline uint32_t DM1_V1_FloorFeatureFNV1aPc34(
    const unsigned char* bytes,
    int byteCount)
{
    uint32_t hash = 2166136261u;
    int i;
    if (!bytes || byteCount <= 0) return 0u;
    for (i = 0; i < byteCount; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

static inline int DM1_V1_FloorFeatureFindSourceMaterialPc34(
    const DM1_V1_FloorFeatureSourceMaterialPc34* materials,
    int materialCount,
    int graphicIndex,
    int srcX,
    int srcY,
    int width,
    int height,
    uint32_t* outPixelsFNV1a)
{
    int i;
    if (outPixelsFNV1a) *outPixelsFNV1a = 0u;
    if (!materials || materialCount <= 0 || graphicIndex < 0 || srcX < 0 ||
        srcY < 0 || width <= 0 || height <= 0) return 0;
    for (i = 0; i < materialCount; ++i) {
        const DM1_V1_FloorFeatureSourceMaterialPc34* source = &materials[i];
        uint32_t hash;
        if (!source->graphicsDatOwned || source->graphicIndex != graphicIndex ||
            !source->indexedPixels || source->width <= 0 || source->height <= 0 ||
            source->indexedPixelCount < source->width * source->height ||
            srcX + width > source->width || srcY + height > source->height) {
            continue;
        }
        hash = DM1_V1_FloorFeatureFNV1aPc34(source->indexedPixels,
                                             source->indexedPixelCount);
        if (hash == 0u || hash != source->pixelsFNV1a) continue;
        if (outPixelsFNV1a) *outPixelsFNV1a = hash;
        return 1;
    }
    return 0;
}

#endif /* FIRESTAFF_DM1_V1_FLOOR_FEATURE_MATERIAL_PC34_COMPAT_H */
