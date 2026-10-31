#ifndef DM1_V1_IIGS_GRAPHICS_DAT_H
#define DM1_V1_IIGS_GRAPHICS_DAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Structural access to the original 575-record little-endian GRAPHICS.GAME
 * bank. This is not an edition/identity or runtime admission gate. */
int dm1_v1_iigs_graphics_probe(const uint8_t *data, size_t size);
int dm1_v1_iigs_graphics_read_raw(const uint8_t *data, size_t size,
    uint16_t index, uint8_t *out, size_t capacity, size_t *length);

/* Raster records are 0..532 except unused font record 12; record 21 is a
 * native IIGS bitmap. Query returns native width and height. Decode converts
 * F0466's packed 4-bit bitmap into row-major palette indices (one byte per
 * pixel), without selecting a palette. Returns 1 on success, 0 on error.
 * Output dimensions are zero on failure; pixels may be partial on failure. */
int dm1_v1_iigs_graphics_query(const uint8_t *data, size_t size,
    uint16_t index, uint16_t *out_width, uint16_t *out_height);
int dm1_v1_iigs_graphics_decode(const uint8_t *data, size_t size,
    uint16_t index, uint8_t *indexed_pixels, size_t pixel_capacity,
    uint16_t *out_width, uint16_t *out_height);

/* Decode a bounded native record including its little-endian dimensions.
 * The caller owns record identity/type when using this low-level API. */
int dm1_v1_iigs_graphics_decode_item(const uint8_t *item, size_t item_size,
    uint8_t *indexed_pixels, size_t pixel_capacity,
    uint16_t *out_width, uint16_t *out_height);

/* Extract one 16x16 object icon from a decoded native IIGS 256x32 strip.
 * relative_icon_index is local to a ReDMCSB F0036 seven-bank selection
 * (0..31); output is row-major palette indices. */
int dm1_v1_iigs_graphics_extract_icon(const uint8_t *strip_pixels,
    size_t strip_pixel_count, unsigned relative_icon_index,
    uint8_t *icon_pixels, size_t icon_pixel_capacity);

#ifdef __cplusplus
}
#endif
#endif
