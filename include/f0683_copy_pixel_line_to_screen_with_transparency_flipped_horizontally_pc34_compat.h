/*
 * Bounded caller-owned adapter for ReDMCSB IMAGE3.C:612-829
 * F0683_CopyPixelLineToScreenWithTransparencyFlippedHorizontally.
 */
#ifndef FIRESTAFF_F0683_COPY_PIXEL_LINE_TO_SCREEN_WITH_TRANSPARENCY_FLIPPED_HORIZONTALLY_PC34_COMPAT_H
#define FIRESTAFF_F0683_COPY_PIXEL_LINE_TO_SCREEN_WITH_TRANSPARENCY_FLIPPED_HORIZONTALLY_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Copies pixel_count packed 4-bit pixels from source to destination in
 * reverse source order. source_pixel_index and destination_pixel_index name
 * nibble positions: pixel zero is the high nibble of byte zero. A source
 * nibble equal to transparent_color leaves the corresponding destination
 * nibble unchanged. Both bitmaps remain packed 4bpp; this adapter does not
 * expand them to a byte-per-pixel raster.
 *
 * Returns 1 on success. Invalid pointers, incomplete ranges, arithmetic
 * overflow, or transparent colors outside the source function's 0..15 table
 * domain return 0 without modifying destination.
 */
int f0683_copy_pixel_line_to_screen_with_transparency_flipped_horizontally_pc34_compat(
    const uint8_t *source,
    size_t source_bytes,
    size_t source_pixel_index,
    uint8_t *destination,
    size_t destination_bytes,
    size_t destination_pixel_index,
    size_t pixel_count,
    uint8_t transparent_color);

const char *f0683_copy_pixel_line_to_screen_with_transparency_flipped_horizontally_pc34_compat_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif
