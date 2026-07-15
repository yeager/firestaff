#include "dm2_v1_anim_bootstrap.h"

#include <stdio.h>
#include <string.h>

static void dm2_v1_anim_bootstrap_clear(DM2_V1_AnimBootstrapReceipt *receipt)
{
    if (receipt) {
        memset(receipt, 0, sizeof(*receipt));
    }
}

static int dm2_v1_anim_bootstrap_set(DM2_V1_AnimBootstrapReceipt *receipt,
                                     const char *symbol,
                                     int source_line,
                                     int argc,
                                     const char *const *argv)
{
    int i;
    size_t used = 0;

    dm2_v1_anim_bootstrap_clear(receipt);
    if (!receipt || !symbol || !argv || argc <= 0 ||
        argc > DM2_V1_ANIM_BOOTSTRAP_MAX_ARGS) {
        return 0;
    }
    receipt->valid = 1;
    receipt->symbol = symbol;
    receipt->source_file = "SKWIN/SkWinCore.cpp";
    receipt->source_line = source_line;
    receipt->argc = argc;
    for (i = 0; i < argc; ++i) {
        int wrote;
        receipt->argv[i] = argv[i];
        wrote = snprintf(receipt->command_line + used,
                         sizeof(receipt->command_line) - used,
                         "%s%s", i ? " " : "", argv[i]);
        if (wrote < 0 || (size_t)wrote >= sizeof(receipt->command_line) - used) {
            dm2_v1_anim_bootstrap_clear(receipt);
            return 0;
        }
        used += (size_t)wrote;
    }
    return 1;
}

int dm2_v1_anim_bootstrap_swoosh(DM2_V1_AnimBootstrapReceipt *out_receipt)
{
    static const char *const argv[] = {"ANIM", "swoosh", "+pm", "+sb"};
    return dm2_v1_anim_bootstrap_set(out_receipt,
                                     "ANIM_BOOTSTRAP_SWOOSH",
                                     2034,
                                     4,
                                     argv);
}

int dm2_v1_anim_bootstrap_title(DM2_V1_AnimBootstrapReceipt *out_receipt)
{
    static const char *const argv[] = {
        "ANIM", "title", "+ah", "+as", "+ab", "+pm", "+sb"
    };
    return dm2_v1_anim_bootstrap_set(out_receipt,
                                     "ANIM_BOOTSTRAP_TITLE",
                                     2045,
                                     7,
                                     argv);
}

int dm2_v1_anim_setpixel_seq_4bpp(uint8_t *dst,
                                  size_t dst_size,
                                  uint16_t pixel_offset,
                                  uint8_t color)
{
    size_t byte_offset = (size_t)pixel_offset >> 1;
    uint8_t nibble = (uint8_t)(color & 0x0fu);

    if (!dst || byte_offset >= dst_size) {
        return 0;
    }
    if ((pixel_offset & 1u) != 0u) {
        dst[byte_offset] = (uint8_t)((dst[byte_offset] & 0xf0u) | nibble);
    } else {
        dst[byte_offset] = (uint8_t)((dst[byte_offset] & 0x0fu) |
                                     (uint8_t)(nibble << 4));
    }
    return 1;
}

int dm2_v1_anim_fill_seq_4bpp(uint8_t *dst,
                              size_t dst_size,
                              uint16_t pixel_offset,
                              uint8_t color,
                              uint16_t count)
{
    uint16_t i;

    if (count == 0u) {
        return 0;
    }
    for (i = 0; i < count; ++i) {
        if (!dm2_v1_anim_setpixel_seq_4bpp(
                dst, dst_size, (uint16_t)(pixel_offset + i), color)) {
            return 0;
        }
    }
    return 1;
}

static int dm2_v1_anim_get_nibble(const uint8_t *src,
                                  size_t src_size,
                                  size_t pixel_offset,
                                  uint8_t *out_color)
{
    size_t byte_offset = pixel_offset >> 1;
    if (!src || !out_color || byte_offset >= src_size) {
        return 0;
    }
    if ((pixel_offset & 1u) != 0u) {
        *out_color = (uint8_t)(src[byte_offset] & 0x0fu);
    } else {
        *out_color = (uint8_t)((src[byte_offset] >> 4) & 0x0fu);
    }
    return 1;
}

static int dm2_v1_anim_copy_literal_nibbles(const uint8_t *src,
                                            size_t src_size,
                                            size_t src_offset,
                                            uint8_t *dst,
                                            size_t dst_size,
                                            uint16_t pixel_offset,
                                            uint16_t count)
{
    uint16_t i;
    for (i = 0; i < count; ++i) {
        uint8_t color;
        if (!dm2_v1_anim_get_nibble(src, src_size, (size_t)i, &color) ||
            !dm2_v1_anim_setpixel_seq_4bpp(
                dst, dst_size, (uint16_t)(pixel_offset + i), color)) {
            return 0;
        }
    }
    (void)src_offset;
    return 1;
}

