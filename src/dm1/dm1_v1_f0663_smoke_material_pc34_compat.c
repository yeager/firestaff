#include "dm1_v1_f0663_smoke_material_pc34_compat.h"

#include <string.h>

static const unsigned char kDm1V1F0663SmokePaletteChangesPc34[
    DM1_V1_F0663_PALETTE_COUNT_PC34] = {
    0, 1, 2, 3, 4, 5, 12, 1, 8, 9, 10, 11, 12, 13, 14, 15
};

uint32_t dm1_v1_f0663_smoke_material_fnv1a_pc34(
    const unsigned char* bytes, int byteCount)
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

const unsigned char* dm1_v1_f0663_smoke_palette_changes_pc34(void)
{
    return kDm1V1F0663SmokePaletteChangesPc34;
}

int dm1_v1_f0663_smoke_surface_receipt_pc34(
    const DM1_V1_F0663SourceSurfacePc34* surface,
    int expectedGraphicIndex,
    const unsigned char* paletteChanges,
    int paletteChangeCount,
    DM1_V1_F0663SmokeSurfaceReceiptPc34* outReceipt)
{
    uint32_t sourceHash;
    uint32_t paletteHash;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!surface || !surface->graphicsDatOwned || !surface->indexedPixels ||
        surface->graphicIndex != expectedGraphicIndex ||
        surface->width <= 0 || surface->height <= 0 ||
        surface->indexedPixelCount != surface->width * surface->height ||
        (expectedGraphicIndex != DM1_V1_F0663_C488_POISON_SOURCE_PC34 &&
         (expectedGraphicIndex < DM1_V1_F0663_C498_SMOKE_PATTERN_SMALL_PC34 ||
          expectedGraphicIndex > DM1_V1_F0663_C500_SMOKE_PATTERN_LARGE_PC34)) ||
        !paletteChanges ||
        paletteChangeCount != DM1_V1_F0663_PALETTE_COUNT_PC34 ||
        memcmp(paletteChanges, kDm1V1F0663SmokePaletteChangesPc34,
               DM1_V1_F0663_PALETTE_COUNT_PC34) != 0) {
        return 0;
    }
    sourceHash = dm1_v1_f0663_smoke_material_fnv1a_pc34(
        surface->indexedPixels, surface->indexedPixelCount);
    paletteHash = dm1_v1_f0663_smoke_material_fnv1a_pc34(
        paletteChanges, paletteChangeCount);
    if (!sourceHash || sourceHash != surface->pixelsFNV1a || !paletteHash) {
        return 0;
    }
    outReceipt->valid = 1;
    outReceipt->suppressSyntheticFallback = 1;
    outReceipt->graphicIndex = expectedGraphicIndex;
    outReceipt->paletteChangeCount = paletteChangeCount;
    outReceipt->replacementSourceA = 6;
    outReceipt->replacementDestinationA = 12;
    outReceipt->replacementSourceB = 7;
    outReceipt->replacementDestinationB = 1;
    outReceipt->sourceFingerprint = sourceHash;
    outReceipt->paletteFingerprint = paletteHash;
    return 1;
}

int dm1_v1_f0663_smoke_material_receipt_pc34(
    const DM1_V1_F0663SourceSurfacePc34* surfaces,
    int surfaceCount,
    const unsigned char* paletteChanges,
    int paletteChangeCount,
    DM1_V1_F0663SmokeMaterialReceiptPc34* outReceipt)
{
    const int graphics[DM1_V1_F0663_SURFACE_COUNT_PC34] = {
        DM1_V1_F0663_C488_POISON_SOURCE_PC34,
        DM1_V1_F0663_C498_SMOKE_PATTERN_SMALL_PC34,
        DM1_V1_F0663_C499_SMOKE_PATTERN_MEDIUM_PC34,
        DM1_V1_F0663_C500_SMOKE_PATTERN_LARGE_PC34
    };
    uint32_t hashes[DM1_V1_F0663_SURFACE_COUNT_PC34] = {0u, 0u, 0u, 0u};
    uint32_t paletteHash;
    uint32_t fingerprint = 2166136261u;
    int found[DM1_V1_F0663_SURFACE_COUNT_PC34] = {0, 0, 0, 0};
    int i;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!surfaces || !paletteChanges ||
        surfaceCount != DM1_V1_F0663_SURFACE_COUNT_PC34 ||
        paletteChangeCount != DM1_V1_F0663_PALETTE_COUNT_PC34 ||
        memcmp(paletteChanges, kDm1V1F0663SmokePaletteChangesPc34,
               DM1_V1_F0663_PALETTE_COUNT_PC34) != 0) {
        return 0;
    }
    for (i = 0; i < surfaceCount; ++i) {
        const DM1_V1_F0663SourceSurfacePc34* surface = &surfaces[i];
        uint32_t hash;
        int index = -1;
        int j;
        if (!surface->graphicsDatOwned || !surface->indexedPixels ||
            surface->width <= 0 || surface->height <= 0 ||
            surface->indexedPixelCount != surface->width * surface->height) {
            return 0;
        }
        for (j = 0; j < DM1_V1_F0663_SURFACE_COUNT_PC34; ++j) {
            if (surface->graphicIndex == graphics[j]) {
                index = j;
                break;
            }
        }
        if (index < 0 || found[index]) return 0;
        hash = dm1_v1_f0663_smoke_material_fnv1a_pc34(
            surface->indexedPixels, surface->indexedPixelCount);
        if (!hash || hash != surface->pixelsFNV1a) return 0;
        hashes[index] = hash;
        found[index] = 1;
    }
    for (i = 0; i < DM1_V1_F0663_SURFACE_COUNT_PC34; ++i) {
        if (!found[i]) return 0;
        fingerprint ^= hashes[i];
        fingerprint *= 16777619u;
    }
    paletteHash = dm1_v1_f0663_smoke_material_fnv1a_pc34(
        paletteChanges, paletteChangeCount);
    if (!paletteHash) return 0;
    fingerprint ^= paletteHash;
    fingerprint *= 16777619u;
    if (!fingerprint) return 0;
    outReceipt->valid = 1;
    outReceipt->suppressSyntheticFallback = 1;
    outReceipt->poisonGraphicIndex = graphics[0];
    outReceipt->smokePatternFirstGraphicIndex = graphics[1];
    outReceipt->smokePatternCount = 3;
    outReceipt->paletteChangeCount = paletteChangeCount;
    outReceipt->replacementSourceA = 6;
    outReceipt->replacementDestinationA = 12;
    outReceipt->replacementSourceB = 7;
    outReceipt->replacementDestinationB = 1;
    outReceipt->materialFingerprint = fingerprint;
    return 1;
}
