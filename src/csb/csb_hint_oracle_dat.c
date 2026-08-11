/* See include/csb_hint_oracle_dat.h. */
#include "csb_hint_oracle_dat.h"

#include <string.h>

static uint16_t read_be16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8u) | (uint16_t)p[1]);
}

int csb_hint_oracle_dat_parse(const uint8_t *data, size_t data_size,
                              CSB_HintOracleDAT *out)
{
    CSB_HintOracleDAT parsed;
    size_t header_size;
    size_t offset;
    size_t i;
    uint16_t count;

    if (!data || !out) {
        return CSB_HINT_ORACLE_DAT_ERR_ARGUMENT;
    }
    if (data_size < 2u) {
        return CSB_HINT_ORACLE_DAT_ERR_TRUNCATED;
    }
    count = read_be16(data);
    if (count == 0u || count > CSB_HINT_ORACLE_DAT_MAX_SEGMENTS) {
        return CSB_HINT_ORACLE_DAT_ERR_BAD_COUNT;
    }
    header_size = 2u + ((size_t)count * 4u);
    if (header_size > data_size) {
        return CSB_HINT_ORACLE_DAT_ERR_TRUNCATED;
    }
    memset(&parsed, 0, sizeof(parsed));
    parsed.data = data;
    parsed.data_size = data_size;
    parsed.segment_count = count;
    parsed.payload_offset = header_size;
    offset = header_size;
    for (i = 0u; i < count; ++i) {
        uint16_t first = read_be16(data + 2u + (i * 2u));
        uint16_t second = read_be16(data + 2u + ((size_t)count * 2u) +
                                    (i * 2u));
        if (first != second) {
            return CSB_HINT_ORACLE_DAT_ERR_BAD_TABLE;
        }
        if (first == 0u || (size_t)first > data_size - offset) {
            return CSB_HINT_ORACLE_DAT_ERR_BAD_SIZE;
        }
        parsed.segment_offsets[i] = offset;
        parsed.segment_sizes[i] = first;
        offset += (size_t)first;
    }
    if (offset != data_size) {
        return CSB_HINT_ORACLE_DAT_ERR_BAD_SIZE;
    }
    *out = parsed;
    return CSB_HINT_ORACLE_DAT_OK;
}

int csb_hint_oracle_dat_get_segment(const CSB_HintOracleDAT *archive,
                                    size_t index,
                                    const uint8_t **out_bytes,
                                    size_t *out_size)
{
    if (!archive || !out_bytes || !out_size || !archive->data ||
        index >= archive->segment_count) {
        return CSB_HINT_ORACLE_DAT_ERR_ARGUMENT;
    }
    *out_bytes = archive->data + archive->segment_offsets[index];
    *out_size = archive->segment_sizes[index];
    return CSB_HINT_ORACLE_DAT_OK;
}

const char *csb_hint_oracle_dat_result_name(int result)
{
    switch (result) {
    case CSB_HINT_ORACLE_DAT_OK: return "OK";
    case CSB_HINT_ORACLE_DAT_ERR_ARGUMENT: return "argument";
    case CSB_HINT_ORACLE_DAT_ERR_TRUNCATED: return "truncated";
    case CSB_HINT_ORACLE_DAT_ERR_BAD_COUNT: return "bad-count";
    case CSB_HINT_ORACLE_DAT_ERR_BAD_TABLE: return "bad-table";
    case CSB_HINT_ORACLE_DAT_ERR_BAD_SIZE: return "bad-size";
    default: return "unknown";
    }
}
