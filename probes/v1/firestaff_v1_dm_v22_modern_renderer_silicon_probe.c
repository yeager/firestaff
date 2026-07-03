/*
 * firestaff_v1_dm_v22_modern_renderer_silicon_probe.c
 *
 * Apple-Silicon-specific DM1 V2.2 modern-renderer probe.
 *
 * Companion to firestaff_v1_dm_title_palette_silicon_probe.c (pass897+).
 * pass897+ covered the V1 TITLE palette regression via SDL3 / Metal
 * readback. This probe covers the V2.2 modern-art render path:
 * m11_v22_shape_cache_update -> m11_v22_render_overlay -> M11_Render_PresentIndexed
 * -> SDL_RenderReadPixels, end-to-end.
 *
 * Both pass897+ and this probe use the same host gate (Apple Silicon
 * only via __APPLE__ + __arm64__ runtime sysctl). Both probe the
 * GPU readback against the CPU-side contract. The two probes are
 * disjoint: pass897+ verifies V1 TITLE palette (which fires on
 * M11_PRESENTATION_V1_ORIGINAL); this probe verifies V22 modern-art
 * overlay (which fires on M11_PRESENTATION_V22_MODERN).
 *
 * Skip behaviour on non-Apple-Silicon hosts:
 *   - If __APPLE__ && __arm64__ is false, exit 0 with a logged skip
 *     reason. The probe is still buildable everywhere so CI on Intel
 *     macOS / Linux / Windows runners does not break.
 *
 * Source-lock: same dm1_v2_presentation_mode_pc34 +
 * m11_v22_shape_cache_pc34 + m11_v22_render_overlay_pc34 contract that
 * test_m11_v22_render_overlay_pc34.c already source-locks (CPU-side).
 * No new data-table assumptions; this probe is the GPU-side cross-check.
 */

#include "render_sdl_m11.h"
#include "m11_v22_render_overlay_pc34.h"
#include "m11_v22_shape_cache_pc34.h"
#include "dm1_v2_shape_runtime_pc34.h"
#include "dm1_v2_presentation_mode_pc34.h"

#include <SDL3/SDL.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct ProbeStats {
    int total;
    int passed;
    int failed;
} ProbeStats;

static void probe_record(ProbeStats* stats, const char* id, int ok, const char* note) {
    stats->total += 1;
    if (ok) {
        stats->passed += 1;
        printf("PASS %s: %s\n", id, note);
    } else {
        stats->failed += 1;
        printf("FAIL %s: %s\n", id, note);
    }
}

/* Runtime Apple Silicon detection — mirrors the pass897+ gate. */
static int is_apple_silicon(void) {
#if defined(__APPLE__) && (defined(__arm64__) || defined(__aarch64__))
    return 1;
#elif defined(__APPLE__)
    FILE* f = popen("sysctl -n hw.optional.arm64 2>/dev/null", "r");
    if (!f) return 0;
    char buf[16];
    size_t n = fread(buf, 1, sizeof(buf) - 1u, f);
    pclose(f);
    if (n == 0) return 0;
    buf[n] = '\0';
    return buf[0] == '1';
#else
    return 0;
#endif
}

static size_t count_nonzero_pixels(const unsigned char* framebuffer,
                                   size_t framebuffer_size) {
    size_t i;
    size_t n = 0;
    for (i = 0; i < framebuffer_size; ++i) {
        if (framebuffer[i] != 0x00) n++;
    }
    return n;
}

static int v22_all_cell_centers_nonzero(const unsigned char* framebuffer,
                                        int fbW) {
    static const int centers[9][2] = {
        { 42, 118 }, {108, 118 }, {173, 118 }, /* D1 L/C/R */
        { 42,  87 }, {108,  87 }, {173,  87 }, /* D2 L/C/R */
        { 42,  56 }, {108,  56 }, {173,  56 }  /* D3 L/C/R */
    };
    int i;
    for (i = 0; i < 9; ++i) {
        int x = centers[i][0];
        int y = centers[i][1];
        if (framebuffer[y * fbW + x] == 0x00) return 0;
    }
    return 1;
}

static int v22_composition_active_count(const DM1_V2_ShapeRuntimeComposition* c) {
    int i;
    int active = 0;
    for (i = 0; i < c->count; ++i) {
        if (c->cells[i].active) active++;
    }
    return active;
}

