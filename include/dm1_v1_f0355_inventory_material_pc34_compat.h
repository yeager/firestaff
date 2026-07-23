#ifndef FIRESTAFF_DM1_V1_F0355_INVENTORY_MATERIAL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0355_INVENTORY_MATERIAL_PC34_COMPAT_H

#include <stdint.h>

enum {
    DM1_V1_F0355_C017_INVENTORY_PC34 = 17,
    DM1_V1_F0355_C033_SLOT_PC34 = 33,
    DM1_V1_F0355_M653_PC34 = 695,
    DM1_V1_F0355_M653_LEGACY_PC34 = 557,
    DM1_V1_F0355_M653_BYTES_PC34 = 768
};

typedef struct DM1_V1_F0355SourceSurfacePc34 {
    int graphicsDatOwned;
    int graphicIndex;
    int width;
    int height;
    int indexedPixelCount;
    const unsigned char* indexedPixels;
    uint32_t pixelsFNV1a;
} DM1_V1_F0355SourceSurfacePc34;

typedef struct DM1_V1_F0355GlyphSourcePc34 {
    int graphicsDatOwned;
    int graphicIndex;
    const unsigned char* bits;
    int byteCount;
    uint32_t bitsFNV1a;
} DM1_V1_F0355GlyphSourcePc34;

typedef struct DM1_V1_F0355InventoryMaterialReceiptPc34 {
    int valid;
    int suppressSyntheticFallback;
    int inventoryGraphicIndex;
    int inventoryWidth;
    int inventoryHeight;
    int slotGraphicIndex;
    int slotWidth;
    int slotHeight;
    int m653GraphicIndex;
    uint32_t materialFingerprint;
} DM1_V1_F0355InventoryMaterialReceiptPc34;

uint32_t dm1_v1_f0355_inventory_material_fnv1a_pc34(
    const unsigned char* bytes, int byteCount);

/* PANEL.C F0355: the live inventory route may publish C017 and its C033
 * slots only from one authenticated raw GRAPHICS.DAT/M653 session. */
int dm1_v1_f0355_inventory_material_receipt_pc34(
    const DM1_V1_F0355SourceSurfacePc34* surfaces, int surfaceCount,
    const DM1_V1_F0355GlyphSourcePc34* glyphs, int glyphCount,
    DM1_V1_F0355InventoryMaterialReceiptPc34* outReceipt);

#endif
