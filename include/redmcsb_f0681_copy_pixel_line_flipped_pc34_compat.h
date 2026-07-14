/*
 * Source-faithful PC 3.4 C25_VGA implementation boundary for
 * F0681_CopyPixelLineToScreenWithoutTransparencyFlippedHorizontally.
 */
#ifndef FIRESTAFF_REDMCSB_F0681_COPY_PIXEL_LINE_FLIPPED_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0681_COPY_PIXEL_LINE_FLIPPED_PC34_COMPAT_H

#include "redmcsb_f0680_copy_pixels_to_screen_pc34_compat.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * F0681 reads the selected packed 4bpp source line from right to left and
 * writes it left to right.  Its C25_VGA pixels have the same one-byte A000h
 * aperture representation as F0680: viewport_color_index_offset | nibble.
 * No colour is skipped; zero is an ordinary opaque pixel.
 */
bool redmcsb_f0681_copy_pixel_line_flipped_pc34_compat(
    const uint8_t *source,
    size_t source_byte_count,
    size_t source_pixel_index,
    RedmcsbF0680C25VgaAperturePc34Compat *vga_aperture,
    size_t destination_pixel_index,
    size_t pixel_count,
    uint8_t viewport_color_index_offset);

const char *redmcsb_f0681_copy_pixel_line_flipped_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