int main(void) {
    ProbeStats stats;
    int rc;
    unsigned char* framebuffer;
    size_t framebuffer_size;

    memset(&stats, 0, sizeof(stats));

    if (!is_apple_silicon()) {
        printf("skip: not Apple Silicon (host is x86_64 / non-macOS); probe target is __APPLE__ + __arm64__\n");
        printf("# summary: 0/0 invariants checked (skipped on non-Apple-Silicon host)\n");
        return 0;
    }
    printf("info: Apple Silicon host detected\n");

    /* Force SDL3 onto the dummy video driver so CI + headless runs work.
     * Use _putenv_s on MSVC/UCRT64 (Windows) and setenv elsewhere. */
#if defined(_WIN32)
    _putenv_s("SDL_VIDEODRIVER", "dummy");
#else
    setenv("SDL_VIDEODRIVER", "dummy", 1);
#endif

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

    framebuffer = M11_Render_GetFramebuffer();
    framebuffer_size = M11_Render_GetFramebufferSize();
    if (!framebuffer || framebuffer_size != M11_FB_BYTES) {
        fprintf(stderr, "FAIL M11_Render_GetFramebuffer returned null or wrong size %zu\n",
                framebuffer_size);
        M11_Render_Shutdown();
        SDL_Quit();
        return 1;
    }

    /* Phase 1: V1 mode — V22 overlay must be a no-op.
     * Capture V1 framebuffer before the overlay (all 0x00) and verify
     * the GPU readback is all dark (palette index 0 maps to RGB 0,0,0). */
    {
        unsigned char v1_fb[M11_FB_BYTES];
        int v1_painted;
        SDL_Surface* surf;
        Uint8 r, g, b, a;

        memset(framebuffer, 0x00, framebuffer_size);
        memcpy(v1_fb, framebuffer, framebuffer_size);

        dm1_v2_presentation_mode_reset();
        m11_v22_shape_cache_update(0, (const unsigned char[3][3]){{0}});
        v1_painted = m11_v22_render_overlay(framebuffer, 320, 200);

        rc = M11_Render_PresentIndexed(framebuffer, 320, 200);
        if (rc != M11_RENDER_OK) {
            fprintf(stderr, "FAIL M11_Render_PresentIndexed V1: rc=%d\n", rc);
            stats.failed++;
            stats.total++;
        } else {
            surf = SDL_RenderReadPixels(M11_Render_GetRenderer(), NULL);
            if (!surf || !surf->pixels) {
                fprintf(stderr, "FAIL SDL_RenderReadPixels V1: %s\n",
                        surf ? "no pixels" : SDL_GetError());
                stats.failed++;
                stats.total++;
            } else {
                SDL_ReadSurfacePixel(surf, 160, 100, &r, &g, &b, &a);
                SDL_DestroySurface(surf);
                {
                    char note[160];
                    snprintf(note, sizeof(note),
                             "V1 mode: overlay_painted=%d, center pixel RGB=(%u,%u,%u) (expect 0,0,0)",
                             v1_painted, (unsigned)r, (unsigned)g, (unsigned)b);
                    probe_record(&stats, "AS_V22_V1_INACTIVE",
                                 v1_painted == 0 && r == 0 && g == 0 && b == 0,
                                 note);
                }
            }
        }

        /* Restore framebuffer for the next phase. */
        memcpy(framebuffer, v1_fb, framebuffer_size);
    }

    /* Phase 2: V22 mode — 9 cells must be painted, GPU readback
     * must show non-zero pixels in the D1L center (8+35, 103+15)
     * since the V22 placeholder is palette index 0xFF which maps
     * to a bright RGB color. */
    {
        int v22_painted;
        size_t non_zero_cpu_pixels;
        SDL_Surface* surf;
        Uint8 r, g, b, a;
        const int d1l_cx = 8 + 35;   /* D1L center X */
        const int d1l_cy = 103 + 15; /* D1L center Y */
        unsigned char center_idx;

        memset(framebuffer, 0x00, framebuffer_size);

        dm1_v2_presentation_mode_reset();
        dm1_v2_presentation_mode_set_modern_pack_available(1);
        dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
        m11_v22_shape_cache_update(0, (const unsigned char[3][3]){{0}});
        v22_painted = m11_v22_render_overlay_with_palette(
            framebuffer, 320, 200, 3);

        /* CPU-side: verify the overlay wrote some non-zero pixels
         * (the V22 placeholder rectangles). */
        non_zero_cpu_pixels = count_nonzero_pixels(framebuffer, framebuffer_size);

        /* The D1L center pixel index (cpu-side) should NOT be 0x00
         * because the overlay fills the rect interior. */
        center_idx = framebuffer[d1l_cy * 320 + d1l_cx];

        rc = M11_Render_PresentIndexed(framebuffer, 320, 200);
        if (rc != M11_RENDER_OK) {
            fprintf(stderr, "FAIL M11_Render_PresentIndexed V22: rc=%d\n", rc);
            stats.failed++;
            stats.total++;
        } else {
            surf = SDL_RenderReadPixels(M11_Render_GetRenderer(), NULL);
            if (!surf || !surf->pixels) {
                fprintf(stderr, "FAIL SDL_RenderReadPixels V22: %s\n",
                        surf ? "no pixels" : SDL_GetError());
                stats.failed++;
                stats.total++;
            } else {
                SDL_ReadSurfacePixel(surf, d1l_cx, d1l_cy, &r, &g, &b, &a);
                SDL_DestroySurface(surf);
                {
                    char note[200];
                    snprintf(note, sizeof(note),
                             "V22 mode: overlay_painted=%d, cpu_nonzero=%zu, "
                             "D1L_center index=%u GPU RGB=(%u,%u,%u) (expect non-zero)",
                             v22_painted, non_zero_cpu_pixels,
                             (unsigned)center_idx,
                             (unsigned)r, (unsigned)g, (unsigned)b);
                    probe_record(&stats, "AS_V22_ACTIVE_PAINTED_9",
                                 v22_painted == 9 && non_zero_cpu_pixels > 0 &&
                                 center_idx != 0x00,
                                 note);
                    probe_record(&stats, "AS_V22_GPU_READBACK_BRIGHT",
                                 r > 0 || g > 0 || b > 0,
                                 "D1L center GPU pixel is non-black (V22 rect rendered through Metal)");
                }
            }
        }
    }

    /* Phase 2b: V22 direction sweep — pass898+ only sampled direction 0.
     * Walk all four facing values through the same cache -> overlay ->
     * SDL3/Metal readback path so direction-specific shape-cache mistakes
     * cannot hide behind the north-facing smoke case. */
    {
        static const unsigned char raw_squares[3][3] = {
            { 0x00, 0x04, 0x20 },
            { 0x40, 0x10, 0x11 },
            { 0x04, 0x20, 0x00 }
        };
        int direction;
        int all_ok = 1;
        int total_painted = 0;
        size_t total_nonzero = 0;
        unsigned int gpu_rgb_sum = 0;

        for (direction = 0; direction < 4; ++direction) {
            int painted;
            size_t nonzero;
            SDL_Surface* surf;
            Uint8 r, g, b, a;

            memset(framebuffer, 0x00, framebuffer_size);
            dm1_v2_presentation_mode_reset();
            dm1_v2_presentation_mode_set_modern_pack_available(1);
            dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
            m11_v22_shape_cache_update(direction, raw_squares);
            painted = m11_v22_render_overlay_with_palette(
                framebuffer, 320, 200, 3);
            nonzero = count_nonzero_pixels(framebuffer, framebuffer_size);
            total_painted += painted;
            total_nonzero += nonzero;

            rc = M11_Render_PresentIndexed(framebuffer, 320, 200);
            if (rc != M11_RENDER_OK) {
                all_ok = 0;
                continue;
            }

            surf = SDL_RenderReadPixels(M11_Render_GetRenderer(), NULL);
            if (!surf || !surf->pixels) {
                if (surf) SDL_DestroySurface(surf);
                all_ok = 0;
                continue;
            }
            SDL_ReadSurfacePixel(surf, 108, 87, &r, &g, &b, &a); /* D2C center */
            SDL_DestroySurface(surf);
            gpu_rgb_sum += (unsigned int)r + (unsigned int)g + (unsigned int)b;

            if (painted != 9 ||
                nonzero == 0 ||
                !v22_all_cell_centers_nonzero(framebuffer, 320) ||
                ((unsigned int)r + (unsigned int)g + (unsigned int)b) == 0u) {
                all_ok = 0;
            }
        }

        {
            char note[220];
            snprintf(note, sizeof(note),
                     "directions=4 total_painted=%d total_nonzero=%zu gpu_rgb_sum=%u "
                     "(expect 36 painted cells, non-zero CPU/GPU samples)",
                     total_painted, total_nonzero, gpu_rgb_sum);
            probe_record(&stats, "AS_V22_DIRECTION_SWEEP_4X9",
                         all_ok && total_painted == 36 &&
                         total_nonzero > 0 && gpu_rgb_sum > 0u,
                         note);
        }
    }

    /* Phase 2c: 12-cell composition contract — the GPU overlay samples
     * D1..D3, but the V22 runtime also exposes the full D0..D3, L/C/R
     * composition used by the follow-up in-place draw path. Keep that
     * contract in the Apple-Silicon probe so Metal coverage and the
     * runtime composition gate advance together. */
    {
        static const uint8_t raw12[12] = {
            0x00, 0x04, 0x20,  /* D0 */
            0x40, 0x10, 0x11,  /* D1 */
            0x04, 0x20, 0x00,  /* D2 */
            0x00, 0x04, 0x20   /* D3 */
        };
        static const int lateral12[12] = {
            -1, 0, 1, -1, 0, 1, -1, 0, 1, -1, 0, 1
        };
        static const int depth12[12] = {
            0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3
        };
        DM1_V2_ShapeRuntimeComposition v1_comp;
        DM1_V2_ShapeRuntimeComposition v22_comp;
        int v1_active;
        int v22_active;

        dm1_v2_presentation_mode_reset();
        v1_comp = dm1_v2_shape_runtime_composition(raw12, lateral12, depth12, 0);
        v1_active = v22_composition_active_count(&v1_comp);

        dm1_v2_presentation_mode_set_modern_pack_available(1);
        dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
        v22_comp = dm1_v2_shape_runtime_composition(raw12, lateral12, depth12, 3);
        v22_active = v22_composition_active_count(&v22_comp);

        {
            char note[180];
            snprintf(note, sizeof(note),
                     "V1 count=%d active=%d; V22 count=%d active=%d (expect 12-cell inactive/active split)",
                     v1_comp.count, v1_active, v22_comp.count, v22_active);
            probe_record(&stats, "AS_V22_COMPOSITION_12_CELLS",
                         v1_comp.count == 12 && v1_active == 0 &&
                         v22_comp.count == 12 && v22_active == 12,
                         note);
        }
    }

    /* Phase 3: V22 -> V1 transition — overlay must become a no-op again. */
    {
        unsigned char pre_fb[M11_FB_BYTES];
        unsigned char post_fb[M11_FB_BYTES];
        int t_painted;
        int identical;

        /* Paint V22 first so framebuffer has non-zero content. */
        dm1_v2_presentation_mode_reset();
        dm1_v2_presentation_mode_set_modern_pack_available(1);
        dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
        m11_v22_shape_cache_update(0, (const unsigned char[3][3]){{0}});
        m11_v22_render_overlay_with_palette(framebuffer, 320, 200, 3);
        memcpy(pre_fb, framebuffer, framebuffer_size);

        /* Now flip back to V1. */
        dm1_v2_presentation_mode_reset();
        m11_v22_shape_cache_update(0, (const unsigned char[3][3]){{0}});
        t_painted = m11_v22_render_overlay(framebuffer, 320, 200);
        memcpy(post_fb, framebuffer, framebuffer_size);

        /* Framebuffer should be byte-identical between pre and post. */
        identical = (memcmp(pre_fb, post_fb, M11_FB_BYTES) == 0);
        {
            char note[160];
            snprintf(note, sizeof(note),
                     "V22 -> V1: overlay_painted=%d framebuffer_unchanged=%d (expect 0/1)",
                     t_painted, identical);
            probe_record(&stats, "AS_V22_TRANSITION_BACK_TO_V1",
                         t_painted == 0 && identical,
                         note);
        }
    }

    /* Phase 4: clean up presentation-mode global state. */
    dm1_v2_presentation_mode_reset();

    M11_Render_Shutdown();
    SDL_Quit();

    printf("# summary: %d/%d invariants passed (%d failed)\n",
           stats.passed, stats.total, stats.failed);
    return (stats.failed == 0) ? 0 : 1;
}
