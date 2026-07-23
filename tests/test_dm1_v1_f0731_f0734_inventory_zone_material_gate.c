#include "asset_loader_m11.h"
#include "dm1_v1_f0731_f0734_inventory_zone_material_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct TestDraw {
    int calls;
    int left;
    int right;
    int top;
    int bottom;
    int color;
} TestDraw;

static void invert_box(void* context, int left, int right, int top, int bottom)
{
    TestDraw* draw = (TestDraw*)context;
    if (!draw) return;
    ++draw->calls;
    draw->left = left;
    draw->right = right;
    draw->top = top;
    draw->bottom = bottom;
}

static void fill_box(void* context, int left, int right, int top, int bottom,
                     int color)
{
    TestDraw* draw = (TestDraw*)context;
    if (!draw) return;
    ++draw->calls;
    draw->left = left;
    draw->right = right;
    draw->top = top;
    draw->bottom = bottom;
    draw->color = color;
}

static int load_graphics(M11_AssetLoader* loader, char* path, size_t pathSize)
{
    const char* root = getenv("FIRESTAFF_DM1_DATA_DIR");
    const char* home = getenv("HOME");

    if (root && root[0]) snprintf(path, pathSize, "%s/GRAPHICS.DAT", root);
    else if (home && home[0]) {
        snprintf(path, pathSize, "%s/.firestaff/data/dm1/GRAPHICS.DAT", home);
    } else return 0;
    return M11_AssetLoader_Init(loader, path);
}

static void surface_from_slot(const M11_AssetSlot* slot,
                              DM1_V1_F0731F0734SurfacePc34* surface)
{
    memset(surface, 0, sizeof(*surface));
    surface->graphicsDatOwned = 1;
    surface->graphicIndex = (int)slot->graphicIndex;
    surface->width = (int)slot->width;
    surface->height = (int)slot->height;
    surface->indexedPixelCount = surface->width * surface->height;
    surface->indexedPixels = slot->pixels;
    surface->indexedPixelsFNV1a = dm1_v1_f0731_f0734_fnv1a_pc34(
        surface->indexedPixels, surface->indexedPixelCount);
}

int main(void)
{
    M11_AssetLoader loader;
    const M11_AssetSlot* spell;
    const M11_AssetSlot* inventory;
    DM1_V1_F0731F0734SurfacePc34 spellSurface;
    DM1_V1_F0731F0734SurfacePc34 inventorySurface;
    DM1_V1_F0731F0734ZonePc34 spellZone = { 224, 233, 277, 42, 49 };
    DM1_V1_F0731F0734ZonePc34 iconZone = { 562, 174, 218, 2, 12 };
    DM1_V1_F0731F0734ReceiptPc34 receipt;
    TestDraw draw;
    char path[1024];

    memset(&loader, 0, sizeof(loader));
    if (!load_graphics(&loader, path, sizeof(path))) {
        if (getenv("FIRESTAFF_DM1_DATA_DIR")) return 1;
        puts("SKIP: PC34 GRAPHICS.DAT not installed");
        return 0;
    }
    spell = M11_AssetLoader_Load(&loader, 9);
    inventory = M11_AssetLoader_Load(&loader, 17);
    if (!spell || !inventory || !spell->loaded || !inventory->loaded ||
        !spell->pixels || !inventory->pixels) goto fail;
    surface_from_slot(spell, &spellSurface);
    surface_from_slot(inventory, &inventorySurface);
    memset(&draw, 0, sizeof(draw));
    if (!dm1_v1_f0731_invert_spell_caster_zone_pc34(
            &spellSurface, &spellZone, invert_box, &draw, &receipt) ||
        !receipt.valid || receipt.graphicIndex != 9 || receipt.zoneIndex != 224 ||
        draw.calls != 1 || draw.left != 233 || draw.right != 277 ||
        draw.top != 42 || draw.bottom != 49) goto fail;
    if (!dm1_v1_f0734_clear_inventory_icon_zone_pc34(
            &inventorySurface, &iconZone, fill_box, &draw, &receipt) ||
        !receipt.valid || receipt.graphicIndex != 17 || receipt.zoneIndex != 562 ||
        draw.calls != 2 || draw.color != 12) goto fail;
    ++spellSurface.indexedPixelsFNV1a;
    if (dm1_v1_f0731_invert_spell_caster_zone_pc34(
            &spellSurface, &spellZone, invert_box, &draw, &receipt) ||
        draw.calls != 2) goto fail;
    --spellSurface.indexedPixelsFNV1a;
    iconZone.right = 224;
    if (dm1_v1_f0734_clear_inventory_icon_zone_pc34(
            &inventorySurface, &iconZone, fill_box, &draw, &receipt) ||
        draw.calls != 2) goto fail;
    M11_AssetLoader_Shutdown(&loader);
    puts("ok: real PC34 F0731/F0734 inventory zone material gate");
    return 0;
fail:
    M11_AssetLoader_Shutdown(&loader);
    return 1;
}
