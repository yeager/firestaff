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

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!data || data_size < 12) return 0;
    if (read_be32(data) != NEXUS_RES_MAGIC) return 0;

    out->file_size = read_be32(data + 4);
    count = read_be16(data + 8);
    if (count > NEXUS_RES_MAX_ENTRIES) count = NEXUS_RES_MAX_ENTRIES;

    if (12 + count * 12 > data_size) return 0;

    for (i = 0; i < count; ++i) {
        const uint8_t *e = data + 12 + i * 12;
        Nexus_V1_ResEntry *entry = &out->entries[i];
        memcpy(entry->tag, e, 4);
        entry->tag[4] = '\0';
        entry->index = read_be32(e + 4);
        entry->offset = read_be32(e + 8);

        if (i + 1 < count) {
            uint32_t next_off = read_be32(data + 12 + (i + 1) * 12 + 8);
            entry->size = next_off - entry->offset;
        } else {
            entry->size = out->file_size - entry->offset;
        }
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
