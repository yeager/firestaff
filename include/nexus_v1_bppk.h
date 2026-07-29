#ifndef NEXUS_V1_BPPK_H
#define NEXUS_V1_BPPK_H

#include <stdint.h>

#define NEXUS_BPPK_MAGIC       0x4250504B
#define NEXUS_BMPD_MAGIC       0x424D5044
#define NEXUS_BPPK_MAX_ENTRIES 256

typedef struct {
    uint16_t width;
    uint16_t height;
    uint32_t flags;
    uint32_t prs3_offset;
    uint32_t prs3_uncompressed;
    uint32_t prs3_compressed;
    int      valid_prs3;
} Nexus_V1_BppkEntry;

typedef struct {
    int valid;
    uint32_t file_size;
    int entry_count;
    int prs3_count;
    uint32_t data_hash;
} Nexus_V1_BppkDecodeResult;

int nexus_v1_bppk_decode(const uint8_t *data, int data_size,
                          Nexus_V1_BppkDecodeResult *out);

#endif
