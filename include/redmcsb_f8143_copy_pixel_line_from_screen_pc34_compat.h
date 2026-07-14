/* ReDMCSB VIDEODRV.C F8143, PC 3.4 C25_VGA route. */
#ifndef FIRESTAFF_REDMCSB_F8143_COPY_PIXEL_LINE_FROM_SCREEN_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F8143_COPY_PIXEL_LINE_FROM_SCREEN_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Reads C25 A000h aperture bytes and packs their low nibbles into a 4bpp
 * bitmap. Packed bitmap pixel zero occupies the high nibble of byte zero.
 */
bool redmcsb_f8143_copy_pixel_line_from_screen_pc34_compat(
    const uint8_t *aperture, size_t aperture_byte_count,
    size_t source_pixel_index, uint8_t *destination,
    size_t destination_byte_count, size_t destination_pixel_index,
    size_t pixel_count);

const char *redmcsb_f8143_copy_pixel_line_from_screen_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
