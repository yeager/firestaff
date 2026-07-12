/*
 * DM1 V1 TITLE C001-vs-TITLE.DAT fallback selection gate.
 *
 * ReDMCSB TITLE.C F0437 PC/F20 source-lock:
 *   - lines 309-310 load/decompress C001_GRAPHIC_TITLE.
 *   - lines 319-324 blit PRESENTS from C001 source y=137.
 *   - lines 333-340 prepare the 320x57 MASTER/STRIKES BACK strip from
 *     C001 source y=80.
 *   - lines 340-360 build the 18 C001 zoom bitmaps.
 *   - lines 362-367 switch to C13_DUNGEON + C14_MASTER before the zoom.
 *
 * The decoded TITLE.DAT bank is Firestaff's visible fallback when GRAPHICS.DAT
 * C001 is absent or too small.  It must not replace a usable C001 runtime path.
 * pass842 still owns deep TITLE.DAT frame/palette regression coverage, pass897
 * still owns SDL/Metal special-palette readback, and the SWSH handoff probe
 * still owns RGBA->indexed texture-state reset.  This gate only pins the source
 * selection seam those probes intentionally do not own.
 */

#include "asset_loader_m11.h"
#include "title_frontend_v1.h"
#include "vga_palette_pc34_compat.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int g_pass = 0;
static int g_fail = 0;

static const char* runtime_source_name(V1_TitleFrontendRuntimeSource source) {
    switch (source) {
        case V1_TITLE_FRONTEND_RUNTIME_SOURCE_GRAPHICS_C001: return "GRAPHICS_C001";
        case V1_TITLE_FRONTEND_RUNTIME_SOURCE_TITLE_DAT_FALLBACK: return "TITLE_DAT_FALLBACK";
        case V1_TITLE_FRONTEND_RUNTIME_SOURCE_SKIP: return "SKIP";
    }
    return "UNKNOWN";
}

static void expect_i(const char* label, int got, int want) {
    if (got == want) {
        ++g_pass;
    } else {
        ++g_fail;
        printf("FAIL %s: got %d want %d\n", label, got, want);
    }
}

static void expect_u(const char* label, unsigned int got, unsigned int want) {
    if (got == want) {
        ++g_pass;
    } else {
        ++g_fail;
        printf("FAIL %s: got %u want %u\n", label, got, want);
    }
}

static void expect_truth(const char* label, int ok) {
    if (ok) {
        ++g_pass;
    } else {
        ++g_fail;
        printf("FAIL %s\n", label);
    }
}

static void expect_source(const char* label,
                          V1_TitleFrontendRuntimeSourceDecision decision,
                          V1_TitleFrontendRuntimeSource want) {
    if (decision.source == want) {
        ++g_pass;
    } else {
        ++g_fail;
        printf("FAIL %s: got %s want %s\n",
               label,
               runtime_source_name(decision.source),
               runtime_source_name(want));
    }
}

