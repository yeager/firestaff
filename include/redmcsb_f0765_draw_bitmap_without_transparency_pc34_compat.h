/*
 * ReDMCSB DUNVIEW.C F0765_DUNGEONVIEW_DrawBitmapWithoutTransparency,
 * PC 3.4 I34M route.
 */
#ifndef FIRESTAFF_REDMCSB_F0765_DRAW_BITMAP_WITHOUT_TRANSPARENCY_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0765_DRAW_BITMAP_WITHOUT_TRANSPARENCY_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* The F0630 STRUCT2 fields consumed by F0765. */
typedef struct redmcsb_f0765_bitmap_struct2_pc34 {
    int16_t x;
    int16_t y;
    int16_t width;
    int16_t height;
} redmcsb_f0765_bitmap_struct2_pc34;

typedef void *(*redmcsb_f0765_init_bitmap_struct2_pc34_compat)(
    void *context,
    int16_t native_bitmap_index,
    redmcsb_f0765_bitmap_struct2_pc34 *bitmap_struct2);

typedef int (*redmcsb_f0765_init_zone_pc34_compat)(
    void *context,
    void *bitmap,
    int16_t xyz[4],
    int16_t zone_index,
    int16_t *width,
    int16_t *height);

typedef int16_t (*redmcsb_f0765_bitmap_pixel_width_pc34_compat)(
    void *context,
    const void *bitmap);

typedef void (*redmcsb_f0765_video_blit_pc34_compat)(
    void *context,
    const void *bitmap,
    void *viewport_bitmap,
    const int16_t xyz[4],
    int16_t source_x,
    int16_t source_y,
    int16_t source_pixel_width,
    int16_t viewport_pixel_width,
    int16_t transparency_color);

typedef struct redmcsb_f0765_renderer_pc34_compat {
    redmcsb_f0765_init_bitmap_struct2_pc34_compat init_bitmap_struct2;
    redmcsb_f0765_init_zone_pc34_compat init_zone;
    redmcsb_f0765_bitmap_pixel_width_pc34_compat bitmap_pixel_width;
    redmcsb_f0765_video_blit_pc34_compat video_blit;
    void *context;
    void *viewport_bitmap;
    int16_t viewport_pixel_width;
} redmcsb_f0765_renderer_pc34_compat;

/* ReDMCSB CM1_COLOR_NO_TRANSPARENCY. */
#define REDMCSB_F0765_COLOR_NO_TRANSPARENCY_PC34_COMPAT INT16_C(-1)

/*
 * Executes the PC 3.4 F0765 call sequence. All bitmap/zone/pixel data come
 * from the supplied original-data renderer callbacks; this adapter supplies
 * no synthetic graphics.
 */
void redmcsb_f0765_draw_bitmap_without_transparency_pc34_compat(
    const redmcsb_f0765_renderer_pc34_compat *renderer,
    int16_t native_bitmap_index,
    int16_t zone_index);

const char *redmcsb_f0765_draw_bitmap_without_transparency_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
