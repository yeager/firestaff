#include "redmcsb_f0792_dungeonview_draw_bitmap_yyy_pc34_compat.h"

#include <stddef.h>

enum {
    REDMCSB_F0792_COLOR_NO_TRANSPARENCY_PC34 = -1
};

void redmcsb_f0792_dungeonview_draw_bitmap_yyy_pc34_compat(
    const RedmcsbF0792DungeonviewHooksPc34 *hooks,
    void *viewport_bitmap,
    int16_t viewport_pixel_width,
    int16_t native_bitmap_index,
    int16_t zone_index,
    int16_t flip)
{
    void *bitmap;
    RedmcsbF0792BitmapStruct2Pc34 bitmap_struct2;
    int16_t xyz[4];

    bitmap = hooks->init_bitmap_struct2(
        hooks->context, native_bitmap_index, &bitmap_struct2);
    if (bitmap != NULL && hooks->resolve_zone(
                              hooks->context,
                              bitmap,
                              xyz,
                              zone_index,
                              &bitmap_struct2.width,
                              &bitmap_struct2.height)) {
        hooks->video_blit(
            hooks->context,
            bitmap,
            viewport_bitmap,
            xyz,
            (int16_t)(bitmap_struct2.x + bitmap_struct2.width),
            (int16_t)(bitmap_struct2.y + bitmap_struct2.height),
            hooks->bitmap_pixel_width(hooks->context, bitmap),
            viewport_pixel_width,
            REDMCSB_F0792_COLOR_NO_TRANSPARENCY_PC34,
            flip);
    }
}

const char *redmcsb_f0792_dungeonview_draw_bitmap_yyy_source_evidence_pc34(void)
{
    return "ReDMCSB DUNVIEW.C:3288-3301; F0630, F0635 and opaque F0132 viewport blit";
}