static void check_selection_contract(void) {
    V1_TitleFrontendRuntimeSourceDecision c001Only =
        V1_TitleFrontend_SelectRuntimeSource(1, 320u, 175u, 0);
    V1_TitleFrontendRuntimeSourceDecision c001BeatsFallback =
        V1_TitleFrontend_SelectRuntimeSource(1, 320u, 175u, 1);
    V1_TitleFrontendRuntimeSourceDecision widthTooSmall =
        V1_TitleFrontend_SelectRuntimeSource(1, 319u, 175u, 1);
    V1_TitleFrontendRuntimeSourceDecision heightTooSmall =
        V1_TitleFrontend_SelectRuntimeSource(1, 320u, 174u, 1);
    V1_TitleFrontendRuntimeSourceDecision missingC001 =
        V1_TitleFrontend_SelectRuntimeSource(0, 0u, 0u, 1);
    V1_TitleFrontendRuntimeSourceDecision noCandidateDespiteDims =
        V1_TitleFrontend_SelectRuntimeSource(0, 320u, 175u, 0);
    V1_TitleFrontendRuntimeSourceDecision nothingPlayable =
        V1_TitleFrontend_SelectRuntimeSource(1, 319u, 174u, 0);

    expect_source("usable C001 without TITLE.DAT selects GRAPHICS.DAT C001",
                  c001Only,
                  V1_TITLE_FRONTEND_RUNTIME_SOURCE_GRAPHICS_C001);
    expect_i("usable C001 flag is set", c001Only.graphicsC001Usable, 1);
    expect_i("TITLE.DAT fallback flag is clear", c001Only.titleDatFallbackUsable, 0);

    expect_source("usable C001 wins even when TITLE.DAT exists",
                  c001BeatsFallback,
                  V1_TITLE_FRONTEND_RUNTIME_SOURCE_GRAPHICS_C001);
    expect_i("TITLE.DAT availability is still recorded",
             c001BeatsFallback.titleDatFallbackUsable,
             1);

    expect_source("319-wide C001 falls back to TITLE.DAT",
                  widthTooSmall,
                  V1_TITLE_FRONTEND_RUNTIME_SOURCE_TITLE_DAT_FALLBACK);
    expect_i("319-wide C001 is not usable", widthTooSmall.graphicsC001Usable, 0);

    expect_source("174-high C001 falls back to TITLE.DAT",
                  heightTooSmall,
                  V1_TITLE_FRONTEND_RUNTIME_SOURCE_TITLE_DAT_FALLBACK);
    expect_i("174-high C001 is not usable", heightTooSmall.graphicsC001Usable, 0);

    expect_source("missing C001 with TITLE.DAT selects fallback",
                  missingC001,
                  V1_TITLE_FRONTEND_RUNTIME_SOURCE_TITLE_DAT_FALLBACK);
    expect_i("missing C001 is not usable", missingC001.graphicsC001Usable, 0);

    expect_source("candidate flag is authoritative even with source dimensions",
                  noCandidateDespiteDims,
                  V1_TITLE_FRONTEND_RUNTIME_SOURCE_SKIP);
    expect_source("invalid C001 without TITLE.DAT skips title",
                  nothingPlayable,
                  V1_TITLE_FRONTEND_RUNTIME_SOURCE_SKIP);

    expect_truth("source evidence cites TITLE.C",
                 c001BeatsFallback.sourceLineEvidence &&
                 strstr(c001BeatsFallback.sourceLineEvidence, "TITLE.C") != 0);
    expect_truth("source evidence cites F0437",
                 c001BeatsFallback.sourceLineEvidence &&
                 strstr(c001BeatsFallback.sourceLineEvidence, "F0437") != 0);
}

