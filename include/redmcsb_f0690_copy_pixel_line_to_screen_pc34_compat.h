/*
 * ReDMCSB IMAGE3.C F0690 PC 3.4 video-driver forwarder.
 */
#ifndef FIRESTAFF_REDMCSB_F0690_COPY_PIXEL_LINE_TO_SCREEN_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0690_COPY_PIXEL_LINE_TO_SCREEN_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Exact VIDRV_00 argument order from IMAGE3.C:932-937.  The callback owns
 * presentation; F0690 performs no decoding, clipping, or surface rendering.
 */
typedef void (*RedmcsbF0690CopyPixelsToScreenPc34Compat)(
    void *context,
    const uint8_t *bitmap_pixel_line,
    uint16_t destination_x,
    uint16_t destination_pixel_index,
    int16_t pixel_count);

typedef struct {
    RedmcsbF0690CopyPixelsToScreenPc34Compat copy_pixels_to_screen;
    void *context;
} RedmcsbF0690VideoDriverPc34Compat;

/*
 * Caller supplies the valid G2158_auc_Bitmap_PixelLine equivalent and a
 * populated VIDRV_00 callback.  This forwards precisely as F0690 does.
 */
void redmcsb_f0690_copy_pixel_line_to_screen_pc34_compat(
    const RedmcsbF0690VideoDriverPc34Compat *video_driver,
    const uint8_t *bitmap_pixel_line,
    uint16_t destination_pixel_index,
    int16_t pixel_count);

const char *redmcsb_f0690_copy_pixel_line_to_screen_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
