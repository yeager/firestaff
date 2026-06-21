/*
 * firestaff_v1_dm_v21_upscale_renderer_silicon_probe.c
 *
 * Apple-Silicon-specific DM1 V2.1 upscale renderer probe.
 *
 * This is the V2.1 companion to the existing V2.2 Metal readback gate.
 * It drives the V2.1 pipeline end to end:
 *   V1 320x200 indexed framebuffer -> EPX 2x -> RGBA 640x400
 *   -> M11_Render_PresentRGBA -> SDL_RenderReadPixels.
 *
 * Skip behavior on non-Apple-Silicon hosts:
 *   - Exit 0 with a logged skip reason. The probe stays buildable on
 *     Linux, Windows, and Intel macOS runners.
 *
 * Source-lock: ReDMCSB DUNVIEW.C:8318-8542 owns the V1 draw composition
 * order that produces the indexed source framebuffer; V2.1 only upscales
 * that presentation buffer with EPX and palette expansion.
 */

#include "dm1_v2_presentation_mode_pc34.h"
#include "dm1_v2_viewport_renderer_pc34.h"
#include "render_sdl_m11.h"

#include <SDL3/SDL.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct ProbeStats {
    int total;
    int passed;
    int failed;
} ProbeStats;

static void probe_record(ProbeStats* stats,
                         const char* id,
                         int ok,
                         const char* note) {
    stats->total += 1;
    if (ok) {
        stats->passed += 1;
        printf("PASS %s: %s\n", id, note);
    } else {
        stats->failed += 1;
        printf("FAIL %s: %s\n", id, note);
    }
}

static int is_apple_silicon(void) {
#if defined(__APPLE__) && (defined(__arm64__) || defined(__aarch64__))
    return 1;
#elif defined(__APPLE__)
    FILE* f = popen("sysctl -n hw.optional.arm64 2>/dev/null", "r");
    char buf[16];
    size_t n;
    if (!f) return 0;
    n = fread(buf, 1, sizeof(buf) - 1u, f);
    pclose(f);
    if (n == 0u) return 0;
    buf[n] = '\0';
    return buf[0] == '1';
#else
    return 0;
#endif
}

static void fill_v21_source_pattern(uint8_t* fb) {
    int x;
    int y;
    if (!fb) return;
    memset(fb, 0x00, 320u * 200u);

    /* Sparse, deterministic V1-indexed pattern:
     * - center white pixel verifies 2x EPX expansion and GPU readback
     * - vertical/horizontal colored lines verify broader non-zero output
     * - dim diagonal verifies high-nibble palette-level preservation */
    fb[100 * 320 + 160] = M11_FB_ENCODE(15, 0);
    for (y = 40; y <= 160; ++y) {
        fb[y * 320 + 80] = M11_FB_ENCODE(9, 0);
    }
    for (x = 40; x <= 220; ++x) {
        fb[70 * 320 + x] = M11_FB_ENCODE(10, 0);
    }
    for (x = 0; x < 120; ++x) {
        y = 30 + (x / 2);
        fb[y * 320 + (180 + x)] = M11_FB_ENCODE(12, 3);
    }
}

static size_t count_nonblack_rgba(const uint32_t* rgba, int w, int h) {
    size_t count = 0u;
    int i;
    if (!rgba || w <= 0 || h <= 0) return 0u;
    for (i = 0; i < w * h; ++i) {
        if ((rgba[i] & 0x00FFFFFFu) != 0u) {
            count++;
        }
    }
    return count;
}

static int center_2x_block_is_nonblack_and_equal(const uint32_t* rgba, int w) {
    uint32_t p00;
    uint32_t p10;
    uint32_t p01;
    uint32_t p11;
    if (!rgba || w < 322) return 0;
    p00 = rgba[200 * w + 320];
    p10 = rgba[200 * w + 321];
    p01 = rgba[201 * w + 320];
    p11 = rgba[201 * w + 321];
    return ((p00 & 0x00FFFFFFu) != 0u) &&
           p00 == p10 && p00 == p01 && p00 == p11;
}

