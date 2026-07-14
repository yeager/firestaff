#include "redmcsb_f0691_draw_compressed_img3_pc34_compat.h"
#include "redmcsb_f0685_img3_line_fill_pc34_compat.h"
#include "redmcsb_f0687_f0688_img3_pc34_compat.h"

#define REDMCSB_F0691_PC34_SCREEN_WIDTH 320U
#define REDMCSB_F0691_PC34_SCREEN_HEIGHT 200U

static int redmcsb_f0691_next_command_pc34(
    redmcsb_f0687_img3_stream_pc34_compat *stream,
    size_t remaining,
    uint8_t *out_command,
    uint8_t *out_color,
    size_t *out_count)
{
    uint8_t command;
    uint8_t color = 0;
    uint16_t count;

    if (!redmcsb_f0687_img3_get_nibble_pc34_compat(stream, &command)) {
        return 0;
    }
    if ((command & 7U) == 7U &&
        !redmcsb_f0687_img3_get_nibble_pc34_compat(stream, &color)) {
        return 0;
    }
    if ((command & 8U) != 0U) {
        if (!redmcsb_f0688_img3_get_pixel_count_pc34_compat(stream, &count)) {
            return 0;
        }
    } else {
        count = 1;
    }
    if (count == 0 || (size_t)count > remaining) {
        return 0;
    }
    if (out_command != 0) {
        *out_command = command;
    }
    if (out_color != 0) {
        *out_color = color;
    }
    *out_count = count;
    return 1;
}

static int redmcsb_f0691_validate_pc34(
    const uint8_t *graphic,
    size_t graphic_size,
    size_t pixel_count)
{
    redmcsb_f0687_img3_stream_pc34_compat stream;
    uint8_t ignored_nibble;
    size_t offset = 0;
    size_t count;
    size_t index;

    stream.bytes = graphic;
    stream.byte_count = graphic_size;
    stream.pixel_index = 8;
    for (index = 0; index < 6; ++index) {
        if (!redmcsb_f0687_img3_get_nibble_pc34_compat(&stream,
                                                        &ignored_nibble)) {
            return 0;
        }
    }
    while (offset < pixel_count) {
        if (!redmcsb_f0691_next_command_pc34(
                &stream, pixel_count - offset, 0, 0, &count)) {
            return 0;
        }
        offset += count;
    }
    return 1;
}

int redmcsb_f0691_draw_compressed_img3_pc34_compat(
    const uint8_t *graphic,
    size_t graphic_size,
    int16_t x,
    int16_t y,
    uint8_t *pixel_line,
    size_t pixel_line_size,
    redmcsb_f0691_img3_pc34_sink sink,
    void *sink_context)
{
    redmcsb_f0687_img3_stream_pc34_compat stream;
    uint8_t palette[6];
    uint8_t command;
    uint8_t color;
    int writes_pixels;
    int16_t signed_width;
    int16_t signed_height;
    size_t width;
    size_t height;
    size_t total;
    size_t offset = 0;
    size_t count;
    size_t index;

    if (graphic == 0 || pixel_line == 0 || sink == 0 || graphic_size < 7) {
        return 0;
    }
    signed_width = (int16_t)((uint16_t)graphic[0] | ((uint16_t)graphic[1] << 8));
    signed_height = (int16_t)((uint16_t)graphic[2] | ((uint16_t)graphic[3] << 8));
    if (signed_width <= 0 || signed_height <= 0 || x < 0 || y < 0) {
        return 0;
    }
    width = (size_t)signed_width;
    height = (size_t)signed_height;
    if (width > REDMCSB_F0691_PC34_SCREEN_WIDTH ||
        height > REDMCSB_F0691_PC34_SCREEN_HEIGHT ||
        (size_t)x > REDMCSB_F0691_PC34_SCREEN_WIDTH - width ||
        (size_t)y > REDMCSB_F0691_PC34_SCREEN_HEIGHT - height ||
        height > SIZE_MAX / width) {
        return 0;
    }
    total = width * height;
    if (pixel_line_size < (width + 1U) / 2U ||
        !redmcsb_f0691_validate_pc34(graphic, graphic_size, total)) {
        return 0;
    }

    stream.bytes = graphic;
    stream.byte_count = graphic_size;
    stream.pixel_index = 8;
    for (index = 0; index < 6; ++index) {
        (void)redmcsb_f0687_img3_get_nibble_pc34_compat(&stream, &palette[index]);
    }
    while (offset < total) {
        if (!redmcsb_f0691_next_command_pc34(
                &stream, total - offset, &command, &color, &count)) {
            return 0;
        }
        writes_pixels = (command & 7U) != 6U;
        if (writes_pixels) {
            if ((command & 7U) < 6U) {
                color = palette[command & 7U];
            }
        }
        while (count != 0) {
            size_t row_remaining = width - (offset % width);
            size_t step = count < row_remaining ? count : row_remaining;

            if (writes_pixels && !redmcsb_f0685_img3_line_fill_pc34_compat(
                                     pixel_line, pixel_line_size,
                                     offset % width, color, step)) {
                return 0;
            }
            offset += step;
            count -= step;
            if (offset % width == 0) {
                size_t row = offset / width - 1U;
                sink(sink_context, pixel_line,
                     ((size_t)y + row) * REDMCSB_F0691_PC34_SCREEN_WIDTH +
                         (size_t)x,
                     width);
            }
        }
    }
    return 1;
}
