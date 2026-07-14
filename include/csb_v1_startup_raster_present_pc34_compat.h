#ifndef CSB_V1_STARTUP_RASTER_PRESENT_PC34_COMPAT_H
#define CSB_V1_STARTUP_RASTER_PRESENT_PC34_COMPAT_H

#include "redmcsb_f0693_wait_vertical_blank_pc34_compat.h"

#include <stddef.h>
#include <stdint.h>

typedef struct {
    const uint8_t *indexed_pixels;
    uint16_t width;
    uint16_t height;
    int valid;
    int real_asset_matched;
} csb_v1_startup_real_raster_pc34_compat;

/* Converts only an authenticated 320x200 indexed startup raster to the PC34
 * packed-4bpp host page. F0692 clears the caller-owned page first and F0693
 * synchronizes presentation; unsupported pixels or a missing VBlank route
 * fail without a fallback surface. */
int csb_v1_startup_present_real_raster_pc34_compat(
    const csb_v1_startup_real_raster_pc34_compat *raster,
    uint8_t *packed_page,
    size_t packed_page_byte_count,
    ReDMCSBF0693WaitVerticalBlankPc34Compat *vblank_gate);

#endif
