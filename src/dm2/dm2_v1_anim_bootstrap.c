#include "dm2_v1_anim_bootstrap.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define DM2_V1_ANIM_MAX_FILE_HANDLES 16
#define DM2_V1_ANIM_READ_CHUNK_SIZE 0x8000u

static FILE *dm2_v1_anim_files[DM2_V1_ANIM_MAX_FILE_HANDLES];

static void dm2_v1_anim_file_receipt_set(DM2_V1_AnimFileReceipt *receipt,
                                         const char *symbol,
                                         int source_line,
                                         int handle)
{
    if (receipt) {
        memset(receipt, 0, sizeof(*receipt));
        receipt->valid = 1;
        receipt->symbol = symbol;
        receipt->source_file = "SKWIN/SkWinCore.cpp";
        receipt->source_line = source_line;
        receipt->handle = handle;
    }
}

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

int dm2_v1_anim_file_open(const char *filename,
                          DM2_V1_AnimFileReceipt *out_receipt)
{
    int i;

    if (out_receipt) {
        memset(out_receipt, 0, sizeof(*out_receipt));
    }
    if (!filename) {
        return 0;
    }
    for (i = 1; i < DM2_V1_ANIM_MAX_FILE_HANDLES; ++i) {
        if (!dm2_v1_anim_files[i]) {
            FILE *fp = fopen(filename, "rb");
            if (!fp) {
                return 0;
            }
            dm2_v1_anim_files[i] = fp;
            dm2_v1_anim_file_receipt_set(
                out_receipt, "ANIM_FILE_OPEN", 915, i);
            return i;
        }
    }
    return 0;
}

uint32_t dm2_v1_anim_get_file_size(int handle,
                                   DM2_V1_AnimFileReceipt *out_receipt)
{
    FILE *fp;
    long current;
    long size;

    if (out_receipt) {
        memset(out_receipt, 0, sizeof(*out_receipt));
    }
    if (handle <= 0 || handle >= DM2_V1_ANIM_MAX_FILE_HANDLES ||
        !dm2_v1_anim_files[handle]) {
        return 0;
    }
    fp = dm2_v1_anim_files[handle];
    current = ftell(fp);
    if (current < 0 || fseek(fp, 0, SEEK_END) != 0) {
        return 0;
    }
    size = ftell(fp);
    if (size < 0 || fseek(fp, current, SEEK_SET) != 0) {
        return 0;
    }
    dm2_v1_anim_file_receipt_set(
        out_receipt, "ANIM_GET_FILE_SIZE", 924, handle);
    if (out_receipt) {
        out_receipt->file_size = (uint32_t)size;
    }
    return (uint32_t)size;
}

int dm2_v1_anim_read_huge_file(int handle,
                               uint32_t read_size,
                               uint8_t *buffer,
                               DM2_V1_AnimFileReceipt *out_receipt)
{
    FILE *fp;
    uint8_t *cursor = buffer;
    uint32_t remaining = read_size;
    uint32_t chunks = 0;

    if (out_receipt) {
        memset(out_receipt, 0, sizeof(*out_receipt));
    }
    if (handle <= 0 || handle >= DM2_V1_ANIM_MAX_FILE_HANDLES ||
        !dm2_v1_anim_files[handle] || (!buffer && read_size != 0u)) {
        return 0;
    }
    fp = dm2_v1_anim_files[handle];
    while (remaining > 0u) {
        uint32_t chunk = remaining > DM2_V1_ANIM_READ_CHUNK_SIZE
                             ? DM2_V1_ANIM_READ_CHUNK_SIZE
                             : remaining;
        if (fread(cursor, 1, chunk, fp) != chunk) {
            return 0;
        }
        cursor += chunk;
        remaining -= chunk;
        ++chunks;
    }
    dm2_v1_anim_file_receipt_set(
        out_receipt, "ANIM_READ_HUGE_FILE", 939, handle);
    if (out_receipt) {
        out_receipt->requested_bytes = read_size;
        out_receipt->transferred_bytes = read_size;
        out_receipt->chunk_count = chunks;
    }
    return 1;
}

void dm2_v1_anim_file_close(int handle,
                            DM2_V1_AnimFileReceipt *out_receipt)
{
    if (out_receipt) {
        memset(out_receipt, 0, sizeof(*out_receipt));
    }
    if (handle > 0 && handle < DM2_V1_ANIM_MAX_FILE_HANDLES &&
        dm2_v1_anim_files[handle]) {
        fclose(dm2_v1_anim_files[handle]);
        dm2_v1_anim_files[handle] = NULL;
        dm2_v1_anim_file_receipt_set(
            out_receipt, "ANIM_FILE_CLOSE", 970, handle);
    }
}

char *dm2_v1_anim_strcpy(char *dst,
                         const char *src,
                         DM2_V1_AnimFileReceipt *out_receipt)
{
    if (!dst || !src) {
        if (out_receipt) {
            memset(out_receipt, 0, sizeof(*out_receipt));
        }
        return NULL;
    }
    dm2_v1_anim_file_receipt_set(out_receipt, "ANIM_STRCPY", 984, 0);
    return strcpy(dst, src);
}

int dm2_v1_anim_toupper(int value,
                        DM2_V1_AnimFileReceipt *out_receipt)
{
    if (value == -1) {
        dm2_v1_anim_file_receipt_set(out_receipt, "ANIM_TOUPPER", 1308, 0);
        return -1;
    }
    dm2_v1_anim_file_receipt_set(out_receipt, "ANIM_TOUPPER", 1308, 0);
    return (unsigned char)toupper((unsigned char)value);
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

int dm2_v1_anim_blit_to_memory_row_4to4bpp(const uint8_t *src,
                                           size_t src_size,
                                           uint16_t off_src,
                                           uint8_t *dst,
                                           size_t dst_size,
                                           uint16_t off_dst,
                                           uint16_t width,
                                           DM2_V1_AnimFileReceipt *out_receipt)
{
    uint16_t i;

    if (out_receipt) {
        memset(out_receipt, 0, sizeof(*out_receipt));
    }
    if (!src || !dst || width == 0u) {
        return 0;
    }
    for (i = 0; i < width; ++i) {
        uint8_t color;
        if (!dm2_v1_anim_get_nibble(src,
                                    src_size,
                                    (size_t)off_src + i,
                                    &color) ||
            !dm2_v1_anim_setpixel_seq_4bpp(dst,
                                           dst_size,
                                           (uint16_t)(off_dst + i),
                                           color)) {
            return 0;
        }
    }
    dm2_v1_anim_file_receipt_set(
        out_receipt, "ANIM_BLIT_TO_MEMORY_ROW_4TO4BPP", 1399, 0);
    if (out_receipt) {
        out_receipt->requested_bytes = width;
        out_receipt->transferred_bytes = width;
    }
    return 1;
}
