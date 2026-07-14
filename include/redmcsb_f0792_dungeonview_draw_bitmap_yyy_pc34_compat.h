/*
 * ReDMCSB DUNVIEW.C F0792_DUNGEONVIEW_DrawBitmapYYY, PC 3.4 route.
 */
#ifndef FIRESTAFF_REDMCSB_F0792_DUNGEONVIEW_DRAW_BITMAP_YYY_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0792_DUNGEONVIEW_DRAW_BITMAP_YYY_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct RedmcsbF0792BitmapStruct2Pc34 {
    int16_t x;
    int16_t y;
    int16_t width;
    int16_t height;
} RedmcsbF0792BitmapStruct2Pc34;

typedef void *(*RedmcsbF0792InitBitmapPc34)(
    void *context,
    int16_t native_bitmap_index,
    RedmcsbF0792BitmapStruct2Pc34 *bitmap);

typedef bool (*RedmcsbF0792ResolveZonePc34)(
    void *context,
    const void *bitmap,
    int16_t xyz[4],
    int16_t zone_index,
    int16_t *width,
    int16_t *height);

typedef int16_t (*RedmcsbF0792BitmapPixelWidthPc34)(
    void *context,
    const void *bitmap);

typedef void (*RedmcsbF0792VideoBlitPc34)(
    void *context,
    const void *source,
    void *destination,
    const int16_t xyz[4],
    int16_t destination_x,
    int16_t destination_y,
    int16_t source_pixel_width,
    int16_t destination_pixel_width,
    int16_t transparent_color,
    int16_t flip);

typedef struct RedmcsbF0792DungeonviewHooksPc34 {
    void *context;
    RedmcsbF0792InitBitmapPc34 init_bitmap_struct2;
    RedmcsbF0792ResolveZonePc34 resolve_zone;
    RedmcsbF0792BitmapPixelWidthPc34 bitmap_pixel_width;
    RedmcsbF0792VideoBlitPc34 video_blit;
} RedmcsbF0792DungeonviewHooksPc34;

/*
 * The hook table represents the already-existing engine services F0630,
 * F0635 and F0132. This adapter does not decode or synthesize bitmap pixels.
 */
void redmcsb_f0792_dungeonview_draw_bitmap_yyy_pc34_compat(
    const RedmcsbF0792DungeonviewHooksPc34 *hooks,
    void *viewport_bitmap,
    int16_t viewport_pixel_width,
    int16_t native_bitmap_index,
    int16_t zone_index,
    int16_t flip);

const char *redmcsb_f0792_dungeonview_draw_bitmap_yyy_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
