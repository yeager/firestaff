
#include "firestaff_graphics_dat_reader.h"
#include <string.h>
#include <stdio.h>

/* GRAPHICS.DAT format (PC-34):
 * Offset 0: uint16 graphic_count
 * Offset 2: uint32[graphic_count] offset table
 * Per graphic at offset: uint16 width, uint16 height, packed pixels
 *
 * Source: ReDMCSB MEMORY.C F0467_MEMORY_GetGraphicOffset
 *         ReDMCSB MEMORY.C F0479_MEMORY_ReadGraphicsDatHeader */

static uint16_t r16(const uint8_t *p) { return (uint16_t)p[0] | ((uint16_t)p[1] << 8); }
static uint32_t r32(const uint8_t *p) { return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24); }

int fs_gfx_load(FS_GraphicsDat *gfx, const uint8_t *data, int size) {
    int i, count, header_size;
    if (!gfx || !data || size < 6) return -1;
    memset(gfx, 0, sizeof(*gfx));
    gfx->raw_data = data;
    gfx->raw_size = size;

    count = r16(data);
    if (count > FS_GFX_MAX_GRAPHICS) count = FS_GFX_MAX_GRAPHICS;
    gfx->graphic_count = count;
    header_size = 2 + count * 4;

    for (i = 0; i < count && 2 + i * 4 + 4 <= size; i++) {
        uint32_t off = r32(data + 2 + i * 4);
        gfx->entries[i].offset = (int)off;
        if ((int)off + 4 <= size) {
            gfx->entries[i].width = r16(data + off);
            gfx->entries[i].height = r16(data + off + 2);
        }
    }

    gfx->loaded = 1;
    printf("GRAPHICS.DAT: %d graphics loaded\n", count);
    return count;
}

int fs_gfx_get_bitmap(const FS_GraphicsDat *gfx, int index,
    uint8_t *out_pixels, int max_size, int *out_w, int *out_h)
{
    int w, h, pixel_size, off;
    if (!gfx || !gfx->loaded || index < 0 || index >= gfx->graphic_count)
        return -1;

    off = gfx->entries[index].offset;
    w = gfx->entries[index].width;
    h = gfx->entries[index].height;

    if (out_w) *out_w = w;
    if (out_h) *out_h = h;

    /* Simplified: copy raw pixel data (real format needs decompression) */
    pixel_size = w * h;
    if (pixel_size > max_size || off + 4 + pixel_size > gfx->raw_size)
        return -1;

    if (out_pixels)
        memcpy(out_pixels, gfx->raw_data + off + 4, pixel_size);

    return pixel_size;
}

int fs_gfx_get_palette(const FS_GraphicsDat *gfx, uint32_t *rgba_out) {
    /* DM1 VGA palette is at a fixed offset or embedded.
     * For now, use the standard DM1 palette. */
    (void)gfx;
    if (!rgba_out) return -1;
    /* Standard VGA 16-color + grayscale ramp */
    uint32_t pal16[] = {
        0xFF000000, 0xFF0000AA, 0xFF00AA00, 0xFF00AAAA,
        0xFFAA0000, 0xFFAA00AA, 0xFFAA5500, 0xFFAAAAAA,
        0xFF555555, 0xFF5555FF, 0xFF55FF55, 0xFF55FFFF,
        0xFFFF5555, 0xFFFF55FF, 0xFFFFFF55, 0xFFFFFFFF,
    };
    memcpy(rgba_out, pal16, 16 * 4);
    for (int i = 16; i < 256; i++) {
        uint8_t v = (uint8_t)(i);
        rgba_out[i] = 0xFF000000 | ((uint32_t)v<<16) | ((uint32_t)v<<8) | v;
    }
    return 256;
}

