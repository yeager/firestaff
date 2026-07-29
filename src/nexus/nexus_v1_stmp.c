#include "nexus_v1_stmp.h"
#include <string.h>

static uint32_t read_be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

static uint16_t read_be16(const uint8_t *p) {
    return (uint16_t)((p[0] << 8) | p[1]);
}

int nexus_v1_stmp_decode(const uint8_t *data, int data_size,
                          Nexus_V1_StmpDecodeResult *out) {
    uint32_t table_offset;
    int i;
    uint32_t fnv;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!data || data_size < 32) return 0;
    if (read_be32(data) != NEXUS_STMP_MAGIC) return 0;

    out->file_size = read_be32(data + 4);
    table_offset = read_be32(data + 8);
    out->palette_offset = read_be32(data + 16);
    out->palette_color_count = (int)(read_be32(data + 20) / 2);
    out->tiledata_offset = read_be32(data + 24);
    out->tiledata_size = read_be32(data + 28);

    for (i = 0; i < NEXUS_STMP_MAX_PLANES; i++) {
        uint32_t entry_off = table_offset + (uint32_t)i * 4;
        uint32_t rel_off, abs_off;

        if ((int)(entry_off + 4) > data_size) break;
        rel_off = read_be32(data + entry_off);
        if (rel_off == 0) break;

        abs_off = table_offset + rel_off;
        if ((int)(abs_off + 6) > data_size) break;

        out->planes[i].rel_offset = rel_off;
        out->planes[i].width = read_be16(data + abs_off);
        out->planes[i].height = read_be16(data + abs_off + 2);
        out->planes[i].tile_count = out->planes[i].width * out->planes[i].height;
        out->plane_count++;
    }

    fnv = 0x811C9DC5U;
    for (i = 0; i < data_size; i++) {
        fnv = (fnv ^ data[i]) * 0x01000193U;
    }
    out->data_hash = fnv;

    out->valid = 1;
    return 1;
}
