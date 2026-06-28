/*
 * firestaff_v1_dm_v21_upscale_readback_receipt_silicon_probe.c
 *
 * Narrow Apple-Silicon receipt for the DM1 V2.1 EPX/RGBA handoff.
 *
 * Companion to firestaff_v1_dm_v21_upscale_renderer_silicon_probe.c:
 * that probe proves the path is non-black and stable; this one compares
 * a deterministic spread of SDL_RenderReadPixels samples against the
 * exact CPU RGBA bytes handed to M11_Render_PresentRGBA.
 *
 * Source-lock: ReDMCSB DUNVIEW.C:8318-8542 owns the V1 indexed source
 * framebuffer composition order. The EPX 2x upscale and SDL/Metal
 * readback path are Firestaff presentation-only work.
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

typedef struct RgbaSample {
    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;
} RgbaSample;

typedef struct ReadbackPoint {
    int srcX;
    int srcY;
    unsigned char value;
    const char* label;
} ReadbackPoint;

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

static uint32_t fnv1a_sample(uint32_t hash, RgbaSample s) {
    hash ^= s.r; hash *= 16777619u;
    hash ^= s.g; hash *= 16777619u;
    hash ^= s.b; hash *= 16777619u;
    hash ^= s.a; hash *= 16777619u;
    return hash;
}

static RgbaSample expected_sample_from_cpu_bytes(const uint32_t* rgba,
                                                 int outW,
                                                 int outX,
                                                 int outY) {
    const unsigned char* bytes =
        (const unsigned char*)&rgba[outY * outW + outX];
    RgbaSample s;
    s.r = bytes[0];
    s.g = bytes[1];
    s.b = bytes[2];
    s.a = bytes[3];
    return s;
}

static int same_sample(RgbaSample a, RgbaSample b) {
    return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
}

static int sample_is_nonblack(RgbaSample s) {
    return ((unsigned)s.r + (unsigned)s.g + (unsigned)s.b) > 0u;
}

static void fill_receipt_source(uint8_t* fb,
                                const ReadbackPoint* points,
                                size_t pointCount) {
    size_t i;
    if (!fb) return;
    memset(fb, 0x00, M11_FB_BYTES);
    for (i = 0; i < pointCount; ++i) {
        fb[points[i].srcY * M11_FB_WIDTH + points[i].srcX] = points[i].value;
    }
}

static int read_surface_samples(SDL_Surface* surf,
                                const uint32_t* rgba,
                                int outW,
                                const ReadbackPoint* points,
                                size_t pointCount,
                                uint32_t* outCpuHash,
                                uint32_t* outGpuHash,
                                int* outMatched,
                                int* outNonblack,
                                char* firstMismatch,
                                size_t firstMismatchSize) {
    size_t i;
    uint32_t cpuHash = 2166136261u;
    uint32_t gpuHash = 2166136261u;
    int matched = 0;
    int nonblack = 0;
    int firstMismatchWritten = 0;

    if (!surf || !rgba || !points) return 0;

    for (i = 0; i < pointCount; ++i) {
        int outX = points[i].srcX * 2;
        int outY = points[i].srcY * 2;
        RgbaSample expected = expected_sample_from_cpu_bytes(rgba, outW, outX, outY);
        RgbaSample actual;
        SDL_ReadSurfacePixel(surf, outX, outY,
                             &actual.r, &actual.g, &actual.b, &actual.a);
        cpuHash = fnv1a_sample(cpuHash, expected);
        gpuHash = fnv1a_sample(gpuHash, actual);
        if (same_sample(expected, actual)) {
            matched++;
        } else if (!firstMismatchWritten && firstMismatch && firstMismatchSize > 0u) {
            snprintf(firstMismatch, firstMismatchSize,
                     "%s at (%d,%d): cpu=(%u,%u,%u,%u) gpu=(%u,%u,%u,%u)",
                     points[i].label, outX, outY,
                     (unsigned)expected.r, (unsigned)expected.g,
                     (unsigned)expected.b, (unsigned)expected.a,
                     (unsigned)actual.r, (unsigned)actual.g,
                     (unsigned)actual.b, (unsigned)actual.a);
            firstMismatchWritten = 1;
        }
        if (sample_is_nonblack(actual)) {
            nonblack++;
        }
    }

    if (outCpuHash) *outCpuHash = cpuHash;
    if (outGpuHash) *outGpuHash = gpuHash;
    if (outMatched) *outMatched = matched;
    if (outNonblack) *outNonblack = nonblack;
    if (!firstMismatchWritten && firstMismatch && firstMismatchSize > 0u) {
        firstMismatch[0] = '\0';
    }
    return 1;
}

int main(void) {
    static const ReadbackPoint points[] = {
        {  16,  16, M11_FB_ENCODE(15, 0), "white upper-left" },
        {  80,  42, M11_FB_ENCODE(9, 0),  "blue vertical lane" },
        { 160, 100, M11_FB_ENCODE(12, 1), "dim red center" },
        { 245, 150, M11_FB_ENCODE(10, 0), "green lower-right" },
        { 300,  24, M11_FB_ENCODE(14, 2), "dim yellow far-right" },
        {  40, 176, M11_FB_ENCODE(13, 3), "dark magenta lower-left" },
        {  10, 190, 0x00,                 "black control" }
    };
    ProbeStats stats;
    uint8_t* src;
    uint8_t source_before[M11_FB_BYTES];
    const uint32_t* rgba;
    SDL_Surface* firstSurface;
    SDL_Surface* repeatSurface;
    uint32_t firstCpuHash = 0u;
    uint32_t firstGpuHash = 0u;
    uint32_t repeatCpuHash = 0u;
    uint32_t repeatGpuHash = 0u;
    int firstMatched = 0;
    int firstNonblack = 0;
    int repeatMatched = 0;
    int repeatNonblack = 0;
    int outW = 0;
    int outH = 0;
    int rc;
    char mismatch[240];

    memset(&stats, 0, sizeof(stats));
    mismatch[0] = '\0';

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
    probe_record(&stats, "AS_V21_RECEIPT_PRESENTATION_MODE",
                 dm1_v2_presentation_mode_is_v21() == 1,
                 "DM1 V2 presentation mode resolves to V2.1 upscale");

    v21_viewport_init(2);
    src = v21_viewport_get_v1_framebuffer_mut();
    if (!src) {
        fprintf(stderr, "FAIL v21_viewport_get_v1_framebuffer_mut returned NULL\n");
        return 1;
    }
    fill_receipt_source(src, points, sizeof(points) / sizeof(points[0]));
    memcpy(source_before, src, sizeof(source_before));
    v21_viewport_render_full_pipeline();
    rgba = v21_viewport_get_rgba(&outW, &outH);
    probe_record(&stats, "AS_V21_RECEIPT_CPU_DIMENSIONS",
                 rgba != NULL && outW == 640 && outH == 400,
                 "V2.1 receipt source expands to 640x400 CPU RGBA");
    probe_record(&stats, "AS_V21_RECEIPT_SOURCE_UNCHANGED",
                 memcmp(source_before, src, sizeof(source_before)) == 0,
                 "receipt render leaves the indexed V1 source unchanged");

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

    rc = M11_Render_PresentRGBA((const unsigned char*)rgba, outW, outH);
    if (rc != M11_RENDER_OK) {
        fprintf(stderr, "FAIL M11_Render_PresentRGBA first receipt: rc=%d\n", rc);
        M11_Render_Shutdown();
        SDL_Quit();
        return 1;
    }
    firstSurface = SDL_RenderReadPixels(M11_Render_GetRenderer(), NULL);
    if (!firstSurface || !firstSurface->pixels) {
        fprintf(stderr, "FAIL SDL_RenderReadPixels first receipt: %s\n",
                firstSurface ? "no pixels" : SDL_GetError());
        if (firstSurface) SDL_DestroySurface(firstSurface);
        M11_Render_Shutdown();
        SDL_Quit();
        return 1;
    }
    probe_record(&stats, "AS_V21_RECEIPT_SURFACE_DIMENSIONS",
                 firstSurface->w == 640 && firstSurface->h == 400,
                 "SDL readback surface keeps the 640x400 V2.1 output extent");
    (void)read_surface_samples(firstSurface, rgba, outW,
                               points, sizeof(points) / sizeof(points[0]),
                               &firstCpuHash, &firstGpuHash,
                               &firstMatched, &firstNonblack,
                               mismatch, sizeof(mismatch));
    SDL_DestroySurface(firstSurface);

    {
        char note[240];
        snprintf(note, sizeof(note),
                 "matched=%d/%u cpuHash=0x%08X gpuHash=0x%08X%s%s",
                 firstMatched,
                 (unsigned)(sizeof(points) / sizeof(points[0])),
                 firstCpuHash, firstGpuHash,
                 mismatch[0] ? " firstMismatch=" : "",
                 mismatch[0] ? mismatch : "");
        probe_record(&stats, "AS_V21_RECEIPT_GPU_MATCHES_CPU_SAMPLES",
                     firstMatched == (int)(sizeof(points) / sizeof(points[0])) &&
                     firstCpuHash == firstGpuHash,
                     note);
    }
    probe_record(&stats, "AS_V21_RECEIPT_NONBLACK_SPREAD",
                 firstNonblack == 6,
                 "six colored receipt samples survive GPU readback and one black control stays dark");

    rc = M11_Render_PresentRGBA((const unsigned char*)rgba, outW, outH);
    if (rc != M11_RENDER_OK) {
        fprintf(stderr, "FAIL M11_Render_PresentRGBA repeat receipt: rc=%d\n", rc);
        M11_Render_Shutdown();
        SDL_Quit();
        return 1;
    }
    repeatSurface = SDL_RenderReadPixels(M11_Render_GetRenderer(), NULL);
    if (!repeatSurface || !repeatSurface->pixels) {
        fprintf(stderr, "FAIL SDL_RenderReadPixels repeat receipt: %s\n",
                repeatSurface ? "no pixels" : SDL_GetError());
        if (repeatSurface) SDL_DestroySurface(repeatSurface);
        M11_Render_Shutdown();
        SDL_Quit();
        return 1;
    }
    (void)read_surface_samples(repeatSurface, rgba, outW,
                               points, sizeof(points) / sizeof(points[0]),
                               &repeatCpuHash, &repeatGpuHash,
                               &repeatMatched, &repeatNonblack,
                               mismatch, sizeof(mismatch));
    SDL_DestroySurface(repeatSurface);

    {
        char note[180];
        snprintf(note, sizeof(note),
                 "repeat matched=%d/%u firstGpuHash=0x%08X repeatGpuHash=0x%08X",
                 repeatMatched,
                 (unsigned)(sizeof(points) / sizeof(points[0])),
                 firstGpuHash, repeatGpuHash);
        probe_record(&stats, "AS_V21_RECEIPT_DETERMINISTIC_REPEAT",
                     repeatMatched == firstMatched &&
                     repeatNonblack == firstNonblack &&
                     repeatCpuHash == firstCpuHash &&
                     repeatGpuHash == firstGpuHash,
                     note);
    }

    dm1_v2_presentation_mode_reset();
    M11_Render_Shutdown();
    SDL_Quit();

    printf("# summary: %d/%d invariants passed (%d failed)\n",
           stats.passed, stats.total, stats.failed);
    return (stats.failed == 0) ? 0 : 1;
}