static void check_palette_cross_source_contract(void) {
    V1_TitleFrontendSourceAnimationStep presentsStep;
    V1_TitleFrontendSourceAnimationStep zoomStep;
    V1_TitleFrontendSourceAnimationStep strikesStep;
    V1_TitleFrontendC001BlitPlan presentsPlan;
    V1_TitleFrontendC001BlitPlan zoomPlan;
    V1_TitleFrontendC001BlitPlan strikesPlan;
    V1_TitleFrontendC001BlitPlan waitPlan;
    int presentsPalette = -1;
    int fallbackPresentsPalette = -1;
    int zoomPalette = -1;
    int fallbackZoomPalette = -1;
    int fallbackLatePalette = -1;

    memset(&presentsStep, 0, sizeof(presentsStep));
    memset(&zoomStep, 0, sizeof(zoomStep));
    memset(&strikesStep, 0, sizeof(strikesStep));
    memset(&presentsPlan, 0, sizeof(presentsPlan));
    memset(&zoomPlan, 0, sizeof(zoomPlan));
    memset(&strikesPlan, 0, sizeof(strikesPlan));
    memset(&waitPlan, 0, sizeof(waitPlan));

    expect_i("C001 source step 1 exists",
             V1_TitleFrontend_GetSourceAnimationStep(1u, &presentsStep),
             1);
    expect_u("C001 source step 1 is PRESENTS",
             (unsigned int)presentsStep.kind,
             (unsigned int)V1_TITLE_FRONTEND_SOURCE_EVENT_PRESENTS);
    expect_i("C001 source step 19 exists",
             V1_TitleFrontend_GetSourceAnimationStep(19u, &zoomStep),
             1);
    expect_u("C001 source step 19 is ZOOM",
             (unsigned int)zoomStep.kind,
             (unsigned int)V1_TITLE_FRONTEND_SOURCE_EVENT_ZOOM_BLIT);
    expect_i("C001 source step 22 exists",
             V1_TitleFrontend_GetSourceAnimationStep(22u, &strikesStep),
             1);
    expect_u("C001 source step 22 is STRIKES BACK",
             (unsigned int)strikesStep.kind,
             (unsigned int)V1_TITLE_FRONTEND_SOURCE_EVENT_MASTER_STRIKES_BACK_BLIT);

    expect_i("C001 PRESENTS palette resolves",
             V1_TitleFrontend_GetStepPalette(presentsStep.kind, &presentsPalette),
             1);
    expect_i("fallback frame 1 palette resolves",
             V1_TitleFrontend_GetFallbackFramePalette(1u, &fallbackPresentsPalette),
             1);
    expect_i("C001 ZOOM palette resolves",
             V1_TitleFrontend_GetStepPalette(zoomStep.kind, &zoomPalette),
             1);
    expect_i("fallback frame 2 palette resolves",
             V1_TitleFrontend_GetFallbackFramePalette(2u, &fallbackZoomPalette),
             1);
    expect_i("fallback frame 53 palette resolves",
             V1_TitleFrontend_GetFallbackFramePalette(V1_TITLE_DAT_FRAME_MAX,
                                                      &fallbackLatePalette),
             1);

    expect_i("C001 PRESENTS and TITLE.DAT frame 1 share C12_PRESENTS",
             presentsPalette,
             fallbackPresentsPalette);
    expect_i("C001 ZOOM and TITLE.DAT frame 2 share C13+C14",
             zoomPalette,
             fallbackZoomPalette);
    expect_i("TITLE.DAT late frame stays on C13+C14",
             fallbackLatePalette,
             zoomPalette);
    expect_i("PRESENTS uses the special PRESENTS palette",
             presentsPalette,
             VGA_PALETTE_PC34_SPECIAL_TITLE_PRESENTS);
    expect_i("ZOOM uses the special TITLE palette",
             zoomPalette,
             VGA_PALETTE_PC34_SPECIAL_TITLE);
    expect_truth("PRESENTS and ZOOM palettes remain distinct",
                 presentsPalette != zoomPalette);

    expect_i("PRESENTS C001 blit plan resolves",
             V1_TitleFrontend_GetC001BlitPlanForStep(&presentsStep, &presentsPlan),
             1);
    expect_u("PRESENTS blit kind",
             (unsigned int)presentsPlan.kind,
             (unsigned int)V1_TITLE_FRONTEND_C001_BLIT_REGION);
    expect_u("PRESENTS source y",
             presentsPlan.srcY,
             137u);
    expect_u("PRESENTS destination y",
             presentsPlan.dstY,
             90u);
    expect_u("PRESENTS height",
             presentsPlan.srcH,
             16u);
    expect_i("PRESENTS clears first",
             presentsPlan.clearBeforeBlit,
             1);

    expect_i("ZOOM C001 blit plan resolves",
             V1_TitleFrontend_GetC001BlitPlanForStep(&zoomStep, &zoomPlan),
             1);
    expect_u("ZOOM blit kind",
             (unsigned int)zoomPlan.kind,
             (unsigned int)V1_TITLE_FRONTEND_C001_BLIT_SCALED_REGION);
    expect_u("ZOOM source x is full C001 title origin",
             zoomPlan.srcX,
             0u);
    expect_u("ZOOM source y is full C001 title origin",
             zoomPlan.srcY,
             0u);
    expect_u("ZOOM source width is full C001 title width",
             zoomPlan.srcW,
             320u);
    expect_u("ZOOM source height is full C001 title height",
             zoomPlan.srcH,
             80u);
    expect_u("ZOOM destination x is source-centred box",
             zoomPlan.dstX,
             zoomStep.x);
    expect_u("ZOOM destination y is source-centred box",
             zoomPlan.dstY,
             zoomStep.y);
    expect_u("ZOOM destination width is source-centred box width",
             zoomPlan.dstW,
             zoomStep.width);
    expect_u("ZOOM destination height is source-centred box height",
             zoomPlan.dstH,
             zoomStep.height);
    expect_i("ZOOM clears first",
             zoomPlan.clearBeforeBlit,
             1);

    expect_i("STRIKES BACK C001 blit plan resolves",
             V1_TitleFrontend_GetC001BlitPlanForStep(&strikesStep, &strikesPlan),
             1);
    expect_u("STRIKES BACK blit kind",
             (unsigned int)strikesPlan.kind,
             (unsigned int)V1_TITLE_FRONTEND_C001_BLIT_REGION);
    expect_u("STRIKES BACK source y",
             strikesPlan.srcY,
             80u);
    expect_u("STRIKES BACK destination y",
             strikesPlan.dstY,
             118u);
    expect_i("STRIKES BACK uses black transparency",
             strikesPlan.transparentColor,
             0);

    expect_i("wait step C001 blit plan resolves",
             V1_TitleFrontend_GetC001BlitPlanForStep(&((V1_TitleFrontendSourceAnimationStep){
                 .kind = V1_TITLE_FRONTEND_SOURCE_EVENT_POST_ZOOM_VBLANK
             }), &waitPlan),
             1);
    expect_u("wait step has no blit",
             (unsigned int)waitPlan.kind,
             (unsigned int)V1_TITLE_FRONTEND_C001_BLIT_NONE);
}

