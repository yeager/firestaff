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

int csb_hint_oracle_dat_img2_decode(const uint8_t *s, size_t n,
                                    uint16_t *ow, uint16_t *oh,
                                    uint8_t *out, size_t cap, size_t *used)
{
    uint16_t w, h;
    size_t total, src = 4u, pos = 0u, count, i;
    if (used) *used = 0u;
    if (!s || !ow || !oh || !out || n < 4u) return 0;
    w = read_be16(s); h = read_be16(s + 2u);
    if (!w || !h || w > 640u || h > 400u || (size_t)w > ((size_t)-1) / h) return 0;
    total = (size_t)w * h;
    if (total > cap) return 0;
    while (pos < total && src < n) {
        uint8_t cmd = s[src++], color = cmd & 15u;
        if (!(cmd & 0x80u)) {
            count = ((size_t)(cmd >> 4u) & 7u) + 1u;
            if (count > total - pos) return 0;
            memset(out + pos, color, count); pos += count; continue;
        }
        if (cmd & 0x40u) { if (src + 1u >= n) return 0; count = ((size_t)s[src] << 8u | s[src + 1u]) + 1u; src += 2u; }
        else { if (src >= n) return 0; count = (size_t)s[src++] + 1u; }
        if ((cmd & 0x30u) == 0u) { if (count > total - pos) return 0; memset(out + pos, color, count); pos += count; }
        else if ((cmd & 0x30u) == 0x10u) {
            if (count > total - pos) return 0;
            if (count & 1u) { out[pos++] = color; --count; }
            if (src + count / 2u > n) return 0;
            for (i = 0u; i < count / 2u; ++i) { uint8_t p = s[src++]; out[pos++] = p >> 4u; out[pos++] = p & 15u; }
        } else if ((cmd & 0x30u) == 0x30u) {
            if (count >= total - pos || pos < w || count > pos % w + w) return 0;
            for (i = 0u; i < count; ++i) { out[pos] = out[pos - w]; ++pos; }
            out[pos++] = color;
        } else return 0;
    }
    if (pos != total) return 0;
    *ow = w; *oh = h; if (used) *used = src; return 1;
}

int csb_hint_oracle_dat_palette_decode(const uint8_t *segment,
                                       size_t segment_size,
                                       uint8_t out_rgb4[48])
{
    size_t i;
    if (!segment || !out_rgb4 || segment_size != 32u) return 0;
    for (i = 0u; i < 16u; ++i) {
        uint16_t color = read_be16(segment + (i * 2u));
        out_rgb4[i * 3u] = (uint8_t)((((color & 0x0700u) >> 8u) * 15u) / 7u);
        out_rgb4[i * 3u + 1u] = (uint8_t)((((color & 0x0070u) >> 4u) * 15u) / 7u);
        out_rgb4[i * 3u + 2u] = (uint8_t)(((color & 0x0007u) * 15u) / 7u);
    }
    out_rgb4[0] = out_rgb4[1] = out_rgb4[2] = 0u;
    return 1;
}
