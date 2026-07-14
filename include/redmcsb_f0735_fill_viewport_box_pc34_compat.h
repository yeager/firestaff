/*
 * ReDMCSB BLITFILL.C F0735_FillViewportBox, PC 3.4 (I34E/I34M) route.
 *
 * F0735 directly forwards the supplied box and color to F0135_VIDEO_FillBox
 * for the 224-pixel-wide viewport bitmap. The PC 3.4 call has no height
 * argument.
 */
#ifndef FIRESTAFF_REDMCSB_F0735_FILL_VIEWPORT_BOX_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0735_FILL_VIEWPORT_BOX_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*redmcsb_f0735_fill_box_pc34_compat)(
    void *context,
    int16_t *xyz,
    int16_t color,
    int16_t bitmap_pixel_width);

typedef struct {
    redmcsb_f0735_fill_box_pc34_compat fill_box;
    void *context;
} redmcsb_f0735_graphics_pc34_compat;

/*
 * Executes the PC 3.4 F0735 call exactly:
 * F0135_VIDEO_FillBox(G0296_puc_Bitmap_Viewport, xyz, color, 224).
 *
 * As in the source, the callback must be valid and xyz is passed through
 * unchanged, including a null value.
 */
void redmcsb_f0735_fill_viewport_box_pc34_compat(
    const redmcsb_f0735_graphics_pc34_compat *graphics,
    int16_t *xyz,
    int16_t color);

const char *redmcsb_f0735_fill_viewport_box_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
