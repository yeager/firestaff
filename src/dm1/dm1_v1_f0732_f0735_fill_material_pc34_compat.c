#include "dm1_v1_f0732_f0735_fill_material_pc34_compat.h"

#include <string.h>

enum {
    DM1_V1_F0732_FUNCTION_ID_PC34 = 732,
    DM1_V1_F0733_FUNCTION_ID_PC34 = 733,
    DM1_V1_F0735_FUNCTION_ID_PC34 = 735,
    DM1_V1_F0732_COLOR_BLACK_PC34 = 0
};

uint32_t dm1_v1_f0732_f0735_fnv1a_pc34(const unsigned char* bytes,
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

static int source_surface_valid(const DM1_V1_F0732F0735SurfacePc34* surface,
                                int graphicIndex, int width, int height,
                                uint32_t* outFingerprint)
{
    uint32_t fingerprint;

    if (!surface || !outFingerprint || !surface->graphicsDatOwned ||
        surface->graphicIndex != graphicIndex || !surface->indexedPixels ||
        surface->width != width || surface->height != height ||
        surface->indexedPixelCount != width * height) {
        return 0;
    }
    fingerprint = dm1_v1_f0732_f0735_fnv1a_pc34(
        surface->indexedPixels, surface->indexedPixelCount);
    if (!fingerprint || fingerprint != surface->indexedPixelsFNV1a) return 0;
    *outFingerprint = fingerprint;
    return 1;
}

static int palette_index_valid(int paletteIndex)
{
    return paletteIndex >= 0 && paletteIndex < 16;
}

static int spell_box_valid(const DM1_V1_F0732F0735BoxPc34* box)
{
    return box && box->left == DM1_V1_F0732_SPELL_LEFT_PC34 &&
           box->right == DM1_V1_F0732_SPELL_RIGHT_PC34 &&
           box->top == DM1_V1_F0732_SPELL_TOP_PC34 &&
           box->bottom == DM1_V1_F0732_SPELL_BOTTOM_PC34;
}

static void receipt_write(DM1_V1_F0732F0735ReceiptPc34* receipt,
                          int functionId, int graphicIndex, int zoneIndex,
                          int paletteIndex,
                          const DM1_V1_F0732F0735BoxPc34* box,
                          uint32_t fingerprint)
{
    receipt->valid = 1;
    receipt->suppressSyntheticFallback = 1;
    receipt->functionId = functionId;
    receipt->graphicIndex = graphicIndex;
    receipt->zoneIndex = zoneIndex;
    receipt->paletteIndex = paletteIndex;
    receipt->box = *box;
    receipt->sourceFingerprint = fingerprint;
}

int dm1_v1_f0732_clear_spell_area_pc34(
    const DM1_V1_F0732F0735SurfacePc34* spellSurface,
    const DM1_V1_F0732F0735ZonePc34* zone,
    int paletteIndex,
    DM1_V1_F0732F0733ScreenFillPc34 fillScreen,
    void* context,
    DM1_V1_F0732F0735ReceiptPc34* outReceipt)
{
    DM1_V1_F0732F0735BoxPc34 box;
    uint32_t fingerprint;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!zone || !fillScreen || paletteIndex != DM1_V1_F0732_COLOR_BLACK_PC34 ||
        zone->left != DM1_V1_F0732_SPELL_LEFT_PC34 ||
        zone->top != DM1_V1_F0732_SPELL_TOP_PC34 || zone->width != 96 ||
        zone->height != 33 || !source_surface_valid(
            spellSurface, DM1_V1_F0732_SPELL_BACKGROUND_GRAPHIC_PC34,
            87, 25, &fingerprint)) {
        return 0;
    }
    box.left = zone->left;
    box.right = zone->left + zone->width - 1;
    box.top = zone->top;
    box.bottom = zone->top + zone->height - 1;
    if (!spell_box_valid(&box)) return 0;
    fillScreen(context, &box, paletteIndex);
    receipt_write(outReceipt, DM1_V1_F0732_FUNCTION_ID_PC34,
                  spellSurface->graphicIndex, DM1_V1_F0732_SPELL_ZONE_INDEX_PC34,
                  paletteIndex, &box, fingerprint);
    return 1;
}

int dm1_v1_f0733_fill_spell_zone_pc34(
    const DM1_V1_F0732F0735SurfacePc34* spellSurface,
    int zoneIndex,
    const DM1_V1_F0732F0735BoxPc34* resolvedBox,
    int paletteIndex,
    DM1_V1_F0732F0733ScreenFillPc34 fillScreen,
    void* context,
    DM1_V1_F0732F0735ReceiptPc34* outReceipt)
{
    uint32_t fingerprint;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!fillScreen || zoneIndex != DM1_V1_F0732_SPELL_ZONE_INDEX_PC34 ||
        paletteIndex != DM1_V1_F0732_COLOR_BLACK_PC34 ||
        !spell_box_valid(resolvedBox) || !source_surface_valid(
            spellSurface, DM1_V1_F0732_SPELL_BACKGROUND_GRAPHIC_PC34,
            87, 25, &fingerprint)) {
        return 0;
    }
    fillScreen(context, resolvedBox, paletteIndex);
    receipt_write(outReceipt, DM1_V1_F0733_FUNCTION_ID_PC34,
                  spellSurface->graphicIndex, zoneIndex, paletteIndex,
                  resolvedBox, fingerprint);
    return 1;
}

int dm1_v1_f0735_fill_inventory_viewport_pc34(
    const DM1_V1_F0732F0735SurfacePc34* inventorySurface,
    const DM1_V1_F0732F0735BoxPc34* box,
    int paletteIndex,
    DM1_V1_F0735ViewportFillPc34 fillViewport,
    void* context,
    DM1_V1_F0732F0735ReceiptPc34* outReceipt)
{
    uint32_t fingerprint;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!fillViewport || !palette_index_valid(paletteIndex) || !box ||
        box->left < 0 || box->right < box->left ||
        box->right >= DM1_V1_F0735_VIEWPORT_WIDTH_PC34 || box->top < 0 ||
        box->bottom < box->top ||
        box->bottom >= DM1_V1_F0735_VIEWPORT_HEIGHT_PC34 ||
        !source_surface_valid(inventorySurface,
                              DM1_V1_F0735_INVENTORY_GRAPHIC_PC34,
                              DM1_V1_F0735_VIEWPORT_WIDTH_PC34,
                              DM1_V1_F0735_VIEWPORT_HEIGHT_PC34,
                              &fingerprint)) {
        return 0;
    }
    fillViewport(context, box, paletteIndex);
    receipt_write(outReceipt, DM1_V1_F0735_FUNCTION_ID_PC34,
                  inventorySurface->graphicIndex, -1, paletteIndex,
                  box, fingerprint);
    return 1;
}
