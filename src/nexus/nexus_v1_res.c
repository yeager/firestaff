#include "nexus_v1_res.h"
#include <string.h>

static uint32_t read_be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

static uint16_t read_be16(const uint8_t *p) {
    return (uint16_t)((p[0] << 8) | p[1]);
}

int nexus_v1_res_decode(const uint8_t *data, int data_size,
                         Nexus_V1_ResDecodeResult *out) {
    int count, i;
    uint32_t previous_offset = 0U;
    uint32_t table_end;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!data || data_size < 12) return 0;
    if (read_be32(data) != NEXUS_RES_MAGIC) return 0;

    /* DMWeb Nexus RES* resources use the big-endian file-size/count header
     * and an ordered 12-byte directory whose offsets delimit each payload. */
    out->file_size = read_be32(data + 4);
    count = read_be16(data + 8);
    if (out->file_size != (uint32_t)data_size || count <= 0 ||
        count > NEXUS_RES_MAX_ENTRIES) return 0;
    table_end = 12U + (uint32_t)count * 12U;
    if (table_end > out->file_size) return 0;

    for (i = 0; i < count; ++i) {
        const uint8_t *e = data + 12 + i * 12;
        Nexus_V1_ResEntry *entry = &out->entries[i];
        uint32_t next_off;
        memcpy(entry->tag, e, 4);
        entry->tag[4] = '\0';
        entry->index = read_be32(e + 4);
        entry->offset = read_be32(e + 8);

        next_off = i + 1 < count
            ? read_be32(data + 12 + (i + 1) * 12 + 8)
            : out->file_size;
        if (entry->offset < table_end || entry->offset >= out->file_size ||
            (i > 0 && entry->offset <= previous_offset) ||
            next_off <= entry->offset || next_off > out->file_size) {
            memset(out, 0, sizeof(*out));
            return 0;
        }
        entry->size = next_off - entry->offset;
        previous_offset = entry->offset;
    }
    out->entry_count = count;
    out->valid = 1;
    return 1;
}

const Nexus_V1_ResEntry *nexus_v1_res_find(const Nexus_V1_ResDecodeResult *res,
                                            const char *tag, int index) {
    int i;
    if (!res || !res->valid || !tag) return NULL;
    for (i = 0; i < res->entry_count; ++i) {
        if (memcmp(res->entries[i].tag, tag, 4) == 0 &&
            (int)res->entries[i].index == index) {
            return &res->entries[i];
        }
    }
    return NULL;
}
