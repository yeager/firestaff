/*
 * firestaff_v1_dm_v20_filtered_renderer_silicon_probe.c
 *
 * Apple-Silicon-specific DM1 V2.0 filtered renderer probe.
 *
 * This is the V2.0 companion to the V2.1 and V2.2 Metal readback gates.
 * It drives the indexed V1 framebuffer through the DM1 V2.0 filter chain:
 *   V1 320x200 indexed framebuffer -> palette correction LUT
 *   -> CRT scanlines -> M11_Render_PresentIndexed -> SDL_RenderReadPixels.
 *
 * Skip behavior on non-Apple-Silicon hosts:
 *   - Exit 0 with a logged skip reason. The probe stays buildable on
 *     Linux, Windows, and Intel macOS runners.
 *
 * Source-lock: ReDMCSB DUNVIEW.C:8318-8542 owns the V1 draw composition
 * order that produces the indexed source framebuffer. The V2.0 filter
 * chain is Firestaff-only presentation work; include/dm1v2/dm1_v2_filters.h
 * documents that it has no ReDMCSB gameplay equivalent.
 */

#include "dm1_v2_presentation_mode_pc34.h"
#include "dm1v2/dm1_v2_filters.h"
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

typedef struct RgbSample {
    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;
} RgbSample;

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

static int rgb_nonblack(RgbSample s) {
    return ((unsigned)s.r + (unsigned)s.g + (unsigned)s.b) > 0u;
}

static int rgb_equal(RgbSample a, RgbSample b) {
    return a.r == b.r && a.g == b.g && a.b == b.b;
}

static int rgb_matches_lut(RgbSample sample,
                           unsigned char lut[DM1_V2_PALETTE_LEVELS][16][3],
                           int level,
                           int index) {
    return sample.r == lut[level][index][0] &&
           sample.g == lut[level][index][1] &&
           sample.b == lut[level][index][2];
}

static int read_pixel_pair(RgbSample* even_sample, RgbSample* odd_sample) {
    SDL_Surface* surf = SDL_RenderReadPixels(M11_Render_GetRenderer(), NULL);
    if (!surf || !surf->pixels) {
        fprintf(stderr, "FAIL SDL_RenderReadPixels V20: %s\n",
                surf ? "no pixels" : SDL_GetError());
        if (surf) SDL_DestroySurface(surf);
        return 0;
    }
    SDL_ReadSurfacePixel(surf, 160, 100,
                         &even_sample->r, &even_sample->g,
                         &even_sample->b, &even_sample->a);
    SDL_ReadSurfacePixel(surf, 160, 101,
                         &odd_sample->r, &odd_sample->g,
                         &odd_sample->b, &odd_sample->a);
    SDL_DestroySurface(surf);
    return 1;
}

static void fill_v20_source_pattern(uint8_t* fb) {
    int x;
    int y;
    if (!fb) return;
    memset(fb, 0x00, M11_FB_BYTES);

    for (y = 96; y <= 104; ++y) {
        for (x = 152; x <= 168; ++x) {
            fb[y * M11_FB_WIDTH + x] = M11_FB_ENCODE(9, 0);
        }
    }
    for (x = 32; x < 288; ++x) {
        fb[140 * M11_FB_WIDTH + x] = M11_FB_ENCODE(12, 0);
    }
}

