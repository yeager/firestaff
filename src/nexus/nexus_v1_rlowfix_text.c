#include "nexus_v1_rlowfix_text.h"

#include <string.h>

static uint16_t be16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

static int rlowfix_resource_tag(const uint8_t *p)
{
    return memcmp(p, "TEXT", 4) == 0 || memcmp(p, "PALT", 4) == 0 ||
           memcmp(p, "PLRD", 4) == 0 || memcmp(p, "TABL", 4) == 0 ||
           memcmp(p, "CRET", 4) == 0;
}

int nexus_v1_rlowfix_text_parse(const uint8_t *data, size_t size,
                                uint32_t resource_offset,
                                Nexus_V1_RlowfixText *out)
{
    uint32_t table_end;
    uint32_t cursor;
    uint32_t previous_start = 0U;
    uint16_t string_index;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!data || resource_offset > size || size - resource_offset < 8 ||
        memcmp(data + resource_offset, "TEXT", 4) != 0) return 0;
    out->resource_index = be16(data + resource_offset + 6);
    out->string_count = be16(data + resource_offset + 8);
    table_end = resource_offset + 10U + (uint32_t)out->string_count * 2U;
    if (table_end > size) return 0;
    out->resource_offset = resource_offset;
    out->table_end = table_end;
    out->resource_end = (uint32_t)size;
    for (cursor = table_end; cursor + 4U <= size; ++cursor) {
        if (rlowfix_resource_tag(data + cursor)) {
            out->resource_end = cursor;
            break;
        }
    }
    if (out->resource_end < table_end) return 0;
    /* DMWeb TEXT offsets are relative to the eight-byte resource header.
     * Validate the complete ordered span table before exposing the resource;
     * accepting only the requested string could hide a malformed neighbour. */
    for (string_index = 0; string_index < out->string_count;
         ++string_index) {
        uint32_t start = (uint32_t)be16(
            data + resource_offset + 10U + (uint32_t)string_index * 2U) +
            resource_offset + 8U;
        if (start < table_end || start > out->resource_end ||
            (string_index > 0U && start < previous_start)) {
            memset(out, 0, sizeof(*out));
            return 0;
        }
        previous_start = start;
    }
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
    if (!data || !text || !text->valid || text->resource_end > size ||
        !bytes || !byte_count ||
        string_index >= text->string_count) return 0;
    /* DMWeb DecodeTEXT offsets are relative to the eight-byte TEXT header. */
    start = be16(data + text->resource_offset + 10U +
                 (uint32_t)string_index * 2U) + text->resource_offset + 8U;
    end = (string_index + 1U < text->string_count) ?
        be16(data + text->resource_offset + 10U +
             (uint32_t)(string_index + 1U) * 2U) + text->resource_offset + 8U :
        text->resource_end;
    if (start < text->table_end || end < start || end > text->resource_end)
        return 0;
    *bytes = data + start;
    *byte_count = (size_t)(end - start);
    return 1;
}

int nexus_v1_rlowfix_tabl_parse(const uint8_t *data, size_t size,
                                uint32_t resource_offset,
                                Nexus_V1_RlowfixTabl *out)
{
    uint16_t i;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!data || resource_offset > size || size - resource_offset < 8 ||
        memcmp(data + resource_offset, "TABL", 4) != 0) return 0;
    out->entry_count = 216;
    if (size - resource_offset < 8U + (size_t)out->entry_count * 2U)
        return 0;
    out->resource_offset = resource_offset;
    for (i = 0; i < out->entry_count; ++i) {
        const uint8_t *p = data + resource_offset + 8U + (size_t)i * 2U;
        /* DMWeb: 00 xx is a one-byte code; otherwise both bytes form it. */
        out->code[i] = p[0] == 0 ? p[1] :
            (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
    }
    out->valid = 1;
    return 1;
}
