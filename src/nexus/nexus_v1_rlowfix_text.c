#include "nexus_v1_rlowfix_text.h"

#include <string.h>

static uint16_t be16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

int nexus_v1_rlowfix_text_parse(const uint8_t *data, size_t size,
                                uint32_t resource_offset,
                                Nexus_V1_RlowfixText *out)
{
    uint32_t table_end;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!data || resource_offset > size || size - resource_offset < 8 ||
        memcmp(data + resource_offset, "TEXT", 4) != 0) return 0;
    out->resource_index = be16(data + resource_offset + 4);
    out->string_count = be16(data + resource_offset + 6);
    table_end = resource_offset + 8U + (uint32_t)out->string_count * 2U;
    if (table_end > size) return 0;
    out->resource_offset = resource_offset;
    out->table_end = table_end;
    out->valid = 1;
    return 1;
}

int nexus_v1_rlowfix_text_span(const uint8_t *data, size_t size,
                               const Nexus_V1_RlowfixText *text,
                               uint16_t string_index,
                               const uint8_t **bytes, size_t *byte_count)
{
    uint32_t start, end;
    if (bytes) *bytes = NULL;
    if (byte_count) *byte_count = 0;
    if (!data || !text || !text->valid || !bytes || !byte_count ||
        string_index >= text->string_count) return 0;
    /* DMWeb DecodeTEXT offsets are relative to the six-byte TEXT prefix
     * (tag + resource index), not to the tag itself. */
    start = be16(data + text->resource_offset + 8U +
                 (uint32_t)string_index * 2U) + text->resource_offset + 6U;
    end = (string_index + 1U < text->string_count) ?
        be16(data + text->resource_offset + 8U +
             (uint32_t)(string_index + 1U) * 2U) + text->resource_offset + 6U :
        (uint32_t)size;
    if (start < text->table_end || end < start || end > size) return 0;
    *bytes = data + start;
    *byte_count = (size_t)(end - start);
    return 1;
}
