/* ReDMCSB VIDEODRV.C F8213 C25 aperture single-pixel write. */
#ifndef FIRESTAFF_REDMCSB_F8213_SET_SINGLE_PIXEL_C25_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F8213_SET_SINGLE_PIXEL_C25_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Writes viewport_color_index_offset OR color at the supplied aperture index. */
bool redmcsb_f8213_set_single_pixel_c25_pc34_compat(
    uint8_t *aperture, size_t aperture_byte_count, int16_t pixel_index,
    uint8_t color, uint8_t viewport_color_index_offset);

const char *redmcsb_f8213_set_single_pixel_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
