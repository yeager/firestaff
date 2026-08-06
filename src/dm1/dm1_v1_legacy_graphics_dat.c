#include "dm1_v1_legacy_graphics_dat.h"

#include <string.h>

#define DM1_LEGACY_GRAPHICS_COUNT 575u

static uint16_t rd16(const uint8_t *p, int be)
{
    return be ? (uint16_t)(((uint16_t)p[0] << 8) | p[1])
              : (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}

int dm1_v1_legacy_graphics_is_bitmap_index(uint16_t graphic_index)
{
    return graphic_index <= 20u ||
           (graphic_index >= 22u && graphic_index <= 532u);
}

static int record_bounds(const uint8_t *data, size_t size, int be,
                         uint16_t index, size_t *out_offset,
                         size_t *out_length)
{
    const size_t table = 2u;
    const size_t payload = table + DM1_LEGACY_GRAPHICS_COUNT * 4u;
    size_t offset;
    uint16_t i;

    if (!data || !out_offset || !out_length ||
        index >= DM1_LEGACY_GRAPHICS_COUNT || size < payload) return 0;
    offset = payload;
    for (i = 0u; i < index; ++i) {
        size_t pos = table + (size_t)i * 2u;
        uint16_t length = rd16(data + pos, be);
        if ((size_t)length > size - offset) return 0;
        offset += length;
    }
    {
        size_t pos = table + (size_t)index * 2u;
        uint16_t length = rd16(data + pos, be);
        if ((size_t)length > size - offset || length < 4u) return 0;
        *out_offset = offset;
        *out_length = length;
    }
    return 1;
}

int dm1_v1_legacy_graphics_probe(const uint8_t *data, size_t size, int be)
{
    size_t header = 2u + DM1_LEGACY_GRAPHICS_COUNT * 4u;
    size_t offset;
    uint32_t total = 0u;
    uint16_t i;

    if (!data || size < header || rd16(data, be) != DM1_LEGACY_GRAPHICS_COUNT)
        return 0;
    offset = header;
    for (i = 0u; i < DM1_LEGACY_GRAPHICS_COUNT; ++i) {
        size_t pos = 2u + (size_t)i * 2u;
        uint16_t compressed = rd16(data + pos, be);
        uint16_t expanded = rd16(data + 2u + DM1_LEGACY_GRAPHICS_COUNT * 2u +
                                 (size_t)i * 2u, be);
        if (compressed != expanded || (size_t)compressed > size - offset)
            return 0;
        total += compressed;
        offset += compressed;
    }
    return offset == size && total == size - header;
}

int dm1_v1_legacy_graphics_query(const uint8_t *data, size_t size, int be,
                                 uint16_t index, uint16_t *out_width,
                                 uint16_t *out_height)
{
    size_t offset, length;
    uint16_t width, height;
    if (!dm1_v1_legacy_graphics_probe(data, size, be) ||
        !dm1_v1_legacy_graphics_is_bitmap_index(index) ||
        !record_bounds(data, size, be, index, &offset, &length)) return 0;
    width = rd16(data + offset, be);
    height = rd16(data + offset + 2u, be);
    if (width == 0u || height == 0u || width > 640u || height > 400u ||
        (size_t)width * height > 1024u * 1024u || length < 4u) return 0;
    if (out_width) *out_width = width;
    if (out_height) *out_height = height;
    return 1;
}

static int read_nibble(const uint8_t *data, size_t offset, size_t length,
                       size_t *nibble_pos, uint8_t *out)
{
    size_t byte_pos;
    if (!data || !nibble_pos || !out || *nibble_pos >= (length - 4u) * 2u)
        return 0;
    byte_pos = offset + 4u + (*nibble_pos / 2u);
    *out = ((*nibble_pos & 1u) == 0u)
        ? (uint8_t)(data[byte_pos] >> 4)
        : (uint8_t)(data[byte_pos] & 0x0fu);
    ++*nibble_pos;
    return 1;
}

static int read_nibble_value(const uint8_t *data, size_t offset, size_t length,
                             size_t *nibble_pos, unsigned int nibble_count,
                             size_t *out)
{
    unsigned int i;
    size_t value = 0u;
    uint8_t nibble;
    for (i = 0u; i < nibble_count; ++i) {
        if (!read_nibble(data, offset, length, nibble_pos, &nibble)) return 0;
        value = (value << 4) | (size_t)nibble;
    }
    *out = value;
    return 1;
}

/* IMAGE1/IMAGE2 command stream.  The format is nibble-based RLE as
 * documented by DMWeb's data-files page and ReDMCSB IMAGE2.C.  IMG1 uses a
 * big-endian width/height header; IMG2 uses little endian.  The pixel stream
 * itself is identical on both targets. */
int dm1_v1_legacy_graphics_decode_item(const uint8_t *data, size_t size,
                                       int be, uint8_t *pixels,
                                       size_t capacity, uint16_t *out_width,
                                       uint16_t *out_height)
{
    size_t offset = 0u, length = size, nibble_pos = 0u, pos = 0u;
    size_t total, count, i;
    uint16_t width, height;
    if (!data || !pixels || size < 4u) return 0;
    width = rd16(data, be);
    height = rd16(data + 2u, be);
    if (width == 0u || height == 0u || width > 640u || height > 400u ||
        (size_t)width * height > 1024u * 1024u) return 0;
    total = (size_t)width * height;
    if (capacity < total) return 0;
    memset(pixels, 0, total);
    while (pos < total) {
        uint8_t control;
        uint8_t color;
        if (!read_nibble(data, offset, length, &nibble_pos, &control) ||
            !read_nibble(data, offset, length, &nibble_pos, &color)) return 0;

        switch (control) {
        case 0u: case 1u: case 2u: case 3u:
        case 4u: case 5u: case 6u: case 7u:
            count = (size_t)control + 1u;
            if (count > total - pos) return 0;
            memset(pixels + pos, color, count);
            pos += count;
            break;
        case 8u:
            if (!read_nibble_value(data, offset, length, &nibble_pos, 2u,
                                   &count)) return 0;
            ++count;
            if (count > total - pos) return 0;
            memset(pixels + pos, color, count);
            pos += count;
            break;
        case 9u:
        case 0x0du:
            if (!read_nibble_value(data, offset, length, &nibble_pos,
                                   control == 9u ? 2u : 4u, &count)) return 0;
            if ((count & 1u) == 0u) {
                if (pos >= total) return 0;
                pixels[pos++] = color;
            } else {
                count += 1u;
            }
            /* For an even source count the leading Nibble2 is one pixel and
             * the source count remains the number of literal color nibbles.
             * For an odd source count Nibble2 is ignored and the literal run
             * is source_count + 1 pixels. */
            if (count > total - pos) return 0;
            for (i = 0u; i < count; ++i) {
                if (!read_nibble(data, offset, length, &nibble_pos, &color))
                    return 0;
                pixels[pos++] = color;
            }
            break;
        case 0x0bu:
        case 0x0fu:
            if (!read_nibble_value(data, offset, length, &nibble_pos,
                                   control == 0x0bu ? 2u : 4u, &count)) return 0;
            ++count;
            if (count + 1u > total - pos) return 0;
            for (i = 0u; i < count; ++i) {
                if (pos < (size_t)width) return 0;
                pixels[pos] = pixels[pos - width];
                ++pos;
            }
            pixels[pos++] = color;
            break;
        case 0x0cu:
            if (!read_nibble_value(data, offset, length, &nibble_pos, 4u,
                                   &count)) return 0;
            ++count;
            if (count > total - pos) return 0;
            memset(pixels + pos, color, count);
            pos += count;
            break;
        case 0x0au:
            count = (size_t)color + 1u;
            if (count > total - pos) return 0;
            memset(pixels + pos, 0, count);
            pos += count;
            break;
        case 0x0eu:
            if (color <= 0x0cu) {
                count = (size_t)color + 17u;
            } else if (color == 0x0du) {
                if (!read_nibble_value(data, offset, length, &nibble_pos, 2u,
                                       &count)) return 0;
                ++count;
            } else if (color == 0x0eu) {
                if (!read_nibble_value(data, offset, length, &nibble_pos, 2u,
                                       &count)) return 0;
                count += 257u;
            } else {
                if (!read_nibble_value(data, offset, length, &nibble_pos, 4u,
                                       &count)) return 0;
                ++count;
            }
            if (count > total - pos) return 0;
            memset(pixels + pos, 0, count);
            pos += count;
            break;
        default:
            return 0;
        }
    }
    if (pos != total) return 0;
    if (out_width) *out_width = width;
    if (out_height) *out_height = height;
    return 1;
}

int dm1_v1_legacy_graphics_decode(const uint8_t *data, size_t size, int be,
                                  uint16_t index, uint8_t *pixels,
                                  size_t capacity, uint16_t *out_width,
                                  uint16_t *out_height)
{
    size_t offset;
    size_t length;
    if (!pixels || !dm1_v1_legacy_graphics_query(data, size, be, index,
                                                  out_width, out_height) ||
        !record_bounds(data, size, be, index, &offset, &length)) return 0;
    return dm1_v1_legacy_graphics_decode_item(
        data + offset, length, be, pixels, capacity, out_width, out_height);
}
