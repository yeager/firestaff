#include "f0135_video_fillbox_bounded_20260714_pc34_compat.h"

size_t f0135_video_fillbox_bounded_20260714_pc34_compat(
    uint8_t *bitmap,
    size_t bitmap_size,
    int pixel_width,
    int pixel_height,
    const int16_t box[4],
    uint16_t color)
{
    size_t row_bytes;
    size_t required_size;
    int left;
    int right;
    int top;
    int bottom;
    int y;
    const uint8_t pixel = (uint8_t)(color & 0x000fu);

    if (!bitmap || !box || pixel_width <= 0 || pixel_height <= 0) {
        return 0;
    }

    row_bytes = ((size_t)pixel_width + 1u) / 2u;
    if ((size_t)pixel_height > SIZE_MAX / row_bytes) {
        return 0;
    }
    required_size = row_bytes * (size_t)pixel_height;
    if (bitmap_size < required_size) {
        return 0;
    }

    left = box[0];
    right = box[1];
    top = box[2];
    bottom = box[3];
    if (left > right || top > bottom || right < 0 || bottom < 0 ||
        left >= pixel_width || top >= pixel_height) {
        return 0;
    }
    if (left < 0) {
        left = 0;
    }
    if (right >= pixel_width) {
        right = pixel_width - 1;
    }
    if (top < 0) {
        top = 0;
    }
    if (bottom >= pixel_height) {
        bottom = pixel_height - 1;
    }

    for (y = top; y <= bottom; ++y) {
        uint8_t *row = bitmap + ((size_t)y * row_bytes);
        int x;

        for (x = left; x <= right; ++x) {
            uint8_t *packed_pixel = row + ((size_t)x / 2u);

            if ((x & 1) == 0) {
                *packed_pixel = (uint8_t)((*packed_pixel & 0x0fu) |
                                          (pixel << 4));
            } else {
                *packed_pixel = (uint8_t)((*packed_pixel & 0xf0u) | pixel);
            }
        }
    }

    return (size_t)(right - left + 1) * (size_t)(bottom - top + 1);
}
