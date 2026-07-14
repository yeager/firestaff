/*
 * ReDMCSB BLITFILL.C F0732_FillScreenArea, PC 3.4 (I34E/I34M) route.
 *
 * F0732 passes its ZONE array to F0135_VIDEO_FillBox with a NULL bitmap,
 * the unmodified 16-bit color, and G2071_C320_ScreenPixelWidth.  PC 3.4
 * F0135 expands the ZONE into the inclusive BOX required by VIDRV_01.
 */
#ifndef FIRESTAFF_REDMCSB_F0732_FILL_SCREEN_AREA_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0732_FILL_SCREEN_AREA_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum { REDMCSB_F0732_SCREEN_PIXEL_WIDTH_PC34 = 320 };

/* ZONE is four int16_t values: left, top, width, height. */
typedef void (*redmcsb_f0732_fill_box_callback_pc34_compat)(
    void *context,
    uint8_t *bitmap,
    int16_t *box,
    int16_t color,
    int16_t width);

typedef struct {
    redmcsb_f0732_fill_box_callback_pc34_compat fill_box;
    void *context;
} redmcsb_f0732_video_driver_pc34_compat;

/*
 * Executes the PC 3.4 F0732 path.  zone and video_driver->fill_box must be
 * valid, matching the original global driver-vector contract.
 */
void redmcsb_f0732_fill_screen_area_pc34_compat(
    const redmcsb_f0732_video_driver_pc34_compat *video_driver,
    int16_t *zone,
    uint16_t color);

const char *redmcsb_f0732_fill_screen_area_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
