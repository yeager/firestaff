/*
 * firestaff_v1_dm_title_swoosh_handoff_palette_probe.c
 *
 * Regression probe for the release-3.0.2 MacBook Pro report:
 * after the FTL/SWSH intro, the following Dungeon Master C001 title
 * animation used the wrong palette.
 *
 * The older pass841/pass842/pass897 probes cover SWSH decode, TITLE
 * palette tables, and isolated SDL special-palette readback. This probe
 * covers the missing runtime order:
 *
 *   RGBA SWSH-like frame -> discard handoff texture -> GRAPHICS.DAT C001
 *   indexed TITLE zoom blit -> C13_DUNGEON + C14_MASTER special palette.
 *
 * Source-lock:
 *   ReDMCSB SWSH.C F2255 presents the FTL logo before startup handoff.
 *   STARTUP1.C:143 calls F0437_STARTEND_DrawTitle.
 *   TITLE.C:362-367 applies C13_DUNGEON and C14_MASTER before the 18
 *   C001 zoom blits at TITLE.C:385-387.
 */

#include "asset_loader_m11.h"
#include "render_sdl_m11.h"
#include "title_frontend_v1.h"
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

static void record(ProbeStats* stats, const char* id, int ok, const char* note) {
    stats->total += 1;
    if (ok) {
        stats->passed += 1;
        printf("PASS %s %s\n", id, note);
    } else {
        stats->failed += 1;
        printf("FAIL %s %s\n", id, note);
    }
}

static int read_renderer_pixel_rgb(int x, int y, unsigned char outRgb[3]) {
    SDL_Surface* surf;
    Uint8 r = 0, g = 0, b = 0, a = 0;
    int ok = 0;
    if (!outRgb) return 0;
    surf = SDL_RenderReadPixels(M11_Render_GetRenderer(), NULL);
    if (surf && surf->pixels && x >= 0 && y >= 0 && x < surf->w && y < surf->h) {
        if (SDL_ReadSurfacePixel(surf, x, y, &r, &g, &b, &a)) {
            outRgb[0] = r;
            outRgb[1] = g;
            outRgb[2] = b;
            ok = 1;
        }
    }
    if (surf) SDL_DestroySurface(surf);
    return ok;
}

static int find_title_sample(const unsigned char* framebuffer,
                             int* outX,
                             int* outY,
                             unsigned char* outIndex) {
    int x;
    int y;
    if (!framebuffer || !outX || !outY || !outIndex) return 0;
    for (y = 40; y < 120; ++y) {
        for (x = 0; x < 320; ++x) {
            unsigned char idx = framebuffer[y * 320 + x] & 0x0fU;
            const unsigned char* titleRgb;
            if (idx == 0U) continue;
            titleRgb = F9011_VGA_GetSpecialColorRgb_Compat(
                idx, VGA_PALETTE_PC34_SPECIAL_TITLE);
            if (!titleRgb) continue;
            if (!(titleRgb[0] == 0x11 && titleRgb[1] == 0x22 && titleRgb[2] == 0x33)) {
                *outX = x;
                *outY = y;
                *outIndex = idx;
                return 1;
            }
        }
    }
    return 0;
}

