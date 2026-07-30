#include "csb_v1_startup_img3_decode_pc34_compat.h"

#include <limits.h>
#include <stdlib.h>
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

static int csb_v1_img2_store_pixel_pc34(uint8_t *pixels, size_t pixel_count,
                                         uint16_t width, uint16_t stride,
                                         size_t physical_pos, uint8_t color)
{
    size_t row;
    size_t column;

    if (!pixels || width == 0u || stride < width) return 0;
    row = physical_pos / stride;
    column = physical_pos % stride;
    if (column >= width || row > SIZE_MAX / width || row * width + column >= pixel_count)
        return column >= width;
    pixels[row * width + column] = color;
    return 1;
}

static int csb_v1_img2_get_pixel_pc34(const uint8_t *pixels, size_t pixel_count,
                                       uint16_t width, uint16_t stride,
                                       size_t physical_pos, uint8_t *out_color)
{
    size_t row;
    size_t column;

    if (!pixels || !out_color || width == 0u || stride < width) return 0;
    row = physical_pos / stride;
    column = physical_pos % stride;
    if (column >= width) {
        *out_color = 0u;
        return 1;
    }
    if (row > SIZE_MAX / width || row * width + column >= pixel_count) return 0;
    *out_color = pixels[row * width + column];
    return 1;
}

/* Canonical PC 3.4 CSB startup records use the big-endian IMG2 stream from
 * ReDMCSB IMAGE2.C F0689.  IMG2 commands are bytes, not IMG3 nibbles. */
static int csb_v1_img2_decode_to_indexed_pc34(
    const uint8_t *graphic, size_t graphic_byte_count, uint16_t expected_width,
    uint16_t expected_height, uint8_t *indexed_pixels,
    size_t indexed_pixel_byte_count,
    CSB_V1_StartupGraphicDecodeReceipt_PC34 *out_receipt)
{
    size_t pixel_count;
    uint16_t physical_width;
    size_t physical_count;
    size_t pixel_pos = 0u;
    size_t source_pos = 4u;

    if (!graphic || !indexed_pixels || graphic_byte_count < 5u ||
        csb_v1_img3_read_be16_pc34(graphic) != expected_width ||
        csb_v1_img3_read_be16_pc34(graphic + 2u) != expected_height ||
        expected_height > SIZE_MAX / expected_width) return 0;
    pixel_count = (size_t)expected_width * expected_height;
    if (pixel_count > indexed_pixel_byte_count) return 0;
    /* CSB's PC GRAPHICS.DAT stream is byte-addressed at its declared width.
     * ReDMCSB's Atari IMG2 path rounds the destination stride for planar
     * screens, but applying that pad corrupts PC C002's 105-pixel strip. */
    physical_width = expected_width;
    if (physical_width == 0u || expected_height > SIZE_MAX / physical_width) return 0;
    physical_count = (size_t)physical_width * expected_height;
    memset(indexed_pixels, 0, pixel_count);

    while (pixel_pos < physical_count) {
        uint8_t command;
        uint8_t color;
        size_t count;
        size_t i;

        if (source_pos >= graphic_byte_count) return 0;
        command = graphic[source_pos++];
        color = command & 0x0fu;
        if ((command & 0x80u) == 0u) {
            count = (size_t)(command >> 4u) + 1u;
            if (count > physical_count - pixel_pos) return 0;
            for (i = 0u; i < count; ++i)
                if (!csb_v1_img2_store_pixel_pc34(indexed_pixels, pixel_count,
                                                   expected_width, physical_width,
                                                   pixel_pos++, color)) return 0;
        } else {
            if ((command & 0x40u) == 0u) {
                if (source_pos >= graphic_byte_count) return 0;
                count = (size_t)graphic[source_pos++] + 1u;
            } else {
                if (source_pos + 1u >= graphic_byte_count) return 0;
                count = (size_t)(((uint16_t)graphic[source_pos] << 8u) |
                                 graphic[source_pos + 1u]) + 1u;
                source_pos += 2u;
            }
            if (count > physical_count - pixel_pos) return 0;
            switch (command & 0x30u) {
            case 0x00u:
                for (i = 0u; i < count; ++i)
                    if (!csb_v1_img2_store_pixel_pc34(indexed_pixels, pixel_count,
                                                       expected_width, physical_width,
                                                       pixel_pos++, color)) return 0;
                break;
            case 0x10u:
                if ((count & 1u) != 0u) {
                    if (!csb_v1_img2_store_pixel_pc34(indexed_pixels, pixel_count,
                                                       expected_width, physical_width,
                                                       pixel_pos++, color)) return 0;
                    --count;
                }
                if (source_pos + count / 2u > graphic_byte_count) return 0;
                for (i = 0u; i < count / 2u; ++i) {
                    uint8_t literal = graphic[source_pos++];
                    if (!csb_v1_img2_store_pixel_pc34(indexed_pixels, pixel_count,
                                                       expected_width, physical_width,
                                                       pixel_pos++, literal >> 4u) ||
                        !csb_v1_img2_store_pixel_pc34(indexed_pixels, pixel_count,
                                                       expected_width, physical_width,
                                                       pixel_pos++, literal & 0x0fu)) return 0;
                }
                break;
            case 0x30u:
                if (count + 1u > physical_count - pixel_pos || pixel_pos < physical_width)
                    return 0;
                for (i = 0u; i < count; ++i) {
                    uint8_t copied;
                    if (!csb_v1_img2_get_pixel_pc34(indexed_pixels, pixel_count,
                                                     expected_width, physical_width,
                                                     pixel_pos - physical_width, &copied) ||
                        !csb_v1_img2_store_pixel_pc34(indexed_pixels, pixel_count,
                                                       expected_width, physical_width,
                                                       pixel_pos++, copied)) return 0;
                }
                if (!csb_v1_img2_store_pixel_pc34(indexed_pixels, pixel_count,
                                                   expected_width, physical_width,
                                                   pixel_pos++, color)) return 0;
                break;
            default:
                return 0;
            }
        }
    }

    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->width = expected_width;
        out_receipt->height = expected_height;
        out_receipt->stream_byte_count = graphic_byte_count;
        out_receipt->stream_bytes_consumed = source_pos;
        out_receipt->emitted_planar_pixels = pixel_pos;
        out_receipt->physical_planar_pixels = pixel_count;
        out_receipt->stream_fnv1a = csb_v1_img3_fnv1a_pc34(graphic, graphic_byte_count);
        out_receipt->indexed_pixel_fnv1a = csb_v1_img3_fnv1a_pc34(indexed_pixels, pixel_count);
        out_receipt->ended_at_record_boundary = source_pos == graphic_byte_count;
        out_receipt->indexed_colors_are_4bit = 1;
    }
    return 1;
}

