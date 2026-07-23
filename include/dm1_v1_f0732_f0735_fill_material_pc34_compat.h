#ifndef FIRESTAFF_DM1_V1_F0732_F0735_FILL_MATERIAL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0732_F0735_FILL_MATERIAL_PC34_COMPAT_H

#include <stdint.h>

enum {
    DM1_V1_F0732_SPELL_BACKGROUND_GRAPHIC_PC34 = 9,
    DM1_V1_F0735_INVENTORY_GRAPHIC_PC34 = 17,
    DM1_V1_F0732_SCREEN_WIDTH_PC34 = 320,
    DM1_V1_F0732_SCREEN_HEIGHT_PC34 = 200,
    DM1_V1_F0732_SPELL_ZONE_INDEX_PC34 = 13,
    DM1_V1_F0732_SPELL_LEFT_PC34 = 224,
    DM1_V1_F0732_SPELL_RIGHT_PC34 = 319,
    DM1_V1_F0732_SPELL_TOP_PC34 = 42,
    DM1_V1_F0732_SPELL_BOTTOM_PC34 = 74,
    DM1_V1_F0735_VIEWPORT_WIDTH_PC34 = 224,
    DM1_V1_F0735_VIEWPORT_HEIGHT_PC34 = 136
};

typedef struct DM1_V1_F0732F0735SurfacePc34 {
    int graphicsDatOwned;
    int graphicIndex;
    int width;
    int height;
    int indexedPixelCount;
    const unsigned char* indexedPixels;
    uint32_t indexedPixelsFNV1a;
} DM1_V1_F0732F0735SurfacePc34;

typedef struct DM1_V1_F0732F0735BoxPc34 {
    int left;
    int right;
    int top;
    int bottom;
} DM1_V1_F0732F0735BoxPc34;

typedef struct DM1_V1_F0732F0735ZonePc34 {
    int left;
    int top;
    int width;
    int height;
} DM1_V1_F0732F0735ZonePc34;

typedef void (*DM1_V1_F0732F0733ScreenFillPc34)(void* context,
                                                  const DM1_V1_F0732F0735BoxPc34* box,
                                                  int paletteIndex);
typedef void (*DM1_V1_F0735ViewportFillPc34)(void* context,
                                              const DM1_V1_F0732F0735BoxPc34* box,
                                              int paletteIndex);

typedef struct DM1_V1_F0732F0735ReceiptPc34 {
    int valid;
    int suppressSyntheticFallback;
    int functionId;
    int graphicIndex;
    int zoneIndex;
    int paletteIndex;
    DM1_V1_F0732F0735BoxPc34 box;
    uint32_t sourceFingerprint;
} DM1_V1_F0732F0735ReceiptPc34;

uint32_t dm1_v1_f0732_f0735_fnv1a_pc34(const unsigned char* bytes,
                                        int byteCount);

/* BLITFILL.C F0732: the C009-owned spell-area clear reaches a caller-owned
 * 320x200 screen fill only when its exact source ZONE and palette index are
 * admitted. */
int dm1_v1_f0732_clear_spell_area_pc34(
    const DM1_V1_F0732F0735SurfacePc34* spellSurface,
    const DM1_V1_F0732F0735ZonePc34* zone,
    int paletteIndex,
    DM1_V1_F0732F0733ScreenFillPc34 fillScreen,
    void* context,
    DM1_V1_F0732F0735ReceiptPc34* outReceipt);

/* BLITFILL.C F0733: F0638's source spell-area zone is forwarded unchanged
 * to the 320x200 screen fill. */
int dm1_v1_f0733_fill_spell_zone_pc34(
    const DM1_V1_F0732F0735SurfacePc34* spellSurface,
    int zoneIndex,
    const DM1_V1_F0732F0735BoxPc34* resolvedBox,
    int paletteIndex,
    DM1_V1_F0732F0733ScreenFillPc34 fillScreen,
    void* context,
    DM1_V1_F0732F0735ReceiptPc34* outReceipt);

/* BLITFILL.C F0735 directly forwards a caller-owned box over the raw C017
 * 224x136 viewport material. It creates neither a panel nor a palette. */
int dm1_v1_f0735_fill_inventory_viewport_pc34(
    const DM1_V1_F0732F0735SurfacePc34* inventorySurface,
    const DM1_V1_F0732F0735BoxPc34* box,
    int paletteIndex,
    DM1_V1_F0735ViewportFillPc34 fillViewport,
    void* context,
    DM1_V1_F0732F0735ReceiptPc34* outReceipt);

#endif
