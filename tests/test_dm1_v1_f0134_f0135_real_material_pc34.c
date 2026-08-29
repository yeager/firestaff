#include "asset_loader_m11.h"
#include "dm1_v1_champion_panel_food_water_status_box_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>

static const char *graphics_dat_path(void)
{
    const char *root = getenv("FIRESTAFF_DM1_DATA_DIR");
    static char path[2048];
    if (!root || root[0] == '\0') return NULL;
    snprintf(path, sizeof(path), "%s/GRAPHICS.DAT", root);
    return path;
}

int main(void)
{
    const char *path = graphics_dat_path();
    M11_AssetLoader loader;
    const M11_AssetSlot *panel;
    const M11_AssetSlot *food;
    const M11_AssetSlot *water;
    dm1_v1_champion_panel_food_water_material_receipt_pc34_t receipt;

    if (!path) {
        puts("SKIP: FIRESTAFF_DM1_DATA_DIR is not selected");
        return 77;
    }
    if (!M11_AssetLoader_Init(&loader, path)) {
        fputs("FAIL: unable to decode local DM1 GRAPHICS.DAT\n", stderr);
        return 1;
    }
    panel = M11_AssetLoader_Load(&loader, 20u);
    food = M11_AssetLoader_Load(&loader, 30u);
    water = M11_AssetLoader_Load(&loader, 31u);
    if (!dm1_v1_champion_panel_food_water_material_admit_graphics_slots_pc34(
            panel, food, water, &receipt) ||
        !receipt.graphics_dat_loader_ready ||
        !receipt.indexed_vga4_format_valid ||
        receipt.rejected_invalid_pixel_format ||
        receipt.f0134_status_fill_color != 12 ||
        receipt.panel_pixel_fingerprint == 0u ||
        receipt.food_label_pixel_fingerprint == 0u ||
        receipt.water_label_pixel_fingerprint == 0u) {
        M11_AssetLoader_Shutdown(&loader);
        fputs("FAIL: F0134/F0135 did not admit local original material\n", stderr);
        return 1;
    }
    M11_AssetLoader_Shutdown(&loader);
    puts("PASS: DM1 F0134/F0135 production GRAPHICS.DAT material receipt");
    return 0;
}
