/*
 * theron_v2_texture_upscale_pc34.c
 *
 * Theron V2.1 texture upscale pipeline. Mirror of
 * src/csb/csb_v2_texture_upscale_pc34.c with theron_ prefix.
 * See include/theron_v2_texture_upscale_pc34.h for source-lock
 * references and the Theron-vs-DM1/CSB difference list.
 */

#include "theron_v2_texture_upscale_pc34.h"

#include <string.h>

static Theron_V2_TextureUpscaleConfig theron_v2_upscale_global_config;

void theron_v2_upscale_init(Theron_V2_TextureUpscaleConfig* config) {
    if (config) {
        theron_v2_upscale_global_config = *config;
    } else {
        theron_v2_upscale_global_config.scale_factor = 2;
        theron_v2_upscale_global_config.use_bilinear = false;
        theron_v2_upscale_global_config.sharpen = false;
    }
}

void theron_v2_upscale_nearest(const uint8_t* src, int sw, int sh, uint8_t* dst, int dw, int dh) {
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

void theron_v2_upscale_bilinear(const uint8_t* src, int sw, int sh, uint8_t* dst, int dw, int dh) {
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

void theron_v2_upscale_process_frame(const uint8_t* src, int sw, int sh, uint8_t* dst) {
    if (!src || !dst) return;
    int dw = sw * theron_v2_upscale_global_config.scale_factor;
    int dh = sh * theron_v2_upscale_global_config.scale_factor;
    if (theron_v2_upscale_global_config.use_bilinear) {
        theron_v2_upscale_bilinear(src, sw, sh, dst, dw, dh);
    } else {
        theron_v2_upscale_nearest(src, sw, sh, dst, dw, dh);
    }
}

void theron_v2_upscale_set_scale(int factor) {
    if (factor == 1 || factor == 2 || factor == 4) {
        theron_v2_upscale_global_config.scale_factor = factor;
    }
}

void theron_v2_upscale_epx(const uint8_t *src, int sw, int sh,
    uint8_t *dst, int dw, int dh)
{
    int x, y;
    (void)dw; (void)dh;
    if (!src || !dst || sw <= 0 || sh <= 0) return;

    for (y = 0; y < sh; y++) {
        for (x = 0; x < sw; x++) {
            uint8_t P = src[y * sw + x];
            uint8_t A = (y > 0)      ? src[(y-1) * sw + x] : P;
            uint8_t B = (x < sw - 1) ? src[y * sw + (x+1)] : P;
            uint8_t C = (x > 0)      ? src[y * sw + (x-1)] : P;
            uint8_t D = (y < sh - 1) ? src[(y+1) * sw + x] : P;

            int ox = x * 2, oy = y * 2;
            int ow = sw * 2;

            dst[oy * ow + ox]         = (C == A && C != D && A != B) ? A : P;
            dst[oy * ow + ox + 1]     = (A == B && A != C && B != D) ? B : P;
            dst[(oy+1) * ow + ox]     = (D == C && D != B && C != A) ? C : P;
            dst[(oy+1) * ow + ox + 1] = (B == D && B != A && D != C) ? D : P;
        }
    }
}

void theron_v2_upscale_palette_to_rgba(const uint8_t *indexed, int w, int h,
    const uint32_t *palette, int palette_size, uint32_t *rgba_out)
{
    int i, total;
    if (!indexed || !palette || !rgba_out) return;
    total = w * h;
    for (i = 0; i < total; i++) {
        int idx = indexed[i];
        rgba_out[i] = (idx < palette_size) ? palette[idx] : 0xFF000000;
    }
}

void theron_v2_upscale_full_pipeline(const uint8_t *v1_indexed, int v1_w, int v1_h,
    const uint32_t *palette, int palette_size,
    uint8_t *epx_buffer, uint32_t *rgba_out, int target_scale)
{
    int epx_w, epx_h;
    if (!v1_indexed || !palette || !epx_buffer || !rgba_out) return;
    if (target_scale < 1) target_scale = 1;

    epx_w = v1_w * 2;
    epx_h = v1_h * 2;
    theron_v2_upscale_epx(v1_indexed, v1_w, v1_h, epx_buffer, epx_w, epx_h);
    theron_v2_upscale_palette_to_rgba(epx_buffer, epx_w, epx_h, palette, palette_size, rgba_out);
    (void)target_scale;
}

void theron_v2_upscale_ntsc_fullscreen(const uint8_t* v1_indexed,
    const uint32_t* palette, int palette_size,
    uint8_t* epx_buffer, uint32_t* rgba_out)
{
    /* PC Engine CD NTSC native 256x224. The 4x3 letterboxed viewport
     * is the gameplay view; the full 256x224 is the entire screen. */
    if (theron_v2_upscale_global_config.scale_factor == 1) {
        theron_v2_upscale_palette_to_rgba(v1_indexed, 256, 224, palette, palette_size, rgba_out);
        return;
    }
    theron_v2_upscale_full_pipeline(v1_indexed, 256, 224, palette, palette_size,
        epx_buffer, rgba_out, theron_v2_upscale_global_config.scale_factor);
}

void theron_v2_upscale_dungeon_viewport(const uint8_t* v1_indexed, int w, int h,
    const uint32_t* palette, int palette_size,
    uint8_t* epx_buffer, uint32_t* rgba_out)
{
    /* 192x160 letterboxed dungeon viewport (the actual gameplay
     * view at x=32, y=24 within the 256x224 screen). */
    if (theron_v2_upscale_global_config.scale_factor == 1) {
        theron_v2_upscale_palette_to_rgba(v1_indexed, w, h, palette, palette_size, rgba_out);
        return;
    }
    theron_v2_upscale_full_pipeline(v1_indexed, w, h, palette, palette_size,
        epx_buffer, rgba_out, theron_v2_upscale_global_config.scale_factor);
}

const char* theron_v2_upscale_v21_source_evidence(void) {
    return
        "Theron V2.1 upscale pipeline: V1 256x224 (NTSC) -> EPX 2x -> palette RGBA\n"
        "EPX/Scale2x: edge-preserving doubler for indexed pixel art (http://www.scale2x.it/)\n"
        "Palette-aware: maps through Theron's V1 16-color palette (4bpp HuC6270 VCE)\n"
        "Theron full screen: 256x224 NTSC\n"
        "Theron dungeon viewport: 192x160 (4x3 letterbox, 24 tiles wide x 20 tiles tall)\n"
        "Theron right panel: 96x160 (stats + compass)\n"
        "Theron bottom panel: 320x56 (party panel, 4 slots 80px each)\n"
        "Mirror of dm1_v2_texture_upscale_pc34.c + csb_v2_texture_upscale_pc34.c\n"
        "Source: THQUEST.ASM T400/T520/T600; HuC6260/HuC6270 VDC/VCE; "
        "tqr_v1_phase2_data_formats_H2339.md §7\n";
}
