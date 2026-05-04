#ifndef FIRESTAFF_DM1_V2_TEXTURE_UPSCALE_PC34_H
#define FIRESTAFF_DM1_V2_TEXTURE_UPSCALE_PC34_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int scale_factor;
    bool use_bilinear;
    bool sharpen;
} M11_V2_TextureUpscaleConfig;

void v2_upscale_init(M11_V2_TextureUpscaleConfig* config);
void v2_upscale_nearest(const uint8_t* src, int sw, int sh, uint8_t* dst, int dw, int dh);
void v2_upscale_bilinear(const uint8_t* src, int sw, int sh, uint8_t* dst, int dw, int dh);
void v2_upscale_process_frame(const uint8_t* src, int sw, int sh, uint8_t* dst);
void v2_upscale_set_scale(int factor);

#ifdef __cplusplus
}
#endif

#endif
