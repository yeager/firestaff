#include "csb_v1_startup_img3_decode_pc34_compat.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

/*
 * CSB Amiga v3.1 GRAPHICS.DAT uses dmweb "IMG1" encoding: nibble-based
 * RLE with big-endian width/height header.  The previous ExpandGraphic
 * byte-format decoder was for a different (CSBWin-specific) format and
 * produced diagonal distortion on large images.
 *
 * IMG1 reference: dmweb.free.fr "Data Files" documentation.
 */

typedef struct {
    const uint8_t *data;
    size_t data_size;
    size_t byte_pos;
    int nibble_phase;
} csb_v1_img1_nibble_reader_pc34;

static uint16_t csb_v1_startup_read_be16_pc34(const uint8_t *bytes)
{
    return (uint16_t)(((uint16_t)bytes[0] << 8) | bytes[1]);
}

static uint32_t csb_v1_startup_fnv1a_pc34(const uint8_t *bytes, size_t count)
{
    uint32_t hash = 2166136261u;
    size_t index;

    if (!bytes || count == 0U) return 0U;
    for (index = 0U; index < count; ++index) {
        hash ^= bytes[index];
        hash *= 16777619u;
    }
    return hash ? hash : 1U;
}

static int csb_v1_img1_has_nibble_pc34(
    const csb_v1_img1_nibble_reader_pc34 *reader)
{
    if (reader->nibble_phase == 0)
        return reader->byte_pos < reader->data_size;
    return 1;
}

static uint8_t csb_v1_img1_read_nibble_pc34(
    csb_v1_img1_nibble_reader_pc34 *reader)
{
    uint8_t byte_val;
    uint8_t nibble;

    if (reader->byte_pos >= reader->data_size) return 0U;
    byte_val = reader->data[reader->byte_pos];
    if (reader->nibble_phase == 0) {
        nibble = (uint8_t)((byte_val >> 4) & 0x0FU);
        reader->nibble_phase = 1;
    } else {
        nibble = (uint8_t)(byte_val & 0x0FU);
        reader->nibble_phase = 0;
        reader->byte_pos++;
    }
    return nibble;
}

static uint8_t csb_v1_img1_read_byte_pc34(
    csb_v1_img1_nibble_reader_pc34 *reader)
{
    uint8_t hi = csb_v1_img1_read_nibble_pc34(reader);
    uint8_t lo = csb_v1_img1_read_nibble_pc34(reader);
    return (uint8_t)((hi << 4) | lo);
}

static uint16_t csb_v1_img1_read_word_pc34(
    csb_v1_img1_nibble_reader_pc34 *reader)
{
    uint8_t hi = csb_v1_img1_read_byte_pc34(reader);
    uint8_t lo = csb_v1_img1_read_byte_pc34(reader);
    return (uint16_t)((hi << 8) | lo);
}

static int csb_v1_startup_indexed_region_has_visible_pixel_pc34(
    const uint8_t *indexed_pixels, uint16_t width, uint16_t top,
    uint16_t region_height)
{
    size_t offset;
    size_t count;
    size_t index;

    if (!indexed_pixels || width == 0U || region_height == 0U) return 0;
    offset = (size_t)top * width;
    count = (size_t)region_height * width;
    for (index = 0U; index < count; ++index) {
        if (indexed_pixels[offset + index] != 0U) return 1;
    }
    return 0;
}

int csb_v1_startup_title_c001_regions_admit_pc34_compat(
    const uint8_t *indexed_pixels, uint16_t width, uint16_t height)
{
    return indexed_pixels && width == 320U && height == 153U &&
        csb_v1_startup_indexed_region_has_visible_pixel_pc34(
            indexed_pixels, width, 0U, 80U) &&
        csb_v1_startup_indexed_region_has_visible_pixel_pc34(
            indexed_pixels, width, 80U, 57U) &&
        csb_v1_startup_indexed_region_has_visible_pixel_pc34(
            indexed_pixels, width, 137U, 16U);
}

