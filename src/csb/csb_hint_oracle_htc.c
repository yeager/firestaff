/*
 * csb_hint_oracle_htc.c
 *
 * Conservative HCSB.HTC parser for the CSB Utility Disk Hint Oracle.
 * See include/csb_hint_oracle_htc.h for source references and scope.
 */

#include "csb_hint_oracle_htc.h"

#include <string.h>

#define CSB_HTC_LZW_FIRST_DYNAMIC_CODE 257
#define CSB_HTC_LZW_CLEAR_CODE 256
#define CSB_HTC_LZW_MAX_CODES 1024
#define CSB_HTC_LZW_MAX_BITS 10

static uint16_t read_be16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8u) | (uint16_t)p[1]);
}

static int checked_advance(size_t data_size,
                           size_t *offset,
                           size_t count,
                           size_t elem_size)
{
    size_t bytes;

    if (!offset ||
        (elem_size != 0u && count > ((size_t)-1) / elem_size)) {
        return 0;
    }
    bytes = count * elem_size;
    if (*offset > data_size || bytes > data_size - *offset) {
        return 0;
    }
    *offset += bytes;
    return 1;
}

uint16_t csb_hint_oracle_htc_page_compressed_length(
    const CSB_HintOracleHTC *htc,
    size_t page_index)
{
    if (!htc || !htc->page_lengths || page_index >= htc->page_count) {
        return 0;
    }
    return read_be16(htc->page_lengths + (page_index * 2u));
}

int csb_hint_oracle_htc_parse(const uint8_t *data,
                              size_t data_size,
                              CSB_HintOracleHTC *out)
{
    CSB_HintOracleHTC parsed;
    size_t offset;
    size_t i;
    size_t total_content_size = 0;
    uint16_t location_count;
    uint16_t hint_count;
    uint16_t page_count;

    if (!data || !out) {
        return CSB_HINT_ORACLE_HTC_ERR_ARGUMENT;
    }
    memset(&parsed, 0, sizeof(parsed));
    if (data_size < 10u) {
        return CSB_HINT_ORACLE_HTC_ERR_TRUNCATED;
    }

    parsed.format_word = read_be16(data + 0u);
    parsed.dungeon_id = read_be16(data + 2u);
    parsed.header_word_count = read_be16(data + 4u);
    if (parsed.format_word != CSB_HINT_ORACLE_HTC_FORMAT_WORD) {
        return CSB_HINT_ORACLE_HTC_ERR_BAD_FORMAT;
    }
    if (parsed.dungeon_id != CSB_HINT_ORACLE_HTC_DUNGEON_ID) {
        return CSB_HINT_ORACLE_HTC_ERR_BAD_DUNGEON;
    }
    if (parsed.header_word_count < 3u ||
        (size_t)parsed.header_word_count > data_size / 2u) {
        return CSB_HINT_ORACLE_HTC_ERR_BAD_HEADER;
    }

    offset = (size_t)parsed.header_word_count * 2u;
    if (!checked_advance(data_size, &offset, 2u, 2u)) {
        return CSB_HINT_ORACLE_HTC_ERR_TRUNCATED;
    }
    location_count = read_be16(data + offset - 4u);
    parsed.location_record_size = read_be16(data + offset - 2u);
    if (parsed.location_record_size !=
        CSB_HINT_ORACLE_HTC_LOCATION_RECORD_SIZE) {
        return CSB_HINT_ORACLE_HTC_ERR_BAD_RECORD_SIZE;
    }
    parsed.locations = data + offset;
    if (!checked_advance(data_size, &offset, location_count,
                         parsed.location_record_size)) {
        return CSB_HINT_ORACLE_HTC_ERR_TRUNCATED;
    }

    if (!checked_advance(data_size, &offset, 2u, 2u)) {
        return CSB_HINT_ORACLE_HTC_ERR_TRUNCATED;
    }
    hint_count = read_be16(data + offset - 4u);
    parsed.hint_record_size = read_be16(data + offset - 2u);
    if (parsed.hint_record_size != CSB_HINT_ORACLE_HTC_HINT_RECORD_SIZE) {
        return CSB_HINT_ORACLE_HTC_ERR_BAD_RECORD_SIZE;
    }
    parsed.hints = data + offset;
    if (!checked_advance(data_size, &offset, hint_count,
                         parsed.hint_record_size)) {
        return CSB_HINT_ORACLE_HTC_ERR_TRUNCATED;
    }

    if (!checked_advance(data_size, &offset, 1u, 2u)) {
        return CSB_HINT_ORACLE_HTC_ERR_TRUNCATED;
    }
    page_count = read_be16(data + offset - 2u);
    parsed.page_lengths = data + offset;
    if (!checked_advance(data_size, &offset, page_count, 2u)) {
        return CSB_HINT_ORACLE_HTC_ERR_TRUNCATED;
    }

    parsed.contents = data + offset;
    parsed.content_offset = offset;
    parsed.content_size = data_size - offset;
    parsed.location_count = location_count;
    parsed.hint_count = hint_count;
    parsed.page_count = page_count;
    parsed.data = data;
    parsed.data_size = data_size;

    for (i = 0; i < parsed.page_count; ++i) {
        total_content_size += csb_hint_oracle_htc_page_compressed_length(
            &parsed, i);
    }
    if (total_content_size != parsed.content_size) {
        return CSB_HINT_ORACLE_HTC_ERR_BAD_CONTENT_SIZE;
    }

    for (i = 0; i < parsed.hint_count; ++i) {
        uint16_t first_page;
        uint16_t pages;
        const uint8_t *hint = parsed.hints + (i * parsed.hint_record_size);
        first_page = read_be16(hint + CSB_HINT_ORACLE_HTC_HINT_NAME_BYTES);
        pages = read_be16(hint + CSB_HINT_ORACLE_HTC_HINT_NAME_BYTES + 2u);
        if (pages == 0u ||
            first_page >= parsed.page_count ||
            (size_t)pages > parsed.page_count - (size_t)first_page ||
            csb_hint_oracle_htc_page_compressed_length(&parsed,
                                                       first_page) == 0u) {
            return CSB_HINT_ORACLE_HTC_ERR_BAD_HINT_RANGE;
        }
    }

    for (i = 0; i < parsed.location_count; ++i) {
        const uint8_t *loc = parsed.locations + (i * parsed.location_record_size);
        uint16_t hint_index = read_be16(loc + 4u);
        if (hint_index >= parsed.hint_count) {
            return CSB_HINT_ORACLE_HTC_ERR_BAD_HINT_RANGE;
        }
    }

    *out = parsed;
    return CSB_HINT_ORACLE_HTC_OK;
}

