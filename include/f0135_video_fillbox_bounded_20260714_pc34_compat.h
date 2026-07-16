#ifndef FIRESTAFF_F0135_VIDEO_FILLBOX_BOUNDED_20260714_PC34_COMPAT_H
#define FIRESTAFF_F0135_VIDEO_FILLBOX_BOUNDED_20260714_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB FILLBOX.C F0135_VIDEO_FillBox packed-bitmap contract:
 * box is { left, right, top, bottom }, with inclusive endpoints. Each
 * byte stores two 4-bit pixels: even X in the high nibble, odd X in the low.
 * The bounded adapter returns the number of pixels changed, or zero when no
 * valid clipped rectangle can be written. */
size_t f0135_video_fillbox_bounded_20260714_pc34_compat(
    uint8_t *bitmap,
    size_t bitmap_size,
    int pixel_width,
    int pixel_height,
    const int16_t box[4],
    uint16_t color);

#ifdef __cplusplus
}
#endif

#endif
