#include "redmcsb_f0550_video_fill_screen_box_pc34_compat.h"

#include <limits.h>
#include <string.h>

static uint16_t read_be16(const uint8_t *address)
{
    return (uint16_t)(((uint16_t)address[0] << 8) | address[1]);
}

static void write_be16(uint8_t *address, uint16_t value)
{
    address[0] = (uint8_t)(value >> 8);
    address[1] = (uint8_t)value;
}

static void read_box(const void *box, bool use_byte_box_coordinates,
                     int coordinates[4])
{
    if (use_byte_box_coordinates) {
        const uint8_t *byte_box = box;

        coordinates[0] = byte_box[0];
        coordinates[1] = byte_box[1];
        coordinates[2] = byte_box[2];
        coordinates[3] = byte_box[3];
    } else {
        int16_t word_box[4];

        memcpy(word_box, box, sizeof(word_box));
        coordinates[0] = word_box[0];
        coordinates[1] = word_box[1];
        coordinates[2] = word_box[2];
        coordinates[3] = word_box[3];
    }
}

bool F0550_VIDEO_FillScreenBox_PC34(
    uint8_t *bitmap,
    size_t bitmap_size,
    size_t byte_width,
    size_t pixel_height,
    const void *box,
    bool use_byte_box_coordinates,
    uint16_t color)
{
    int coordinates[4];
    size_t pixel_width;
    size_t required_size;
    int y;
    const unsigned int color_index = color & 0x000fu;
    const bool shade = (color & 0x8000u) != 0u;

    if (bitmap == NULL || box == NULL || byte_width == 0u ||
        (byte_width % 8u) != 0u || pixel_height == 0u ||
        byte_width > SIZE_MAX / 2u ||
        pixel_height > SIZE_MAX / byte_width) {
        return false;
    }

    pixel_width = byte_width * 2u;
    required_size = byte_width * pixel_height;
    if (bitmap_size < required_size || pixel_width > (size_t)INT_MAX ||
        pixel_height > (size_t)INT_MAX) {
        return false;
    }

    read_box(box, use_byte_box_coordinates, coordinates);
    if (coordinates[0] < 0 || coordinates[2] < 0 ||
        coordinates[0] > coordinates[1] || coordinates[2] > coordinates[3] ||
        (size_t)coordinates[1] >= pixel_width ||
        (size_t)coordinates[3] >= pixel_height) {
        return false;
    }

    for (y = coordinates[2]; y <= coordinates[3]; ++y) {
        int x;

        for (x = coordinates[0]; x <= coordinates[1]; ++x) {
            size_t word_offset;
            uint16_t pixel_mask;
            unsigned int plane;

            /* VIDEO.C toggles its 0x5555/0xAAAA mask for every scanline. */
            if (shade && ((x + y) & 1) == 0) {
                continue;
            }

            word_offset = (size_t)y * byte_width + (size_t)(x / 16) * 8u;
            pixel_mask = (uint16_t)(0x8000u >> (x & 15));
            for (plane = 0u; plane < 4u; ++plane) {
                uint8_t *plane_word = bitmap + word_offset + plane * 2u;
                uint16_t value = read_be16(plane_word);

                if ((color_index & (1u << plane)) != 0u) {
                    value |= pixel_mask;
                } else {
                    value &= (uint16_t)~pixel_mask;
                }
                write_be16(plane_word, value);
            }
        }
    }

    return true;
}
