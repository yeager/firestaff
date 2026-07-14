/* ReDMCSB VIDEODRV.C F8139 C25 packed-pixel aperture transfer. */
#ifndef FIRESTAFF_REDMCSB_F8139_COPY_PACKED_PIXELS_C25_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F8139_COPY_PACKED_PIXELS_C25_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Expands pixel_count packed 4bpp source pixels into byte-addressed C25 VGA
 * aperture pixels. The source's odd/even nibble order and bytewise OR with
 * G8177's viewport colour-index offset are retained.
 */
bool redmcsb_f8139_copy_packed_pixels_c25_pc34_compat(
    const uint8_t *source, size_t source_byte_count,
    uint16_t source_pixel_index, uint8_t *aperture,
    size_t aperture_byte_count, uint16_t destination_pixel_index,
    uint16_t pixel_count, uint8_t viewport_color_index_offset);

const char *redmcsb_f8139_copy_packed_pixels_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
