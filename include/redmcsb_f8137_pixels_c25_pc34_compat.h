/* ReDMCSB VIDEODRV.C F8137 C25 VGA aperture fill. */
#ifndef FIRESTAFF_REDMCSB_F8137_PIXELS_C25_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F8137_PIXELS_C25_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define REDMCSB_F8137_C25_APERTURE_BYTES_PC34 64000U

/* A 320x200 M11 indexed framebuffer has this exact C25 aperture layout. */
bool redmcsb_f8137_set_multiple_pixels_c25_pc34_compat(
    uint8_t *aperture, size_t aperture_byte_count, int16_t pixel_index,
    uint8_t color, int16_t pixel_count, uint8_t viewport_color_index_offset);

const char *redmcsb_f8137_pixels_c25_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
