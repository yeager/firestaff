/* ReDMCSB VIDEODRV.C F8161_VIDRV_09_BlitViewPort, PC 3.4 C25 route. */
#ifndef FIRESTAFF_REDMCSB_F8161_VIDRV_BLIT_VIEWPORT_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F8161_VIDRV_BLIT_VIEWPORT_PC34_COMPAT_H

#include "redmcsb_f8151_vidrv_blit_pc34_compat.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define REDMCSB_F8161_VIEWPORT_SOURCE_WIDTH_PC34 224
#define REDMCSB_F8161_VIEWPORT_DESTINATION_WIDTH_PC34 320
#define REDMCSB_F8161_VIEWPORT_COLOR_OFFSET_PC34 0x10U

/*
 * C25 F8161 sets G8177 to 0x10 around one opaque, unflipped F8151 blit.
 * The adapter carries that otherwise-global state as F8151's explicit
 * aperture colour offset, so no host-global framebuffer state is invented.
 */
bool redmcsb_f8161_vidrv_blit_viewport_pc34_compat(
    const uint8_t *bitmap, size_t bitmap_byte_count,
    RedmcsbF0680C25VgaAperturePc34Compat *aperture,
    const RedmcsbF8151BoxPc34Compat *box);

const char *redmcsb_f8161_vidrv_blit_viewport_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
