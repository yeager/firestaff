#ifndef FIRESTAFF_DM1_V1_DOOR_SOURCE_MATERIAL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_DOOR_SOURCE_MATERIAL_PC34_COMPAT_H

#include <stdint.h>

/* A decoded PC34 GRAPHICS.DAT surface admitted for F0111.  This is source
 * material, not a host-created door texture: its byte identity must match
 * the supplied pixels and its source origin must be in bounds. F0111 may
 * first compose a scaled side-panel temporary bitmap, so destination extent
 * is intentionally not treated as a raw GRAPHICS.DAT source rectangle. */
typedef struct DM1_V1_DoorSourceMaterialPc34 {
    int graphicsDatOwned;
    int graphicIndex;
    int width;
    int height;
    const unsigned char* pixels;
    int pixelByteCount;
    uint32_t pixelsFNV1a;
} DM1_V1_DoorSourceMaterialPc34;

typedef struct DM1_V1_DoorSourceBlitPc34 {
    int graphicIndex;
    int srcX;
    int srcY;
    int width;
    int height;
} DM1_V1_DoorSourceBlitPc34;

static inline uint32_t DM1_V1_DoorSourcePixelsFNV1aPc34(
    const unsigned char* pixels,
    int pixelByteCount)
{
    uint32_t hash = 2166136261u;
    int i;
    if (!pixels || pixelByteCount <= 0) {
        return 0u;
    }
    for (i = 0; i < pixelByteCount; ++i) {
        hash ^= pixels[i];
        hash *= 16777619u;
    }
    return hash;
}

static inline int DM1_V1_DoorSourceMaterialForBlitPc34(
    const DM1_V1_DoorSourceMaterialPc34* materials,
    int materialCount,
    const DM1_V1_DoorSourceBlitPc34* blit,
    uint32_t* outPixelsFNV1a)
{
    int i;
    if (outPixelsFNV1a) {
        *outPixelsFNV1a = 0u;
    }
    if (!materials || materialCount <= 0 || !blit ||
        blit->graphicIndex < 0 || blit->srcX < 0 || blit->srcY < 0 ||
        blit->width <= 0 || blit->height <= 0) {
        return 0;
    }
    for (i = 0; i < materialCount; ++i) {
        const DM1_V1_DoorSourceMaterialPc34* material = &materials[i];
        uint32_t hash;
        if (!material->graphicsDatOwned ||
            material->graphicIndex != blit->graphicIndex ||
            !material->pixels || material->width <= 0 || material->height <= 0 ||
            material->pixelByteCount <= 0 ||
            material->pixelByteCount < ((material->width + 7) / 8) *
                material->height ||
            blit->srcX >= material->width || blit->srcY >= material->height) {
            continue;
        }
        hash = DM1_V1_DoorSourcePixelsFNV1aPc34(
            material->pixels, material->pixelByteCount);
        if (hash == 0u || hash != material->pixelsFNV1a) {
            continue;
        }
        if (outPixelsFNV1a) {
            *outPixelsFNV1a = hash;
        }
        return 1;
    }
    return 0;
}

#endif /* FIRESTAFF_DM1_V1_DOOR_SOURCE_MATERIAL_PC34_COMPAT_H */