int main(void) {
    ProbeStats stats;
    uint8_t framebuffer[M11_FB_BYTES];
    uint8_t source_before[M11_FB_BYTES];
    unsigned char palette_lut[DM1_V2_PALETTE_LEVELS][16][3];
    RgbSample baseline_even;
    RgbSample baseline_odd;
    RgbSample filtered_even;
    RgbSample filtered_odd;
    RgbSample filtered_even_repeat;
    RgbSample filtered_odd_repeat;
    int rc;
    int crt_enabled = -1;
    int crt_strength = -1;
    int palette_enabled = -1;
    int palette_gamma100 = -1;
    int palette_brightness = -1;
    int palette_contrast = -1;
    int palette_interp_enabled = -1;
    int palette_interp_strength = -1;
    int dither_enabled = -1;
    int sharpen_enabled = -1;
    int sharpen_strength = -1;

    memset(&stats, 0, sizeof(stats));
    memset(&baseline_even, 0, sizeof(baseline_even));
    memset(&baseline_odd, 0, sizeof(baseline_odd));
    memset(&filtered_even, 0, sizeof(filtered_even));
    memset(&filtered_odd, 0, sizeof(filtered_odd));
    memset(&filtered_even_repeat, 0, sizeof(filtered_even_repeat));
    memset(&filtered_odd_repeat, 0, sizeof(filtered_odd_repeat));

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
    dm1_v2_presentation_mode_set(DM1_V2_PM_V20_FILTERED);
    probe_record(&stats, "AS_V20_PRESENTATION_MODE",
                 dm1_v2_presentation_mode_is_v20() == 1,
                 "DM1 V2 presentation mode resolves to V2.0 filtered");

    fill_v20_source_pattern(framebuffer);
    memcpy(source_before, framebuffer, sizeof(source_before));

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "FAIL SDL_Init: %s\n", SDL_GetError());
        return 1;
    }

    rc = M11_Render_Init(320, 200, M11_SCALE_1X);
    if (rc != M11_RENDER_OK) {
        fprintf(stderr, "FAIL M11_Render_Init: rc=%d\n", rc);
        SDL_Quit();
        return 1;
    }
    (void)M11_Render_SetDisplayAspectMode(M11_DISPLAY_ASPECT_CONTENT);
    (void)M11_Render_SetIntegerScaling(0);

    (void)M11_Render_SetV2Filters(0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 0);
    rc = M11_Render_PresentIndexed(framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT);
    if (rc != M11_RENDER_OK || !read_pixel_pair(&baseline_even, &baseline_odd)) {
        fprintf(stderr, "FAIL baseline V20 readback: rc=%d\n", rc);
        M11_Render_Shutdown();
        SDL_Quit();
        return 1;
    }
    {
        char note[180];
        snprintf(note, sizeof(note),
                 "baseline even RGB=(%u,%u,%u), odd RGB=(%u,%u,%u)",
                 (unsigned)baseline_even.r, (unsigned)baseline_even.g, (unsigned)baseline_even.b,
                 (unsigned)baseline_odd.r, (unsigned)baseline_odd.g, (unsigned)baseline_odd.b);
        probe_record(&stats, "AS_V20_BASELINE_UNFILTERED",
                     rgb_nonblack(baseline_even) &&
                     rgb_nonblack(baseline_odd) &&
                     rgb_equal(baseline_even, baseline_odd),
                     note);
    }

    if (dm1_v2_filter_palette_build_lut(100, 20, 0, palette_lut) != 0) {
        fprintf(stderr, "FAIL dm1_v2_filter_palette_build_lut\n");
        M11_Render_Shutdown();
        SDL_Quit();
        return 1;
    }

    (void)M11_Render_SetV2Filters(
        1, 100,   /* CRT scanlines on, fully darken even rows */
        1, 100, 20, 0,  /* palette correction LUT: gamma 1.00, +20 brightness */
        0, 0,    /* palette interpolation off for exact LUT comparison */
        0,       /* dither cleanup off; source framebuffer must remain unchanged */
        0, 0);   /* sharpen off */
    (void)M11_Render_GetV2Filters(
        &crt_enabled, &crt_strength,
        &palette_enabled, &palette_gamma100,
        &palette_brightness, &palette_contrast,
        &palette_interp_enabled, &palette_interp_strength,
        &dither_enabled, &sharpen_enabled, &sharpen_strength);
    probe_record(&stats, "AS_V20_FILTER_CONFIG",
                 crt_enabled == 1 && crt_strength == 100 &&
                 palette_enabled == 1 && palette_gamma100 == 100 &&
                 palette_brightness == 20 && palette_contrast == 0 &&
                 palette_interp_enabled == 0 && palette_interp_strength == 0 &&
                 dither_enabled == 0 && sharpen_enabled == 0 &&
                 sharpen_strength == 0,
                 "renderer accepted V2.0 CRT + palette correction config");

    rc = M11_Render_PresentIndexed(framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT);
    if (rc != M11_RENDER_OK || !read_pixel_pair(&filtered_even, &filtered_odd)) {
        fprintf(stderr, "FAIL filtered V20 readback: rc=%d\n", rc);
        M11_Render_Shutdown();
        SDL_Quit();
        return 1;
    }
    rc = M11_Render_PresentIndexed(framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT);
    if (rc != M11_RENDER_OK || !read_pixel_pair(&filtered_even_repeat, &filtered_odd_repeat)) {
        fprintf(stderr, "FAIL filtered V20 repeat readback: rc=%d\n", rc);
        M11_Render_Shutdown();
        SDL_Quit();
        return 1;
    }

    {
        char note[220];
        snprintf(note, sizeof(note),
                 "filtered even RGB=(%u,%u,%u), odd RGB=(%u,%u,%u), expected odd LUT=(%u,%u,%u)",
                 (unsigned)filtered_even.r, (unsigned)filtered_even.g, (unsigned)filtered_even.b,
                 (unsigned)filtered_odd.r, (unsigned)filtered_odd.g, (unsigned)filtered_odd.b,
                 (unsigned)palette_lut[0][9][0],
                 (unsigned)palette_lut[0][9][1],
                 (unsigned)palette_lut[0][9][2]);
        probe_record(&stats, "AS_V20_GPU_CRT_SCANLINE",
                     filtered_even.r == 0 && filtered_even.g == 0 && filtered_even.b == 0 &&
                     rgb_nonblack(filtered_odd),
                     note);
    }

    probe_record(&stats, "AS_V20_GPU_PALETTE_LUT",
                 rgb_matches_lut(filtered_odd, palette_lut, 0, 9) &&
                 !rgb_equal(filtered_odd, baseline_odd),
                 "odd scanline uses the corrected V2.0 palette LUT and differs from baseline");
    probe_record(&stats, "AS_V20_SOURCE_UNCHANGED",
                 memcmp(source_before, framebuffer, sizeof(source_before)) == 0,
                 "V2.0 GPU presentation leaves the indexed source framebuffer unchanged");
    probe_record(&stats, "AS_V20_GPU_DETERMINISTIC",
                 rgb_equal(filtered_even, filtered_even_repeat) &&
                 rgb_equal(filtered_odd, filtered_odd_repeat),
                 "same V2.0 filtered source renders byte-identical sampled readback twice");

    dm1_v2_presentation_mode_reset();
    (void)M11_Render_SetV2Filters(0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 0);
    M11_Render_Shutdown();
    SDL_Quit();

    printf("# summary: %d/%d invariants passed (%d failed)\n",
           stats.passed, stats.total, stats.failed);
    return (stats.failed == 0) ? 0 : 1;
}
