#include "redmcsb_f0765_draw_bitmap_without_transparency_pc34_compat.h"

#include <stddef.h>

void redmcsb_f0765_draw_bitmap_without_transparency_pc34_compat(
    const redmcsb_f0765_renderer_pc34_compat *renderer,
    int16_t native_bitmap_index,
    int16_t zone_index)
{
    redmcsb_f0765_bitmap_struct2_pc34 bitmap_struct2;
    void *bitmap = renderer->init_bitmap_struct2(
        renderer->context, native_bitmap_index, &bitmap_struct2);

    if (bitmap != NULL) {
        int16_t xyz[4];

        if (renderer->init_zone(renderer->context, bitmap, xyz, zone_index,
                                &bitmap_struct2.width,
                                &bitmap_struct2.height)) {
            renderer->video_blit(
                renderer->context, bitmap, renderer->viewport_bitmap, xyz,
                (int16_t)(bitmap_struct2.x + bitmap_struct2.width),
                (int16_t)(bitmap_struct2.y + bitmap_struct2.height),
                renderer->bitmap_pixel_width(renderer->context, bitmap),
                renderer->viewport_pixel_width,
                REDMCSB_F0765_COLOR_NO_TRANSPARENCY_PC34_COMPAT);
        }
    }
}

const char *redmcsb_f0765_draw_bitmap_without_transparency_source_evidence_pc34(void)
{
    return "ReDMCSB DUNVIEW.C:3159-3185: F0765 initializes STRUCT2 via "
           "F0630, accepts F0635 zone framing only when non-null, then calls "
           "F0132_VIDEO_Blit with the native bitmap, viewport, STRUCT2 "
           "right/bottom source origin, M100 pixel width, C224 viewport "
           "width, and CM1_COLOR_NO_TRANSPARENCY.";
}
