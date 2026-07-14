/* ReDMCSB VIDEODRV.C F8167/F8168 mouse-pointer aperture save/restore. */
#ifndef FIRESTAFF_REDMCSB_F8167_F8168_MOUSE_POINTER_C25_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F8167_F8168_MOUSE_POINTER_C25_PC34_COMPAT_H

#include "redmcsb_f8166_blit_prefixed_bitmap_c25_pc34_compat.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define REDMCSB_F8167_MOUSE_POINTER_MAX_WIDTH_PC34 18U
#define REDMCSB_F8167_MOUSE_POINTER_MAX_HEIGHT_PC34 18U
#define REDMCSB_F8167_SCREEN_WIDTH_PC34 320U
#define REDMCSB_F8167_SCREEN_HEIGHT_PC34 200U

/* F8167 captures the source-clamped cursor rectangle into G8133's F8165 form. */
bool redmcsb_f8167_capture_mouse_pointer_c25_pc34_compat(
    const RedmcsbF0680C25VgaAperturePc34Compat *aperture,
    int16_t x, int16_t y, uint8_t *prefixed_bitmap,
    size_t prefixed_bitmap_byte_count);

/* F8168 restores precisely the F8167/G8133 F8165-form bitmap. */
bool redmcsb_f8168_restore_mouse_pointer_c25_pc34_compat(
    const uint8_t *prefixed_bitmap, size_t prefixed_bitmap_byte_count,
    RedmcsbF0680C25VgaAperturePc34Compat *aperture);

const char *redmcsb_f8167_f8168_mouse_pointer_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
