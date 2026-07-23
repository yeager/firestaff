#include "dm1_v1_f0675_scaled_material_pc34_compat.h"

#include "dm1_v1_f0663_smoke_material_pc34_compat.h"

#include <string.h>

uint32_t dm1_v1_f0675_scaled_material_fnv1a_pc34(
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

int dm1_v1_f0675_scaled_material_receipt_pc34(
    const DM1_V1_F0675SourceSurfacePc34* surface,
    int scaledWidth,
    int scaledHeight,
    const unsigned char* paletteChanges,
    int paletteChangeCount,
    DM1_V1_F0675ScaledMaterialReceiptPc34* outReceipt)
{
    uint32_t sourceFingerprint;
    uint32_t paletteFingerprint = 0u;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!surface || !surface->graphicsDatOwned || surface->graphicIndex < 0 ||
        surface->width <= 0 || surface->height <= 0 ||
        surface->indexedPixelCount != surface->width * surface->height ||
        !surface->indexedPixels || scaledWidth <= 0 || scaledHeight <= 0) {
        return 0;
    }
    sourceFingerprint = dm1_v1_f0675_scaled_material_fnv1a_pc34(
        surface->indexedPixels, surface->indexedPixelCount);
    if (!sourceFingerprint || sourceFingerprint != surface->pixelsFNV1a) {
        return 0;
    }
    if (paletteChanges || paletteChangeCount) {
        if (!paletteChanges ||
            paletteChangeCount != DM1_V1_F0663_PALETTE_COUNT_PC34 ||
            memcmp(paletteChanges, dm1_v1_f0663_smoke_palette_changes_pc34(),
                   DM1_V1_F0663_PALETTE_COUNT_PC34) != 0) {
            return 0;
        }
        paletteFingerprint = dm1_v1_f0675_scaled_material_fnv1a_pc34(
            paletteChanges, paletteChangeCount);
        if (!paletteFingerprint) return 0;
    }
    outReceipt->valid = 1;
    outReceipt->suppressSyntheticFallback = 1;
    outReceipt->graphicIndex = surface->graphicIndex;
    outReceipt->sourceWidth = surface->width;
    outReceipt->sourceHeight = surface->height;
    outReceipt->scaledWidth = scaledWidth;
    outReceipt->scaledHeight = scaledHeight;
    outReceipt->paletteChangeCount = paletteChangeCount;
    outReceipt->sourceFingerprint = sourceFingerprint;
    outReceipt->paletteFingerprint = paletteFingerprint;
    return 1;
}
