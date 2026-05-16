
#ifndef FIRESTAFF_GRAPHICS_DAT_READER_H
#define FIRESTAFF_GRAPHICS_DAT_READER_H
#include <stdint.h>

/* GRAPHICS.DAT reader — extract bitmaps from DM1 packed format.
 * Source: ReDMCSB MEMORY.C F0474_MEMORY_LoadGraphic_CPSDF
 *         ReDMCSB MEMORY.C F0490_MEMORY_LoadDecompressAndExpandGraphic
 *
 * Format: header (graphic count, offsets) + packed bitmap data.
 * Each graphic: dimensions + RLE/packed pixel data. */

#define FS_GFX_MAX_GRAPHICS 800

typedef struct {
    int width, height;
    int offset;
    int compressed_size;
} FS_GraphicEntry;

typedef struct {
    const uint8_t *raw_data;
    int raw_size;
    FS_GraphicEntry entries[FS_GFX_MAX_GRAPHICS];
    int graphic_count;
    int loaded;
} FS_GraphicsDat;

int fs_gfx_load(FS_GraphicsDat *gfx, const uint8_t *data, int size);
int fs_gfx_get_bitmap(const FS_GraphicsDat *gfx, int index,
    uint8_t *out_pixels, int max_size, int *out_w, int *out_h);
int fs_gfx_get_palette(const FS_GraphicsDat *gfx, uint32_t *rgba_out);

#endif

