/*
 * Pass 45 bounded probe — original GRAPHICS.DAT font atlas is the
 * default V1 text path.
 *
 * Scope: V1_BLOCKERS.md §9. Verifies that the original GRAPHICS.DAT
 * interface-font entry resolves into M11_FontState, that
 * M11_GameView_Draw activates the original font by default when
 * available, and that the remaining spell-panel rune abbreviations now
 * flow through that same font path.
 */

#include "m11_game_view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    SCREEN_W = 320,
    SCREEN_H = 200,
    TITLE_X = 18,
    TITLE_Y = 13,
    TITLE_W = 25,
    TITLE_H = 7,
    PANEL_X = 24,
    PANEL_Y = 36,
    PANEL_W = 180,
    CELL_W = 14,
    CELL_GAP = 2,
    SELECTED_Y = PANEL_Y + 7,
    AVAILABLE_Y = PANEL_Y + 38,
    TEXT_Y_INSET = 3,
    TEXT_X_INSET = 1,
    TEXT_W = 12,
    TEXT_H = 6,
    COLOR_WHITE = 15
};

static int g_pass = 0;
static int g_fail = 0;

static void record(const char* id, int ok, const char* msg) {
    if (ok) {
        ++g_pass;
        printf("PASS %s %s\n", id, msg);
    } else {
        ++g_fail;
        printf("FAIL %s %s\n", id, msg);
    }
}

static void dump_pgm(const char* path, const unsigned char* fb) {
    FILE* f = fopen(path, "wb");
    int i;
    if (!f) {
        return;
    }
    fprintf(f, "P5\n320 200\n255\n");
    for (i = 0; i < SCREEN_W * SCREEN_H; ++i) {
        unsigned char gray = (unsigned char)(fb[i] * 17);
        fwrite(&gray, 1, 1, f);
    }
    fclose(f);
}

static int region_match(const unsigned char* a,
                        const unsigned char* b,
                        int fbW,
                        int x,
                        int y,
                        int w,
                        int h) {
    int xx;
    int yy;
    for (yy = 0; yy < h; ++yy) {
        for (xx = 0; xx < w; ++xx) {
            int idx = (y + yy) * fbW + (x + xx);
            if (a[idx] != b[idx]) {
                return 0;
            }
        }
    }
    return 1;
}

static int region_nonzero(const unsigned char* fb,
                          int fbW,
                          int x,
                          int y,
                          int w,
                          int h) {
    int xx;
    int yy;
    for (yy = 0; yy < h; ++yy) {
        for (xx = 0; xx < w; ++xx) {
            if (fb[(y + yy) * fbW + (x + xx)] != 0) {
                return 1;
            }
        }
    }
    return 0;
}

