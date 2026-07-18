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

/* EPX (Eric's Pixel Expansion) / Scale2x — edge-preserving 2x doubler.
 * Input:  sw×sh indexed pixels. Output: dw=sw*2, dh=sh*2 indexed pixels.
 * Preserves sharp pixel-art edges without palette interpolation.
 * Reference: http://www.scale2x.it/ */
void v2_upscale_epx(const uint8_t *src, int sw, int sh,
    uint8_t *dst, int dw, int dh);

#ifdef __cplusplus
}
#endif

void v2_upscale_palette_to_rgba(const uint8_t *indexed, int w, int h, const uint32_t *palette, int palette_size, uint32_t *rgba_out);

void v2_upscale_full_pipeline(const uint8_t *v1_indexed, int v1_w, int v1_h, const uint32_t *palette, int palette_size, uint8_t *epx_buffer, uint32_t *rgba_out, int target_scale);

const char *v2_upscale_v21_source_evidence(void);

#endif
