#ifndef NEXUS_V1_LOGOBG_DG2_H
#define NEXUS_V1_LOGOBG_DG2_H

#include <stdint.h>

typedef struct {
    int valid;
    uint16_t format;
    uint16_t width;
    uint16_t height;
    int palette_color_count;
    int pixel_count;
    uint32_t pixel_hash;
    uint32_t data_hash;
} Nexus_V1_LogobgDg2DecodeResult;

int nexus_v1_logobg_dg2_decode(const uint8_t *data, int data_size,
                                Nexus_V1_LogobgDg2DecodeResult *out);

#endif
