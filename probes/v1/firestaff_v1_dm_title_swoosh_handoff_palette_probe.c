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

static uint32_t hash_indexed_frame(const unsigned char* framebuffer) {
    uint32_t hash = 2166136261u;

    if (!framebuffer) return 0u;
    for (int i = 0; i < M11_FB_BYTES; ++i) {
        hash ^= framebuffer[i];
        hash *= 16777619u;
    }
    return hash;
}

static void verify_original_startup_palette_frame(
    ProbeStats* stats,
    const char* name,
    const M11_AssetSlot* graphic,
    unsigned char* framebuffer,
    int specialPalette) {
    const unsigned char* expected;
    const unsigned char* presented;
    uint32_t sourceHash;
    int x;
    int y;
    int sampleX = -1;
    int sampleY = -1;
    unsigned char sampleIndex = 0;
    int presentedW = 0;
    int presentedH = 0;
    char id[96];

    snprintf(id, sizeof(id), "%s_SOURCE_ASSET_READY", name);
    if (!graphic || graphic->width != M11_FB_WIDTH ||
        graphic->height != M11_FB_HEIGHT || !framebuffer) {
        record(stats, id, 0, "original 320x200 GRAPHICS.DAT frame unavailable");
        return;
    }
    record(stats, id, 1, "original 320x200 GRAPHICS.DAT frame loaded");
    M11_AssetLoader_Blit(graphic, framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT,
                         0, 0, -1);
    sourceHash = hash_indexed_frame(framebuffer);
    for (y = 0; y < M11_FB_HEIGHT && sampleX < 0; ++y) {
        for (x = 0; x < M11_FB_WIDTH; ++x) {
            const unsigned char index = framebuffer[y * M11_FB_WIDTH + x] & 0x0fU;
            if (index != 0U) {
                sampleX = x;
                sampleY = y;
                sampleIndex = index;
                break;
            }
        }
    }
    expected = F9011_VGA_GetSpecialColorRgb_Compat(
        sampleIndex, (unsigned)specialPalette);
    snprintf(id, sizeof(id), "%s_SOURCE_SAMPLE_FOUND", name);
    record(stats, id, sampleX >= 0 && expected != NULL,
           "found a non-black original indexed pixel");
    if (sampleX < 0 || !expected) return;

    (void)M11_Render_SetV2Filters(0, 0, 1, 100, -25, 0,
                                  0, 0, 0, 0, 0);
    snprintf(id, sizeof(id), "%s_V20_FILTERED_PRESENT", name);
    record(stats, id,
           M11_Render_PresentIndexedWithSpecialPalette(
               framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT, specialPalette) ==
               M11_RENDER_OK,
           "V2.0 filters only the source palette-expanded presentation copy");
    presented = M11_Render_GetPresentedRGBA(&presentedW, &presentedH);
    snprintf(id, sizeof(id), "%s_V20_FILTERED_PIXEL", name);
    if (presented && sampleX < presentedW && sampleY < presentedH) {
        const unsigned char* px = presented +
            ((sampleY * presentedW + sampleX) * 4);
        record(stats, id,
               px[0] != expected[0] || px[1] != expected[1] ||
                   px[2] != expected[2],
               "V2.0 changes the expanded original RGB sample");
    } else {
        record(stats, id, 0, "filtered original sample unavailable");
    }
    snprintf(id, sizeof(id), "%s_SOURCE_INDEXED_UNCHANGED", name);
    record(stats, id, hash_indexed_frame(framebuffer) == sourceHash,
           "V2.0 never rewrites the original indexed frame");

    (void)M11_Render_SetV2Filters(0, 0, 0, 100, 0, 0,
                                  0, 0, 0, 0, 0);
    snprintf(id, sizeof(id), "%s_V1_PALETTE_RESTORED", name);
    record(stats, id,
           M11_Render_PresentIndexedWithSpecialPalette(
               framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT, specialPalette) ==
               M11_RENDER_OK,
           "disabling V2.0 restores the source special-palette route");
    presented = M11_Render_GetPresentedRGBA(&presentedW, &presentedH);
    snprintf(id, sizeof(id), "%s_V1_PALETTE_EXACT", name);
    if (presented && sampleX < presentedW && sampleY < presentedH) {
        const unsigned char* px = presented +
            ((sampleY * presentedW + sampleX) * 4);
        record(stats, id,
               px[0] == expected[0] && px[1] == expected[1] &&
                   px[2] == expected[2],
               "unfiltered original sample is byte-identical to its palette");
    } else {
        record(stats, id, 0, "restored original sample unavailable");
    }
}

