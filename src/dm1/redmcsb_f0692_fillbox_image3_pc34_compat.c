#include "redmcsb_f0692_fillbox_image3_pc34_compat.h"

static void redmcsb_f0692_set_pixel(uint8_t *line, int x, uint8_t color)
{
    uint8_t *packed = line + (size_t)x / 2u;

    if ((x & 1) == 0) {
        *packed = (uint8_t)((*packed & 0x0fu) | (uint8_t)(color << 4));
    } else {
        *packed = (uint8_t)((*packed & 0xf0u) | color);
    }
}

static void redmcsb_f0692_fill_line(
    uint8_t *line,
    int left,
    int right,
    uint8_t color)
{
    int x = left;

    if ((x & 1) != 0) {
        redmcsb_f0692_set_pixel(line, x, color);
        ++x;
    }

    while (x + 1 <= right) {
        line[(size_t)x / 2u] = (uint8_t)((color << 4) | color);
        x += 2;
    }

    if (x <= right) {
        redmcsb_f0692_set_pixel(line, x, color);
    }
}

int redmcsb_f0692_fillbox_image3_pc34_compat(
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
    const uint8_t pixel = (uint8_t)(color & 0x000fu);

    if (!bitmap || !box || row_bytes == 0 || pixel_height == 0 ||
        pixel_height > SIZE_MAX / row_bytes) {
        return 0;
    }

    required_size = row_bytes * pixel_height;
    if (bitmap_size < required_size || row_bytes > SIZE_MAX / 2u ||
        row_bytes * 2u > (size_t)INT16_MAX ||
        pixel_height > (size_t)INT16_MAX) {
        return 0;
    }
    pixel_width = row_bytes * 2u;

    left = box[0];
    right = box[1];
    top = box[2];
    bottom = box[3];
    if (left < 0 || top < 0 || left > right || top > bottom ||
        (size_t)right >= pixel_width || (size_t)bottom >= pixel_height) {
        return 0;
    }

    for (y = top; y <= bottom; ++y) {
        uint8_t *line = bitmap + (size_t)y * row_bytes;

        redmcsb_f0692_fill_line(line, left, right, pixel);
    }

    return 1;
}
