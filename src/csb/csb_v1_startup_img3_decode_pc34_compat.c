#include "csb_v1_startup_img3_decode_pc34_compat.h"
#include "redmcsb_f0691_draw_compressed_img3_pc34_compat.h"

#include <stdlib.h>

typedef struct {
    uint8_t *pixels;
    size_t pixel_byte_count;
    uint16_t width;
    uint16_t height;
    size_t delivered_rows;
    int valid;
} csb_v1_startup_img3_sink_pc34;

static void csb_v1_startup_img3_copy_row_pc34(
    void *context,
    const uint8_t *packed_pixel_line,
    size_t destination_pixel_index,
    size_t pixel_count)
{
    csb_v1_startup_img3_sink_pc34 *sink =
        (csb_v1_startup_img3_sink_pc34 *)context;
    size_t row = destination_pixel_index / 320U;
    size_t column;

    if (!sink->valid || packed_pixel_line == 0 || pixel_count != sink->width ||
        row >= sink->height || sink->delivered_rows != row ||
        (size_t)sink->width * sink->height > sink->pixel_byte_count) {
        sink->valid = 0;
        return;
    }
    for (column = 0; column < sink->width; ++column) {
        uint8_t packed = packed_pixel_line[column / 2U];
        sink->pixels[row * sink->width + column] =
            (column & 1U) != 0U ? (packed & 0x0fU) : ((packed >> 4) & 0x0fU);
    }
    sink->delivered_rows++;
}

int csb_v1_startup_img3_decode_to_indexed_pc34_compat(
    const uint8_t *graphic,
    size_t graphic_byte_count,
    uint16_t expected_width,
    uint16_t expected_height,
    uint8_t *indexed_pixels,
    size_t indexed_pixel_byte_count)
{
    csb_v1_startup_img3_sink_pc34 sink;
    uint8_t *pixel_line;
    size_t pixel_line_byte_count;
    size_t pixel_count;
    int decoded;

    if (graphic == 0 || indexed_pixels == 0 || graphic_byte_count < 4U ||
        expected_width == 0U || expected_height == 0U ||
        (uint16_t)(graphic[0] | ((uint16_t)graphic[1] << 8)) != expected_width ||
        (uint16_t)(graphic[2] | ((uint16_t)graphic[3] << 8)) != expected_height ||
        expected_height > SIZE_MAX / expected_width) {
        return 0;
    }
    pixel_count = (size_t)expected_width * expected_height;
    pixel_line_byte_count = ((size_t)expected_width + 1U) / 2U;
    if (pixel_count > indexed_pixel_byte_count || pixel_line_byte_count == 0U) {
        return 0;
    }
    pixel_line = (uint8_t *)calloc(pixel_line_byte_count, 1U);
    if (pixel_line == 0) {
        return 0;
    }
    sink.pixels = indexed_pixels;
    sink.pixel_byte_count = indexed_pixel_byte_count;
    sink.width = expected_width;
    sink.height = expected_height;
    sink.delivered_rows = 0U;
    sink.valid = 1;
    decoded = redmcsb_f0691_draw_compressed_img3_pc34_compat(
        graphic, graphic_byte_count, 0, 0, pixel_line, pixel_line_byte_count,
        csb_v1_startup_img3_copy_row_pc34, &sink);
    free(pixel_line);
    return decoded && sink.valid && sink.delivered_rows == expected_height;
}