int csb_hint_oracle_htc_get_location(
    const CSB_HintOracleHTC *htc,
    size_t index,
    CSB_HintOracleHTC_Location *out_location)
{
    const uint8_t *loc;

    if (!htc || !htc->locations || !out_location ||
        index >= htc->location_count) {
        return CSB_HINT_ORACLE_HTC_ERR_ARGUMENT;
    }

    loc = htc->locations + (index * htc->location_record_size);
    out_location->x = loc[0];
    out_location->y = loc[1];
    out_location->level = loc[2];
    out_location->unused = loc[3];
    out_location->hint_index = read_be16(loc + 4u);
    return CSB_HINT_ORACLE_HTC_OK;
}

int csb_hint_oracle_htc_get_hint(const CSB_HintOracleHTC *htc,
                                 size_t index,
                                 CSB_HintOracleHTC_Hint *out_hint)
{
    const uint8_t *hint;
    size_t i;

    if (!htc || !htc->hints || !out_hint || index >= htc->hint_count) {
        return CSB_HINT_ORACLE_HTC_ERR_ARGUMENT;
    }

    hint = htc->hints + (index * htc->hint_record_size);
    memset(out_hint, 0, sizeof(*out_hint));
    for (i = 0; i < CSB_HINT_ORACLE_HTC_HINT_NAME_BYTES; ++i) {
        if (hint[i] == 0u) {
            break;
        }
        out_hint->name[i] = (char)hint[i];
    }
    out_hint->name[CSB_HINT_ORACLE_HTC_HINT_NAME_BYTES] = '\0';
    out_hint->first_page_index = read_be16(
        hint + CSB_HINT_ORACLE_HTC_HINT_NAME_BYTES);
    out_hint->page_count = read_be16(
        hint + CSB_HINT_ORACLE_HTC_HINT_NAME_BYTES + 2u);
    return CSB_HINT_ORACLE_HTC_OK;
}