int dm2_v1_anim_decode_img1(const uint8_t *src,
                            size_t src_size,
                            uint8_t *dst,
                            size_t dst_size,
                            DM2_V1_AnimDecodeImg1Receipt *out_receipt)
{
    size_t pos = 4;
    uint16_t width;
    uint16_t height;
    uint16_t even_width;
    uint32_t total_pixels;
    uint32_t di = 0;
    uint32_t previous_flush = 0;
    DM2_V1_AnimDecodeImg1Receipt receipt;

    memset(&receipt, 0, sizeof(receipt));
    if (out_receipt) {
        memset(out_receipt, 0, sizeof(*out_receipt));
    }
    if (!src || !dst || src_size < 4u) {
        return 0;
    }
    width = (uint16_t)(((uint16_t)src[0] << 8) | src[1]);
    height = (uint16_t)(((uint16_t)src[2] << 8) | src[3]);
    even_width = (uint16_t)((width + 1u) & 0xfffeu);
    total_pixels = (uint32_t)even_width * (uint32_t)height;
    if (width == 0u || height == 0u ||
        ((total_pixels + 1u) >> 1) > dst_size) {
        return 0;
    }

    while (di < total_pixels) {
        uint8_t op;
        uint16_t count;
        if (pos >= src_size) {
            return 0;
        }
        op = src[pos++];
        if ((op & 0x80u) == 0u) {
            count = (uint16_t)((op >> 4) + 1u);
            if (di + count > total_pixels ||
                !dm2_v1_anim_fill_seq_4bpp(
                    dst, dst_size, (uint16_t)di, op, count)) {
                return 0;
            }
            di += count;
            ++receipt.fill_run_count;
            continue;
        }

        switch (op & 0x30u) {
        case 0x00u:
            if ((op & 0x40u) == 0u) {
                if (pos >= src_size) return 0;
                count = (uint16_t)(src[pos++] + 1u);
            } else {
                if (pos + 1u >= src_size) return 0;
                count = (uint16_t)(((uint16_t)src[pos] << 8) |
                                   src[pos + 1u]);
                pos += 2u;
                ++count;
            }
            if (di + count > total_pixels ||
                !dm2_v1_anim_fill_seq_4bpp(
                    dst, dst_size, (uint16_t)di, op, count)) {
                return 0;
            }
            di += count;
            ++receipt.fill_run_count;
            break;
        case 0x10u:
            if ((op & 0x40u) == 0u) {
                if (pos >= src_size) return 0;
                count = (uint16_t)(src[pos++] + 1u);
            } else {
                if (pos + 1u >= src_size) return 0;
                count = (uint16_t)(((uint16_t)src[pos] << 8) |
                                   src[pos + 1u]);
                pos += 2u;
                ++count;
            }
            if ((count & 1u) != 0u) {
                if (di >= total_pixels ||
                    !dm2_v1_anim_setpixel_seq_4bpp(
                        dst, dst_size, (uint16_t)di, op)) {
                    return 0;
                }
                ++di;
                --count;
            }
            if (di + count > total_pixels ||
                pos + ((size_t)count >> 1) > src_size ||
                !dm2_v1_anim_copy_literal_nibbles(
                    src + pos, src_size - pos, pos, dst, dst_size,
                    (uint16_t)di, count)) {
                return 0;
            }
            pos += (size_t)count >> 1;
            di += count;
            ++receipt.literal_run_count;
            break;
        case 0x20u:
            count = (uint16_t)(((op >> 2) & 16u) | (op & 15u));
            if (count == 0x1du) {
                if (pos >= src_size) return 0;
                count = (uint16_t)(src[pos++] + 1u);
            } else if (count == 0x1eu) {
                if (pos >= src_size) return 0;
                count = (uint16_t)(src[pos++] + 0x101u);
            } else if (count == 0x1fu) {
                if (pos + 1u >= src_size) return 0;
                count = (uint16_t)(((uint16_t)src[pos] << 8) |
                                   src[pos + 1u]);
                pos += 2u;
                ++count;
            } else {
                ++count;
            }
            if (di + count > total_pixels) {
                return 0;
            }
            di += count;
            previous_flush = di;
            ++receipt.skip_run_count;
            break;
        case 0x30u:
            if ((op & 0x40u) == 0u) {
                if (pos >= src_size) return 0;
                count = (uint16_t)(src[pos++] + 1u);
            } else {
                if (pos + 1u >= src_size) return 0;
                count = (uint16_t)(((uint16_t)src[pos] << 8) |
                                   src[pos + 1u]);
                pos += 2u;
                ++count;
            }
            if (di < even_width || di + count + 1u > total_pixels) {
                return 0;
            }
            {
                uint16_t i;
                for (i = 0; i < count; ++i) {
                    uint8_t color;
                    if (!dm2_v1_anim_get_nibble(
                            dst, dst_size, (size_t)(di - even_width + i),
                            &color) ||
                        !dm2_v1_anim_setpixel_seq_4bpp(
                            dst, dst_size, (uint16_t)(di + i), color)) {
                        return 0;
                    }
                }
            }
            di += count;
            if (!dm2_v1_anim_setpixel_seq_4bpp(
                    dst, dst_size, (uint16_t)di, op)) {
                return 0;
            }
            ++di;
            ++receipt.previous_row_run_count;
            break;
        default:
            return 0;
        }
    }

    receipt.valid = 1;
    receipt.symbol = "ANIM_DECODE_IMG1";
    receipt.source_file = "SKWIN/SkWinCore.cpp";
    receipt.source_line = 1110;
    receipt.width = width;
    receipt.height = height;
    receipt.even_width = even_width;
    receipt.decoded_pixels = di;
    receipt.consumed_bytes = (uint32_t)pos;
    (void)previous_flush;
    if (out_receipt) {
        *out_receipt = receipt;
    }
    return 1;
}
