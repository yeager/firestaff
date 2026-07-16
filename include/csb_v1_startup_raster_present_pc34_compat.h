#ifndef CSB_V1_STARTUP_RASTER_PRESENT_PC34_COMPAT_H
#define CSB_V1_STARTUP_RASTER_PRESENT_PC34_COMPAT_H

#include "csb_v1_boot.h"
#include "redmcsb_f0693_wait_vertical_blank_pc34_compat.h"

#include <stddef.h>
#include <stdint.h>

typedef struct {
    const uint8_t *indexed_pixels;
    const CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 *host_surface_receipt;
    uint16_t width;
    uint16_t height;
    uint32_t source_pixel_hash;
    uint32_t source_route_hash;
    uint32_t source_host_surface_hash;
    int valid;
    int real_asset_matched;
} csb_v1_startup_real_raster_pc34_compat;

/* Converts only an authenticated 320x200 indexed startup raster to the PC34
 * packed-4bpp host page. The raster must be the exact source page named by a
 * CSB startup host-surface receipt, so title/PRESENTS/opening/HUD pages cannot
 * be replayed through a wrapper-owned or stale route. F0692 clears the
 * caller-owned page first and F0693 synchronizes presentation; unsupported
 * pixels or a missing VBlank route fail without a fallback surface. */
int csb_v1_startup_present_real_raster_pc34_compat(
    const csb_v1_startup_real_raster_pc34_compat *raster,
    uint8_t *packed_page,
    size_t packed_page_byte_count,
    ReDMCSBF0693WaitVerticalBlankPc34Compat *vblank_gate);

#endif
