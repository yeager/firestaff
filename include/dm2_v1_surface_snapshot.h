#ifndef FIRESTAFF_DM2_V1_SURFACE_SNAPSHOT_H
#define FIRESTAFF_DM2_V1_SURFACE_SNAPSHOT_H

#include <stdint.h>

typedef struct {
    uint8_t *framebuffer;
    uint16_t width;
    uint16_t height;
    uint16_t stride;
    uint8_t resolution;
    uint32_t generation;
} DM2_V1_ViewportSurfaceSnapshot;

#endif
