#include "nexus_v1_font012.h"

#include <string.h>

static uint16_t be16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

static uint32_t be32(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | p[3];
}

int nexus_v1_font012_parse(const uint8_t *data, size_t size,
                           uint32_t resource_index,
                           Nexus_V1_Font012Receipt *out)
{
    uint32_t header_word12;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!data || size < 32U || memcmp(data, "FONT", 4) != 0 ||
        memcmp(data + 8U, "FONT", 4) != 0 ||
        be32(data + 4U) != resource_index)
        return 0;

    header_word12 = be32(data + 12U);

    out->resource_index = resource_index;
    out->header_word12 = header_word12;
    out->format_word = be32(data + 16U);
    out->character_count = be16(data + 20U);
    out->character_width = be16(data + 22U);
    out->character_height = be16(data + 24U);
    out->resource_size = (uint32_t)size;

    /* The Translation Kit documents FONT012 as 6-pixel half-width or
     * 12-pixel full-width glyphs, 12 pixels high.  Do not infer any bitmap
     * packing from these fields. */
    if (out->character_count == 0U ||
        (out->character_width != 6U && out->character_width != 12U) ||
        out->character_height != 12U)
        return 0;

    out->valid = 1;
    return 1;
}

int nexus_v1_font012_decode_glyph(const uint8_t *data, size_t size,
                                  uint32_t resource_index,
                                  uint16_t glyph_index,
                                  uint8_t *pixels, size_t pixel_capacity)
{
    Nexus_V1_Font012Receipt receipt;
    size_t bytes_per_line;
    size_t glyph_bytes;
    size_t glyph_offset;
    size_t y;
    if (!pixels || !nexus_v1_font012_parse(data, size, resource_index,
                                            &receipt)) return 0;
    if (glyph_index >= receipt.character_count) return 0;
    bytes_per_line = ((size_t)receipt.character_width + 3U) / 4U;
    glyph_bytes = bytes_per_line * (size_t)receipt.character_height;
    glyph_offset = 36U + (size_t)glyph_index * glyph_bytes;
    if ((size_t)receipt.character_width * receipt.character_height >
            pixel_capacity || glyph_offset > size ||
        glyph_bytes > size - glyph_offset) return 0;

    for (y = 0; y < receipt.character_height; ++y) {
        uint32_t packed_row = 0U;
        size_t x;
        for (x = 0; x < bytes_per_line; ++x)
            packed_row = (packed_row << 8) |
                         data[glyph_offset + y * bytes_per_line + x];
        for (x = 0; x < receipt.character_width; ++x) {
            size_t bit_shift = (bytes_per_line * 8U) - 2U - 2U * x;
            pixels[y * receipt.character_width + x] =
                (uint8_t)(3U - ((packed_row >> bit_shift) & 3U));
        }
    }
    return 1;
}
