/*
 * ReDMCSB BASE.C F0766_BlitToScreen, PC 3.4 I34M route.
 */
#ifndef FIRESTAFF_REDMCSB_F0766_BLIT_TO_SCREEN_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0766_BLIT_TO_SCREEN_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int16_t (*redmcsb_f0766_bitmap_pixel_width_pc34_compat)(
    void *context,
    const void *bitmap);

typedef void (*redmcsb_f0766_video_blit_pc34_compat)(
    void *context,
    const void *source_bitmap,
    void *destination_bitmap,
    const int16_t xyz[4],
    int16_t source_x,
    int16_t source_y,
    int16_t source_pixel_width,
    int16_t destination_pixel_width,
    int16_t transparent_color,
    int16_t flip);

typedef struct redmcsb_f0766_renderer_pc34_compat {
    redmcsb_f0766_bitmap_pixel_width_pc34_compat bitmap_pixel_width;
    redmcsb_f0766_video_blit_pc34_compat video_blit;
    void *context;
    void *screen_bitmap;
    int16_t screen_pixel_width;
} redmcsb_f0766_renderer_pc34_compat;

/* ReDMCSB MASK0x0000_NO_FLIP. */
#define REDMCSB_F0766_NO_FLIP_PC34_COMPAT INT16_C(0)

/*
 * Executes BASE.C F0766's PC 3.4 I34M F0132_VIDEO_Blit call. Bitmap pixels
 * and destination storage are owned by the supplied original-data renderer.
 */
void redmcsb_f0766_blit_to_screen_pc34_compat(
    const redmcsb_f0766_renderer_pc34_compat *renderer,
    const void *bitmap,
    const int16_t xyz[4],
    int16_t transparent_color);

const char *redmcsb_f0766_blit_to_screen_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
