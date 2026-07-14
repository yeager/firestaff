/* ReDMCSB VIDEODRV.C F8152_VIDRV_01_FillBox, PC 3.4 C25_VGA route. */
#ifndef FIRESTAFF_REDMCSB_F8152_VIDRV_FILL_BOX_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F8152_VIDRV_FILL_BOX_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define REDMCSB_F8152_SCREEN_STRIDE_PIXELS_PC34 320U

typedef struct {
    uint8_t *bytes;
    size_t byte_count;
} RedmcsbF8152C25VgaAperturePc34Compat;

typedef struct {
    int16_t left;
    int16_t right;
    int16_t top;
    int16_t bottom;
} RedmcsbF8152BoxPc34Compat;

/*
 * Fills an inclusive source box in the C25 byte-per-pixel A000h aperture.
 * The original invokes F8137 once per row, at top*320+left, and F8137 writes
 * viewport_color_index_offset | color to every selected aperture byte.
 */
bool redmcsb_f8152_vidrv_fill_box_pc34_compat(
    RedmcsbF8152C25VgaAperturePc34Compat *aperture,
    const RedmcsbF8152BoxPc34Compat *box,
    uint8_t color, uint8_t viewport_color_index_offset);

const char *redmcsb_f8152_vidrv_fill_box_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
