/* ReDMCSB VIDEODRV.C F8155_VIDRV_06_HatchScreenBox, PC 3.4 C25 route. */
#ifndef FIRESTAFF_REDMCSB_F8155_VIDRV_HATCH_SCREEN_BOX_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F8155_VIDRV_HATCH_SCREEN_BOX_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define REDMCSB_F8155_SCREEN_STRIDE_PIXELS_PC34 320U

typedef struct {
    uint8_t *bytes;
    size_t byte_count;
} RedmcsbF8155C25VgaAperturePc34Compat;

/*
 * C25 VGA F8155 clears pixels where (x ^ y) is even in the inclusive box;
 * odd-parity pixels are intentionally left unchanged.
 */
bool redmcsb_f8155_vidrv_hatch_screen_box_pc34_compat(
    RedmcsbF8155C25VgaAperturePc34Compat *aperture,
    int16_t x1, int16_t x2, int16_t y1, int16_t y2);

const char *redmcsb_f8155_vidrv_hatch_screen_box_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
