#ifndef NEXUS_V1_PRS3_DECODE_H
#define NEXUS_V1_PRS3_DECODE_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    int valid;
    uint32_t version;
    uint32_t uncompressed_size;
    uint32_t compressed_size;
    const uint8_t *stream;
} Nexus_V1_Prs3Header;

typedef struct {
    int success;
    uint32_t bytes_produced;
    uint32_t bytes_consumed;
} Nexus_V1_Prs3DecodeResult;

int nexus_v1_prs3_parse_header(const uint8_t *data, int data_size,
                               Nexus_V1_Prs3Header *out);

Nexus_V1_Prs3DecodeResult nexus_v1_prs3_decompress(
    const uint8_t *compressed, int compressed_size,
    uint8_t *output, int output_capacity,
    int uncompressed_size);

#endif
