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
