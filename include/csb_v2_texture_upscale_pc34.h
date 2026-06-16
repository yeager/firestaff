#ifndef FIRESTAFF_CSB_V2_TEXTURE_UPSCALE_PC34_H
#define FIRESTAFF_CSB_V2_TEXTURE_UPSCALE_PC34_H

/*
 * csb_v2_texture_upscale_pc34.h
 *
 * CSB V2.1 texture upscale pipeline. Parallel to
 * dm1_v2_texture_upscale_pc34.h; the EPX/bilinear/nearest
 * implementations are intentionally mirrored because both
 * games share the same indexed-pixel-art problem (CSBWin/
 * Viewport.cpp:7290 + DM1's 320x200 VGA base).
 *
 * CSB-specific differences from DM1:
 *   - 9-square (3x3) viewport layout instead of DM1's 4x3
 *   - CSB has separate viewport (224x136) and panel (320x64)
 *     regions that are upscaled independently
 *   - Modern pack at ~/.firestaff/assets/csb/modern/ holds
 *     the 1920x1080 native art for V2.2
 *
 * Pipeline: V1 indexed 320x200 -> EPX 2x -> optional bilinear
 * to target resolution -> optional sharpening.
 *
 * Source-lock references:
 *   - CSBWin/Viewport.cpp:7290  CSB 9-square viewport blit
 *   - CSBWin/Graphics.cpp:3186 filter pair
 *   - ReDMCSB DUNVIEW.C F0128 V1 320x200 base (same anchor
 *     as DM1; CSB's first-person view uses the same DM1
 *     320x200 indexed source)
 *   - dm1_v2_texture_upscale_pc34.c (mirror reference)
 *   - http://www.scale2x.it/ (EPX/Scale2x algorithm)
 *
 * Module lives in src/csb/csb_v2_texture_upscale_pc34.c
 * Test:   tests/test_csb_v2_texture_upscale_pc34.c
 * Probe:  probes/firestaff_csb_v2_texture_upscale_probe.c
 */

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
} CSB_V2_TextureUpscaleConfig;

void csb_v2_upscale_init(CSB_V2_TextureUpscaleConfig* config);
void csb_v2_upscale_nearest(const uint8_t* src, int sw, int sh, uint8_t* dst, int dw, int dh);
void csb_v2_upscale_bilinear(const uint8_t* src, int sw, int sh, uint8_t* dst, int dw, int dh);
void csb_v2_upscale_process_frame(const uint8_t* src, int sw, int sh, uint8_t* dst);
void csb_v2_upscale_set_scale(int factor);

/* Read the active scale factor (1, 2, 4). Used by tests/probes to
 * verify the per-game V2 settings wire-up from M12 menu config. */
int  csb_v2_upscale_get_scale(void);

/* Read the active bilinear flag. */
int  csb_v2_upscale_get_bilinear(void);

/* EPX/Scale2x — edge-preserving 2x doubler (CSB mirror of DM1).
 * Reference: http://www.scale2x.it/ */
void csb_v2_upscale_epx(const uint8_t* src, int sw, int sh,
    uint8_t* dst, int dw, int dh);

/* Full CSB V2.1 pipeline: V1 indexed -> EPX 2x -> palette -> RGBA.
 * Mirrors dm1_v2_upscale_full_pipeline so the two games share the
 * same pixel-art upgrade story. */
void csb_v2_upscale_palette_to_rgba(const uint8_t* indexed, int w, int h,
    const uint32_t* palette, int palette_size, uint32_t* rgba_out);

void csb_v2_upscale_full_pipeline(const uint8_t* v1_indexed, int v1_w, int v1_h,
    const uint32_t* palette, int palette_size,
    uint8_t* epx_buffer, uint32_t* rgba_out, int target_scale);

/* CSB V2.1 specific: upscale the 9-square viewport (224x136 -> 448x272
 * by default). csb_v2_upscale_set_scale(2) sets the EPX step. */
void csb_v2_upscale_9square_viewport(const uint8_t* v1_indexed, int w, int h,
    const uint32_t* palette, int palette_size,
    uint8_t* epx_buffer, uint32_t* rgba_out);

/* CSB V2.1 specific: upscale the panel region (320x64 -> 640x128). */
void csb_v2_upscale_panel(const uint8_t* v1_indexed, int w, int h,
    const uint32_t* palette, int palette_size,
    uint8_t* epx_buffer, uint32_t* rgba_out);

/* Source evidence string for tests/probes. */
const char* csb_v2_upscale_v21_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V2_TEXTURE_UPSCALE_PC34_H */
