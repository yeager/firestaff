#ifndef FIRESTAFF_CSB_V1_VIEWPORT_SURFACE_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_SURFACE_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

/*
 * PC 3.4 CSB presents its source-owned F0128 bitmap in the 320x200 game
 * page at (48,33).  The bitmap itself remains the ReDMCSB C224 x C136
 * viewport surface; callers must not treat the top-left of the full page as
 * its origin, because that overwrites status/HUD source material.
 *
 * ReDMCSB: VIEWPORT.C:21-23,62-96 (M091_BITPLANE_SIZE(224,136)).
 * CSBWin: Viewport.cpp screen viewport rectangle (48,33,224,136).
 */
enum {
    CSB_V1_VIEWPORT_SCREEN_X_PC34 = 48,
    CSB_V1_VIEWPORT_SCREEN_Y_PC34 = 33,
    CSB_V1_VIEWPORT_SCREEN_WIDTH_PC34 = 224,
    CSB_V1_VIEWPORT_SCREEN_HEIGHT_PC34 = 136
};

/* Resolve the source viewport sub-surface inside an indexed full screen.
 * Returns zero without changing outputs when the supplied screen cannot
 * contain the complete original rectangle. */
int csb_v1_viewport_screen_surface_pc34(
    uint8_t *screen_pixels, size_t screen_byte_count,
    int screen_width, int screen_height,
    uint8_t **out_viewport_pixels, int *out_stride);

const char *csb_v1_viewport_screen_surface_source_evidence_pc34(void);

#endif
