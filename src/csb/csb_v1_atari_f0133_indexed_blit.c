#include "csb_v1_atari_f0133_indexed_blit.h"

#include <stddef.h>

int csb_v1_atari_f0133_fill_indexed_box(
    const uint8_t *source_pixels,
    int source_width,
    int source_height,
    int last_unit,
    int first_unit,
    uint8_t *destination,
    int destination_stride,
    int destination_x,
    int destination_y,
    int box_width,
    int box_height,
    int transparent_color,
    const uint8_t palette_map[16])
{
    int units_per_row;
    int source_unit_count;
    int destination_units_per_row;
    int destination_unit;
    int written = 0;

    if (!source_pixels || !destination || source_width <= 0 ||
        source_width % 16 != 0 || source_height <= 0 ||
        destination_stride <= 0 || destination_x < 0 || destination_y < 0 ||
        box_width <= 0 || box_height <= 0 ||
        box_width > destination_stride ||
        destination_x > destination_stride - box_width ||
        transparent_color < 0 || transparent_color > 15) {
        return -1;
    }
    units_per_row = source_width / 16;
    source_unit_count = units_per_row * source_height;
    if (units_per_row <= 0 || last_unit <= first_unit || first_unit < 0 ||
        last_unit > source_unit_count) {
        return -1;
    }
    destination_units_per_row = (box_width + 15) / 16;
    for (destination_unit = 0;
         destination_unit < destination_units_per_row * box_height;
         ++destination_unit) {
        const int destination_row = destination_unit /
            destination_units_per_row;
        const int destination_column = destination_unit %
            destination_units_per_row;
        const int unit_offset = destination_unit;
        const int source_unit = (first_unit + unit_offset) % last_unit;
        const int source_y = source_unit / units_per_row;
        const int source_x = (source_unit % units_per_row) * 16;
        int pixel_in_unit;

        for (pixel_in_unit = 0; pixel_in_unit < 16; ++pixel_in_unit) {
            const int x = destination_column * 16 + pixel_in_unit;
            uint8_t color;
            if (x >= box_width) continue;
            color = source_pixels[(size_t)source_y *
                                  (size_t)source_width +
                                  (size_t)(source_x + pixel_in_unit)];
            if (palette_map) color = palette_map[color & 0x0fu];
            if ((int)(color & 0x0fu) == transparent_color) continue;
            destination[(size_t)(destination_y + destination_row) *
                        (size_t)destination_stride +
                        (size_t)(destination_x + x)] = color & 0x0fu;
            ++written;
        }
    }
    return written;
}
