#ifndef DM1_V1_LEGACY_GRAPHICS_DAT_H
#define DM1_V1_LEGACY_GRAPHICS_DAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DM1 legacy GRAPHICS.DAT is the pre-0x8001 container used by FM Towns
 * (little-endian) and Amiga (big-endian).  The table stores compressed and
 * expanded record sizes; each record starts with an endian-native width and
 * height followed by the IMAGE2 byte stream. */
int dm1_v1_legacy_graphics_probe(const uint8_t *data, size_t size,
                                 int big_endian);

/* DM1 FM Towns/Amiga v2.x stores non-raster code, sound, text and font
 * records in the same 575-entry table.  Only 0..20 and 22..532 are IMG1/
 * IMG2 bitmaps in the original DM1 data files. */
int dm1_v1_legacy_graphics_is_bitmap_index(uint16_t graphic_index);

int dm1_v1_legacy_graphics_query(const uint8_t *data, size_t size,
                                 int big_endian, uint16_t graphic_index,
                                 uint16_t *out_width, uint16_t *out_height);

int dm1_v1_legacy_graphics_decode(const uint8_t *data, size_t size,
                                  int big_endian, uint16_t graphic_index,
                                  uint8_t *indexed_pixels,
                                  size_t pixel_capacity,
                                  uint16_t *out_width, uint16_t *out_height);

/* Decode one already-bounded original IMG1/IMG2 record. This is shared by
 * the 575-record FM Towns/Amiga container and the 563-record Atari ST
 * DMCSB1 container after its Atari-LZW envelope has been removed. */
int dm1_v1_legacy_graphics_decode_item(const uint8_t *item, size_t item_size,
                                       int big_endian,
                                       uint8_t *indexed_pixels,
                                       size_t pixel_capacity,
                                       uint16_t *out_width,
                                       uint16_t *out_height);

#ifdef __cplusplus
}
#endif

#endif
