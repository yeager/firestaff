#include "redmcsb_f0684_blit_c25_pc34_compat.h"
#include "redmcsb_f0681_copy_pixel_line_flipped_pc34_compat.h"
#include "redmcsb_f0682_copy_pixel_line_transparent_pc34_compat.h"
#include "redmcsb_f0683_c25_vga_flipped_transparent_copy_pc34_compat.h"

static int32_t redmcsb_f0684_even_integer(int16_t value)
{
    return ((int32_t)value + 1) & ~1;
}

bool redmcsb_f0684_blit_c25_pc34_compat(
    const uint8_t *source, size_t source_byte_count,
    RedmcsbF0680C25VgaAperturePc34Compat *destination,
    const RedmcsbF0684BoxPc34Compat *box, int16_t source_x,
    int16_t source_y, int16_t source_width, int16_t destination_width,
    int16_t transparent_color, int16_t flip, uint8_t viewport_color_index_offset)
{
    int32_t source_stride, destination_stride, width, height, row;
    bool horizontal, vertical;

    if (source == NULL || destination == NULL || destination->bytes == NULL || box == NULL ||
        source_x < 0 || source_y < 0 || source_width <= 0 || destination_width <= 0 ||
        box->left < 0 || box->top < 0 || box->right < box->left || box->bottom < box->top ||
        transparent_color > 15 || flip < REDMCSB_F0684_FLIP_NONE_PC34 ||
        flip > REDMCSB_F0684_FLIP_BOTH_PC34) return false;

    /* DEFS.H:3419 M104_EVEN_INTEGER rounds odd rows up, never down. */
    source_stride = redmcsb_f0684_even_integer(source_width);
    destination_stride = redmcsb_f0684_even_integer(destination_width);
    width = (int32_t)box->right - (int32_t)box->left + 1;
    height = (int32_t)box->bottom - (int32_t)box->top + 1;
    if (width <= 0 || height <= 0 || source_x + width > source_stride ||
        box->left + width > destination_stride) return false;

    horizontal = (flip & REDMCSB_F0684_FLIP_HORIZONTAL_PC34) != 0;
    vertical = (flip & REDMCSB_F0684_FLIP_VERTICAL_PC34) != 0;
    for (row = 0; row < height; ++row) {
        const int32_t source_row = vertical ? source_y + height - 1 - row : source_y + row;
        const size_t source_index = (size_t)source_row * (size_t)source_stride + (size_t)source_x;
        const size_t destination_index = (size_t)(box->top + row) * (size_t)destination_stride + (size_t)box->left;
        bool copied;
        if (transparent_color < 0) {
            copied = horizontal
                ? redmcsb_f0681_copy_pixel_line_flipped_pc34_compat(source, source_byte_count, source_index, destination, destination_index, (size_t)width, viewport_color_index_offset)
                : redmcsb_f0680_copy_pixels_to_screen_pc34_compat(source, source_byte_count, source_index, destination, destination_index, (size_t)width, viewport_color_index_offset);
        } else if (horizontal) {
            copied = redmcsb_f0683_c25_vga_flipped_transparent_copy_pc34_compat(source, source_byte_count, source_index, destination->bytes, destination->byte_count, destination_index, (size_t)width, (uint8_t)transparent_color, viewport_color_index_offset) != 0;
        } else {
            copied = redmcsb_f0682_copy_pixel_line_transparent_pc34_compat(source, source_byte_count, source_index, (RedmcsbF0682C25VgaAperturePc34Compat *)destination, destination_index, (size_t)width, (uint8_t)transparent_color, viewport_color_index_offset);
        }
        if (!copied) return false;
    }
    return true;
}

const char *redmcsb_f0684_blit_source_evidence_pc34(void)
{
    return "ReDMCSB IMAGE3.C:831-928 F0684 C25 source-to-screen route: M104 "
           "even strides, box rows, F0680/F0682 ordinary and F0681/F0683 flipped lines.";
}
