#include "csb_v1_startup_img3_decode_pc34_compat.h"

#include <limits.h>
#include <string.h>

/*
 * PC34 GRAPHICS.DAT uses ReDMCSB IMAGE3.C's packed-nibble IMG3 stream.
 * The first four bytes are a little-endian width/height header.  Six local
 * palette entries follow at nibble offset 8; commands then select a palette
 * color, a literal color, or a copy from the preceding scanline.
 */
typedef struct {
    const uint8_t *data;
    size_t data_size;
    size_t nibble_pos;
} CSB_V1_IMG3Reader_PC34;

static uint16_t csb_v1_img3_read_le16_pc34(const uint8_t *bytes)
{
    return (uint16_t)((uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8));
}

static uint16_t csb_v1_img3_read_be16_pc34(const uint8_t *bytes)
{
    return (uint16_t)(((uint16_t)bytes[0] << 8) | bytes[1]);
}

static uint32_t csb_v1_img3_fnv1a_pc34(const uint8_t *bytes, size_t count)
{
    uint32_t hash = 2166136261u;
    size_t index;

    if (!bytes || count == 0u) return 0u;
    for (index = 0u; index < count; ++index) {
        hash ^= bytes[index];
        hash *= 16777619u;
    }
    return hash ? hash : 1u;
}

static int csb_v1_img3_read_nibble_pc34(CSB_V1_IMG3Reader_PC34 *reader,
                                         uint8_t *out)
{
    size_t byte_pos;

    if (!reader || !out) return 0;
    byte_pos = reader->nibble_pos >> 1u;
    if (byte_pos >= reader->data_size) return 0;
    *out = (uint8_t)(((reader->nibble_pos & 1u) != 0u)
                         ? (reader->data[byte_pos] & 0x0fu)
                         : (reader->data[byte_pos] >> 4u));
    reader->nibble_pos++;
    return 1;
}

static int csb_v1_img3_read_count_pc34(CSB_V1_IMG3Reader_PC34 *reader,
                                        size_t *out_count)
{
    uint8_t nibble;
    uint8_t hi;
    uint8_t lo;
    uint8_t word_nibble;

    if (!reader || !out_count || !csb_v1_img3_read_nibble_pc34(reader, &nibble))
        return 0;
    if (nibble != 0x0fu) {
        *out_count = (size_t)nibble + 2u;
        return 1;
    }
    if (!csb_v1_img3_read_nibble_pc34(reader, &hi) ||
        !csb_v1_img3_read_nibble_pc34(reader, &lo)) return 0;
    if ((uint8_t)((hi << 4u) | lo) != 0xffu) {
        *out_count = (size_t)((hi << 4u) | lo) + 17u;
        return 1;
    }
    *out_count = 0u;
    for (word_nibble = 0u; word_nibble < 4u; ++word_nibble) {
        if (!csb_v1_img3_read_nibble_pc34(reader, &nibble)) return 0;
        *out_count = (*out_count << 4u) | nibble;
    }
    return *out_count != 0u;
}

/* The canonical PC 3.4 CSB archive selected by the boot profile stores its
 * startup records as IMG1 after LZW.  Keep this decoder distinct from the
 * IMAGE3 path above: the two formats have incompatible headers and commands. */
