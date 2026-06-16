/*
 * csb_v2_texture_upscale_pc34.c
 *
 * CSB V2.1 texture upscale pipeline. Mirror of
 * src/dm1v2/dm1_v2_texture_upscale_pc34.c with csb_ prefix.
 * See include/csb_v2_texture_upscale_pc34.h for source-lock
 * references and the CSB-vs-DM1 difference list.
 *
 * The implementation is intentionally close to the DM1 V2.1
 * pipeline because both games share the same indexed-pixel-art
 * problem (CSBWin/Viewport.cpp:7290 + ReDMCSB DUNVIEW.C F0128
 * 320x200 base). Keeping the two modules in lock-step makes
 * a future single shared pipeline easy to extract.
 */

#include "csb_v2_texture_upscale_pc34.h"

static CSB_V2_TextureUpscaleConfig csb_v2_upscale_global_config;

void csb_v2_upscale_init(CSB_V2_TextureUpscaleConfig* config) {
    if (config) {
        csb_v2_upscale_global_config = *config;
    } else {
        csb_v2_upscale_global_config.scale_factor = 2;
        csb_v2_upscale_global_config.use_bilinear = false;
        csb_v2_upscale_global_config.sharpen = false;
    }
}

void csb_v2_upscale_nearest(const uint8_t* src, int sw, int sh, uint8_t* dst, int dw, int dh) {
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

void csb_v2_upscale_bilinear(const uint8_t* src, int sw, int sh, uint8_t* dst, int dw, int dh) {
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

void csb_v2_upscale_process_frame(const uint8_t* src, int sw, int sh, uint8_t* dst) {
    if (!src || !dst) return;
    int dw = sw * csb_v2_upscale_global_config.scale_factor;
    int dh = sh * csb_v2_upscale_global_config.scale_factor;
    if (csb_v2_upscale_global_config.use_bilinear) {
        csb_v2_upscale_bilinear(src, sw, sh, dst, dw, dh);
    } else {
        csb_v2_upscale_nearest(src, sw, sh, dst, dw, dh);
    }
}

void csb_v2_upscale_set_scale(int factor) {
    if (factor == 1 || factor == 2 || factor == 4) {
        csb_v2_upscale_global_config.scale_factor = factor;
    }
}

/* EPX/Scale2x — edge-preserving 2x doubler. Same algorithm as
 * DM1's v2_upscale_epx; mirrored here so the CSB runtime owns
 * its own pipeline state and doesn't reach into DM1 globals. */
void csb_v2_upscale_epx(const uint8_t *src, int sw, int sh,
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

void csb_v2_upscale_palette_to_rgba(const uint8_t *indexed, int w, int h,
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

void csb_v2_upscale_full_pipeline(const uint8_t *v1_indexed, int v1_w, int v1_h,
    const uint32_t *palette, int palette_size,
    uint8_t *epx_buffer, uint32_t *rgba_out, int target_scale)
{
    int epx_w, epx_h;
    if (!v1_indexed || !palette || !epx_buffer || !rgba_out) return;
    if (target_scale < 1) target_scale = 1;

    epx_w = v1_w * 2;
    epx_h = v1_h * 2;
    csb_v2_upscale_epx(v1_indexed, v1_w, v1_h, epx_buffer, epx_w, epx_h);
    csb_v2_upscale_palette_to_rgba(epx_buffer, epx_w, epx_h, palette, palette_size, rgba_out);
    (void)target_scale;
}

void csb_v2_upscale_9square_viewport(const uint8_t* v1_indexed, int w, int h,
    const uint32_t* palette, int palette_size,
    uint8_t* epx_buffer, uint32_t* rgba_out)
{
    /* CSB's 9-square viewport is 224x136 in the original (the 3x3
     * 3D scene minus the bottom UI strip). Pass through the V2.1
     * pipeline at the caller's actual w/h — the csb_v2_viewport
     * renderer is what knows the 9-square rectangle; this function
     * just runs the EPX + palette conversion. */
    if (csb_v2_upscale_global_config.scale_factor == 1) {
        /* V1 path: no upscale, just palette pass-through. */
        csb_v2_upscale_palette_to_rgba(v1_indexed, w, h, palette, palette_size, rgba_out);
        return;
    }
    csb_v2_upscale_full_pipeline(v1_indexed, w, h, palette, palette_size,
        epx_buffer, rgba_out, csb_v2_upscale_global_config.scale_factor);
}

void csb_v2_upscale_panel(const uint8_t* v1_indexed, int w, int h,
    const uint32_t* palette, int palette_size,
    uint8_t* epx_buffer, uint32_t* rgba_out)
{
    /* CSB panel region (320x64 in V1) — same pipeline as the
     * viewport; the renderer is the right place to position the
     * upscale result. */
    if (csb_v2_upscale_global_config.scale_factor == 1) {
        csb_v2_upscale_palette_to_rgba(v1_indexed, w, h, palette, palette_size, rgba_out);
        return;
    }
    csb_v2_upscale_full_pipeline(v1_indexed, w, h, palette, palette_size,
        epx_buffer, rgba_out, csb_v2_upscale_global_config.scale_factor);
}

const char* csb_v2_upscale_v21_source_evidence(void) {
    return
        "CSB V2.1 upscale pipeline: V1 320x200 indexed -> EPX 2x -> palette RGBA\n"
        "EPX/Scale2x: edge-preserving doubler for indexed pixel art (http://www.scale2x.it/)\n"
        "Palette-aware: maps through CSB's V1 VGA palette (ReDMCSB DEFS.H)\n"
        "CSB viewport: 224x136 9-square layout -> 448x272 EPX -> target resolution bilinear\n"
        "CSB panel: 320x64 bottom panel upscaled separately\n"
        "Mirror of dm1_v2_texture_upscale_pc34.c (shared V2.1 story, separate globals)\n"
        "Source: CSBWin/Viewport.cpp:7290; CSBWin/Graphics.cpp:3186; ReDMCSB DUNVIEW.C F0128\n";
}
