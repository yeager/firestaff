#include "dm1_v1_f0355_inventory_material_pc34_compat.h"

#include <string.h>

uint32_t dm1_v1_f0355_inventory_material_fnv1a_pc34(
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

int dm1_v1_f0355_inventory_material_receipt_pc34(
    const DM1_V1_F0355SourceSurfacePc34* surfaces, int surfaceCount,
    const DM1_V1_F0355GlyphSourcePc34* glyphs, int glyphCount,
    DM1_V1_F0355InventoryMaterialReceiptPc34* outReceipt)
{
    uint32_t inventoryHash = 0u;
    uint32_t slotHash = 0u;
    uint32_t glyphHash = 0u;
    uint32_t fingerprint = 2166136261u;
    int inventoryOk = 0;
    int slotOk = 0;
    int glyphIndex = -1;
    int i;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    for (i = 0; surfaces && i < surfaceCount; ++i) {
        const DM1_V1_F0355SourceSurfacePc34* source = &surfaces[i];
        uint32_t hash;
        int expectedWidth;
        int expectedHeight;
        if (!source->graphicsDatOwned || !source->indexedPixels ||
            (source->graphicIndex != DM1_V1_F0355_C017_INVENTORY_PC34 &&
             source->graphicIndex != DM1_V1_F0355_C033_SLOT_PC34)) {
            continue;
        }
        expectedWidth = source->graphicIndex == DM1_V1_F0355_C017_INVENTORY_PC34
            ? 224 : 18;
        expectedHeight = source->graphicIndex == DM1_V1_F0355_C017_INVENTORY_PC34
            ? 136 : 18;
        if (source->width != expectedWidth || source->height != expectedHeight ||
            source->indexedPixelCount < expectedWidth * expectedHeight) {
            continue;
        }
        hash = dm1_v1_f0355_inventory_material_fnv1a_pc34(
            source->indexedPixels, source->indexedPixelCount);
        if (!hash || hash != source->pixelsFNV1a) continue;
        if (source->graphicIndex == DM1_V1_F0355_C017_INVENTORY_PC34) {
            inventoryHash = hash;
            inventoryOk = 1;
        } else {
            slotHash = hash;
            slotOk = 1;
        }
    }
    for (i = 0; glyphs && i < glyphCount; ++i) {
        const DM1_V1_F0355GlyphSourcePc34* glyph = &glyphs[i];
        if (!glyph->graphicsDatOwned || !glyph->bits ||
            glyph->byteCount != DM1_V1_F0355_M653_BYTES_PC34 ||
            (glyph->graphicIndex != DM1_V1_F0355_M653_PC34 &&
             glyph->graphicIndex != DM1_V1_F0355_M653_LEGACY_PC34)) {
            continue;
        }
        glyphHash = dm1_v1_f0355_inventory_material_fnv1a_pc34(
            glyph->bits, glyph->byteCount);
        if (glyphHash && glyphHash == glyph->bitsFNV1a) {
            glyphIndex = glyph->graphicIndex;
        }
    }
    if (!inventoryOk || !slotOk || glyphIndex < 0) return 0;
    fingerprint ^= inventoryHash; fingerprint *= 16777619u;
    fingerprint ^= slotHash; fingerprint *= 16777619u;
    fingerprint ^= glyphHash; fingerprint *= 16777619u;
    if (!fingerprint) return 0;

    outReceipt->valid = 1;
    outReceipt->suppressSyntheticFallback = 1;
    outReceipt->inventoryGraphicIndex = DM1_V1_F0355_C017_INVENTORY_PC34;
    outReceipt->inventoryWidth = 224;
    outReceipt->inventoryHeight = 136;
    outReceipt->slotGraphicIndex = DM1_V1_F0355_C033_SLOT_PC34;
    outReceipt->slotWidth = 18;
    outReceipt->slotHeight = 18;
    outReceipt->m653GraphicIndex = glyphIndex;
    outReceipt->materialFingerprint = fingerprint;
    return 1;
}
