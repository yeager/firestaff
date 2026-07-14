#ifndef FIRESTAFF_VIDEO_F0134_FILL_BITMAP_BOUNDED_PC34_COMPAT_H
#define FIRESTAFF_VIDEO_F0134_FILL_BITMAP_BOUNDED_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB SOURCE/ENGINE/VIDEO.C F0134_VIDEO_FillBitmap (1027-1057).
 * A unit is sixteen 4-bit planar pixels: four big-endian 16-bit plane masks
 * (bits 0 through 3 of color), or eight bytes total.  The source routine
 * assumes a nonzero unit count and enough destination storage; this wrapper
 * checks both preconditions before performing any write. */
int video_f0134_fill_bitmap_bounded_pc34_compat(
    uint8_t *bitmap,
    size_t bitmap_bytes,
    uint8_t color,
    size_t unit_count);

#ifdef __cplusplus
}
#endif

#endif
