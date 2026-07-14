/* ReDMCSB VIDEODRV.C F8154_VIDRV_05_InvertBox, PC 3.4 C25_VGA route. */
#ifndef FIRESTAFF_REDMCSB_F8154_VIDRV_INVERT_BOX_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F8154_VIDRV_INVERT_BOX_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define REDMCSB_F8154_SCREEN_STRIDE_PIXELS_PC34 320U

typedef struct {
    uint8_t *bytes;
    size_t byte_count;
} RedmcsbF8154C25VgaAperturePc34Compat;

/* Applies the C25 branch's inclusive x1..x2/y1..y2 XOR 0x04 rectangle. */
bool redmcsb_f8154_vidrv_invert_box_pc34_compat(
    RedmcsbF8154C25VgaAperturePc34Compat *aperture,
    int16_t x1, int16_t x2, int16_t y1, int16_t y2);

const char *redmcsb_f8154_vidrv_invert_box_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
