#ifndef CSB_V1_STARTUP_IMG3_DECODE_PC34_COMPAT_H
#define CSB_V1_STARTUP_IMG3_DECODE_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

/*
 * Converts one real, already-decompressed CSB PC 3.4 C001--C005 planar
 * stream into indexed pixels. This is the CSBWin ExpandGraphic format:
 * big-endian width/height followed by its four-plane command stream. The
 * legacy function name is retained at the startup compatibility boundary.
 */
int csb_v1_startup_img3_decode_to_indexed_pc34_compat(
    const uint8_t *graphic,
    size_t graphic_byte_count,
    uint16_t expected_width,
    uint16_t expected_height,
    uint8_t *indexed_pixels,
    size_t indexed_pixel_byte_count);

#endif
