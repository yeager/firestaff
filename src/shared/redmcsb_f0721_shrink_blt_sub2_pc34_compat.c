#include "redmcsb_f0721_shrink_blt_sub2_pc34_compat.h"

void
redmcsb_f0721_shrink_blt_sub2_pc34_compat(
    const uint8_t *bitmap_source,
    uint8_t *bitmap_destination,
    const uint8_t *palette_changes,
    int16_t source_row_offset,
    uint16_t destination_pixel_offset,
    uint16_t source_pixel_ratio,
    uint16_t destination_pixel_width)
{
    uint16_t source_position = (uint16_t)(source_pixel_ratio >> 1);
    uint16_t destination_position = destination_pixel_offset;

    while ((uint16_t)(destination_pixel_width + destination_pixel_offset) >
           destination_position) {
        uint16_t source_pixel = (uint16_t)(
            (source_position >> 6) + (uint16_t)source_row_offset);
        uint8_t source_byte = bitmap_source[source_pixel >> 1];
        uint8_t destination_high_nibble;

        source_position = (uint16_t)(source_position + source_pixel_ratio);
        if ((source_pixel & UINT16_C(1)) != 0U) {
            destination_high_nibble =
                (uint8_t)(palette_changes[source_byte & UINT8_C(0x0f)] << 4);
        } else {
            destination_high_nibble =
                (uint8_t)(palette_changes[source_byte >> 4] << 4);
        }

        source_pixel = (uint16_t)(
            (source_position >> 6) + (uint16_t)source_row_offset);
        source_byte = bitmap_source[source_pixel >> 1];
        source_position = (uint16_t)(source_position + source_pixel_ratio);
        if ((source_pixel & UINT16_C(1)) != 0U) {
            bitmap_destination[destination_position >> 1] =
                (uint8_t)(destination_high_nibble |
                          palette_changes[source_byte & UINT8_C(0x0f)]);
        } else {
            bitmap_destination[destination_position >> 1] =
                (uint8_t)(destination_high_nibble |
                          palette_changes[source_byte >> 4]);
        }
        destination_position = (uint16_t)(destination_position + 2U);
    }
}
