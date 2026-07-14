#include "dm1_f0135_video_fillbox_planar_20260714_pc34_compat.h"

#include <limits.h>

static uint16_t dm1_f0135_read_be16(const uint8_t *address)
{
    return (uint16_t)(((uint16_t)address[0] << 8) | address[1]);
}

static void dm1_f0135_write_be16(uint8_t *address, uint16_t value)
{
    address[0] = (uint8_t)(value >> 8);
    address[1] = (uint8_t)value;
}

int dm1_f0135_video_fillbox_planar_20260714_pc34_compat(
    uint8_t *bitmap,
    size_t bitmap_size,
    size_t row_bytes,
    size_t pixel_height,
    const int16_t box[4],
    uint16_t color)
{
    size_t required_size;
    size_t pixel_width;
    int left;
    int right;
    int top;
    int bottom;
    int y;
    const unsigned int color_index = color & 0x000fu;
    const int alternate_pixels = (color & 0x8000u) != 0;

    if (!bitmap || !box || row_bytes == 0 || (row_bytes % 8u) != 0 ||
        pixel_height == 0 || pixel_height > SIZE_MAX / row_bytes) {
        return 0;
    }

    required_size = row_bytes * pixel_height;
    if (bitmap_size < required_size || row_bytes > SIZE_MAX / 2u) {
        return 0;
    }
    pixel_width = row_bytes * 2u;
    if (pixel_width > (size_t)INT_MAX || pixel_height > (size_t)INT_MAX) {
        return 0;
    }

    left = box[0];
    right = box[1];
    top = box[2];
    bottom = box[3];
    if (left < 0 || top < 0 || left > right || top > bottom ||
        (size_t)right >= pixel_width || (size_t)bottom >= pixel_height) {
        return 0;
    }

    for (y = top; y <= bottom; ++y) {
        int x;

        for (x = left; x <= right; ++x) {
            size_t word_offset;
            uint16_t pixel_mask;
            unsigned int plane;

            if (alternate_pixels && (((x - left) + (y - top)) & 1) != 0) {
                continue;
            }

            word_offset = (size_t)y * row_bytes + (size_t)(x / 16) * 8u;
            pixel_mask = (uint16_t)(0x8000u >> (x & 15));
            for (plane = 0; plane < 4u; ++plane) {
                uint8_t *word_address = bitmap + word_offset + plane * 2u;
                uint16_t word = dm1_f0135_read_be16(word_address);

                if ((color_index & (1u << plane)) != 0) {
                    word |= pixel_mask;
                } else {
                    word &= (uint16_t)~pixel_mask;
                }
                dm1_f0135_write_be16(word_address, word);
            }
        }
    }

    return 1;
}
