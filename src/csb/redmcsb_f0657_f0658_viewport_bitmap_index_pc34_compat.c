#include "redmcsb_f0657_f0658_viewport_bitmap_index_pc34_compat.h"

#include <stddef.h>

static int valid_f0657_renderer(
    const redmcsb_f0657_f0658_renderer_pc34_compat *renderer)
{
    return renderer != NULL && renderer->init_bitmap_struct2 != NULL &&
           renderer->bitmap_pixel_width != NULL && renderer->video_blit != NULL &&
           renderer->viewport_bitmap != NULL;
}

int redmcsb_f0657_blit_bitmap_index_to_viewport_zone_with_transparency_pc34_compat(
    const redmcsb_f0657_f0658_renderer_pc34_compat *renderer,
    int16_t bitmap_index,
    const int16_t xyz[4],
    int16_t transparent_color)
{
    redmcsb_f0657_f0658_bitmap_struct2_pc34_compat bitmap_struct2;
    const uint8_t *bitmap;

    if (!valid_f0657_renderer(renderer) || xyz == NULL) return 0;
    bitmap = renderer->init_bitmap_struct2(renderer->context, bitmap_index,
                                           &bitmap_struct2);
    if (bitmap == NULL) return 0;
    renderer->video_blit(renderer->context, bitmap, renderer->viewport_bitmap,
                         xyz, bitmap_struct2.x, bitmap_struct2.y,
                         renderer->bitmap_pixel_width(renderer->context, bitmap),
                         renderer->viewport_pixel_width, transparent_color,
                         REDMCSB_F0657_F0658_PC34_NO_FLIP);
    return 1;
}

int redmcsb_f0658_blit_bitmap_index_to_zone_index_with_transparency_pc34_compat(
    const redmcsb_f0657_f0658_renderer_pc34_compat *renderer,
    int16_t bitmap_index,
    int16_t zone_index,
    int16_t transparent_color)
{
    redmcsb_f0657_f0658_bitmap_struct2_pc34_compat bitmap_struct2;
    const uint8_t *bitmap;
    int16_t x;
    int16_t y;
    int16_t xyz[4];

    if (!valid_f0657_renderer(renderer) || renderer->resolve_zone == NULL) {
        return 0;
    }
    bitmap = renderer->init_bitmap_struct2(renderer->context, bitmap_index,
                                           &bitmap_struct2);
    if (bitmap == NULL) return 0;
    if (bitmap_index < 0) {
        x = bitmap_struct2.width;
        y = bitmap_struct2.height;
    } else {
        x = 0;
        y = 0;
    }
    if (!renderer->resolve_zone(renderer->context, bitmap, xyz, zone_index,
                                &x, &y)) {
        return 0;
    }
    renderer->video_blit(renderer->context, bitmap, renderer->viewport_bitmap,
                         xyz, (int16_t)(bitmap_struct2.x + x),
                         (int16_t)(bitmap_struct2.y + y),
                         renderer->bitmap_pixel_width(renderer->context, bitmap),
                         renderer->viewport_pixel_width, transparent_color,
                         REDMCSB_F0657_F0658_PC34_NO_FLIP);
    return 1;
}

const char *redmcsb_f0657_f0658_viewport_bitmap_index_source_evidence_pc34(void)
{
    return "ReDMCSB BASE.C F0657_BlitBitmapIndexToViewportZoneWithTransparency "
           "(1320-1338); F0658_BlitBitmapIndexToZoneIndexWithTransparency "
           "(1341-1370); COORD.C F0630_InitBitmapStruct2 (1939-2000)";
}
