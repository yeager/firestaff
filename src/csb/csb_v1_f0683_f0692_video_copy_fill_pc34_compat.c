#include "f0683_copy_pixel_line_to_screen_with_transparency_flipped_horizontally_pc34_compat.h"
#include "redmcsb_f0692_fillbox_image3_pc34_compat.h"

#include <stdbool.h>
#include <stdint.h>

static uint8_t read_packed_4bpp_pixel(const uint8_t *bitmap, size_t pixel_index)
{
    const uint8_t packed = bitmap[pixel_index / 2u];

    if ((pixel_index & 1u) == 0u) {
        return (uint8_t)(packed >> 4);
    }
    return (uint8_t)(packed & 0x0fu);
}

static void write_packed_4bpp_pixel(
    uint8_t *bitmap,
    size_t pixel_index,
    uint8_t color)
{
    uint8_t *packed = bitmap + pixel_index / 2u;

    if ((pixel_index & 1u) == 0u) {
        *packed = (uint8_t)((*packed & 0x0fu) | (uint8_t)(color << 4));
    } else {
        *packed = (uint8_t)((*packed & 0xf0u) | (color & 0x0fu));
    }
}

static bool packed_range_is_valid(
    size_t byte_count,
    size_t pixel_index,
    size_t pixel_count)
{
    size_t available_pixels;

    if (byte_count > SIZE_MAX / 2u) {
        return false;
    }
    available_pixels = byte_count * 2u;
    return pixel_index <= available_pixels &&
           pixel_count <= available_pixels - pixel_index;
}

int f0683_copy_pixel_line_to_screen_with_transparency_flipped_horizontally_pc34_compat(
    const uint8_t *source,
    size_t source_bytes,
    size_t source_pixel_index,
    uint8_t *destination,
    size_t destination_bytes,
    size_t destination_pixel_index,
    size_t pixel_count,
    uint8_t transparent_color)
{
    size_t index;

    if (source == 0 || destination == 0 || transparent_color > 0x0fu ||
        !packed_range_is_valid(source_bytes, source_pixel_index, pixel_count) ||
        !packed_range_is_valid(destination_bytes,
                               destination_pixel_index,
                               pixel_count)) {
        return 0;
    }

    for (index = 0u; index < pixel_count; index++) {
        const size_t flipped_source_index =
            source_pixel_index + pixel_count - 1u - index;
        const uint8_t pixel = read_packed_4bpp_pixel(
            source,
            flipped_source_index);

        if (pixel != transparent_color) {
            write_packed_4bpp_pixel(
                destination,
                destination_pixel_index + index,
                pixel);
        }
    }
    return 1;
}

int F0683_CopyPixelLineToScreenWithTransparencyFlippedHorizontally(
    const uint8_t *source,
    size_t source_bytes,
    size_t source_pixel_index,
    uint8_t *destination,
    size_t destination_bytes,
    size_t destination_pixel_index,
    size_t pixel_count,
    uint8_t transparent_color)
{
    return f0683_copy_pixel_line_to_screen_with_transparency_flipped_horizontally_pc34_compat(
        source,
        source_bytes,
        source_pixel_index,
        destination,
        destination_bytes,
        destination_pixel_index,
        pixel_count,
        transparent_color);
}

const char *f0683_copy_pixel_line_to_screen_with_transparency_flipped_horizontally_pc34_compat_source_evidence(void)
{
    return "ReDMCSB IMAGE3.C:612-829 "
           "F0683_CopyPixelLineToScreenWithTransparencyFlippedHorizontally";
}

static int f0692_fill_impl(
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

    if (bitmap == 0 || box == 0 || row_bytes == 0u || pixel_height == 0u ||
        row_bytes > SIZE_MAX / 2u ||
        pixel_height > SIZE_MAX / row_bytes) {
        return 0;
    }
    required_size = row_bytes * pixel_height;
    pixel_width = row_bytes * 2u;
    if (bitmap_size < required_size || pixel_width > (size_t)INT16_MAX ||
        pixel_height > (size_t)INT16_MAX) {
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

    for (y = top; y <= bottom; y++) {
        int x;
        const size_t row_pixel_index = (size_t)y * pixel_width;

        for (x = left; x <= right; x++) {
            write_packed_4bpp_pixel(bitmap, row_pixel_index + (size_t)x, pixel);
        }
    }
    return 1;
}

int F0692_FillBox(
    uint8_t *bitmap,
    size_t bitmap_size,
    size_t row_bytes,
    size_t pixel_height,
    const int16_t box[4],
    uint16_t color)
{
    return f0692_fill_impl(
        bitmap,
        bitmap_size,
        row_bytes,
        pixel_height,
        box,
        color);
}

const char *redmcsb_f0692_fillbox_image3_pc34_compat_source_evidence(void)
{
    return "ReDMCSB IMAGE3.C F0692_FillBox; CEDT027.C:1124";
}
