/*
 * DM1 V1 fullscreen map font-scale fit probe.
 *
 * Source evidence:
 *   The fullscreen map overlay (M key) is a Firestaff diagnostic
 *   surface that draws "LEVEL N" at the top and "PRESS M" at the
 *   bottom using g_text_shadow (scale=1, LIGHT_GRAY) and g_text_small
 *   (scale=1, DARK_GRAY).  Both text styles flow through m11_draw_text
 *   which applies g_m11_font_scale_override when nonzero.
 *
 *   m11_draw_fullscreen_map (src/engine/m11_game_view.c:52691) fills
 *   the entire 320x200 framebuffer with black, then draws within
 *   panelX=8, panelY=6, panelW=304, panelH=188.  Title text at
 *   (panelX+6, panelY+4) = (14, 10).  Footer text at
 *   (panelX+6, panelY+panelH-12) = (14, 182).
 *
 * What this probe locks:
 *
 *   1. The title "LEVEL N" text is drawn at scale 1, 2, 3 and
 *      the LIGHT_GRAY pixel count in the title region grows
 *      monotonically with scale.
 *
 *   2. The footer "PRESS M" text is drawn at every scale and
 *      the DARK_GRAY pixel count in the footer region is nonzero
 *      at each scale (count may not grow monotonically because
 *      the footer is pushed up at higher scales, overlapping
 *      map grid tiles).
 *
 *   3. No text bleeds outside the panel border (no non-black
 *      pixels in the margins at scale 3).
 *
 * This probe skips safely when the user-supplied data dir does
 * not contain hash-verified DM1 PC 3.4 data files.
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"
#include "ui_scale_m11.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    PROBE_FB_W = 320,
    PROBE_FB_H = 200
};

enum {
    PROBE_TITLE_X = 14,
    PROBE_TITLE_Y = 10,
    PROBE_TITLE_W = 200,
    PROBE_TITLE_H = 14,
    PROBE_FOOTER_X = 14,
    PROBE_FOOTER_Y = 182,
    PROBE_FOOTER_W = 200,
    PROBE_FOOTER_H = 14,
    PROBE_PANEL_X = 8,
    PROBE_PANEL_Y = 6,
    PROBE_PANEL_W = 304,
    PROBE_PANEL_H = 188,
    PROBE_LIGHT_GRAY = 2,
    PROBE_DARK_GRAY = 12,
    PROBE_BLACK = 0
};

static int g_pass;
static int g_fail;

static int file_exists(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return 0;
    fclose(f);
    return 1;
}

static const char* narrow_dm1_data_dir(const char* dataDir,
                                       char* out,
                                       size_t outSize) {
    char graphicsPath[512];
    char dungeonPath[512];
    if (!dataDir || !out || outSize == 0U) return dataDir;
    snprintf(graphicsPath, sizeof(graphicsPath), "%s/dm1/GRAPHICS.DAT", dataDir);
    snprintf(dungeonPath, sizeof(dungeonPath), "%s/dm1/DUNGEON.DAT", dataDir);
    if (file_exists(graphicsPath) && file_exists(dungeonPath)) {
        snprintf(out, outSize, "%s/dm1", dataDir);
        return out;
    }
    return dataDir;
}

static void check_true(const char* label, int cond) {
    if (cond) {
        ++g_pass;
        printf("PASS %s\n", label);
    } else {
        ++g_fail;
        printf("FAIL %s\n", label);
    }
}

static void check_ge(const char* label, int got, int wantAtLeast) {
    char buf[128];
    snprintf(buf, sizeof(buf), "%s got=%d want>=%d", label, got, wantAtLeast);
    check_true(buf, got >= wantAtLeast);
}

static void check_eq(const char* label, int got, int want) {
    char buf[128];
    snprintf(buf, sizeof(buf), "%s got=%d want=%d", label, got, want);
    check_true(buf, got == want);
}

static size_t count_color(const unsigned char* fb,
                          int x, int y, int w, int h,
                          unsigned char color) {
    size_t count = 0;
    int yy, xx;
    if (!fb) return 0;
    for (yy = y; yy < y + h; ++yy) {
        if (yy < 0 || yy >= PROBE_FB_H) continue;
        for (xx = x; xx < x + w; ++xx) {
            if (xx < 0 || xx >= PROBE_FB_W) continue;
            if (M11_FB_DECODE_INDEX(fb[yy * PROBE_FB_W + xx]) == color)
                ++count;
        }
    }
    return count;
}

static size_t count_nonblack(const unsigned char* fb,
                             int x, int y, int w, int h) {
    size_t count = 0;
    int yy, xx;
    if (!fb) return 0;
    for (yy = y; yy < y + h; ++yy) {
        if (yy < 0 || yy >= PROBE_FB_H) continue;
        for (xx = x; xx < x + w; ++xx) {
            if (xx < 0 || xx >= PROBE_FB_W) continue;
            if (M11_FB_DECODE_INDEX(fb[yy * PROBE_FB_W + xx]) != PROBE_BLACK)
                ++count;
        }
    }
    return count;
}

static void render_map_at_scale(M11_GameViewState* game,
                                unsigned char* fb,
                                size_t fbBytes,
                                int fontScale) {
    game->mapOverlayActive = 1;
    game->showDebugHUD = 1;
    game->fontScale = fontScale;
    memset(fb, 0, fbBytes);
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
}

int main(int argc, char** argv) {
    const char* root = argc > 1 ? argv[1] : NULL;
    const char* dataDir;
    char narrowed[512];
    M12_StartupMenuState menu;
    M11_GameViewState game;
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    int titlePixels1, titlePixels2, titlePixels3;
    int footerPixels1, footerPixels2, footerPixels3;
    int marginLeft, marginRight, marginTop, marginBottom;

    if (!root) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = narrow_dm1_data_dir(root, narrowed, sizeof(narrowed));

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        printf("SKIP could not open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 0;
    }

    printf("probe=firestaff_dm1_v1_fullscreen_map_font_scale_fit_runtime_probe\n");
    printf("dataDir=%s\n", dataDir);

    /* (1) Title "LEVEL N" text pixel count grows with fontScale */
    render_map_at_scale(&game, fb, sizeof(fb), 1);
    titlePixels1 = (int)count_color(fb, PROBE_TITLE_X, PROBE_TITLE_Y,
                                     PROBE_TITLE_W, PROBE_TITLE_H,
                                     PROBE_LIGHT_GRAY);

    render_map_at_scale(&game, fb, sizeof(fb), 2);
    titlePixels2 = (int)count_color(fb, PROBE_TITLE_X, PROBE_TITLE_Y,
                                     PROBE_TITLE_W, PROBE_TITLE_H,
                                     PROBE_LIGHT_GRAY);

    render_map_at_scale(&game, fb, sizeof(fb), 3);
    titlePixels3 = (int)count_color(fb, PROBE_TITLE_X, PROBE_TITLE_Y,
                                     PROBE_TITLE_W, PROBE_TITLE_H,
                                     PROBE_LIGHT_GRAY);

    check_ge("title LIGHT_GRAY pixels at scale 1", titlePixels1, 4);
    check_ge("title LIGHT_GRAY pixels at scale 2", titlePixels2, 4);
    check_ge("title LIGHT_GRAY pixels at scale 3", titlePixels3, 4);
    check_true("title pixel count grows (scale 2 > scale 1)",
               titlePixels2 > titlePixels1);
    check_true("title pixel count grows (scale 3 > scale 2)",
               titlePixels3 > titlePixels2);

    /* (2) Footer "PRESS M" text pixel count grows with fontScale */
    render_map_at_scale(&game, fb, sizeof(fb), 1);
    footerPixels1 = (int)count_color(fb, PROBE_FOOTER_X, PROBE_FOOTER_Y,
                                      PROBE_FOOTER_W, PROBE_FOOTER_H,
                                      PROBE_DARK_GRAY);

    render_map_at_scale(&game, fb, sizeof(fb), 2);
    footerPixels2 = (int)count_color(fb, PROBE_FOOTER_X, PROBE_FOOTER_Y,
                                      PROBE_FOOTER_W, PROBE_FOOTER_H,
                                      PROBE_DARK_GRAY);

    render_map_at_scale(&game, fb, sizeof(fb), 3);
    footerPixels3 = (int)count_color(fb, PROBE_FOOTER_X, PROBE_FOOTER_Y,
                                      PROBE_FOOTER_W, PROBE_FOOTER_H,
                                      PROBE_DARK_GRAY);

    check_ge("footer DARK_GRAY pixels at scale 1", footerPixels1, 4);
    check_ge("footer DARK_GRAY pixels at scale 2", footerPixels2, 4);
    check_ge("footer DARK_GRAY pixels at scale 3", footerPixels3, 4);

    /* (3) At scale 3, no text bleeds outside the panel margins.
     * The map fills black outside the panel, so any non-black
     * pixels in the margins would be text overflow. */
    render_map_at_scale(&game, fb, sizeof(fb), 3);
    marginLeft = (int)count_nonblack(fb, 0, 0, PROBE_PANEL_X, PROBE_FB_H);
    marginRight = (int)count_nonblack(fb,
                                       PROBE_PANEL_X + PROBE_PANEL_W, 0,
                                       PROBE_FB_W - PROBE_PANEL_X - PROBE_PANEL_W,
                                       PROBE_FB_H);
    marginTop = (int)count_nonblack(fb, 0, 0, PROBE_FB_W, PROBE_PANEL_Y);
    marginBottom = (int)count_nonblack(fb, 0,
                                        PROBE_PANEL_Y + PROBE_PANEL_H,
                                        PROBE_FB_W,
                                        PROBE_FB_H - PROBE_PANEL_Y - PROBE_PANEL_H);
    check_eq("no text bleed into left margin at scale 3", marginLeft, 0);
    check_eq("no text bleed into right margin at scale 3", marginRight, 0);
    check_eq("no text bleed into top margin at scale 3", marginTop, 0);
    check_eq("no text bleed into bottom margin at scale 3", marginBottom, 0);

    printf("summary passed=%d failed=%d\n", g_pass, g_fail);
    printf("titlePixels: scale1=%d scale2=%d scale3=%d\n",
           titlePixels1, titlePixels2, titlePixels3);
    printf("footerPixels: scale1=%d scale2=%d scale3=%d\n",
           footerPixels1, footerPixels2, footerPixels3);

    M11_GameView_Shutdown(&game);
    return g_fail == 0 ? 0 : 1;
}