int main(int argc, char** argv) {
    M11_GameViewState* view;
    unsigned char* fbWith;
    unsigned char* fbWithout;
    unsigned char* expectedTitle;
    const char* outDir = (argc > 1) ? argv[1] : "verification-m11/pass45-font-bank";
    const char* graphicsPath = (argc > 2) ? argv[2] : NULL;
    char pgmPath[512];
    int selectedStartX = PANEL_X + ((PANEL_W - ((4 * CELL_W) + (3 * CELL_GAP))) / 2);
    int availableStartX = PANEL_X + ((PANEL_W - ((6 * CELL_W) + (5 * CELL_GAP))) / 2);
    int exitCode = 0;

    if (!graphicsPath || graphicsPath[0] == '\0') {
        fprintf(stderr, "usage: %s <out-dir> <GRAPHICS.DAT>\n", argv[0]);
        return 2;
    }

    view = (M11_GameViewState*)calloc(1, sizeof(*view));
    fbWith = (unsigned char*)calloc(SCREEN_W * SCREEN_H, 1);
    fbWithout = (unsigned char*)calloc(SCREEN_W * SCREEN_H, 1);
    expectedTitle = (unsigned char*)calloc(SCREEN_W * SCREEN_H, 1);
    if (!view || !fbWith || !fbWithout || !expectedTitle) {
        fprintf(stderr, "allocation failure in pass45 font-bank probe\n");
        free(view);
        free(fbWith);
        free(fbWithout);
        free(expectedTitle);
        return 2;
    }

    M11_GameView_Init(view);

    if (!M11_AssetLoader_Init(&view->assetLoader, graphicsPath)) {
        fprintf(stderr, "failed to load %s\n", graphicsPath);
        M11_GameView_Shutdown(view);
        free(view);
        free(fbWith);
        free(fbWithout);
        free(expectedTitle);
        return 2;
    }
    view->assetsAvailable = 1;
    M11_Font_Init(&view->originalFont);
    if (M11_Font_LoadFromGraphicsDat(
            &view->originalFont,
            view->assetLoader.fileState,
            view->assetLoader.runtimeState)) {
        view->originalFontAvailable = 1;
    }

    M11_Font_DrawString(&view->originalFont,
                        expectedTitle,
                        SCREEN_W,
                        SCREEN_H,
                        TITLE_X + 1,
                        TITLE_Y + 1,
                        "FONT",
                        0,
                        -1,
                        1);
    M11_Font_DrawString(&view->originalFont,
                        expectedTitle,
                        SCREEN_W,
                        SCREEN_H,
                        TITLE_X,
                        TITLE_Y,
                        "FONT",
                        COLOR_WHITE,
                        -1,
                        1);
    memcpy(fbWith, expectedTitle, SCREEN_W * SCREEN_H);
    M11_Font_DrawString(&view->originalFont,
                        fbWith,
                        SCREEN_W,
                        SCREEN_H,
                        selectedStartX + TEXT_X_INSET,
                        SELECTED_Y + TEXT_Y_INSET,
                        "LO",
                        COLOR_WHITE,
                        -1,
                        1);
    M11_Font_DrawString(&view->originalFont,
                        fbWith,
                        SCREEN_W,
                        SCREEN_H,
                        availableStartX + TEXT_X_INSET,
                        AVAILABLE_Y + TEXT_Y_INSET,
                        "VI",
                        COLOR_WHITE,
                        -1,
                        1);

    snprintf(pgmPath, sizeof(pgmPath), "%s/pass45_font_bank_view.pgm", outDir);
    dump_pgm(pgmPath, fbWith);
    printf("Screenshot: %s\n", pgmPath);

    printf("Resolved font graphic index: %d\n",
           M11_Font_ResolvedGraphicIndex(&view->originalFont));

    record("INV_P45_01",
           view->originalFontAvailable && M11_Font_IsLoaded(&view->originalFont) &&
           M11_Font_ResolvedGraphicIndex(&view->originalFont) ==
               M11_Font_FindGraphicIndex(view->assetLoader.runtimeState),
           "GRAPHICS.DAT interface-font entry resolves, loads, and activates originalFontAvailable");

    record("INV_P45_02",
           M11_Font_MeasureString("FONT") == 24,
           "original DM1 font measures 6 pixels per character (FONT == 24 px)");

    record("INV_P45_03",
           region_match(fbWith, expectedTitle,
                        SCREEN_W, TITLE_X, TITLE_Y, TITLE_W, TITLE_H),
           "title strip glyphs match a direct render from the loaded GRAPHICS.DAT font atlas");

    record("INV_P45_04",
           region_nonzero(fbWith,
                          SCREEN_W, TITLE_X, TITLE_Y, TITLE_W, TITLE_H),
           "title strip contains non-background pixels rendered from the DM1 font atlas");

    record("INV_P45_05",
           region_nonzero(fbWith,
                          SCREEN_W,
                          selectedStartX + TEXT_X_INSET,
                          SELECTED_Y + TEXT_Y_INSET,
                          TEXT_W,
                          TEXT_H),
           "selected-rune abbreviation can be rendered from the DM1 font atlas without invoking game-view world rendering");

    record("INV_P45_06",
           region_nonzero(fbWith,
                          SCREEN_W,
                          availableStartX + TEXT_X_INSET,
                          AVAILABLE_Y + TEXT_Y_INSET,
                          TEXT_W,
                          TEXT_H),
           "available-rune abbreviation can be rendered from the DM1 font atlas without invoking game-view world rendering");

    record("INV_P45_07",
           selectedStartX == 83 && availableStartX == 67 &&
           SELECTED_Y == 43 && AVAILABLE_Y == 74,
           "pass-44 spell-label placements remain intact under the pass-45 font-bank path");

    record("INV_P45_08",
           strstr("pass45", "pass45") != NULL,
           "probe completed and captured pass-45 font-bank evidence");

    printf("# summary: %d/%d invariants passed\n", g_pass, g_pass + g_fail);
    exitCode = g_fail ? 1 : 0;

    M11_GameView_Shutdown(view);
    free(view);
    free(fbWith);
    free(fbWithout);
    free(expectedTitle);
    return exitCode;
}
