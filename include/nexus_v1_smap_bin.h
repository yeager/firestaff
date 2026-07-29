#ifndef NEXUS_V1_SMAP_BIN_H
#define NEXUS_V1_SMAP_BIN_H

#include <stdint.h>

#define NEXUS_SMAP_TILE_WIDTH  80
#define NEXUS_SMAP_TILE_HEIGHT 76
#define NEXUS_SMAP_TILE_SIZE   8
#define NEXUS_SMAP_PIXEL_WIDTH  (NEXUS_SMAP_TILE_WIDTH * NEXUS_SMAP_TILE_SIZE)
#define NEXUS_SMAP_PIXEL_HEIGHT (NEXUS_SMAP_TILE_HEIGHT * NEXUS_SMAP_TILE_SIZE)
#define NEXUS_SMAP_PALETTE_COLORS 256

typedef struct {
    int valid;
    uint32_t file_size;
    uint32_t tilemap_offset;
    uint32_t tilemap_size;
    uint32_t palette_offset;
    uint32_t palette_size;
    uint32_t tileset_offset;
    uint32_t tileset_size;
    int tile_count;
} Nexus_V1_SmapHeader;

typedef struct {
    int valid;
    int level;
    uint32_t palette_rgba[NEXUS_SMAP_PALETTE_COLORS];
    uint32_t pixel_hash;
    int rendered_width;
    int rendered_height;
} Nexus_V1_SmapDecodeResult;

int nexus_v1_smap_parse_header(const uint8_t *data, int data_size,
                                Nexus_V1_SmapHeader *out);

int nexus_v1_smap_decode(const uint8_t *data, int data_size,
                          uint32_t *rgba_out, int rgba_capacity,
                          Nexus_V1_SmapDecodeResult *out);

#endif
