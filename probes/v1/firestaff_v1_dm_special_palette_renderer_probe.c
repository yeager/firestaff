/*
 * firestaff_v1_dm_special_palette_renderer_probe.c
 *
 * Cross-platform DM1 V1 special-palette renderer readback.
 *
 * The Apple-Silicon-specific TITLE probe can skip on non-arm64 hosts, but
 * release smoke showed that the risky boundary is the live
 * M11_Render_PresentIndexedWithSpecialPalette path used by TITLE and
 * ENTRANCE.  This probe forces SDL's dummy video driver and reads back
 * one rendered pixel for each DM1 special palette so CI exercises the
 * same palette upload/texture path on every platform.
 *
 * Source anchors:
 *   - ReDMCSB TITLE.C F0437 lines 319-324 uses C12_PRESENTS.
 *   - ReDMCSB TITLE.C F0437 lines 340-402 uses the PC34 VGA
 *     C13_DUNGEON+C14_MASTER rows from VIDEODRV.C C25_VGA.
 *   - ReDMCSB ENTRANCE.C lines 426-595 switches to the entrance palette.
 */

#include "render_sdl_m11.h"
#include "vga_palette_pc34_compat.h"

#include <SDL3/SDL.h>

#include <stdio.h>
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

static void force_dummy_video_driver(void) {
#if defined(_WIN32)
    _putenv_s("SDL_VIDEODRIVER", "dummy");
#else
    setenv("SDL_VIDEODRIVER", "dummy", 1);
#endif
}

static int read_sample(unsigned char out_rgba[4]) {
    SDL_Surface* surf;
    int ok = 0;
    Uint8 r = 0;
    Uint8 g = 0;
    Uint8 b = 0;
    Uint8 a = 0;

    surf = SDL_RenderReadPixels(M11_Render_GetRenderer(), NULL);
    if (surf && surf->pixels && SDL_ReadSurfacePixel(surf, 0, 0, &r, &g, &b, &a)) {
        out_rgba[0] = r;
        out_rgba[1] = g;
        out_rgba[2] = b;
        out_rgba[3] = a;
        ok = 1;
    }
    if (surf) {
        SDL_DestroySurface(surf);
    }
    return ok;
}

int main(void) {
    static const struct {
        const char* name;
        int specialPalette;
        unsigned char colorIndex;
    } kCases[] = {
        { "TITLE_PRESENTS_WHITE", VGA_PALETTE_PC34_SPECIAL_TITLE_PRESENTS, 0x0Fu },
        { "TITLE_MASTER_RED",     VGA_PALETTE_PC34_SPECIAL_TITLE,          0x0Cu },
        { "TITLE_MASTER_WHITE",   VGA_PALETTE_PC34_SPECIAL_TITLE,          0x0Fu },
        { "ENTRANCE_WHITE",       VGA_PALETTE_PC34_SPECIAL_ENTRANCE,       0x0Fu },
        { "CREDITS_WHITE",        VGA_PALETTE_PC34_SPECIAL_CREDITS,        0x0Fu },
        { "TITLE_MASTER_BLUE",    VGA_PALETTE_PC34_SPECIAL_TITLE,          0x0Eu },
    };
    ProbeStats stats;
    unsigned char* framebuffer;
    size_t framebuffer_size;
    size_t i;
    int rc;

    memset(&stats, 0, sizeof(stats));
    printf("probe=firestaff_v1_dm_special_palette_renderer\n");

    force_dummy_video_driver();
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
        fprintf(stderr, "FAIL framebuffer size=%zu\n", framebuffer_size);
        M11_Render_Shutdown();
        SDL_Quit();
        return 1;
    }

    for (i = 0; i < sizeof(kCases) / sizeof(kCases[0]); ++i) {
        const unsigned char* expected;
        unsigned char sample[4] = {0, 0, 0, 0};
        char id[96];
        char note[192];
        int ok;

        memset(framebuffer, M11_FB_ENCODE(kCases[i].colorIndex, 0u), framebuffer_size);
        rc = M11_Render_PresentIndexedWithSpecialPalette(
                framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT, kCases[i].specialPalette);
        if (rc != M11_RENDER_OK) {
            snprintf(note, sizeof(note), "present rc=%d", rc);
            probe_record(&stats, kCases[i].name, 0, note);
            continue;
        }
        if (!read_sample(sample)) {
            snprintf(note, sizeof(note), "SDL_RenderReadPixels failed: %s", SDL_GetError());
            probe_record(&stats, kCases[i].name, 0, note);
            continue;
        }

        expected = F9011_VGA_GetSpecialColorRgb_Compat(
                kCases[i].colorIndex, (unsigned)kCases[i].specialPalette);
        ok = expected
             && sample[0] == expected[0]
             && sample[1] == expected[1]
             && sample[2] == expected[2];
        snprintf(id, sizeof(id), "DM1_SPECIAL_PALETTE_%s", kCases[i].name);
        snprintf(note, sizeof(note),
                 "readback=(%u,%u,%u,%u) expected=(%u,%u,%u)",
                 sample[0], sample[1], sample[2], sample[3],
                 expected ? expected[0] : 0u,
                 expected ? expected[1] : 0u,
                 expected ? expected[2] : 0u);
        probe_record(&stats, id, ok, note);
    }

    M11_Render_Shutdown();
    SDL_Quit();

    printf("# summary: %d/%d invariants passed (%d failed)\n",
           stats.passed, stats.total, stats.failed);
    return stats.failed == 0 ? 0 : 1;
}
