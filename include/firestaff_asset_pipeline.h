
#ifndef FIRESTAFF_ASSET_PIPELINE_H
#define FIRESTAFF_ASSET_PIPELINE_H

#include <stdint.h>

/* Firestaff Asset Pipeline — load original game data files.
 * Supports DM1, CSB, DM2 data formats.
 *
 * Pipeline: file → decompress → cache → render-ready */

typedef struct {
    uint8_t *graphics_data;
    int graphics_size;
    uint8_t *dungeon_data;
    int dungeon_size;
    uint32_t palette[256];
    int loaded;
} FS_AssetBundle;

int fs_assets_load_dm1(FS_AssetBundle *bundle, const char *data_dir);
int fs_assets_load_csb(FS_AssetBundle *bundle, const char *data_dir);
int fs_assets_load_dm2(FS_AssetBundle *bundle, const char *data_dir);
void fs_assets_free(FS_AssetBundle *bundle);

/* VGA palette: 6-bit DAC values → 8-bit RGBA */
void fs_assets_expand_vga_palette(const uint8_t *vga_6bit, uint32_t *rgba_out, int count);

#endif

