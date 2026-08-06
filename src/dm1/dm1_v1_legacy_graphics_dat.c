#include "dm1_v1_legacy_graphics_dat.h"

#include <string.h>

#define DM1_LEGACY_GRAPHICS_COUNT 575u

static uint16_t rd16(const uint8_t *p, int be)
{
    return be ? (uint16_t)(((uint16_t)p[0] << 8) | p[1])
              : (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
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
        !record_bounds(data, size, be, index, &offset, &length)) return 0;
    width = rd16(data + offset, be);
    height = rd16(data + offset + 2u, be);
    if (width == 0u || height == 0u || width > 640u || height > 400u ||
        (size_t)width * height > 1024u * 1024u || length < 4u) return 0;
    if (out_width) *out_width = width;
    if (out_height) *out_height = height;
    return 1;
}

/* IMAGE2 command stream.  Counts are byte/BE16 encoded by the original
 * image format on both targets; only the embedded dimensions follow the
 * target byte order. */
int dm1_v1_legacy_graphics_decode(const uint8_t *data, size_t size, int be,
                                  uint16_t index, uint8_t *pixels,
                                  size_t capacity, uint16_t *out_width,
                                  uint16_t *out_height)
{
    size_t offset, length, src = 4u, pos = 0u, total, count, i;
    uint16_t width, height;
    if (!pixels || !dm1_v1_legacy_graphics_query(data, size, be, index,
                                                  &width, &height) ||
        !record_bounds(data, size, be, index, &offset, &length)) return 0;
    total = (size_t)width * height;
    if (capacity < total) return 0;
    memset(pixels, 0, total);
    while (pos < total && src < length) {
        uint8_t command = data[offset + src++];
        uint8_t color = (uint8_t)(command & 0x0fu);
        if ((command & 0x80u) == 0u) {
            count = (size_t)((command >> 4) & 7u) + 1u;
            if (count > total - pos) return 0;
            memset(pixels + pos, color, count);
            pos += count;
            continue;
        }
        if ((command & 0x40u) == 0u) {
            if (src >= length) return 0;
            count = (size_t)data[offset + src++] + 1u;
        } else {
            if (src + 1u >= length) return 0;
            count = (size_t)(((uint16_t)data[offset + src] << 8) |
                             data[offset + src + 1u]) + 1u;
            src += 2u;
        }
        switch ((command >> 4) & 3u) {
        case 0u:
            if (count > total - pos) return 0;
            memset(pixels + pos, color, count);
            pos += count;
            break;
        case 1u:
            if (count > total - pos || src + (count + 1u) / 2u > length)
                return 0;
            if (count & 1u) pixels[pos++] = color;
            for (i = 0u; i < count / 2u; ++i) {
                uint8_t packed = data[offset + src++];
                pixels[pos++] = (uint8_t)(packed >> 4);
                pixels[pos++] = (uint8_t)(packed & 0x0fu);
            }
            break;
        case 3u:
            if (count > total - pos) return 0;
            for (i = 0u; i < count; ++i) {
                if (pos < width) return 0;
                pixels[pos] = pixels[pos - width];
                ++pos;
            }
            if (pos >= total) return 0;
            pixels[pos++] = color;
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
