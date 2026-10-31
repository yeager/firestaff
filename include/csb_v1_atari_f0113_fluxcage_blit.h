#ifndef CSB_V1_ATARI_F0113_FLUXCAGE_BLIT_H
#define CSB_V1_ATARI_F0113_FLUXCAGE_BLIT_H

#include "csb_v1_atari_f0113_fluxcage_plan.h"

/* ReDMCSB MEDIA020 F0113 + MEDIA008 F0133/F0132 composition on decoded
 * Atari indexed pixels. `last_unit` is F0133's exclusive restart sentinel;
 * mask pixels must decode to the Atari source's 0/15 plane mask. */
int csb_v1_atari_f0113_blit_fluxcage(
    const CSB_V1_AtariF0113FluxcagePlan *plan,
    const uint8_t *field_pixels,
    int field_width,
    int field_height,
    const uint8_t *mask_pixels,
    int mask_width,
    int mask_height,
    int last_unit,
    int first_unit,
    uint8_t *framebuffer,
    int framebuffer_width,
    int framebuffer_height,
    int viewport_x,
    int viewport_y);

#endif
