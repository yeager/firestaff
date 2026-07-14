#include "redmcsb_f1004_video_blit_shrink_with_palette_changes_pc34_compat.h"

#include <stddef.h>

static uint16_t redmcsb_f1004_even_pixel_width(uint16_t width)
{
    return (uint16_t)((width + UINT16_C(1)) & UINT16_C(0xfffe));
}

static uint8_t redmcsb_f1004_source_pixel(
    const uint8_t *bitmap_source,
    uint16_t pixel_offset)
{
    uint8_t packed_pixel = bitmap_source[pixel_offset >> 1u];

    return (pixel_offset & UINT16_C(1)) != 0u
               ? (uint8_t)(packed_pixel & UINT8_C(0x0f))
               : (uint8_t)(packed_pixel >> 4u);
}

void F1004_VIDEO_BlitShrinkWithPaletteChanges_PC34(
    const uint8_t *bitmap_source,
    uint8_t *bitmap_destination,
    uint16_t source_pixel_width,
    uint16_t source_height,
    int16_t destination_pixel_width,
    int16_t destination_height,
    const uint8_t *palette_changes)
{
    uint16_t destination_row;
    uint16_t destination_width = (uint16_t)destination_pixel_width;
    uint16_t destination_rows = (uint16_t)destination_height;
    uint16_t source_row_stride = redmcsb_f1004_even_pixel_width(source_pixel_width);
    uint16_t destination_row_stride =
        redmcsb_f1004_even_pixel_width(destination_width);
    uint16_t width_ratio = (uint16_t)(((uint32_t)source_pixel_width << 6u) /
                                      destination_width);
    uint16_t height_ratio = (uint16_t)(((uint32_t)source_height << 6u) /
                                       destination_rows);
    uint16_t source_y = (uint16_t)(height_ratio >> 1u);

    for (destination_row = 0u; destination_row < destination_rows;
         ++destination_row) {
        uint16_t destination_x;
        uint16_t source_x = (uint16_t)(width_ratio >> 1u);
        uint16_t source_row_offset =
            (uint16_t)((source_y >> 6u) * source_row_stride);
        uint16_t destination_row_offset =
            (uint16_t)(destination_row * destination_row_stride);

        for (destination_x = 0u; destination_x < destination_width;
             destination_x = (uint16_t)(destination_x + 2u)) {
            uint16_t source_pixel_offset =
                (uint16_t)(source_row_offset + (source_x >> 6u));
            uint8_t first_source_pixel = redmcsb_f1004_source_pixel(
                bitmap_source, source_pixel_offset);
            uint8_t first_output_pixel = palette_changes == NULL
                                             ? first_source_pixel
                                             : palette_changes[first_source_pixel];
            uint8_t second_source_pixel;
            uint8_t second_output_pixel;

            source_x = (uint16_t)(source_x + width_ratio);
            source_pixel_offset =
                (uint16_t)(source_row_offset + (source_x >> 6u));
            second_source_pixel = redmcsb_f1004_source_pixel(
                bitmap_source, source_pixel_offset);
            second_output_pixel = palette_changes == NULL
                                      ? second_source_pixel
                                      : palette_changes[second_source_pixel];
            bitmap_destination[(destination_row_offset + destination_x) >> 1u] =
                (uint8_t)((uint8_t)(first_output_pixel << 4u) |
                          second_output_pixel);
            source_x = (uint16_t)(source_x + width_ratio);
        }
        source_y = (uint16_t)(source_y + height_ratio);
    }
}

const char *redmcsb_f1004_video_blit_shrink_with_palette_changes_source_evidence(void)
{
    return "ReDMCSB BLTSHRNK.C:1556-1595, MEDIA458_P20JA_P20JB: "
           "F1004 copies the 16 palette bytes literally, writes output in "
           "high-nibble-first pixel pairs, rounds source and destination row "
           "widths with M104_EVEN_INTEGER, starts both "
           "fixed-point coordinates at ratio >> 1, and advances them by "
           "(source_dimension << 6) / destination_dimension. Unlike F0129, "
           "F1004 palette bytes are not divided by 10.";
}
