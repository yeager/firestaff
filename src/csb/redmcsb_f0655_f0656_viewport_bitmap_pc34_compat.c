#include "redmcsb_f0655_f0656_viewport_bitmap_pc34_compat.h"

#include <string.h>

static int16_t bitmap_dimension(const uint8_t *bitmap, int offset)
{
    int16_t value;

    memcpy(&value, bitmap + offset, sizeof(value));
    return value;
}

int redmcsb_f0655_copy_bitmap_and_flip_pc34_compat(
    const redmcsb_f0655_f0656_renderer_pc34_compat *renderer,
    const uint8_t *source_bitmap,
    uint8_t *destination_bitmap,
    int16_t flip)
{
    int16_t width;
    int16_t height;
    int16_t xyz[4];

    if (renderer == NULL || renderer->video_blit == NULL ||
        source_bitmap == NULL || destination_bitmap == NULL) {
        return 0;
    }
    width = bitmap_dimension(source_bitmap, -4);
    height = bitmap_dimension(source_bitmap, -2);
    memcpy(destination_bitmap - 4, source_bitmap - 4, 4U);
    xyz[0] = 0;
    xyz[1] = 0;
    xyz[2] = (int16_t)(width - 1);
    xyz[3] = (int16_t)(height - 1);
    renderer->video_blit(renderer->context, source_bitmap, destination_bitmap,
                         xyz, 0, 0, width, width,
                         REDMCSB_F0655_F0656_PC34_NO_TRANSPARENCY, flip);
    return 1;
}

int redmcsb_f0656_blit_bitmap_to_viewport_zone_with_transparency_pc34_compat(
    const redmcsb_f0655_f0656_renderer_pc34_compat *renderer,
    const uint8_t *bitmap,
    int16_t zone_index,
    int16_t transparent_color)
{
    int16_t x = 0;
    int16_t y = 0;
    int16_t xyz[4];

    if (renderer == NULL || renderer->video_blit == NULL ||
        renderer->resolve_viewport_zone == NULL ||
        renderer->viewport_bitmap == NULL || bitmap == NULL) {
        return 0;
    }
    if (!renderer->resolve_viewport_zone(renderer->context, bitmap, xyz,
                                         zone_index, &x, &y)) {
        return 0;
    }
    renderer->video_blit(renderer->context, bitmap, renderer->viewport_bitmap,
                         xyz, x, y, bitmap_dimension(bitmap, -4),
                         renderer->viewport_pixel_width, transparent_color,
                         REDMCSB_F0655_F0656_PC34_NO_FLIP);
    return 1;
}

const char *redmcsb_f0655_f0656_viewport_bitmap_source_evidence_pc34(void)
{
    return "ReDMCSB BASE.C F0655_CopyBitmapAndFlip (1216-1244); "
           "F0656_BlitBitmapToViewportZoneIndexWithTransparency (1292-1315); "
           "MEMORY.C F0615_CopyBitmapDimensions (2715-2721); "
           "DEFS.H M100/M101 (3444-3445)";
}