int csb_hint_oracle_htc_find_hints_for_location(
    const CSB_HintOracleHTC *htc,
    uint8_t level,
    uint8_t x,
    uint8_t y,
    uint16_t *out_hint_indices,
    size_t out_hint_capacity,
    size_t *out_hint_count)
{
    size_t i;
    size_t matches = 0;

    if (!htc || !out_hint_count ||
        (out_hint_capacity > 0u && !out_hint_indices)) {
        return CSB_HINT_ORACLE_HTC_ERR_ARGUMENT;
    }

    for (i = 0; i < htc->location_count; ++i) {
        CSB_HintOracleHTC_Location loc;
        int rc = csb_hint_oracle_htc_get_location(htc, i, &loc);
        if (rc != CSB_HINT_ORACLE_HTC_OK) {
            return rc;
        }
        if (loc.level == level &&
            ((loc.x == x && loc.y == y) ||
             (loc.x == CSB_HINT_ORACLE_HTC_ANY_XY &&
              loc.y == CSB_HINT_ORACLE_HTC_ANY_XY))) {
            if (matches < out_hint_capacity) {
                out_hint_indices[matches] = loc.hint_index;
            }
            ++matches;
        }
    }

    *out_hint_count = matches;
    return matches > out_hint_capacity ?
        CSB_HINT_ORACLE_HTC_ERR_OUTPUT_TOO_SMALL :
        CSB_HINT_ORACLE_HTC_OK;
}

int csb_hint_oracle_htc_get_hint_content_slice(
    const CSB_HintOracleHTC *htc,
    size_t hint_index,
    const uint8_t **out_compressed,
    size_t *out_compressed_size)
{
    CSB_HintOracleHTC_Hint hint;
    size_t offset = 0;
    size_t i;
    uint16_t length;
    int rc;

    if (!htc || !out_compressed || !out_compressed_size ||
        hint_index >= htc->hint_count) {
        return CSB_HINT_ORACLE_HTC_ERR_ARGUMENT;
    }

    rc = csb_hint_oracle_htc_get_hint(htc, hint_index, &hint);
    if (rc != CSB_HINT_ORACLE_HTC_OK) {
        return rc;
    }
    for (i = 0; i < hint.first_page_index; ++i) {
        offset += csb_hint_oracle_htc_page_compressed_length(htc, i);
    }
    length = csb_hint_oracle_htc_page_compressed_length(
        htc, hint.first_page_index);
    if (offset > htc->content_size || (size_t)length > htc->content_size - offset) {
        return CSB_HINT_ORACLE_HTC_ERR_BAD_CONTENT_SIZE;
    }

    *out_compressed = htc->contents + offset;
    *out_compressed_size = length;
    return CSB_HINT_ORACLE_HTC_OK;
}

static int lzw_read_code(const uint8_t *compressed,
                         size_t compressed_size,
                         size_t *bit_pos,
                         int bit_count)
{
    size_t total_bits = compressed_size * 8u;
    unsigned int code = 0;
    int i;

    if (!bit_pos || bit_count <= 0 || *bit_pos + (size_t)bit_count > total_bits) {
        return -1;
    }
    for (i = 0; i < bit_count; ++i) {
        size_t pos = *bit_pos + (size_t)i;
        if ((compressed[pos / 8u] & (uint8_t)(1u << (pos % 8u))) != 0u) {
            code |= 1u << (unsigned int)i;
        }
    }
    *bit_pos += (size_t)bit_count;
    return (int)code;
}

