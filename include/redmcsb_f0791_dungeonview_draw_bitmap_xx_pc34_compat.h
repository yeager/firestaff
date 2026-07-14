/*
 * ReDMCSB DUNVIEW.C F0791_DUNGEONVIEW_DrawBitmapXX, PC 3.4 I34M route.
 */
#ifndef FIRESTAFF_REDMCSB_F0791_DUNGEONVIEW_DRAW_BITMAP_XX_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0791_DUNGEONVIEW_DRAW_BITMAP_XX_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*redmcsb_f0791_init_zone_pc34_compat)(
    void *context,
    const void *source_bitmap,
    int16_t xyz[4],
    int16_t zone_index,
    int16_t *x,
    int16_t *y);

typedef int16_t (*redmcsb_f0791_bitmap_pixel_width_pc34_compat)(
    void *context,
    const void *bitmap);

typedef int16_t (*redmcsb_f0791_bitmap_pixel_height_pc34_compat)(
    void *context,
    const void *bitmap);

typedef void (*redmcsb_f0791_video_blit_pc34_compat)(
    void *context,
    const void *source_bitmap,
    void *destination_bitmap,
    const int16_t xyz[4],
    int16_t source_x,
    int16_t source_y,
    int16_t source_pixel_width,
    int16_t destination_pixel_width,
    int16_t transparent_color,
    uint16_t flip);

typedef struct redmcsb_f0791_renderer_pc34_compat {
    redmcsb_f0791_init_zone_pc34_compat init_zone;
    redmcsb_f0791_bitmap_pixel_width_pc34_compat bitmap_pixel_width;
    redmcsb_f0791_bitmap_pixel_height_pc34_compat bitmap_pixel_height;
    redmcsb_f0791_video_blit_pc34_compat video_blit;
    void *context;
} redmcsb_f0791_renderer_pc34_compat;

#define REDMCSB_F0791_ZONE_UNKNOWN_PC34_COMPAT INT16_C(-1)
#define REDMCSB_F0791_SHIFT_OBJECTS_AND_CREATURES_PC34_COMPAT UINT16_C(0x8000)
#define REDMCSB_F0791_SHIFT_UNREADABLE_INSCRIPTION_PC34_COMPAT UINT16_C(0x4000)
#define REDMCSB_F0791_FLIP_HORIZONTAL_PC34_COMPAT UINT16_C(0x0001)
#define REDMCSB_F0791_FLIP_VERTICAL_PC34_COMPAT UINT16_C(0x0002)

/*
 * Executes DUNVIEW.C F0791's PC 3.4 source-zone route. Bitmap, layout, and
 * blit data remain owned by the supplied original-data renderer callbacks.
 */
void redmcsb_f0791_dungeonview_draw_bitmap_xx_pc34_compat(
    const redmcsb_f0791_renderer_pc34_compat *renderer,
    const void *source_bitmap,
    void *destination_bitmap,
    int16_t zone_index,
    uint16_t flip,
    int16_t transparent_color,
    int16_t zone_shift_x,
    int16_t zone_shift_y);

const char *redmcsb_f0791_dungeonview_draw_bitmap_xx_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