static unsigned int count_non_black(const unsigned char* pixels,
                                    unsigned int width,
                                    unsigned int x,
                                    unsigned int y,
                                    unsigned int region_width,
                                    unsigned int region_height) {
    unsigned int count = 0u;
    unsigned int row;
    unsigned int col;

    for (row = 0u; row < region_height; ++row) {
        for (col = 0u; col < region_width; ++col) {
            if (pixels[(y + row) * width + x + col] != 0u) {
                ++count;
            }
        }
    }
    return count;
}

static void check_real_pc34_c001(const char* graphics_path) {
    M11_AssetLoader loader;
    const M11_AssetSlot* c001;
    unsigned char framebuffer[320u * 200u];
    V1_TitleFrontendSourceAnimationStep first_zoom;
    V1_TitleFrontendSourceAnimationStep last_zoom;
    V1_TitleFrontendSourceAnimationStep strikes_back;
    V1_TitleFrontendC001BlitPlan plan;

    if (!graphics_path || !graphics_path[0]) {
        printf("SKIP real C001 fixture: pass hash-verified GRAPHICS.DAT path\n");
        return;
    }
    memset(&loader, 0, sizeof(loader));
    expect_i("real C001: loader opens supplied PC34 GRAPHICS.DAT",
             M11_AssetLoader_Init(&loader, graphics_path), 1);
    if (!M11_AssetLoader_IsReady(&loader)) {
        return;
    }
    c001 = M11_AssetLoader_Load(&loader, 1u);
    expect_truth("real C001: graphic 1 decodes", c001 != NULL);
    if (!c001) {
        M11_AssetLoader_Shutdown(&loader);
        return;
    }
    expect_u("real C001: width", c001->width, 320u);
    expect_u("real C001: height", c001->height, 200u);
    expect_truth("real C001: PRESENTS source strip has original pixels",
                 count_non_black(c001->pixels, c001->width,
                                 0u, 137u, 320u, 16u) > 0u);
    expect_truth("real C001: DUNGEON MASTER source strip has original pixels",
                 count_non_black(c001->pixels, c001->width,
                                 0u, 0u, 320u, 80u) > 0u);
    expect_truth("real C001: MASTER STRIKES BACK source strip has original pixels",
                 count_non_black(c001->pixels, c001->width,
                                 0u, 80u, 320u, 57u) > 0u);

    expect_i("real C001: first zoom step resolves",
             V1_TitleFrontend_GetSourceAnimationStep(2u, &first_zoom), 1);
    expect_i("real C001: last zoom step resolves",
             V1_TitleFrontend_GetSourceAnimationStep(19u, &last_zoom), 1);
    expect_i("real C001: strikes-back step resolves",
             V1_TitleFrontend_GetSourceAnimationStep(22u, &strikes_back), 1);

    memset(framebuffer, 0, sizeof(framebuffer));
    expect_i("real C001: first zoom plan resolves",
             V1_TitleFrontend_GetC001BlitPlanForStep(&first_zoom, &plan), 1);
    M11_AssetLoader_BlitSubRectScaled(c001, framebuffer, 320, 200,
                                      (int)plan.dstX, (int)plan.dstY,
                                      (int)plan.dstW, (int)plan.dstH,
                                      (int)plan.srcX, (int)plan.srcY,
                                      (int)plan.srcW, (int)plan.srcH,
                                      plan.transparentColor);
    expect_truth("real C001: first source-ordered zoom blit is visible",
                 count_non_black(framebuffer, 320u, plan.dstX, plan.dstY,
                                 plan.dstW, plan.dstH) > 0u);

    memset(framebuffer, 0, sizeof(framebuffer));
    expect_i("real C001: final zoom plan resolves",
             V1_TitleFrontend_GetC001BlitPlanForStep(&last_zoom, &plan), 1);
    M11_AssetLoader_BlitSubRectScaled(c001, framebuffer, 320, 200,
                                      (int)plan.dstX, (int)plan.dstY,
                                      (int)plan.dstW, (int)plan.dstH,
                                      (int)plan.srcX, (int)plan.srcY,
                                      (int)plan.srcW, (int)plan.srcH,
                                      plan.transparentColor);
    expect_truth("real C001: final source-ordered zoom blit is visible",
                 count_non_black(framebuffer, 320u, plan.dstX, plan.dstY,
                                 plan.dstW, plan.dstH) > 0u);

    memset(framebuffer, 0, sizeof(framebuffer));
    expect_i("real C001: strikes-back plan resolves",
             V1_TitleFrontend_GetC001BlitPlanForStep(&strikes_back, &plan), 1);
    M11_AssetLoader_BlitRegion(c001, (int)plan.srcX, (int)plan.srcY,
                               (int)plan.srcW, (int)plan.srcH,
                               framebuffer, 320, 200,
                               (int)plan.dstX, (int)plan.dstY,
                               plan.transparentColor);
    expect_truth("real C001: strikes-back source blit is visible",
                 count_non_black(framebuffer, 320u, plan.dstX, plan.dstY,
                                 plan.dstW, plan.dstH) > 0u);
    M11_AssetLoader_Shutdown(&loader);
}

int main(int argc, char** argv) {
    check_selection_contract();
    check_palette_cross_source_contract();
    check_real_pc34_c001(argc > 1 ? argv[1] : getenv("FIRESTAFF_DM1_GRAPHICS_DAT"));

    if (g_fail) {
        printf("summary=%d passed %d failed\n", g_pass, g_fail);
        return 1;
    }
    printf("ok: DM1 V1 TITLE C001 fallback gate passed (%d checks)\n", g_pass);
    return 0;
}
