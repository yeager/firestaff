#include "redmcsb_f8151_vidrv_blit_pc34_compat.h"

#include "redmcsb_f0682_copy_pixel_line_transparent_pc34_compat.h"

bool redmcsb_f8151_vidrv_blit_pc34_compat(
    const uint8_t *source, size_t source_byte_count,
    RedmcsbF0680C25VgaAperturePc34Compat *destination,
    const RedmcsbF8151BoxPc34Compat *box,
    int16_t source_x, int16_t source_y,
    int16_t source_width, int16_t destination_width,
    int16_t transparent_color, int16_t flip,
    uint8_t viewport_color_index_offset)
{
    int32_t width;
    int32_t height;
    int32_t source_stride;
    int32_t destination_stride;
    int32_t row;
    bool horizontal;
    bool vertical;

    if (source == NULL || destination == NULL || box == NULL || source_x < 0 || source_y < 0 ||
        source_width <= 0 || destination_width <= 0 || box->left < 0 || box->top < 0 ||
        box->right < box->left || box->bottom < box->top || transparent_color > 15 ||
        flip < REDMCSB_F8151_FLIP_NONE_PC34 || flip > REDMCSB_F8151_FLIP_BOTH_PC34) {
        return false;
    }

    /* F8151 uses M104_EVEN_INTEGER for both line strides. */
    source_stride = ((int32_t)source_width + 1) & ~1;
    destination_stride = ((int32_t)destination_width + 1) & ~1;
    width = (int32_t)box->right - (int32_t)box->left + 1;
    height = (int32_t)box->bottom - (int32_t)box->top + 1;
    if (source_stride <= 0 || destination_stride <= 0 || width <= 0 || height <= 0 ||
        source_x + width > source_stride || box->left + width > destination_stride) {
        return false;
    }

    horizontal = (flip & REDMCSB_F8151_FLIP_HORIZONTAL_PC34) != 0;
    vertical = (flip & REDMCSB_F8151_FLIP_VERTICAL_PC34) != 0;
    /* VIDEODRV.C:1537-1542 and :2656-2660 leave C25 F0681/F0683 empty. */
    if (horizontal) {
        return true;
    }
    for (row = 0; row < height; ++row) {
        int32_t source_row = vertical ? source_y + height - 1 - row : source_y + row;
        size_t source_pixel_index = (size_t)source_row * (size_t)source_stride + (size_t)source_x;
        size_t destination_pixel_index = ((size_t)(box->top + row) * (size_t)destination_stride) +
                                         (size_t)box->left;
        bool copied;

        copied = transparent_color < 0
            ? redmcsb_f0680_copy_pixels_to_screen_pc34_compat(
                source, source_byte_count, source_pixel_index, destination,
                destination_pixel_index, (size_t)width, viewport_color_index_offset)
            : redmcsb_f0682_copy_pixel_line_transparent_pc34_compat(
                source, source_byte_count, source_pixel_index,
                (RedmcsbF0682C25VgaAperturePc34Compat *)destination,
                destination_pixel_index, (size_t)width, (uint8_t)transparent_color,
                viewport_color_index_offset);
        if (!copied) return false;
    }
    return true;
}

const char *redmcsb_f8151_vidrv_blit_source_evidence_pc34(void)
{
    return "ReDMCSB NEC816.C:1559-1704 and VIDEODRV.C:2943-3125, PC 3.4 "
           "C25_VGA F8151 source-to-screen branch; F0680/F0682 line routes, "
           "empty C25 F0681/F0683 flip branches, and M104 even strides.";
}
