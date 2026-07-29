#ifndef NEXUS_V1_TITLE_CG_H
#define NEXUS_V1_TITLE_CG_H

#include <stdint.h>

#define NEXUS_TITLE_CG_TILE_SIZE     8
#define NEXUS_TITLE_CG_TILE_BYTES    32
#define NEXUS_TITLE_CG_TILE_COUNT    5249

typedef struct {
    int valid;
    int tile_count;
    uint32_t tile_hash;
} Nexus_V1_TitleCgDecodeResult;

int nexus_v1_title_cg_decode(const uint8_t *data, int data_size,
                              Nexus_V1_TitleCgDecodeResult *out);

int nexus_v1_title_cg_render_tile(const uint8_t *data, int data_size,
                                   int tile_index,
                                   const uint32_t palette[16],
                                   uint32_t *rgba_out);

#endif
