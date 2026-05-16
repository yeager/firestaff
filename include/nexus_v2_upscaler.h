
#ifndef NEXUS_V2_UPSCALER_H
#define NEXUS_V2_UPSCALER_H
#include <stdint.h>

/* EPX/Scale2x upscaler for Nexus V2.1.
 * Same algorithm as DM1 V2.1 — pixel-art aware scaling.
 * Takes V1 320x200 indexed → 640x400 RGBA via palette. */

void nexus_v2_epx_upscale(
    const uint8_t *src, int src_w, int src_h,
    uint32_t *dst, int dst_w, int dst_h,
    const uint32_t *palette);

/* Optional bilinear pass after EPX for smoother result */
void nexus_v2_bilinear_smooth(uint32_t *buf, int w, int h);

#endif

