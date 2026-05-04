#include "dm1_v2_texture_upscale_pc34.h"

static M11_V2_TextureUpscaleConfig v2_upscale_global_config;

void v2_upscale_init(M11_V2_TextureUpscaleConfig* config) {
    if (config) {
        v2_upscale_global_config = *config;
    } else {
        v2_upscale_global_config.scale_factor = 2;
        v2_upscale_global_config.use_bilinear = false;
        v2_upscale_global_config.sharpen = false;
    }
}

void v2_upscale_nearest(const uint8_t* src, int sw, int sh, uint8_t* dst, int dw, int dh) {
    if (!src || !dst || sw <= 0 || sh <= 0 || dw <= 0 || dh <= 0) return;
    for (int y = 0; y < dh; y++) {
        for (int x = 0; x < dw; x++) {
            int sx = (x * sw) / dw;
            int sy = (y * sh) / dh;
            if (sx >= sw) sx = sw - 1;
            if (sy >= sh) sy = sh - 1;
            dst[y * dw + x] = src[sy * sw + sx];
        }
    }
}

void v2_upscale_bilinear(const uint8_t* src, int sw, int sh, uint8_t* dst, int dw, int dh) {
    if (!src || !dst || sw <= 0 || sh <= 0 || dw <= 0 || dh <= 0) return;
    float fx = (float)sw / dw;
    float fy = (float)sh / dh;

    for (int y = 0; y < dh; y++) {
        for (int x = 0; x < dw; x++) {
            float sx = x * fx;
            float sy = y * fy;
            int x0 = (int)sx;
            int y0 = (int)sy;
            int x1 = x0 + 1;
            int y1 = y0 + 1;
            if (x1 >= sw) x1 = sw - 1;
            if (y1 >= sh) y1 = sh - 1;

            float dx = sx - x0;
            float dy = sy - y0;

            uint8_t tl = src[y0 * sw + x0];
            uint8_t tr = src[y0 * sw + x1];
            uint8_t bl = src[y1 * sw + x0];
            uint8_t br = src[y1 * sw + x1];

            float top = tl + (tr - tl) * dx;
            float bot = bl + (br - bl) * dx;
            float val = top + (bot - top) * dy;

            dst[y * dw + x] = (uint8_t)(val < 0.0f ? 0.0f : (val > 255.0f ? 255.0f : val));
        }
    }
}

void v2_upscale_process_frame(const uint8_t* src, int sw, int sh, uint8_t* dst) {
    if (!src || !dst) return;
    int dw = sw * v2_upscale_global_config.scale_factor;
    int dh = sh * v2_upscale_global_config.scale_factor;
    if (v2_upscale_global_config.use_bilinear) {
        v2_upscale_bilinear(src, sw, sh, dst, dw, dh);
    } else {
        v2_upscale_nearest(src, sw, sh, dst, dw, dh);
    }
}

void v2_upscale_set_scale(int factor) {
    if (factor == 1 || factor == 2 || factor == 4) {
        v2_upscale_global_config.scale_factor = factor;
    }
}