static int lzw_emit_byte(uint8_t byte,
                         uint8_t *out,
                         size_t out_capacity,
                         size_t *out_pos,
                         int *repeat_pending,
                         uint8_t *last_byte)
{
    if (!out || !out_pos || !repeat_pending || !last_byte) {
        return CSB_HINT_ORACLE_HTC_ERR_ARGUMENT;
    }

    if (*repeat_pending) {
        *repeat_pending = 0;
        if (byte == 0u) {
            if (*out_pos >= out_capacity) {
                return CSB_HINT_ORACLE_HTC_ERR_OUTPUT_TOO_SMALL;
            }
            out[(*out_pos)++] = 0x90u;
            *last_byte = 0x90u;
            return CSB_HINT_ORACLE_HTC_OK;
        }
        while (byte > 1u) {
            if (*out_pos >= out_capacity) {
                return CSB_HINT_ORACLE_HTC_ERR_OUTPUT_TOO_SMALL;
            }
            out[(*out_pos)++] = *last_byte;
            --byte;
        }
        return CSB_HINT_ORACLE_HTC_OK;
    }

    if (byte == 0x90u) {
        *repeat_pending = 1;
        return CSB_HINT_ORACLE_HTC_OK;
    }
    if (*out_pos >= out_capacity) {
        return CSB_HINT_ORACLE_HTC_ERR_OUTPUT_TOO_SMALL;
    }
    out[(*out_pos)++] = byte;
    *last_byte = byte;
    return CSB_HINT_ORACLE_HTC_OK;
}

static int lzw_emit_string(int code,
                           const int *prefix,
                           const uint8_t *suffix,
                           uint8_t *stack,
                           size_t stack_capacity,
                           uint8_t *out,
                           size_t out_capacity,
                           size_t *out_pos,
                           int *repeat_pending,
                           uint8_t *last_byte,
                           uint8_t *first_char)
{
    size_t stack_size = 0;
    int rc;

    if (code < 0 || code >= CSB_HTC_LZW_MAX_CODES) {
        return CSB_HINT_ORACLE_HTC_ERR_BAD_LZW;
    }
    while (code >= 256) {
        if (code >= CSB_HTC_LZW_MAX_CODES ||
            stack_size >= stack_capacity) {
            return CSB_HINT_ORACLE_HTC_ERR_BAD_LZW;
        }
        stack[stack_size++] = suffix[code];
        code = prefix[code];
    }
    if (stack_size >= stack_capacity) {
        return CSB_HINT_ORACLE_HTC_ERR_BAD_LZW;
    }
    stack[stack_size++] = (uint8_t)code;
    *first_char = (uint8_t)code;

    while (stack_size > 0u) {
        rc = lzw_emit_byte(stack[--stack_size], out, out_capacity, out_pos,
                           repeat_pending, last_byte);
        if (rc != CSB_HINT_ORACLE_HTC_OK) {
            return rc;
        }
    }
    return CSB_HINT_ORACLE_HTC_OK;
}

