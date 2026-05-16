
#ifndef FIRESTAFF_BITMAP_EXTRACT_H
#define FIRESTAFF_BITMAP_EXTRACT_H
#include <stdint.h>

/* Extract individual bitmaps from GRAPHICS.DAT.
 *
 * PC34 new format (sig 0x8001):
 *   Header: [sig:2][count:2]
 *   comp_sizes[count]:  uint16 each
 *   decomp_sizes[count]: uint16 each
 *   dims[count]: packed uint32 (width:16 | height:16)
 *   Data: concatenated 4bpp pixel data
 *
 * 4bpp = 2 pixels per byte (high nibble first).
 * Palette applied at render time. */

#define GFX_MAX_BITMAPS 1024

typedef struct {
    int width, height;
    int comp_size;
    int decomp_size;
    int data_offset;    /* offset into raw data */
} FS_BitmapInfo;

typedef struct {
    uint8_t *raw_data;
    int raw_size;
    FS_BitmapInfo bitmaps[GFX_MAX_BITMAPS];
    int bitmap_count;
    uint32_t palette[256];
    int palette_loaded;
} FS_GraphicsAtlas;

/* Load entire GRAPHICS.DAT and parse header */
int fs_gfx_load(FS_GraphicsAtlas *atlas, const char *path);

/* Extract one bitmap to 8-bit indexed buffer (caller allocates w*h bytes) */
int fs_gfx_extract_bitmap(const FS_GraphicsAtlas *atlas, int index,
    uint8_t *out_pixels, int max_size);

/* Get bitmap dimensions */
int fs_gfx_get_size(const FS_GraphicsAtlas *atlas, int index, int *w, int *h);

/* Convert indexed bitmap to RGBA using atlas palette */
void fs_gfx_indexed_to_rgba(const uint8_t *indexed, int w, int h,
    const uint32_t *palette, uint32_t *rgba_out);

/* Free atlas */
void fs_gfx_free(FS_GraphicsAtlas *atlas);

#endif

