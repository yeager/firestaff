
#include "firestaff_asset_pipeline.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int load_file(const char *path, uint8_t **out, int *out_size) {
    FILE *f = fopen(path, "rb");
    long size;
    if (!f) return -1;
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    *out = (uint8_t *)malloc(size);
    if (!*out) { fclose(f); return -1; }
    *out_size = (int)fread(*out, 1, size, f);
    fclose(f);
    return 0;
}

int fs_assets_load_dm1(FS_AssetBundle *bundle, const char *data_dir) {
    char path[512];
    if (!bundle || !data_dir) return -1;
    memset(bundle, 0, sizeof(*bundle));

    snprintf(path, sizeof(path), "%s/GRAPHICS.DAT", data_dir);
    if (load_file(path, &bundle->graphics_data, &bundle->graphics_size) < 0) {
        /* Try lowercase */
        snprintf(path, sizeof(path), "%s/graphics.dat", data_dir);
        if (load_file(path, &bundle->graphics_data, &bundle->graphics_size) < 0)
            return -1;
    }

    snprintf(path, sizeof(path), "%s/DUNGEON.DAT", data_dir);
    if (load_file(path, &bundle->dungeon_data, &bundle->dungeon_size) < 0) {
        snprintf(path, sizeof(path), "%s/dungeon.dat", data_dir);
        load_file(path, &bundle->dungeon_data, &bundle->dungeon_size);
    }

    bundle->loaded = 1;
    return 0;
}

int fs_assets_load_csb(FS_AssetBundle *bundle, const char *data_dir) {
    return fs_assets_load_dm1(bundle, data_dir); /* Same DAT format */
}

int fs_assets_load_dm2(FS_AssetBundle *bundle, const char *data_dir) {
    return fs_assets_load_dm1(bundle, data_dir); /* Similar format */
}

void fs_assets_free(FS_AssetBundle *bundle) {
    if (!bundle) return;
    free(bundle->graphics_data); bundle->graphics_data = NULL;
    free(bundle->dungeon_data); bundle->dungeon_data = NULL;
    bundle->loaded = 0;
}

void fs_assets_expand_vga_palette(const uint8_t *vga_6bit, uint32_t *rgba_out, int count) {
    int i;
    if (!vga_6bit || !rgba_out) return;
    for (i = 0; i < count; i++) {
        /* VGA 6-bit (0-63) → 8-bit (0-255): multiply by 4 + rounding */
        uint8_t r = (vga_6bit[i * 3 + 0] << 2) | (vga_6bit[i * 3 + 0] >> 4);
        uint8_t g = (vga_6bit[i * 3 + 1] << 2) | (vga_6bit[i * 3 + 1] >> 4);
        uint8_t b = (vga_6bit[i * 3 + 2] << 2) | (vga_6bit[i * 3 + 2] >> 4);
        rgba_out[i] = 0xFF000000 | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
    }
}

