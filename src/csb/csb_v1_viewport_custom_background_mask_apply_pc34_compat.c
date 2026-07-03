#include "csb_v1_viewport_pc34_compat.h"

int csb_v1_viewport_custom_background_apply_aligned_mask_pc34(
    const CSB_V1_ViewportCustomBackgroundMask *mask,
    const uint32_t *bitmap_words,
    size_t bitmap_word_count,
    uint32_t *viewport_words,
    size_t viewport_word_count,
    int viewport_width_pixels)
{
    const int viewport_word_stride = viewport_width_pixels / 8;
    int bitmap_width_pixels;
    int bitmap_word_stride;
    int src_x_words;
    int dst_x_words;
    int width_words;
    size_t src_last_word;
    size_t dst_last_word;
    size_t mask_words_needed;
    int copied_words = 0;

    /* ReDMCSB: DUNVIEW.C F0098 lines 2962-3002 has no masked overlay path;
     * CSBWin Viewport.cpp lines 6444-6470 adds ApplyBackground. This helper
     * source-locks the aligned branch: width must be 16-pixel aligned, srcX
     * and dstX must be 16-pixel aligned, and each 16-bit mask word is
     * expanded to both halves of the two 32-bit viewport words it controls. */
    if (!mask || !bitmap_words || !viewport_words ||
        bitmap_word_count == 0 || viewport_word_count == 0 ||
        viewport_width_pixels <= 0 || (viewport_width_pixels & 15) != 0 ||
        mask->src_x < 0 || mask->src_y < 0 ||
        mask->dst_y < 0 || mask->width <= 0 || mask->height <= 0 ||
        mask->dst_x > viewport_width_pixels ||
        (mask->width & 15) != 0) {
        return 0;
    }

    if (((mask->dst_x & 15) | (mask->src_x & 15)) != 0) {
        return -2;
    }

    bitmap_width_pixels = (int)(bitmap_words[0] & 0xffffu);
    if (bitmap_width_pixels <= 0 || (bitmap_width_pixels & 15) != 0) {
        return 0;
    }

    bitmap_word_stride = bitmap_width_pixels / 8;
    src_x_words = (mask->src_x / 16) * 2;
    dst_x_words = (mask->dst_x / 16) * 2;
    width_words = (mask->width / 16) * 2;
    mask_words_needed = (size_t)(mask->width / 16) * (size_t)mask->height;

    if (!mask->mask_words || mask->mask_word_count < mask_words_needed) {
        return 0;
    }
    if (mask->dst_x < 0 || mask->dst_x + mask->width > viewport_width_pixels) {
        return 0;
    }

    src_last_word = 1u +
        (size_t)bitmap_word_stride * (size_t)(mask->src_y + mask->height - 1) +
        (size_t)src_x_words + (size_t)width_words;
    dst_last_word =
        (size_t)viewport_word_stride * (size_t)(mask->dst_y + mask->height - 1) +
        (size_t)dst_x_words + (size_t)width_words;

    if (src_last_word > bitmap_word_count || dst_last_word > viewport_word_count) {
        return 0;
    }

    for (int row = 0; row < mask->height; ++row) {
        const uint32_t *src_row = bitmap_words + 1u +
            (size_t)bitmap_word_stride * (size_t)(mask->src_y + row) +
            (size_t)src_x_words;
        uint32_t *dst_row = viewport_words +
            (size_t)viewport_word_stride * (size_t)(mask->dst_y + row) +
            (size_t)dst_x_words;
        const uint16_t *mask_row = mask->mask_words +
            (size_t)(mask->width / 16) * (size_t)row;

        for (int col = 0; col < width_words; col += 2) {
            uint32_t m = (uint32_t)mask_row[col / 2];
            uint32_t expanded_mask = m | (m << 16);
            uint32_t keep_mask = ~expanded_mask;

            dst_row[col] = (dst_row[col] & keep_mask) |
                           (src_row[col] & expanded_mask);
            dst_row[col + 1] = (dst_row[col + 1] & keep_mask) |
                               (src_row[col + 1] & expanded_mask);
            copied_words += 2;
        }
    }

    return copied_words;
}
