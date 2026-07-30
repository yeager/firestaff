#ifndef FIRESTAFF_CSB_V1_CSBWIN_PLANAR_BITMAP_H
#define FIRESTAFF_CSB_V1_CSBWIN_PLANAR_BITMAP_H

#include <stddef.h>
#include <stdint.h>

/* CSBWin Graphics.cpp:TAG0088b2 consumes Atari ST four-plane words: every
 * 16-pixel group occupies eight bytes, one big-endian word per colour plane.
 * This restores that source layout from Firestaff's decoder-owned indexed
 * raster before the original blitter command projection is ported. */
typedef struct {
    const uint8_t *bytes;
    uint16_t width;
    uint16_t height;
    uint16_t byte_stride;
} CSB_V1_CSBWinPlanarBitmap;

int csb_v1_csbwin_planar_bitmap_pack_indexed(
    const uint8_t *indexed_pixels, uint16_t width, uint16_t height,
    uint8_t **out_bytes, size_t *out_byte_count);

int csb_v1_csbwin_planar_bitmap_pixel_at(
    const CSB_V1_CSBWinPlanarBitmap *bitmap, uint16_t x, uint16_t y,
    uint8_t *out_color);

/* Performs the source-owned indexed equivalent of a clipped TAG0088b2 copy.
 * The packed four-plane addressing, source bounds and transparency are exact;
 * planar destination masks and the individual F0128 projection commands are
 * deliberately not claimed by this helper. */
int csb_v1_csbwin_planar_bitmap_blit_indexed(
    const CSB_V1_CSBWinPlanarBitmap *source,
    int source_x, int source_y, int width, int height,
    uint8_t *destination, int destination_width, int destination_height,
    int destination_stride, int destination_x, int destination_y,
    int transparent_color);

#endif
