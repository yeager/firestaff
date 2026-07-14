/* ReDMCSB VIDEODRV.C F8216 C25 aperture previous-row copy. */
#ifndef FIRESTAFF_REDMCSB_F8216_COPY_PREVIOUS_ROW_C25_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F8216_COPY_PREVIOUS_ROW_C25_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define REDMCSB_F8216_SCREEN_STRIDE_PC34 320U

/*
 * Copies pixel_count bytes from destination_offset-320 to destination_offset.
 * The original `movs` sequence advances forward, including its observable
 * propagation if a source/destination span overlaps.
 */
bool redmcsb_f8216_copy_previous_row_c25_pc34_compat(
    uint8_t *aperture, size_t aperture_byte_count,
    int16_t destination_offset, int16_t pixel_count);

const char *redmcsb_f8216_copy_previous_row_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
