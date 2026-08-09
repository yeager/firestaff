/*
 * DM1 V1 utility panel font-scale fit probe.
 *
 * Source evidence:
 *   m11_draw_utility_panel (src/engine/m11_game_view.c:48422) draws
 *   the right-column action/spell area.  In debug HUD mode, host
 *   text "MENU" and champion metadata appear at (222, 34) and
 *   (250, 34) using g_text_small (scale=1).  Both flow through
 *   m11_draw_text which applies g_m11_font_scale_override.
 *
 *   In normal DM1 source mode with authentic GRAPHICS.DAT C009/C010,
 *   no host text is drawn on the utility panel — the source assets
 *   own the entire strip.  This probe therefore tests the debug HUD
 *   path to verify fontScale propagation, then confirms that the
 *   source-locked action/spell strip is NOT affected by fontScale.
 *
 * What this probe locks:
 *
 *   1. In debug HUD mode, the "MENU" label is drawn at scale 1/2/3
 *      and its pixel count grows monotonically.
 *
 *   2. In source mode with authentic assets, the action/spell strip
 *      (x=224, y=45, w=87, h=78) pixel content is identical at
 *      every fontScale — fontScale must NOT leak into source-owned
 *      bitmap surfaces.
 *
 * This probe skips safely when DM1 PC 3.4 data files are absent.
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"
#include "ui_scale_m11.h"
#include "firestaff/dm1/v1/box_action_area_pc34_compat.h"

#include <stdio.h>
#include <string.h>
#include "firestaff_dm1_probe_data_dir.h"

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    PROBE_FB_W = 320,
    PROBE_FB_H = 200,
    PROBE_WHITE = 15,
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

static uint32_t region_hash(const unsigned char* fb,
                            int x, int y, int w, int h) {
    uint32_t hash = 2166136261u;
    int yy, xx;
    if (!fb) return 0;
    for (yy = y; yy < y + h; ++yy) {
        if (yy < 0 || yy >= PROBE_FB_H) continue;
        for (xx = x; xx < x + w; ++xx) {
            if (xx < 0 || xx >= PROBE_FB_W) continue;
            hash ^= (uint32_t)fb[yy * PROBE_FB_W + xx];
            hash *= 16777619u;
        }
    }
    return hash;
}

int main(int argc, char** argv) {
    const char* root = argc > 1 ? argv[1] : NULL;
    const char* dataDir;
    char narrowed[512];
    M12_StartupMenuState menu;
    M11_GameViewState game;
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];

    if (!root) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = firestaff_dm1_probe_narrow_data_dir(root, narrowed, sizeof(narrowed));

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        printf("SKIP could not open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 0;
    }

    printf("probe=firestaff_dm1_v1_utility_panel_font_scale_fit_runtime_probe\n");
    printf("dataDir=%s\n", dataDir);

    /* (1) Debug HUD mode: "MENU" label at (222, 34) uses g_text_small.
     * fontScale should increase the white pixel count. */
    {
        int menuPixels1, menuPixels2, menuPixels3;
        int savedDebug = game.showDebugHUD;
        game.showDebugHUD = 1;

        game.fontScale = 1;
        memset(fb, 0, sizeof(fb));
        M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);
        menuPixels1 = (int)count_color(fb, 222, 30, 90, 16, PROBE_WHITE);

        game.fontScale = 2;
        memset(fb, 0, sizeof(fb));
        M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);
        menuPixels2 = (int)count_color(fb, 222, 30, 90, 16, PROBE_WHITE);

        game.fontScale = 3;
        memset(fb, 0, sizeof(fb));
        M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);
        menuPixels3 = (int)count_color(fb, 222, 30, 90, 16, PROBE_WHITE);

        check_ge("debug MENU white pixels at scale 1", menuPixels1, 4);
        check_ge("debug MENU white pixels at scale 2", menuPixels2, 4);
        check_ge("debug MENU white pixels at scale 3", menuPixels3, 4);
        check_true("debug MENU pixels grow (scale 2 > scale 1)",
                   menuPixels2 > menuPixels1);
        check_true("debug MENU pixels grow (scale 3 > scale 2)",
                   menuPixels3 > menuPixels2);

        printf("debugMenuPixels: scale1=%d scale2=%d scale3=%d\n",
               menuPixels1, menuPixels2, menuPixels3);

        game.showDebugHUD = savedDebug;
    }

    /* (2) Source mode: the action/spell strip must be fontScale-invariant.
     * Hash the strip region at scale 1/2/3 — all three must match. */
    {
        uint32_t hash1, hash2, hash3;
        DM1_V1_ActionAreaRectPc34 action = dm1_v1_action_area_rect_pc34();
        int stripX = action.x;
        int stripY = action.y;
        int stripW = action.w;
        int stripH = action.h + 33;

        game.showDebugHUD = 0;

        game.fontScale = 1;
        memset(fb, 0, sizeof(fb));
        M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);
        hash1 = region_hash(fb, stripX, stripY, stripW, stripH);

        game.fontScale = 2;
        memset(fb, 0, sizeof(fb));
        M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);
        hash2 = region_hash(fb, stripX, stripY, stripW, stripH);

        game.fontScale = 3;
        memset(fb, 0, sizeof(fb));
        M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);
        hash3 = region_hash(fb, stripX, stripY, stripW, stripH);

        check_true("source action/spell strip is fontScale-invariant (scale 2 == scale 1)",
                   hash2 == hash1);
        check_true("source action/spell strip is fontScale-invariant (scale 3 == scale 1)",
                   hash3 == hash1);

        printf("actionSpellStripHash: scale1=0x%08x scale2=0x%08x scale3=0x%08x\n",
               hash1, hash2, hash3);
    }

    printf("summary passed=%d failed=%d\n", g_pass, g_fail);
    M11_GameView_Shutdown(&game);
    return g_fail == 0 ? 0 : 1;
}
