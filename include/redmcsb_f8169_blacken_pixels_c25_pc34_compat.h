/* ReDMCSB VIDEODRV.C F8169 LFSR aperture blackening, PC 3.4 C25 route. */
#ifndef FIRESTAFF_REDMCSB_F8169_BLACKEN_PIXELS_C25_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F8169_BLACKEN_PIXELS_C25_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define REDMCSB_F8169_SCREEN_PIXELS_PC34 64000U

typedef void (*RedmcsbF8169PixelWritePc34Compat)(
    void *context, uint16_t pixel_index, uint8_t aperture_value);

/*
 * Replays F8169's maximal 16-bit LFSR order. Each C25 F8137 single-pixel
 * write stores `viewport_color_index_offset | C00_COLOR_BLACK`, i.e. offset.
 * The optional callback observes the actual source-order write sequence.
 */
bool redmcsb_f8169_blacken_all_pixels_c25_pc34_compat(
    uint8_t *aperture, size_t aperture_byte_count,
    uint8_t viewport_color_index_offset,
    RedmcsbF8169PixelWritePc34Compat on_pixel_write, void *context);

const char *redmcsb_f8169_blacken_pixels_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