int csb_v1_startup_img3_decode_to_indexed_with_receipt_pc34_compat(
    const uint8_t *graphic, size_t graphic_byte_count, uint16_t expected_width,
    uint16_t expected_height, uint8_t *indexed_pixels,
    size_t indexed_pixel_byte_count,
    CSB_V1_StartupGraphicDecodeReceipt_PC34 *out_receipt)
{
    csb_v1_img1_nibble_reader_pc34 reader;
    size_t pixel_count;
    size_t pixel_pos;
    size_t count;
    size_t i;
    uint8_t n1;
    uint8_t n2;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!graphic || !indexed_pixels || graphic_byte_count < 5U ||
        expected_width == 0U || expected_height == 0U ||
        csb_v1_startup_read_be16_pc34(graphic) != expected_width ||
        csb_v1_startup_read_be16_pc34(graphic + 2U) != expected_height ||
        expected_height > SIZE_MAX / expected_width) return 0;
    pixel_count = (size_t)expected_width * expected_height;
    if (pixel_count > indexed_pixel_byte_count) return 0;

    memset(indexed_pixels, 0, pixel_count);
    memset(&reader, 0, sizeof(reader));
    reader.data = graphic;
    reader.data_size = graphic_byte_count;
    reader.byte_pos = 4U;
    pixel_pos = 0U;

    while (pixel_pos < pixel_count && csb_v1_img1_has_nibble_pc34(&reader)) {
        n1 = csb_v1_img1_read_nibble_pc34(&reader);
        n2 = csb_v1_img1_read_nibble_pc34(&reader);

        if (n1 <= 7U) {
            count = (size_t)n1 + 1U;
            for (i = 0U; i < count && pixel_pos < pixel_count; ++i)
                indexed_pixels[pixel_pos++] = n2;
        } else if (n1 == 8U) {
            count = (size_t)csb_v1_img1_read_byte_pc34(&reader) + 1U;
            for (i = 0U; i < count && pixel_pos < pixel_count; ++i)
                indexed_pixels[pixel_pos++] = n2;
        } else if (n1 == 0x0CU) {
            count = (size_t)csb_v1_img1_read_word_pc34(&reader) + 1U;
            for (i = 0U; i < count && pixel_pos < pixel_count; ++i)
                indexed_pixels[pixel_pos++] = n2;
        } else if (n1 == 0x0BU) {
            count = (size_t)csb_v1_img1_read_byte_pc34(&reader) + 1U;
            for (i = 0U; i < count && pixel_pos < pixel_count; ++i) {
                if (pixel_pos >= (size_t)expected_width)
                    indexed_pixels[pixel_pos] =
                        indexed_pixels[pixel_pos - expected_width];
                pixel_pos++;
            }
            if (pixel_pos < pixel_count)
                indexed_pixels[pixel_pos++] = n2;
        } else if (n1 == 0x0FU) {
            count = (size_t)csb_v1_img1_read_word_pc34(&reader) + 1U;
            for (i = 0U; i < count && pixel_pos < pixel_count; ++i) {
                if (pixel_pos >= (size_t)expected_width)
                    indexed_pixels[pixel_pos] =
                        indexed_pixels[pixel_pos - expected_width];
                pixel_pos++;
            }
            if (pixel_pos < pixel_count)
                indexed_pixels[pixel_pos++] = n2;
        } else if (n1 == 9U) {
            uint8_t byte1 = csb_v1_img1_read_byte_pc34(&reader);
            if ((byte1 & 1U) == 0U) {
                if (pixel_pos < pixel_count)
                    indexed_pixels[pixel_pos++] = n2;
                for (i = 0U; i < (size_t)byte1 && pixel_pos < pixel_count; ++i)
                    indexed_pixels[pixel_pos++] =
                        csb_v1_img1_read_nibble_pc34(&reader);
            } else {
                count = (size_t)byte1 + 1U;
                for (i = 0U; i < count && pixel_pos < pixel_count; ++i)
                    indexed_pixels[pixel_pos++] =
                        csb_v1_img1_read_nibble_pc34(&reader);
            }
        } else if (n1 == 0x0DU) {
            uint16_t word1 = csb_v1_img1_read_word_pc34(&reader);
            if ((word1 & 1U) == 0U) {
                if (pixel_pos < pixel_count)
                    indexed_pixels[pixel_pos++] = n2;
                for (i = 0U; i < (size_t)word1 && pixel_pos < pixel_count; ++i)
                    indexed_pixels[pixel_pos++] =
                        csb_v1_img1_read_nibble_pc34(&reader);
            } else {
                count = (size_t)word1 + 1U;
                for (i = 0U; i < count && pixel_pos < pixel_count; ++i)
                    indexed_pixels[pixel_pos++] =
                        csb_v1_img1_read_nibble_pc34(&reader);
            }
        } else if (n1 == 0x0AU) {
            count = (size_t)n2 + 1U;
            for (i = 0U; i < count && pixel_pos < pixel_count; ++i)
                indexed_pixels[pixel_pos++] = 0U;
        } else if (n1 == 0x0EU) {
            if (n2 <= 0x0CU) {
                count = (size_t)n2 + 17U;
            } else if (n2 == 0x0DU) {
                count = (size_t)csb_v1_img1_read_byte_pc34(&reader) + 1U;
            } else if (n2 == 0x0EU) {
                count = (size_t)csb_v1_img1_read_byte_pc34(&reader) + 257U;
            } else {
                count = (size_t)csb_v1_img1_read_word_pc34(&reader) + 1U;
            }
            for (i = 0U; i < count && pixel_pos < pixel_count; ++i)
                indexed_pixels[pixel_pos++] = 0U;
        }
    }

    if (out_receipt) {
        size_t bytes_consumed = reader.byte_pos;
        if (reader.nibble_phase) bytes_consumed++;

        out_receipt->valid = 1;
        out_receipt->width = expected_width;
        out_receipt->height = expected_height;
        out_receipt->stream_byte_count = graphic_byte_count;
        out_receipt->stream_bytes_consumed = bytes_consumed;
        out_receipt->emitted_planar_pixels = pixel_pos;
        out_receipt->physical_planar_pixels = pixel_count;
        out_receipt->stream_fnv1a =
            csb_v1_startup_fnv1a_pc34(graphic, graphic_byte_count);
        out_receipt->indexed_pixel_fnv1a =
            csb_v1_startup_fnv1a_pc34(indexed_pixels, pixel_count);
        out_receipt->ended_at_record_boundary =
            (reader.byte_pos >= graphic_byte_count);
        out_receipt->implicit_blank_tail = (pixel_pos < pixel_count);
        out_receipt->indexed_colors_are_4bit = 1;
        if (out_receipt->stream_fnv1a == 0U ||
            out_receipt->indexed_pixel_fnv1a == 0U) {
            memset(out_receipt, 0, sizeof(*out_receipt));
            return 0;
        }
    }
    return 1;
}

int csb_v1_startup_img3_decode_to_indexed_pc34_compat(
    const uint8_t *graphic, size_t graphic_byte_count, uint16_t expected_width,
    uint16_t expected_height, uint8_t *indexed_pixels,
    size_t indexed_pixel_byte_count)
{
    return csb_v1_startup_img3_decode_to_indexed_with_receipt_pc34_compat(
        graphic, graphic_byte_count, expected_width, expected_height,
        indexed_pixels, indexed_pixel_byte_count, NULL);
}
