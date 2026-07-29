#ifndef NEXUS_V1_RAW_BIN_H
#define NEXUS_V1_RAW_BIN_H

#include <stdint.h>

#define NEXUS_RAW_TYPE_UNKNOWN   0
#define NEXUS_RAW_TYPE_VDP_DATA  1
#define NEXUS_RAW_TYPE_SH2_CODE  2
#define NEXUS_RAW_TYPE_TILEMAP   3

typedef struct {
    int valid;
    int content_type;
    int file_size;
    int non_zero_bytes;
    int prs3_offset;
    uint32_t data_hash;
} Nexus_V1_RawBinDecodeResult;

int nexus_v1_raw_bin_decode(const uint8_t *data, int data_size,
                             Nexus_V1_RawBinDecodeResult *out);

#endif
