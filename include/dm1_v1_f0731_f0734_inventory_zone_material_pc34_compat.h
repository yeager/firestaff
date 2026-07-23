#ifndef FIRESTAFF_DM1_V1_F0731_F0734_INVENTORY_ZONE_MATERIAL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0731_F0734_INVENTORY_ZONE_MATERIAL_PC34_COMPAT_H

#include <stdint.h>

enum {
    DM1_V1_F0731_SPELL_BACKGROUND_GRAPHIC_PC34 = 9,
    DM1_V1_F0734_INVENTORY_GRAPHIC_PC34 = 17,
    DM1_V1_F0734_DARKEST_GRAY_PC34 = 12,
    DM1_V1_F0734_VIEWPORT_WIDTH_PC34 = 224,
    DM1_V1_F0734_VIEWPORT_HEIGHT_PC34 = 136,
    DM1_V1_F0734_ZONE_SAVE_PC34 = 562,
    DM1_V1_F0734_ZONE_REST_PC34 = 564,
    DM1_V1_F0734_ZONE_CLOSE_PC34 = 566,
    DM1_V1_F0734_ZONE_MUSIC_PC34 = 568
};

typedef struct DM1_V1_F0731F0734SurfacePc34 {
    int graphicsDatOwned;
    int graphicIndex;
    int width;
    int height;
    int indexedPixelCount;
    const unsigned char* indexedPixels;
    uint32_t indexedPixelsFNV1a;
} DM1_V1_F0731F0734SurfacePc34;

typedef struct DM1_V1_F0731F0734ZonePc34 {
    int zoneIndex;
    int left;
    int right;
    int top;
    int bottom;
} DM1_V1_F0731F0734ZonePc34;

typedef void (*DM1_V1_F0731InvertBoxPc34)(void* context,
                                            int left, int right,
                                            int top, int bottom);
typedef void (*DM1_V1_F0734FillViewportBoxPc34)(void* context,
                                                  int left, int right,
                                                  int top, int bottom,
                                                  int color);

typedef struct DM1_V1_F0731F0734ReceiptPc34 {
    int valid;
    int suppressSyntheticFallback;
    int graphicIndex;
    int zoneIndex;
    int operation;
    uint32_t sourceFingerprint;
} DM1_V1_F0731F0734ReceiptPc34;

uint32_t dm1_v1_f0731_f0734_fnv1a_pc34(const unsigned char* bytes,
                                        int byteCount);

/* INVRTZON.C F0731 resolves one of four caster tabs in a five-zone group
 * before forwarding its inclusive rectangle to the video inverter. */
int dm1_v1_f0731_invert_spell_caster_zone_pc34(
    const DM1_V1_F0731F0734SurfacePc34* spellSurface,
    const DM1_V1_F0731F0734ZonePc34* zone,
    DM1_V1_F0731InvertBoxPc34 invertBox,
    void* context,
    DM1_V1_F0731F0734ReceiptPc34* outReceipt);

/* BLITFILL.C F0734 clears only the PC34 inventory icon zones with C12.
 * The destination stays caller-owned; no cursor, font, or replacement panel
 * is constructed here. */
int dm1_v1_f0734_clear_inventory_icon_zone_pc34(
    const DM1_V1_F0731F0734SurfacePc34* inventorySurface,
    const DM1_V1_F0731F0734ZonePc34* zone,
    DM1_V1_F0734FillViewportBoxPc34 fillBox,
    void* context,
    DM1_V1_F0731F0734ReceiptPc34* outReceipt);

#endif
