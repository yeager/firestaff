#include "nexus_v1_raw_bin.h"
#include <string.h>

static uint32_t read_be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

static int detect_sh2_code(const uint8_t *data, int size) {
    int sh2_ops = 0, i;
    if (size < 32) return 0;
    for (i = 0; i < size - 1 && i < 128; i += 2) {
        uint8_t hi = data[i];
        if (hi == 0xD0 || hi == 0xD1 || hi == 0x2F ||
            hi == 0x4F || hi == 0x60 || hi == 0x61 ||
            hi == 0xE0 || hi == 0xE1)
            sh2_ops++;
    }
    return sh2_ops >= 8;
}

int nexus_v1_raw_bin_decode(const uint8_t *data, int data_size,
                             Nexus_V1_RawBinDecodeResult *out) {
    int i;
    uint32_t fnv;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!data || data_size < 4) return 0;

    out->file_size = data_size;
    out->prs3_offset = -1;

    for (i = 0; i < data_size; i++) {
        if (data[i] != 0) out->non_zero_bytes++;
    }

    for (i = 0; i <= data_size - 4; i += 4) {
        if (read_be32(data + i) == 0x50525333) {
            out->prs3_offset = i;
            break;
        }
    }

    if (data_size % 32 == 0 && data_size <= 65536 && !detect_sh2_code(data, data_size))
        out->content_type = NEXUS_RAW_TYPE_TILEMAP;
    else if (detect_sh2_code(data, data_size))
        out->content_type = NEXUS_RAW_TYPE_SH2_CODE;
    else
        out->content_type = NEXUS_RAW_TYPE_VDP_DATA;

    fnv = 0x811C9DC5U;
    for (i = 0; i < data_size; i++) {
        fnv = (fnv ^ data[i]) * 0x01000193U;
    }
    out->data_hash = fnv;

    out->valid = 1;
    return 1;
}
