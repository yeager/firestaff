#include "asset_loader_m11.h"
#include "dm1_v1_f0732_f0735_fill_material_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct TestFill {
    int calls;
    DM1_V1_F0732F0735BoxPc34 box;
    int paletteIndex;
} TestFill;

static void fill_surface(void* context, const DM1_V1_F0732F0735BoxPc34* box,
                         int paletteIndex)
{
    TestFill* fill = (TestFill*)context;
    if (!fill || !box) return;
    ++fill->calls;
    fill->box = *box;
    fill->paletteIndex = paletteIndex;
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
                              DM1_V1_F0732F0735SurfacePc34* surface)
{
    memset(surface, 0, sizeof(*surface));
    surface->graphicsDatOwned = 1;
    surface->graphicIndex = (int)slot->graphicIndex;
    surface->width = (int)slot->width;
    surface->height = (int)slot->height;
    surface->indexedPixelCount = surface->width * surface->height;
    surface->indexedPixels = slot->pixels;
    surface->indexedPixelsFNV1a = dm1_v1_f0732_f0735_fnv1a_pc34(
        surface->indexedPixels, surface->indexedPixelCount);
}

int main(void)
{
    M11_AssetLoader loader;
    const M11_AssetSlot* spell;
    const M11_AssetSlot* inventory;
    DM1_V1_F0732F0735SurfacePc34 spellSurface;
    DM1_V1_F0732F0735SurfacePc34 inventorySurface;
    DM1_V1_F0732F0735ZonePc34 spellZone = { 224, 42, 96, 33 };
    DM1_V1_F0732F0735BoxPc34 spellBox = { 224, 319, 42, 74 };
    DM1_V1_F0732F0735BoxPc34 viewportBox = { 0, 223, 0, 135 };
    DM1_V1_F0732F0735ReceiptPc34 receipt;
    TestFill fill;
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
    memset(&fill, 0, sizeof(fill));
    if (!dm1_v1_f0732_clear_spell_area_pc34(
            &spellSurface, &spellZone, 0, fill_surface, &fill, &receipt) ||
        !receipt.valid || receipt.functionId != 732 || fill.calls != 1 ||
        fill.box.right != 319 || fill.paletteIndex != 0) goto fail;
    if (!dm1_v1_f0733_fill_spell_zone_pc34(
            &spellSurface, 13, &spellBox, 0, fill_surface, &fill, &receipt) ||
        !receipt.valid || receipt.functionId != 733 || fill.calls != 2 ||
        fill.box.bottom != 74) goto fail;
    if (!dm1_v1_f0735_fill_inventory_viewport_pc34(
            &inventorySurface, &viewportBox, 12, fill_surface, &fill, &receipt) ||
        !receipt.valid || receipt.functionId != 735 || fill.calls != 3 ||
        fill.box.right != 223 || fill.box.bottom != 135 ||
        fill.paletteIndex != 12) goto fail;
    ++spellSurface.indexedPixelsFNV1a;
    if (dm1_v1_f0733_fill_spell_zone_pc34(
            &spellSurface, 13, &spellBox, 0, fill_surface, &fill, &receipt) ||
        fill.calls != 3) goto fail;
    --spellSurface.indexedPixelsFNV1a;
    viewportBox.right = 224;
    if (dm1_v1_f0735_fill_inventory_viewport_pc34(
            &inventorySurface, &viewportBox, 12, fill_surface, &fill, &receipt) ||
        fill.calls != 3) goto fail;
    M11_AssetLoader_Shutdown(&loader);
    puts("ok: real PC34 F0732/F0733/F0735 fill material gate");
    return 0;
fail:
    M11_AssetLoader_Shutdown(&loader);
    return 1;
}
