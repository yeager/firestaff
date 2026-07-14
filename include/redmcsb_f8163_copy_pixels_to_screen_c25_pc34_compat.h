/* ReDMCSB VIDEODRV.C F8163 C25 source-bitmap aperture transfer. */
#ifndef FIRESTAFF_REDMCSB_F8163_COPY_PIXELS_TO_SCREEN_C25_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F8163_COPY_PIXELS_TO_SCREEN_C25_PC34_COMPAT_H

#include "redmcsb_f0680_copy_pixels_to_screen_pc34_compat.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C25 F8163 assigns G2159 to bitmap, then delegates the exact requested span
 * to F0680. The source bitmap remains packed 4bpp; aperture pixels are bytes.
 */
bool redmcsb_f8163_copy_pixels_to_screen_c25_pc34_compat(
    const uint8_t *bitmap, size_t bitmap_byte_count,
    uint16_t source_pixel_index, uint16_t destination_pixel_index,
    uint16_t pixel_count,
    RedmcsbF0680C25VgaAperturePc34Compat *aperture,
    uint8_t viewport_color_index_offset);

const char *redmcsb_f8163_copy_pixels_to_screen_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