int main(int argc, char** argv) {
    const char* graphicsPath;
    FILE* graphicsFile;
    M11_AssetLoader loader;
    const M11_AssetSlot* titleGraphic;
    const M11_AssetSlot* entranceGraphic;
    const M11_AssetSlot* creditsGraphic;
    unsigned char* framebuffer;
    unsigned char* poisonRgba;
    V1_TitleFrontendSourceAnimationStep step;
    V1_TitleFrontendC001BlitPlan blitPlan;
    int sampleX = 0;
    int sampleY = 0;
    unsigned char sampleIndex = 0;
    const unsigned char* expected;
    const unsigned char* presented;
    int presentedW = 0;
    int presentedH = 0;
    unsigned char readbackRgb[3] = {0, 0, 0};
    unsigned char filteredRgb[3] = {0, 0, 0};
    uint32_t sourceHash;
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
    printf("sourceEvidence=SWSH.C F2255; STARTUP1.C:143; TITLE.C:362-367,385-387; VIDEODRV.C C25_VGA G8160/G8161\n");

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
    entranceGraphic = M11_AssetLoader_Load(&loader, 4U);
    creditsGraphic = M11_AssetLoader_Load(&loader, 5U);
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

    memset(&blitPlan, 0, sizeof(blitPlan));
    if (!V1_TitleFrontend_GetSourceAnimationStep(2U, &step) ||
        step.kind != V1_TITLE_FRONTEND_SOURCE_EVENT_ZOOM_BLIT ||
        !V1_TitleFrontend_GetC001BlitPlanForStep(&step, &blitPlan) ||
        blitPlan.kind != V1_TITLE_FRONTEND_C001_BLIT_SCALED_REGION) {
        fprintf(stderr, "FAIL source step 2 unavailable\n");
        free(poisonRgba);
        M11_AssetLoader_Shutdown(&loader);
        M11_Render_Shutdown();
        SDL_Quit();
        return 1;
    }

    record(&stats,
           "TITLE_SWOOSH_HANDOFF_ZOOM_GEOMETRY",
           blitPlan.srcX == 0U && blitPlan.srcY == 0U &&
               blitPlan.srcW == 320U && blitPlan.srcH == 80U &&
               blitPlan.dstX == step.x && blitPlan.dstY == step.y &&
               blitPlan.dstW == step.width && blitPlan.dstH == step.height &&
               blitPlan.dstW == 48U && blitPlan.dstH == 12U,
           "F0437 shrinks complete C001 320x80 into the centred first zoom box");

    memset(framebuffer, 0, (size_t)M11_FB_BYTES);
    M11_AssetLoader_BlitSubRectScaled(titleGraphic,
                                      framebuffer,
                                      M11_FB_WIDTH,
                                      M11_FB_HEIGHT,
                                      (int)blitPlan.dstX,
                                      (int)blitPlan.dstY,
                                      (int)blitPlan.dstW,
                                      (int)blitPlan.dstH,
                                      (int)blitPlan.srcX,
                                      (int)blitPlan.srcY,
                                      (int)blitPlan.srcW,
                                      (int)blitPlan.srcH,
                                      blitPlan.transparentColor);
    sourceHash = hash_indexed_frame(framebuffer);
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

    /* V2.0 is allowed to filter only the RGBA presentation copy after the
     * original C001 palette expansion. The original indexed C001 frame must
     * remain intact and a disabled filter returns the exact source RGB. */
    M11_Render_SetV2PresentationActive(1);
    (void)M11_Render_SetV2Filters(0, 0, 1, 100, -25, 0,
                                  0, 0, 0, 0, 0);
    record(&stats,
           "TITLE_SWOOSH_HANDOFF_V20_FILTERED_PRESENT",
           M11_Render_PresentIndexedWithSpecialPalette(
               framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT,
               VGA_PALETTE_PC34_SPECIAL_TITLE) == M11_RENDER_OK,
           "V2.0 filters the expanded C001 special-palette presentation");
    presented = M11_Render_GetPresentedRGBA(&presentedW, &presentedH);
    if (presented && sampleX >= 0 && sampleY >= 0 &&
        sampleX < presentedW && sampleY < presentedH) {
        const unsigned char* px = presented +
            ((sampleY * presentedW + sampleX) * 4);
        filteredRgb[0] = px[0];
        filteredRgb[1] = px[1];
        filteredRgb[2] = px[2];
        record(&stats,
               "TITLE_SWOOSH_HANDOFF_V20_FILTERED_PIXELS",
               filteredRgb[0] != readbackRgb[0] ||
                   filteredRgb[1] != readbackRgb[1] ||
                   filteredRgb[2] != readbackRgb[2],
               "V2.0 changes only the presented C001 RGB sample");
    } else {
        record(&stats, "TITLE_SWOOSH_HANDOFF_V20_FILTERED_PIXELS", 0,
               "filtered C001 sample unavailable");
    }
    record(&stats,
           "TITLE_SWOOSH_HANDOFF_SOURCE_INDEXED_UNCHANGED",
           hash_indexed_frame(framebuffer) == sourceHash,
           "V2.0 never rewrites the source C001 indexed frame");

    (void)M11_Render_SetV2Filters(0, 0, 0, 100, 0, 0,
                                  0, 0, 0, 0, 0);
    record(&stats,
           "TITLE_SWOOSH_HANDOFF_V1_PALETTE_RESTORED",
           M11_Render_PresentIndexedWithSpecialPalette(
               framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT,
               VGA_PALETTE_PC34_SPECIAL_TITLE) == M11_RENDER_OK,
           "disabling V2.0 restores the source special-palette route");
    presented = M11_Render_GetPresentedRGBA(&presentedW, &presentedH);
    if (presented && expected && sampleX >= 0 && sampleY >= 0 &&
        sampleX < presentedW && sampleY < presentedH) {
        const unsigned char* px = presented +
            ((sampleY * presentedW + sampleX) * 4);
        record(&stats,
               "TITLE_SWOOSH_HANDOFF_V1_PALETTE_EXACT",
               px[0] == expected[0] && px[1] == expected[1] &&
                   px[2] == expected[2],
               "unfiltered C001 sample is byte-identical to the original palette");
    } else {
        record(&stats, "TITLE_SWOOSH_HANDOFF_V1_PALETTE_EXACT", 0,
               "restored C001 sample unavailable");
    }
    verify_original_startup_palette_frame(
        &stats, "ENTRANCE", entranceGraphic, framebuffer,
        VGA_PALETTE_PC34_SPECIAL_ENTRANCE);
    verify_original_startup_palette_frame(
        &stats, "CREDITS", creditsGraphic, framebuffer,
        VGA_PALETTE_PC34_SPECIAL_CREDITS);
    M11_Render_SetV2PresentationActive(0);

    printf("summary=%d passed %d failed total=%d\n",
           stats.passed, stats.failed, stats.total);

    free(poisonRgba);
    M11_AssetLoader_Shutdown(&loader);
    M11_Render_Shutdown();
    SDL_Quit();
    return stats.failed == 0 ? 0 : 1;
}
