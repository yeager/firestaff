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

#include "title_frontend_v1.h"
#include "vga_palette_pc34_compat.h"

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
    int presentsPalette = -1;
    int fallbackPresentsPalette = -1;
    int zoomPalette = -1;
    int fallbackZoomPalette = -1;
    int fallbackLatePalette = -1;

    memset(&presentsStep, 0, sizeof(presentsStep));
    memset(&zoomStep, 0, sizeof(zoomStep));

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
}

int main(void) {
    check_selection_contract();
    check_palette_cross_source_contract();

    if (g_fail) {
        printf("summary=%d passed %d failed\n", g_pass, g_fail);
        return 1;
    }
    printf("ok: DM1 V1 TITLE C001 fallback gate passed (%d checks)\n", g_pass);
    return 0;
}
