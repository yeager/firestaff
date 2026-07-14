#ifndef FIRESTAFF_REDMCSB_F0550_VIDEO_FILL_SCREEN_BOX_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0550_VIDEO_FILL_SCREEN_BOX_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB VIDEO.C F0550_VIDEO_FillScreenBox. The bitmap consists of four
 * big-endian 16-bit planes per 16-pixel group (eight bytes). `box` is an
 * inclusive {left, right, top, bottom} box of int16_t values, or four
 * unsigned bytes when `use_byte_box_coordinates` is true. Bit 15 of color
 * selects the source's alternating screen-space shade pattern.
 *
 * Returns false, without modifying bitmap, when the bitmap layout, bounds,
 * or coordinates are invalid. */
bool F0550_VIDEO_FillScreenBox_PC34(
    uint8_t *bitmap,
    size_t bitmap_size,
    size_t byte_width,
    size_t pixel_height,
    const void *box,
    bool use_byte_box_coordinates,
    uint16_t color);

#ifdef __cplusplus
}
#endif

#endif