static int csb_v1_img1_decode_to_indexed_pc34(
    const uint8_t *graphic, size_t graphic_byte_count, uint16_t expected_width,
    uint16_t expected_height, uint8_t *indexed_pixels,
    size_t indexed_pixel_byte_count,
    CSB_V1_StartupGraphicDecodeReceipt_PC34 *out_receipt)
{
    CSB_V1_IMG3Reader_PC34 reader;
    size_t pixel_count;
    size_t pixel_pos = 0u;

    if (!graphic || !indexed_pixels || graphic_byte_count < 5u ||
        csb_v1_img3_read_be16_pc34(graphic) != expected_width ||
        csb_v1_img3_read_be16_pc34(graphic + 2u) != expected_height ||
        expected_height > SIZE_MAX / expected_width) return 0;
    pixel_count = (size_t)expected_width * expected_height;
    if (pixel_count > indexed_pixel_byte_count) return 0;
    memset(indexed_pixels, 0, pixel_count);
    reader.data = graphic;
    reader.data_size = graphic_byte_count;
    reader.nibble_pos = 8u;

    while (pixel_pos < pixel_count) {
        uint8_t command;
        uint8_t color;
        size_t count;
        size_t i;

        if (!csb_v1_img3_read_nibble_pc34(&reader, &command) ||
            !csb_v1_img3_read_nibble_pc34(&reader, &color)) return 0;
        if (command <= 7u) {
            count = (size_t)command + 1u;
            if (count > pixel_count - pixel_pos) return 0;
            memset(indexed_pixels + pixel_pos, color, count);
            pixel_pos += count;
        } else if (command == 8u || command == 0x0cu) {
            uint8_t hi;
            uint8_t lo;
            if (command == 8u) {
                if (!csb_v1_img3_read_nibble_pc34(&reader, &hi) ||
                    !csb_v1_img3_read_nibble_pc34(&reader, &lo)) return 0;
                count = (size_t)((hi << 4u) | lo) + 1u;
            } else {
                uint8_t n0;
                uint8_t n1;
                uint8_t n2;
                uint8_t n3;
                if (!csb_v1_img3_read_nibble_pc34(&reader, &n0) ||
                    !csb_v1_img3_read_nibble_pc34(&reader, &n1) ||
                    !csb_v1_img3_read_nibble_pc34(&reader, &n2) ||
                    !csb_v1_img3_read_nibble_pc34(&reader, &n3)) return 0;
                count = ((size_t)n0 << 12u) | ((size_t)n1 << 8u) |
                    ((size_t)n2 << 4u) | n3;
                count++;
            }
            if (count > pixel_count - pixel_pos) return 0;
            memset(indexed_pixels + pixel_pos, color, count);
            pixel_pos += count;
        } else if (command == 0x0bu || command == 0x0fu) {
            uint8_t hi;
            uint8_t lo;
            if (command == 0x0bu) {
                if (!csb_v1_img3_read_nibble_pc34(&reader, &hi) ||
                    !csb_v1_img3_read_nibble_pc34(&reader, &lo)) return 0;
                count = (size_t)((hi << 4u) | lo) + 1u;
            } else {
                uint8_t n0;
                uint8_t n1;
                uint8_t n2;
                uint8_t n3;
                if (!csb_v1_img3_read_nibble_pc34(&reader, &n0) ||
                    !csb_v1_img3_read_nibble_pc34(&reader, &n1) ||
                    !csb_v1_img3_read_nibble_pc34(&reader, &n2) ||
                    !csb_v1_img3_read_nibble_pc34(&reader, &n3)) return 0;
                count = (((size_t)n0 << 12u) | ((size_t)n1 << 8u) |
                    ((size_t)n2 << 4u) | n3) + 1u;
            }
            if (count > pixel_count - pixel_pos) return 0;
            for (i = 0u; i < count; ++i) {
                if (pixel_pos >= expected_width)
                    indexed_pixels[pixel_pos] =
                        indexed_pixels[pixel_pos - expected_width];
                pixel_pos++;
            }
            if (pixel_pos < pixel_count) indexed_pixels[pixel_pos++] = color;
        } else if (command == 9u || command == 0x0du) {
            uint8_t hi;
            uint8_t lo;
            if (command == 9u) {
                if (!csb_v1_img3_read_nibble_pc34(&reader, &hi) ||
                    !csb_v1_img3_read_nibble_pc34(&reader, &lo)) return 0;
                count = (size_t)((hi << 4u) | lo);
            } else {
                uint8_t n0;
                uint8_t n1;
                uint8_t n2;
                uint8_t n3;
                if (!csb_v1_img3_read_nibble_pc34(&reader, &n0) ||
                    !csb_v1_img3_read_nibble_pc34(&reader, &n1) ||
                    !csb_v1_img3_read_nibble_pc34(&reader, &n2) ||
                    !csb_v1_img3_read_nibble_pc34(&reader, &n3)) return 0;
                count = ((size_t)n0 << 12u) | ((size_t)n1 << 8u) |
                    ((size_t)n2 << 4u) | n3;
            }
            if ((count & 1u) == 0u) {
                if (pixel_pos >= pixel_count) return 0;
                indexed_pixels[pixel_pos++] = color;
            } else {
                count++;
            }
            if (count > pixel_count - pixel_pos) return 0;
            for (i = 0u; i < count; ++i) {
                if (!csb_v1_img3_read_nibble_pc34(&reader,
                                                   &indexed_pixels[pixel_pos++]))
                    return 0;
            }
        } else if (command == 0x0au || command == 0x0eu) {
            if (command == 0x0au) {
                count = (size_t)color + 1u;
            } else if (color <= 0x0cu) {
                count = (size_t)color + 17u;
            } else if (color == 0x0du || color == 0x0eu) {
                uint8_t hi;
                uint8_t lo;
                if (!csb_v1_img3_read_nibble_pc34(&reader, &hi) ||
                    !csb_v1_img3_read_nibble_pc34(&reader, &lo)) return 0;
                count = (size_t)((hi << 4u) | lo) +
                    (color == 0x0du ? 1u : 257u);
            } else {
                uint8_t n0;
                uint8_t n1;
                uint8_t n2;
                uint8_t n3;
                if (!csb_v1_img3_read_nibble_pc34(&reader, &n0) ||
                    !csb_v1_img3_read_nibble_pc34(&reader, &n1) ||
                    !csb_v1_img3_read_nibble_pc34(&reader, &n2) ||
                    !csb_v1_img3_read_nibble_pc34(&reader, &n3)) return 0;
                count = (((size_t)n0 << 12u) | ((size_t)n1 << 8u) |
                    ((size_t)n2 << 4u) | n3) + 1u;
            }
            if (count > pixel_count - pixel_pos) return 0;
            pixel_pos += count;
        } else {
            return 0;
        }
    }

    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->width = expected_width;
        out_receipt->height = expected_height;
        out_receipt->stream_byte_count = graphic_byte_count;
        out_receipt->stream_bytes_consumed = (reader.nibble_pos + 1u) >> 1u;
        out_receipt->emitted_planar_pixels = pixel_pos;
        out_receipt->physical_planar_pixels = pixel_count;
        out_receipt->stream_fnv1a = csb_v1_img3_fnv1a_pc34(graphic, graphic_byte_count);
        out_receipt->indexed_pixel_fnv1a = csb_v1_img3_fnv1a_pc34(indexed_pixels, pixel_count);
        out_receipt->ended_at_record_boundary =
            out_receipt->stream_bytes_consumed == graphic_byte_count;
        out_receipt->indexed_colors_are_4bit = 1;
    }
    return 1;
}