int main(int argc, char** argv) {
    const char* graphicsPath;
    FILE* graphicsFile;
    M11_AssetLoader loader;
    const M11_AssetSlot* titleGraphic;
    unsigned char* framebuffer;
    unsigned char* poisonRgba;
    V1_TitleFrontendSourceAnimationStep step;
    int sampleX = 0;
    int sampleY = 0;
    unsigned char sampleIndex = 0;
    const unsigned char* expected;
    const unsigned char* presented;
    int presentedW = 0;
    int presentedH = 0;
    unsigned char readbackRgb[3] = {0, 0, 0};
    ProbeStats stats = {0, 0, 0};

    if (argc < 2) {
        fprintf(stderr, "usage: %s /path/to/GRAPHICS.DAT\n", argv[0]);
        return 2;
    }
    graphicsPath = argv[1];

#if defined(_WIN32)
    _putenv_s("SDL_VIDEODRIVER", "dummy");
#else
    setenv("SDL_VIDEODRIVER", "dummy", 1);
#endif

    printf("probe=firestaff_v1_dm_title_swoosh_handoff_palette\n");
    printf("graphicsDat=%s\n", graphicsPath);
    printf("sourceEvidence=SWSH.C F2255; STARTUP1.C:143; TITLE.C:362-367,385-387; DRAWVIEW.C G8160/G8161\n");

    graphicsFile = fopen(graphicsPath, "rb");
    if (!graphicsFile) {
        printf("SKIP TITLE_SWOOSH_HANDOFF_GRAPHICS_DAT_MISSING %s\n", graphicsPath);
        return 0;
    }
    fclose(graphicsFile);

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "FAIL SDL_Init: %s\n", SDL_GetError());
        return 1;
    }
    if (M11_Render_Init(320, 200, M11_SCALE_1X) != M11_RENDER_OK) {
        fprintf(stderr, "FAIL M11_Render_Init\n");
        SDL_Quit();
        return 1;
    }

    memset(&loader, 0, sizeof(loader));
    if (!M11_AssetLoader_Init(&loader, graphicsPath)) {
        fprintf(stderr, "FAIL M11_AssetLoader_Init\n");
        M11_Render_Shutdown();
        SDL_Quit();
        return 1;
    }
    titleGraphic = M11_AssetLoader_Load(&loader, 1U);
    framebuffer = M11_Render_GetFramebuffer();
    if (!titleGraphic || titleGraphic->width < 320U || titleGraphic->height < 175U || !framebuffer) {
        fprintf(stderr, "FAIL title C001 asset unavailable or wrong size\n");
        M11_AssetLoader_Shutdown(&loader);
        M11_Render_Shutdown();
        SDL_Quit();
        return 1;
    }

    poisonRgba = (unsigned char*)malloc((size_t)M11_FB_BYTES * 4U);
    if (!poisonRgba) {
        M11_AssetLoader_Shutdown(&loader);
        M11_Render_Shutdown();
        SDL_Quit();
        return 1;
    }
    for (int i = 0; i < M11_FB_BYTES; ++i) {
        poisonRgba[i * 4 + 0] = 0x11;
        poisonRgba[i * 4 + 1] = 0x22;
        poisonRgba[i * 4 + 2] = 0x33;
        poisonRgba[i * 4 + 3] = 0xff;
    }
    record(&stats,
           "TITLE_SWOOSH_HANDOFF_RGBA_PRESENT",
           M11_Render_PresentRGBA(poisonRgba, 320, 200) == M11_RENDER_OK,
           "SWSH-like true-colour frame presented before TITLE");

    M11_Render_DiscardPresentationTexture();

    if (!V1_TitleFrontend_GetSourceAnimationStep(19U, &step) ||
        step.kind != V1_TITLE_FRONTEND_SOURCE_EVENT_ZOOM_BLIT) {
        fprintf(stderr, "FAIL source step 19 unavailable\n");
        free(poisonRgba);
        M11_AssetLoader_Shutdown(&loader);
        M11_Render_Shutdown();
        SDL_Quit();
        return 1;
    }

    memset(framebuffer, 0, (size_t)M11_FB_BYTES);
    M11_AssetLoader_BlitSubRectScaled(titleGraphic,
                                      framebuffer,
                                      M11_FB_WIDTH,
                                      M11_FB_HEIGHT,
                                      (int)step.x,
                                      (int)step.y,
                                      (int)step.width,
                                      (int)step.height,
                                      0,
                                      0,
                                      320,
                                      80,
                                      -1);
    record(&stats,
           "TITLE_SWOOSH_HANDOFF_SAMPLE_FOUND",
           find_title_sample(framebuffer, &sampleX, &sampleY, &sampleIndex),
           "found non-black C001 title pixel inside the zoom blit");

    expected = F9011_VGA_GetSpecialColorRgb_Compat(
        sampleIndex, VGA_PALETTE_PC34_SPECIAL_TITLE);
    record(&stats,
           "TITLE_SWOOSH_HANDOFF_INDEXED_PRESENT",
           M11_Render_PresentIndexedWithSpecialPalette(
               framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT,
               VGA_PALETTE_PC34_SPECIAL_TITLE) == M11_RENDER_OK,
           "indexed C001 title frame presented after RGBA handoff");

    presented = M11_Render_GetPresentedRGBA(&presentedW, &presentedH);
    if (presented && expected &&
        sampleX >= 0 && sampleY >= 0 && sampleX < presentedW && sampleY < presentedH) {
        const unsigned char* px = presented + ((sampleY * presentedW + sampleX) * 4);
        int ok = (px[0] == expected[0] && px[1] == expected[1] && px[2] == expected[2]);
        char note[160];
        snprintf(note, sizeof(note),
                 "sample=(%d,%d) idx=%u presented=(%u,%u,%u) expected=(%u,%u,%u)",
                 sampleX, sampleY, sampleIndex,
                 px[0], px[1], px[2], expected[0], expected[1], expected[2]);
        record(&stats, "TITLE_SWOOSH_HANDOFF_PRESENT_BUFFER_RGB", ok, note);
    } else {
        record(&stats, "TITLE_SWOOSH_HANDOFF_PRESENT_BUFFER_RGB", 0,
               "presented buffer unavailable");
    }

    if (expected && read_renderer_pixel_rgb(sampleX, sampleY, readbackRgb)) {
        int ok = (readbackRgb[0] == expected[0] &&
                  readbackRgb[1] == expected[1] &&
                  readbackRgb[2] == expected[2]);
        char note[160];
        snprintf(note, sizeof(note),
                 "sample=(%d,%d) idx=%u readback=(%u,%u,%u) expected=(%u,%u,%u)",
                 sampleX, sampleY, sampleIndex,
                 readbackRgb[0], readbackRgb[1], readbackRgb[2],
                 expected[0], expected[1], expected[2]);
        record(&stats, "TITLE_SWOOSH_HANDOFF_SDL_READBACK_RGB", ok, note);
    } else {
        record(&stats, "TITLE_SWOOSH_HANDOFF_SDL_READBACK_RGB", 0,
               "SDL renderer readback unavailable");
    }

    printf("summary=%d passed %d failed total=%d\n",
           stats.passed, stats.failed, stats.total);

    free(poisonRgba);
    M11_AssetLoader_Shutdown(&loader);
    M11_Render_Shutdown();
    SDL_Quit();
    return stats.failed == 0 ? 0 : 1;
}
