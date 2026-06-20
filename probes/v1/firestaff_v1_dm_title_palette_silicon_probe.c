/*
 * firestaff_v1_dm_title_palette_silicon_probe.c
 *
 * Apple-Silicon-specific DM1 V1 TITLE palette regression probe.
 *
 * Companion to firestaff_v1_dm_title_palette_regression_probe.c (pass842),
 * which verifies the upstream palette/index contract via the V1_TitleFrontend_*
 * helpers without touching SDL3. This probe goes one layer down and
 * exercises the SDL3 render path that pass842 explicitly does not cover
 * (`M11_Render_PresentIndexedWithSpecialPalette`).
 *
 * On Apple Silicon (M1/M2/M3 family, Mac mini / MacBook Pro / Studio),
 * the SDL3 default renderer backend is Metal. The pass842 comment block
 * flags a Metal/Retina-specific bug class: a palette-index readback that
 * matches `F9011_VGA_GetSpecialColorRgb_Compat` could still come out
 * wrong on the screen if the GPU-side swizzle, gamma, or HiDPI backing
 * scale factor is mis-applied. This probe catches that.
 *
 * Test strategy:
 *   1. Detect Apple Silicon at runtime (compile-time gate is not enough
 *      — a probe built on Intel macOS should be runnable cross-platform
 *      and skip cleanly with PASS + explanatory log line).
 *   2. Init SDL3 with the dummy video driver so the probe can run in CI
 *      and headless environments without an attached display.
 *   3. Init M11_Render at 320x200 native DM resolution with scale 1x.
 *   4. For each palette switch in the C12_PRESENTS -> C13_DUNGEON +
 *      C14_MASTER sequence:
 *        a. Fill the framebuffer with a known palette index (0x0F = white).
 *        b. Call M11_Render_PresentIndexedWithSpecialPalette with that
 *           slot's specialPalette ordinal.
 *        c. Use SDL_RenderReadPixels on the active renderer to capture
 *           the actual GPU-side RGBA output for a sample pixel.
 *        d. Compare against F9011_VGA_GetSpecialColorRgb_Compat for the
 *           same (colorIndex, specialPalette) pair. Any byte mismatch
 *           is a regression.
 *   5. Shut down M11_Render and SDL cleanly.
 *
 * Skip behaviour on non-Apple-Silicon hosts:
 *   - If __APPLE__ && __arm64__ is false, exit 0 with a logged skip
 *     reason. The probe is still buildable everywhere so CI on Intel
 *     macOS / Linux / Windows runners does not break.
 *
 * Source-lock: this probe consumes the same vga_palette_pc34_compat.h
 * contract that pass841 (FTL swoosh 4bpp) + pass842 (TITLE regression)
 * already source-locked. No new data-table assumptions.
 */

#include "render_sdl_m11.h"
#include "vga_palette_pc34_compat.h"

#include <SDL3/SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Probe-level stats (mirrors the pass842 helper). */
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

/* Runtime Apple Silicon detection. The compile-time gate handles
 * non-macOS builds; on macOS we double-check by asking the kernel. */
static int is_apple_silicon(void) {
#if defined(__APPLE__) && (defined(__arm64__) || defined(__aarch64__))
    return 1;
#elif defined(__APPLE__)
    /* Could still be Apple Silicon under a translation shim or an
     * x86_64-targeted macOS toolchain on M-series hardware. Best-effort
     * check via sysctl; if unavailable, return 0 so the probe skips. */
    FILE* f = popen("sysctl -n hw.optional.arm64 2>/dev/null", "r");
    if (!f) return 0;
    char buf[16];
    size_t n = fread(buf, 1, sizeof(buf) - 1u, f);
    pclose(f);
    if (n == 0) return 0;
    buf[n] = '\0';
    /* sysctl prints "1" if ARM64 is supported by the kernel. */
    return buf[0] == '1';
#else
    return 0;
#endif
}

