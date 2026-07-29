#include "nexus_v1_title_cg.h"
#include <string.h>

int nexus_v1_title_cg_decode(const uint8_t *data, int data_size,
                              Nexus_V1_TitleCgDecodeResult *out) {
    int expected_size;
    uint32_t fnv;
    int i;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!data || data_size < NEXUS_TITLE_CG_TILE_BYTES) return 0;

    out->tile_count = data_size / NEXUS_TITLE_CG_TILE_BYTES;
    expected_size = out->tile_count * NEXUS_TITLE_CG_TILE_BYTES;

    if (expected_size != data_size) return 0;

    fnv = 0x811C9DC5U;
    for (i = 0; i < data_size; ++i) {
        fnv = (fnv ^ data[i]) * 0x01000193U;
    }
    out->tile_hash = fnv;
    out->valid = 1;
    return 1;
}

int nexus_v1_title_cg_render_tile(const uint8_t *data, int data_size,
                                   int tile_index,
                                   const uint32_t palette[16],
                                   uint32_t *rgba_out) {
    const uint8_t *tile;
    int i;

    if (!data || !palette || !rgba_out) return 0;
    if (tile_index < 0) return 0;
    if ((tile_index + 1) * NEXUS_TITLE_CG_TILE_BYTES > data_size) return 0;

    tile = data + tile_index * NEXUS_TITLE_CG_TILE_BYTES;
    for (i = 0; i < NEXUS_TITLE_CG_TILE_BYTES; ++i) {
        uint8_t b = tile[i];
        rgba_out[i * 2] = palette[(b >> 4) & 0x0F];
        rgba_out[i * 2 + 1] = palette[b & 0x0F];
    }
    return 1;
}