static int csb_v1_startup_indexed_region_has_visible_pixel_pc34(
    const uint8_t *indexed_pixels, uint16_t width, uint16_t top,
    uint16_t region_height)
{
    size_t offset;
    size_t count;
    size_t index;

    if (!indexed_pixels || width == 0u || region_height == 0u) return 0;
    offset = (size_t)top * width;
    count = (size_t)region_height * width;
    for (index = 0u; index < count; ++index) {
        if (indexed_pixels[offset + index] != 0u) return 1;
    }
    return 0;
}

int csb_v1_startup_title_c001_regions_admit_pc34_compat(
    const uint8_t *indexed_pixels, uint16_t width, uint16_t height)
{
    return indexed_pixels && width == 320u && height == 153u &&
        csb_v1_startup_indexed_region_has_visible_pixel_pc34(
            indexed_pixels, width, 0u, 80u) &&
        csb_v1_startup_indexed_region_has_visible_pixel_pc34(
            indexed_pixels, width, 80u, 57u) &&
        csb_v1_startup_indexed_region_has_visible_pixel_pc34(
            indexed_pixels, width, 137u, 16u);
}

int csb_v1_startup_img3_decode_to_indexed_with_receipt_pc34_compat(
    const uint8_t *graphic, size_t graphic_byte_count, uint16_t expected_width,
    uint16_t expected_height, uint8_t *indexed_pixels,
    size_t indexed_pixel_byte_count,
    CSB_V1_StartupGraphicDecodeReceipt_PC34 *out_receipt)
{
    CSB_V1_IMG3Reader_PC34 reader;
    uint8_t local_palette[6];
    size_t pixel_count;
    size_t pixel_pos = 0u;
    unsigned int palette_index;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!graphic || !indexed_pixels || graphic_byte_count < 5u ||
        expected_width == 0u || expected_height == 0u ||
        expected_height > SIZE_MAX / expected_width) return 0;
    if (csb_v1_img3_read_be16_pc34(graphic) == expected_width &&
        csb_v1_img3_read_be16_pc34(graphic + 2u) == expected_height) {
        return csb_v1_img1_decode_to_indexed_pc34(
            graphic, graphic_byte_count, expected_width, expected_height,
            indexed_pixels, indexed_pixel_byte_count, out_receipt);
    }
    if (csb_v1_img3_read_le16_pc34(graphic) != expected_width ||
        csb_v1_img3_read_le16_pc34(graphic + 2u) != expected_height) return 0;
    pixel_count = (size_t)expected_width * expected_height;
    if (pixel_count > indexed_pixel_byte_count) return 0;

    memset(indexed_pixels, 0, pixel_count);
    reader.data = graphic;
    reader.data_size = graphic_byte_count;
    reader.nibble_pos = 8u;
    for (palette_index = 0u; palette_index < 6u; ++palette_index) {
        if (!csb_v1_img3_read_nibble_pc34(&reader, &local_palette[palette_index]))
            return 0;
    }

    while (pixel_pos < pixel_count) {
        uint8_t command;
        uint8_t kind;
        uint8_t color = 0u;
        size_t count;
        size_t copied;

        if (!csb_v1_img3_read_nibble_pc34(&reader, &command)) return 0;
        kind = (uint8_t)(command & 0x07u);
        if (kind < 6u) {
            color = local_palette[kind];
        } else if (kind == 7u &&
                   !csb_v1_img3_read_nibble_pc34(&reader, &color)) {
            return 0;
        }
        if ((command & 0x08u) != 0u) {
            if (!csb_v1_img3_read_count_pc34(&reader, &count)) return 0;
        } else {
            count = 1u;
        }
        if (count == 0u || count > pixel_count - pixel_pos) return 0;

        if (kind == 6u) {
            for (copied = 0u; copied < count; ++copied) {
                size_t source_pos = pixel_pos + copied;
                if (source_pos < expected_width) return 0;
                indexed_pixels[source_pos] =
                    indexed_pixels[source_pos - expected_width];
            }
        } else {
            memset(indexed_pixels + pixel_pos, color, count);
        }
        pixel_pos += count;
    }

    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->width = expected_width;
        out_receipt->height = expected_height;
        out_receipt->stream_byte_count = graphic_byte_count;
        out_receipt->stream_bytes_consumed = (reader.nibble_pos + 1u) >> 1u;
        out_receipt->emitted_planar_pixels = pixel_pos;
        out_receipt->physical_planar_pixels = pixel_count;
        out_receipt->stream_fnv1a = csb_v1_img3_fnv1a_pc34(
            graphic, graphic_byte_count);
        out_receipt->indexed_pixel_fnv1a = csb_v1_img3_fnv1a_pc34(
            indexed_pixels, pixel_count);
        out_receipt->ended_at_record_boundary =
            out_receipt->stream_bytes_consumed == graphic_byte_count;
        out_receipt->implicit_blank_tail = 0;
        out_receipt->indexed_colors_are_4bit = 1;
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
