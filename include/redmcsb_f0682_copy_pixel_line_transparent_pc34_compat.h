/*
 * Source-faithful PC 3.4 C25_VGA implementation boundary for
 * F0682_CopyPixelLineToScreenWithTransparency.
 */
#ifndef FIRESTAFF_REDMCSB_F0682_COPY_PIXEL_LINE_TRANSPARENT_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0682_COPY_PIXEL_LINE_TRANSPARENT_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* C25 VGA addresses A000h as one byte per displayed pixel. */
typedef struct {
    uint8_t *bytes;
    size_t byte_count;
} RedmcsbF0682C25VgaAperturePc34Compat;

/*
 * Copies packed source pixels (high nibble first) into the C25 VGA aperture.
 * A source nibble equal to transparent_color leaves its destination byte
 * unchanged. Every other nibble is written as
 * viewport_color_index_offset | source_nibble, exactly as VIDEODRV.C's
 * C25_VGA branch does.
 */
bool redmcsb_f0682_copy_pixel_line_transparent_pc34_compat(
    const uint8_t *source,
    size_t source_byte_count,
    size_t source_pixel_index,
    RedmcsbF0682C25VgaAperturePc34Compat *vga_aperture,
    size_t destination_pixel_index,
    size_t pixel_count,
    uint8_t transparent_color,
    uint8_t viewport_color_index_offset);

const char *redmcsb_f0682_copy_pixel_line_transparent_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
