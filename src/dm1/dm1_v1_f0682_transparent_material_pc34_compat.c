#include "dm1_v1_f0682_transparent_material_pc34_compat.h"

#include "dm1_v1_f0663_smoke_material_pc34_compat.h"

#include <string.h>

enum {
    DM1_V1_F0682_C486_FIRE_PC34 = 486,
    DM1_V1_F0682_C487_SPELL_PC34 = 487,
    DM1_V1_F0682_C488_POISON_PC34 = 488,
    DM1_V1_F0682_TRANSPARENT_COLOR_PC34 = 0
};

uint32_t dm1_v1_f0682_transparent_material_fnv1a_pc34(
    const unsigned char* bytes, int byteCount)
{
    uint32_t hash = 2166136261u;
    int index;

    if (!bytes || byteCount <= 0) return 0u;
    for (index = 0; index < byteCount; ++index) {
        hash ^= bytes[index];
        hash *= 16777619u;
    }
    return hash;
}

int dm1_v1_f0682_transparent_material_receipt_pc34(
    const DM1_V1_F0682TransparentSurfacePc34* surface,
    int transparentColor,
    const unsigned char* paletteChanges,
    int paletteChangeCount,
    DM1_V1_F0682TransparentMaterialReceiptPc34* outReceipt)
{
    uint32_t sourceFingerprint;
    uint32_t paletteFingerprint = 0u;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!surface || !surface->graphicsDatOwned || !surface->indexedPixels ||
        surface->width <= 0 || surface->height <= 0 ||
        surface->indexedPixelCount != surface->width * surface->height ||
        transparentColor != DM1_V1_F0682_TRANSPARENT_COLOR_PC34 ||
        (surface->graphicIndex != DM1_V1_F0682_C486_FIRE_PC34 &&
         surface->graphicIndex != DM1_V1_F0682_C487_SPELL_PC34 &&
         surface->graphicIndex != DM1_V1_F0682_C488_POISON_PC34)) {
        return 0;
    }
    sourceFingerprint = dm1_v1_f0682_transparent_material_fnv1a_pc34(
        surface->indexedPixels, surface->indexedPixelCount);
    if (!sourceFingerprint || sourceFingerprint != surface->pixelsFNV1a) {
        return 0;
    }
    if (paletteChanges || paletteChangeCount) {
        if (surface->graphicIndex != DM1_V1_F0682_C488_POISON_PC34 ||
            !paletteChanges ||
            paletteChangeCount != DM1_V1_F0663_PALETTE_COUNT_PC34 ||
            memcmp(paletteChanges, dm1_v1_f0663_smoke_palette_changes_pc34(),
                   DM1_V1_F0663_PALETTE_COUNT_PC34) != 0) {
            return 0;
        }
        paletteFingerprint = dm1_v1_f0682_transparent_material_fnv1a_pc34(
            paletteChanges, paletteChangeCount);
        if (!paletteFingerprint) return 0;
    }
    outReceipt->valid = 1;
    outReceipt->suppressSyntheticFallback = 1;
    outReceipt->graphicIndex = surface->graphicIndex;
    outReceipt->transparentColor = transparentColor;
    outReceipt->paletteChangeCount = paletteChangeCount;
    outReceipt->sourceFingerprint = sourceFingerprint;
    outReceipt->paletteFingerprint = paletteFingerprint;
    return 1;
}
