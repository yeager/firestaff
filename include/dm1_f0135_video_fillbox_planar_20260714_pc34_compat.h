#ifndef FIRESTAFF_DM1_F0135_VIDEO_FILLBOX_PLANAR_20260714_PC34_COMPAT_H
#define FIRESTAFF_DM1_F0135_VIDEO_FILLBOX_PLANAR_20260714_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB Toolchains/Common/Source/FILLBOX.C F0135, Atari ST path.
 *
 * The bitmap is four 1-bit planes interleaved by 16-pixel word group:
 * plane 0 through plane 3 are big-endian words at offsets 0, 2, 4, and 6.
 * row_bytes is the original byte-width (eight bytes per 16 pixels). box is
 * {left, right, top, bottom} with inclusive endpoints. Bit 15 in color
 * preserves alternating pixels in a checkerboard phase rooted at box[0, 2].
 *
 * Returns 1 after a complete fill, or 0 if an argument, layout, size, or
 * rectangle bound is invalid. Rejected calls do not modify bitmap.
 */
int dm1_f0135_video_fillbox_planar_20260714_pc34_compat(
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
