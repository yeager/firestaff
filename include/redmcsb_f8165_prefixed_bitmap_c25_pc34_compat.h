/* ReDMCSB VIDEODRV.C F8165 prefixed aperture bitmap, PC 3.4 C25 route. */
#ifndef FIRESTAFF_REDMCSB_F8165_PREFIXED_BITMAP_C25_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F8165_PREFIXED_BITMAP_C25_PC34_COMPAT_H

#include "redmcsb_f0680_copy_pixels_to_screen_pc34_compat.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define REDMCSB_F8165_C25_HEADER_BYTES_PC34 6U
#define REDMCSB_F8165_C25_SCREEN_STRIDE_PC34 320U

/*
 * Operation zero writes `width * height + 6` through result_byte_count. Any
 * nonzero operation writes the C25 three-word little-endian prefix (width,
 * height, source offset), followed by raw byte-per-pixel A000h aperture rows.
 * The source's capture-path return value is uninitialised, so result_byte_count
 * is deliberately not written for that route.
 */
bool redmcsb_f8165_prefixed_bitmap_c25_pc34_compat(
    const RedmcsbF0680C25VgaAperturePc34Compat *aperture,
    uint16_t x, int16_t y, int16_t width, uint16_t height, int16_t operation,
    uint8_t *prefixed_bitmap, size_t prefixed_bitmap_byte_count,
    size_t *result_byte_count);

const char *redmcsb_f8165_prefixed_bitmap_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
