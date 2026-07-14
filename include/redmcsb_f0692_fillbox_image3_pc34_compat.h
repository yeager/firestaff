#ifndef FIRESTAFF_REDMCSB_F0692_FILLBOX_IMAGE3_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0692_FILLBOX_IMAGE3_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB IMAGE3.C F0692_FillBox.
 *
 * bitmap is a caller-owned, packed 4bpp raster: each scanline has row_bytes
 * bytes, with the left pixel in the high nibble. box is {left, right, top,
 * bottom}; all endpoints are inclusive. F0692 delegates each scanline to
 * F0685_IMG3_LineColorFilling, whose char color is reduced to its low four
 * bits when it writes packed nibbles. No color flag behavior is modelled.
 *
 * Returns 1 on a complete fill. Invalid arguments or a box outside the
 * caller-provided raster return 0 without modifying the bitmap.
 */
int redmcsb_f0692_fillbox_image3_pc34_compat(
    uint8_t *bitmap,
    size_t bitmap_size,
    size_t row_bytes,
    size_t pixel_height,
    const int16_t box[4],
    uint16_t color);

#ifdef __cplusplus
}
#endif

#endif
