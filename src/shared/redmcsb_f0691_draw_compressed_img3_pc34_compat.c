#include "redmcsb_f0691_draw_compressed_img3_pc34_compat.h"

/* ReDMCSB: IMAGE2.C F0687_IMG3_GetNibble and F0688_IMG3_GetPixelCount. */
static int read_nibble(const uint8_t *graphic,
                       size_t graphic_size,
                       size_t *nibble_index,
                       uint8_t *value)
{
    size_t byte_index;

    if (graphic == NULL || nibble_index == NULL || value == NULL) {
        return 0;
    }
    byte_index = *nibble_index >> 1;
    if (byte_index >= graphic_size) {
        return 0;
    }
    if ((*nibble_index & 1U) != 0U) {
        *value = (uint8_t)(graphic[byte_index] & 0x0fU);
    } else {
        *value = (uint8_t)(graphic[byte_index] >> 4);
    }
    (*nibble_index)++;
    return 1;
}

static int read_pixel_count(const uint8_t *graphic,
                            size_t graphic_size,
                            size_t *nibble_index,
                            size_t *count)
{
    uint8_t nibble;
    uint8_t high;
    uint8_t low;
    size_t value;

    if (!read_nibble(graphic, graphic_size, nibble_index, &nibble)) {
        return 0;
    }
    if (nibble != 15U) {
        *count = (size_t)nibble + 2U;
        return 1;
    }
    if (!read_nibble(graphic, graphic_size, nibble_index, &high) ||
        !read_nibble(graphic, graphic_size, nibble_index, &low)) {
        return 0;
    }
    value = ((size_t)high << 4) | low;
    if (value != 255U) {
        *count = value + 17U;
        return 1;
    }
    value = 0U;
    for (unsigned int shift = 12U; ; shift -= 4U) {
        if (!read_nibble(graphic, graphic_size, nibble_index, &nibble)) {
            return 0;
        }
        value |= (size_t)nibble << shift;
        if (shift == 0U) {
            break;
        }
    }
    *count = value;
    return 1;
}

/* Validate the exact command grammar first, so a malformed stream cannot
 * produce an observable partial draw. ReDMCSB stops once its logical image
 * index reaches width * height; valid PC assets end exactly there. */
static int validate_stream(const uint8_t *graphic,
                           size_t graphic_size,
                           size_t total_pixels)
{
    size_t nibble_index = 14U;
    size_t pixels = 0U;

    while (pixels < total_pixels) {
        uint8_t command;
        uint8_t kind;
        size_t count;

        if (!read_nibble(graphic, graphic_size, &nibble_index, &command)) {
            return 0;
        }
        kind = (uint8_t)(command & 0x07U);
        if (kind == 7U &&
            !read_nibble(graphic, graphic_size, &nibble_index, &kind)) {
            return 0;
        }
        if ((command & 0x08U) != 0U) {
            if (!read_pixel_count(graphic, graphic_size, &nibble_index,
                                  &count)) {
                return 0;
            }
        } else {
            count = 1U;
        }
        if (count == 0U || count > total_pixels - pixels) {
            return 0;
        }
        pixels += count;
    }
    return 1;
}

/* ReDMCSB IMAGE4.C F0685_IMG3_LineColorFilling for the packed PC bitmap. */
static void fill_packed_pixels(uint8_t *pixel_line,
                               size_t pixel_index,
                               uint8_t color,
                               size_t count)
{
    color &= 0x0fU;
    while (count-- != 0U) {
        size_t byte_index = pixel_index >> 1;

        if ((pixel_index & 1U) == 0U) {
            pixel_line[byte_index] =
                (uint8_t)((pixel_line[byte_index] & 0x0fU) | (color << 4));
        } else {
            pixel_line[byte_index] =
                (uint8_t)((pixel_line[byte_index] & 0xf0U) | color);
        }
        pixel_index++;
    }
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
    uint16_t width;
    uint16_t height;
    uint8_t local_palette[6];
    size_t total_pixels;
    size_t row_pixels_remaining;
    size_t row_pixel_index = 0U;
    size_t destination_pixel_index;
    size_t nibble_index = 8U;

    if (graphic == NULL || graphic_size < 7U || pixel_line == NULL ||
        sink == NULL || x < 0 || y < 0) {
        return 0;
    }
    width = (uint16_t)((uint16_t)graphic[0] | ((uint16_t)graphic[1] << 8));
    height = (uint16_t)((uint16_t)graphic[2] | ((uint16_t)graphic[3] << 8));
    if (width == 0U || width > 320U || height == 0U ||
        (size_t)x + width > 320U ||
        pixel_line_size < ((size_t)width + 1U) / 2U ||
        (size_t)height > SIZE_MAX / width ||
        (size_t)y > (SIZE_MAX - (size_t)x) / 320U) {
        return 0;
    }
    total_pixels = (size_t)width * height;
    if (!validate_stream(graphic, graphic_size, total_pixels)) {
        return 0;
    }

    for (size_t index = 0U; index < 6U; index++) {
        if (!read_nibble(graphic, graphic_size, &nibble_index,
                         &local_palette[index])) {
            return 0;
        }
    }

    row_pixels_remaining = width;
    destination_pixel_index = (size_t)y * 320U + (size_t)x;
    while (row_pixel_index < total_pixels) {
        uint8_t command;
        uint8_t kind;
        uint8_t color = 0U;
        size_t count;

        (void)read_nibble(graphic, graphic_size, &nibble_index, &command);
        kind = (uint8_t)(command & 0x07U);
        if (kind == 7U) {
            (void)read_nibble(graphic, graphic_size, &nibble_index, &color);
        } else if (kind < 6U) {
            color = local_palette[kind];
        }
        if ((command & 0x08U) != 0U) {
            (void)read_pixel_count(graphic, graphic_size, &nibble_index,
                                   &count);
        } else {
            count = 1U;
        }

        while (count >= row_pixels_remaining) {
            if (kind != 6U) {
                fill_packed_pixels(pixel_line, row_pixel_index % width,
                                   color, row_pixels_remaining);
            }
            sink(sink_context, pixel_line, destination_pixel_index, width);
            row_pixel_index += row_pixels_remaining;
            count -= row_pixels_remaining;
            destination_pixel_index += 320U;
            row_pixels_remaining = width;
        }
        if (count != 0U) {
            if (kind != 6U) {
                fill_packed_pixels(pixel_line, row_pixel_index % width,
                                   color, count);
            }
            row_pixel_index += count;
            row_pixels_remaining -= count;
        }
    }
    return 1;
}