int main(void) {
    ProbeStats stats;
    uint8_t source_before[320 * 200];
    uint32_t* first_rgba;
    uint8_t* src;
    const uint32_t* rgba;
    int out_w = 0;
    int out_h = 0;
    int rc;

    memset(&stats, 0, sizeof(stats));

    if (!is_apple_silicon()) {
        printf("skip: not Apple Silicon (host is x86_64 / non-macOS); probe target is __APPLE__ + __arm64__\n");
        printf("# summary: 0/0 invariants checked (skipped on non-Apple-Silicon host)\n");
        return 0;
    }
    printf("info: Apple Silicon host detected\n");

#if defined(_WIN32)
    _putenv_s("SDL_VIDEODRIVER", "dummy");
#else
    setenv("SDL_VIDEODRIVER", "dummy", 1);
#endif

    dm1_v2_presentation_mode_reset();
    dm1_v2_presentation_mode_set(DM1_V2_PM_V21_UPSCALED);
    probe_record(&stats, "AS_V21_PRESENTATION_MODE",
                 dm1_v2_presentation_mode_is_v21() == 1,
                 "DM1 V2 presentation mode resolves to V2.1 upscale");

    v21_viewport_init(2);
    src = v21_viewport_get_v1_framebuffer_mut();
    if (!src) {
        fprintf(stderr, "FAIL v21_viewport_get_v1_framebuffer_mut returned NULL\n");
        return 1;
    }
    fill_v21_source_pattern(src);
    memcpy(source_before, src, sizeof(source_before));

    v21_viewport_render_full_pipeline();
    rgba = v21_viewport_get_rgba(&out_w, &out_h);
    probe_record(&stats, "AS_V21_OUTPUT_DIMENSIONS",
                 rgba != NULL && out_w == 640 && out_h == 400,
                 "V2.1 pipeline reports 640x400 RGBA output for 2x EPX");
    probe_record(&stats, "AS_V21_SOURCE_UNCHANGED",
                 memcmp(source_before, src, sizeof(source_before)) == 0,
                 "EPX render leaves the V1 indexed source framebuffer unchanged");
    probe_record(&stats, "AS_V21_CPU_EPX_CENTER_2X",
                 center_2x_block_is_nonblack_and_equal(rgba, out_w),
                 "single V1 center pixel expands to one non-black 2x2 RGBA block");

    {
        size_t nonblack = count_nonblack_rgba(rgba, out_w, out_h);
        char note[160];
        snprintf(note, sizeof(note),
                 "CPU RGBA non-black pixels=%zu after sparse V1 source pattern",
                 nonblack);
        probe_record(&stats, "AS_V21_CPU_NONBLACK_COVERAGE",
                     nonblack > 1000u,
                     note);
    }

    first_rgba = (uint32_t*)malloc((size_t)out_w * (size_t)out_h * sizeof(uint32_t));
    if (!first_rgba) {
        fprintf(stderr, "FAIL malloc first_rgba\n");
        return 1;
    }
    memcpy(first_rgba, rgba, (size_t)out_w * (size_t)out_h * sizeof(uint32_t));
    v21_viewport_render_full_pipeline();
    rgba = v21_viewport_get_rgba(&out_w, &out_h);
    probe_record(&stats, "AS_V21_CPU_DETERMINISTIC",
                 memcmp(first_rgba, rgba, (size_t)out_w * (size_t)out_h * sizeof(uint32_t)) == 0,
                 "same source framebuffer renders byte-identical RGBA twice");
    free(first_rgba);

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "FAIL SDL_Init: %s\n", SDL_GetError());
        return 1;
    }

    rc = M11_Render_Init(640, 400, M11_SCALE_1X);
    if (rc != M11_RENDER_OK) {
        fprintf(stderr, "FAIL M11_Render_Init: rc=%d\n", rc);
        SDL_Quit();
        return 1;
    }
    (void)M11_Render_SetDisplayAspectMode(M11_DISPLAY_ASPECT_CONTENT);
    (void)M11_Render_SetIntegerScaling(0);

    rc = M11_Render_PresentRGBA((const unsigned char*)rgba, out_w, out_h);
    if (rc != M11_RENDER_OK) {
        fprintf(stderr, "FAIL M11_Render_PresentRGBA V21: rc=%d\n", rc);
        stats.failed++;
        stats.total++;
    } else {
        SDL_Surface* surf = SDL_RenderReadPixels(M11_Render_GetRenderer(), NULL);
        if (!surf || !surf->pixels) {
            fprintf(stderr, "FAIL SDL_RenderReadPixels V21: %s\n",
                    surf ? "no pixels" : SDL_GetError());
            if (surf) SDL_DestroySurface(surf);
            stats.failed++;
            stats.total++;
        } else {
            Uint8 center_r = 0;
            Uint8 center_g = 0;
            Uint8 center_b = 0;
            Uint8 center_a = 0;
            Uint8 corner_r = 1;
            Uint8 corner_g = 1;
            Uint8 corner_b = 1;
            Uint8 corner_a = 0;
            char note[220];

            SDL_ReadSurfacePixel(surf, 320, 200,
                                 &center_r, &center_g, &center_b, &center_a);
            SDL_ReadSurfacePixel(surf, 2, 2,
                                 &corner_r, &corner_g, &corner_b, &corner_a);
            SDL_DestroySurface(surf);
            snprintf(note, sizeof(note),
                     "GPU center RGB=(%u,%u,%u), corner RGB=(%u,%u,%u)",
                     (unsigned)center_r, (unsigned)center_g, (unsigned)center_b,
                     (unsigned)corner_r, (unsigned)corner_g, (unsigned)corner_b);
            probe_record(&stats, "AS_V21_GPU_READBACK",
                         ((unsigned)center_r + (unsigned)center_g + (unsigned)center_b) > 0u &&
                         corner_r == 0 && corner_g == 0 && corner_b == 0,
                         note);
        }
    }

    dm1_v2_presentation_mode_reset();
    M11_Render_Shutdown();
    SDL_Quit();

    printf("# summary: %d/%d invariants passed (%d failed)\n",
           stats.passed, stats.total, stats.failed);
    return (stats.failed == 0) ? 0 : 1;
}
