#include "asset_loader_m11.h"
#include "dm1_v1_champion_panel_food_water_status_box_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>

static const char *graphics_dat_path(void)
{
    const char *configured = getenv("FIRESTAFF_DM1_GRAPHICS_DAT");
    const char *home = getenv("HOME");
    static char path[1024];
    FILE *file;

    if (configured && configured[0] != '\0') {
        return configured;
    }
    if (!home || home[0] == '\0') {
        return NULL;
    }
    snprintf(path, sizeof(path), "%s/.firestaff/data/dm1/GRAPHICS.DAT", home);
    file = fopen(path, "rb");
    if (!file) {
        return NULL;
    }
    fclose(file);
    return path;
}

int main(void)
{
    const char *path = graphics_dat_path();
    M11_AssetLoader loader;
    const M11_AssetSlot *panel;
    const M11_AssetSlot *food;
    const M11_AssetSlot *water;
    dm1_v1_champion_panel_food_water_material_surface_pc34_t panel_material;
    dm1_v1_champion_panel_food_water_material_surface_pc34_t food_material;
    dm1_v1_champion_panel_food_water_material_surface_pc34_t water_material;
    dm1_v1_champion_panel_food_water_material_receipt_pc34_t receipt;

    if (!path) {
        puts("SKIP: no local DM1 GRAPHICS.DAT");
        return 0;
    }
    if (!M11_AssetLoader_Init(&loader, path)) {
        fputs("FAIL: unable to decode local DM1 GRAPHICS.DAT\n", stderr);
        return 1;
    }
    panel = M11_AssetLoader_Load(&loader, 20u);
    food = M11_AssetLoader_Load(&loader, 30u);
    water = M11_AssetLoader_Load(&loader, 31u);
    panel_material = (dm1_v1_champion_panel_food_water_material_surface_pc34_t){
        1, 20, panel ? (int)panel->width : 0, panel ? (int)panel->height : 0,
        panel ? panel->pixels : NULL};
    food_material = (dm1_v1_champion_panel_food_water_material_surface_pc34_t){
        1, 30, food ? (int)food->width : 0, food ? (int)food->height : 0,
        food ? food->pixels : NULL};
    water_material = (dm1_v1_champion_panel_food_water_material_surface_pc34_t){
        1, 31, water ? (int)water->width : 0, water ? (int)water->height : 0,
        water ? water->pixels : NULL};

    if (!dm1_v1_champion_panel_food_water_material_admit_pc34(
            &panel_material, &food_material, &water_material, &receipt) ||
        receipt.f0134_status_fill_color != 12 ||
        receipt.panel_pixel_fingerprint == 0u ||
        receipt.food_label_pixel_fingerprint == 0u ||
        receipt.water_label_pixel_fingerprint == 0u) {
        M11_AssetLoader_Shutdown(&loader);
        fputs("FAIL: F0134/F0135 did not admit local original material\n", stderr);
        return 1;
    }
    M11_AssetLoader_Shutdown(&loader);
    puts("PASS: DM1 F0134/F0135 real GRAPHICS.DAT material receipt");
    return 0;
}