int csb_hint_oracle_htc_lzw_decompress(const uint8_t *compressed,
                                       size_t compressed_size,
                                       uint8_t *out,
                                       size_t out_capacity,
                                       size_t *out_size)
{
    int prefix[CSB_HTC_LZW_MAX_CODES];
    uint8_t suffix[CSB_HTC_LZW_MAX_CODES];
    uint8_t stack[CSB_HTC_LZW_MAX_CODES];
    size_t bit_pos = 0;
    size_t out_pos = 0;
    int bit_count = 9;
    int current_max_code = (1 << 9) - 1;
    int next_code = CSB_HTC_LZW_FIRST_DYNAMIC_CODE;
    int old_code;
    int code;
    int repeat_pending = 0;
    uint8_t last_byte = 0;
    uint8_t first_char = 0;
    int i;
    int rc;

    if (!compressed || !out || !out_size) {
        return CSB_HINT_ORACLE_HTC_ERR_ARGUMENT;
    }

    for (i = 0; i < CSB_HTC_LZW_MAX_CODES; ++i) {
        prefix[i] = 0;
        suffix[i] = (uint8_t)(i & 0xff);
    }

    old_code = lzw_read_code(compressed, compressed_size, &bit_pos, bit_count);
    if (old_code < 0 || old_code >= 256) {
        return CSB_HINT_ORACLE_HTC_ERR_BAD_LZW;
    }
    rc = lzw_emit_byte((uint8_t)old_code, out, out_capacity, &out_pos,
                       &repeat_pending, &last_byte);
    if (rc != CSB_HINT_ORACLE_HTC_OK) {
        return rc;
    }
    first_char = (uint8_t)old_code;

    while ((code = lzw_read_code(compressed, compressed_size,
                                 &bit_pos, bit_count)) >= 0) {
        int in_code = code;

        if (code == CSB_HTC_LZW_CLEAR_CODE) {
            for (i = 0; i < 256; ++i) {
                prefix[i] = 0;
                suffix[i] = (uint8_t)i;
            }
            bit_count = 9;
            current_max_code = (1 << bit_count) - 1;
            next_code = CSB_HTC_LZW_CLEAR_CODE;
            old_code = lzw_read_code(compressed, compressed_size,
                                     &bit_pos, bit_count);
            if (old_code < 0) {
                break;
            }
            if (old_code >= 256) {
                return CSB_HINT_ORACLE_HTC_ERR_BAD_LZW;
            }
            rc = lzw_emit_byte((uint8_t)old_code, out, out_capacity,
                               &out_pos, &repeat_pending, &last_byte);
            if (rc != CSB_HINT_ORACLE_HTC_OK) {
                return rc;
            }
            first_char = (uint8_t)old_code;
            continue;
        }

        if (code >= next_code) {
            if (code != next_code) {
                return CSB_HINT_ORACLE_HTC_ERR_BAD_LZW;
            }
            rc = lzw_emit_string(old_code, prefix, suffix, stack,
                                 sizeof(stack), out, out_capacity, &out_pos,
                                 &repeat_pending, &last_byte, &first_char);
            if (rc != CSB_HINT_ORACLE_HTC_OK) {
                return rc;
            }
            rc = lzw_emit_byte(first_char, out, out_capacity, &out_pos,
                               &repeat_pending, &last_byte);
            if (rc != CSB_HINT_ORACLE_HTC_OK) {
                return rc;
            }
        } else {
            rc = lzw_emit_string(code, prefix, suffix, stack,
                                 sizeof(stack), out, out_capacity, &out_pos,
                                 &repeat_pending, &last_byte, &first_char);
            if (rc != CSB_HINT_ORACLE_HTC_OK) {
                return rc;
            }
        }

        if (next_code < CSB_HTC_LZW_MAX_CODES) {
            prefix[next_code] = old_code;
            suffix[next_code] = first_char;
            ++next_code;
            if (next_code > current_max_code &&
                bit_count < CSB_HTC_LZW_MAX_BITS) {
                ++bit_count;
                current_max_code = (1 << bit_count) - 1;
            }
        }
        old_code = in_code;
    }

    *out_size = out_pos;
    return CSB_HINT_ORACLE_HTC_OK;
}

const char *csb_hint_oracle_htc_result_name(int result)
{
    switch (result) {
    case CSB_HINT_ORACLE_HTC_OK:
        return "OK";
    case CSB_HINT_ORACLE_HTC_ERR_ARGUMENT:
        return "ERR_ARGUMENT";
    case CSB_HINT_ORACLE_HTC_ERR_TRUNCATED:
        return "ERR_TRUNCATED";
    case CSB_HINT_ORACLE_HTC_ERR_BAD_FORMAT:
        return "ERR_BAD_FORMAT";
    case CSB_HINT_ORACLE_HTC_ERR_BAD_DUNGEON:
        return "ERR_BAD_DUNGEON";
    case CSB_HINT_ORACLE_HTC_ERR_BAD_HEADER:
        return "ERR_BAD_HEADER";
    case CSB_HINT_ORACLE_HTC_ERR_BAD_RECORD_SIZE:
        return "ERR_BAD_RECORD_SIZE";
    case CSB_HINT_ORACLE_HTC_ERR_BAD_HINT_RANGE:
        return "ERR_BAD_HINT_RANGE";
    case CSB_HINT_ORACLE_HTC_ERR_BAD_CONTENT_SIZE:
        return "ERR_BAD_CONTENT_SIZE";
    case CSB_HINT_ORACLE_HTC_ERR_OUTPUT_TOO_SMALL:
        return "ERR_OUTPUT_TOO_SMALL";
    case CSB_HINT_ORACLE_HTC_ERR_BAD_LZW:
        return "ERR_BAD_LZW";
    default:
        return "ERR_UNKNOWN";
    }
}
