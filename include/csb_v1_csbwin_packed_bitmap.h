#ifndef FIRESTAFF_CSB_V1_CSBWIN_PACKED_BITMAP_H
#define FIRESTAFF_CSB_V1_CSBWIN_PACKED_BITMAP_H

#include <stddef.h>
#include <stdint.h>

/* CSBWin Bitmaps.cpp:187 and Graphics.cpp:TAG0088b2 operate on 4-bit source
 * pixels packed high-nibble first.  This boundary deliberately preserves the
 * original byte stride after the DMCSB1 decoder has expanded an item to a
 * convenient indexed raster. */
typedef struct {
    const uint8_t *bytes;
    uint16_t width;
    uint16_t height;
    uint16_t byte_stride;
} CSB_V1_CSBWinPackedBitmap;

/* Packs a decoded 4-bit indexed raster into CSBWin's high-nibble-first rows.
 * The caller owns the returned buffer and releases it with free(). */
int csb_v1_csbwin_packed_bitmap_pack_indexed(
    const uint8_t *indexed_pixels, uint16_t width, uint16_t height,
    uint8_t **out_bytes, size_t *out_byte_count);

/* Reads one source pixel without inventing padding pixels. */
int csb_v1_csbwin_packed_bitmap_pixel_at(
    const CSB_V1_CSBWinPackedBitmap *bitmap, uint16_t x, uint16_t y,
    uint8_t *out_color);

/* Copies a source rectangle to an indexed destination.  The source rectangle
 * is clipped to the real packed bitmap, the destination to its supplied
 * extent, and the transparent colour leaves destination bytes untouched.
 * This is the source-byte ownership needed before porting CSBWin's planar
 * TAG0088b2 projection commands to M11. */
int csb_v1_csbwin_packed_bitmap_blit_indexed(
    const CSB_V1_CSBWinPackedBitmap *source,
    int source_x, int source_y, int width, int height,
    uint8_t *destination, int destination_width, int destination_height,
    int destination_stride, int destination_x, int destination_y,
    int transparent_color);

#endif
