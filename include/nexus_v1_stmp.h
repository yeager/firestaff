#ifndef NEXUS_V1_STMP_H
#define NEXUS_V1_STMP_H

#include <stdint.h>

#define NEXUS_STMP_MAGIC        0x53544D50
#define NEXUS_STMP_MAX_PLANES   16

typedef struct {
    uint32_t rel_offset;
    uint16_t width;
    uint16_t height;
    int      tile_count;
} Nexus_V1_StmpPlane;

typedef struct {
    int valid;
    uint32_t file_size;
    int plane_count;
    Nexus_V1_StmpPlane planes[NEXUS_STMP_MAX_PLANES];
    uint32_t palette_offset;
    int      palette_color_count;
    uint32_t tiledata_offset;
    uint32_t tiledata_size;
    uint32_t data_hash;
} Nexus_V1_StmpDecodeResult;

int nexus_v1_stmp_decode(const uint8_t *data, int data_size,
                          Nexus_V1_StmpDecodeResult *out);

#endif
