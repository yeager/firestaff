#ifndef CSB_V1_STARTUP_IMG3_DECODE_PC34_COMPAT_H
#define CSB_V1_STARTUP_IMG3_DECODE_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

/*
 * Converts one real, already-decompressed PC 3.4 IMG3 record into indexed
 * pixels for the CSB startup surface. The GRAPHICS.DAT header owns the
 * expected dimensions. A non-IMG3 or malformed record is rejected; no visual
 * substitute is produced.
 */
int csb_v1_startup_img3_decode_to_indexed_pc34_compat(
    const uint8_t *graphic,
    size_t graphic_byte_count,
    uint16_t expected_width,
    uint16_t expected_height,
    uint8_t *indexed_pixels,
    size_t indexed_pixel_byte_count);

#endif
