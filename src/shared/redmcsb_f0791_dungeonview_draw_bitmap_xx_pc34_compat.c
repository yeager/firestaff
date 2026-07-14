#include "redmcsb_f0791_dungeonview_draw_bitmap_xx_pc34_compat.h"

void redmcsb_f0791_dungeonview_draw_bitmap_xx_pc34_compat(
    const redmcsb_f0791_renderer_pc34_compat *renderer,
    const void *source_bitmap,
    void *destination_bitmap,
    int16_t zone_index,
    uint16_t flip,
    int16_t transparent_color,
    int16_t zone_shift_x,
    int16_t zone_shift_y)
{
    int16_t source_x;
    int16_t source_y;
    int16_t xyz[4];

    if (zone_index == REDMCSB_F0791_ZONE_UNKNOWN_PC34_COMPAT) {
        return;
    }

    if ((uint16_t)zone_index &
        (REDMCSB_F0791_SHIFT_OBJECTS_AND_CREATURES_PC34_COMPAT |
         REDMCSB_F0791_SHIFT_UNREADABLE_INSCRIPTION_PC34_COMPAT)) {
        source_x = zone_shift_x;
        source_y = zone_shift_y;
        zone_index = (int16_t)((uint16_t)zone_index &
            ~REDMCSB_F0791_SHIFT_UNREADABLE_INSCRIPTION_PC34_COMPAT);
    } else {
        source_x = INT16_C(0);
        source_y = INT16_C(0);
    }

    if (renderer->init_zone(renderer->context, source_bitmap, xyz, zone_index,
                            &source_x, &source_y)) {
        int16_t bitmap_width = renderer->bitmap_pixel_width(renderer->context,
                                                             source_bitmap);
        int16_t bitmap_height = renderer->bitmap_pixel_height(renderer->context,
                                                               source_bitmap);
        int16_t combined = (int16_t)(source_x + xyz[2]);

        if (bitmap_width > combined &&
            (flip & REDMCSB_F0791_FLIP_HORIZONTAL_PC34_COMPAT)) {
            combined = (int16_t)(bitmap_width - combined);
        } else {
            combined = INT16_C(0);
        }
        if (source_x != 0 &&
            (flip & REDMCSB_F0791_FLIP_HORIZONTAL_PC34_COMPAT)) {
            source_x = INT16_C(0);
        }
        source_x = (int16_t)(source_x + combined);

        combined = (int16_t)(source_y + xyz[3]);
        if (bitmap_height > combined &&
            (flip & REDMCSB_F0791_FLIP_VERTICAL_PC34_COMPAT)) {
            combined = (int16_t)(bitmap_height - combined);
        } else {
            combined = INT16_C(0);
        }
        if (source_y != 0 &&
            (flip & REDMCSB_F0791_FLIP_VERTICAL_PC34_COMPAT)) {
            source_y = INT16_C(0);
        }
        source_y = (int16_t)(source_y + combined);

        renderer->video_blit(
            renderer->context, source_bitmap, destination_bitmap, xyz,
            source_x, source_y, bitmap_width,
            renderer->bitmap_pixel_width(renderer->context, destination_bitmap),
            transparent_color, flip);
    }
}

const char *redmcsb_f0791_dungeonview_draw_bitmap_xx_source_evidence_pc34(void)
{
    return "ReDMCSB DUNVIEW.C:3394-3473, MEDIA463 P20JA/P20JB/I34E/I34M/"
           "P31J branch: F0791 rejects CM1_UNKNOWN, seeds F0635 with "
           "G2154/G2155 for shifted zones, clears only MASK0x4000, adjusts "
           "source offsets for horizontal/vertical flips, then calls "
           "F0132_VIDEO_Blit with source and destination M100 widths.";
}
