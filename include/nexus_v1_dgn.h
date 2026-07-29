#ifndef NEXUS_V1_DGN_H
#define NEXUS_V1_DGN_H

#include <stdint.h>

#define NEXUS_DGN_BLOCK_SIZE      2048
#define NEXUS_DGN_GRID_SIZE       64
#define NEXUS_DGN_GRID_CELLS      (NEXUS_DGN_GRID_SIZE * NEXUS_DGN_GRID_SIZE)
#define NEXUS_DGN_CELL_SIZE       8
#define NEXUS_DGN_GRID_BYTES      (NEXUS_DGN_GRID_CELLS * NEXUS_DGN_CELL_SIZE)
#define NEXUS_DGN_MODEL_REF_SIZE  24
#define NEXUS_DGN_TEX_DESC_SIZE   20
#define NEXUS_DGN_MAX_TEXTURES    128
#define NEXUS_DGN_MAX_MODELS      128
#define NEXUS_DGN_MAX_DOORS       128

typedef struct {
    uint16_t floor_word;
    uint8_t  byte2;
    int8_t   height;
    uint8_t  byte4;
    uint8_t  byte5;
    uint16_t model_ref;
    int      floor_tex_index;
    int      floor_rotation;
    int      floor_flip_x;
    int      floor_flip_y;
    int      slope;
    int      ceiling_tex_sel;
    int      has_door;
} Nexus_V1_DgnCell;

typedef struct {
    uint16_t image_id;
    uint16_t encoding;
    uint16_t palette_id;
    uint16_t width;
    uint16_t height;
    uint32_t image_offset;
    uint32_t palette_offset;
} Nexus_V1_DgnTexDesc;

typedef struct {
    uint8_t  flags;
    uint8_t  model_index;
    uint8_t  rotation;
    uint8_t  byte3;
} Nexus_V1_DgnModelRef;

typedef struct {
    uint8_t  y;
    uint8_t  x;
    uint8_t  flags;
    uint8_t  orientation_and_index;
    uint8_t  model_index;
    uint8_t  width;
} Nexus_V1_DgnDoor;

typedef struct {
    int valid;
    uint8_t  level_number;
    uint16_t s1_block_offset;
    uint16_t s1_block_count;
    uint32_t s1_data_size;
    uint16_t s2_block_offset;
    uint16_t s2_block_count;
    uint32_t s2_data_size;
    uint16_t s3_block_offset;
    uint16_t s3_block_count;
    uint32_t s3_data_size;

    uint8_t  ceiling_tex[3];
    int      model_ref_count;
    uint32_t s1b_offset;
    uint32_t s1c_offset;
    uint32_t s1e_offset;
    uint32_t s1f_offset;
    uint32_t s1g_offset;

    int      texture_count;
    int      s3_model_count;
    int      door_count;
    int      wall_cell_count;
    int      open_cell_count;

    uint32_t grid_hash;
    uint32_t s2_hash;
} Nexus_V1_DgnDecodeResult;

int nexus_v1_dgn_decode(const uint8_t *data, int data_size,
                         Nexus_V1_DgnDecodeResult *out);

#endif