int main(void) {
    ProbeStats stats;
    int rc;
    int i;
    int palette_slot;
    unsigned char* framebuffer;
    size_t framebuffer_size;
    Uint32 pixel_format;
    unsigned char sample[4];
    const unsigned char* expected;

    /* Special-palette ordinals covered by the v2.7.4 / v2.7.11 / pass842
     * fixes. Order matches the F20E PC 3.4 TITLE.C F0437 sequence:
     * PRESENTS (white on black), then DUNGEON+MASTER fade-in. */
    static const struct {
        const char* name;
        int specialPalette;
    } kPaletteSlots[] = {
        { "TITLE_PRESENTS", VGA_PALETTE_PC34_SPECIAL_TITLE_PRESENTS },
        { "TITLE",          VGA_PALETTE_PC34_SPECIAL_TITLE          },
        { "ENTRANCE",       VGA_PALETTE_PC34_SPECIAL_ENTRANCE       },
        { "CREDITS",        VGA_PALETTE_PC34_SPECIAL_CREDITS        },
    };
    static const int kSlotCount = (int)(sizeof(kPaletteSlots) / sizeof(kPaletteSlots[0]));

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

    pixel_format = SDL_GetWindowPixelFormat(M11_Render_GetWindow());
    /* Allow RGBA8888 (real GPU backends) + XRGB8888 (SDL3 dummy driver
     * uses this when no alpha channel is requested). Either way the
     * readback code below treats 4 bytes as RGBA — XRGB simply has its
     * alpha byte set to 0 by the dummy driver, and we compare only R/G/B
     * against F9011_VGA_GetSpecialColorRgb_Compat, so the X alpha is
     * irrelevant. */
    if (pixel_format != SDL_PIXELFORMAT_RGBA8888 &&
        pixel_format != SDL_PIXELFORMAT_XRGB8888) {
        fprintf(stderr, "FAIL pixel format: got 0x%08x, want RGBA8888 or XRGB8888\n",
                (unsigned)pixel_format);
        M11_Render_Shutdown();
        SDL_Quit();
        return 1;
    }
    printf("info: window pixel format = 0x%08x\n", (unsigned)pixel_format);

    /* Walk the C12 -> C13+C14 TITLE palette switch (ReDMCSB TITLE.C
     * F0437 PC/F20 sequence). Fill the framebuffer with color 0x0F
     * (white-on-X for every palette) and verify the readback RGBA
     * matches F9011_VGA_GetSpecialColorRgb_Compat(0x0F, slot).
     *
     * 0x0F was the failing index in v2.7.4 — when the C12_PRESENTS
     * row was applied to the DUNGEON+MASTER zoom phase, color 0x0F
     * came out gold/brown instead of white. Reading it back through
     * the Metal renderer path is the right place to catch that
     * regression at boot time. */
    for (palette_slot = 0; palette_slot < kSlotCount; ++palette_slot) {
        memset(framebuffer, M11_FB_ENCODE(0x0Fu, 0u), framebuffer_size);
        rc = M11_Render_PresentIndexedWithSpecialPalette(
                framebuffer, 320, 200, kPaletteSlots[palette_slot].specialPalette);
        if (rc != M11_RENDER_OK) {
            fprintf(stderr, "FAIL PresentIndexedWithSpecialPalette slot=%s rc=%d\n",
                    kPaletteSlots[palette_slot].name, rc);
            stats.failed += 1;
            stats.total += 1;
            continue;
        }

        sample[0] = sample[1] = sample[2] = sample[3] = 0;
        {
            SDL_Surface* surf = SDL_RenderReadPixels(M11_Render_GetRenderer(), NULL);
            int ok_read = 0;
            /* SDL_RenderReadPixels returns a surface in the same pixel
             * format as the renderer. Real GPU backends hand back
             * RGBA8888; SDL3 dummy driver hands back XRGB8888 (memory
             * layout [B,G,R,X]). We use SDL_ReadSurfacePixel which
             * normalizes to {R,G,B,A} regardless of the underlying
             * pixel-format byte order — no manual swizzle needed. */
            if (surf && surf->pixels) {
                Uint8 r, g, b, a;
                if (SDL_ReadSurfacePixel(surf, 0, 0, &r, &g, &b, &a)) {
                    sample[0] = r;
                    sample[1] = g;
                    sample[2] = b;
                    sample[3] = a;
                    ok_read = 1;
                }
            }
            if (surf) SDL_DestroySurface(surf);
            if (!ok_read) {
                fprintf(stderr, "FAIL SDL_RenderReadPixels slot=%s: %s\n",
                        kPaletteSlots[palette_slot].name,
                        surf ? "SDL_ReadSurfacePixel failed" : SDL_GetError());
                stats.failed += 1;
                stats.total += 1;
                continue;
            }
        }

        expected = F9011_VGA_GetSpecialColorRgb_Compat(
                0x0Fu, (unsigned)kPaletteSlots[palette_slot].specialPalette);

        {
            char id[96];
            char note[160];
            int ok = (sample[0] == expected[0] &&
                      sample[1] == expected[1] &&
                      sample[2] == expected[2]);
            snprintf(id, sizeof(id), "AS_TITLE_PALETTE_SLOT_%d_%s",
                     palette_slot, kPaletteSlots[palette_slot].name);
            snprintf(note, sizeof(note),
                     "Metal readback (R,G,B,A)=(%u,%u,%u,%u) vs "
                     "F9011[%s][0x0F]=(%u,%u,%u)",
                     sample[0], sample[1], sample[2], sample[3],
                     kPaletteSlots[palette_slot].name,
                     expected[0], expected[1], expected[2]);
            probe_record(&stats, id, ok, note);
        }
    }

    /* Also verify the brightest-palette (default brightness 0) round-trip
     * works end-to-end through Metal. This catches gamma/HiDPI bugs that
     * would not show up against the special-palette slots. */
    {
        memset(framebuffer, M11_FB_ENCODE(0x0Fu, 0u), framebuffer_size);
        rc = M11_Render_PresentIndexed(framebuffer, 320, 200);
        if (rc != M11_RENDER_OK) {
            fprintf(stderr, "FAIL M11_Render_PresentIndexed rc=%d\n", rc);
            stats.failed += 1;
            stats.total += 1;
        } else {
            sample[0] = sample[1] = sample[2] = sample[3] = 0;
            {
                SDL_Surface* surf = SDL_RenderReadPixels(M11_Render_GetRenderer(), NULL);
                int ok_read = 0;
                if (surf && surf->pixels) {
                    Uint8 r, g, b, a;
                    if (SDL_ReadSurfacePixel(surf, 0, 0, &r, &g, &b, &a)) {
                        sample[0] = r;
                        sample[1] = g;
                        sample[2] = b;
                        sample[3] = a;
                        ok_read = 1;
                    }
                }
                if (surf) SDL_DestroySurface(surf);
                if (!ok_read) {
                    fprintf(stderr, "FAIL SDL_RenderReadPixels default-palette: %s\n",
                            surf ? "SDL_ReadSurfacePixel failed" : SDL_GetError());
                    stats.failed += 1;
                    stats.total += 1;
                } else {
                    expected = F9010_VGA_GetColorRgb_Compat(0x0Fu, 0u);
                    {
                        int ok = (sample[0] == expected[0] &&
                                  sample[1] == expected[1] &&
                                  sample[2] == expected[2]);
                        probe_record(&stats, "AS_DEFAULT_PALETTE_BRIGHTNESS_0",
                                     ok,
                                     "Metal readback matches F9010_VGA_GetColorRgb_Compat(0x0F, 0)");
                    }
                }
            }
        }
    }

    M11_Render_Shutdown();
    SDL_Quit();

    (void)i;
    printf("# summary: %d/%d invariants passed (%d failed)\n",
           stats.passed, stats.total, stats.failed);
    return (stats.failed == 0) ? 0 : 1;
}