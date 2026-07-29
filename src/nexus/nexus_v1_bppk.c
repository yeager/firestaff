#include "nexus_v1_bppk.h"
#include <string.h>

static uint32_t read_be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

int nexus_v1_bppk_decode(const uint8_t *data, int data_size,
                          Nexus_V1_BppkDecodeResult *out) {
    uint32_t bmpd_off;
    int offset_count, i;
    uint32_t fnv;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!data || data_size < 16) return 0;
    if (read_be32(data) != NEXUS_BPPK_MAGIC) return 0;

    out->file_size = read_be32(data + 4);

    bmpd_off = 12;
    if ((int)(bmpd_off + 8) > data_size) return 0;
    if (read_be32(data + bmpd_off) != NEXUS_BMPD_MAGIC) return 0;

    offset_count = 0;
    for (i = 0; i < NEXUS_BPPK_MAX_ENTRIES; ++i) {
        uint32_t entry_off;
        int abs_off;

        if ((int)(bmpd_off + 8 + (i + 1) * 4) > data_size) break;
        entry_off = read_be32(data + bmpd_off + 8 + i * 4);

        if (entry_off == 0) break;
        abs_off = (int)(bmpd_off + entry_off);
        if (abs_off + 8 > data_size) break;

        if (i > 0 && entry_off < read_be32(data + bmpd_off + 8 + (i - 1) * 4)) break;

        offset_count++;

        if (abs_off + 12 <= data_size) {
            uint32_t maybe_prs3 = read_be32(data + abs_off + 8);
            if (maybe_prs3 == 0x50525333) {
                out->prs3_count++;
            }
        }
    }
    out->entry_count = offset_count;

    fnv = 0x811C9DC5U;
    for (i = 0; i < data_size; ++i) {
        fnv = (fnv ^ data[i]) * 0x01000193U;
    }
    out->data_hash = fnv;

    out->valid = 1;
    return 1;
}
