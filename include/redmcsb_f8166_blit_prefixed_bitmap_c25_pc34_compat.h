/* ReDMCSB VIDEODRV.C F8166 prefixed aperture bitmap playback, PC 3.4 C25. */
#ifndef FIRESTAFF_REDMCSB_F8166_BLIT_PREFIXED_BITMAP_C25_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F8166_BLIT_PREFIXED_BITMAP_C25_PC34_COMPAT_H

#include "redmcsb_f8165_prefixed_bitmap_c25_pc34_compat.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Replays F8165's width, height, offset prefix and raw C25 aperture bytes. */
bool redmcsb_f8166_blit_prefixed_bitmap_c25_pc34_compat(
    const uint8_t *prefixed_bitmap, size_t prefixed_bitmap_byte_count,
    RedmcsbF0680C25VgaAperturePc34Compat *aperture);

const char *redmcsb_f8166_blit_prefixed_bitmap_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
