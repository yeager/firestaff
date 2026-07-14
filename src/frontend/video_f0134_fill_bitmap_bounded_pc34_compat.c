#include "video_f0134_fill_bitmap_bounded_pc34_compat.h"

int video_f0134_fill_bitmap_bounded_pc34_compat(
    uint8_t *bitmap,
    size_t bitmap_bytes,
    uint8_t color,
    size_t unit_count)
{
    size_t unit_index;
    unsigned int plane;

    if (bitmap == NULL || unit_count == 0u || unit_count > bitmap_bytes / 8u) {
        return 0;
    }

    for (unit_index = 0u; unit_index < unit_count; ++unit_index) {
        uint8_t *unit = bitmap + (unit_index * 8u);

        for (plane = 0u; plane < 4u; ++plane) {
            uint8_t mask = (color & (uint8_t)(1u << plane)) != 0u ? 0xffu : 0u;

            unit[plane * 2u] = mask;
            unit[(plane * 2u) + 1u] = mask;
        }
    }

    return 1;
}