/* CSBWin's PC ExpandGraphic writes a four-plane Atari-compatible buffer,
 * then presents that buffer as indexed pixels.  The plane-group padding is
 * part of the source format; it matters for C002's 105-pixel door strip. */
static int csb_v1_img2_planar_decode_to_indexed_pc34(
    const uint8_t *graphic, size_t graphic_byte_count, uint16_t width,
    uint16_t height, uint8_t *pixels, size_t pixel_capacity,
    CSB_V1_StartupGraphicDecodeReceipt_PC34 *receipt)
{
    size_t pitch, physical_width, physical_pixels, plane_bytes, pixel_count;
    size_t source = 4u, emitted = 0u, i, pixel;
    uint8_t *planes;

    if (!graphic || !pixels || graphic_byte_count < 5u || width == 0u ||
        height == 0u || csb_v1_img3_read_be16_pc34(graphic) != width ||
        csb_v1_img3_read_be16_pc34(graphic + 2u) != height ||
        height > SIZE_MAX / width) return 0;
    pitch = (((size_t)width + 15u) >> 1u) & ~(size_t)7u;
    physical_width = (pitch / 8u) * 16u;
    pixel_count = (size_t)width * height;
    if (pitch == 0u || physical_width == 0u || height > SIZE_MAX / pitch ||
        height > SIZE_MAX / physical_width || pixel_count > pixel_capacity) return 0;
    physical_pixels = physical_width * height;
    plane_bytes = pitch * height;
    planes = (uint8_t *)calloc(plane_bytes, 1u);
    if (!planes) return 0;

    while (emitted < physical_pixels && source < graphic_byte_count) {
        uint8_t command = graphic[source++];
        uint8_t color = command & 0x0fu;
        size_t count;
        int literal = 0;
        int previous = 0;
        if ((command & 0x80u) == 0u) {
            count = (size_t)(command >> 4u) + 1u;
        } else {
            if (source >= graphic_byte_count) goto fail;
            count = (size_t)graphic[source++] + 1u;
            if ((command & 0x40u) != 0u) {
                if (source >= graphic_byte_count) goto fail;
                count = (count - 1u) * 256u + graphic[source++] + 1u;
            }
            literal = (command & 0x10u) != 0u;
            previous = literal && (command & 0x20u) != 0u;
        }
        if (previous && emitted < physical_width) goto fail;
        if (count > physical_pixels - emitted) {
            /* CSBWin ExpandGraphic is destination-bounded: its final packed
             * command can cross its target edge. Its TAG02170a exit drops
             * the remainder, so preserve only the source-owned destination
             * pixels rather than rejecting a valid C001--C005 record or
             * writing beyond the source rectangle. */
            count = physical_pixels - emitted;
        }
        for (i = 0u; i < count; ++i) {
            uint8_t value = color;
            size_t position = emitted;
            if (previous) {
                size_t source_pixel = emitted - physical_width;
                unsigned int plane;
                value = 0u;
                for (plane = 0u; plane < 4u; ++plane) {
                    size_t offset = (source_pixel / physical_width) * pitch +
                        ((source_pixel % physical_width) / 16u) * 8u + plane * 2u;
                    uint16_t word = csb_v1_img3_read_be16_pc34(planes + offset);
                    if ((word & (uint16_t)(1u << (15u - (source_pixel & 15u)))) != 0u)
                        value |= (uint8_t)(1u << plane);
                }
            } else if (literal) {
                if ((count & 1u) != 0u && i == 0u) {
                    value = color;
                } else {
                    size_t literal_index = ((count & 1u) != 0u) ? i - 1u : i;
                    uint8_t packed;
                    if (source + literal_index / 2u >= graphic_byte_count) goto fail;
                    packed = graphic[source + literal_index / 2u];
                    value = (literal_index & 1u) == 0u ? packed >> 4u : packed & 0x0fu;
                    if (i + 1u == count) source += (literal_index / 2u) + 1u;
                }
            }
            for (unsigned int plane = 0u; plane < 4u; ++plane) {
                if ((value & (uint8_t)(1u << plane)) != 0u) {
                    size_t offset = (position / physical_width) * pitch +
                        ((position % physical_width) / 16u) * 8u + plane * 2u;
                    uint16_t word = csb_v1_img3_read_be16_pc34(planes + offset);
                    word |= (uint16_t)(1u << (15u - (position & 15u)));
                    planes[offset] = (uint8_t)(word >> 8u);
                    planes[offset + 1u] = (uint8_t)word;
                }
            }
            ++emitted;
        }
    }
    for (pixel = 0u; pixel < pixel_count; ++pixel) {
        size_t position = (pixel / width) * physical_width + (pixel % width);
        uint8_t value = 0u;
        for (unsigned int plane = 0u; plane < 4u; ++plane) {
            size_t offset = (position / physical_width) * pitch +
                ((position % physical_width) / 16u) * 8u + plane * 2u;
            if ((csb_v1_img3_read_be16_pc34(planes + offset) &
                 (uint16_t)(1u << (15u - (position & 15u)))) != 0u)
                value |= (uint8_t)(1u << plane);
        }
        pixels[pixel] = value;
    }
    if (receipt) {
        receipt->valid = 1;
        receipt->width = width;
        receipt->height = height;
        receipt->stream_byte_count = graphic_byte_count;
        receipt->stream_bytes_consumed = source;
        receipt->emitted_planar_pixels = emitted;
        receipt->physical_planar_pixels = physical_pixels;
        receipt->stream_fnv1a = csb_v1_img3_fnv1a_pc34(graphic, graphic_byte_count);
        receipt->indexed_pixel_fnv1a = csb_v1_img3_fnv1a_pc34(pixels, pixel_count);
        receipt->ended_at_record_boundary = emitted == physical_pixels;
        receipt->implicit_blank_tail = emitted < physical_pixels;
        receipt->indexed_colors_are_4bit = 1;
    }
    free(planes);
    return 1;
fail:
    free(planes);
    return 0;
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
    return indexed_pixels && width == 320u && height >= 153u &&
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
        /* The PC3.4 C001--C005 stream is an IMG2 byte stream whose declared
         * width is its raster stride.  The planar expansion remains a narrow
         * fallback for records that reject that native PC contract; accepting
         * it first can consume a valid PC record with the wrong layout and
         * produce a geometrically valid but visibly scrambled title/door. */
        if (csb_v1_img2_decode_to_indexed_pc34(
                graphic, graphic_byte_count, expected_width, expected_height,
                indexed_pixels, indexed_pixel_byte_count, out_receipt)) return 1;
        return csb_v1_img2_planar_decode_to_indexed_pc34(
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
