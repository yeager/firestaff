#include "dm1_v1_f0731_f0734_inventory_zone_material_pc34_compat.h"

#include <string.h>

enum {
    DM1_V1_F0731_OPERATION_INVERT_PC34 = 1,
    DM1_V1_F0734_OPERATION_CLEAR_PC34 = 2,
    DM1_V1_F0731_CASTER_ZONE_FIRST_PC34 = 224,
    DM1_V1_F0731_CASTER_ZONE_GROUP_COUNT_PC34 = 4,
    DM1_V1_F0731_CASTER_ZONE_GROUP_STRIDE_PC34 = 5,
    DM1_V1_F0731_TAB_Y0_PC34 = 42,
    DM1_V1_F0731_TAB_Y1_PC34 = 48
};

uint32_t dm1_v1_f0731_f0734_fnv1a_pc34(const unsigned char* bytes,
                                        int byteCount)
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

static int source_surface_valid(const DM1_V1_F0731F0734SurfacePc34* surface,
                                int expectedGraphic, uint32_t* outFingerprint)
{
    uint32_t fingerprint;

    if (!surface || !outFingerprint || !surface->graphicsDatOwned ||
        surface->graphicIndex != expectedGraphic || !surface->indexedPixels ||
        surface->width <= 0 || surface->height <= 0 ||
        surface->indexedPixelCount != surface->width * surface->height ||
        (expectedGraphic == DM1_V1_F0731_SPELL_BACKGROUND_GRAPHIC_PC34 &&
         (surface->width != 87 || surface->height != 25)) ||
        (expectedGraphic == DM1_V1_F0734_INVENTORY_GRAPHIC_PC34 &&
         (surface->width != DM1_V1_F0734_VIEWPORT_WIDTH_PC34 ||
          surface->height != DM1_V1_F0734_VIEWPORT_HEIGHT_PC34))) {
        return 0;
    }
    fingerprint = dm1_v1_f0731_f0734_fnv1a_pc34(
        surface->indexedPixels, surface->indexedPixelCount);
    if (!fingerprint || fingerprint != surface->indexedPixelsFNV1a) return 0;
    *outFingerprint = fingerprint;
    return 1;
}

static int spell_zone_valid(const DM1_V1_F0731F0734ZonePc34* zone)
{
    static const int tabLeft[4] = { 233, 280, 294, 308 };
    static const int tabRight[4] = { 277, 291, 305, 319 };
    int relative;
    int tab;

    if (!zone) return 0;
    relative = zone->zoneIndex - DM1_V1_F0731_CASTER_ZONE_FIRST_PC34;
    if (relative < 0 || relative >=
            DM1_V1_F0731_CASTER_ZONE_GROUP_COUNT_PC34 *
            DM1_V1_F0731_CASTER_ZONE_GROUP_STRIDE_PC34) {
        return 0;
    }
    tab = relative % DM1_V1_F0731_CASTER_ZONE_GROUP_STRIDE_PC34;
    return tab < 4 && zone->left == tabLeft[tab] &&
           zone->right == tabRight[tab] &&
           zone->top == DM1_V1_F0731_TAB_Y0_PC34 &&
           zone->bottom == (tab == 0 ? 49 : DM1_V1_F0731_TAB_Y1_PC34);
}

static int inventory_zone_valid(const DM1_V1_F0731F0734ZonePc34* zone)
{
    if (!zone || (zone->zoneIndex != DM1_V1_F0734_ZONE_SAVE_PC34 &&
                  zone->zoneIndex != DM1_V1_F0734_ZONE_REST_PC34 &&
                  zone->zoneIndex != DM1_V1_F0734_ZONE_CLOSE_PC34 &&
                  zone->zoneIndex != DM1_V1_F0734_ZONE_MUSIC_PC34)) {
        return 0;
    }
    return zone->left >= 0 && zone->right >= zone->left &&
           zone->right < DM1_V1_F0734_VIEWPORT_WIDTH_PC34 &&
           zone->top >= 0 && zone->bottom >= zone->top &&
           zone->bottom < DM1_V1_F0734_VIEWPORT_HEIGHT_PC34;
}

int dm1_v1_f0731_invert_spell_caster_zone_pc34(
    const DM1_V1_F0731F0734SurfacePc34* spellSurface,
    const DM1_V1_F0731F0734ZonePc34* zone,
    DM1_V1_F0731InvertBoxPc34 invertBox,
    void* context,
    DM1_V1_F0731F0734ReceiptPc34* outReceipt)
{
    uint32_t fingerprint;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!invertBox || !source_surface_valid(
            spellSurface, DM1_V1_F0731_SPELL_BACKGROUND_GRAPHIC_PC34,
            &fingerprint) || !spell_zone_valid(zone)) {
        return 0;
    }
    invertBox(context, zone->left, zone->right, zone->top, zone->bottom);
    outReceipt->valid = 1;
    outReceipt->suppressSyntheticFallback = 1;
    outReceipt->graphicIndex = spellSurface->graphicIndex;
    outReceipt->zoneIndex = zone->zoneIndex;
    outReceipt->operation = DM1_V1_F0731_OPERATION_INVERT_PC34;
    outReceipt->sourceFingerprint = fingerprint;
    return 1;
}

int dm1_v1_f0734_clear_inventory_icon_zone_pc34(
    const DM1_V1_F0731F0734SurfacePc34* inventorySurface,
    const DM1_V1_F0731F0734ZonePc34* zone,
    DM1_V1_F0734FillViewportBoxPc34 fillBox,
    void* context,
    DM1_V1_F0731F0734ReceiptPc34* outReceipt)
{
    uint32_t fingerprint;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!fillBox || !source_surface_valid(
            inventorySurface, DM1_V1_F0734_INVENTORY_GRAPHIC_PC34,
            &fingerprint) || !inventory_zone_valid(zone)) {
        return 0;
    }
    fillBox(context, zone->left, zone->right, zone->top, zone->bottom,
            DM1_V1_F0734_DARKEST_GRAY_PC34);
    outReceipt->valid = 1;
    outReceipt->suppressSyntheticFallback = 1;
    outReceipt->graphicIndex = inventorySurface->graphicIndex;
    outReceipt->zoneIndex = zone->zoneIndex;
    outReceipt->operation = DM1_V1_F0734_OPERATION_CLEAR_PC34;
    outReceipt->sourceFingerprint = fingerprint;
    return 1;
}
