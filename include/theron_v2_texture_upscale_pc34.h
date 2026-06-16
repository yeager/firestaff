#ifndef FIRESTAFF_THERON_V2_TEXTURE_UPSCALE_PC34_H
#define FIRESTAFF_THERON_V2_TEXTURE_UPSCALE_PC34_H

/*
 * theron_v2_texture_upscale_pc34.h
 *
 * Theron V2.1 texture upscale pipeline. Parallel to
 * dm1_v2_texture_upscale_pc34.h + csb_v2_texture_upscale_pc34.h.
 *
 * Theron's Quest is the PC Engine CD port (HuC6280 + HuC6260 VDC +
 * HuC6270 VCE), so the V1 base is 256x224 (NTSC native) with 8x8
 * tiles. The V1 indexed pixel art is 16-color (4bpp) in V1, but
 * Theron's V2.1 can fan out to 32-color or 64-color as needed
 * for the modernised HUD.
 *
 * The V2.1 EPX/bilinear/nearest stages are the same generic
 * EPX/Scale2x algorithm used by DM1 and CSB (the only difference
 * is the V1 base resolution and the planar framebuffer layout).
 *
 * Pipeline: V1 indexed 256x224 -> EPX 2x -> optional bilinear
 * to target resolution -> optional sharpening.
 *
 * Source-lock references:
 *   - THQUEST.ASM T400  - tile bank loading (HuCard ROM mapping)
 *   - THQUEST.ASM T520  - tile selection + viewport blit
 *   - THQUEST.ASM T600  - UI overlay zones
 *   - HuC6260/HuC6270 datasheet - VDC/VCE rendering
 *   - tqr_v1_phase2_data_formats_H2339.md §7 - tile data format
 *   - include/dm1_v2_texture_upscale_pc34.h
 *   - include/csb_v2_texture_upscale_pc34.h
 *   - http://www.scale2x.it/ (EPX/Scale2x algorithm)
 *
 * Module lives in src/theron/theron_v2_texture_upscale_pc34.c
 * Test:   tests/test_theron_v2_texture_upscale_pc34.c
 * Probe:  probes/firestaff_theron_v2_texture_upscale_probe.c
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int scale_factor;
    bool use_bilinear;
    bool sharpen;
} Theron_V2_TextureUpscaleConfig;

void theron_v2_upscale_init(Theron_V2_TextureUpscaleConfig* config);
void theron_v2_upscale_nearest(const uint8_t* src, int sw, int sh, uint8_t* dst, int dw, int dh);
void theron_v2_upscale_bilinear(const uint8_t* src, int sw, int sh, uint8_t* dst, int dw, int dh);
void theron_v2_upscale_process_frame(const uint8_t* src, int sw, int sh, uint8_t* dst);
void theron_v2_upscale_set_scale(int factor);

/* EPX/Scale2x — edge-preserving 2x doubler. */
void theron_v2_upscale_epx(const uint8_t* src, int sw, int sh,
    uint8_t* dst, int dw, int dh);

void theron_v2_upscale_palette_to_rgba(const uint8_t* indexed, int w, int h,
    const uint32_t* palette, int palette_size, uint32_t* rgba_out);

void theron_v2_upscale_full_pipeline(const uint8_t* v1_indexed, int v1_w, int v1_h,
    const uint32_t* palette, int palette_size,
    uint8_t* epx_buffer, uint32_t* rgba_out, int target_scale);

/* Theron V2.1 specific: 256x224 (NTSC native) full-screen upscale. */
void theron_v2_upscale_ntsc_fullscreen(const uint8_t* v1_indexed,
    const uint32_t* palette, int palette_size,
    uint8_t* epx_buffer, uint32_t* rgba_out);

/* Theron V2.1 specific: 192x160 letterboxed dungeon viewport upscale
 * (the actual gameplay view; the 32x24 letterbox on each side is
 * left to the V1 path). */
void theron_v2_upscale_dungeon_viewport(const uint8_t* v1_indexed, int w, int h,
    const uint32_t* palette, int palette_size,
    uint8_t* epx_buffer, uint32_t* rgba_out);

const char* theron_v2_upscale_v21_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_THERON_V2_TEXTURE_UPSCALE_PC34_H */
