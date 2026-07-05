#include "csb_v1_viewport_pc34_compat.h"

static int csb_v1_apply_background3_pc34(
    const CSB_V1_ViewportCustomBackgroundMask *mask,
    const uint32_t *bitmap_words,
    size_t bitmap_word_count,
    uint32_t *viewport_words,
    size_t viewport_word_count,
    int viewport_width_pixels)
{
    uint32_t ssr;
    uint32_t ssl;
    uint32_t smr;
    uint32_t sml;
    uint32_t s_bit;
    uint32_t d_bit;
    int negative_words;
    int pre_pixel_count;
    int post_pixel_count;
    int num_full_groups;
    int pixels_per_line;
    int bitmap_width_pixels;
    int bitmap_word_stride;
    int viewport_word_stride;
    int src_line_incr;
    int dst_line_incr;
    int mask_line_incr;
    int copied_bytes = 0;
    size_t viewport_height;
    uint8_t *viewport_bytes = (uint8_t *)viewport_words;
    const uint8_t *source_bytes = (const uint8_t *)bitmap_words;
    const uint8_t *mask_bytes;

    /* CSBWin Viewport.cpp:6124-6442 ApplyBackground3 handles unaligned
     * CustomBackgrounds masks after ApplyBackground's alignment dispatch.
     * Keep the same unsupported SMR<8/SSR>=8 branch deferred because the
     * CSBWin source itself leaves that branch commented/NotImplemented. */
    if (!mask || !bitmap_words || !viewport_words || !mask->mask_words ||
        bitmap_word_count <= 1u || viewport_word_count == 0u ||
        viewport_width_pixels <= 0 || (viewport_width_pixels & 15) != 0 ||
        mask->src_x < 0 || mask->src_y < 0 ||
        mask->dst_y < 0 || mask->width <= 0 || mask->height <= 0 ||
        (mask->width & 15) != 0) {
        return 0;
    }
    mask_bytes = (const uint8_t *)mask->mask_words;

    bitmap_width_pixels = (int)(bitmap_words[0] & 0xffffu);
    if (bitmap_width_pixels <= 0 || (bitmap_width_pixels & 15) != 0) {
        return 0;
    }
    viewport_word_stride = viewport_width_pixels / 8;
    bitmap_word_stride = bitmap_width_pixels / 8;
    viewport_height = viewport_word_count / (size_t)viewport_word_stride;
    if (viewport_height == 0u ||
        (size_t)(mask->dst_y + mask->height) > viewport_height ||
        mask->dst_x >= viewport_width_pixels ||
        mask->dst_x + mask->width <= 0 ||
        mask->src_x + mask->width > bitmap_width_pixels ||
        mask->src_y + mask->height <= 0 ||
        (size_t)(mask->src_y + mask->height) >
            (bitmap_word_count - 1u) / (size_t)bitmap_word_stride) {
        return 0;
    }
    if (mask->mask_word_count <
        (size_t)(mask->width / 16) * (size_t)mask->height) {
        return 0;
    }

    s_bit = (uint32_t)(mask->src_x & 15);
    if (mask->dst_x < 0) {
        negative_words = (-mask->dst_x + 15) / 16;
    } else {
        negative_words = 0;
    }
    d_bit = (uint32_t)((mask->dst_x + negative_words * 16) & 15);
    smr = d_bit;
    sml = 16u - smr;
    ssr = d_bit - s_bit;
    if (s_bit > d_bit) {
        ssr += 16u;
    }
    ssl = 16u - ssr;
    pre_pixel_count = (int)d_bit;
    src_line_incr = bitmap_word_stride;
    dst_line_incr = viewport_word_stride;
    mask_line_incr = mask->width >> 4;
    pixels_per_line = mask->width;
    if (mask->dst_x + pixels_per_line > viewport_width_pixels) {
        pixels_per_line = viewport_width_pixels - mask->dst_x;
    }
    post_pixel_count = (pixels_per_line - (int)sml) & 15;
    num_full_groups = (pixels_per_line - (int)sml) / 16;
    if (negative_words) {
        num_full_groups -= negative_words - (pre_pixel_count ? 1 : 0);
        pre_pixel_count = 0;
    }
    if (num_full_groups < 0) {
        return 0;
    }
    if (smr < 8u && ssr >= 8u) {
        return -2;
    }

#define CSB_APPLY_BYTE(DST_, SRC_, MASK_) do { \
        uint8_t csb_m_ = (uint8_t)(MASK_); \
        uint8_t *csb_d_ = (DST_); \
        *csb_d_ = (uint8_t)(((uint8_t)(SRC_) & csb_m_) | \
                            (*csb_d_ & (uint8_t)(~csb_m_))); \
        ++copied_bytes; \
    } while (0)

    for (int row = 0; row < mask->height; ++row) {
        const uint8_t *src = source_bytes + 4u +
            ((size_t)src_line_incr * (size_t)(mask->src_y + row) +
             (size_t)(2 * (mask->src_x / 16))) * 4u;
        uint8_t *dst = viewport_bytes +
            ((size_t)dst_line_incr * (size_t)(mask->dst_y + row)) * 4u;
        const uint8_t *msk = mask_bytes +
            (size_t)mask_line_incr * (size_t)row * 2u;

        if (negative_words) {
            dst += (size_t)(negative_words * 2) * 4u;
            src += (size_t)(negative_words * 2) * 4u;
            msk += (size_t)negative_words * 2u;
        } else {
            dst += (size_t)(2 * (mask->dst_x / 16)) * 4u;
        }

        if (smr == 0u) {
            const uint8_t *s = src;
            uint8_t *d = dst;
            const uint8_t *m = msk;
            uint32_t local_ssl = ssl - 8u;
            for (int j = 0; j <= num_full_groups; ++j, m += 2) {
                uint8_t m0 = m[0];
                uint8_t m1 = m[1];
                for (uint32_t jj = 0; jj < 4u; ++jj, d += 2, s += 2) {
                    CSB_APPLY_BYTE(&d[0], (((uint16_t)s[1] << 8) | s[8]) >> ssr, m0);
                    CSB_APPLY_BYTE(&d[1], (((uint16_t)s[8] << 8) | s[9]) >> ssr, m1);
                }
            }
            if (post_pixel_count > 0) {
                uint8_t m0 = m[1];
                for (uint32_t jj = 0; jj < 4u; ++jj, d += 2, s += 2) {
                    CSB_APPLY_BYTE(&d[0], (uint8_t)(s[1] << local_ssl), m0);
                }
            }
        } else if (smr < 8u) {
            const uint8_t *s = src;
            uint8_t *d = dst;
            const uint8_t *m = msk;
            uint32_t local_sml = sml - 8u;
            uint32_t local_ssl = ssl - 8u;
            if (pre_pixel_count != 0) {
                uint8_t m0 = (uint8_t)(m[0] >> smr);
                uint8_t m1 = (uint8_t)((((uint16_t)m[0] << 8) | m[1]) >> smr);
                m += 2;
                for (uint32_t jj = 0; jj < 4u; ++jj, d += 2, s += 2) {
                    CSB_APPLY_BYTE(&d[0], s[0] >> ssr, m0);
                    CSB_APPLY_BYTE(&d[1], (((uint16_t)s[0] << 8) | s[1]) >> ssr, m1);
                }
            }
            s -= 8;
            d -= 8;
            m -= 2;
            for (int j = 0; j < num_full_groups; ++j, m += 2) {
                uint8_t m0 = (uint8_t)((((uint16_t)m[1] << 8) | m[2]) >> smr);
                uint8_t m1 = (uint8_t)((((uint16_t)m[2] << 8) | m[3]) >> smr);
                for (uint32_t jj = 0; jj < 4u; ++jj, d += 2, s += 2) {
                    CSB_APPLY_BYTE(&d[8], (((uint16_t)s[1] << 8) | s[8]) >> ssr, m0);
                    CSB_APPLY_BYTE(&d[9], (((uint16_t)s[8] << 8) | s[9]) >> ssr, m1);
                }
            }
            if (post_pixel_count > 0) {
                uint8_t m0 = (uint8_t)(m[1] << local_sml);
                for (uint32_t jj = 0; jj < 4u; ++jj, d += 2, s += 2) {
                    CSB_APPLY_BYTE(&d[8], (uint8_t)(s[1] << local_ssl), m0);
                }
            }
        } else {
            const uint8_t *s = src;
            uint8_t *d = dst;
            const uint8_t *m = msk;
            uint32_t local_smr = smr - 8u;
            if (ssr < 8u) {
                uint8_t m1 = (uint8_t)(m[0] >> local_smr);
                for (uint32_t jj = 0; jj < 4u; ++jj, d += 2, s += 2) {
                    CSB_APPLY_BYTE(&d[1], s[0] >> ssr, m1);
                }
                s -= 8;
                d -= 8;
                for (int j = 0; j < num_full_groups; ++j, m += 2) {
                    uint8_t m0 = (uint8_t)((((uint16_t)m[0] << 8) | m[1]) >> local_smr);
                    uint8_t m1b = (uint8_t)((((uint16_t)m[1] << 8) | m[2]) >> local_smr);
                    for (uint32_t jj = 0; jj < 4u; ++jj, d += 2, s += 2) {
                        CSB_APPLY_BYTE(&d[8], (((uint16_t)s[0] << 8) | s[1]) >> ssr, m0);
                        CSB_APPLY_BYTE(&d[9], (((uint16_t)s[1] << 8) | s[8]) >> ssr, m1b);
                    }
                }
                if (post_pixel_count != 0) {
                    uint8_t m0 = (uint8_t)((((uint16_t)m[0] << 8) | m[1]) >> local_smr);
                    uint8_t m1b = (uint8_t)(m[1] << sml);
                    for (uint32_t jj = 0; jj < 4u; ++jj, d += 2, s += 2) {
                        CSB_APPLY_BYTE(&d[8], (((uint16_t)s[0] << 8) | s[1]) >> ssr, m0);
                        CSB_APPLY_BYTE(&d[9], (((uint16_t)s[1] << 8) | s[8]) >> ssr, m1b);
                    }
                }
            } else {
                uint32_t local_ssr = ssr - 8u;
                if (pre_pixel_count != 0) {
                    uint8_t m1 = (uint8_t)(m[0] >> local_smr);
                    m += 2;
                    for (uint32_t jj = 0; jj < 4u; ++jj, d += 2, s += 2) {
                        CSB_APPLY_BYTE(&d[1], s[0] >> local_ssr, m1);
                    }
                }
                s -= 8;
                d -= 8;
                m -= 2;
                for (int j = 0; j < num_full_groups; ++j, m += 2) {
                    uint8_t m0 = (uint8_t)((((uint16_t)m[0] << 8) | m[1]) >> local_smr);
                    uint8_t m1b = (uint8_t)((((uint16_t)m[1] << 8) | m[2]) >> local_smr);
                    for (uint32_t jj = 0; jj < 4u; ++jj, d += 2, s += 2) {
                        CSB_APPLY_BYTE(&d[8], (((uint16_t)s[0] << 8) | s[1]) >> local_ssr, m0);
                        CSB_APPLY_BYTE(&d[9], (((uint16_t)s[1] << 8) | s[8]) >> local_ssr, m1b);
                    }
                }
                if (post_pixel_count != 0) {
                    uint8_t m0 = (uint8_t)((((uint16_t)m[0] << 8) | m[1]) >> local_smr);
                    uint8_t m1b = (uint8_t)(m[1] << sml);
                    for (uint32_t jj = 0; jj < 4u; ++jj, d += 2, s += 2) {
                        CSB_APPLY_BYTE(&d[8], (((uint16_t)s[0] << 8) | s[1]) >> local_ssr, m0);
                        CSB_APPLY_BYTE(&d[9], (((uint16_t)s[1] << 8) | s[8]) >> local_ssr, m1b);
                    }
                }
            }
        }
    }

#undef CSB_APPLY_BYTE
    return copied_bytes;
}

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

    bitmap_width_pixels = (int)(bitmap_words[0] & 0xffffu);
    if (bitmap_width_pixels <= 0 || (bitmap_width_pixels & 15) != 0) {
        return 0;
    }

    if (((mask->dst_x & 15) | (mask->src_x & 15)) != 0) {
        return csb_v1_apply_background3_pc34(mask,
                                             bitmap_words,
                                             bitmap_word_count,
                                             viewport_words,
                                             viewport_word_count,
                                             viewport_width_pixels);
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
