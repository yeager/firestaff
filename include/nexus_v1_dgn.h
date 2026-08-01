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

/* Structure1Fa: floor item (DMWeb: 8 bytes per entry, first byte always 0x10) */
typedef struct {
    uint8_t  marker;       /* always 0x10 */
    uint8_t  x;            /* X in 64x64 grid */
    uint8_t  y;            /* Y in 64x64 grid */
    uint8_t  location;     /* 0=NW 1=NE 2=SE 3=SW 4=Center */
    uint8_t  item_id;      /* ITEM.IBS index */
    uint8_t  attr1;        /* magic item attribute */
    uint8_t  reserved;     /* always 0 */
    uint8_t  attr2;        /* charges / fill level */
} Nexus_V1_DgnFloorItem;

/* Structure1Fb: floor decoration (DMWeb: 12 bytes) */
typedef struct {
    uint8_t  marker;       /* always 0x11 */
    uint8_t  x;
    uint8_t  y;
    int8_t   offset_x;    /* signed, from cell center */
    int8_t   offset_y;
    uint8_t  model_index;  /* Structure3 model index */
    uint8_t  rotation;     /* 0-255, +64 = 90 degrees */
    uint8_t  type;         /* 0x03=model, 0x82=texture */
    uint8_t  tex_width;
    uint8_t  tex_height;
    uint8_t  byte10;
    uint8_t  byte11;
} Nexus_V1_DgnFloorDecor;

/* Structure1Fc: floor sensor (DMWeb: 16 bytes) */
typedef struct {
    uint8_t  marker;       /* always 0x12 */
    uint8_t  x;
    uint8_t  y;
    uint8_t  byte3;
    uint8_t  byte4;
    uint8_t  model_disabled; /* Structure3 model for disabled state, 0xFF=none */
    uint8_t  model_enabled;  /* Structure3 model for enabled state, 0xFF=none */
    uint8_t  byte7;
    uint8_t  byte8;
    uint8_t  byte9;
    uint8_t  active_width;   /* 0..80 */
    uint8_t  active_height;  /* 0..80 */
    uint8_t  sensor_type;
    uint8_t  dest_x;
    uint8_t  dest_y;
    uint8_t  dest_orientation;
} Nexus_V1_DgnFloorSensor;

/* Structure1Fe: wall decoration (DMWeb: 12 bytes) */
typedef struct {
    uint8_t  marker;       /* always 0x14 */
    uint8_t  x;
    uint8_t  y;
    uint8_t  wall_side;    /* 0=N 1=E 2=S 3=W */
    uint8_t  model_index;
    int8_t   offset_x;
    int8_t   offset_y;
    uint8_t  rotation;
    uint8_t  byte8;
    uint8_t  byte9;
    uint8_t  byte10;
    uint8_t  byte11;
} Nexus_V1_DgnWallDecor;

/* Structure1Ff: wall sensor (DMWeb: 12 bytes) */
typedef struct {
    uint8_t  marker;       /* always 0x15 */
    uint8_t  x;
    uint8_t  y;
    uint8_t  wall_side;
    uint8_t  tex_index;    /* wall texture for sensor */
    uint8_t  sensor_type;
    uint8_t  dest_x;
    uint8_t  dest_y;
    uint8_t  dest_orientation;
    uint8_t  byte9;
    uint8_t  byte10;
    uint8_t  byte11;
} Nexus_V1_DgnWallSensor;

int nexus_v1_dgn_decode(const uint8_t *data, int data_size,
                         Nexus_V1_DgnDecodeResult *out);

#endif
