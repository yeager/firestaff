#ifndef NEXUS_V1_FACE_BIN_H
#define NEXUS_V1_FACE_BIN_H

#include <stdint.h>

#define NEXUS_FACE_BIN_PORTRAIT_COUNT 20
#define NEXUS_FACE_BIN_PORTRAIT_WIDTH 56
#define NEXUS_FACE_BIN_PORTRAIT_HEIGHT 56
#define NEXUS_FACE_BIN_PALETTE_COLORS 64
#define NEXUS_FACE_BIN_TILE_SIZE 8
#define NEXUS_FACE_BIN_TILES_PER_ROW 7
#define NEXUS_FACE_BIN_TILE_COUNT 49

typedef struct {
    int valid;
    int portrait_count;
    uint32_t file_size;
    uint16_t offsets[NEXUS_FACE_BIN_PORTRAIT_COUNT];
} Nexus_V1_FaceBinHeader;

typedef struct {
    int valid;
    int index;
    uint32_t palette_rgba[NEXUS_FACE_BIN_PALETTE_COLORS];
    uint8_t pixels[NEXUS_FACE_BIN_PORTRAIT_WIDTH *
                   NEXUS_FACE_BIN_PORTRAIT_HEIGHT];
    uint32_t pixel_hash;
} Nexus_V1_FaceBinPortrait;

typedef struct {
    int valid;
    int portrait_count;
    int decoded_count;
    int failed_count;
    Nexus_V1_FaceBinPortrait portraits[NEXUS_FACE_BIN_PORTRAIT_COUNT];
} Nexus_V1_FaceBinDecodeResult;

int nexus_v1_face_bin_parse_header(const uint8_t *data, int data_size,
                                   Nexus_V1_FaceBinHeader *out);

int nexus_v1_face_bin_decode_portrait(const uint8_t *data, int data_size,
                                      const Nexus_V1_FaceBinHeader *header,
                                      int portrait_index,
                                      Nexus_V1_FaceBinPortrait *out);

int nexus_v1_face_bin_decode_all(const uint8_t *data, int data_size,
                                 Nexus_V1_FaceBinDecodeResult *out);

#endif
