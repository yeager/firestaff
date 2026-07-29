#include "nexus_v1_logobg_dg2.h"
#include <string.h>

static uint16_t read_be16(const uint8_t *p) {
    return (uint16_t)((p[0] << 8) | p[1]);
}

int nexus_v1_logobg_dg2_decode(const uint8_t *data, int data_size,
                                Nexus_V1_LogobgDg2DecodeResult *out) {
    uint32_t fnv;
    int expected, pal_bytes, pixel_bytes, i;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!data || data_size < 6) return 0;

    out->format = read_be16(data);
    out->width = read_be16(data + 2);
    out->height = read_be16(data + 4);

    if (out->width == 0 || out->height == 0) return 0;
    if (out->width > 1024 || out->height > 1024) return 0;

    out->pixel_count = out->width * out->height;
    out->palette_color_count = 256;
    pal_bytes = 256 * 2;
    pixel_bytes = out->pixel_count;
    expected = 6 + pal_bytes + pixel_bytes;
    if (data_size != expected) return 0;

    fnv = 0x811C9DC5U;
    for (i = 6 + pal_bytes; i < data_size; i++) {
        fnv = (fnv ^ data[i]) * 0x01000193U;
    }
    out->pixel_hash = fnv;

    fnv = 0x811C9DC5U;
    for (i = 0; i < data_size; i++) {
        fnv = (fnv ^ data[i]) * 0x01000193U;
    }
    out->data_hash = fnv;

    out->valid = 1;
    return 1;
}
